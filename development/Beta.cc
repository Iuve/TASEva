
#include "DeclareHeaders.hh"

#include "G4ParticleTable.hh"

#include <vector>
#include <iostream>


Beta::Beta(std::string particleType, double transitionQValue, double intensity,
 double finalLevelEnergy, int finalLevelAtomicMass, int finalLevelAtomicNumber):
Transition(particleType, transitionQValue, intensity, finalLevelEnergy,
 finalLevelAtomicMass, finalLevelAtomicNumber)
{
	if(GetParticleType() == "B-")
	{
		//beta minus
//Eva		betaEnergyDistribution_ = FermiDistribution(GetFinalLevelAtomicNumber() - 1, GetTransitionQValue(), -1);
	}
	else if(GetParticleType() == "B+")
	{
		//beta plus
//Eva		betaEnergyDistribution_ = FermiDistribution(GetFinalLevelAtomicNumber() + 1, GetTransitionQValue(), +1);
	}
	else
		std::cout << "Wrong particle type in Beta class." << std::endl; 
}


Beta::~Beta()
{

}


std::vector<Event> Beta::FindBetaEvent()
{
	std::vector<Event> betaEvents;
	G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
	double randomBetaEnergy = betaEnergyDistribution_.GetRandomBetaEnergy();

	if(GetParticleType() == "B-")
	{
		//beta minus
		betaEvents.push_back( Event(randomBetaEnergy, particleTable->FindParticle("e-")) );
	}
	else if(GetParticleType() == "B+")
	{
		//beta plus
		betaEvents.push_back( Event(randomBetaEnergy, particleTable->FindParticle("e+")) );	
	}
	else
		std::cout << "Wrong particle type in Beta class." << std::endl;
		
	return betaEvents;
}
