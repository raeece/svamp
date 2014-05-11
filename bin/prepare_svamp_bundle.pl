#!/usr/bin/perl
use strict;
use Getopt::Long;

my $vcf;
my $bam_dir;
my $tsv;
my $help;
my $program_name="prepare_svamp_bundle.pl";

use Time::HiRes qw(gettimeofday);

# Parse the command line parameters in POSIX style
my $usage = "
Usage : $program_name -v <vcf> -t <tab separated sample information txt file> 
optional -b <bam dir>
\n";
GetOptions('help'=>\$help,'vcf=s' => \$vcf,'tsv=s' => \$tsv,'bamdir=s' => \$bam_dir);
defined $help and die  $usage;
# now validate all the parameters 
(!defined $vcf or !defined $tsv) and die $usage;
if(! -e "$vcf"){
	die "please check if $vcf is present\n";
}
if(! -e "$tsv"){
	die "please check if $tsv is present\n";
}
if(defined $bam_dir && ! -e "$bam_dir"){
	die "please check if $bam_dir is present\n";
}
my $timestamp = int (gettimeofday * 1000);
my $sql_filename="svamp_".$timestamp.".sql";
my $db_filename="svamp_".$timestamp.".db";
open(SQL,">".$sql_filename) or die "Cant open ".$sql_filename;
print SQL "PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE variationTransaction(position int, sample varchar(255), haplotype int, base varchar(2), chrom varchar(255));\n";
my %chrom_list=();
#open the vcf 
open(VCF,"<".$vcf) or die "Cant open ".$vcf;
my @vcf_samples=();
my @vcf_header=();
while(my $line=<VCF>){
	chomp($line);
# process header
	if($line=~m/^#CHROM/){
		@vcf_header=split("\t",$line);
		my $len=scalar(@vcf_header);
		my @rheaders=@vcf_header[8..$len-1];
		@vcf_samples=grep !/FORMAT/,@rheaders;	
#print "@vcf_samples\n";
	}
# process data lines
	if($line!~m/^#/){
		my @vcf_data=split("\t",$line);
		my %index;
		@index{@vcf_header} = (0..$#vcf_header);
		my $chrom = $vcf_data[$index{"#CHROM"}];
		$chrom_list{$chrom}=1;
		my $pos = $vcf_data[$index{"POS"}];
		my $ref = $vcf_data[$index{"REF"}];
		my @alt = split(",",$vcf_data[$index{"ALT"}]);
		if(scalar(@alt)<=0){
			push(@alt,$vcf_data[$index{"ALT"}]);
		}
		for my $sample_name(@vcf_samples){
			my $sample_data = $vcf_data[$index{$sample_name}];
			my @genotype=split(":",$sample_data);
			if(scalar(@genotype)>0 && $genotype[0] ne "."){
				my @gtype=split('/',$genotype[0]);
				my $i=1;
				for my $g (@gtype){
					my $base="";
					$base=($g eq "0")?$ref:$alt[$g-1];
#print "$sample_name\t$genotype[0] chrom = $chrom pos=$pos ref=$ref alt=@alt base=$base\n";
					print SQL  "INSERT INTO variationTransaction VALUES($pos,\'$sample_name\',$i,\'$base\',\'$chrom\');\n";
					$i++;
				}
			}
		}

	}
}
close(VCF);

# check if bam files are present and generate coverage file
if(defined $bam_dir){
	system ("mkdir temp");
	for my $chrom (keys %chrom_list){
	for my $s_name (@vcf_samples){
		if(! -e "$bam_dir/$s_name".".bam"){
			print "BAM file $bam_dir/$s_name".".bam not found\n";
		}
		else{
			print "Generating coverage for $s_name on chromosome $chrom\n"; 
			system("echo -n $s_name > temp/$s_name".".txt");

#	system("bedtools genomecov -d -ibam $bam_dir/$s_name".".bam | cut -f3 >> temp/$s_name".".txt");
			my $command =" bedtools genomecov -bga -ibam $bam_dir/$s_name."."bam | grep -P '^.*\\t[\\d][\\d]*\\t[\\d][\\d]*\\t0\$' | perl -e \'print \"\\t\";\$longstr=\"\";while(<STDIN>){chomp;(\$chrom,\$start,\$end,\$zero)=split /\\t/;\$longstr=\$longstr.\"\".(\$start+1).\"-\".(\$end+1).\":\";} \$len=length\(\$longstr\);print \"\".substr\(\$longstr,0,\$len-1\).\"\\n\";\' >> temp/$s_name".".txt";
			print "$command\n";
			system($command);
		}
	}
my @sample_files=map {"temp/".$_.".txt"} @vcf_samples; 
system("echo \"$chrom\" > genomecov.txt && cat @sample_files >> genomecov.txt && rm -rf temp");
#system("cat @sample_files > genomecov.txt");
}
}

# check the tsv 
use Geo::Coder::Google;

my $geocoder = Geo::Coder::Google->new(apiver => 3);

#p $location->{geometry}{location};
#print "\n";

my %sample_geo_location=map {$_ => -1} @vcf_samples; 
my %sample_year=map {$_ => -1} @vcf_samples; 
my %sample_place=map {$_ => -1} @vcf_samples; 
my %sample_country=map {$_ => -1} @vcf_samples; 
my %geo_cache=();
open(TSV,"<".$tsv) or die "Cant open ".$tsv;
# skip the first line
my $line=<TSV>;
while(my $line=<TSV>){
	chomp($line);
	my @columns=split("\t",$line);
	my $s_name=$columns[0] if $columns[0] ne "";
	$sample_year{$s_name}=$columns[1] if $columns[1] ne "";
	$sample_country{$s_name}=$columns[2] if $columns[2] ne "";
	$sample_place{$s_name}=$columns[3] if $columns[3] ne "";
	my $lat=$columns[4] if $columns[4] ne "";
	my $lng=$columns[5] if $columns[5] ne "";
	my $glookup="";
	if($columns[3]){
	$glookup="\'".$columns[3].", ".$columns[2]."\'";
	}
	else{
	$glookup="\'".$columns[2]."\'";
	}
#if lat,long not provided use google api to lookup
	if($lat eq "" || $lng eq ""){
		if(!$geo_cache{$glookup}){
			my $location = $geocoder->geocode(location => $glookup);
			my $lref=$location->{geometry}{location};
			print "obtaining geo cordinates for $glookup \t".$lref->{lat}.",".$lref->{lng}."\n";
			if($lref->{lat} && $lref->{lng}){
				$sample_geo_location{$s_name}=$lref->{lat}.",".$lref->{lng};
				$geo_cache{$glookup}=$lref->{lat}.",".$lref->{lng};
				sleep(2);
			}
		}
		else{
			print "obtaining geo cordinates for $glookup [CACHED]\t".$geo_cache{$glookup}."\n";
			$sample_geo_location{$s_name}=$geo_cache{$glookup};
		}
	}
	else{
		print "using user provided geo cordinates for $glookup \t".$lat.",".$lng."\n";
		$sample_geo_location{$s_name}=$lat.",".$lng;
		$geo_cache{$glookup}=$lat.",".$lng;
	}
}
close(TSV);

for my $k (keys %sample_geo_location){
	print "MISSING values for sample $k\t$sample_place{$k}\t$sample_country{$k}\t$sample_geo_location{$k}\n" if ($sample_geo_location{$k}==-1 || $sample_place{$k}==-1||$sample_country{$k}==-1);
}
print SQL "CREATE TABLE location(ID int,place varchar(255),country varchar(255), latitude double, longitude double);\n";
my $id=1;
my %geo_cache_id=();
for my $k (keys %geo_cache){
	my ($place,$country)=split(",",$k);
	$place =~ s/^\'//;
	$country =~ s/^\s+//;
	$country =~ s/\'$//;
	my ($lat,$lng)=split(",",$geo_cache{$k});
	print "$id\t$place\t$country\t$lat\t$lng\n";
	print SQL "INSERT INTO location VALUES($id,\'$place\',\'$country\',$lat,$lng);\n";
	$geo_cache_id{$k}=$id;
	$id++;
}
print SQL "INSERT INTO location VALUES(-1,\'Ungrouped\',\'Ungrouped\',0,0);\n";

print SQL "CREATE TABLE Samples (ID int, Name varchar(255), Year int, Origin_id int);\n";
my $sample_id=1;
for my $s_name(@vcf_samples){
	my $glookup="\'".$sample_place{$s_name}.", ".$sample_country{$s_name}."\'";
	my $glid=($geo_cache_id{$glookup})?$geo_cache_id{$glookup}:-1;
	print "$sample_id\t$s_name\t$sample_year{$s_name}\t$glid\n";
	print SQL "INSERT INTO samples VALUES($sample_id,\'$s_name\',$sample_year{$s_name},$glid);\n";
	$sample_id++;
}
print SQL "COMMIT;\n";
close(SQL);
system("cat $sql_filename | sqlite3 $db_filename");
