
#ifndef NEW_SDD_H
#define	NEW_SDD_H

#include "DeclareHeaders.hh"
#include "QtCore"

#include <string>
#include <vector>

class SaveDecayData
{
public:
    SaveDecayData(std::string path);
    SaveDecayData();
    ~SaveDecayData();

    void GeneralDecayInfo(); //calculates general information about the decay
    void SaveDecayStructure();  //writes to disk ALL XML nuclide files
    void SaveENSDecayStructure();  //writes structure in ENS format
    void SaveSpecifiedDecayStructure(Transition* firstTransition, Transition* secondTransition);
    void CreateDecayXML(Transition* firstTransition, Transition* secondTransition);  // create decay.xml (main driver)
    void SaveGeneralDecayInfo(std::string path);
    void SaveGammaEvolution();
    std::vector<std::string> GetXmlFilenames() {return fullFileNames_;}
    std::vector<std::string> GetShortXmlFilenames() {return shortFileNames_;}
    double CalcGammaMultiplictyFromLevel(Level* currentLevel, Level* stopLevel, int gammaMultiplicity = 1);

    QString setENSIdentificationRec(QString Id, QString header, Nuclide* parent);
    QString setENSQvalueRec(QString header, Nuclide* nuclide);
    QString setENSParentRec(std::vector<Nuclide>* nuclidesVector, bool delayedParticle);
    QString setENSNormRec(QString header);
    QString setENSPNormRec(QString header);
    QString setENSLevelRec(QString header, Level* level);
    QString setENSGammaRec(QString header, Transition* transitionFrom, Transition* transitionTo);
    QString  setENSBetaRec(QString header, Transition* transition);
    QString setENSPartRec(QString header, Transition* transitionFrom , Transition* transitionTo);
    QStringList getValueAndError(double value, double uncertainty);
private:
	
  std::string path_;
  std::vector<std::string> fullFileNames_;// with full directory
  std::vector<std::string> shortFileNames_; //without directory

  bool delayedNeutrons_;

  double averageBetaEnergy_ ;
  double d_averageBetaEnergy_ = 0.;
  double averageGammaEnergy_ = 0.;
  double d_averageGammaEnergy_ = 0.;
  double averageNeutronEnergy_ = 0.;
  double d_averageNeutronEnergy_ = 0.;
  double neutronPercentage_ = 0.;
  double growingIntensity_ = 0.;
  int numberOfGammaAddedLevels_ = 0;
  int numberOfGammaDatabaseLevels_ = 0;
  int numberOfNeutronAddedLevels_ = 0;
  int numberOfNeutronDatabaseLevels_ = 0;
  int numberOfUniqueAddedGammas_ = 0;
  int numberOfUniqueDatabaseGammas_ = 0;
  double gammaAverageMultiplicity_ = 0.;
  double gammaAverageMultiplicityNotBetaWeighted_ = 0.;

};

#endif
