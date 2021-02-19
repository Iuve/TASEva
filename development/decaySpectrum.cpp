
#include "DeclareHeaders.hh"
#include "decaySpectrum.h"
#include "HistIds.h"
#include "Exceptions.h"
//Eva #include "MyTemplates.h"
#include "levelRespProvider.h"
#include "responsefunction.h"

#include <QFuture>
#include "qtconcurrentrun.h"
#include <QThreadPool>

#include <fstream>
#include <functional>
//sleep() function
#include <thread>
#include <chrono>

DecaySpectrum::DecaySpectrum()
{

}

DecaySpectrum::~DecaySpectrum()
{

}

bool DecaySpectrum::CheckAndCreateDirectories()
{
    ResponseFunction* responseFunction = ResponseFunction::get();
    if( responseFunction->GetAllFilesReadyFlag() )
    {
        cout << "DecaySpectrum::CheckAndCreateDirectories: All files are ready." << endl;
        return true;
    }

    DecayPath* decayPath = DecayPath::get();
    std::vector<Nuclide>* nuclides = decayPath->GetAllNuclides();
    std::vector<Level>* motherLevels;
    Nuclide* motherNuclide;

    motherNuclide = &nuclides->at(0);
    motherLevels = motherNuclide->GetNuclideLevels();

    bool allMotherLevelFilesReady = true;

    for(std::vector<Level>::iterator itn = motherLevels->begin(); itn !=motherLevels->end(); ++itn)
    {
        Level* motherLevel = &(*itn);
        MotherLevelRespContainer* motherLevelResponseContainer = responseFunction->GetPointerToMotherLevelResp(motherLevel);
        if( motherLevelResponseContainer->motherLevelFilesReady )
            continue;

        std::vector<Transition*>* transitions = motherLevel->GetTransitions(); //all of beta transitions
        bool allLevelFilesReady = true;

        for(std::vector<Transition*>::iterator itt = transitions->begin(); itt != transitions->end(); ++itt)
        {
            Level* currentLevel = (*itt)->GetPointerToFinalLevel();
            LevelRespContainer* levelRespCont =  responseFunction->GetPointerToCorrespondingStructure(currentLevel);
            if( levelRespCont->levelFilesReady )
                continue;

            LevelRespProvider* levelRespProv = new LevelRespProvider(motherLevel, currentLevel);
            levelRespProv->CheckAndCreateDirectories();
            delete levelRespProv;

            allLevelFilesReady *= levelRespCont->levelFilesReady;
        }
        motherLevelResponseContainer->motherLevelFilesReady = allLevelFilesReady;
        allMotherLevelFilesReady *= allLevelFilesReady;
    }
    responseFunction->SetAllFilesReadyFlag( allMotherLevelFilesReady );
    if( allMotherLevelFilesReady )
        cout << "DecaySpectrum::CheckAndCreateDirectories: All files have been just prepared." << endl;
    else
        cout << "DecaySpectrum::CheckAndCreateDirectories ended, but files are still not ready." << endl;

    return allMotherLevelFilesReady;
}

bool DecaySpectrum::MakeSimulations()
{
    ResponseFunction* responseFunction = ResponseFunction::get();
    if( responseFunction->GetAllFilesReadyFlag() )
    {
        cout << "DecaySpectrum::MakeSimulations: All files are ready." << endl;
        return true;
    }

        DecayPath* decayPath = DecayPath::get();
        std::vector<Nuclide>* nuclides = decayPath->GetAllNuclides();
        std::vector<Level>* motherLevels;
        Nuclide* motherNuclide;

        motherNuclide = &nuclides->at(0);
        motherLevels = motherNuclide->GetNuclideLevels();

        bool allFilesReady = true;

        for(std::vector<Level>::iterator itn = motherLevels->begin(); itn !=motherLevels->end(); ++itn)
        {
            MotherLevelRespContainer* motherLevelRespCont = responseFunction->GetPointerToMotherLevelResp( &(*itn) );
            if( motherLevelRespCont->motherLevelFilesReady )
                continue;

            Level* motherLevel = &(*itn);
            std::vector<Transition*>* transitions = motherLevel->GetTransitions();  // getting transitions
            std::vector< QFuture<bool> > futureResults;

            std::chrono::milliseconds timespan2(60000);

            int coresNumber = 4;
            int activeProcesses[coresNumber];
            for(int j = 0; j < coresNumber; j++)
                activeProcesses[j] = 10000;
            int iterationNumber = 0;
            bool processStartedFlag = false;
            int transitionsSize = transitions->size();

            for(std::vector<Transition*>::iterator itt = transitions->begin(); itt != transitions->end(); ++itt)
            {
                Level* currentLevel = (*itt)->GetPointerToFinalLevel();

                LevelRespContainer* levelRespCont = responseFunction->GetPointerToCorrespondingStructure(currentLevel);
                if( levelRespCont->levelFilesReady )
                    continue;

                for(int i = 0; i < coresNumber; i++)
                {
                    if(activeProcesses[i] == 10000 && !processStartedFlag)
                    {
                        futureResults.push_back(QtConcurrent::run(this, &DecaySpectrum::MakeLevelSimulations, motherLevel, currentLevel));
                        activeProcesses[i] = iterationNumber;
                        cout << "Process number " << activeProcesses[i] << " started." << endl;
                        processStartedFlag = true;
                    }
                    else if( activeProcesses[i] != 10000)
                    {
                        if( futureResults.at( activeProcesses[i] ).isFinished() == true )
                        {
                            cout << "Process number " << activeProcesses[i] << " completed." << endl;
                            activeProcesses[i] = 10000;
                        }
                    }

                    if( i == coresNumber - 1 && !processStartedFlag)
                    {
                        i = -1;
                        std::this_thread::sleep_for(timespan2);
                    }
                }

                iterationNumber++;
                processStartedFlag = false;
            }

            bool motherLevelSimReady = true;
           for(unsigned int i = 0; i < futureResults.size(); ++i)
           {
               futureResults.at(i).waitForFinished();
               cout << "QtConcurrent finished for iteration number : " << i << endl;
               motherLevelSimReady *= futureResults.at(i);
           }
           motherLevelRespCont->motherLevelFilesReady = motherLevelSimReady;
           allFilesReady *= motherLevelSimReady;
    }
    responseFunction->SetAllFilesReadyFlag(allFilesReady);
    if( allFilesReady )
        cout << "DecaySpectrum::MakeSimulations: All files have been just prepared." << endl;
    else
        cout << "DecaySpectrum::MakeSimulations ended, but files are still not ready." << endl;

    return allFilesReady;
}

bool DecaySpectrum::MakeLevelSimulations(Level* motherLevel, Level *level)
{
    LevelRespProvider* levelRespProv = new LevelRespProvider(motherLevel, level);
    bool levelSimReady = levelRespProv->MakeSimulations();
    delete levelRespProv;

    return levelSimReady;
}


