

#include "project.h"

#include "QDebug"

#include <QApplication>
#include <QDir>
#include <QMessageBox>
//#include <QDialog>
//#include <QInputDialog>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>      // std::stringstream



Project* Project::instance = NULL;

Project::Project()
{
 projectName_ = " ";
 workingDir_ = "/Users/karny/Desktop/89Rb/ ";
 workingDir_ = QDir::currentPath().toUtf8().constData();
// std::cout << "QDir: " << workingDir_ << std::endl;
 expFile_ = "default.his";
 expSpecID_ = "6300";
 exp2DSpecID_ = "6350";
//Eva inputENSFile_ = "Isotope.ens";
//Eva  outputENSFile_ = "outputIsotope.ens";
 inputDecayFile_ = "Decay.xml";
 outputDecayFile_ = "outputdecay.xml";
 outputSIMFile_ = "simulation.his";
 outputLEVFile_ = "levelResponse.his";
 codeGEANT_ = "MTASSimulation_XML";
 codeGEANTver_ = "empty";
 fitEnergyFrom_ = 0.0;
 fitEnergyTo_ = 5000.0;
 fitLevelsFrom_ = 0.0;
 fitLevelTo_ = 5000.0;
 fitLambda_ = 1.0;
 noFitIterations_ = 100;
 decHist_ = *Histogram::GetEmptyHistogram(0.0,10.0,10);
 expHist_ = *Histogram::GetEmptyHistogram(0.0,10.0,10);
 exp2DHist_ = *Histogram::GetEmptyHistogram(0.0,10.0,10);
 recHist_ = *Histogram::GetEmptyHistogram(0.0,10.0,10);
 difHist_ = *Histogram::GetEmptyHistogram(0.0,10.0,10);
 normMin_ = 0.0;
 normMax_ = 10000;

// slotUpdateProjectPanel(true);
}

void  Project::New()
{
//New Project
    projectName_ = " ";
    workingDir_ = QDir::currentPath().toUtf8().constData();
    expFile_ = " ";
    expSpecID_ = " ";
    exp2DSpecID_ = " ";
//    inputENSFile_ = " ";
//    outputENSFile_ = " ";
    inputDecayFile_ = " ";
    outputDecayFile_ = " ";
    outputSIMFile_ = " ";
    outputLEVFile_ = " ";
    codeGEANT_ = " ";
    fitEnergyFrom_ = 0.0;
    fitEnergyTo_ = 5000.0;
    fitLevelsFrom_ = 0.0;
    fitLevelTo_ = 5000.0;
    fitLambda_ = 1.0;
    noFitIterations_ = 100;
    binning1Dfactor_ = 1;
    binning2Dfactor_ = 1;
    activeCoresForSimulation_ = 4;
    codeSORT_ = "SortCalFold2D";
    sortInputFileName_ = "sortInput.txt";
    siliThreshold_ = 10;
    IMOThreshold_ = 20;
    sortXML_ = false;
    gammaEvolutionEnergies_ = std::make_pair(-1, -1);
    checkOutputROOT_ = true;
}


bool Project::Open(string fileName)
{
    // reading a text file
      inputContamination_.clear();
      std::string line;
      std::ifstream myfile (fileName.c_str()) ;
      if (myfile.is_open())
      {
        while ( getline(myfile,line) )
        {
         if(line.compare(0,2,"//") != 0)
//        if (line.substr(0,2) != '//')
         {
            std::cout << line << '\n';
          std::stringstream a ;
          a << line;
          std::string b1, b2, b3 ,b4;
          a >> b1 >> b2 ;
//          std::cout << b1 << " : " << b2 << std::endl;

        if (b1 == "ProjectName:") {projectName_ = b2;}
        else if (b1 == "WorkingDir:") {workingDir_ = b2;}
        else if (b1 == "ExpFile:") {expFile_ = b2;}
        else if (b1 == "ExpSpecID:") {expSpecIDVec_.push_back(b2);}
        else if (b1 == "Exp2DSpecID:") {exp2DSpecID_ = b2;}
        else if (b1 == "InputDecayFile:") {inputDecayFile_ = b2;}
        else if (b1 == "OutputDecayFile:") {outputDecayFile_ = b2;}
        else if (b1 == "OutputSIMFile:") {outputSIMFile_ = b2;}
        else if (b1 == "OutputLEVFile:") {outputLEVFile_ = b2;}
        else if (b1 == "CodeGEANT:") {codeGEANT_ = b2;}
        else if (b1 == "CodeGEANTver:") {codeGEANTver_ = b2;}
        else if (b1 == "NumberOfSimulations") {numberOfSimulations_ = stoi(b2);
        cout << "aaaa" << numberOfSimulations_ << endl;}
        else if (b1 == "Contamination:")
            {
            std::string b5, b6;
            a  >> b3 >> b4 >> b5 >> b6;
            vector<string> con;
            con.push_back(b2); //b2 is exp ID to which contamination belongs
            con.push_back(b3); //b3 is name of file with contamination
            con.push_back(b4); //b4 is contamination percent
            con.push_back(b5); //b5 is contamination histogram ID in file
            if( !b6.empty() )
                con.push_back(b6); //b6 is additional binning/smoothening factor
            inputContamination_.push_back(con);
            }
        else if (b1 == "Binning1D:")
        {
            binning1Dfactor_ = std::stoi(b2);
            decHist_ = *Histogram::GetEmptyHistogram(0, 100, 100 / binning1Dfactor_);
            expHist_ = *Histogram::GetEmptyHistogram(0, 100, 100 / binning1Dfactor_);
            recHist_ = *Histogram::GetEmptyHistogram(0, 100, 100 / binning1Dfactor_);
            difHist_ = *Histogram::GetEmptyHistogram(0, 100, 100 / binning1Dfactor_);
        }
        else if (b1 == "Binning2D:")
        {
            binning2Dfactor_ = std::stoi(b2);
            exp2DHist_ = *Histogram::GetEmptyHistogram(0, 100, 100 / binning2Dfactor_);
        }
        else if (b1 == "SimulationActiveCores:") {activeCoresForSimulation_ = std::stoi(b2);}
        else if (b1 == "SiliconThreshold:") {siliThreshold_ = std::stoi(b2);}
        else if (b1 == "IMOThreshold:") {IMOThreshold_ = std::stoi(b2);}
        else if (b1 =="SortInputFileName:") {sortInputFileName_ = QString::fromStdString(b2);}
        else if (b1 == "SortProgramName:"){codeSORT_ = QString::fromStdString(b2);}
        else if (b1 == "SortXML:")
        {
            if(b2 == "yes")
                sortXML_ = true;
        }
        else if (b1 == "GammaEvolutionRange:")
        {
            a  >> b3;
            double lowE, highE;
            lowE = std::stoi(b2);
            highE = std::stoi(b3);
            gammaEvolutionEnergies_ = std::make_pair(lowE, highE);
        }
        else if (b1 == "CheckOutputROOT:")
        {
            if(b2 == "no")
                checkOutputROOT_ = false;
        }
        else {std::cout << "unknown text in input file " << b1 <<" i "<< b2 << std::endl; }
         }
        }
        myfile.close();
      }

      else {
          QMessageBox msgBox;
          msgBox.setText("Input file does not exists");
          int r = msgBox.exec();
          projectInputFileName_ = "";

//          int r = QMessageBox::warning(this, tr("Error"),
//                                       tr("Project already exists"),
//                                       QMessageBox::Cancel,
//                                       QMessageBox::Ignore);
//          if (r == QMessageBox::OK){
//              return flase;
//          }
          std::cout << "Unable to open file";
          return false;
      }
      expSpecID_ = expSpecIDVec_.at(0);
      readSortXMLInputFile(sortInputFileName_);
      return true;

}
void Project::Save()
{
//Save project
}

/** Instance is created upon first call */
Project* Project::get() {
    if (!instance) {
        instance = new Project();
    }
    return instance;
}

Project::~Project() {
    delete instance;
    instance = NULL;
}

void Project::setWorkingDir(string s)
{
   if(s != workingDir_)
   {
   if(!QDir::setCurrent(QString::fromStdString(s)))
   {
        qDebug() << "Could not change the current working directory to: " << QString::fromStdString(s);
        qDebug() << "Current working directory remains " << QDir::currentPath();
   }
   else {
         workingDir_ = s;
         qDebug() << "Current working directory has been set to: " << QDir::currentPath();
         }
   }
}

void Project::addExpHist(int Id, Histogram hist)
{
   expHistMap_.insert(std::pair<int,Histogram>(Id,hist));
}

void Project::setExpHist()
{
    int hisId = std::stoi(expSpecID_);
    Histogram tmp = getHistFromExpMap(hisId);
    setExpHist(tmp);
}

void Project::replaceExpHistInMap(int Id, Histogram hist)
{
   std::map<int,Histogram>::iterator it;
   it = expHistMap_.find(Id);
   if (it != expHistMap_.end())expHistMap_.erase(it);
   expHistMap_.insert(std::pair<int,Histogram>(Id,hist));
}

Histogram* Project::getHistFromExpMap(int Id)
{
    for(int i = 0; i < expSpecIDVec_.size(); i++)
        if( std::stoi(expSpecIDVec_.at(i)) == Id)
            return &(expHistMap_.find(Id)->second);
    return 0L;
}

void Project::addSimHist(int Id, Histogram hist)
{
   simHistMap_.insert(std::pair<int,Histogram>(Id,hist));
}

void Project::setSimHist()
{
    int hisId = std::stoi(expSpecID_);
    Histogram tmp = getHistFromSimMap(hisId);
    setDecHist(tmp);
}

void Project::replaceSimHistInMap(int Id, Histogram hist)
{
   std::map<int,Histogram>::iterator it;
   it = simHistMap_.find(Id);
   if (it != simHistMap_.end())simHistMap_.erase(it);
   simHistMap_.insert(std::pair<int,Histogram>(Id,hist));
}

Histogram* Project::getHistFromSimMap(int Id)
{
   if (simHistMap_.find(Id) == simHistMap_.end() )
   {
    QMessageBox msgBox;
     QString qText = "Simulated spectrum for ID = "+ QString::number(Id) + " not found. ";
       msgBox.setText(qText);
       msgBox.exec();
       return 0L;
   }  else
   {
       return &(simHistMap_.find(Id)->second);
   }
}

void Project::addRecHist(int Id, Histogram hist)
{
   recHistMap_.insert(std::pair<int,Histogram>(Id,hist));
}

void Project::setRecHist()
{
    int hisId = std::stoi(expSpecID_);
    Histogram tmp = getHistFromRecMap(hisId);
    setRecHist(tmp);
}

void Project::replaceRecHistInMap(int Id, Histogram hist)
{
   std::map<int,Histogram>::iterator it;
   it = recHistMap_.find(Id);
   if (it != recHistMap_.end())recHistMap_.erase(it);
   recHistMap_.insert(std::pair<int,Histogram>(Id,hist));
}

Histogram* Project::getHistFromRecMap(int Id)
{
    for(int i = 0; i < expSpecIDVec_.size(); i++)
        if( std::stoi(expSpecIDVec_.at(i)) == Id)
            return &(recHistMap_.find(Id)->second);
    return 0L;
}


void Project::removeAllContaminations()
{
    contaminations_.clear();
    inputContamination_.clear();
}

void Project::removeContamination(QString name, QString id)
{
    std::cout << "Project::removeContamination()" << endl;
    std::string filename = name.toStdString();
    int idVal = id.toInt();
    cout << filename << " : " << idVal <<endl;
    //std::vector <Contamination>::iterator it;
    for(auto it = contaminations_.begin(); it != contaminations_.end(); ++it)
    {
        if( (*it).second.filename == filename && (*it).second.id == idVal)
        {
            contaminations_.erase(it);
            std::cout << "Project::removeContamination() Found to ERASE" << endl;
            return;
        }

    }
}

std::vector<Contamination> Project::getContaminationsSpecID(int HistID)
{
    std::vector<Contamination> contaminationsSpecID_;
    for(unsigned int con = 0; con != contaminations_.size(); ++con )
    {
        if(contaminations_.at(con).first == HistID){
        contaminationsSpecID_.push_back(contaminations_.at(con).second);
        }
    }


    return contaminationsSpecID_;
}

void Project::readSortXMLInputFile(QString qFileName)
{
    string fileName = qFileName.toStdString();
    std::ifstream myfile (fileName.c_str()) ;
    string line;
    if (myfile.is_open())
    {
      while ( getline(myfile,line) )
      {
          std::cout << line << '\n';
        std::stringstream a ;
        sortInputXML_.push_back(QString::fromStdString(line));
      }
     }
 //   a << line;
   //     a >> b1 >> b2 ;

}
