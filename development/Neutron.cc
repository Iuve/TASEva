
#include "DeclareHeaders.hh"

#include <iostream>


Neutron::Neutron(std::string particleType, double transitionQValue, double intensity,
 double finalLevelEnergy, int finalLevelAtomicMass, int finalLevelAtomicNumber):
Transition(particleType, transitionQValue, intensity, finalLevelEnergy,
 finalLevelAtomicMass, finalLevelAtomicNumber)
{
 
}


Neutron::~Neutron()
{

}
