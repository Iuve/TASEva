
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
        //betaEnergyDistribution_ = FermiDistribution(GetFinalLevelAtomicNumber() - 1, GetTransitionQValue(), -1);
        //SetAverageBetaEnergy(betaEnergyDistribution_.GetAverageBetaEnergy());
	}
	else if(GetParticleType() == "B+")
	{
		//beta plus
        //betaEnergyDistribution_ = FermiDistribution(GetFinalLevelAtomicNumber() + 1, GetTransitionQValue(), +1);
        //SetAverageBetaEnergy(betaEnergyDistribution_.GetAverageBetaEnergy());
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


void Beta::CalculateAverageBetaEnergy()
{
    CheckBetaTransitionType();

    if(GetParticleType() == "B-")
    {
        //beta minus
        betaEnergyDistribution_ = FermiDistribution(GetFinalLevelAtomicNumber() - 1, GetTransitionQValue(), -1, betaTransitionType_);
        SetAverageBetaEnergy(betaEnergyDistribution_.GetAverageBetaEnergy());
    }
    else if(GetParticleType() == "B+")
    {
        //beta plus
        betaEnergyDistribution_ = FermiDistribution(GetFinalLevelAtomicNumber() + 1, GetTransitionQValue(), +1, betaTransitionType_);
        SetAverageBetaEnergy(betaEnergyDistribution_.GetAverageBetaEnergy());
    }
    else
        std::cout << "Wrong particle type in Beta class." << std::endl;

}

void Beta::CheckBetaTransitionType()
{
    //std::cout << "CheckBetaTransitionType() poczatek" << std::endl;

    double initialLevelSpin = this->GetPointerToInitialLevel()->GetSpin();
    double finalLevelSpin = this->GetPointerToFinalLevel()->GetSpin();
    std::string initialLevelParity = this->GetPointerToInitialLevel()->GetParity();
    std::string finalLevelParity = this->GetPointerToFinalLevel()->GetParity();

//	betaTransitionType = 0: allowed Fermi and GT
//  GT : Gamow-Teller Decay
//  betaTransitionType = 1: 0-: pe^2  + Eν^2 + 2β^2 * Eν * Ee
//  betaTransitionType = 2: 1-: pe^2  + Eν^2 - 4/3 * β^2 * Eν * Ee
//  betaTransitionType = 3: 2-: pe^2  + Eν^2
//	F : Fermi Decay
//  betaTransitionType = 4: 1-: pe^2  + Eν^2 + 2/3 β^2 * Eν * Ee

    //std::cout << "initialLevelParity = " << initialLevelParity << std::endl;
    //std::cout << "finalLevelParity = " << finalLevelParity << std::endl;

    if( !(((initialLevelParity == "+") || (initialLevelParity == "-")) &&
    ((finalLevelParity == "+") || (finalLevelParity == "-"))) )
    {
        //std::cout << "no correct parity" << std::endl;
        betaTransitionType_ = 0;
        return;
    }

    if(finalLevelParity == initialLevelParity)
        betaTransitionType_ = 0;
    else
    {
        double spinDifference = abs(initialLevelSpin - finalLevelSpin);
        //std::cout << "spinDifference = " << spinDifference << std::endl;
        if( (spinDifference >= -0.01) && (spinDifference <= 0.01) )
            betaTransitionType_ = 1;
        else if( (spinDifference >= 0.99) && (spinDifference <= 1.01) )
            betaTransitionType_ = 2;
        else if( (spinDifference >= 1.99) && (spinDifference <= 2.01) )
            betaTransitionType_ = 3;
        else
            betaTransitionType_ = 0;
    }
}
