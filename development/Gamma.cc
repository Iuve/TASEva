
#include "DeclareHeaders.hh"

#include "G4AtomicShells.hh"
#include "Randomize.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"


Gamma::Gamma(std::string particleType, double transitionQValue, double intensity, double finalLevelEnergy, int finalLevelAtomicMass, 
int finalLevelAtomicNumber, double electronConversionCoefficient, int atomicNumber):
 Transition(particleType, transitionQValue, intensity, finalLevelEnergy,
 finalLevelAtomicMass, finalLevelAtomicNumber, electronConversionCoefficient,
 atomicNumber)
{
	atomicTransitionManager_ = G4AtomicTransitionManager::Instance();
	atomicTransitionManager_->Initialise();
	InitializeShellNumbers();
}

Gamma::~Gamma()
{

}

void Gamma::InitializeShellNumbers() 
{
	shellNumbers_[0] = 0;
	shellNumbers_[1] = 1;
	shellNumbers_[2] = 4;
	shellNumbers_[3] = 9;
}

void Gamma::SetShellElectronConvCoef(std::string type, double value)
{
		if(type == "KC" || type == "KC+" )
				for(int i=0; i<numberOfShellIndexes_; i++)
				shellElectonConvCoeff_[i] +=value;
		else if (type == "LC" || type == "LC+" )
				for(int i=1; i<numberOfShellIndexes_; i++)
				shellElectonConvCoeff_[i] +=value;
		else if (type == "MC" || type == "MC+" )
				for(int i=2; i<numberOfShellIndexes_; i++)
				shellElectonConvCoeff_[i] +=value;
		else
				shellElectonConvCoeff_[3] +=value;
}

std::vector<Event> Gamma::FindGammaEvents()
{
	std::vector<Event> gammaDecay;
	if(IsGammaDecay())
	{
		gammaDecay.push_back(Event(GetTransitionQValue(), G4ParticleTable::GetParticleTable()->FindParticle("gamma")));	
	}
	else //IC
	{
		FindICEvent(gammaDecay);
	}
	return gammaDecay;
}

bool Gamma::IsGammaDecay()
{
	double intensity = GetIntensity();
    double randomNumber = G4UniformRand() * intensity * (1+electronConversionCoefficient_);
    if(randomNumber > intensity * electronConversionCoefficient_ )
		return true;
    return false;	
}

void Gamma::FindICEvent(std::vector<Event> &eventList)
{
	int primaryShellIndex = FindPrimaryShellIndex();
	int primaryVacancies = shellNumbers_[primaryShellIndex];
	//std::cout << "primaryVacancies: " << primaryVacancies << endl;
	if(!IsRadiativeTransitionReachableShell(primaryVacancies))
		if(!IsAugerReachableShell(primaryVacancies))
		{
			//std::cout << "Not reachable!" << std::endl;
			return FindICEvent(eventList);
		}


	if(IsRadiativeTransitionReachableShell(primaryVacancies))
	{
		//std::cout << "Radiative Transition Reachable" << endl;
		if(IsAugerReachableShell(primaryVacancies))
		{
			//std::cout << "RTR: Auger Reachable" << endl;
			double totalRadTransitProb = atomicTransitionManager_->
				TotalRadiativeTransitionProbability(atomicNumber_, primaryVacancies);
			if(G4UniformRand()<totalRadTransitProb)
			{
				//std::cout << "RTR: Auger Reachable: Add X" << endl;
				AddXRaysEvent(eventList, primaryVacancies);
			}
			else
			{
				//std::cout << "RTR: Auger Reachable: Add Auger" << endl;
				AddAugerEvent(eventList, primaryVacancies);
			}	
		}
		else
		{
			//std::cout << "RTR: Auger NOT Reachable" << endl;
			AddXRaysEvent(eventList, primaryVacancies);
		}
	}
	else //if(IsAugerReachableShell(primaryVacancies))
	{
		//std::cout << "Radiative Transition NOT Reachable" << endl;
		AddAugerEvent(eventList, primaryVacancies);
	}
	
		double primaryBindingEnergy = G4AtomicShells::GetBindingEnergy(atomicNumber_, primaryVacancies) / keV;
		eventList.push_back(Event(GetTransitionQValue()-primaryBindingEnergy, G4ParticleTable::GetParticleTable()->FindParticle("e-")));
}

int Gamma::FindPrimaryShellIndex()
{	
	double randomNumber = G4UniformRand()* shellElectonConvCoeff_[3];
	if(randomNumber <= shellElectonConvCoeff_[0])//K - shell nr = 0
			return 0;
	for(int i=1; i < numberOfShellIndexes_; i++)
	{
		if(randomNumber > shellElectonConvCoeff_[i-1] && randomNumber <= shellElectonConvCoeff_[i])
			return i;
	}
	return 0;		
}

bool Gamma::IsRadiativeTransitionReachableShell(int shellIndex)
{
	int numberOfRadiativeTransReachableShells = 
		atomicTransitionManager_->NumberOfReachableShells(atomicNumber_);
//	std::cout << "numberOfRadiativeTransReachableShells: " << numberOfRadiativeTransReachableShells << " shell index: " << shellIndex << endl;
	if(shellIndex >= numberOfRadiativeTransReachableShells)
		return false;
	else
		return true;
}

bool Gamma::IsAugerReachableShell(int shellIndex)
{
	int numberOfAugerReachableShells = 
		atomicTransitionManager_->NumberOfReachableAugerShells(atomicNumber_);
	if(shellIndex >= numberOfAugerReachableShells)
		return false;
	else
		return true;
}

void Gamma::AddXRaysEvent(std::vector<Event> &decay, int primaryVacancies)
{
	const G4FluoTransition* fluoTrans = atomicTransitionManager_->
		ReachableShell(atomicNumber_, primaryVacancies);
	const G4DataVector transEnergies = fluoTrans -> TransitionEnergies();
	const G4DataVector transProb = fluoTrans -> TransitionProbabilities();
	int xRayIndex = FindRandomIndex(transProb);
	decay.push_back(Event(transEnergies.at(xRayIndex)/keV, G4ParticleTable::GetParticleTable()->FindParticle("gamma")));
}

void Gamma::AddAugerEvent(std::vector<Event> &decay, int primaryVacancies)
{
	const G4AugerTransition * augerTrans = atomicTransitionManager_->
		ReachableAugerShell(atomicNumber_, primaryVacancies);

	//shell from where electron can fill the vacancy
	const std::vector<G4int> augerPossibilities = *(augerTrans->TransitionOriginatingShellIds());
	G4DataVector totAugerTransFromShell;
	for(unsigned int i=0; i<augerPossibilities.size(); i++)
	{
		const G4DataVector augerTransProb =
			 *(augerTrans -> AugerTransitionProbabilities(augerPossibilities.at(i)));
		double sumProbAugerTrans(0.);
		for(unsigned int j=0; j<augerTransProb.size(); j++)
			sumProbAugerTrans += augerTransProb.at(j);
		totAugerTransFromShell.push_back(sumProbAugerTrans);
	}
	int index = FindRandomIndex(totAugerTransFromShell);

	//electron from augerPossibilities.at(index) will fill the vacancy
	int electronShellId = augerPossibilities.at(index);
	const G4DataVector augerTransProb = *(augerTrans -> AugerTransitionProbabilities(electronShellId));
	const G4DataVector augerTransEnergy =  *(augerTrans -> AugerTransitionEnergies(electronShellId));
	int augerIndex = FindRandomIndex(augerTransProb);
	decay.push_back(Event(augerTransEnergy.at(augerIndex)/keV, G4ParticleTable::GetParticleTable()->FindParticle("e-")));
}

int Gamma::FindRandomIndex( const G4DataVector transProb)
{
	double sumProb (0.);
	G4DataVector transProbSum;
	for (unsigned int i=0; i<transProb.size(); i++)
	{
		sumProb += transProb.at(i);
		transProbSum.push_back(sumProb);
	}
		
	double randomNumber = G4UniformRand()*sumProb;
	int index(-1);
	for (unsigned int i=0; i<transProb.size(); i++)
	{
		if(randomNumber < transProbSum.at(i))
		{
			index = i;
			break;
		}
	}
	return index;
}


