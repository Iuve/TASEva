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

    void addPseudoLevels(double stepEnergy, double minEn, double maxEn, double totInt,  std::string gammaIntensityMethod);
//EVa    void applyStatisticalModel();

    /** Add pseudolevels with no information about spin.
      * Pseudolevels decay to all levels, gamma transitions intensites
      * is calculated anly form energy (as E2 transition)
      **/
    void addSimplePseudoLevels();
    void addCustomPseudoLevels();
    void addRemainingTransition(std::string method);
//EVa    void addStatPseudoLevels();
//Eva    double findTransitionIntensity
//            ( Level* finalLevel, double atomicMass, double energy, SpinAndParity spin);
//EVa    double findTotalTransitionIntensity
 //           (Level* finalLevel, double atomicMass, double energy, std::vector<SpinAndParity> allSpins);
 //Eva   Level* createPseudoLevel(Level* parentLevel, double energy);
 //Eva   void setSpinAndParity(Level* parentLevel, Level* pseudoLevel);
    std::vector<std::string> getIntensityMethodList();
    std::vector<QString> getIntensityMethodListToolTip(){return intensityMethodListToolTip_;}
    void setIntensityMethod(std::string method){intensityMethod_ = method;}
    std::string getIntensityMethod(){return intensityMethod_;}
    void setNuclideIndex(int tabIndex){currentNuclideIndex_ = tabIndex;}
    void changeIntensitiesToChoosenMethod(Level*, std::string);
    void createIntensityMethodList();

private:
    double getE2Intensity(double atomicMass, double energy);
    double getE1Intensity(double atomicMass, double energy);
    double getM2Intensity(double atomicMass, double energy);
    double getM1Intensity(double atomicMass, double energy);
    std::vector<SpinAndParity> findSpinAndParity(Level* parentLevel, double energy);
    void applyModelM1();
    std::vector<double> CalculateModelIntensities(std::string, int, int, double);
    double FindPreciseEnergyLvl(double, Level*);

    //void createIntensityMethodList();
    DecayPath* decayPath_;
    double minEnergy_;
    double maxEnergy_;
    double totIntensity_;
    bool ifStatisticalModel_;
    double deltaE_;
    std::vector<std::string> intensityMethodList_;
    std::vector<QString> intensityMethodListToolTip_;
    std::string intensityMethod_;
    int currentNuclideIndex_;

    //Eva    LevelDensity* levelDensity_;
};

#endif // PSEUDOLEVELSCONTROLLER_H
