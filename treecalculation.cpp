#include "treecalculation.h"
#include <QMessageBox>
#include <QFile>
#include <QApplication>
#include <QDebug>

extern "C"
{
#include "phylip.h"
#include "fitch.c"
#include "dist.h"
}

void TreeCalculation::setParamaters(QString inFileName, QString outFileName, QString outTreeFile, long spNum,  bool globalArr,
                           bool ranSp,int methodCurrentIndex, bool negallow,
                           int dmatrixCurrentIndex, int npower, bool replicatesChecked,
                           bool printtreeChecked, bool outtreeChecked)
{
    this->inFileName = inFileName;
    this->outFileName = outFileName;
    this->outTreeFile = outTreeFile;
    this->spNum = spNum;

    putchar('\n');
    minev = false;
    jumble = false;
    njumble = 3;
    lengths = false;
    outgrno = 1;
    outgropt = false;
    usertree = false;
    printdata = false;
    progress = true;

    if(globalArr)
        global = true;
    else
        global = false;

    if(ranSp)
    {
        jumble = true;
        inseed = 0.5;
    }
    else
    {
        jumble = false;
        njumble= 1;
    }

    if (methodCurrentIndex==0)
    {
        minev = true;
        negallowed = true;
    }else
    {
        minev=false;
    }

    if(negallow)
        negallowed=true;
    else
        negallowed=false;

    if(dmatrixCurrentIndex==0)
    {
        fitchlower=false;
        upper=false;
    }else if (dmatrixCurrentIndex==1)
    {
        fitchlower=false;
        upper=true;
    }else
    {
        fitchlower=true;
        upper=false;
    }

    outgrno = spNum;

    power=npower;

    if(replicatesChecked)
        replicates=true;
    else
        replicates=false;

    if(printtreeChecked)
        treeprint=true;
    else
        treeprint=false;

    if(outtreeChecked)
        trout=true;
    else
        trout=false;
}

void TreeCalculation::treecomputation()
{
    init();

    /*==converting QString to char*==*/
    QByteArray bIn = inFileName.toLatin1();
    char * cIn = bIn.data();

    QByteArray bOut = outFileName.toLatin1();
    char * cOut = bOut.data();

    QByteArray bOutTree = outTreeFile.toLatin1();
    char * cOutTree = bOutTree.data();

    QString text ;

    FILE * myinfile;
    if(!(myinfile=fopen(cIn, "r+")))
    {
        emit processError("can't find file input");
        return;
    }

    FILE * myoutfile;
    if(!(myoutfile=fopen(cOut, "w+")))
    {
        emit processError("can't find file output");
        return;
    }

    FILE * myouttree;
    if(!(myouttree=fopen(cOutTree, "w+")))
    {
        emit processError("can't create output tree");
        return;
    }

    /*==========reset value============*/
    samenumsp2error = false;
    inputnumbers2error = false;
    odd_mallocerror = false;

    // start the tree computation :
    if(likemain(myinfile, myoutfile, myouttree)==0 && !cancel)
    {
        successfull=true;
        emit successAchieved();
        text= "Process completed";

    }else
    {
        if (!checkFileFormat)
        {
            text= "The distance matrix file is of the wrong type.";
        }
        else if (!checkZeroDiagonal)
        {
            text = QString("ERROR: diagonal element of row %1 of distance matrix is not zero.\n Is it a distance matrix?").arg(nonZeroRow);
        }
        else if (!checkSymmetric)
        {
            text =QString("ERROR: distance matrix is not symmetric:\n (%1,%2) element and (%3,%4) element are unequal\n"
                          "They are %5 and %6 respectively.\n Is it a distance matrix?").arg(unsymcoord1[0]).arg(unsymcoord1[1]).arg(unsymcoord2[0]).arg(unsymcoord2[1]).arg(unsym[0]).arg(unsym[1]);
        }
        else  if (inputnumbers2error)
        {
            text= "ERROR: Unable to read the number of species in data set\n The input file is incorrect (perhaps it was not saved text only).";
        }
        else if (samenumsp2error)
        {
            text =QString("ERROR: Unable to read number of species in data set %1, The input file is incorrect (perhaps it was not saved text only).").arg(ith);
        }
        else if(odd_mallocerror)
        {
            text="ERROR: a function asked for an inappropriate amount of memory. Either the input file is incorrect or there is a bug in the program.";
        }
        else if(incorrectntree)
        {
            QString intrname = QString(intreename);
            text = QString("ERROR: number of input trees is read incorrectly from %1.").arg(intrname);
        }else if (cursperr)
        {
            text = QString("ERROR: Inconsistent number of species in data set %1.").arg(ith);
        }
        else if (eofspeciesname)
            text="ERROR: end-of-line or end-of-file in the middle of species name.";
        else if(illegalchar)
            text="Species name may not contain characters ( ) : ; , [ ]";
        else if(dotsinbranch)
            text="ERROR: Branch length found with more than one \'.\' in it.";
        else if (plusinbranch)
            text="ERROR: Branch length found with \'+\' in an unexpected place.";
        else if (dashinbranch)
            text="ERROR: Branch length found with \'-\' in an unexpected place.";
        else if (einbranch)
            text="ERROR: Branch length found with more than one \'E\' in it.";
        else if (openloop)
            text="Error: a loop of nodes was not closed";
        else if(commentbracketsserr)
            text="ERROR: Unmatched comment brackets";
        else if(treenumerr)
            text="Error reading number of trees in tree file";
        else
        {
            text="Process terminated";
        }
        emit processError(text);
    } /*else of a group of errors checking*/
    stop();
    return;
}

void TreeCalculation::setMessage(const QString &message)
{
    messageStr = message;
}

void TreeCalculation::stop()
{
    cancel = true;
}

void TreeCalculation::init()
{
    cancel = false;
    successfull = false;
}

int TreeCalculation::maketree()
{
  /* contruct the tree */
  long nextsp,numtrees;

  //printf("updated spp %d", spp);
  boolean succeeded=false;
  long i, j;
  if (jumb == 1) {
      if(inputdata(replicates, printdata, fitchlower, upper, x, reps)<0)
          return -1;
      setuptree(&curtree, nonodes2);
      setuptree(&priortree, nonodes2);
      setuptree(&bestree, nonodes2);
      if (njumble > 1) setuptree(&bestree2, nonodes2);
  }
  for (i = 1; i <= spp; i++)
      enterorder[i - 1] = i;
  if (jumble)
      randumize(seed, enterorder);
  nextsp = 3;
  buildsimpletree(&curtree, nextsp);
  curtree.start = curtree.nodep[enterorder[0] - 1]->back;
  if (jumb == 1) numtrees = 1;
  nextsp = 4;
  if (progress) {
#ifdef WIN32
      phyFillScreenColor();
#endif
  }
  successfull=false;
  while (nextsp <= spp && !cancel) {

      nums = nextsp;
      buildnewtip(enterorder[nextsp - 1], &curtree, nextsp);
      copy_(&curtree, &priortree);
      bestree.likelihood = -DBL_MAX;
      curtree.start = curtree.nodep[enterorder[0] - 1]->back;
      addtraverse(curtree.nodep[enterorder[nextsp - 1] - 1]->back,
                  curtree.start, true, &numtrees,&succeeded);
      copy_(&bestree, &curtree);
      if (progress) {
#ifdef WIN32
          phyFillScreenColor();
#endif
      }
      if (global && nextsp == spp) {
          if (progress) {
              /*printf("Doing global rearrangements\n");
              printf("  !");
              for (j = spp; j < nonodes2; j++)
                  if ( (j - spp) % (( nonodes2 / 72 ) + 1 ) == 0 )
                      putchar('-');
              printf("!\n");
              printf("   ");*/
          }
      }
      succeeded = true;
      while (succeeded) {
          succeeded = false;
          curtree.start = curtree.nodep[enterorder[0] - 1]->back;
          if (nextsp == spp  && global)
          {
              globrearrange (&numtrees,&succeeded);
              if(openloop)return -1;
          }
          else{
              rearrange(curtree.start,&numtrees,&nextsp,&succeeded);
          }
          if (global && ((nextsp) == spp) && progress)
              printf("\n   ");
      }
      if (global && nextsp == spp) {
          putc('\n', outfile);
          if (progress)
              putchar('\n');
      }
      if (njumble > 1) {
          if (jumb == 1 && nextsp == spp)
              copy_(&bestree, &bestree2);
          else if (nextsp == spp) {
              if (bestree2.likelihood < bestree.likelihood)
                  copy_(&bestree, &bestree2);
          }
      }
      if (nextsp == spp && jumb == njumble) {
          if (njumble > 1) copy_(&bestree2, &curtree);
          curtree.start = curtree.nodep[outgrno - 1]->back;
          printree(curtree.start, treeprint, true, false);
          summarize(numtrees);
      }
      nextsp++;
      emit anotherSampleDone();
  }

}  /* maketree */

int TreeCalculation::likemain(FILE * infilename, FILE * outfilename, FILE * outtreename)
{
    int i;
    int argc = 1; //added
    Char * argv[argc]; //added
    //#ifdef MAC  //why here it is initially only for MAC?
    argc = 1;                /* macsetup("Fitch","");        */
    argv[0]="Fitch";
    //#endif
    //init(argc,argv);

    progname = argv[0];

    infile = infilename;
    outfile = outfilename;

    ibmpc = IBMCRT;
    ansi = ANSICRT;
    mulsets = false;
    datasets = 1;
    firstset = true;
    doinit();
    if(inputnumbers2error)
        return -1;
    if (trout) outtree = outtreename;

    for (i=0;i<spp;++i){
        enterorder[i]=0;}
    for (ith = 1; ith <= datasets; ith++) {
        if (datasets > 1) {
            fprintf(outfile, "Data set # %ld:\n\n",ith);
        }
        fitch_getinput();
        if (samenumsp2error)
            return -1;
        for (jumb = 1; jumb <= njumble; jumb++)
        {
            if (maketree()<0)
                return -1;
            if (illegalchar || incorrectntree || dotsinbranch || plusinbranch || dashinbranch || einbranch || openloop || commentbracketsserr || treenumerr || speciesmissing)
                return -1;
        }
        firstset = false;
        if (eoln(infile) && (ith < datasets))
            scan_eoln(infile);
    }
    if (trout)
        FClose(outtree);
    FClose(outfile);
    FClose(infile);
#ifdef MAC
    fixmacfile(outfile);
    fixmacfile(outtree);
#endif
    return 0;
}
