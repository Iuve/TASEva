
#ifndef NEW_DECAY_H
#define	NEW_DECAY_H

#include "G4ParticleDefinition.hh"

#include <vector>
#include <string>

//class LoadDecayData;
//class Nuclide;
//class Transition;
//class Level;

struct Event
{
	double  energy;
	G4ParticleDefinition* type;
	Event(double energyVal, G4ParticleDefinition* typeVal):
	energy(energyVal), type(typeVal){}
	
};

class DecayPath
{
public:
    //DecayPath(); // loading and preparing data
    static DecayPath* get();
	
	~DecayPath();
	
	void LoadDecay(std::string xmlFileName);
	std::vector<Nuclide>* GetAllNuclides() { return allNuclides_; }

    void RecalculatePointers();
    void SetPointerToLevel(Transition* transition, int atomicNumber, int atomicMass, double energy);

    double GetCycleDurationInSeconds() { return cycleDurationInSeconds_; }
    double GetEventDurationInSeconds() { return eventDurationInSeconds_; }
    Level* GetPointerToStartLevel() { return startLevel_; }
    Level* GetPointerToStopLevel() { return stopLevel_; }
    std::vector<Transition*>* GetBetaTransitionsFromFirstNuclide();
    void FindAndMarkNeutronLevels();
	
private:
    //singleton
        DecayPath();
        DecayPath(DecayPath const&);

        void operator=(DecayPath const&);
        static DecayPath* instance;
    // end singleton

	LoadDecayData* loadDecayData_;
	std::vector<Nuclide>* allNuclides_;
	Level* startLevel_;
	Level* stopLevel_;
	double cycleDurationInSeconds_;
	double eventDurationInSeconds_;
};

#endif	/* DECAY_H */
