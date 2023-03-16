//! Singleton class to hold all the data for the analysis project.

/*!
  Detailed description.
*/

#ifndef PROJECT_H
#define PROJECT_H

#include "histogram.h"
#include "contamination.h"
#include "DeclareHeaders.hh"

#include <QWidget>
#include <QString>

#include <string>
#include <vector>

using namespace std;

class Project
{

public:
    static Project* get();
    ~Project();

    string getProjectName() {return projectName_;} /**< Project Name */
    string getWorkingDir() {return workingDir_;}  /**< Working directory for the Project */
    string getExpFile() {return expFile_;}         /**< Experimental HIS File Name */
    string getExpSpecID() {return expSpecID_;}     /**< MTAS Total Spetrum ID from Experimental HIS File*/
    vector<string> getExpSpecIDVec() {return expSpecIDVec_;}
    void addExpSpecID(string id) {expSpecIDVec_.push_back(id);}
    string getExp2DSpecID() {return exp2DSpecID_;} /**< 2D spectrum ID for 2D fit */
    string getInputDecayFile() {return inputDecayFile_;}   /**<  Decay path data file name  */
    string getOutputDecayFile() {return outputDecayFile_;}  /**< Levelscheme output file  */
    string getOutputSIMFile() {return outputSIMFile_;}  /**< Simulated spectrum output HIS file name */
    string getOutputLEVFile() {return outputLEVFile_;}  /**< HIS file with level response functions */
    string getCodeGEANTName() {return codeGEANT_ ;}        /**< GEANT4 simulation program file name to be used */
//c    string contamination(int row, int column) {return contamination_[row].at(column);} /**< Returns string value from the contamination table (fileName,normalization,SpecID) */
    vector<vector <string> > getInputContaminations() {return inputContamination_;} /**< Returns Contamination matrix  (vec(vec()) */
    //vector<vector <string> > getInputContaminationsBayesian() {return inputContaminationBayesian_;}
    double getFitEnergyFrom() { return fitEnergyFrom_ ;}
    double getFitEnergyTo() { return fitEnergyTo_ ;}
    double getFitLevelsFrom() {return fitLevelsFrom_ ;}
    double getFitLevelsTo() {return fitLevelTo_ ;}
    double getFitLambda() {return fitLambda_ ;}
    int getFitBinning() {return fitBinning_ ;}
    int getNoFitIterations() {return noFitIterations_ ;}
    Histogram* getExpHist() {return &expHist_;}
    Histogram* getExp2DHist() {return &exp2DHist_;}
    Histogram* getDecHist() {return &decHist_;}
    Histogram* getRecHist() {return &recHist_;}
    Histogram* getDifHist() {return &difHist_;}
//    vector<Histogram*> conHist() {return conHist_;}
//    vector<double> getConNorm() {return conNorm_;}
    std::vector< std::pair<int, Contamination> >* getContaminations() {return &contaminations_;}
//Eva    Contamination* getOneContamination(int row) {return contaminations_.at(row);}
    std::vector<Contamination> getContaminationsSpecID(int HistID);
    void removeContamination(QString name, QString id);
    void removeAllContaminations();

    double get2DGate1Low(){return gate1Low;}
    double get2DGate1High(){return gate1High;}


    void setProjectName(string s) { projectName_ = s;}
    void setWorkingDir(string s); // { workingDir_ = s;}
    void setExpFile(string s) {expFile_ = s;}
    void setExpSpecID(string s) { expSpecID_ = s;}
    void setExp2DSpecID(string s) { exp2DSpecID_ = s;}
    void setInputDecayFile(string s) {inputDecayFile_ = s;}
    void setOutputDecayFile(string s) { outputDecayFile_ = s;}
    void setOutputSIMFile(string s) { outputSIMFile_ = s;}
    void setOutputLEVFile(string s) { outputLEVFile_ = s;}
    void setCodeGEANTName(string s) { codeGEANT_ =s ;}
//c    void setContamination(int row, int column, string item) {contamination_[row].at(column) = item;}
    void setFitEnergyFrom(double d) { fitEnergyFrom_ = d;}
    void setFitEnergyTo(double d) { fitEnergyTo_ = d;}
    void setFitLevelsFrom(double d) {fitLevelsFrom_ =d;}
    void setFitLevelsTo(double d) {fitLevelTo_ = d;}
    void setFitLambda(double d) {fitLambda_ = d;}
    void setFitBinning(int d) {fitBinning_ = d;}
    void setNoFitIterations(int i) {noFitIterations_ = i;}
    void setExpHist(Histogram h) {expHist_ = h;}
    void setExpHist();
    void setExp2DHist(Histogram h) {exp2DHist_ = h;}
    void setDecHist(Histogram h) {decHist_ = h;}
    void setRecHist(Histogram h) {recHist_ = h;}
    void setDifHist(Histogram h) {difHist_ = h;}
//Eva    void setConHist(vector<Histogram*> con) {conHist_ = con;}
//Eva    void setConNorm(vector<double> vec) {conNorm_ = vec;}
    void setContaminations(std::vector< std::pair<int, Contamination> > newContaminations ) {contaminations_ = newContaminations;}
    void setOneContamination(int row, Contamination newContamination) {contaminations_.at(row).second = newContamination;}
    void Open(string fileName);
    void set2DGate1Low(double d){gate1Low = d;}
    void set2DGate1High(double d){gate1High = d;}
    Level* GetCurrent2DFitLevel(){return fit2DCurrentLevel_;}
    void setCurrent2DFitLevel(Level* level){fit2DCurrentLevel_ = level;}
    void setExpGate(Histogram h){expGate_ = h;}
    Histogram* getExpGate(){return &expGate_;}
    void setTransitionResponseHist(std::vector <Histogram> gammaRespHist){gammaRespHist_ = gammaRespHist;}
    void setTransitionResponseDouble(std::vector <vector <double> > responses){gammaRespFloat_ = responses;}
    std::vector <Histogram>* getTransitionResponseHist(){return &gammaRespHist_;}
    std::vector <vector <double> > getTransitionResponseFloat(){return gammaRespFloat_;}
    void setGate2DOtherLevelsContribution(Histogram histmp){Gate2DOtherLevelsContribution_ = histmp;}
    Histogram* getGate2DOtherLevelsContribution(){return &Gate2DOtherLevelsContribution_;}
    void SetGate2DNeutronLevelsContribution(Histogram histmp){Gate2DNeutronLevelsContribution_ = histmp;}
    Histogram* GetGate2DNeutronLevelsContribution(){return &Gate2DNeutronLevelsContribution_;}
    void setGateNormFactor(double gateNorm){gateNormFactor_ = gateNorm;}
    double getGateNormFactor(){return gateNormFactor_;}
    //void setGateOtherLevelsNormFactor(double gateNorm){gateOtherLevelsNormFactor_ = gateNorm;}
    //double getGateOtherLevelsNormFactor(){return gateOtherLevelsNormFactor_;}
    void setCustomTransitionIntensities(std::vector<double> customInt ) {customTransitionIntensities_ = customInt;}
    std::vector<double>* getCustomTransitionIntensities() {return &customTransitionIntensities_;}
    void setBinning1Dfactor(int bin){binning1Dfactor_ = bin;}
    int getBinning1Dfactor(){return binning1Dfactor_;}
    void setBinning2Dfactor(int bin){binning2Dfactor_ = bin;}
    int getBinning2Dfactor(){return binning2Dfactor_;}
    void setActiveCoresForSimulation(int number){activeCoresForSimulation_ = number;}
    int getActiveCoresForSimulation(){return activeCoresForSimulation_;}

    QString getSortOutputFileName(){return sortOutputFileName_;}
    QString getSortOutputFileExtension(){return sortOutputFileExtension_;}

    void SetLastAutofitResults( std::vector< std::pair<double, double> > results ){ fitResults_ = results; }
    std::vector< std::pair<double, double> > GetLastAutofitResults(){ return fitResults_; }
    void addExpHist(int Id, Histogram hist);
    void replaceExpHistInMap(int Id, Histogram hist);
    Histogram* getHistFromExpMap(int Id);

    //public slots:
    void New();
    void Save();


private:
    Project();
    Project(Project const&);
    string projectName_;    /**< Project Name */
    string workingDir_;     /**< Working directory */
    string expFile_;        /**< Experimental file name */
    string expSpecID_;      /**< Current Experimental spectrum ID */
    vector<string> expSpecIDVec_ ; /**< VEctor with all exp IDs to be read */
    string exp2DSpecID_;
//    string inputENSFile_;
//    string outputENSFile_;
    string inputDecayFile_;
    string outputDecayFile_;
    string outputSIMFile_;
    string outputLEVFile_;
    string codeGEANT_;
    std::vector<std::vector <std::string> > inputContamination_;
    double fitEnergyFrom_;
    double fitEnergyTo_;
    double fitLevelsFrom_;
    double fitLevelTo_;
    double fitLambda_;
    int fitBinning_;
    int noFitIterations_;
    Histogram expHist_;
    Histogram exp2DHist_;
    Histogram decHist_;
    Histogram recHist_;
    Histogram difHist_;
//Evaout    vector<Histogram*> conHist_;
//Evaout    vector<double> conNorm_;
    std::vector< std::pair<int, Contamination> > contaminations_;
    std::map<int,Histogram> expHistMap_;
    std::vector<double> customTransitionIntensities_;

    //autofit 1D results: beta intensities and uncertainties
    std::vector< std::pair<double, double> > fitResults_;

// response function calculation
    QString sortOutputFileName_ = "sort";
    QString sortOutputFileExtension_ = ".root";

 //2Dfitvariables
    double Display2DXmin_, Display2DXmax_;
    double Display2DYmin_, Display2DYMax_;
    double gate1Low, gate1High;
    double gate2Low, gate2High;
    double fit2DGatelow,fit2DGateHigh;
    Level* fit2DCurrentLevel_;
    Histogram expGate_;    //Gate (#1) set on exp 2D spectrum
    std::vector <Histogram> gammaRespHist_;   // response functions of gamma deexting currentLevel_ in Histograms
    std::vector <vector <double> > gammaRespFloat_; // resp. func. of gamma deexcting currentLevel in vector<float> format
    // Gate2DOtherLevelsContribution_ = Gate2DNeutronLevelsContribution_ + OTHER
    Histogram Gate2DNeutronLevelsContribution_; //contribution from neutrons levels, all transitions
    Histogram Gate2DOtherLevelsContribution_;  //keeps contribution from levels of higier energy to the get set

    double gateNormFactor_;
    int binning1Dfactor_;
    int binning2Dfactor_;
    int activeCoresForSimulation_;

    void operator=(Project const&);
    static Project* instance;//
};

#endif // PROJECT_H
