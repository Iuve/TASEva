
#include "DeclareHeaders.hh"

#include <vector>




Nuclide::Nuclide(int atomicNumber, int atomicMass, double qBeta, std::vector<Level> nuclideLevels):
atomicNumber_(atomicNumber), atomicMass_(atomicMass), nuclideLevels_(nuclideLevels), qBeta_(qBeta)
{
	
}

Nuclide::~Nuclide()
{
	
}

void Nuclide::RemoveLevel(double levelEnergy)
{
    //int levelsSize = nuclideLevels_.size();
    for( auto it = nuclideLevels_.begin(); it != nuclideLevels_.end(); ++it)
    {
        if(fabs(levelEnergy - it->GetLevelEnergy()) <= 0.1)
        {
            //Level* pointerToRemovedLevel = &(*it);
            //nuclideLevels_.erase(it);
            levelsToDelete_.emplace_back( this, it->GetLevelEnergy() );
            CheckTransitionsToLevel( &(*it) );
            break;
        }
    }
    DeleteLevels();
    //int newLevelSize = nuclideLevels_.size();
    //if(newLevelSize - 1 != levelsSize)
    //    std::cout << "Warning: nuclideLevels_ size was not changed or changed incorrectly." << std::endl;
}

void Nuclide::CheckTransitionsToLevel(Level* level)
{
    DecayPath* decayPath = DecayPath::get();
    std::vector<Nuclide>* nuclidesVector = decayPath->GetAllNuclides();

    for ( auto it = nuclidesVector->begin(); it != nuclidesVector->end(); ++it )
    {
        for ( auto jt = it->GetNuclideLevels()->begin(); jt != it->GetNuclideLevels()->end(); ++jt )
        {
            for ( auto kt = jt->GetTransitions()->begin(); kt != jt->GetTransitions()->end(); ++kt )
            {
                if( (*kt)->GetPointerToFinalLevel() == level )
                {
                    //double transitionEnergy = (*kt)->GetTransitionQValue();
                    //std::string transitionType = (*kt)->GetParticleType();
                    //Level* initialLevel = (*kt)->GetPointerToInitialLevel();
                    int nrOfTransitionsFromInitialLevel = (*jt).GetTransitions()->size();
                    std::cout << "nrOfTransitionsFromInitialLevel: " << nrOfTransitionsFromInitialLevel  << std::endl;
                    if(nrOfTransitionsFromInitialLevel <= 1)
                    {
                        levelsToDelete_.emplace_back( &(*it), jt->GetLevelEnergy() );
                        CheckTransitionsToLevel(&(*jt));
                    }
                    else
                    {
                        jt->RemoveTransition( (*kt)->GetParticleType(), (*kt)->GetTransitionQValue() );
                    }
                    break;
                }
            }
        }
    }
}

void Nuclide::DeleteLevels()
{
    DecayPath* decayPath = DecayPath::get();
    std::vector<Nuclide>* nuclidesVector = decayPath->GetAllNuclides();

    placeToGo:
    std::cout << "Inside placeToGo, levelsToDelete_.size(): " << levelsToDelete_.size() << std::endl;

    if( levelsToDelete_.size() == 0)
    {
        decayPath->RecalculatePointers();
        return;
    }

    for ( auto it = levelsToDelete_.begin(); it != levelsToDelete_.end(); ++it)
    {
        for ( auto jt = nuclidesVector->begin(); jt != nuclidesVector->end(); ++jt )
        {;
            if( &(*jt) == (*it).first )
            {
                for ( auto kt = jt->GetNuclideLevels()->begin(); kt != jt->GetNuclideLevels()->end(); ++kt )
                {
                    if(fabs( (*it).second - kt->GetLevelEnergy() ) <= 1.)
                    {

                        std::cout << "Removing Level with energy: " << kt->GetLevelEnergy() << std::endl;
                        std::cout << "Before lvl removal (*jt).nuclideLevels_: " << (*jt).nuclideLevels_.size() << std::endl;
                        (*jt).nuclideLevels_.erase(kt);
                        std::cout << "After lvl removal (*jt).nuclideLevels_: " << (*jt).nuclideLevels_.size() << std::endl;
                        levelsToDelete_.erase(it);
                        goto placeToGo;
                    }
                }
            }
        }
    }

    std::cout << "End of DeleteLevels function. levelsToDelete_ was not cleared, its size: " << levelsToDelete_.size() << std::endl;
    levelsToDelete_.clear();
    std::cout << "levelsToDelete_ is cleared now. Its size: " << levelsToDelete_.size() << std::endl;
}

void Nuclide::AddLevelEI(double energy, double intensity)
{
    double spin =-1;
    std::string parity = "";
    double T12 = 1e-15;
    AddLevel(energy,spin,parity,T12,intensity);

}

void Nuclide::AddLevel(double energy, double spin, std::string parity, double T12, double intensity,
                       double neutronFinalLvlEnergy, double Sn)
{
    nuclideLevels_.push_back( Level(energy, spin, parity, T12) );
    nuclideLevels_.back().setAsPseudoLevel();
    //add default gamma (or neutron) transition to ground state
    if(Sn > 0)
    {
        double neutronKineticEnergy = energy - neutronFinalLvlEnergy - Sn;
        nuclideLevels_.back().AddTransition("N", neutronKineticEnergy, 1.,
                                            neutronFinalLvlEnergy);
    }
    else
        nuclideLevels_.back().AddTransition("G", energy, 1.);
    //add default beta minus transition to created level
    DecayPath* decayPath = DecayPath::get();
    std::vector<Nuclide>* nuclidesVector = decayPath->GetAllNuclides();
    for ( auto jt = nuclidesVector->begin(); jt != nuclidesVector->end(); ++jt )
    {
        if( (atomicNumber_ - 1 == jt->GetAtomicNumber()) && (atomicMass_ == jt->GetAtomicMass()) )
        {
            for ( auto kt = jt->GetNuclideLevels()->begin(); kt != jt->GetNuclideLevels()->end(); ++kt )
            {
                std::cout << "kt->GetLevelEnergy(): " << kt->GetLevelEnergy() << std::endl;
                std::cout << "fabs(kt->GetLevelEnergy()): " << fabs(kt->GetLevelEnergy()) << std::endl;
                if( fabs(kt->GetLevelEnergy()) <= 0.1 )
                {
                    kt->AddTransition("B-", jt->GetQBeta() - energy, intensity);
                    return;
                }
            }
        }
    }
    std::cout << "Nuclide::AddLevel : Default beta transition not added!" << std::endl;
}


void Nuclide::AddCustomLevel(double energy, double deltaE)
{
    double spin = -1;
    std::string parity = "";
    double T12 = 1e-15;
    nuclideLevels_.push_back( Level(energy, spin, parity, T12) );
    nuclideLevels_.back().setAsPseudoLevel();
    if(deltaE > 0.)
        nuclideLevels_.back().AddTransition("G", deltaE, 1.);
}













