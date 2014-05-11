#include "thread.h"
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

Thread::Thread()
{
}

void Thread::setParamaters(QString inFileName, QString outFileName, QString outTreeFile, QString spNum,  bool globalArr,
                           bool ranSp, QString inseedText, QString njumbleText, int methodCurrentIndex, bool negallow,
                           int dmatrixCurrentIndex, bool outgroupChecked, int noutgroup, int npower, bool replicatesChecked,
                           bool printtreeChecked, bool outtreeChecked)
{
    this->inFileName = inFileName;
    this->outFileName = outFileName;
    this->outTreeFile = outTreeFile;
    this->spNum = spNum;

    putchar('\n');
    minev = false;
    jumble = false;
    njumble = 1;
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
        bool ok;
        inseed = inseedText.toLong(&ok,10);
        njumble= njumbleText.toLong(&ok,10);
        //initjumble(&inseed,&inseed0,seed,&njumble);
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

    if(outgroupChecked)
    {
        outgropt = true;
        outgrno = noutgroup;
    }else
        outgropt=false;

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

void Thread::treecomputation()
{
        bool ok;
        started = true;

        /*==converting QString to char*==*/
        QByteArray bIn = inFileName.toLatin1();
        char * cIn = bIn.data();

        QByteArray bOut = outFileName.toLatin1();
        char * cOut = bOut.data();

        QByteArray bOutTree = outTreeFile.toLatin1();
        char * cOutTree = bOutTree.data();

        QMessageBox msgBox;

        FILE * myinfile;
        if(!(myinfile=fopen(cIn, "r+")))
        {
            msgBox.setText("can't find fileinput");
            msgBox.exec();
            return;
        }

        FILE * myoutfile;
        if(!(myoutfile=fopen(cOut, "w+")))
        {
            msgBox.setText("can't create fileoutput");
            msgBox.exec();
            return;
        }

        FILE * myouttree;
        if(!(myouttree=fopen(cOutTree, "w+")))
        {
            msgBox.setText("can't create output tree");
            msgBox.exec();
            return;
        }

        /*==========reset value============*/
        samenumsp2error = false;
        inputnumbers2error = false;
        odd_mallocerror = false;

        //int start =  time(NULL);
        likemain(myinfile, myoutfile, myouttree);
        //int end = time(NULL);
        //int elapsed = end-start;

        if(spNum.toInt(&ok,10)>spp || spNum.toInt(&ok,10)<=0)
        {
            msgBox.setText("Bad species number. It should be less than or equal to total species with numbering starts from 1");
            msgBox.addButton("Return", QMessageBox::YesRole); //Button Special for Error Messages
        }else
        {
                if(!treenumerr && !commentbracketsserr && !openloop && !dashinbranch && !einbranch && !dotsinbranch && !plusinbranch && !illegalchar && !eofspeciesname && !cursperr && !unknownerr && !eoferror && !incorrectntree && !odd_mallocerror && !samenumsp2error && !inputnumbers2error && checkFileFormat && checkZeroDiagonal && checkSymmetric)
                {
                     msgBox.setText("Process completed.Click to view the tree");
                     successfull=true;
                }else
                {
                    QString text ;
                    if (!checkFileFormat)
                    {
                        msgBox.setText("The distance matrix file is of the wrong type.");
                    }
                    else if (!checkZeroDiagonal)
                    {
                        text = QString("ERROR: diagonal element of row %1 of distance matrix is not zero.\n Is it a distance matrix?").arg(nonZeroRow);
                        msgBox.setText(text);
                    }
                    else if (!checkSymmetric)
                    {
                        text =QString("ERROR: distance matrix is not symmetric:\n (%1,%2) element and (%3,%4) element are unequal\n"
                                      "They are %5 and %6 respectively.\n Is it a distance matrix?").arg(unsymcoord1[0]).arg(unsymcoord1[1]).arg(unsymcoord2[0]).arg(unsymcoord2[1]).arg(unsym[0]).arg(unsym[1]);
                        msgBox.setText(text);
                    }
                    else  if (inputnumbers2error)
                    {
                        msgBox.setText("ERROR: Unable to read the number of species in data set\n The input file is incorrect (perhaps it was not saved text only).");
                    }
                    else if (samenumsp2error)
                    {
                        text =QString("ERROR: Unable to read number of species in data set %1, The input file is incorrect (perhaps it was not saved text only).").arg(ith);
                        msgBox.setText(text);
                    }
                    else if(odd_mallocerror)
                    {
                        msgBox.setText("ERROR: a function asked for an inappropriate amount of memory. Either the input file is incorrect or there is a bug in the program.");
                    }
                    else if(incorrectntree)
                    {
                        QString intrname = QString(intreename);
                        text = QString("ERROR: number of input trees is read incorrectly from %1.").arg(intrname);
                        msgBox.setText(text);
                    }else if (cursperr)
                    {
                        text = QString("ERROR: Inconsistent number of species in data set %1.").arg(ith);
                        msgBox.setText(text);
                    }
                    else if (eofspeciesname)
                        msgBox.setText("ERROR: end-of-line or end-of-file in the middle of species name.");
                    else if(illegalchar)
                        msgBox.setText("Species name may not contain characters ( ) : ; , [ ]");
                    else if(dotsinbranch)
                        msgBox.setText("ERROR: Branch length found with more than one \'.\' in it.");
                    else if (plusinbranch)
                        msgBox.setText("ERROR: Branch length found with \'+\' in an unexpected place.");
                    else if (dashinbranch)
                        msgBox.setText("ERROR: Branch length found with \'-\' in an unexpected place.");
                    else if (einbranch)
                        msgBox.setText("ERROR: Branch length found with more than one \'E\' in it.");
                    else if (openloop)
                        msgBox.setText("Error: a loop of nodes was not closed");
                    else if(commentbracketsserr)
                        msgBox.setText("ERROR: Unmatched comment brackets");
                    else if(treenumerr)
                        msgBox.setText("Error reading number of trees in tree file");
                    else
                    {
                        msgBox.setText("Something wrong with the file format or distance matrix. Please check.");
                    }
                    msgBox.addButton("Return", QMessageBox::YesRole); //Button Special for Error Messages
                } /*else of a group of errors checking*/
            }/*else of spp number checking*/

        msgBox.exec();

        stop();
}

//void Thread::getAdvancedOptions(bool globalArrange,bool randomSp, QString inseedText, QString njumbleText, int comboMethodCurrentIndex, )
void Thread::setMessage(const QString &message)
{
    messageStr = message;
}

void Thread::stop()
{
    stopped = true;
    cancel = true;
    this->exit(0);
}

void Thread::init()
{
    cancel = false;
    successfull = false;
    stopped = false;
    started = false;
    //total = spp;
    //donesamples=0;
    setMessage("Tree Threads created");
}

void Thread::run()
{
    while (!stopped)
        if(!started) treecomputation();

    //stopped = false;
}

void Thread::maketree()
{
  /* contruct the tree */
  long nextsp,numtrees;
  //total = spp;
  emit treebuildingstarted(&spp);
  //printf("updated spp %d", spp);
  boolean succeeded=false;
  long i, j;
  if (jumb == 1) {
      if(inputdata(replicates, printdata, fitchlower, upper, x, reps)<0)
          return;
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
      printf("Adding species:\n");
      writename(0, 3, enterorder);
#ifdef WIN32
      phyFillScreenColor();
#endif
  }
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
          writename(nextsp  - 1, 1, enterorder);
#ifdef WIN32
          phyFillScreenColor();
#endif
      }
      if (global && nextsp == spp) {
          if (progress) {
              printf("Doing global rearrangements\n");
              printf("  !");
              for (j = spp; j < nonodes2; j++)
                  if ( (j - spp) % (( nonodes2 / 72 ) + 1 ) == 0 )
                      putchar('-');
              printf("!\n");
              printf("   ");
          }
      }
      succeeded = true;
      while (succeeded) {
          succeeded = false;
          curtree.start = curtree.nodep[enterorder[0] - 1]->back;
          if (nextsp == spp  && global)
          {
              globrearrange (&numtrees,&succeeded);
              if(openloop)return;
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
      //donesamples = nums;
      emit onesamplefinished();
  }

  if (jumb == njumble && progress) {
    printf("\nOutput written to file \"%s\"\n", outfilename);
    if (trout) {
      printf("\nTree also written onto file \"%s\"\n", outtreename);
    }
  }
}  /* maketree */

int Thread::likemain(FILE * infilename, FILE * outfilename, FILE * outtreename)
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
      //openfile(&infile,INFILE,"input file","r",argv[0],infilename);
      //openfile(&outfile,OUTFILE,"output file","w",argv[0],outfilename);

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
        //openfile(&outtree,OUTTREE,"output tree file","w",argv[0],outtreename);
      for (i=0;i<spp;++i){
        enterorder[i]=0;}
      for (ith = 1; ith <= datasets; ith++) {
        if (datasets > 1) {
          fprintf(outfile, "Data set # %ld:\n\n",ith);
          if (progress)
            printf("\nData set # %ld:\n\n",ith);
        }
        fitch_getinput();
        if (samenumsp2error)
            return -1;
        for (jumb = 1; jumb <= njumble; jumb++)
        {
            maketree();
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
      printf("\nDone.\n\n");
    #ifdef WIN32
      phyRestoreConsoleAttributes();
    #endif
      return 0;
}
