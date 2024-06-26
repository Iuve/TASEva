
#include "DeclareHeaders.hh"

#include <vector>
#include "Randomize.hh"
#include "project.h"

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

double round_up_to_Level(double value, double precision = 1.0)
{
    return std::round( (value + 0.5*precision) / precision) * precision;
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
                        if(neutronsFromLvL_.size() == 0)
                            neutronLevel_ = false;
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

void Level::AddTransition(std::string type, double transitionQValue, double intensity) //,
//                          double neutronFinalLvlEnergy)
{
    double finalLevelEnergy;
    int finalLevelAtomicMass;
    int finalLevelAtomicNumber;
    int atomicMass, atomicNumber;
    double qBeta, Sn;

    DecayPath* decayPath = DecayPath::get();
    std::vector<Nuclide>* nuclidesVector = decayPath->GetAllNuclides();

    int nuclideIndex = 0 ;
    for ( auto it = nuclidesVector->begin(); it != nuclidesVector->end(); ++it )
    {

        for ( auto jt = it->GetNuclideLevels()->begin(); jt != it->GetNuclideLevels()->end(); ++jt )
        {
            if( &(*jt) == this )
            {
                atomicMass = it->GetAtomicMass();
                atomicNumber = it->GetAtomicNumber();
                qBeta = it->GetQBeta();   // what is is for?
                if(nuclideIndex < nuclidesVector->size()-1)
                Sn = nuclidesVector->at(nuclideIndex).GetSn();  //gets Sn of the next nuclide in chain
//                //nSeparationEnergy = it;
            }
        }
        nuclideIndex++;
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
        std::cout << "Level::AddTransitions: "
                    << levelEnergy_ << " " << Sn << " " << transitionQValue << std::endl;;
        finalLevelEnergy = levelEnergy_ - Sn - transitionQValue;

//        neutronsFromLvL_.push_back(Neutron(type, transitionQValue, intensity,
//            neutronFinalLvlEnergy, finalLevelAtomicMass, finalLevelAtomicNumber));
        neutronsFromLvL_.push_back(Neutron(type, transitionQValue, intensity,
              finalLevelEnergy, finalLevelAtomicMass, finalLevelAtomicNumber));
        transitions_.push_back(&neutronsFromLvL_.back());
        neutronLevel_ = true;    //what is this for?
    }
    else if(type == "A")    //not checked in TASEva
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


void Level::CalculateBetaTransitionsUncertainties(double expSum)
{
    // MS 20240220
    // This calculation is based on Charlie's Rasco Appendix A
    // Equation A4 is implemented here. Few assumptions are made:
    // 1. Number of counts uncertainty of highest fed level is arbitrary, 10% for now (deltaNj/N).
    // 2. Other levels uncertainties are Poisson, so deltaNj = sqrt(Nj) = sqrt(Ij*N)
    // 3. First assumption may vary for different nuclides, analysis. Current approach
    // assumess deltaNj/N = 10% for every level with intensity higher than half of highest intensity.

    //Project* myProject = Project::get();
    //Histogram* tempExpHist = myProject->getExpHist();
    //double minEnergy = 0.;
    //double maxEnergy = 16000.;
    //double expSum = tempExpHist->GetNrOfCounts(minEnergy,maxEnergy);

    Project* myProject = Project::get();
    double uncertModifierInPercents = myProject->getUncertModifierInPercents();
    double arbitraryFactor = 0.1;
    double highestIntensity = 0.;
    Transition* highestIntensityTransition;
    for ( auto kt = transitions_.begin(); kt != transitions_.end(); ++kt )
    {
        double intensity = (*kt)->GetIntensity();
        if(intensity > highestIntensity)
        {
            highestIntensity = intensity;
            highestIntensityTransition = (*kt);
        }
    }

    std::cout << "highestIntensity = " << highestIntensity << std::endl;

    for ( auto kt = transitions_.begin(); kt != transitions_.end(); ++kt )
    {
        double intensity = (*kt)->GetIntensity();
        double uncertainty(0.), firstPart(0.), secondPart(0.), sumForSecondPart(0.);

        //if( *kt == highestIntensityTransition)
        //if( intensity >= highestIntensity/2. )
        if( intensity >= 0.5 )
        {
            std::cout << "highIntensity = " << intensity << std::endl;
            firstPart = pow(arbitraryFactor * intensity * (1 - intensity), 2);
        }
        else
            firstPart = pow((1 - intensity), 2) * intensity / expSum;

        for ( auto kt2 = transitions_.begin(); kt2 != transitions_.end(); ++kt2 )
        {
            if( *kt2 == *kt )
                continue;

            double tempIntensity = (*kt2)->GetIntensity();
            //if( *kt2 == highestIntensityTransition)
            //if( intensity >= highestIntensity/2. )
            if( intensity >= 0.5 )
                sumForSecondPart += pow(arbitraryFactor * intensity, 2);
            else
                sumForSecondPart += tempIntensity / expSum;
        }
        secondPart = sumForSecondPart * intensity * intensity;
        uncertainty = pow(firstPart + secondPart, 0.5);

        // rough correction for:
        // 87Br: uncertainty Charlie's A4 formula times 10, then added 5% of feeding
        // 88Br: uncertainty Charlie's A4 formula times 10, then added 7% of feeding
        // 88Br: for levels 7050+ uncertainty is multiplied by 1.5 on top of everything else
        uncertainty *= 10;
        uncertainty += intensity * uncertModifierInPercents/100.;
        //double levelEnergy = (*kt)->GetFinalLevelEnergy();
        //if(levelEnergy >= 7050)
        //    uncertainty *= 1.5;

        // now correct for precision
            std::string intensityString = std::to_string(intensity * 100);

            if(intensityString.find('.') == 2) //feeding >= 10
                uncertainty = round_up_to_Level(uncertainty, 0.01);
            else // feeding < 10
            {
                if(intensityString[0] == '0')
                {
                    if(intensityString[2] == '0' && intensityString[3] == '0')
                        uncertainty = round_up_to_Level(uncertainty, 0.00001);
                    else
                        uncertainty = round_up_to_Level(uncertainty, 0.0001);
                }
                else
                    uncertainty = round_up_to_Level(uncertainty, 0.001);
            }

        (*kt)->SetD_Intensity(uncertainty);
    }
}












