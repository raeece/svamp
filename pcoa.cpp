#include "pcoa.h"
#include "ui_mainwindow.h"
#include "math.h"
#include "linalg.h"
#include "unistd.h"
#include <QProgressDialog>

PCOA::PCOA()
{

}

int PCOA::constructDissimilarityMatrix(Variation * var,QStringList selectedList, QList<VariationPosition*> vpList, bool indelsExcluded)
{
    int nPoints = var->samples.size();
    int nfiltered;


    QProgressDialog progressdialog;
    progressdialog.setLabelText(QString("Generating Distance Matrix for selected positions"));

    //calculate number of considered SNPs
    nVars= vpList.size();

    /*int nSelected=nPoints;//equal to number of whole samples if selectedOnly is false
    if(selectedOnly) nSelected=selectedList.count();*/

    //store index of selected samples
    QList<int> selectedIndex;
    for (int s=0;s<nPoints;s++)
    {
        if(isSelectedSample(var->samples[s],selectedList))
        {
            selectedIndex.append(s);
        }
    }

    int nSelected = selectedIndex.count();

    progressdialog.setRange(0,nSelected);
    progressdialog.setModal(false);
    progressdialog.setValue(0);
    progressdialog.setAutoClose(true);
    progressdialog.show();

    oriDataMatrix.setlength(nSelected, nVars);
    dissimilarityMatrix.setlength(nSelected,nSelected);

    double tempValue;

    nfiltered = vpList.size();

    for (int z=0;z<nSelected;z++)
    {
        int x=selectedIndex.at(z);

        for (int c=z+1;c<nSelected;c++)
        {
            int y=selectedIndex.at(c);
            double score=0;
            //qDebug() << "comparing x=" << x << " and " << y << ": " << var->samples[x] << "|" << var->samples[y] ;
            int discardedSnp = 0;
              for (int i=0; i< nfiltered; i++)//iterating through the SNPs
                {
                VariationPosition *v = vpList.at(i);

                VariationData gt = v->data[x];
                VariationData cp = v->data[y];

                //building dissimilarity matrix
                double dissimilarity=0;

                if(indelsExcluded && (gt.insertion || gt.deletion || cp.insertion || cp.deletion)) // if Indels is filtered and the thing is an indel
                {
                    discardedSnp++;
                }
                else if (gt.haplotype1==-1 || gt.haplotype2==-1 || cp.haplotype1==-1 || cp.haplotype2==-1){
                //if ( (gt.haplotype1!=gt.haplotype2) || (cp.haplotype1!=cp.haplotype2)){
                        discardedSnp++;
                }
                else{
                if (gt.haplotype1!=cp.haplotype1)
                    dissimilarity++;
                if (gt.haplotype2!=cp.haplotype2)
                    dissimilarity++;

                score=score+dissimilarity/2;
                }
                //qDebug() << gt.haplotype1 << "/" << gt.haplotype2 << "--" << cp.haplotype1 << "/" << cp.haplotype2 << "=" << score;

            }
            if(nVars!=discardedSnp){
            tempValue= score/(nVars-discardedSnp);
            }
            else{
                tempValue=0;
            }
            //qDebug() << "[" << c << "][" << z << "]=" << tempValue;
            dissimilarityMatrix[c][z] = tempValue;
            dissimilarityMatrix[z][c] = tempValue;
        }

        //fill diagonal with zero
        dissimilarityMatrix[z][z] = 0;

        progressdialog.setValue(z+1);
        qApp->processEvents();

        if (progressdialog.wasCanceled())
        {
            return -1;
        }
    }
    this->printDissimilarityMatrix(nSelected, selectedIndex, var);
    if(indelsExcluded)
        qDebug() << "Indels Excluded from Distance Matrix computation";
    return 0;
}


void PCOA::runPcoaAlgorithm(int numberOfPoints)
{

    projectionMatrix.setlength(numberOfPoints, numberOfPoints);
    symmetryMean.setlength(numberOfPoints);
    double tempRowTotal=0, tempGrandTotal=0;

    //1. dissimilarityMatrix^2
    for (int i=0;i<numberOfPoints;i++)
    {
        for (int j=0;j<numberOfPoints;j++)
        {
            projectionMatrix[i][j]= pow(dissimilarityMatrix[i][j],2);
            tempRowTotal=tempRowTotal+projectionMatrix[i][j];
            tempGrandTotal = tempGrandTotal+ projectionMatrix[i][j];
        }
        symmetryMean[i]=tempRowTotal/numberOfPoints;
        tempRowTotal=0;
    }

    /*2. double centering of X (“Double centering” is subtracting the row and column means of the
    matrix from its elements, adding the grand mean and multiplying by -1/2)*/
    grandMean = tempGrandTotal/(pow(numberOfPoints,2));
    for (int i=0;i<numberOfPoints;i++)
    {
        for (int j=0;j<numberOfPoints;j++)
        {
            projectionMatrix[i][j]= (projectionMatrix[i][j]-symmetryMean[i]-symmetryMean[j]+grandMean)*-0.5;
        }
    }

    //3. get eigenvalue
    alglib::real_1d_array W;
    alglib::real_2d_array Z;

    alglib::smatrixevd(projectionMatrix,numberOfPoints,1,true,W,Z);
    //qDebug() << "final data: ";

    /*Calculate the PCoA points multiplying the eigenvectors by the square root of the top eigenvalues */

    finalData.setlength(numberOfPoints,2);
    for (int x=0;x<numberOfPoints;x++)
    {
        for (int y=0;y<2;y++)
        {
            finalData[x][y]= Z[x][numberOfPoints-1-y]*sqrt(W[numberOfPoints-1-y]);

        }
        //qDebug() << finalData[x][0] << "---" << finalData[x][1];
    }

    //transposing final data for the plot sake
    finalDataTransposed.setlength(2,numberOfPoints);
    for (int x=0;x<2;x++)
    {
        for (int y=0;y<numberOfPoints;y++)
        {
            finalDataTransposed[x][y]= finalData[y][x];
        }

    }

}

void PCOA::printDissimilarityMatrix(int numberOfPoints, QList<int> selectedIndex, Variation * var)
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    QString matrixPhylip, outputFileName;
    matrixPhylip.append("matrixInPhylipFormat.txt");
    outputFileName.append("dissimilarityMatrix.txt");

    QFile disFile(outputFileName);
    QFile disFile2(matrixPhylip);

    if(disFile.open(QIODevice::WriteOnly | QIODevice::Text) && disFile2.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //opening files for similarity print

        QTextStream fout(&disFile); fout.setRealNumberPrecision(2);
        QTextStream fout2(&disFile2); fout2.setRealNumberPrecision(2);

        fout2 << numberOfPoints << "\n";

        for (int i=0;i<numberOfPoints;i++){
            fout<< var->samples[selectedIndex.at(i)]<<"\t";
        }
        fout<<"\n";
        for (int i=0;i<numberOfPoints;i++)
        {
            fout2 << var->samples[selectedIndex.at(i)].left(nmlngth) <<"    " ;
            if (var->samples[selectedIndex.at(i)].length()<nmlngth)
            {
                int ngap=nmlngth-var->samples[selectedIndex.at(i)].length();
                for (int y=0;y<ngap;y++)
                    fout2<< " " ;
            }

            for (int j=0;j<numberOfPoints;j++)
            {
                if (j<numberOfPoints-1)
                    fout << dissimilarityMatrix[i][j] <<"\t";
                else if (j==numberOfPoints-1)
                    fout << dissimilarityMatrix[i][j];

                if (j%7==0 && j!=0)
                {
                    if(j==numberOfPoints-1)
                        fout2 << dissimilarityMatrix[i][j];
                    else
                        fout2 << dissimilarityMatrix[i][j] <<"\n" ;
                }
                else
                    fout2 << dissimilarityMatrix[i][j] << "  " ;
            }
            fout2 << "\n";
            fout << "\n";
        }
        disFile.close();
        disFile2.close();
    }
}

bool PCOA::isSelectedSample(QString sampleName, QStringList itemList)
{
    //qDebug() << "isSelectedSample::checking " << sampleName;
    for (int y=0;y<itemList.size();y++)
    {
        if(itemList.at(y)==sampleName)
            return true;
    }
    return false;
}
