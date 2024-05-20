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
    connect(uiEF->button_ExportDAT, SIGNAL(clicked(bool)), this, SLOT(slotExportDAT(bool)));
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
  void ExportFiles::slotDATRespFunRaw(int flag)
  {
      if(flag==2)
      {
          slotExportResponseFunctions(); //checked
          uiEF->checkBox_DATRespFunFitted->setCheckState(Qt::Unchecked);
         uiEF->checkBox_DATRespFunFitted->setEnabled(false);
      }
      if(flag==0)                               // unchecked
      {
          respFunDATNorm_ = 0;
          uiEF->checkBox_DATRespFunFitted->setEnabled(true);
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
  void ExportFiles::slotDATRespFunFit(int flag)
  {
      if(flag==0)
      {
         uiEF->checkBox_DATRespFuncRaw->setEnabled(true);
      }
      if(flag==2)                               // unchecked
      {
          respFunDATNorm_ = 0;
          uiEF->checkBox_DATRespFuncRaw->setCheckState(Qt::Unchecked);
          uiEF->checkBox_DATRespFuncRaw->setEnabled(false);
          return;
      }

  }
void ExportFiles::slotExportAll(bool flag)
{
    qDebug() << "Export files to: " << QString::fromStdString(path_) ;
    slotExportHIS(flag);
    slotExportASCII(flag);
    slotExportDAT(flag);
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
void ExportFiles::slotExportDAT(bool triggered)
{
    cout << "ExportFiles::slotExportDAT"  << endl;
    // seting the envirorment
        expFlag_ = uiEF->checkBox_DATexpSpec->isChecked();
        simFlag_ = uiEF->checkBox_DATSimSpec->isChecked();
        recFlag_ = uiEF->checkBox_DATRecSpec->isChecked();
        conFlag_ = uiEF->checkBox_DATContaminations->isChecked();
        respFunFitFlag_ = uiEF->checkBox_DATRespFunFitted->isChecked();
        respFunRawFlag_ = uiEF->checkBox_DATRespFuncRaw->isChecked();
        Project* myProject =  Project::get();
        DecayPath* decayPath_= DecayPath::get();
        ResponseFunction* responseFunction_ = ResponseFunction::get();
        std::vector< std::pair<int, Contamination> > contaminations = *(myProject->getContaminations());

        vector<string> expSpecIDVec_ = myProject->getExpSpecIDVec();
        std::vector< std::pair<int, Contamination> >* contaminations_;
        std::vector<Contamination> contaminationsSpecID_;

        bool ok1;
        int binFactor = 1;
        QString qstr_bin = QString::number(binFactor);
        QString qtmp = "Please set a binning factor for " ;
        QString text = QInputDialog::getText(this, tr("ASCI data to be written to disk"),
                                              qtmp, QLineEdit::Normal,
                                              qstr_bin , &ok1);


        if (ok1 && !text.isEmpty())
            binFactor = text.toInt();

        qDebug() << "Binning factor = " << binFactor;

        if(expFlag_ || simFlag_ || recFlag_ || conFlag_ )
        {
            for(int i=0; i!=expSpecIDVec_.size(); ++i)
            {
                int ID = std::stoi(expSpecIDVec_.at(i));
                cout << "simFlag_ = " << myProject->checkForKeySim(ID) << endl;
        //        simFlag_ = myProject->checkForKeySim(ID);
                recFlag_ = myProject->checkForKeyRec(ID);
                if ((simFlag_)&(!myProject->checkForKeySim(ID)))
                {
                  QMessageBox msgBox;
                  QString qtext;
                  qtext = "There is no simulation spectrum for ID = " + QString::number(ID);
                  msgBox.setText(qtext);
                  msgBox.setInformativeText("Spectra will be exported without simulated one.");
                msgBox.setStandardButtons(QMessageBox::Ok );
                msgBox.setDefaultButton(QMessageBox::Ok);
                msgBox.exec();
                }

                string outputFilename;
                if(binFactor == 1)
                {
                outputFilename = path_ + "/"+ expSpecIDVec_.at(i) +"_export.dat";
                } else {
                    outputFilename = path_ + "/"+ expSpecIDVec_.at(i) +"_export_" + binFactor +"bin.dat";
                }
                ofstream outputFile(outputFilename.c_str());
                if (!outputFile.is_open())
                    cout << "Warning message: The file " + (string) outputFilename + " is not open!" << endl;


                //     -----------Header---------
                string tmpline;
                tmpline = "#Energy (keV) |";
                if(expFlag_) tmpline = tmpline + "Exp |";
                if(simFlag_) tmpline = tmpline + "Sim |";
                if(recFlag_) tmpline = tmpline + "Rec |";
                if(conFlag_)
                {
                    int kl = 0;
 //                   std::vector< std::pair<int, Contamination> > contaminations = *(myProject->getContaminations());
                    for (unsigned int ii = 0; ii !=  contaminations.size(); ii++)
                    {
                        if(contaminations.at(ii).first == ID)
                        {
                            ++kl;
                            tmpline = tmpline + "Cont_"+ kl + " |";
                            double fact = contaminations.at(ii).second.intensity * myProject->getExpHist()->GetNrOfCounts();
                            outputFile << "#Normalisation for Cont_" + to_string(kl) + "is = " + fact << endl;
                     }
                    }
                 }
                outputFile << tmpline << endl;
 //   ----------END of Header-------------
                Histogram expHist, simHist, recHist;
                expHist = Histogram(myProject->getHistFromExpMap(ID));
                double normMin = myProject->getNormMin();
                double normMax = myProject->getNormMax();
                double norm = expHist.GetNrOfCounts(normMin,normMax);
                qDebug() << "tu 1-";
                if(binFactor != 1)expHist.Rebin(binFactor);
                qDebug() << "tu 2-";
                if(simFlag_) {simHist = Histogram(myProject->getHistFromSimMap(ID));
                qDebug() << "tu 2a-";
                if(binFactor != 1)simHist.Rebin(binFactor);
                }
                 qDebug() << "tu 3-";
                if(recFlag_)
                {
                    recHist = Histogram(myProject->getHistFromRecMap(ID));
                    // MS 20240520 REC normalization
                    recHist.Normalize(norm,normMin,normMax);
                    if(binFactor != 1)
                        recHist.Rebin(binFactor);
                }
                 qDebug() << "tu 4-";
                double xMax = expHist.GetXMax();
                double xMin = expHist.GetXMin();
//                int xMin = 470;
//                int xMax = 14000;
                vector<double> En = expHist.GetEnergyVectorD();
                vector<double> exp = expHist.GetAllDataD();
                vector <double> sim, rec;
                if(simFlag_) sim = simHist.GetAllDataD();
                if(recFlag_) rec = recHist.GetAllDataD();
                vector< vector<double> > cont;
                if(conFlag_)
                {
//                    cout << "contaminations.size() = " << contaminations.size() << endl;
                    for (unsigned int ii = 0; ii !=  contaminations.size(); ii++)
                    {
                        if(contaminations.at(ii).first == ID)
                        {
                            double fact = contaminations.at(ii).second.intensity * myProject->getExpHist()->GetNrOfCounts();
                            Histogram *tmpCont = new Histogram(contaminations.at(ii).second.hist);
                            tmpCont->Normalize(fact);
                            if(binFactor != 1)tmpCont->Rebin(binFactor);
                            cont.push_back(tmpCont->GetAllDataD());
                            delete tmpCont;
                         }
                     }
                }
                 string tmpline2;
                 cout << "xMin,xMax " << xMin << " " << xMax << endl;


                 bool ok;
                 QString qstr_ID = QString::number(ID);
                 QString qstr_xMax = QString::number(xMax*binFactor);
                 QString qtmp = "Please set xMax for  spectrum " + qstr_ID;
                 QString text = QInputDialog::getText(this, tr("ASCI data to be written to disk"),
                                                       qtmp, QLineEdit::Normal,
                                                       qstr_xMax , &ok);


                 if (ok && !text.isEmpty())
                     xMax =text.toInt()/binFactor;



                for(int i=xMin; i !=xMax; i++)
                {
                    tmpline2 = to_string(En.at(i)) + "  " ;
                    if(expFlag_)tmpline2 = tmpline2 + to_string(exp.at(i)) + "  " ;
                   if(simFlag_)
                    {
                        string value;
                        if(i >= sim.size())
                        {
                            value = "0" ;
                        } else {
                            value = to_string(sim.at(i));
                        }
                        tmpline2 = tmpline2 + value  + "  " ;
                    }
                    if(recFlag_)tmpline2 = tmpline2 + to_string(rec.at(i)) + "  " ;
                    if(conFlag_)
                    {
                        for(int ic=0; ic!=cont.size(); ic++)
                        {
                            tmpline2 = tmpline2 + to_string(cont.at(ic).at(i)) + " " ;
                        }
                    }
                  outputFile << tmpline2 << endl;
                } outputFile.close();
            }

        }

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
/*    qDebug()<< "1203.1234, 0.0123 " << outputXMLFiles->getValueAndError(1203.1234, 0.0123);
    qDebug()<< "0.0123, 0.00123 " << outputXMLFiles->getValueAndError(0.0123, 0.00123);
    qDebug()<< "13.1, 12.3" << outputXMLFiles->getValueAndError(13.1, 12.3);
    qDebug()<< "13.1, 1.3" << outputXMLFiles->getValueAndError(13.1, 1.3);
    qDebug()<< "13.1, 16.3" << outputXMLFiles->getValueAndError(13.1, 16.3);
*/
}

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


