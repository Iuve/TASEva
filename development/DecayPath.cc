
#include "DeclareHeaders.hh"

#include <vector>
#include <iostream>
#include <string>

DecayPath::DecayPath()
{
	
}

DecayPath* DecayPath::instance = NULL;

DecayPath::~DecayPath()
{
    if(loadDecayData_)
            delete loadDecayData_;
    delete instance;
    instance =NULL;
}

DecayPath* DecayPath::get()
{
    if (!instance) {
        instance = new DecayPath();
    }
    return instance;
}

void DecayPath::LoadDecay(std::string xmlFileName)
{
    std::cout << "Loading data from "<< xmlFileName << std::endl;
	loadDecayData_ = new LoadDecayData(xmlFileName);
    allNuclides_ = loadDecayData_->GetAllNuclides();
    startLevel_ = loadDecayData_->GetStartLevel();
    stopLevel_ = loadDecayData_->GetStopLevel();
    cycleDurationInSeconds_ = loadDecayData_->GetCycleDurationInSeconds();
    eventDurationInSeconds_ = loadDecayData_->GetEventDurationInSeconds();
    FindAndMarkNeutronLevels();

    std::cout << std::endl;
    std::cout << "NUMBER OF BETA TRANSITIONS = " << startLevel_->GetTransitions()->size();
    std::cout << std::endl;
    std::cout << "NUMBER OF MOTHER LEVELS = " << allNuclides_->at(1).GetNuclideLevels()->size();
    std::cout << std::endl;
}


void DecayPath::RecalculatePointers()
{
    for ( auto it = allNuclides_->begin(); it != allNuclides_->end(); ++it)
    {
        for ( auto jt = it->GetNuclideLevels()->begin(); jt !=it->GetNuclideLevels()->end(); ++jt )
        {
            (*jt).UpdateTransitionVector();
        }
    }

    loadDecayData_->RecalculatePointersToLevels();

}

void DecayPath::SetPointerToLevel(Transition* transition, int atomicNumber, int atomicMass, double energy)
{
    std::cout << "DecayPath::SetPointerToLevel" << std::endl;
    std::cout << "transition->GetTransitionQValue() " << transition->GetTransitionQValue() << std::endl;
    loadDecayData_->SetPointerToLevel(transition, atomicNumber, atomicMass, energy);
}

std::vector<Transition*>* DecayPath::GetBetaTransitionsFromFirstNuclide()
{
    std::vector<Level>* motherLevels_ = allNuclides_->at(0).GetNuclideLevels();
    std::vector<Transition*>* betaTransitions = motherLevels_->at(0).GetTransitions();
    return betaTransitions;
}

void DecayPath::FindAndMarkNeutronLevels()
{
    std::vector<Transition*>* betaTransitions = GetBetaTransitionsFromFirstNuclide();


    for(auto itb = betaTransitions->begin(); itb != betaTransitions->end(); ++itb)
    {
        Level* tmpLevel = (*itb)->GetPointerToFinalLevel();
        std::vector<Transition*>* transitionsFromLevel = tmpLevel->GetTransitions();
        std::string transitionType;

        for( auto it = transitionsFromLevel->begin(); it != transitionsFromLevel->end(); ++it)
        {
            transitionType = (*it)->GetParticleType();
            if( transitionType == "N" )
            {
                tmpLevel->SetAsNeutronLevel();
                break;
            }
        }
    }
}
