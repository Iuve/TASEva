#ifndef FITCONTROLLER_H
#define FITCONTROLLER_H

#include <QWidget>
#include <vector>
#include "histogram.h"
#include "project.h"

//class Transition;
/*
  Maximum entropy method taken from J.L.Tain, D. Cano-Ott, NIM 571 (2007) 728-738

  Bayesian method taken from P.Shuai et al. (2021)
  equation A3 from appendix A describes method for single module
  equation A4 is for coincidence (2D) spectrum - NOT implemented
  equation A5 shows how to deal with multiple modules and 2D spectrums,
  which is implemented here as "sum" of multiple modules / A3 equations
  */

class FitController : public QWidget
{
Q_OBJECT
public:
    explicit FitController();
    std::vector <float> getBetaIntensities() ;
    std::vector<float> getErrors();
    void applyMaximumLikelyhoodFit (Histogram* expHist);
    void applyBayesianFit (std::vector< std::pair<Histogram*, int> > bayesianHistograms);
    double GetAverageBetaEnergy(){return averageBetaEnergy_;}

signals:

public slots:
private:
    void prepareExperiment(Histogram* expHist);
    void prepareLevelsPart(int simID);
    void prepareContaminationPart(std::vector< std::pair<int, Contamination> > contaminations, int hisID);
    void normalizeToExperiment();
    void BinForFitting();
    void makeLikelyhoodFit();
    void makeBayesianFit();
    void findErrors();
    void notifyContaminations();
    void notifyDecay();

    double minEnergy;
    double maxEnergy;
    double averageBetaEnergy_;
    //double minLevelEn;
    //double maxLevelEn;
    double expNorm;
    double lambda;
    int binning;
    int expHistId;
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

    vector< vector< vector<float> > > bayesianResponses;
    vector< vector<float> > bayesianFeedings;
    vector< vector<float> > bayesianExperiments;
    vector<double> bayesianNormalizationFactors;
    vector<double> bayesianExpNorms;
    vector<double> bayesianContaminationNormalization;
};

#endif // FITCONTROLLER_H
