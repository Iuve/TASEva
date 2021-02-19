
#include "DeclareHeaders.hh"

#include <vector>
#include "Randomize.hh"

Level::Level(double levelEnergy, double spin, std::string parity, double halfLifeTimeInSeconds,
 std::vector<Gamma> gammasFromLvL, std::vector<Beta> betasFromLvL,
 std::vector<Neutron> neutronsFromLvL, std::vector<Alpha> alphasFromLvL):
levelEnergy_(levelEnergy), spin_(spin), parity_(parity), halfLifeTimeInSeconds_(halfLifeTimeInSeconds),
 isPseudoLevel_(false), neutronLevel_(false), gammasFromLvL_(gammasFromLvL), betasFromLvL_(betasFromLvL),
 neutronsFromLvL_(neutronsFromLvL), alphasFromLvL_(alphasFromLvL)
{

}

Level::Level(double levelEnergy, double spin, std::string parity, double halfLifeTimeInSeconds):
 levelEnergy_(levelEnergy), spin_(spin), parity_(parity), halfLifeTimeInSeconds_(halfLifeTimeInSeconds)
{

}

Level::~Level()
{

}

void Level::CalculateTotalProbability()
{
    totalIntensity_ = 0.0;
	for ( auto it = transitions_.begin(); it != transitions_.end(); ++it  )
	{
		totalIntensity_ += (*it)->GetIntensity();
        (*it)->SetRunningIntensity(totalIntensity_);
	}
    std::cout << "Total Probability: " << totalIntensity_ << std::endl;
    std::cout << "Level energy: " << levelEnergy_ << std::endl;

}

void Level::RemoveTransition(std::string type, double energy)
{
    int transitionsSize = transitions_.size();
    std::cout << "transitionsSize: " << transitionsSize << std::endl;
    if(transitionsSize <= 1)
    {
        Nuclide* initialLevelNuclide;
        DecayPath* decayPath = DecayPath::get();
        std::vector<Nuclide>* nuclidesVector = decayPath->GetAllNuclides();

        for ( auto it = nuclidesVector->begin(); it != nuclidesVector->end(); ++it )
        {
            for ( auto jt = it->GetNuclideLevels()->begin(); jt != it->GetNuclideLevels()->end(); ++jt )
            {
                if( &(*jt) == this )
                    initialLevelNuclide = &(*it);
            }
        }

        initialLevelNuclide->RemoveLevel( levelEnergy_ ); //this? energy?

    }
    else
    {
        int otherSize = betasFromLvL_.size() + gammasFromLvL_.size() +
                neutronsFromLvL_.size() + alphasFromLvL_.size();
        //double initialLevelEnergy;


        for ( auto it = transitions_.begin(); it != transitions_.end(); ++it )
        {
            if((*it)->GetParticleType() == type)
            {
                double tempE = (*it)->GetTransitionQValue();
                if( fabs(energy - tempE) <= 1. )
                {
                    std::cout << "(*it)->GetParticleType(): " << (*it)->GetParticleType() << std::endl;
                    std::cout << "type: " << type << std::endl;

                    if(type == "B-" || type == "B+")
                    {
                        for( auto jt = betasFromLvL_.begin(); jt != betasFromLvL_.end(); ++jt )
                        {
                            if( &(*jt) == *it)
                            {
                                std::cout << "Removing Transition with type: " << (*it)->GetParticleType();
                                std::cout << " and energy: " << (*it)->GetTransitionQValue() << std::endl;
                                betasFromLvL_.erase(jt);
                                break;
                            }
                        }
                    }
                    if(type == "G")
                    {
                        for( auto jt = gammasFromLvL_.begin(); jt != gammasFromLvL_.end(); ++jt )
                        {
                            if( &(*jt) == *it)
                            {
                                std::cout << "Removing Transition with type: " << (*it)->GetParticleType();
                                std::cout << " and energy: " << (*it)->GetTransitionQValue() << std::endl;
                                gammasFromLvL_.erase(jt);
                                break;
                            }
                        }
                    }
                    if(type == "N")
                    {
                        for( auto jt = neutronsFromLvL_.begin(); jt != neutronsFromLvL_.end(); ++jt )
                        {
                            if( &(*jt) == *it)
                            {
                             std::cout << "Removing Transition with type: " << (*it)->GetParticleType();
                             std::cout << " and energy: " << (*it)->GetTransitionQValue() << std::endl;
                                neutronsFromLvL_.erase(jt);
                                break;
                            }
                        }
                    }
                    if(type == "A")
                    {
                        for( auto jt = alphasFromLvL_.begin(); jt != alphasFromLvL_.end(); ++jt )
                        {
                            if( &(*jt) == *it)
                            {
                             std::cout << "Removing Transition with type: " << (*it)->GetParticleType();
                             std::cout << " and energy: " << (*it)->GetTransitionQValue() << std::endl;
                                alphasFromLvL_.erase(jt);
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }

    UpdateTransitionVector();
    NormalizeTransitionIntensities();

    int newTransitionsSize = transitions_.size();
    int newOtherSize = betasFromLvL_.size() + gammasFromLvL_.size() +
            neutronsFromLvL_.size() + alphasFromLvL_.size();
    if(newTransitionsSize + 1 != transitionsSize)
        std::cout << "Warning: transitions_ size was not changed or changed incorrectly." << std::endl;
    if(newOtherSize + 1 != otherSize)
        std::cout << "Warning: otherTransitions_ size was not changed or changed incorrectly." << std::endl;

    }

}

void Level::UpdateTransitionVector()
{
    transitions_.clear();
    if( gammasFromLvL_.size() )
        for( auto it = gammasFromLvL_.begin(); it != gammasFromLvL_.end(); ++it )
            transitions_.push_back( &(*it) );
    if( betasFromLvL_.size() )
        for( auto it = betasFromLvL_.begin(); it != betasFromLvL_.end(); ++it )
            transitions_.push_back( &(*it) );
    if( neutronsFromLvL_.size() )
        for( auto it = neutronsFromLvL_.begin(); it != neutronsFromLvL_.end(); ++it )
            transitions_.push_back( &(*it) );
    if( alphasFromLvL_.size() )
        for( auto it = alphasFromLvL_.begin(); it != alphasFromLvL_.end(); ++it )
            transitions_.push_back( &(*it) );
}

void Level::AddTransition(std::string type, double transitionQValue, double intensity)
{
    double finalLevelEnergy;
    int finalLevelAtomicMass;
    int finalLevelAtomicNumber;
    int atomicMass, atomicNumber;
    double qBeta;

    DecayPath* decayPath = DecayPath::get();
    std::vector<Nuclide>* nuclidesVector = decayPath->GetAllNuclides();

    for ( auto it = nuclidesVector->begin(); it != nuclidesVector->end(); ++it )
    {
        for ( auto jt = it->GetNuclideLevels()->begin(); jt != it->GetNuclideLevels()->end(); ++jt )
        {
            if( &(*jt) == this )
            {
                atomicMass = it->GetAtomicMass();
                atomicNumber = it->GetAtomicNumber();
                qBeta = it->GetQBeta();
                //nSeparationEnergy = it;
            }
        }
    }

    if(type == "B-")
    {
        finalLevelAtomicMass = atomicMass;
        finalLevelAtomicNumber = atomicNumber + 1;
        if (transitionQValue + levelEnergy_ > qBeta)
            finalLevelEnergy = 0.;
        else
            finalLevelEnergy = qBeta - transitionQValue;

        betasFromLvL_.push_back(Beta(type, transitionQValue, intensity, finalLevelEnergy,
           finalLevelAtomicMass, finalLevelAtomicNumber));
        transitions_.push_back(&betasFromLvL_.back());
    }
    else if(type == "B+")
    {
        finalLevelAtomicMass = atomicMass;
        finalLevelAtomicNumber = atomicNumber - 1;
        if (transitionQValue + levelEnergy_ > qBeta)
            finalLevelEnergy = 0.;
        else
            finalLevelEnergy = qBeta - levelEnergy_ - transitionQValue;

        betasFromLvL_.push_back(Beta(type, transitionQValue, intensity, finalLevelEnergy,
           finalLevelAtomicMass, finalLevelAtomicNumber));
        transitions_.push_back(&betasFromLvL_.back());
    }
    else if(type == "G")
    {
        finalLevelAtomicMass = atomicMass;
        finalLevelAtomicNumber = atomicNumber;
        if(transitionQValue > levelEnergy_)
            finalLevelEnergy = 0.;
        else
            finalLevelEnergy = levelEnergy_ - transitionQValue;

        gammasFromLvL_.push_back(Gamma(type, transitionQValue, intensity, finalLevelEnergy,
            finalLevelAtomicMass, finalLevelAtomicNumber));
        transitions_.push_back(&gammasFromLvL_.back());
    }
    else if(type == "N")
    {
        finalLevelAtomicMass = atomicMass - 1;
        finalLevelAtomicNumber = atomicNumber;
        //not ready! Sn needed
        //finalLevelEnergy = levelEnergy_ - Sn - transitionQValue;
        finalLevelEnergy = 0.;

        neutronsFromLvL_.push_back(Neutron(type, transitionQValue, intensity,
            finalLevelEnergy, finalLevelAtomicMass, finalLevelAtomicNumber));
        transitions_.push_back(&neutronsFromLvL_.back());
    }
    else if(type == "A")
    {
        finalLevelAtomicMass = atomicMass - 4;
        finalLevelAtomicNumber = atomicNumber - 2;
        //not ready! recoil energy
        finalLevelEnergy = 0.;

        alphasFromLvL_.push_back(Alpha(type, transitionQValue, intensity,
            finalLevelEnergy, finalLevelAtomicMass, finalLevelAtomicNumber));
        transitions_.push_back(&alphasFromLvL_.back());
    }
    else
    {
        finalLevelAtomicMass = 0;
        finalLevelAtomicNumber = 0;
        finalLevelEnergy = 0.;

        std::cout << "Level::AddTransition: Other transition type choosen: " << type << std::endl;
        otherTransitions_.push_back(Transition(type, transitionQValue, intensity,
            finalLevelEnergy, finalLevelAtomicMass, finalLevelAtomicNumber));
        transitions_.push_back(&otherTransitions_.back());
        transitions_.back()->SetAsAddedTransition();
        return;
    }

    transitions_.back()->SetAsAddedTransition();
    //Recalculating all pointers is NOT needed here!
    //2020-10-15 Well it looks like it IS needed. decayPath->SetPointerToLevel doesn't work.
    decayPath->RecalculatePointers();
    //decayPath->SetPointerToLevel(transitions_.back(), finalLevelAtomicNumber, finalLevelAtomicMass, finalLevelEnergy);
}

void Level::NormalizeTransitionIntensities()
{
    totalIntensity_ = 0.;
    for( auto it = transitions_.begin(); it != transitions_.end(); ++it )
    {
        totalIntensity_ += (*it)->GetIntensity();
    }
    for( auto it = transitions_.begin(); it != transitions_.end(); ++it )
    {
        double tempIntensity = (*it)->GetIntensity();
        (*it)->ChangeIntensity( tempIntensity / totalIntensity_ );
    }
}

Nuclide* Level::GetNuclideAddress()
{
    DecayPath* decayPath = DecayPath::get();

        for(auto itn=decayPath->GetAllNuclides()->begin(); itn != decayPath->GetAllNuclides()->end(); ++itn)
        {
            for(auto itl=(itn)->GetNuclideLevels()->begin(); itl !=(itn)->GetNuclideLevels()->end(); ++itl)
            {
                if( &(*itl) == this )
                {
                    return &(*itn);
                }
            }
        }
    std::cout << "GetNuclideAddress failed, returning 0L as pointer." << std::endl;
    //std::cout << "levelEnergy_: " << levelEnergy_ << std::endl;
    return 0L;
}















