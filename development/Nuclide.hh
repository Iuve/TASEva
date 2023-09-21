
#ifndef NEW_NUCLIDE_H
#define	NEW_NUCLIDE_H

#include <vector>
#include <map>


class Nuclide
{
public:
    Nuclide(int atomicNumber, int atomicMass, double qBeta, double Sn, std::vector<Level> nuclideLevels);
	
	~Nuclide();
	
	int GetAtomicMass(){return atomicMass_;}
	int GetAtomicNumber(){return atomicNumber_;}
    double GetQBeta(){return qBeta_;}
    double GetD_QBeta(){return d_QBeta_;}
    double GetSn(){return Sn_;}
	std::vector<Level>* GetNuclideLevels(){return &nuclideLevels_;}

    void SetD_QBeta(double d_QBeta){d_QBeta_ = d_QBeta;}

    void RemoveLevel(double);
    void CheckTransitionsToLevel(Level*);
    void DeleteLevels();
    void AddLevelEnergyInten(double, double); //Energy,Intensity
    void AddCustomLevel(double, double); //Energy
    void AddLevel(double, double, std::string, double, double intensity = 0.,
                  double finalLvlEnergy=0., std::string type = "G"); //energy,spin,parity,T12,intensity,neutron

private:
	int atomicNumber_;
	int atomicMass_;
	std::vector<Level> nuclideLevels_;
    double qBeta_;
    double Sn_;
    double d_QBeta_;
	
    //pair of Levels to delete identified by Nuclide and energy of level
    std::vector< std::pair<Nuclide*, double> > levelsToDelete_;

};

#endif	/* DECAY_H */
