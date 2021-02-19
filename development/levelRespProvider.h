#ifndef LEVELRESPPROVIDER_H
#define LEVELRESPPROVIDER_H
//#include "DeclareHeaders.hh"
#include "transitionRespProvider.h"

#include <string>
#include <QMutex>
#include <QDir>
#include "TH2F.h"
#include "TFile.h"

class Histogram;

class LevelRespProvider
{
public:
    LevelRespProvider(Level *motherLevel, Level* level);
    ~LevelRespProvider();
    //std::map<int, Histogram*> getHistograms();
    Histogram* getResponse(int histId);
//Eva    Histogram* loadXGate(int histId, float xMin, float xMax);
//Eva    Histogram* loadYGate(int histId, float yMin, float yMax);

    bool CheckAndCreateDirectories();
    bool MakeSimulations();

private:

    std::map<int, Histogram*> loadHistograms();
    Level* motherLevel_;
    Level* level_;
    std::vector<Transition*>* transitions_;
    //TransitionRespProvider* transResp_;

};

#endif // LEVELRESPPROVIDER_H

