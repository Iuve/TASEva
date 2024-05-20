#ifndef TWODIMFITCONTROLLER_H
#define TWODIMFITCONTROLLER_H

 #include <QWidget>

//#include "DecayPath.hh"
#include "histogram.h"
#include "project.h"

//namespace  Ui  {
//class Analysis2D;
//}

class TwoDimFitController : public QWidget
{

public:
    TwoDimFitController(QWidget *parent=0);
 //   TwoDimFitController();
    ~TwoDimFitController();
    void makeXGammaFit();
    void setHistId(int histId){histId_ = histId;}
    void setLambda_(double lambda){lambda_ = lambda;}
    void setNrOfIter(int nrOfIter){nrOfIter_ = nrOfIter;}
    void setLevelEnergy(double energy){energy_ = energy;}
    void setExperimentalHistogram(Histogram* hist);
    void setExperimentalVector(){experiment_ = expGate_->GetAllDataD();}
    void setSimulatedHistogram(Histogram *histSim){simGate_ =  *histSim;}
//EVa    void setCurrentLevel(Level* level){level_ = level;}
    void setLeftLimit(double minEn){minEn_ = minEn;}
    void setRightLimit(double maxEn){maxEn_ = maxEn;}
    void setBinningFactor(int bin){binningFactor_ = bin;}

    int getHistId(){return histId_;}
    double getLambda(){return lambda_;}
    int getNrOfIter(){return nrOfIter_;}
    double getLevelEnergy(){return energy_;}
    Histogram* getExperimentalHistogram(){return expGate_;}
    Histogram* getSimulatedHistogram(){return &simGate_;}
    Histogram* getRecalculatedHistogram(){return &recGate_;}
    //Histogram* getLevelsRespHistogram(){return &levelsResp;}
    Histogram* getDiffHistogram(){return &diffHist_;}
    Level* getCurrentLevel(){return level_;}

    void prepareRestLevelsResponseFromOutside();

    void prepareTransitionResponses();
    void prepareFeedings();
    void findCorrespondingLevel();
    void calculateSimulatedHistogram();
    void calculateDiffHistogram();

private:
    double energy_;
    int histId_;
    DecayPath* decayPath;
    Project* myProject;
    Level* level_;
    double betaFeedingToLevel_;
    //Level* currentLevel_;
    double minEn_;
    double maxEn_;
    int nrOfIter_;
    double lambda_;
    int binningFactor_;


    float min;
    float max;
    vector <double> feedings;
    vector <double> feedingsBeforeFit;
    std::vector<double> experiment_;
    std::vector< vector<double> > responses;
    //std::vector <Histogram>* gammaRespHist;
    Histogram* otherLevelsResponse_;    // contribution from other levels
    Histogram* neutronLevelsResponse_;
    Histogram* expGate_;
    Histogram simGate_;
    Histogram recGate_;
    Histogram diffHist_;
    vector <double> errors;
    double otherLevelsToFeedingsRatio_;
    int normalizeStartPoint_;
    int normalizeEndPoint_;
    int expSpectra2Dbinning_;

    void PrepareNeutronLevelsResponse();
    Histogram prepareTransitionResponse(Transition *transition_, Level *tmpLevel_);
    Histogram prepareRestLevelsResponse();
    void calculateRecHistogram();
    void saveResponses();
    void makeFit();
    void BinForFitting(std::vector<double>* exp, std::vector< vector<double> >* resp);
    void findErrors();
    void notifyObservers();
    void printResults();

};
#endif // TWODIMFITCONTROLLER_H
