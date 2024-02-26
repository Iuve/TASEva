#ifndef RESPONSEFUNCTION_H
#define RESPONSEFUNCTION_H

#include <vector>
#include <map>

#include "QString"
#include "QDir"

//class Nuclide;
//class Level;
//class Transition;
//class Histogram;


struct TransitionRespContainer
{
    std::map<int, Histogram> TransitionRespHisIDMap;
    double transitionEnergy;
    double transitionIntensity;

    Transition* pointerToTransition; //pointer to Daughter Transition in decayPath
    bool xmlFilesCorrect; //Decay.xml and other .xml exist and the same as before
    bool simulationDone; //true when xmlFilesCorrect==true and output.root is present
    bool sortingDone; //true when xmlFilesCorrect==true, simulationDone==true and sort.root is present
    bool transitionResponseReady; //true when all of the above flags are true and response function uploaded into program
    std::vector< std::pair<QString, bool*> > transitionRespContainerFlags;

};

struct LevelRespContainer
{
    std::vector<TransitionRespContainer> TransitionResp;
    std::map <int, Histogram> LevelRespHisIDMap;
    double levelEnergy;
    double betafeedingtothatLevel;

    Level* pointerToLevel; //pointer to Daughter levels in decayPath
    Transition* pointerToBetaTransitionToThisLevel;
    bool levelFilesReady;
    bool allTransitionResponsesReady;
    bool levelResponseReady;
    std::vector< std::pair<QString, bool*> > levelRespContainerFlags;
};

struct MotherLevelRespContainer
{
    std::vector<LevelRespContainer> LevelResp_;
    std::map<int, Histogram> motherLevelRespHisIDMap;
    double motherLevelEnergy;
    int AtomicMass;
    int AtomicNumber;

    Level* pointerToMotherLevel;
    bool motherLevelFilesReady;
    bool allDaughterLevelResponsesReady;
    bool motherLevelResponseReady;
    std::vector< std::pair<QString, bool*> > motherLevelRespContainerFlags;
};


class ResponseFunction
{
public:
    static ResponseFunction* get();

    ~ResponseFunction();

    void CreateStructure();
    void UpdateStructure();
    void FillTransition(Transition*, std::map<int, Histogram>);
    void CalculateOneLevelRespFunction(Level*);
    void CalculateAllLevelsRespFunction();
    Histogram* GetLevelRespFunction(Level* level, int histID);
    Histogram* GetResponseFunction(Level* motherLevel, int histID);
    void RefreshFlags();
    void ChangeContainerTransitionIntensity(Transition* transition, double newIntensity);
    void ChangeContainerDaughterLevelIntensity(Level* level, double newIntensity);
    void UpdateWholeContainerIntensities();
    void UpdateMotherLevelBetaIntensities(Level* motherLevel);
    void CreateFlagVectors();

    //Histogram* GetLevelRespoFunction(Nuclide* mothernuclide, Level* motherlevel, Transition* transition, int histID);
    //double GetBetaFeedingtothatLevel(Nuclide* motherNuclide, Level* motherlevel, Transition* transition);
    //std::vector<Histogram*> GetAllLevelRespFun(double motherLevelEnergy, int histId);
    //void FillTransition(Transition* transition, std::map<int,Histogram> map);

    //Histogram GetOneTransitionRespFunction(Level* motherLevel, Level* level, Transition* transition, int histID);

    Histogram loadXGate(QDir dirName_, int histId, double xMin, double xMax);
    Histogram loadYGate(QDir dirName_, int histId, double yMin, double yMax);

    TransitionRespContainer* GetPointerToCorrespondingStructure(Transition*);
    LevelRespContainer* GetPointerToCorrespondingStructure(Level*);
    MotherLevelRespContainer* GetPointerToMotherLevelResp(Level*);
    bool GetAllFilesReadyFlag() { return allFilesReady_; }
    void SetAllFilesReadyFlag(bool flag) { allFilesReady_ = flag; }
    bool GetResponseFunctionReady() { return responseFunctionReady_; }
    void SetResponsFunctionReady( bool flag ) { responseFunctionReady_ = flag; }


private:
    //singleton
    ResponseFunction();
    ResponseFunction(ResponseFunction const&);
    void operator=(ResponseFunction const&);
    static ResponseFunction* instance;
    // end singleton

    std::vector<MotherLevelRespContainer> NuclideResponse;
    bool allFilesReady_;
    bool responseFunctionReady_;

};

#endif // RESPONSEFUNCTION_H
