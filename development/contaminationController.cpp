#include "contaminationController.h"
//Eva #include "ui_pileupController.h"
//Eva #include "pileupController.h"

#include <project.h>

ContaminationController::ContaminationController()
{
    //Project *myProject = Project::get();
    //contaminations_ = *(myProject->getContaminations());
  /*    connect(contaminationPanel,
            SIGNAL(contaminationAdded(QString, QString, QString)),
            SLOT(addContamination(QString, QString, QString)));
    connect(contaminationPanel,
            SIGNAL(contaminationRemoved(QString, QString)),
            SLOT(removeContamination(QString, QString)));
    connect(contaminationPanel,
            SIGNAL(contaminationEdited(QString, QString, QString)),
            SLOT(editContamination(QString, QString, QString)));
*/
}

ContaminationController::~ContaminationController()
{
    //Project *myProject = Project::get();
    //myProject->setContaminations(contaminations_);
    cout << "Contaminations saved in ContaminationController destructor." << endl;
//    contaminations_.clear();
}

void ContaminationController::SaveAll()
{
       Project *myProject = Project::get();
       myProject->setContaminations(contaminations_);
       cout << "contaminations saved" << endl;
}

/*
void ContaminationController::removeAll()
{
    contaminations_.clear();
    Project *myProject = Project::get();
    myProject->removeAllContaminations();

}
*/
/*
Histogram* ContaminationController::getContamination(string name, int id)
{
    for(unsigned int i = 0; i != contaminations_.size(); ++i)
    {
        if(contaminations_.at(i).filename == name && contaminations_.at(i).id == id)
            return contaminations_.at(i).hist;
    }
    cout << "no contaminations named " << name <<" id: " << id;
    return 0L;
}
*/
/*
float ContaminationController::getNormalization(string name, int id)
{
    for(unsigned int i = 0; i != contaminations_.size(); ++i)
    {
        if(contaminations_.at(i)->filename == name && contaminations_.at(i)->id == id)
            return contaminations_.at(i)->normalization;
    }
    cout << "no containations named " << name <<" id: " << id;
    return 0L;
}

float ContaminationController::getIntensity(string name, int id)
{
    for(unsigned int i = 0; i != contaminations_.size(); ++i)
    {
        if(contaminations_.at(i)->filename == name && contaminations_.at(i)->id == id)
            return contaminations_.at(i)->intensity;
    }
    cout << "no containations named " << name <<" id: " << id;
    return 0L;
}


std::vector<Contamination*> ContaminationController::getContaminations()
{
    return contaminations_;
}
*/
/*
Histogram* ContaminationController::getTotalContamination()
{
    Histogram* totalContamination = Histogram::GetEmptyHistogram(0,10,10);
    std::vector <Contamination*>::iterator it;
    for(it = contaminations_.begin(); it != contaminations_.end(); ++it)
    {
//mk        float intensity = (*it)->normalization;
        totalContamination->Add((*it)->hist, (*it)->intensity);
    }
    return totalContamination;
}
*/
void ContaminationController::addContamination(QString expID, QString name, QString id, QString intensity)
{
    int expIDvalue = expID.toInt();
    std::string filename = name.toStdString();
    int idValue = id.toInt();
    float intensityValue = intensity.toFloat()/100.;
//    cout << filename<<" " << intensityValue << endl;
    Histogram* histtmp = new Histogram(filename,idValue);
    histtmp->Normalize(1.);    // We normalize to 1.0 so we have probability like distribution
/*    std::cout << "liczba zliczen w hist " << histtmp->GetNrOfCounts() << std::endl;
    std::cout << "filename: " <<filename <<std::endl;
    std::cout << "idValue: " <<idValue <<std::endl;
    std::cout << "intensityValue: " <<intensityValue <<std::endl;
    std::cout << "hist: " << histtmp <<std::endl;
*/
    Contamination* test = new Contamination(histtmp, filename, idValue, 1.0, intensityValue);
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "filename: " <<test->filename <<std::endl;
    std::cout << "idValue: " <<test->id <<std::endl;
    std::cout << "intensityValue: " <<test->intensity <<std::endl;
    std::cout << "Normalization: " <<test->normalization <<std::endl;
    std::cout << "histnr of counts: " << test->hist.GetNrOfCounts() <<std::endl;
    contaminations_.emplace_back( expIDvalue, *test );
    std::cout << "size: " << contaminations_.size() << std::endl;
    std::cout << "----------------------------------" << std::endl;

    delete test;
    delete histtmp;
}
/*
void ContaminationController::editContamination(QString name, QString id, QString intensity)
{
    std::string filename = name.toStdString();
    int idVal = id.toInt();
    float newIntensity = intensity.toFloat()/100.;
    std::vector <Contamination*>::iterator it;
    for(it = contaminations_.begin(); it != contaminations_.end(); ++it)
    {
        if( (*it)->filename == filename && (*it)->id == idVal)
        {
            (*it)->intensity = newIntensity;
///            (*it)->normalization = newIntensity;

//          myProject->setContaminations(contaminations_);
            return;

        }
    }
    cout << "ContaminationController::editContamination no containations named "
         << filename <<" id: " << idVal;
}

void ContaminationController::removeContamination(QString name, QString id)
{
    std::cout << "ContaminationController::removeContamination()" << endl;
    std::string filename = name.toStdString();
    int idVal = id.toInt();
    cout << filename << " : " << idVal <<endl;
    std::vector <Contamination*>::iterator it;
    for(it = contaminations_.begin(); it != contaminations_.end(); ++it)
    {
        if( (*it)->filename == filename && (*it)->id == idVal)
        {
            contaminations_.erase(it);
//            std::cout << "ContaminationController::removeContamination() Found to ERASE" << endl;
            Project *myProject = Project::get();
            myProject->setContaminations(contaminations_);
            return;
        }

    }
}

*/
