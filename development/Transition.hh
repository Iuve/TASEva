
#ifndef NEW_TRANSITION_H
#define	NEW_TRANSITION_H

#include "G4AtomicTransitionManager.hh" // It is needed but I have no idea why

#include <string>
#include <vector>


class Transition
{
public:
	Transition(std::string type, double transitionQValue, double intensity,
	 double finalLevelEnergy, int finalLevelAtomicMass, int finalLevelAtomicNumber,
	 double electronConversionCoefficient=0., int atomicNumber=0);
	
    //Pure Virtual Destructor - it makes Transition class Abstract, hence you cannot create object of that class
    //virtual ~Transition() = 0;

    //Virtual Destructor
    virtual ~Transition();
	
	std::string GetParticleType(){return particleType_;}
	double GetTransitionQValue(){return transitionQValue_;}
	double GetIntensity(){return intensity_;}
	double GetFinalLevelEnergy(){return finalLevelEnergy_;}
	int GetFinalLevelAtomicMass(){return finalLevelAtomicMass_;}
	int GetFinalLevelAtomicNumber(){return finalLevelAtomicNumber_;}
	
	void ChangeIntensity( double newIntensity) { intensity_ = newIntensity; }
	
	// intensities for calculations
	void SetRunningIntensity(double runningIntensity){runningIntensity_ = runningIntensity;}
	double GetRunningIntensity(){return runningIntensity_;}

    void SetPointerToFinalLevel(Level* pointerToFinalLevel){pointerToFinalLevel_ = pointerToFinalLevel;}
    Level* GetPointerToFinalLevel(){return pointerToFinalLevel_;}
    void SetPointerToInitialLevel(Level* pointerToInitialLevel){pointerToInitialLevel_ = pointerToInitialLevel;}
    Level* GetPointerToInitialLevel(){return pointerToInitialLevel_;}

    void SetIntensityFitFlag(bool parameter){freeIntensityFit_ = parameter;}
    bool GetIntensityFitFlag(){return freeIntensityFit_;}

    void SetAsAddedTransition(){isAddedTransition_ = true;}
    bool IsAddedTransition(){return isAddedTransition_;}

   // void SetTransitionRespFlag(bool flag){transitionRespFlag_ = flag;}
    //bool GetTransitionRespFlag(){return transitionRespFlag_;}

	// Virtual methods needed for B+, B- and G
	virtual std::vector<Event> FindBetaEvent() {
		std::cout << "I'm inside virtual FindBetaEvent, that should never happen! Something went wrong." <<std::endl;
		std::vector<Event> temp;
		return temp;
        }
	virtual std::vector<Event> FindGammaEvents() {
		std::cout << "I'm inside virtual FindGammaEvents, that should never happen! Something went wrong." <<std::endl;
		std::vector<Event> temp;
		return temp;
        }
	virtual double GetElectronConversionCoefficient() {
		std::cout << "I'm inside virtual GetElectronConversionCoefficient, that should never happen! Something went wrong." <<std::endl;
		return 0;
        }
    virtual double* GetShellElectronConversionCoefficient() {
        std::cout << "I'm inside virtual GetShellElectronConversionCoefficient, that should never happen! Something went wrong." <<std::endl;
        return 0;
        }
    virtual void CalculateAverageBetaEnergy() {
        std::cout << "I'm inside virtual GetShellElectronConversionCoefficient, that should never happen! Something went wrong." <<std::endl;
        return;
        }
    virtual double GetAverageBetaEnergy() {
        std::cout << "I'm inside virtual GetShellElectronConversionCoefficient, that should never happen! Something went wrong." <<std::endl;
        return 0;
        }

private:
	std::string particleType_;
	double transitionQValue_;
	double intensity_;
	double finalLevelEnergy_;
	int finalLevelAtomicMass_;
	int finalLevelAtomicNumber_;
	
    Level* pointerToFinalLevel_;
    Level* pointerToInitialLevel_;
	double runningIntensity_;
    bool freeIntensityFit_; // for fitting purposes
    bool isAddedTransition_; // = false if Transition in official, = true if added in analysis
	
	double electronConversionCoefficient_;
	int atomicNumber_;


};

#endif	/* DECAY_H */
