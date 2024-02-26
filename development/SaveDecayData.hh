
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
    QString setENSParentRec(Nuclide* parent);
    QString setENSNormRec(QString header);
    QString setENSPNormRec(QString header);
    QString setENSLevelRec(QString header, Level* level);
    QString setENSGammaRec(QString header, Transition* transitionFrom, Transition* transitionTo);
    QString  setENSBetaRec(QString header, Transition* transition);
    QString setENSPartRec(QString header, Transition* transitionFrom , Transition* transitionTo);
private:
	
  std::string path_;
  std::vector<std::string> fullFileNames_;// with full directory
  std::vector<std::string> shortFileNames_; //without directory

  bool delayedNeutrons_;
};

#endif
