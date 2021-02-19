#include "DeclareHeaders.hh"
#include "histogram.h"
#include "responsefunction.h"
#include "transitionRespProvider.h"

#include "project.h"
#include "MyTemplates.h"
#include "binningController.h"
#include "TH2F.h"
#include "TFile.h"
#include "TKey.h"
#include "TClass.h"
#include "TROOT.h"

#include "qdebug.h"

ResponseFunction::ResponseFunction()
{
    CreateStructure();
    allFilesReady_ = false;
    responseFunctionReady_ = false;
}

ResponseFunction* ResponseFunction::instance = NULL;

ResponseFunction::~ResponseFunction()
{
    delete instance;
    instance =NULL;
}

ResponseFunction* ResponseFunction::get()
{
    if (!instance) {
        instance = new ResponseFunction();
    }
    return instance;
}


void ResponseFunction::CreateStructure()
{
    DecayPath* decayPath = DecayPath::get();

    std::vector<Nuclide>* nuclides = decayPath->GetAllNuclides();
    Nuclide* motherNuclide;
    std::vector<Level>* motherLevels;

    motherNuclide = &nuclides->at(0);
    motherLevels = motherNuclide->GetNuclideLevels();

    for(std::vector<Level>::iterator itl=motherLevels->begin();  itl !=motherLevels->end();  ++itl)
    {
        MotherLevelRespContainer motherLevelRespTMP = MotherLevelRespContainer();
        motherLevelRespTMP.AtomicMass = motherNuclide->GetAtomicMass();
        motherLevelRespTMP.AtomicNumber = motherNuclide->GetAtomicNumber()  ;
        motherLevelRespTMP.motherLevelEnergy = itl->GetLevelEnergy();
        motherLevelRespTMP.pointerToMotherLevel = &(*itl);
        motherLevelRespTMP.motherLevelFilesReady = false;
        motherLevelRespTMP.allDaughterLevelResponsesReady = false;
        motherLevelRespTMP.motherLevelResponseReady = false;

        std::vector<Transition*>* transitions = itl->GetTransitions();
        for(std::vector<Transition*>::iterator itt=transitions->begin(); itt !=transitions->end(); ++itt)
        {
            LevelRespContainer levelRespTMP = LevelRespContainer();
            levelRespTMP.betafeedingtothatLevel = (*itt)->GetIntensity();
            Level* level = (*itt)->GetPointerToFinalLevel();
            levelRespTMP.levelEnergy = level->GetLevelEnergy();
            levelRespTMP.pointerToBetaTransitionToThisLevel = (*itt);
            levelRespTMP.pointerToLevel = level;
            levelRespTMP.levelFilesReady = false;
            levelRespTMP.allTransitionResponsesReady = false;
            levelRespTMP.levelResponseReady = false;

            std::vector<Transition*>* levelTransitions = level->GetTransitions();

            if(levelTransitions->size() == 0)//ground state
            {
                     level->AddTransition( "Fake", 0., 1. );
                     levelTransitions = level->GetTransitions();
            }

            for(auto ittl=levelTransitions->begin(); ittl != levelTransitions->end(); ++ ittl)
            {
                TransitionRespContainer transRespTMP = TransitionRespContainer();
                transRespTMP.transitionEnergy = (*ittl)->GetTransitionQValue();
                transRespTMP.transitionIntensity = (*ittl)->GetIntensity();
                transRespTMP.pointerToTransition = (*ittl);
                transRespTMP.xmlFilesCorrect = false;
                transRespTMP.simulationDone = false;
                transRespTMP.sortingDone = false;
                transRespTMP.transitionResponseReady = false;

                levelRespTMP.TransitionResp.push_back(transRespTMP);
            }
            motherLevelRespTMP.LevelResp_.push_back(levelRespTMP);
        }
        NuclideResponse.push_back(motherLevelRespTMP);
    }
    CreateFlagVectors();
}

void ResponseFunction::CreateFlagVectors()
{
    for( auto mt = NuclideResponse.begin(); mt != NuclideResponse.end(); ++mt)
    {
        for( auto lt = mt->LevelResp_.begin(); lt != mt->LevelResp_.end(); ++lt )
        {
            for( auto tt = lt->TransitionResp.begin(); tt != lt->TransitionResp.end(); ++tt )
            {
                tt->transitionRespContainerFlags.emplace_back( "xmlFilesCorrect", &tt->xmlFilesCorrect );
                tt->transitionRespContainerFlags.emplace_back( "simulationDone", &tt->simulationDone );
                tt->transitionRespContainerFlags.emplace_back( "sortingDone", &tt->sortingDone );
                tt->transitionRespContainerFlags.emplace_back( "transitionResponseReady", &tt->transitionResponseReady );
            }
            lt->levelRespContainerFlags.emplace_back( "levelFilesReady", &lt->levelFilesReady );
            lt->levelRespContainerFlags.emplace_back( "allTransitionResponsesReady", &lt->allTransitionResponsesReady );
            lt->levelRespContainerFlags.emplace_back( "levelResponseReady", &lt->levelResponseReady );
        }
        mt->motherLevelRespContainerFlags.emplace_back( "motherLevelFilesReady", &mt->motherLevelFilesReady );
        mt->motherLevelRespContainerFlags.emplace_back( "allDaughterLevelResponsesReady", &mt->allDaughterLevelResponsesReady );
        mt->motherLevelRespContainerFlags.emplace_back( "motherLevelResponseReady", &mt->motherLevelResponseReady );


    }
}

void ResponseFunction::UpdateStructure()
{
    NuclideResponse.clear();
    CreateStructure();
    allFilesReady_ = false;
    responseFunctionReady_ = false;
}

void ResponseFunction::CalculateAllLevelsRespFunction()
{
    if( responseFunctionReady_ )
    {
        cout << "Response function ready, no need to calculate it." << endl;
        return;
    }

    for( auto it =  NuclideResponse.begin(); it != NuclideResponse.end(); ++it )
    {
        if( it->allDaughterLevelResponsesReady )
            continue;

        Level* motherLevel = it->pointerToMotherLevel;
        for( auto lt = it->LevelResp_.begin(); lt != it->LevelResp_.end(); ++lt )
        {
            if( lt->allTransitionResponsesReady && lt->levelResponseReady )
                continue;

            Level* level = lt->pointerToLevel;
            if( !(lt->allTransitionResponsesReady) )
            {
                bool allTransRespReady = true;
                for( auto tt = lt->TransitionResp.begin(); tt != lt->TransitionResp.end(); ++tt)
                {
                    if( tt->transitionResponseReady )
                        continue;

                    Transition* transition = tt->pointerToTransition;
                    TransitionRespProvider* transitionRespProv = new TransitionRespProvider(motherLevel, transition, level);
                    std::map<int, Histogram> tmpMap = transitionRespProv->getHistograms();
                    FillTransition(transition, tmpMap);
                    allTransRespReady *= tt->transitionResponseReady;
                }
                lt->allTransitionResponsesReady = allTransRespReady;
            }

            if( lt->allTransitionResponsesReady )
                CalculateOneLevelRespFunction(level);
        }
    }

    int iHistID;
    std::map<int, Histogram>* auxiliaryTransitionMap = &NuclideResponse.at(0).LevelResp_.at(0).LevelRespHisIDMap;
    if( !( auxiliaryTransitionMap->size() > 0) )
        cout << "ResponseFunction::CalculateAllLevelsRespFunction: Something is wrong with response function!!" << endl;

    bool respFunctionReady = true;
    for( auto it = NuclideResponse.begin(); it != NuclideResponse.end(); ++it )
    {
        if( it->motherLevelResponseReady && it->allDaughterLevelResponsesReady )
            continue;

        it->motherLevelRespHisIDMap.clear();

        bool allDaughterLevelRespReady = true;
        for( auto id = auxiliaryTransitionMap->begin(); id != auxiliaryTransitionMap->end(); ++id )
        {
            Histogram tmpHist = *Histogram::GetEmptyHistogram();
            iHistID = id->first;
            for( auto lt = it->LevelResp_.begin(); lt != it->LevelResp_.end(); ++lt )
            {
                allDaughterLevelRespReady *= lt->levelResponseReady;
                if( lt->levelResponseReady )
                    tmpHist.Add( &lt->LevelRespHisIDMap.find(iHistID)->second, lt->betafeedingtothatLevel );
                else
                    cout << "ResponseFunction::CalculateAllLevelsRespFunction: allTransitionResponsesReady = false. Response function not added properly." << endl;
            }
            it->motherLevelRespHisIDMap.emplace(iHistID, tmpHist);
        }
        it->allDaughterLevelResponsesReady = allDaughterLevelRespReady;
        if( it->motherLevelRespHisIDMap.size() == auxiliaryTransitionMap->size() )
            it->motherLevelResponseReady = true;
        else
        {
            it->motherLevelResponseReady = false;
            cout << "ResponseFunction::CalculateAllLevelsRespFunction: motherLevelResponseReady = false. That should never happen." << endl;
        }
        respFunctionReady *= it->motherLevelResponseReady;
    }
    responseFunctionReady_ = respFunctionReady;
    cout << "ResponseFunction::CalculateAllLevelsRespFunction: Response function calculation finished with result: " << responseFunctionReady_ << endl;
}

void ResponseFunction::FillTransition(Transition* transition, std::map<int, Histogram> map)
{
    TransitionRespContainer* transitionRespCont = GetPointerToCorrespondingStructure(transition);
    transitionRespCont->TransitionRespHisIDMap = map;
    if( transitionRespCont->TransitionRespHisIDMap.size() > 0)
        transitionRespCont->transitionResponseReady = true;
    else
    {
        cout << "Transition not filled with map or map.size() <= 0." << endl;
        transitionRespCont->transitionResponseReady = false;
    }
}

void ResponseFunction::CalculateOneLevelRespFunction(Level* level)
{
    int iHistID;
    LevelRespContainer* levelRespCont = GetPointerToCorrespondingStructure(level);
    if( levelRespCont->allTransitionResponsesReady && levelRespCont->levelResponseReady )
    {
        cout << "No need to CalculateOneLevelRespFunction. Why am I here?" << endl;
        return;
    }

    levelRespCont->LevelRespHisIDMap.clear();

    std::map<int, Histogram>* auxiliaryTransitionMap = &levelRespCont->TransitionResp.at(0).TransitionRespHisIDMap;
    if( !( auxiliaryTransitionMap->size() > 0) )
        cout << "ResponseFunction::CalculateOneLevelRespFunction: Something is wrong with response function!!" << endl;

    for( auto it = auxiliaryTransitionMap->begin(); it != auxiliaryTransitionMap->end(); ++it )
    {
        Histogram tmpHist = *Histogram::GetEmptyHistogram();
        iHistID = it->first;
        for( auto tt = levelRespCont->TransitionResp.begin(); tt != levelRespCont->TransitionResp.end(); ++tt)
        {
            if( tt->transitionResponseReady )
                tmpHist.Add( &tt->TransitionRespHisIDMap.find(iHistID)->second, tt->transitionIntensity );
            else
                cout << "ResponseFunction::CalculateOneLevelRespFunction: transitionResponseReady = false. Response function not added properly." << endl;
        }
        levelRespCont->LevelRespHisIDMap.emplace(iHistID, tmpHist);
    }
    double energy = levelRespCont->levelEnergy;
    cout << "ResponseFunction::CalculateOneLevelRespFunction for energy level = " << energy << " completed." << endl;
//            ", map.size() = " << levelRespCont->LevelRespHisIDMap.size() << endl;

    levelRespCont->levelResponseReady = true;
}

TransitionRespContainer* ResponseFunction::GetPointerToCorrespondingStructure(Transition* transition)
{
    for( auto it = NuclideResponse.begin(); it != NuclideResponse.end(); ++it)
    {
        for( auto jt = it->LevelResp_.begin(); jt != it->LevelResp_.end(); ++jt )
        {
            for( auto kt = jt->TransitionResp.begin(); kt != jt->TransitionResp.end(); ++kt)
            {
                if( kt->pointerToTransition == transition )
                    return &(*kt);
            }
        }
    }
    cout << "ResponseFunction::GetPointerToCorrespondingStructure(Transition*) failed, returning 0L." << endl;
    return 0L;
}

LevelRespContainer* ResponseFunction::GetPointerToCorrespondingStructure(Level* level)
{
    for( auto it = NuclideResponse.begin(); it != NuclideResponse.end(); ++it)
    {
        for( auto jt = it->LevelResp_.begin(); jt != it->LevelResp_.end(); ++jt )
        {
            if( jt->pointerToLevel == level )
                return &(*jt);
        }
    }
    cout << "ResponseFunction::GetPointerToCorrespondingStructure(Level*) failed, returning 0L." << endl;
    return 0L;
}

MotherLevelRespContainer* ResponseFunction::GetPointerToMotherLevelResp(Level* motherLevel)
{
    for( auto it = NuclideResponse.begin(); it != NuclideResponse.end(); ++it)
    {
        if( it->pointerToMotherLevel == motherLevel )
            return &(*it);
    }
    cout << "ResponseFunction::GetPointerToMotherLevelResp(Level*) failed, returning 0L." << endl;
    return 0L;
}

void ResponseFunction::RefreshFlags()
{
    if( NuclideResponse.size() <= 0 )
        return;

    allFilesReady_ = true;
    responseFunctionReady_ = true;
    for( auto mt = NuclideResponse.begin(); mt != NuclideResponse.end(); ++mt)
    {
        mt->motherLevelFilesReady = true;
        mt->allDaughterLevelResponsesReady = true;
        for( auto lt = mt->LevelResp_.begin(); lt != mt->LevelResp_.end(); ++lt)
        {
            lt->levelFilesReady = true;
            lt->allTransitionResponsesReady = true;
            for( auto tt = lt->TransitionResp.begin(); tt != lt->TransitionResp.end(); ++tt)
            {
                lt->levelFilesReady *= tt->xmlFilesCorrect * tt->simulationDone * tt->sortingDone;
                lt->allTransitionResponsesReady *= tt->transitionResponseReady;
            }
            if( !(lt->allTransitionResponsesReady) )
                lt->levelResponseReady = false;
            mt->motherLevelFilesReady *= lt->levelFilesReady;
            mt->allDaughterLevelResponsesReady *= lt->levelResponseReady;
        }
        if( !(mt->allDaughterLevelResponsesReady) )
        {
            mt->motherLevelResponseReady = false;
        }
        allFilesReady_ *= mt->motherLevelFilesReady;
        responseFunctionReady_ *= mt->motherLevelResponseReady;
    }
}

void ResponseFunction::ChangeContainerTransitionIntensity(Transition* transition, double newIntensity)
{
    TransitionRespContainer* transRespCont = GetPointerToCorrespondingStructure(transition);
    transRespCont->transitionIntensity = newIntensity;

    for( auto mt = NuclideResponse.begin(); mt != NuclideResponse.end(); ++mt )
    {
        for( auto lt = mt->LevelResp_.begin(); lt != mt->LevelResp_.end(); ++lt )
        {
            for( auto tt = lt->TransitionResp.begin(); tt != lt->TransitionResp.end(); ++tt )
            {
                if( transRespCont == &(*tt) )
                {
                    lt->levelResponseReady = false;
                    //RefreshFlags();
                    return;
                }
            }
        }
    }
    cout << "End of ResponseFunction::ChangeContainerTransitionIntensity function. That should never happen!" << endl;
}

void ResponseFunction::ChangeContainerDaughterLevelIntensity(Level* level, double newIntensity)
{
    LevelRespContainer* levelRespCont = GetPointerToCorrespondingStructure(level);
    levelRespCont->betafeedingtothatLevel = newIntensity;

    for( auto mt = NuclideResponse.begin(); mt != NuclideResponse.end(); ++mt )
    {
        for( auto lt = mt->LevelResp_.begin(); lt != mt->LevelResp_.end(); ++lt )
        {
            if( levelRespCont == &(*lt) )
            {
                mt->motherLevelResponseReady = false;
                //RefreshFlags();
                return;
            }
        }
    }
    cout << "End of ResponseFunction::ChangeContainerDaughterLevelIntensity function. That should never happen!" << endl;
}

void ResponseFunction::UpdateWholeContainerIntensities()
{
    for( auto mt = NuclideResponse.begin(); mt != NuclideResponse.end(); ++mt )
    {
        for( auto lt = mt->LevelResp_.begin(); lt != mt->LevelResp_.end(); ++lt )
        {
            for( auto tt = lt->TransitionResp.begin(); tt != lt->TransitionResp.end(); ++tt )
            {
                Transition* transition = tt->pointerToTransition;
                tt->transitionIntensity = transition->GetIntensity();
            }
            lt->levelResponseReady = false;
            Transition* transition = lt->pointerToBetaTransitionToThisLevel;
            lt->betafeedingtothatLevel = transition->GetIntensity();
        }
        mt->motherLevelResponseReady = false;
    }
    responseFunctionReady_ = false;
}




Histogram ResponseFunction::loadXGate(QDir dirName_, int histId, double xMin, double xMax)
{
    cout << "=======>ResponseFunction::loadXGate(int histId, double xMin, double xMax) POCZATEK" << endl;
    Project *myProject = Project::get();
    QString sortOutputFileNameFull = myProject->getSortOutputFileName() + myProject->getSortOutputFileExtension();
    qDebug() << "sortOutputFileNameFull: " << sortOutputFileNameFull;
    QString filename = dirName_.filePath(sortOutputFileNameFull);
    qDebug() << "dirName_ "  << dirName_ ;
    string filenameStr = filename.toStdString();
    cout << "plik z symulacja: " << filenameStr << endl;
    TFile* dataFile = new TFile (filenameStr.c_str());
    std::string histName = num2string(histId);
    TH2F* matrix = (TH2F*) dataFile->Get(histName.c_str());
  cout << "po wczytaniu macierzy" << endl;
    int minBin = static_cast<int> (xMin/BinningController::get2DBinningFactor());
    int maxBin = static_cast<int> (xMax/BinningController::get2DBinningFactor());
    std::string projectionName = histName + "_xGate_" + num2string(xMin) + "_" + num2string(xMax);
    cout << " nazwa widma " << projectionName << " minBin:maxBin " << minBin << ": " <<maxBin << endl;
    TH1F* projection = (TH1F*)matrix->ProjectionY(projectionName.c_str(), minBin, maxBin);

    cout << "przed przed " << projection->GetXaxis()->GetXmin() << endl;
    cout << "przed wyjsciem z GammaRespProvider::loadXGate" << endl;
    Histogram* projectionHist = new Histogram(projection);
    delete projection;
    delete matrix;
    delete dataFile;
    cout << "=======>ResponseFunction::loadXGate(int histId, double xMin, double xMax) KONIEC" << endl;
    return *projectionHist;
}

Histogram ResponseFunction::loadYGate(QDir dirName_, int histId, double yMin, double yMax)
{
    cout << "ResponseFunction::loadYGate" << endl;
    Project *myProject = Project::get();
    QString sortOutputFileNameFull = myProject->getSortOutputFileName() + myProject->getSortOutputFileExtension();
    QString filename = dirName_.filePath(sortOutputFileNameFull);
    string filenameStr = filename.toStdString();

    TFile* dataFile = new TFile (filenameStr.c_str());
    std::string histName = num2string(histId);
    TH2F* matrix = (TH2F*) dataFile->Get(histName.c_str());

    int minBin = static_cast<int> (yMin/BinningController::get2DBinningFactor());
    int maxBin = static_cast<int> (yMax/BinningController::get2DBinningFactor());
    std::string projectionName = histName + "_yGate_" + num2string(yMin) + "_" + num2string(yMax);
    TH1F* projection = (TH1F*)matrix->ProjectionY(projectionName.c_str(), minBin, maxBin);
    Histogram* projectionHist = new Histogram(projection);
    delete projection;
    delete matrix;
    delete dataFile;
    return *projectionHist;
}

Histogram* ResponseFunction::GetLevelRespFunction(Level* level, int histID)
{
    return &GetPointerToCorrespondingStructure(level)->LevelRespHisIDMap.find(histID)->second;
}

Histogram* ResponseFunction::GetResponseFunction(Level* motherLevel, int histID)
{
    //Histogram* tmpHist = &GetPointerToMotherLevelResp(motherLevel)->motherLevelRespHisIDMap.find(histID)->second;
    //cout << "tmpHist->GetNrOfBins(): " << tmpHist->GetNrOfBins() << endl;
    //cout << "tmpHist->GetTitle(): " << tmpHist->GetTitle() << endl;

    return &GetPointerToMotherLevelResp(motherLevel)->motherLevelRespHisIDMap.find(histID)->second;
}


