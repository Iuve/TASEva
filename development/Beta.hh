
#ifndef NEW_BETA_H
#define	NEW_BETA_H

#include "FermiDistribution.hh"

#include <string>


class Beta : public Transition
{
public:
	Beta(std::string particleType, double transitionQValue, double intensity,
	 double finalLevelEnergy, int finalLevelAtomicMass, int finalLevelAtomicNumber);
	 
	~Beta();
	
	std::vector<Event> FindBetaEvent();
    double GetAverageBetaEnergy(){return averageBetaEnergy_;}
    void SetAverageBetaEnergy(double avEnergy){averageBetaEnergy_ = avEnergy;}
	
	
private:

	//int eCharge_;
	FermiDistribution betaEnergyDistribution_;
    double averageBetaEnergy_;
	//std::string particleType_;
};

#endif
