
#ifndef NEW_NEUTRON_H
#define	NEW_NEUTRON_H

#include <string>


class Neutron : public Transition
{
public:
	Neutron(std::string particleType, double transitionQValue, double intensity,
	 double finalLevelEnergy, int finalLevelAtomicMass, int finalLevelAtomicNumber);
	 
	~Neutron();
	
	
private:


};

#endif
