#include "DeclareHeaders.hh"
#include "levelRespProvider.h"
#include "histogram.h"
#include "responsefunction.h"

#include "TFile.h"
#include "TKey.h"
#include "TClass.h"
#include "TROOT.h"

#include <QFile>
#include <QCryptographicHash>
#include <QProcess>
#include <string>

LevelRespProvider::LevelRespProvider(Level* motherLevel, Level* level)
{
    motherLevel_ = motherLevel;
    level_ = level;
    transitions_ = level_->GetTransitions();
}

LevelRespProvider::~LevelRespProvider()
{

}

bool LevelRespProvider::CheckAndCreateDirectories()
{
    ResponseFunction* responseFunction = ResponseFunction::get();
    LevelRespContainer* levelRespCont =  responseFunction->GetPointerToCorrespondingStructure(level_);

    //response functions have been calculated and checked earlier
    if(levelRespCont->allTransitionResponsesReady)
        return true;

    //response function haven't been calculated or checked earlier
    bool levelFilesReady = true;
    for( auto it = levelRespCont->TransitionResp.begin(); it != levelRespCont->TransitionResp.end(); ++it )
    {
        bool transitionRespReady = it->xmlFilesCorrect && it->simulationDone && it->sortingDone;
        if( !transitionRespReady )
        {
            TransitionRespProvider* transRespProv = new TransitionRespProvider(motherLevel_, it->pointerToTransition, level_);
            transitionRespReady = transRespProv->CheckAndCreateDirectories();
            delete transRespProv;
        }

        levelFilesReady *= transitionRespReady;
    }

    levelRespCont->levelFilesReady = levelFilesReady;
    return levelFilesReady;
}

bool LevelRespProvider::MakeSimulations()
{
    ResponseFunction* responseFunction = ResponseFunction::get();
    LevelRespContainer* levelRespCont =  responseFunction->GetPointerToCorrespondingStructure(level_);

    bool levelSimReady = true;
    for( auto it = levelRespCont->TransitionResp.begin(); it != levelRespCont->TransitionResp.end(); ++it )
    {
        bool transitionSimReady = it->simulationDone && it->sortingDone;
        if( !transitionSimReady )
        {
            TransitionRespProvider* transRespProv = new TransitionRespProvider(motherLevel_, it->pointerToTransition, level_);
            transitionSimReady = transRespProv->MakeSimulationAndSort();
            //transitionRespReady = transRespProv->CheckAndCreateDirectories();
            delete transRespProv;
        }
        levelSimReady *= transitionSimReady;
    }

    levelRespCont->levelFilesReady = levelSimReady;
    return levelSimReady;
}



/*
LevelRespProvider::LevelRespProvider(Level* motherLevel, Level* level)
{
    motherLevel_ = motherLevel;
    level_ = level;
    transitions_ = level_->GetTransitions();
    bool TransitionsOKflag = true;
    ResponseFunction* responseFunction = ResponseFunction::get();

    if(transitions_->size() == 0)//ground state
    {
             level->AddTransition( "Fake", 0., 1. );
             transitions_ = level->GetTransitions();
    }

    std::vector<Transition*>::iterator it;
    for(it  = transitions_->begin(); it != transitions_->end(); it++)
    {
//Eva        (*it)->MakeResponseFunction(decay_, level_);
       if( (*it)->GetTransitionRespFlag() )
       {
           continue ; //true;
       }
       else
       {
           try
           {
              TransitionRespProvider* transResp = new TransitionRespProvider(motherLevel_,(*it), level_);
              TransitionsOKflag = TransitionsOKflag && (*it)->GetTransitionRespFlag();
              responseFunction->FillTransitions( motherLevel_, level_, (*it), transResp->getHistograms() );
           }
           catch(GenError e)
           {
               cout << "MakeTransitionResponseFunction(Decay* decay, Level* level)" << e.show() << endl;
               throw e;
           }
           continue ; //true;
       }
       // gdzies tu dodac level resp w contanerze !!!!!!!!
    }
    if(TransitionsOKflag)
    {
        level_->SetLevelRespFlag(true);
        responseFunction->CalculateOneLevelRespFunction(motherLevel_, level_);
    }
}

Histogram* LevelRespProvider::getResponse(int histId)
{
    Histogram* resp = Histogram::GetEmptyHistogram(0.,10.0,10);
    std::vector<Transition*>::iterator TransitionIt;
    for(TransitionIt  = transResp_->begin(); TransitionIt != transResp_->end(); TransitionIt++)
    {
        float norm = (*TransitionIt)->GetIntensity();
        resp->Add((*TransitionIt)->GetResponseFunction(decay_, level_, histId), norm);
    }
    return resp;

}
*/

/*Eva
Histogram* LevelRespProvider::loadXGate(int histId, float xMin, float xMax)
{
// to create a right size vector for getting resp I need to "copy"
// a single gamma response
    cout << "=====>LevelRespProvider::loadXGate(int histId, float xMin, float xMax) POCZATEK" << endl;

    Histogram *resp = Histogram::GetEmptyHistogram(0.0, 10000.0, 10000); //do zautomatyzowania jakos ale nie tak jak wyzej ;)

    cout << "!=====>LevelRespProvider::loadXGate() po gamma_.at(0)->GetXGate" << endl;

    std::vector<Gamma*>::iterator gammaIt;
    cout << "LevelRespProvider::loadXGate: " << "histId: " <<histId
         << " xMin: " << xMin << " xMax: " << xMax << endl;
    for(gammaIt  = gamma_.begin(); gammaIt != gamma_.end(); gammaIt++)
    {
        float norm = (*gammaIt)->GetTotalIntensity();
        cout << "norm: " << norm<< endl;
       resp->Add((*gammaIt)->GetXGate(histId,  xMin, xMax), norm);
        cout << "AFTER ADD to the histogram" << endl;
    }
    cout << "LevelRespProvider::loadXGate() DO ZWROTU WYZEJ: ";
    cout << " NrOfBins: " << resp->GetNrOfBins() ;
    cout << " xMin: " << resp->GetXMin() ;
    cout << " xMax: " << resp->GetXMax() << endl;
            cout << "=====>LevelRespProvider::loadXGate(int histId, float xMin, float xMax) KONIEC" << endl;

    return resp;
}

Histogram* LevelRespProvider::loadYGate(int histId, float yMin, float yMax)
{
    Histogram* resp = Histogram::GetEmptyHistogram();
    std::vector<Gamma*>::iterator gammaIt;
    for(gammaIt  = gamma_.begin(); gammaIt != gamma_.end(); gammaIt++)
    {
        float norm = (*gammaIt)->GetTotalIntensity();
        resp->Add((*gammaIt)->GetYGate(histId,  yMin, yMax), norm);
    }
    return resp;
}

*/
