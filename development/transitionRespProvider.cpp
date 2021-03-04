#include "DeclareHeaders.hh"
#include "transitionRespProvider.h"
#include "MyTemplates.h"
#include "project.h"
#include "responsefunction.h"

#include "TFile.h"
#include "TKey.h"
#include "TClass.h"
#include "TROOT.h"


#include <QFile>
#include <QCryptographicHash>
#include <QTextStream>
#include <QProcess>
#include <string>
#include <QDebug>
#include <iostream>
#include <fstream>

//sleep() function
#include <thread>
#include <chrono>

TransitionRespProvider::TransitionRespProvider(Level* motherLevel , Transition* transition, Level* level)
{
    transition_ = transition;
    level_ = level;
    motherTransition_ = 0L;
    Project *myProject = Project::get();

    try
    {
        for( auto itt = motherLevel->GetTransitions()->begin(); itt != motherLevel->GetTransitions()->end(); ++itt )
        {
            if( (*itt)->GetPointerToFinalLevel() == level_)
            {
                motherTransition_ = (*itt); //beta transition
                break;
            }
        }

    }
    catch(GenError e)
    {
        cout << "TransitionRespProvider:: Can NOT locate mother level for: " << transition->GetTransitionQValue() << " " << e.show() << endl;
        throw e;
    }

    double levelEnergy = level_->GetLevelEnergy();
    double transitionEnergy = transition_->GetTransitionQValue();
    levelDirName = QString("%1").arg(levelEnergy);
    transitionDirName = QString("%1").arg(transitionEnergy);
    dirName_ = levelDirName + "/" + transitionDirName;
    levelDir_ = checkAndMakeDirectory();
    geantOutputFileName_ = "output";
    geantOutputFileExtension_ = ".root";
    sortOutputFileName_ = myProject->getSortOutputFileName();
    sortOutputFileExtension_ = myProject->getSortOutputFileExtension();
    sortProgramName_ = "sortCalFold2D";
    geantInputFileName_ = "Decay";
    geantInputFileExtension_ = ".xml";
    geantProgramName_ = QString::fromStdString(myProject->getCodeGEANTName()); //  "MTASSimulation_XML";
    macroToGeantName_ = "input.mac";
    inputToSortName_ = "sortInput.txt";
//    makeSimulation();

}

bool TransitionRespProvider::CheckAndCreateDirectories()
{
    ResponseFunction* responseFunction = ResponseFunction::get();
    TransitionRespContainer* respContainer = responseFunction->GetPointerToCorrespondingStructure(transition_);

    //levelDir_ = checkAndMakeDirectory();
    makeInputToGeant( levelDir_);
    cout << "TransitionRespProvider::CheckAndCreateDirectories(): levelDir_: " << levelDir_.filePath(" ").toStdString() << endl;
    vector<bool> xmlUpToDateVector;

    for( auto it = xmlFilenames_.begin(); it != xmlFilenames_.end(); ++it)
    {
        xmlUpToDateVector.push_back( isInputToGeantValid(*it) );
    }

    bool xmlReady = true;
    for (auto it = xmlUpToDateVector.begin(); it != xmlUpToDateVector.end(); ++it)
        xmlReady *= *it;

    respContainer->xmlFilesCorrect = true;
    cout << "TransitionRespProvider::CheckAndCreateDirectories(): xmlReady: " << xmlReady << endl;
    cout << "TransitionRespProvider::CheckAndCreateDirectories(): checkGeantFile(): " << checkGeantFile() << endl;
    cout << "TransitionRespProvider::CheckAndCreateDirectories(): checkSortFile(): " << checkSortFile() << endl << endl;

    if( xmlReady )
    {
        respContainer->simulationDone = checkGeantFile();
        respContainer->sortingDone = checkSortFile();
    }
    else
    {
        respContainer->simulationDone = false;
        respContainer->sortingDone = false;
    }

    bool result = respContainer->xmlFilesCorrect * respContainer->simulationDone * respContainer->sortingDone;
    if( result )
    {
        for( auto it = xmlFilenames_.begin(); it != xmlFilenames_.end(); ++it)
        {
            QString oldName = QString::fromStdString(*it); // with Temp
            string tempName = *it;
            tempName.erase(tempName.begin(),tempName.begin()+4); //Removing 'Temp' part
            QString newName = QString::fromStdString(tempName); // w/o Temp

            levelDir_.remove(newName);
            if (!levelDir_.rename(oldName, newName))
                throw GenError("TransitionRespProvider::runSimulation() : rename exception");
        }
    }

    return result;
}

QDir TransitionRespProvider::checkAndMakeDirectory()
{
    QDir directory(dirName_);
    if(directory.exists())
    {
        //cout << "TransitionRespProvider::checkAndMakeDirectory dir exists" << endl;
        return directory;
    }

    QDir levelDirectory(levelDirName);
    if(!levelDirectory.exists())
        QDir().mkdir(levelDirName);

     if (!QDir().mkdir(dirName_))
         throw GenError("Exception TransitionRespProvider::checkAndMakeDirectory");
     return QDir(dirName_);
}

void TransitionRespProvider::makeInputToGeant(QDir currentDir)
{
    string path =currentDir.absolutePath().toStdString();
    SaveDecayData* outputXMLfiles = new SaveDecayData(path);

    outputXMLfiles->CreateDecayXML( motherTransition_ , transition_);
    outputXMLfiles->SaveSpecifiedDecayStructure( motherTransition_ , transition_);
    xmlFilenames_ = outputXMLfiles->GetShortXmlFilenames();
    xmlFilenamesFullDirectory_ = outputXMLfiles->GetXmlFilenames();
    delete outputXMLfiles;

    ofstream xmlNamesFile;
    string xmlName = path + "/tmp.txt";
    xmlNamesFile.open (xmlName.c_str(), std::ofstream::out | std::ofstream::trunc);
    for( auto it = xmlFilenames_.begin(); it != xmlFilenames_.end(); ++it)
    {
        xmlNamesFile << *it << endl;
    }
    xmlNamesFile.close();
}

bool TransitionRespProvider::checkGeantFile()
{
    return levelDir_.exists(geantOutputFileName_ + geantOutputFileExtension_);
}

bool TransitionRespProvider::checkSortFile()
{
    return levelDir_.exists(sortOutputFileName_ + sortOutputFileExtension_);
}

bool TransitionRespProvider::isInputToGeantValid(string filename)
{
    static QMutex mutex;
    mutex.lock();

    string tempName = filename;
    tempName.erase(tempName.begin(),tempName.begin()+4); //Removing 'Temp' part
    QString oldFilename = levelDir_.filePath( QString::fromStdString(tempName) );
    QFile oldInputFile(oldFilename);

    if(!QFile::exists(oldFilename) || !oldInputFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        cout << "file " << oldFilename.toStdString() << " doesn't exists" << endl;
        mutex.unlock();
        return false;
    }

    QByteArray oldFileData = oldInputFile.readAll();

    QByteArray oldHashData = QCryptographicHash::hash(oldFileData,QCryptographicHash::Md5);

    QString newFilename = levelDir_.filePath( QString::fromStdString(filename) );
    QFile newInputFile(newFilename);
    if(!QFile::exists(newFilename) || !newInputFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
            cout << "file " << newFilename.toStdString() << " doesn't exists" << endl;
            mutex.unlock();
            return false;
    }
    QByteArray newFileData = newInputFile.readAll();

    QByteArray newHashData = QCryptographicHash::hash(newFileData,QCryptographicHash::Md5);
    if(oldHashData != newHashData)
        cout << oldFilename.toStdString() << " is not valid " << endl;

    mutex.unlock();
    return oldHashData == newHashData;
}

bool TransitionRespProvider::MakeSimulationAndSort()
{
    ResponseFunction* responseFunction = ResponseFunction::get();
    TransitionRespContainer* respContainer = responseFunction->GetPointerToCorrespondingStructure(transition_);

    if(respContainer->xmlFilesCorrect)
    {
        if(respContainer->sortingDone)
            return true;
        if(respContainer->simulationDone)
        {
            sortFile();
            return true;
        }
        else
        {
            loadXmlFilenames();

            int lastVersion = findLastVersion();
            if(lastVersion >= 0)
                renameFiles();
            runSimulation();
            return true;
        }
    }
    else
    {
        cout << "Input to Geant files (xml) not ready. Simulation and sorting not done." << endl;
        return false;
    }
}

void TransitionRespProvider::sortFile()
{
    ResponseFunction* responseFunction = ResponseFunction::get();
    TransitionRespContainer* respContainer = responseFunction->GetPointerToCorrespondingStructure(transition_);

    makeInputToSort();
    QDir currentDir = QDir::current();
    QStringList args;
    QString inputFilePath = levelDir_.filePath(inputToSortName_);
    args << inputFilePath;
    QString sortProgramPath = currentDir.absoluteFilePath(sortProgramName_);

    std::chrono::milliseconds timespan1(15000);
    std::chrono::milliseconds timespan2(30000);
    bool ifSortFileExist = levelDir_.exists(sortOutputFileName_ + sortOutputFileExtension_);
/*
    if(!QFile::exists(sortProgramPath))
    {
        std::cout << "TransitionRespProvider::sortFile() no sort program" << std::endl;
        throw GenError("TransitionRespProvider::sortFile() no sort program");
    }
*/
    if(QProcess::execute(sortProgramName_, args) < 0 )  //-1 crash, -2  not started
    {
        respContainer->sortingDone = false;
        qDebug() << "Sorting not done! That will cause errors. ifSortFileExist: " << ifSortFileExist
             << " " << dirName_; //.toStdString() << endl;
    }
    else
    {
        respContainer->sortingDone = true;
    }

    ifSortFileExist = levelDir_.exists(sortOutputFileName_ + sortOutputFileExtension_);

    std::this_thread::sleep_for(timespan1);
    if ( !ifSortFileExist )
    {
        qDebug() << "End of sortFile(). Sort.root wasn't created properly! Loop starts now. " << dirName_; //.toStdString() << endl;

        int ii = 1;

        do
        {
            if(ii > 500)
            {
                qDebug() << "Not able to sort file after 500 iterations. Error. " << dirName_; //.toStdString() << endl;
                qDebug() << "Unsuccessful sort command: " << sortProgramName_ << " "
                     << args[0]; //.toStdString() << endl;
                break;
            }

            qDebug() << ii << " sorting " << dirName_ << " status: " << QProcess::execute(sortProgramName_, args);

            std::this_thread::sleep_for(timespan2);
            ifSortFileExist = levelDir_.exists(sortOutputFileName_ + sortOutputFileExtension_);

            ii++;
        }
        while(!ifSortFileExist);
    }

    if( !ifSortFileExist )
    {
        respContainer->sortingDone = false;
    }
    else
    {
        qDebug() << "Sorting file " << dirName_ << " done properly";
        respContainer->sortingDone = true;
    }
}

bool TransitionRespProvider::makeInputToSort()
{
    int eMax = 9000; //?????
    QFile file(levelDir_.filePath(inputToSortName_));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QString inputPath = levelDir_.filePath(geantOutputFileName_);
    QString outputPath = levelDir_.filePath(sortOutputFileName_);
    QTextStream out(&file);
    out << "GeantOutput " << inputPath << geantOutputFileExtension_ << "\n";
    out << "SortOutput " << outputPath << sortOutputFileExtension_ << "\n";
//    out << "EMin " << 0 << "\n";
    out << "EMax " << eMax << "\n";
    out << "2DbinningFactor " << BinningController::get2DBinningFactor() << "\n";
//    out << "2DbinningFactor " <<  "1" << "\n";
    out << "siliThreshold " << 20 << "\n";

    return true;
}

void TransitionRespProvider::loadXmlFilenames()
{
    QFile xmlNamesFile(levelDir_.filePath("tmp.txt"));
    if (xmlNamesFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&xmlNamesFile);
       while (!in.atEnd())
       {
          QString line = in.readLine();
          xmlFilenames_.push_back(line.toStdString());
          //qDebug() << "line: " << line;
       }
       xmlNamesFile.close();
    }
}

void TransitionRespProvider::runSimulation()
{
    ResponseFunction* responseFunction = ResponseFunction::get();
    TransitionRespContainer* respContainer = responseFunction->GetPointerToCorrespondingStructure(transition_);

    for( auto it = xmlFilenames_.begin(); it != xmlFilenames_.end(); ++it)
//    for( auto it = xmlFilenamesFullDirectory_.begin(); it != xmlFilenamesFullDirectory_.end(); ++it)
    {
        QString oldName = QString::fromStdString(*it); // with Temp
        string tempName = *it;
        tempName.erase(tempName.begin(),tempName.begin()+4); //Removing 'Temp' part
        QString newName = QString::fromStdString(tempName); // w/o Temp

        levelDir_.remove(newName);
        if (!levelDir_.rename(oldName, newName))
            throw GenError("TransitionRespProvider::runSimulation() : rename exception");
    }

    makeMacroToGeant();

    bool ifProcessFinished;
    bool ifGeantFileExist;
    std::chrono::milliseconds timespan1(10000);
    std::chrono::milliseconds timespan2(60000);

    QProcess *process = new QProcess();
    process->setWorkingDirectory(levelDir_.path());
    QStringList args;
    args << macroToGeantName_ << geantInputFileName_ + geantInputFileExtension_;
    QString logFilePath = levelDir_.filePath("mtasSim.log");
    process->setStandardOutputFile(logFilePath);
    process->start(geantProgramName_, args);

    ifProcessFinished = process->waitForFinished(-1);
    process->setWorkingDirectory(QDir::currentPath()); // what does this line do??

    std::this_thread::sleep_for(timespan1);
    ifGeantFileExist = levelDir_.exists(geantOutputFileName_ + geantOutputFileExtension_);

    if(ifProcessFinished)
    {
        qDebug() << "Geant simulation process finished correctly. " << dirName_; //.toStdString() << endl;
        qDebug() << "If output.root exists: " << ifGeantFileExist; // << endl;
    }
    else
    {
        qDebug() << "Geant simulation process not finished. " <<
                 dirName_; //.toStdString() << endl;
        qDebug() << "If output.root exists (process not finished): " << ifGeantFileExist; // << endl;
    }

    delete process;

    int it = 1;

    while( !(ifGeantFileExist && ifProcessFinished) )
    {
        qDebug() << "Geant simulation loop. Iteration number: " << it
             << ". " << dirName_; //.toStdString() << endl;
        qDebug() << "ifGeantFileExist: " << ifGeantFileExist << ", ifProcessFinished: " << ifProcessFinished; // << endl;
        std::this_thread::sleep_for(timespan2);
        QProcess *process = new QProcess();
        process->setWorkingDirectory(levelDir_.path());
        process->setStandardOutputFile(logFilePath);
        process->start(geantProgramName_, args);
        ifProcessFinished = process->waitForFinished(-1);
        std::this_thread::sleep_for(timespan1);
        ifGeantFileExist = levelDir_.exists(geantOutputFileName_ + geantOutputFileExtension_);

        qDebug() << "End of Geant simulation loop. Iteration number: " << it
             << ". " << dirName_; //.toStdString() << endl;
        qDebug() << "(ifGeantFileExist && ifProcessFinished): " << (ifGeantFileExist && ifProcessFinished); // << endl;

        it++;
        delete process;
    }

    if( (ifGeantFileExist && ifProcessFinished) )
    {
        qDebug() << "Geant simulation done, moving to sortFile() function. " << dirName_;
        respContainer->simulationDone = true;
        sortFile();
    }
}

bool TransitionRespProvider::makeMacroToGeant()
{
    int totalNorm = 1e8;
    int maxNorm = 1e6;
//Eva    int norm = static_cast<int> (totalNorm*level_->GetBetaFeedingFunction() * gamma_->GetTotalIntensity());
    int nrOfEvents = 1e5;
//Eva    if (norm > nrOfEvents)
//Eva        nrOfEvents = norm;
    if(nrOfEvents > maxNorm)
       nrOfEvents = maxNorm;

    QFile file(levelDir_.filePath(macroToGeantName_));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

     QTextStream out(&file);
     out << "/control/verbose 1" << "\n";
     out << "/run/verbose 2" << "\n";
     out << "/process/verbose 0" << "\n";
     out << "/tracking/verbose 0" << "\n";
     out << "/vis/verbose 0" << "\n";
     out << "/event/verbose 0" << "\n";
     out << "/material/verbose 0" << "\n";
     out << "/random/resetEngineFrom currentEvent.rndm" << "\n";
     out << "/random/setSavingFlag true" << "\n";
     out << "/run/beamOn " << nrOfEvents << "\n";

     file.close();

     return true;
}

int TransitionRespProvider::findLastVersion()
{
    QStringList fileName;
    QString filter = geantOutputFileName_ + "*" + sortOutputFileExtension_;
    fileName << filter;
    QStringList fileList = levelDir_.entryList(fileName, QDir::Files, QDir::Name);
    if(fileList.empty())
        return -1;
    QString lastName = fileList.back();
    QString lastVersionAndExtension = lastName.remove(geantOutputFileName_ + "_");
    QString lastVersionString = lastVersionAndExtension.remove(geantOutputFileExtension_);
    int lastVersion = lastVersionString.toInt();
    return lastVersion;
}

bool TransitionRespProvider::renameFiles()
{
    int currentVersion = findLastVersion() + 1;
    QString version = QString("_%1").arg(currentVersion, 3, 10,  QLatin1Char('0'));
    QString oldName = geantOutputFileName_ + geantOutputFileExtension_;
    QString newName = geantOutputFileName_ + version + geantOutputFileExtension_;
    levelDir_.rename(oldName, newName);
//    if (!currentDir.rename(oldName, newName))
//        throw GenError("TransitionRespProvider::renameFiles : No geant output file");

    oldName = sortOutputFileName_ + sortOutputFileExtension_;
    newName = sortOutputFileName_ + version + sortOutputFileExtension_;
    levelDir_.rename(oldName, newName);
//    if (!currentDir.rename(oldName, newName))
//        throw GenError("TransitionRespProvider::renameFiles : No sort file");

    oldName = geantInputFileName_ + geantInputFileExtension_;
    newName = geantInputFileName_ + version + geantInputFileExtension_;
    levelDir_.rename(oldName, newName);
//    if (!currentDir.rename(oldName, newName))
//        throw GenError("TransitionRespProvider::renameFiles : No input to geant file");
}

std::map<int, Histogram> TransitionRespProvider::getHistograms()
{
    try
    {
        return loadHistograms();
    }
    catch (GenError e)
    {
        cout << "TransitionRespProvider::getHistograms " << e.show() << endl;
        throw e;
    }
}

std::map<int, Histogram> TransitionRespProvider::loadHistograms()
{
    QString filename = levelDir_.filePath(sortOutputFileName_ + sortOutputFileExtension_);
    string filenameStr = filename.toStdString();
    //const char *filenameCStr = filename.toLocal8Bit().data();
    //cout<< "std::map<int, Histogram*> TransitionRespProvider::loadHistograms() " << filenameStr << endl;

    TFile* dataFile = new TFile (filenameStr.c_str());

    //load all histograms from this file
    TIter next(dataFile->GetListOfKeys());
    TKey *key;
    std::map<int, Histogram> allHistInFile;

    while (key=(TKey*)next())
    {
        TClass *cl = gROOT->GetClass(key->GetClassName());
        if (!cl->InheritsFrom("TH1F"))
            continue;
        TH1F* h = (TH1F*)key->ReadObj();
        std::string histName = h->GetName();
        int histID = string2num <int> (histName, std::dec);

        Histogram* tmpHistogram = new Histogram(h);
        allHistInFile[histID]= *tmpHistogram;

        delete tmpHistogram;
        delete h;

        //binowanie??????? 05.08.13
//Eva        allHistInFile[histID]->Rebin(BinningController::getBinningFactor());//20keV/ch
        //allHistInFile[histID]= Histogram::GetEmptyHistogram();
    }
    dataFile->Close();

    delete dataFile;
    delete key;

    return allHistInFile;
}








/*
void TransitionRespProvider::makeSimulation()
{
    try
    {
        makeInputToGeant( levelDir_);

        if(isInputToGeantValid())
        {
//            cout << "checkSortFile(): " << checkSortFile() << endl;
//            cout << "checkGeantFile(): " << checkGeantFile() << endl;
            if (checkSortFile())
            {
                transition_->SetTransitionRespFlag(true);
                return;
            }
            else if (checkGeantFile())
            {
                sortFile();
                return;
            }
            else
            {
                runSimulation();
                return;
            }

        }
        else
        {
            int lastVersion = findLastVersion();
            if(lastVersion >= 0)
                renameFiles();
            runSimulation();
            return;

        }
    }
    catch (GenError e)
    {
        cout << "TransitionRespProvider::makeSimulation() " << e.show() << endl;
        throw e;
    }
}
*/

/*
  TH2F* GammaRespProvider::load2DHistogram(int histId)
{
    QString filename = levelDir_.filePath(sortOutputFileName_ + sortOutputFileExtension_);
    string filenameStr = filename.toStdString();
    TFile* dataFile = new TFile (filenameStr.c_str());
    std::string histName = num2string(histId);
    TH2F* matrix = (TH2F*) dataFile->Get(histName.c_str());
    return matrix;
}*/
/*Eva
Histogram* TransitionRespProvider::loadXGate(int histId, double xMin, double xMax)
{
    cout << "=======>GammaRespProvider::loadXGate(int histId, double xMin, double xMax) POCZATEK" << endl;
    QString filename = levelDir_.filePath(sortOutputFileName_ + sortOutputFileExtension_);
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
    cout << "=======>GammaRespProvider::loadXGate(int histId, double xMin, double xMax) KONIEC" << endl;

    return projectionHist;
}

Histogram* TransitionRespProvider::loadYGate(int histId, double yMin, double yMax)
{
    cout << "GammaRespProvider::loadYGate" << endl;
    QString filename = levelDir_.filePath(sortOutputFileName_ + sortOutputFileExtension_);
    string filenameStr = filename.toStdString();

    TFile* dataFile = new TFile (filenameStr.c_str());
    std::string histName = num2string(histId);
    TH2F* matrix = (TH2F*) dataFile->Get(histName.c_str());

    int minBin = static_cast<int> (yMin/BinningController::get2DBinningFactor());
    int maxBin = static_cast<int> (yMax/BinningController::get2DBinningFactor());
    std::string projectionName = histName + "_yGate_" + num2string(yMin) + "_" + num2string(yMax);
    TH1F* projection = (TH1F*)matrix->ProjectionX(projectionName.c_str(), minBin, maxBin);
    Histogram* projectionHist = new Histogram(projection);
    delete projection;
    delete matrix;
    delete dataFile;
    return projectionHist;
}
*/


