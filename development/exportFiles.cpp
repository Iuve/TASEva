#include "exportFiles.h"
#include "ui_exportFiles.h"
#include "project.h"
#include "histogram.h"
#include "contamination.h"
#include "histogramOutputController.h"
#include "responsefunction.h"
#include "PeriodicTable.hh"

//Qt INCLUDES
#include <QtCore/QTime>
#include <QDialog>
#include <QDir>
#include <QDialog>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

ExportFiles::ExportFiles(QWidget *parent) :
    QWidget(parent),
    uiEF(new Ui::ExportFiles)
{
   cout << "==================Export files===============" << endl;
    uiEF->setupUi(this);

    connect(uiEF->button_ExportHIS, SIGNAL(clicked(bool)), this, SLOT(slotExportHIS(bool)));
    connect(uiEF->buttonClose, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(uiEF->button_ExportASCII, SIGNAL(clicked(bool)), this, SLOT(slotExportASCII(bool)));
    connect(uiEF->checkBox_RespFuncRaw, SIGNAL(stateChanged(int)),this, SLOT(slotRespFunRaw(int)));
    connect(uiEF->checkBox_RespFunFitted, SIGNAL(stateChanged(int)),this, SLOT(slotRespFunFit(int)));
    connect(uiEF->buttonExpAll, SIGNAL(clicked(bool)),this, SLOT(slotExportAll(bool)));

    path_ = checkAndPreparePath();
    QString Qpath_ ;
    if(path_.size() < 40)
    {
    Qpath_ = QString::fromStdString(path_);
    } else {
    Qpath_ = QString::fromStdString(path_.substr(0,15)) + "..."  + QString::fromStdString(path_.substr(path_.size() - 25,path_.size()));
    }
    uiEF->labelEFMain->setText("To: " + Qpath_);
    DecayPath* decayPath_= DecayPath::get();
    if(!decayPath_)
    {
        int r = QMessageBox::warning(this, tr("Error"),
                                     tr("First load decay file"),
                                     QMessageBox::Ok);
        if (r == QMessageBox::Ok)
        {
            return;
        }
    }
}

ExportFiles::~ExportFiles()
{
    delete uiEF;
}

  void ExportFiles::slotRespFunRaw(int flag)
  {
 //     qDebug() << "FLAG: " << flag;
      if(flag==2)
      {
          slotExportResponseFunctions(); //checked
          uiEF->checkBox_RespFunFitted->setCheckState(Qt::Unchecked);
         uiEF->checkBox_RespFunFitted->setEnabled(false);
      }
      if(flag==0)                               // unchecked
      {
//         qDebug() << "in raw O: " << flag;
          respFunNorm_ = 0;
//          uiEF->checkBox_RespFuncRaw->setCheckState(Qt::Unchecked);
          uiEF->checkBox_RespFunFitted->setEnabled(true);
          return;
      }

  }

  void ExportFiles::slotRespFunFit(int flag)
  {
//      qDebug() << "FLAG: " << flag;
      if(flag==0)
      {
    //      slotExportResponseFunctions(); //checked
//          uiEF->checkBox_RespFunFitted->setCheckState(Qt::Unchecked);
         uiEF->checkBox_RespFuncRaw->setEnabled(true);
      }
      if(flag==2)                               // unchecked
      {
          respFunNorm_ = 0;
          uiEF->checkBox_RespFuncRaw->setCheckState(Qt::Unchecked);
          uiEF->checkBox_RespFuncRaw->setEnabled(false);
          return;
      }

  }
void ExportFiles::slotExportAll(bool flag)
{
    qDebug() << "Export files to: " << QString::fromStdString(path_) ;
    slotExportHIS(flag);
    slotExportASCII(flag);
    uiEF->buttonClose->click();
}
void ExportFiles::slotExportASCII(bool triggered)
{
    decayXMLFlag_ = uiEF->checkBox_ExportDecayXML->isChecked();
    decayENSFlag_ = uiEF->checkBox_ExportDecayENS->isChecked();
    decGenInfoFlag_ = uiEF->checkBox_GenDecInfo->isChecked();
    feedBGTFlag_ = uiEF->checkBox_Feeding->isChecked();
    gammaEvoFlag_ = uiEF->checkBox_GammaEvo->isChecked();

    if(decayXMLFlag_) exportXMLDecayFiles();
    if(decayENSFlag_) exportENSDecayFile();
    if(decGenInfoFlag_) exportDecayInfo();
    if(feedBGTFlag_) exportFeedingBGT();
    if(gammaEvoFlag_) exportGammaEvolution();
}

void ExportFiles::slotExportHIS(bool triggered)
{
// seting the envirorment
    expFlag_ = uiEF->checkBox_expSpec->isChecked();
    simFlag_ = uiEF->checkBox_SimSpec->isChecked();
    recFlag_ = uiEF->checkBox_RecSpec->isChecked();
    conFlag_ = uiEF->checkBox_Contaminations->isChecked();
    respFunFitFlag_ = uiEF->checkBox_RespFunFitted->isChecked();
    respFunRawFlag_ = uiEF->checkBox_RespFuncRaw->isChecked();
 // checking if any FALG_ is set, important for the ExportALL button
    if(expFlag_ || simFlag_ || recFlag_ || conFlag_ || respFunFitFlag_ || respFunRawFlag_)
    {
    Project* myProject =  Project::get();
    DecayPath* decayPath_= DecayPath::get();
    ResponseFunction* responseFunction_ = ResponseFunction::get();

  // getting file name from the user - might be changed to some default
    QString fileNameHIS = QFileDialog::getSaveFileName(this,
        tr("Save Simulated spectrum HIS file"), QString::fromStdString(path_),
        tr("HIS file (*.his)"));
    if(fileNameHIS=="")return;
    int p =  fileNameHIS.lastIndexOf(".his",-1, Qt::CaseInsensitive);
    qDebug() << "filename: =====" << fileNameHIS.left(p) ;
    fileNameHIS = fileNameHIS.left(p); //It is a full path not just a filename.

  // reading and setting flags

  // setting the normalisation for further export
//  if(respFunFitFlag_)respFunNorm_=0;
//  if(respFunRawFlag_)slotExportResponseFunctions();

// getting number of transitions i.e. response functions
  std::vector<Nuclide>* nuclides_ = decayPath_->GetAllNuclides();
  std::vector<Level>* motherLevels_ =nuclides_->at(0).GetNuclideLevels();
  Level *motherLevel_ = &(motherLevels_->at(0));
  std::vector<Transition*>* betaTransitions = motherLevels_->at(0).GetTransitions();

  int nrOfTransitions = betaTransitions->size();


  vector<string> expSpecIDVec_ = myProject->getExpSpecIDVec();

  std::vector< std::pair<int, Contamination> >* contaminations_;
  std::vector<Contamination> contaminationsSpecID_;
//  contaminationsSpecID_ = myProject->getContaminationsSpecID(histID)
  // caluclate number of 1D spectra
     //#exp+#sim+#cont for each exp+ #rec + #resp for exp
     set<int> HistIDList;

      for(int i=0; i!=expSpecIDVec_.size(); ++i)
      {
         if(expFlag_) HistIDList.insert(std::stoi(expSpecIDVec_.at(i)));  //exp spec from 6000
         if(simFlag_) HistIDList.insert(std::stoi(expSpecIDVec_.at(i))+1000); // sim spec  from 7000 up
         if(recFlag_) HistIDList.insert(std::stoi(expSpecIDVec_.at(i))+2000);  //rec spec from 8000 up
         if(conFlag_)
         {
             // to odpowiednia ilosc contaminacji
           contaminationsSpecID_ = myProject->getContaminationsSpecID(std::stoi(expSpecIDVec_.at(i)));
            int k;
           for(unsigned ii=0; ii!=contaminationsSpecID_.size(); ++ii)
            {
            k= std::stoi(expSpecIDVec_.at(i)) + 1 +ii;
            HistIDList.insert(k);
            }
         }
         if(respFunFitFlag_ || respFunRawFlag_)
         {
             // tu odpowiednia ilosc response functions
             int offset = 0;
             if(expSpecIDVec_.at(i) == "6300") offset=0;
             if(expSpecIDVec_.at(i) == "6310") offset=1000;
             if(expSpecIDVec_.at(i) == "6320") offset=2000;
             if(expSpecIDVec_.at(i) == "6330") offset=3000;
             if(expSpecIDVec_.at(i) == "6340") offset=4000;

             for(int i=1; i!=nrOfTransitions+1;++i)
             {
                 HistIDList.insert(i+offset);
             }
         }
      }
  HistogramOutputController *levelsOutController = new HistogramOutputController(fileNameHIS.toStdString(), HistIDList);
  std::map<int,Histogram> expHistMap_;

 QFile file; // file(fileNameHIS+".log");
  file.setFileName(fileNameHIS+".log");
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))return;

  QTextStream out(&file);
   out << "List of HistIDs saved to " << fileNameHIS<<".his" << "\n" ;
   out << "=====================================================================" << "\n";
   if(respFunFitFlag_ || respFunRawFlag_)
   {
       if(respFunFitFlag_)out << "      Response functions are multiplied by Beta feeding " <<"\n";
       if(respFunRawFlag_)out << "      Response functions RAW from GEANT simulations normalised to: "
                               << respFunNorm_ << " \n";
       out << "         1 - 1000 response functions for experimental spectrum 6300 " << "\n" ;
       out << "      1001 - 2000 response functions for experimental spectrum 6310 " << "\n" ;
       out << "      2001 - 3000 response functions for experimental spectrum 6320 " << "\n" ;
       out << "      3001 - 4000 response functions for experimental spectrum 6330 " << "\n" ;
       out << "      4001 - 5000 response functions for experimental spectrum 6330 " << "\n" ;
   }
   if(expFlag_) out << "      6300, 6310, 6320, 6330, 6340  -  experimental spectra (total,central,Inner,Middle,Outter)" << "\n";
   if(conFlag_)
   {
       out << "      6301 - 6309 Contaminations for 6300 exp. spectrum " << "\n";
       out << "      6311 - 6319 Contaminations for 6310 exp. spectrum " << "\n";
       out << "      6321 - 6329 Contaminations for 6320 exp. spectrum " << "\n";
       out << "      6331 - 6339 Contaminations for 6330 exp. spectrum " << "\n";
       out << "      6341 - 6349 Contaminations for 6340 exp. spectrum " << "\n";
   }
   if(simFlag_) out << "      7300, 7310, 7320, 7330, 7340  -  Simulated decay spectrum (no contaminations)" << "\n";
   if(recFlag_) out << "      8300, 8310, 8320, 8330, 8340  -  Reconstructed spectrum (decay + contaminations)" << "\n";
   out << "=====================================================================" << "\n";
    out <<" ";
   for (auto it : HistIDList) {
              out << it << "\n ";
          }



//  int end;
 // end = std::min(static_cast<int>(expSpecIDVec_.size()), myProject->getSimMapSize());
  for(unsigned long i=0; i < expSpecIDVec_.size(); i++)
  {
    int ID = std::stoi(expSpecIDVec_.at(i));
    cout << "-------new set of outputs for EXP spectrum: " << ID <<" ---------" << endl;
    if(expFlag_)
    {
//save exp spectra expHistMap_ from Project 6300,6310,6320,6330
        Histogram* tmp = myProject->getHistFromExpMap(ID);
       levelsOutController->saveHistogram(tmp,ID);
    }
    if(simFlag_)
    {
      //7300,7310,7320,7330
        int histID = ID + 1000;
        Histogram* tmp = myProject->getHistFromSimMap(ID);
       if(tmp != 0L)
       levelsOutController->saveHistogram(tmp,histID);
      //save sim spectrum, and create rest of sepctra adns save them
    }
    if(recFlag_)
    {    //8300.8310,8320,8330
        int histID = ID + 2000;
        Histogram* tmp = myProject->getHistFromRecMap(ID);
        levelsOutController->saveHistogram(tmp,histID);
      //save rec spectrum, and create rest of sepctra adn save them
    }
    if(conFlag_)
    {       //save rec spectrum, and create rest of sepctra adn save them
        //6301....6309; 6311...6319; 6321...6329; 6331...6339
        int k;
        int kl = 0;
//        contaminationsSpecID_ = myProject->getContaminationsSpecID(ID);
        std::vector< std::pair<int, Contamination> > contaminations = *(myProject->getContaminations());
        for (unsigned int ii = 0; ii !=  contaminations.size(); ii++)
        {
            if(contaminations.at(ii).first == ID)
            {
                ++kl;
                double fact = contaminations.at(ii).second.intensity * myProject->getExpHist()->GetNrOfCounts();
                Histogram *tmpCont = new Histogram(contaminations.at(ii).second.hist);
                fact= 1e6;
   // Przemyslec normalizacje zeby byla OK to pierwsze jest kiepskie a to drugie jeszcze bardzoej ;)
                tmpCont->Normalize(fact);
                k= ID + kl;
                qDebug() << "contamination k to be inserted : fact" << k << ":" << fact ;
                levelsOutController->saveHistogram(tmpCont,k);
                delete tmpCont;
            }
        }
     }


    if(respFunFitFlag_ || respFunRawFlag_)
    {
      //save rec spectrum, and create rest of sepctra adn save them
    int i=1;
    int ij=0;
    if(ID==6300)ij=0;
    if(ID==6310)ij=1000;
    if(ID==6320)ij=2000;
    if(ID==6330)ij=3000;
    if(ID==6340)ij=4000;
    int sumNormCont = 0; // normalizacja calkowita kontaminacji
    for (std::vector<Transition*>::iterator it = betaTransitions->begin(); it !=  betaTransitions->end(); it++)
    {
        Histogram* tmpHist = new Histogram(responseFunction_->GetLevelRespFunction( (*it)->GetPointerToFinalLevel(), ID ));
        double norm_ ; // = (*it)->GetIntensity() * myProject->getExpHist()->GetNrOfCounts() *(1-sumNormCont);
        if (respFunNorm_ != 0)
        {
          norm_ = respFunNorm_;
        } else
        {
            norm_ = (*it)->GetIntensity() * myProject->getExpHist()->GetNrOfCounts() *(1-sumNormCont);
        }
        tmpHist->Scale(norm_);
        levelsOutController->saveHistogram(tmpHist,i+ij);
        i=i+1;
    }
    }
  }
  }
}

void ExportFiles::slotExportResponseFunctions()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Normalisation"),
                                          tr("Normalisatio of the Response Functions"), QLineEdit::Normal,
                                          "100000", &ok);

    respFunNorm_ = text.toInt();

    if (ok && respFunNorm_ !=0)
     {
     } else {
         QMessageBox msgBox;
         msgBox.setText("Data not correct - norm set to 10e5 counts");
         msgBox.exec();
         return;
         respFunNorm_ = 10e5;
     }
}


string ExportFiles::checkAndPreparePath()
{
    QDateTime now = QDateTime::currentDateTime();
    const QString timestamp = now.toString(QLatin1String("yyyyMMdd-hhmm"));
    QDir newDir(timestamp);
//    qDebug() << newDir.exists();
//   if(!QDir::exists(timestamp))
//    {
    QDir().mkdir(timestamp);
//    qDebug() << timestamp;
//    }
    QDir currentDir = QDir::current();
    string currentPath = currentDir.absolutePath().toStdString();
    string path = currentPath + "/" + timestamp.toStdString();
    return path;
}
void ExportFiles::exportXMLDecayFiles()
{    
    SaveDecayData* outputXMLfiles = new SaveDecayData(path_);
    outputXMLfiles->SaveDecayStructure();
    delete outputXMLfiles;
}

void ExportFiles::exportDecayInfo()
{
//    QDir currentDir = QDir::current();
//    string currentPath = currentDir.absolutePath().toStdString();
    SaveDecayData* tempSaveDecayData = new SaveDecayData();
    tempSaveDecayData->SaveGeneralDecayInfo(path_);
}

void ExportFiles::exportFeedingBGT()
{

}

void ExportFiles::exportGammaEvolution()
{
    SaveDecayData* outputXMLfiles = new SaveDecayData(path_);
    outputXMLfiles->SaveGammaEvolution();
    delete outputXMLfiles;
}


void ExportFiles::exportENSDecayFile()
{
    SaveDecayData* outputXMLFiles = new SaveDecayData(path_);
    outputXMLFiles->SaveENSDecayStructure();
    delete outputXMLFiles;
}
/*    QFile file;
    file.setFileName(QString::fromStdString(path_)+"/Isotope.ens");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))return;
    QTextStream out(&file);
    DecayPath* decayPath = DecayPath::get();

    std::vector<Nuclide>* nuclides= decayPath->GetAllNuclides();

    QString parent;
    parent = setENSParentQString(nuclides->at(0));
    out << parent;
//========================== example
    double energia =12345.0;
    QString qstr = QString("%1\n").arg(energia,10,'g',2,'+');
    //arg dla double
    // precision liczba cyfr total nieliczac kropki np. 3 -> 1.23 ale 2-> 1.2
    // dla formatu g - foramt z exp np: e+2
    // dla formatu f precision oznacza liczbe miejsc po przecinku
    out << qstr;
    out << QString("%1\n").arg(energia,10,'f',2,'+');

   out << "=====================================================================" << "\n";


}
*/
QString ExportFiles::setENSParentQString(Nuclide parent)
{
    QChar space = ' ';
    QString qstr;
     for (int i=0; i<80; i++)qstr.push_back(space);
     qstr.push_back("\n");
    int atomicNumber = parent.GetAtomicNumber();
    int atomicMass = parent.GetAtomicMass();
    double qBeta = parent.GetQBeta();
    // GetSn();
    vector<Level>* levels = parent.GetNuclideLevels();
    double energy = levels->at(0).GetLevelEnergy();
    double T12 = levels->at(0).GetHalfLifeTime();
    QString unit = "S";
    int spin = int( levels->at(0).GetSpin()*2);
    QString parity = QString::fromStdString(levels->at(0).GetParity());
    QString qMass = QString("%1").arg(atomicMass,3,10);
    QString qstr4to5=QString::fromStdString(PeriodicTable::GetAtomicNameCap(atomicNumber));
    QString qstr10to19 = QString("%1").arg(energy,10,'f',2,space);
    QString qstr22to39 = QString("%1/2").arg(spin,14,10,space)+QString("%1").arg(parity,1,space)+space;
//    QString qstr22to39 = QString("%1").arg(spin,16,'f',1,space)+QString("%1").arg(parity,1,space)+space;
    QString qstr40to49 = QString("%1").arg(T12,7,'f',2,space)+QString("%1").arg(unit,2,space);
    QString qstr65to74 = QString("%1").arg(qBeta,10,'f',2,space);
    qstr.replace(0,5,qMass+qstr4to5); // Nuclide Identification
    qstr.replace(5,1,space);
    qstr.replace(6,1,space);
    qstr.replace(7,1,"P");   // P for Parent
    qstr.replace(8,1,space); //balnk or integer in case of multiple P records in the data set
    qstr.replace(9,10,qstr10to19); // energy
//    qstr.replace(19,2,qstr20to21); //denergy
    qstr.replace(21,18,qstr22to39); //spin and parity
    qstr.replace(39,10,qstr40to49); //Half-life with units
//    qstr.replace(49,6,qstr50to55); // dT1/2
//    qstr.replace(?,?,qstr56to64); // MUST be BLANK
    qstr.replace(64,10,qstr65to74);  // qBeta
//    qstr.replace(74,2,qstr75to76); // dQBeta
//    qstr.replace(76,4,qstr77to80); // IonizationState(for Ionized Atom decay) otherwise blank
    return qstr;
}
/*
QString ExportFiles::setENSLevelQString(Level level)
{
    QChar space = ' ';
    QString qstr;
     for (int i=0; i<80; i++)qstr.push_back(space);
     qstr.push_back("\n");
    int atomicNumber = parent.GetAtomicNumber();
    int atomicMass = parent.GetAtomicMass();
    double qBeta = parent.GetQBeta();
    // GetSn();
//    vector<Level>* levels = parent.GetNuclideLevels();
    double energy = level.GetLevelEnergy();
    double T12 = level.GetHalfLifeTime();
    QString unit = "S";
    int spin = int( level.GetSpin()*2);
    QString parity = QString::fromStdString(level.GetParity());
    QString qMass = QString("%1").arg(atomicMass,3,10);
    QString qstr4to5=QString::fromStdString(PeriodicTable::GetAtomicNameCap(atomicNumber));
    QString qstr10to19 = QString("%1").arg(energy,10,'f',2,space);
    QString qstr22to39 = QString("%1/2").arg(spin,14,10,space)+QString("%1").arg(parity,1,space)+space;
//    QString qstr22to39 = QString("%1").arg(spin,16,'f',1,space)+QString("%1").arg(parity,1,space)+space;
    QString qstr40to49 = QString("%1").arg(T12,7,'f',2,space)+QString("%1").arg(unit,2,space);
    QString qstr65to74 = QString("%1").arg(qBeta,10,'f',2,space);
// output string construction
    qstr.replace(0,5,qMass+qstr4to5); // Nuclide Identification
    qstr.replace(5,1,space);
    qstr.replace(6,1,space);
    qstr.replace(7,1,"L");   // L for Level
    qstr.replace(8,1,space); //balnk or integer in case of multiple P records in the data set
    qstr.replace(9,10,qstr10to19); // energy
//    qstr.replace(19,2,qstr20to21); //denergy
    qstr.replace(21,18,qstr22to39); //spin and parity
    qstr.replace(39,10,qstr40to49); //Half-life with units
//    qstr.replace(49,6,qstr50to55); // dT1/2
//    qstr.replace(?,?,qstr56to64); // L Angular momentum transfer in the reacition determianinf the data set.
//    qstr.replace(64,10,qstr65to74);  // S Spectroscopiv strength for this level as determined from the reaction
//    qstr.replace(74,2,qstr75to76); // dS
//      qstr.replace(76,1,"C") ;      // Comment  FLAG used to refer to a particular comment record
//    qstr.replace(77,2,qstr78to79); // Metastabel state is denoted by 'M' or "M1' for the first (lowest) M2, for the second etc.
                                    // For ionized atom decay filed gives the atomic electron shell or subshell in which B- particle is captured.
//    qstr.replace(79,1,qstr80); // The character '?' denotes an uncertain or questionable level.Letter 'S' denotes neutron,proton,alpha,separation
                                 //energy or a leel expected but not observed.
    return qstr;
}
*/
/*
QString ExportFiles::setENSGammaQString(Transition transition)
{
    QChar space = ' ';
    QString qstr;
     for (int i=0; i<80; i++)qstr.push_back(space);
     qstr.push_back("\n");
    int atomicNumber = parent.GetAtomicNumber();
    int atomicMass = parent.GetAtomicMass();
    // GetSn();
    double energy = transition.GetLevelEnergy();
    QString unit = "S";
    int spin = int( levels->at(0).GetSpin()*2);
    QString parity = QString::fromStdString(levels->at(0).GetParity());
    QString qMass = QString("%1").arg(atomicMass,3,10);
    QString qstr4to5=QString::fromStdString(PeriodicTable::GetAtomicNameCap(atomicNumber));
    QString qstr10to19 = QString("%1").arg(energy,10,'f',2,space);
    QString qstr22to39 = QString("%1/2").arg(spin,14,10,space)+QString("%1").arg(parity,1,space)+space;
//    QString qstr22to39 = QString("%1").arg(spin,16,'f',1,space)+QString("%1").arg(parity,1,space)+space;
    QString qstr40to49 = QString("%1").arg(T12,7,'f',2,space)+QString("%1").arg(unit,2,space);
    QString qstr65to74 = QString("%1").arg(qBeta,10,'f',2,space);
    qstr.replace(0,5,qMass+qstr4to5); // Nuclide Identification
    qstr.replace(5,1,space);
    qstr.replace(6,1,space);
    qstr.replace(7,1,"G");   // G for gamma
    qstr.replace(8,1,space); //must be blank
    qstr.replace(9,10,qstr10to19); // energy
//    qstr.replace(19,2,qstr20to21); //denergy
//    qstr.replace(21,18,qstr22to29); //RI, relative photon intensity I
//    qstr.replace(29,2,qstr30to31); //dRI
//    qstr.replace(31,10,qstr32to41); //Multipolarity of transition
//    qstr.replace(41,8,qstr42to49); // MR Mixing ratio (sigma) (Sign must be shown explicitly if known.
                                    // If no sign is given it will be assumed to be unknown.)
//    qstr.replace(49,6,qstr50to55); // dMR, Standard uncertainty of Mixing (MR)
//    qstr.replace(55,7,qstr56to62); // CC, Total conversion coeficient
//    qstr.replace(62,2,qstr63to64); // dCC
    qstr.replace(64,10,qstr65to74);  // TI, Relative total transition intensity. (Normalisation given in NORMALISATION record)
//    qstr.replace(74,2,qstr75to76); // dTI*----
//    qstr.replace(76,1,qstr77); // Comment FLAG used to refere to particular comment record.
                              // The symbol '*' denotes multiply placed g-ray.
                              // '&' - a multiplaced transitions with intensity not divided,
                              // '@' - a multiplaced transitions with intensity suitably divided.
                              // '%' - denotes that intensity given as RI is the branching in the SUper DEformed Band.
    qstr.replace(77,1,' ');  //  Letter 'C' denotes placement confirmed by coincidence.
                              // Symbol '?' denotes questionable coincidence.
    qstr.replace(78,1,space);
    qstr.replace(79,1,qstr80); // The Character '?' denotes an uncertain placement of the transition in the lvel scheme.
                               // Letter 'S' denotes an expected, but as yet unobserved, transition.
    return qstr;
}

*/
