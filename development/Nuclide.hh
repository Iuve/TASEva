
#ifndef NEW_NUCLIDE_H
#define	NEW_NUCLIDE_H

#include <vector>
#include <map>


class Nuclide
{
public:
    Nuclide(int atomicNumber, int atomicMass, double qBeta, std::vector<Level> nuclideLevels);
	
	~Nuclide();
	
	int GetAtomicMass(){return atomicMass_;}
	int GetAtomicNumber(){return atomicNumber_;}
    double GetQBeta(){return qBeta_;}
	std::vector<Level>* GetNuclideLevels(){return &nuclideLevels_;}

    void RemoveLevel(double);
    void CheckTransitionsToLevel(Level*);
    void DeleteLevels();
    void AddLevelEI(double, double); //Energy,Intensity
    void AddLevel(double, double, std::string, double, double intensity = 0.); //energy,spin,parity,T12,intensity

private:
	int atomicNumber_;
	int atomicMass_;
	std::vector<Level> nuclideLevels_;
    double qBeta_;
	
    //pair of Levels to delete identified by Nuclide and energy of level
    std::vector< std::pair<Nuclide*, double> > levelsToDelete_;

};

#endif	/* DECAY_H */
