#ifndef FITCONTROLLER_H
#define FITCONTROLLER_H

#include <QWidget>
#include <vector>
#include "histogram.h"
#include "project.h"

//class Transition;
/*
  Maximum entropy method taken from J.L.Tain, D. Cano-Ott, NIM 571 (2007) 728-738
  TODO move it to separate class
  */

class FitController : public QWidget
{
Q_OBJECT
public:
    explicit FitController();
    std::vector <float> getBetaIntensities() ;
    std::vector<float> getErrors();
    void applyFit (Histogram* expHist);

signals:

public slots:
private:
    void prepareExperiment(Histogram* expHist);
    void prepareLevelsPart();
    void prepareContaminationPart();
    void normalizeToExperiment();
    void makeFit();
    float findNormalisation();
    void findErrors();
    void notifyContaminations();
    void notifyDecay();

    double minEnergy;
    double maxEnergy;
    //double minLevelEn;
    //double maxLevelEn;
    double expNorm;
    double lambda;
    int histId;
    int nrOfIterations;
    int nrOfLevels;
    int nrOfContaminations;
    float normBeforeFit;
    float normAfterFit;
    vector <float> feedings;
    std::vector<float> experiment;
    std::vector< vector<float> > responses;
    vector <float> errors;
    Project* myProject ;
    std::vector<Transition*> transitionsUsed;
    vector<bool> intensityFitFlags;
    double normalizationFactor_;
    double feedingsToFitSum_;

};

#endif // FITCONTROLLER_H
