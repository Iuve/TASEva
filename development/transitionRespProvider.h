#ifndef TRANSITIONRESPPROVIDER_H
#define TRANSITIONRESPPROVIDER_H

#include "histogram.h"
#include <map>
#include <string>
#include <QMutex>
#include <QDir>
#include "TH2F.h"

//class Gamma;
//class Transition;
//class Level;
//class DecayPath;

class TransitionRespProvider
{
public:
    TransitionRespProvider(Level *motherLevel_, Transition* transition, Level* level);
    std::map<int, Histogram> getHistograms();
    //TH2F* load2DHistogram(int histId);
//EVa    Histogram* loadXGate(int histId, double xMin, double xMax);
//Eva    Histogram* loadYGate(int histId, double yMin, double yMax);
    QString GetSortOutputFileNameFull(){ return (sortOutputFileName_+sortOutputFileExtension_); }
    bool CheckAndCreateDirectories();
    bool MakeSimulationAndSort();

private:
    void makeSimulation();
    void makeInputToGeant(QDir currentDir);
    QDir checkAndMakeDirectory();
    bool checkGeantFile();
    bool checkSortFile();
    bool isInputToGeantValid(string);
    int findLastVersion();
    bool renameFiles();
    bool makeMacroToGeant();
    bool makeInputToSort();
    std::map<int, Histogram> loadHistograms();
    void runSimulation();
    void sortFile();
    void loadXmlFilenames();

    std::string directoryName_;
    Transition* transition_;
    Transition* motherTransition_;
//Eva out?    Gamma* gamma_;
    Level* level_;
    QString dirName_;
    QDir levelDir_;
    QString nuclidDirName;
    QString levelDirName;
    QString transitionDirName;

    QString geantOutputFileName_;
    QString geantOutputFileExtension_;
    QString sortOutputFileName_;
    QString sortOutputFileExtension_;
    QString sortProgramName_;
    QString geantInputFileName_;
    QString geantInputFileExtension_;
    QString geantProgramName_;
    QString macroToGeantName_;
    QString inputToSortName_;

    string codeGEANTver_;

    vector<string> xmlFilenames_;
    vector<string> xmlFilenamesFullDirectory_;
    bool xmlYesAll_;
    bool xmlAnsAll_;

};

#endif // TRANSITIONRESPPROVIDER_H

