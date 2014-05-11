#ifndef PCOA_H
#define PCOA_H

#include "ap.h"
#include "variation.h"
#include "trackcontainer.h"
#include "mainwindow.h"
#include "phylip.h"

class PCOA
{
public:
    PCOA();

    alglib::ae_int_t nVars;//number of dimension
    alglib::ae_int_t nPoints; // number of points
    alglib::real_2d_array oriDataMatrix;
    alglib::real_2d_array dissimilarityMatrix;
    alglib::real_2d_array projectionMatrix;
    alglib::real_1d_array symmetryMean;//mean of each column, mean of each row
    alglib::real_2d_array finalData, finalDataTransposed;

    double grandMean;

    int constructDissimilarityMatrix(Variation *var, QStringList selectedList, QList<VariationPosition *> vpList, bool indelsExcluded);

    void runPcoaAlgorithm(int numberOfPoints);
    void printDissimilarityMatrix(int numberOfPoints, QList<int> selectedIndex, Variation *var);
    static bool isSelectedSample(QString sampleName, QStringList itemList);

signals:
    void oneLineFinished();

};

#endif // PCOA_H
