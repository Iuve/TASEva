#ifndef DECAYSPECTRUM_H
#define DECAYSPECTRUM_H

//#include "DeclareHeaders.hh"
#include "histogram.h"

class LevelRespProvider;

class DecaySpectrum
{
public:
    //DecaySpectrum(std::vector<Level*> levelsVal);
    DecaySpectrum();

    virtual  ~DecaySpectrum();

    //Histogram* GetDecaySpectrum(int histID);

    //std::vector < Histogram* > getLevelsResponse(int histID);
    //Histogram* getSingleLevelResponse(Level* level, int histID);

    //bool MakeLevelResponseFunction(Level *motherLevel, Level *level);
    bool CheckAndCreateDirectories();
    bool MakeSimulations();
    bool MakeLevelSimulations(Level* motherLevel, Level *level);

private:
   // std::vector<Level*> levels;
};

#endif // DECAYSPECTRUM_H




