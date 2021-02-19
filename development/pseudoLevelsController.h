#ifndef PSEUDOLEVELSCONTROLLER_H
#define PSEUDOLEVELSCONTROLLER_H
//#include "levelDensity.h"

using namespace std;
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
    PseudoLevelsController(DecayPath* decay);
    virtual ~PseudoLevelsController(){std::cout<<"delete pseudoLevelsController"<< std::endl;}//empty destructor so far
    void addPseudoLevels(double stepEnergy, double minEn, double maxEn, double totInt,  string gammaIntensityMethod);
//EVa    void applyStatisticalModel();

    /** Add pseudolevels with no information about spin.
      * Pseudolevels decay to all levels, gamma transitions intensites
      * is calculated anly form energy (as E2 transition)
      **/
    void addSimplePseudoLevels();
    void addRemainingTransition(string method);
//EVa    void addStatPseudoLevels();
//Eva    double findTransitionIntensity
//            ( Level* finalLevel, double atomicMass, double energy, SpinAndParity spin);
//EVa    double findTotalTransitionIntensity
 //           (Level* finalLevel, double atomicMass, double energy, std::vector<SpinAndParity> allSpins);
 //Eva   Level* createPseudoLevel(Level* parentLevel, double energy);
 //Eva   void setSpinAndParity(Level* parentLevel, Level* pseudoLevel);
    std::vector<string> getIntensityMethodList();
    void setIntensityMethod(string method){intensityMethod_ =method;}
    string getIntensityMethod(){return intensityMethod_;}
    void setNuclideIndex(int tabIndex){currentNuclideIndex_ = tabIndex;}


private:
    double getE2Intensity(double atomicMass, double energy);
    double getE1Intensity(double atomicMass, double energy);
    double getM2Intensity(double atomicMass, double energy);
    double getM1Intensity(double atomicMass, double energy);
    std::vector<SpinAndParity> findSpinAndParity(Level* parentLevel, double energy);
    void createIntensityMethodList();
    DecayPath* decayPath_;
    double minEnergy_;
    double maxEnergy_;
    double totIntensity_;
    bool ifStatisticalModel_;
    double deltaE_;
    std::vector<string> intensityMethodList_;
    string intensityMethod_;
    int currentNuclideIndex_;

    //Eva    LevelDensity* levelDensity_;
};

#endif // PSEUDOLEVELSCONTROLLER_H
