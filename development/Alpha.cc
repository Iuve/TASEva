
#include "DeclareHeaders.hh"

#include <iostream>


Alpha::Alpha(std::string particleType, double transitionQValue, double intensity,
 double finalLevelEnergy, int finalLevelAtomicMass, int finalLevelAtomicNumber):
Transition(particleType, transitionQValue, intensity, finalLevelEnergy,
 finalLevelAtomicMass, finalLevelAtomicNumber)
{
 
}


Alpha::~Alpha()
{

}

