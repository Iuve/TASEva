
#ifndef NEW_GAMMA_H
#define	NEW_GAMMA_H

#include "G4DataVector.hh"
#include "G4AtomicTransitionManager.hh"
#include <string>


class Gamma : public Transition
{
public:
	Gamma(std::string type, double transitionQValue, double intensity,
	 double finalLevelEnergy, int finalLevelAtomicMass, int finalLevelAtomicNumber,
	 double electronConversionCoefficient=0., int atomicNumber=0);
	 
	~Gamma();
	
	double GetElectronConversionCoefficient(){return electronConversionCoefficient_;}
	void FindICEvent(std::vector<Event> &decay);
	void SetShellElectronConvCoef(std::string type, double value);
	std::vector<Event> FindGammaEvents();
    double* GetShellElectronConversionCoefficient(){return shellElectonConvCoeff_;}

	
	
private:

	static const int numberOfShellIndexes_ = 4;
	void InitializeShellNumbers();
	int FindPrimaryShellIndex();
	bool IsRadiativeTransitionReachableShell(int shellIndex);
	bool IsAugerReachableShell(int shellIndex);
	void AddXRaysEvent(std::vector<Event> &decay, int primaryVacancies);
	void AddAugerEvent(std::vector<Event> &decay, int primaryVacancies);
	int FindRandomIndex( const G4DataVector transProb);
	bool IsGammaDecay();
	double electronConversionCoefficient_;
	int atomicNumber_;
	
	
	double shellElectonConvCoeff_[numberOfShellIndexes_];
	int shellNumbers_[numberOfShellIndexes_]; 
	G4AtomicTransitionManager* atomicTransitionManager_;



};

#endif
