#ifndef PSEUDOLEVELSCONTROLLER_H
#define PSEUDOLEVELSCONTROLLER_H
//#include "levelDensity.h"

#include <vector>
#include <string>
#include <QString>

class DecayPath;

class SpinAndParity
{
    public:
    SpinAndParity(int numerator, int denominator, char parity, double intensity = 1)
    {
        numerator_ = numerator;
        denominator_ = denominator;
        parity_ = parity;
        intensity_ = intensity;

    }
    int numerator_;
    int denominator_;
    char parity_;
    double intensity_;
};

class PseudoLevelsController
{
public:
    PseudoLevelsController();
    ~PseudoLevelsController();

    void addPseudoLevels(double stepEnergy, double minEn, double maxEn, double totInt,
                         std::string gammaIntensityMethod, double finalNeutronE);
    void addPseudoLevels(double stepEnergy, double minEn, double maxEn, double totInt,
                         std::string pathMethod);
//EVa    void applyStatisticalModel();

    /** Add pseudolevels with no information about spin.
      * Pseudolevels decay to all levels, gamma transitions intensites
      * is calculated anly form energy (as E2 transition)
      **/
    void addSimplePseudoLevels();
    void addCustomPseudoLevels();
    void addRemainingGammaTransitions();
    void addNeutronLevels();
    void addRemainingNeutronTransition(double LevelEnergy);
//    void addRemainingNeutronTransitions();
    //EVa    void addStatPseudoLevels();
//Eva    double findTransitionIntensity
//            ( Level* finalLevel, double atomicMass, double energy, SpinAndParity spin);
//EVa    double findTotalTransitionIntensity
 //           (Level* finalLevel, double atomicMass, double energy, std::vector<SpinAndParity> allSpins);
 //Eva   Level* createPseudoLevel(Level* parentLevel, double energy);
 //Eva   void setSpinAndParity(Level* parentLevel, Level* pseudoLevel);
    std::vector<QString> getGammaIntensityMethodList(){ return intensityGammaMethodList_;}
    std::vector<QString> getGammaIntensityMethodListToolTip(){return intensityGammaMethodListToolTip_;}
    std::vector<QString> getGammaPathMethodList(){return pathGammaMethodList_;};
    std::vector<QString> getGammaPathMethodListToolTip(){return pathGammaMethodListToolTip_;}
    std::vector<QString> getParticleIntensityMethodList(){  return intensityParticleMethodList_;}
    std::vector<QString> getParticleIntensityMethodListToolTip(){return intensityParticleMethodListToolTip_;}
    std::vector<QString> getParticlePathMethodList(){return pathParticleMethodList_;};
    std::vector<QString> getParticlePathMethodListToolTip(){return pathParticleMethodListToolTip_;}
    std::vector<QString> getMainPathMethodList(){return pathMainMethodList_;};
    std::vector<QString> getMainPathMethodListToolTip(){return pathMainMethodListToolTip_;}

    void setGammaIntensityMethod(std::string method){intensityGammaMethod_ = QString::fromStdString(method);}
    void setGammaIntensityMethod(QString qmethod){intensityGammaMethod_ = qmethod;}
    void setParticleIntensityMethod(std::string method){intensityParticleMethod_ = QString::fromStdString(method);}
    void setParticleIntensityMethod(QString qmethod){intensityParticleMethod_ = qmethod;}
    void setPathMainMethod(std::string method){pathMainMethod_ = QString::fromStdString(method);}
    void setPathMainMethod(QString qmethod){pathMainMethod_ = qmethod;}
    void setParticlePathMethod(QString qmethod){pathParticleMethod_ = qmethod;}
    void setGammaPathMethod(QString qmethod){pathGammaMethod_ = qmethod;}

    std::string getGammaIntensityMethod(){return intensityGammaMethod_.toStdString();}
    std::string getParticleIntensityMethod(){return intensityParticleMethod_.toStdString();}
    std::string getPathMainMethod() {return pathMainMethod_.toStdString();}
    std::string getParticlePathMethod(){return pathParticleMethod_.toStdString();}
    std::string getGammaPathMethod(){return pathGammaMethod_.toStdString();}
    void setNuclideIndex(int tabIndex){currentNuclideIndex_ = tabIndex;}
    void changeIntensitiesToChoosenMethod(Level*, std::string);
    void createGammaIntensityMethodList();
    void createGammaPathMethodList();
    void createParticleIntensityMethodList();
    void createParticlePathMethodList();
    void createMainDeExcitationPathPathMethodList();


private:
    double calculateIntensity(std::string method, double transitionEnergy, int atomicMass);

    double getE2Intensity(double atomicMass, double energy);
    double getE1Intensity(double atomicMass, double energy);
    double getM2Intensity(double atomicMass, double energy);
    double getM1Intensity(double atomicMass, double energy);
    double getEnPowerIntensity(double energy, double power);
    std::vector<SpinAndParity> findSpinAndParity(Level* parentLevel, double energy);
    void applyModelM1();
    std::vector<double> CalculateModelIntensities(std::string, int, int, double);
    double FindPreciseEnergyLvl(double, Level*);

    //void createIntensityMethodList();
    DecayPath* decayPath_;
    double minEnergy_;
    double maxEnergy_;
    double totIntensity_;
    double finalNeutronE_;
    double Sn_;
    bool ifStatisticalModel_;
    double deltaE_;
    int currentNuclideIndex_;
    std::vector<QString> intensityGammaMethodList_;
    std::vector<QString> intensityGammaMethodListToolTip_;
    std::vector<QString> pathGammaMethodList_;
    std::vector<QString> pathGammaMethodListToolTip_;
    QString intensityGammaMethod_;
    std::vector<QString> intensityParticleMethodList_;
    std::vector<QString> intensityParticleMethodListToolTip_;
    QString intensityParticleMethod_;
    std::vector<QString> pathParticleMethodList_;
    std::vector<QString> pathParticleMethodListToolTip_;
    std::vector<QString> pathMainMethodList_;
    std::vector<QString> pathMainMethodListToolTip_;
    QString pathMainMethod_;
    QString pathGammaMethod_;
    QString pathParticleMethod_;



    //Eva    LevelDensity* levelDensity_;
};

#endif // PSEUDOLEVELSCONTROLLER_H
