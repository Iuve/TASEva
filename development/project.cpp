

#include "project.h"

#include "QDebug"

#include <QApplication>
#include <QDir>
#include <QMessageBox>

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
    binning2Dfactor_ = 1;
    activeCoresForSimulation_ = 4;
}


void Project::Open(string fileName)
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
        else if (b1 == "Contamination:")
            {
            a  >> b3 >> b4;
            vector<string> con;
            con.push_back(b2);
            con.push_back(b3);
            con.push_back(b4);
            inputContamination_.push_back(con);
            }
        else if (b1 == "Binning2D:") {binning2Dfactor_ = std::stoi(b2);}
        else if (b1 == "SimulationActiveCores:") {activeCoresForSimulation_ = std::stoi(b2);}
        else {std::cout << "unknown text in input file " << b1 <<" i "<< b2 << std::endl; }
         }
        }
        myfile.close();
      }

      else std::cout << "Unable to open file";
      expSpecID_ = expSpecIDVec_.at(0);
//      return 0;

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

Histogram Project::getHistFromExpMap(int Id)
{
   return expHistMap_.find(Id)->second;
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
    std::vector <Contamination>::iterator it;
    for(it = contaminations_.begin(); it != contaminations_.end(); ++it)
    {
        if( (*it).filename == filename && (*it).id == idVal)
        {
            contaminations_.erase(it);
            std::cout << "Project::removeContamination() Found to ERASE" << endl;
            return;
        }

    }
}

