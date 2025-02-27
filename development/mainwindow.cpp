#include <DeclareHeaders.hh>
#include "mainwindow.h"
#include "contaminationController.h"
#include "pileupController.h"
#include "fitController.h"
#include "responsefunction.h"
#include "histogramgraph.h"
#include "histogramOutputController.h" 
//#include "exportFiles.h"  decalred in mainwindow.h

#include "tablecontroller.h"
#include "PeriodicTable.hh"

// GUI includes
#include "ui_mainwindow.h"
#include "ui_histogramgraph.h"
#include "ui_pileupController.h"
#include "ui_DecayPathEditor.h"
#include "ui_analysis2d.h"
#include "ui_status.h"
#include "ui_manualfitgraph.h"
#include "ui_tableinput.h"
#include "ui_exportFiles.h"

// Qt INCLUDES
#include <QApplication>
#include <QtWidgets/QMainWindow>
#include <QPushButton>
#include <QCheckBox>
#include <QProgressBar>
#include <QSlider>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QScatterSeries>
#include <QtCore/QTime>
#include <QDialog>
#include <QInputDialog>

// Other includes

#include <string>
#include <iostream>

class DialogOptionsWidget;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QMessageBox msgBox;
    msgBox.setText("Did you remember of setting G4LEDATA Geant4 system variable?");
    msgBox.exec();

    //Initialization
    t1 = new DecayPathEditor();
    m1 = new ManualFitGraph();

    levelEditorOpen_ = false;
    projectOpen_ = false;
    s_ui = 0L;
    fittingMethod_ = 0;
    SetcomboBoxFit();


    slotUpdateProjectPanel(true);

    //Mouse right click actions contamination table
        ui->frameContaminationPanel->setContextMenuPolicy(Qt::ActionsContextMenu);
        QAction* actionAddContamination = new QAction("Add contamination");
        QAction* actionRemoveContamination = new QAction("Remove contamination");
        connect(actionAddContamination, SIGNAL(triggered()), this, SLOT(slotAddContamination()));
        connect(actionRemoveContamination, SIGNAL(triggered()), this, SLOT(slotRemoveContamination()));
        ui->frameContaminationPanel->addAction(actionAddContamination);
        ui->frameContaminationPanel->addAction(actionRemoveContamination);

    //Mouse right click acctions END on contamination table

    ui->actionStatus->setEnabled(false);
    //Menu Bar
//    connect(ui->loadInputENS, SIGNAL(triggered(bool)), this, SLOT(loadENSFile(bool)));
    connect(ui->actionNewProject, SIGNAL(triggered(bool)), this, SLOT(newProject(bool)));
    connect(ui->actionOpenProject, SIGNAL(triggered(bool)), this, SLOT(openProject(bool)));
    connect(ui->actionSaveProject, SIGNAL(triggered(bool)), this, SLOT(saveProject(bool)));

    connect(ui->loadDecay, SIGNAL(triggered(bool)), this, SLOT(loadDecayFile(bool)));
    connect(ui->loadInputHIS, SIGNAL(triggered(bool)), this, SLOT(loadHISFile(bool)));
//Calibratiom
    connect(ui->actionCalEnergy , SIGNAL(triggered(bool)), this, SLOT(slotEnergyCalibration(bool)));
    connect(ui->actionCalFWHM, SIGNAL(triggered(bool)), this, SLOT(slotFWHMCalibration(bool)));
    connect(ui->actionCompare_Spectra, SIGNAL(triggered(bool)),this,SLOT(slotCompareSpectra(bool)));
    ui->actionCompare_Spectra->setEnabled(false);
    //Status
    connect(ui->actionStatus, SIGNAL(triggered(bool)), this, SLOT(slotShowStatus(bool)));

    connect(ui->actionExport_Files, SIGNAL(triggered(bool)),this,SLOT(slotExportFiles(bool)));
    connect(ui->actionCalculate_Uncertainties, SIGNAL(triggered(bool)),this,SLOT(slotCalculate_Uncertainties(bool)));


 //   connect(ui->export_ENS_File, SIGNAL(triggered(bool)), this, SLOT(exportENSFile(bool)));
    //contaminations
    connect(ui->actionSignal_Signal, SIGNAL(triggered(bool)), this, SLOT(slotPileupSignalSignal()));
    connect(ui->actionSignal_Bkg, SIGNAL(triggered(bool)), this, SLOT(slotPileupSignalBackground()));
    connect(this, SIGNAL(signalUpdateContaminationPanel()), this, SLOT(slotUpdateContaminationPanel()));

 // Project data panel
    connect(ui->lineProjectName, SIGNAL(returnPressed()), this, SLOT(slotProjectNameUpdate()));
    connect(ui->lineWorkingDir, SIGNAL(returnPressed()), this, SLOT(slotProjectWorkingDirUpdate()));
    connect(ui->lineExpFile, SIGNAL(returnPressed()), this, SLOT(slotProjectExpFileUpdate()));
    connect(ui->lineExpFile, SIGNAL(returnPressed()), this, SLOT(slotUpdateSpecPlot()));  // not working expHis not read in updateslot

//    connect(ui->lineExpSpecID, SIGNAL(returnPressed()), this, SLOT(slotProjectExpSpecIDUpdate()));
    connect(ui->comboBox_ExpSpecID, SIGNAL(activated(int)), this, SLOT(slotProjectExpSpecIDUpdate(int)));

    connect(ui->lineExp2DSpecID, SIGNAL(returnPressed()), this, SLOT(slotProjectExp2DSpecIDUpdate()));
    connect(ui->lineInputDecayFile, SIGNAL(returnPressed()), this, SLOT(slotProjectInputDecayFileUpdate()));
    connect(ui->lineOutputDecayFile, SIGNAL(returnPressed()), this, SLOT(slotProjectOutputDecayFileUpdate()));
    connect(ui->lineOutputSIMFile, SIGNAL(returnPressed()), this, SLOT(slotProjectOutputSIMFileUpdate()));
    connect(ui->lineOutputLEVFile, SIGNAL(returnPressed()), this, SLOT(slotProjectOutputLEVFileUpdate()));
    connect(ui->lineCodeGEANT, SIGNAL(returnPressed()), this, SLOT(slotProjectCodeGEANTUpdate()));

    //SpecPLot
    connect(ui->specPlot, SIGNAL(selectionChangedByUser()), this, SLOT(slotSelectionChanged()));

    //Buttons
    connect(ui->buttonClose, SIGNAL(clicked()), QApplication::instance(), SLOT (quit()));

    // Response function panel
//    connect(ui->buttonShowNNDCLevelSchemePDF, SIGNAL(clicked()), SLOT(showLevelSchemePDF()));
    ui->buttonEditLevelScheme->setEnabled(false);
    connect(ui->buttonEditLevelScheme, SIGNAL(clicked(bool)), SLOT(slotEditLevelSchemeClicked(bool)));
    connect(t1->uiT->buttonClose, SIGNAL(clicked(bool)), this , SLOT(CloseLevelSchemeEditor(bool)));
    connect(t1, SIGNAL(signalUpdateTables()), this, SLOT(slotUpdateLevelEditor()));
    connect(t1, SIGNAL(signalDecayPathEdited()), this, SLOT(slotDecayPathEdited()));

    connect(t1->uiT->buttonClose, SIGNAL(clicked(bool)), this, SLOT(slotEditLevelSchemeClicked(bool)));

    ui->comboBox_ExpSpecID->setEnabled(false);  //disabling this input line before the check simulations is not done for a default spectrum.
    ui->buttonMakeDirsAndCheckFiles->setEnabled(false);
    ui->buttonMakeSimulations->setEnabled(false);
    ui->buttonUploadAndCalculateResponse->setEnabled(false);
    ui->buttonCalculateSIMSpec->setEnabled(false);
    ui->buttonUpdateResponseContainer->setEnabled(false);
    ui->frameFit->setEnabled(false);
    ui->frameFitParam->setEnabled(false);
    connect(ui->buttonMakeDirsAndCheckFiles, SIGNAL(clicked()), this, SLOT(slotMakeDirsAndCheckFiles() ) );
    connect(ui->buttonMakeSimulations, SIGNAL(clicked(bool)), this, SLOT(slotMakeSimulations()));
    connect(ui->buttonUploadAndCalculateResponse, SIGNAL(clicked()), this, SLOT(slotUploadAndCalculateResponse() ) );
    connect(ui->buttonCalculateSIMSpec,SIGNAL(clicked()),SLOT(slotCalculateDECSpectrum()));
    connect(this,SIGNAL(signalUpdateDecSpec()), this, SLOT(slotCalculateDECSpectrum()));
    connect(ui->buttonUpdateResponseContainer,SIGNAL(clicked()),SLOT(slotUpdateResponseContainer()));

    // Spectrum display panel
    connect(this, SIGNAL(signalUpdateSpecPlot()), this, SLOT(slotUpdateSpecPlot()));
    connect(ui->MainPlotLogScale, SIGNAL(clicked(bool)), this, SLOT(slotSetMainLogScale(bool)));
    connect(ui->lineEditXAxisMax, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(ui->lineEditXAxisMin, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(ui->lineEditYAxisMax, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(ui->lineEditYAxisMin, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(ui->lineEditNormMin, SIGNAL(returnPressed()),this,SLOT(slotSetNormRange()));
    connect(ui->lineEditNormMax, SIGNAL(returnPressed()),this,SLOT(slotSetNormRange()));

// Contaminations Panel
    connect(ui->tableContamination,SIGNAL(cellChanged(int,int)), SLOT(slotContaminationTableChanged(int,int)));
    connect(ui->tableContamination,SIGNAL(cellClicked(int,int)), SLOT(slotContaminationTableClicked(int,int)));

//    connect(ui->tableContamination,SIGNAL(cellChanged(int,int)), SLOT(slotUpdateContaminationData(int,int)));


    //2D Analysis panel
    connect(ui->buttonAnalysis2D, SIGNAL(clicked()), this, SLOT(slotOpen2DAnalysis()));
    connect(ui->comboBoxFit1D, SIGNAL(activated(QString)), this, SLOT(slot1DFittingMethod(QString)));

    connect(m1, SIGNAL(signalRecalculateLevel()), this, SLOT(slotCalculateDECSpectrum()));
//    connect(this, SIGNAL(signalUpdateSpecPlot()), m1, SLOT(takesMySignal()));
    connect(this, SIGNAL(signalUpdateSpecPlot()), this, SLOT(updateHistogram()));

    //Fitting Panel
    ui->buttonAutoFit->setEnabled(false);
    ui->buttonErrorCal->setEnabled(false);
    ui->buttonAnalysis2D->setEnabled(false);
    ui->buttonManualFit->setEnabled(false);
    ui->lineEditFitBinning->setText("10");
    connect(ui->buttonAutoFit, SIGNAL(clicked(bool)), this, SLOT(slotAutoFit()));
    connect(ui->buttonManualFit, SIGNAL(clicked(bool)), this, SLOT(slotManualFit()));
    connect(ui->lineEditFitEnergyFrom, SIGNAL(returnPressed()), this, SLOT(slotProjectFitEnergyFrom()));
    connect(ui->lineEditFitEnergyTo, SIGNAL(returnPressed()), this, SLOT(slotProjectFitEnergyTo()));
    connect(ui->lineEditFitLevelsFrom, SIGNAL(returnPressed()), this, SLOT(slotProjectFitLevelsFrom()));
    connect(ui->lineEditFitLevelsTo, SIGNAL(returnPressed()), this, SLOT(slotProjectFitLevelsTo()));
    connect(ui->lineEditFitLambda, SIGNAL(returnPressed()), this, SLOT(slotProjectFitLambda()));
    connect(ui->lineEditNoFitIterations, SIGNAL(returnPressed()), this, SLOT(slotProjectNoFitIterations()));
    connect(ui->lineEditFitBinning, SIGNAL(returnPressed()), this, SLOT(slotProjectFitBinning()));

    connect(ui->lineEditFitEnergyFrom, SIGNAL(editingFinished()), this, SLOT(slotProjectFitEnergyFrom()));
    connect(ui->lineEditFitEnergyTo, SIGNAL(editingFinished()), this, SLOT(slotProjectFitEnergyTo()));
    connect(ui->lineEditFitLevelsFrom, SIGNAL(editingFinished()), this, SLOT(slotProjectFitLevelsFrom()));
    connect(ui->lineEditFitLevelsTo, SIGNAL(editingFinished()), this, SLOT(slotProjectFitLevelsTo()));
    connect(ui->lineEditFitLambda, SIGNAL(editingFinished()), this, SLOT(slotProjectFitLambda()));
    connect(ui->lineEditNoFitIterations, SIGNAL(editingFinished()),this, SLOT(slotProjectNoFitIterations()));
    connect(ui->lineEditFitBinning, SIGNAL(editingFinished()), this, SLOT(slotProjectFitBinning()));
    //    connect(ui->lineEditFitHistId, SIGNAL(returnPressed()), this, SLOT(slotProjectExpSpecIDUpdate()));
    connect(ui->openHistogramGraph, SIGNAL(clicked()), this, SLOT(openHistogramGraph()));

}

MainWindow::~MainWindow()
{
    delete mainInstance;
    mainInstance = NULL;
    m1->close();
    //delete m1;
    t1->close();
    delete t1;
    delete ui;
    //delete a2D_ui;

}

MainWindow* MainWindow::mainInstance = NULL;
MainWindow* MainWindow::GetInstance(QWidget *parent)
{
    if(mainInstance == NULL)
    {
        mainInstance = new MainWindow(parent);
    }
    std::cout << "MAINWINDOW INSTANCE  w kreowaniu" << mainInstance <<std::endl;
    return mainInstance;
}


void MainWindow::createDecayInstance(const string DecayFileName)
{
     std::cout<< "Creating decay Instance..." << DecayFileName<<std::endl;
/*     std::cout << decayPath << std::endl;
     if(decayPath != 0L)
     {
         delete decayPath;
     }
*/
     DecayPath* decayPath= DecayPath::get();
     decayPath->LoadDecay(DecayFileName);
     ui->actionStatus->setEnabled(true);
     ui->buttonEditLevelScheme->setEnabled(true);
     ui->buttonMakeDirsAndCheckFiles->setEnabled(true);
     ui->buttonMakeSimulations->setEnabled(false);
     ui->buttonUploadAndCalculateResponse->setEnabled(false);
     ui->buttonCalculateSIMSpec->setEnabled(false);
     ui->buttonUpdateResponseContainer->setEnabled(false);
     ui->actionCompare_Spectra->setEnabled(false);
   qDebug() << "QValue: "<< decayPath->GetAllNuclides()->at(0).GetNuclideLevels()->at(0).GetTransitions()->at(0)->GetTransitionQValue() ;
}



void MainWindow::slotEnergyCalibration(bool trigered)
{
   calE = new CalibrateEnergy();
//   cal_ui->show();
}

void MainWindow::slotFWHMCalibration(bool trigered)
{
  qDebug() << "NOT IMPLEMENTED YET" ;
}


void MainWindow::slotCompareSpectra(bool trigered)
{
    specComp_ui = new ManualFitGraph();
    specComp_ui->setResponseType("c"); //compare spectra
    specComp_ui->uiM->buttonNormalise->setText("Add spectrum");
    specComp_ui->uiM->buttonRecalculate->setText("checked");
    specComp_ui->uiM->buttonRecalculate->hide();
    specComp_ui->uiM->checkContrOtherLevels->hide();
    specComp_ui->uiM->labelNoCounts->hide();
    QStringList header;
    header << "Display ?" << "Spec ID" << " " << "File Name" ;
    specComp_ui->ManualFitGraph::setHeader(header);
    specComp_ui->uiM->tableLevels->hideColumn(2);
    specComp_ui->show();


    Project* myProject = Project::get();
    std::string expFileName = myProject->getExpFile();
    std::vector<string> expID = myProject->getExpSpecIDVec();
//simID to be filled based on the available data in the memory.
   std::vector<string> simID = myProject->getExpSpecIDVec(); // so simID corresponds to ExpID

//---------Table-------------

    std::vector <bool> displayStatus;
    std::vector<RowData> rowData_;


    DecayPath* decayPath= DecayPath::get();
    QString QEmpty = " ";

//assuming one isotopes = nuclides
    std::vector <Level>* motherLevels = decayPath->GetAllNuclides()->at(0).GetNuclideLevels();
    std::vector<Transition*>transitionsUsed;
// for experimental spectra
    for(unsigned int i = 0; i != expID.size(); ++i)
    {
    QString QDisplayStatus_ = "true";
    QString QID_ = QString::fromStdString(expID.at(i));
//    QString QSource_ = QString::fromStdString(expFileName);
    QString QSource_ = "ExpMap";
    rowData_.push_back(RowData(QDisplayStatus_, QID_, QEmpty, QSource_));
    }
// for simulated spectra
    for(unsigned int i = 0; i != simID.size(); ++i)
    {
    QString QDisplayStatus_ = "true";
    QString QID_ = QString::fromStdString(simID.at(i));
    QString QSource_ = "SimMap";
    rowData_.push_back(RowData(QDisplayStatus_, QID_, QEmpty, QSource_));
    }
    specComp_ui->ManualFitGraph::initializeTable(rowData_);

    //-------end of Table related code ---

    QVector<double> x = QVector<double>::fromStdVector(myProject->getExpHist()->GetEnergyVectorD());
    QVector<double> y1 = QVector<double>::fromStdVector(myProject->getExpHist()->GetAllDataD());
    QVector<double> y2 = QVector<double>::fromStdVector(myProject->getRecHist()->GetAllDataD());
    QVector<double> d12 = QVector<double>::fromStdVector(myProject->getDifHist()->GetAllDataD());


    string respType = "c";
    specComp_ui->setResponseType(respType);
    specComp_ui->initializeGraphs();
    specComp_ui->setXValues(x);  //sets X value vector so rang is correctly calculated
//    specComp_ui->setYValues(y1);
//    specComp_ui->setY2Values(y2);
//   specComp_ui->setDiffValues(d12);
    //    specComp_ui->showDataExpSimDiff(x, y1, y2, d12);
    specComp_ui->showResponseFunctions();
//-----spectra display code ------

    specComp_ui->show();
}

void MainWindow::slotShowStatus(bool trigered)
{
    qInfo() << "MainWindow::slotShowStatus" ;
    if(!trigered)
    {
        if(s_ui != 0L) delete s_ui;
        s_ui = new Status();
        s_ui->show();
    }
}

void MainWindow::slotOpen2DAnalysis()
{
    a2D_ui = new Analysis2D();
    a2D_ui->show();
}

 void MainWindow::slotAddContamination()
{
    qInfo() << "MainWindow::slotAddContamination" ;
     QString qExpId;
    QString qFileName;
    QString qHistId;
    QString qIntensity;
    bool ok;
     QString text = QInputDialog::getText(this, tr("New Contamination"),
                                           tr("ExpID : FileName : HistID : Intensity(%)"), QLineEdit::Normal,
                                           "ExpID : FileName : HistID : Intensity", &ok);
     QStringList stringList= text.split(":",QString::SkipEmptyParts);
//     for (int i = 0; i < stringList.size(); i++) qDebug(stringList.at(i).toUtf8());
//     qDebug()<<stringList;
     if(ok && stringList.size() == 4)
     {
         qExpId = stringList.at(0);
         qFileName = stringList.at(1);
         qHistId = stringList.at(2);
         qIntensity = stringList.at(3);
     } else
     {
         QMessageBox msgBox;
         msgBox.setText("Data not correct - no changes applied");
         msgBox.exec();
         return;
     }
    ContaminationController *contaminationController = new ContaminationController();
    contaminationController->addContamination(qExpId, qFileName, qHistId, qIntensity, 0);
    contaminationController->SaveAll();
    delete contaminationController;

    slotUpdateContaminationPanel();
    slotCalculateTotalRECSpectrum();
    emit signalUpdateSpecPlot();
}


void MainWindow::slotRemoveContamination()
{
    qInfo() << "MainWindow::slotRemoveContamination" ;

    Project* myProject = Project::get();
    QString Qfilename_;
    QString QId_ ;
     QModelIndexList selection = ui->tableContamination->selectionModel()->selectedRows();

     // Multiple rows can be selected mk-not finnished implementation look at energy variable

     for(int i=0; i< selection.count(); i++)
     {
         QModelIndex index = selection.at(i);
         qDebug() << index.row();
         QTableWidgetItem* itm = ui->tableContamination->item( index.row(), 0 );
         if (itm)  Qfilename_ = itm->text();
         QTableWidgetItem* itmId = ui->tableContamination->item( index.row(), 3 );
         if (itmId) QId_ = itmId->text();
        qDebug() << Qfilename_ << QId_;
         myProject->removeContamination(Qfilename_, QId_);
         ui->tableContamination->clear();
    }
     slotUpdateContaminationPanel();
     slotCalculateTotalRECSpectrum();
     emit signalUpdateSpecPlot();
}



void MainWindow::slotAutoFit()
{
    Project* myProject = Project::get();

    std::cout << "-------------MainWindow::AutoFit-------------------" <<std::endl;
    double  minEnergy = ui->lineEditFitEnergyFrom->text().toDouble();
    double  maxEnergy = ui->lineEditFitEnergyTo->text().toDouble();
//    double  minLevelEn = ui->lineEditFitLevelsFrom->text().toDouble();
//    double  maxLevelEn = ui->lineEditFitLevelsTo->text().toDouble();
//    double  lambda = ui->lineEditFitLambda->text().toDouble();
//    int     nrOfIterations = ui->lineEditNoFitIterations->text().toInt();

    if(maxEnergy<=minEnergy)
    {
        int r = QMessageBox::warning(this, tr("Error"),
                                     tr("Max energy must be higher than min energy"),
                                     QMessageBox::Ok);
        if (r == QMessageBox::Ok)
            return;
    }

    // fitResults and errors are not needed
    //std::vector< std::pair<double, double> > fitResults;
    //std::vector <float> errors;

    switch (fittingMethod_)
    {case 1:
        { //Maximum likely hood method
           try
            {
                checkHistogram( myProject->getExpHist() );
            }
            catch(GenError e)
            {
                return;
            }


            FitController* fitController = new FitController();

            fitController->applyMaximumLikelyhoodFit( myProject->getExpHist() );
            ui->buttonErrorCal->setEnabled(true);
            //errors = fitController->getErrors();
            break;
        }
    case 2:
        {  //Baysian multi spectrum fit
            try
             {
                 checkHistogram( myProject->getExpHist() );
             }
             catch(GenError e)
             {
                 return;
             }
             QString idString = ui->lineEditFitHistId->text();
             QStringList idList = idString.split(";",QString::SkipEmptyParts);

             FitController* fitController = new FitController();
             std::vector< std::pair<Histogram*, int> > bayesianHistograms;

             qDebug() << "Spectra to be fitted" << idList;
             qDebug() << "First spectrum: " << idList.at(0);
             qDebug() << "Second spectrum: " << idList.at(1);
//             qDebug() << "Third spectrum: " <<idList.at(2);
             for(int i = 0; i < idList.size(); i++)
             {
                 int histId = idList.at(i).toInt();
                 Histogram* tmpHis = myProject->getHistFromExpMap(histId);
                 qDebug() << "Impporting histId: " << histId ;
                 qDebug() << tmpHis->GetNrOfBins() ;
                 if(tmpHis == 0L)
                 {
                     myProject->addExpHist(histId, Histogram(myProject->getExpFile(), histId));
                     myProject->addExpSpecID(std::to_string(histId));
                     tmpHis = myProject->getHistFromExpMap(histId);
                     qDebug() << tmpHis->GetNrOfBins() ;
                 }
                 bayesianHistograms.emplace_back( tmpHis, histId );
             }

             fitController->applyBayesianFit( bayesianHistograms );
             //ui->buttonErrorCal->setEnabled(true);
             //errors = fitController->getErrors();
             break;
        }
    default:
        {int r = QMessageBox::warning(this, tr("Error"),
                                      tr("Please select fitting method first"),
                                      QMessageBox::Ok);
                   if (r == QMessageBox::Ok)
                       return;
                   break;
        }
    }

     ResponseFunction* responseFunction = ResponseFunction::get();
     //responseFunction->UpdateStructure();
     responseFunction->CalculateAllLevelsRespFunction();

     cout << "DONE! FIT RESULT: DECAY" << endl;
     DecayPath* decayPath = DecayPath::get();
     std::vector<Nuclide>* nuclides = decayPath->GetAllNuclides();
     std::vector<Level>* motherLevels;
     Nuclide* motherNuclide;

     motherNuclide = &nuclides->at(0);
     motherLevels = motherNuclide->GetNuclideLevels();

     Level* motherLevel = &motherLevels->at(0);
     std::vector<Transition*>* transitions = motherLevel->GetTransitions();  // getting transitions

     cout << "Level energy  Feeding(%)  Uncertainty"<< endl;
     for(auto itt = transitions->begin(); itt != transitions->end(); ++itt)
     {
         cout << (*itt) -> GetFinalLevelEnergy() << " "
         << (*itt) -> GetIntensity() * 100
         << " " << (*itt) -> GetD_Intensity() * 100 << endl;
     }
//     myProject->SetLastAutofitResults(fitResults);
     cout << "FIT RESULT: CONTAMINATION" << endl;
     cout << "<File name>  :  intensity " << endl;

//    for(unsigned int i = 0; i < contaminationController_->getContaminations().size(); i++)

     std::vector< std::pair<int, Contamination> >* contaminations = myProject->getContaminations();
    for(unsigned int i = 0; i < contaminations->size(); i++)
    {
        std::cout << contaminations->at(i).second.filename << " : " <<
                     contaminations->at(i).second.intensity << std::endl;
    }
    std::cout << "-------------MainWindow::AutoFit------END-------------" <<std::endl;

    emit signalUpdateDecSpec();
    emit signalUpdateContaminationPanel();
}



void MainWindow::slotEditLevelSchemeClicked(bool checked)
{
        if (checked) {
              OpenLevelSchemeEditor();
        } else {
              CloseLevelSchemeEditor(checked);
        }
}

void MainWindow::OpenLevelSchemeEditor()
{
    qInfo() << "MainWindow::OpenLevelSchemeEditor";
    ui->buttonEditLevelScheme->setText("Decay path editor OPENED");

    QStringList header;
//    int tabIndex = t1->uiT->tabDecay->currentIndex();
    std::vector<RowData> rowDataMother_;
    std::vector<RowData> rowDataDaughter_;
    std::vector<RowData> rowDataGrandDaughter_;
    std::vector <bool>  futureResults;

    DecayPath* decayPath= DecayPath::get();
    TableController *tableController_ = new TableController();


    std::vector<Nuclide>* nuclides_=decayPath->GetAllNuclides();
    for(unsigned int n =0; n !=nuclides_->size(); ++n)
    {
        std::vector<Level>* levels_ = nuclides_->at(n).GetNuclideLevels();

        int atomicNumber_ = nuclides_->at(n).GetAtomicNumber();
        QString QAtomicNumber = QString::number(atomicNumber_);
        QString QMassNumber = QString::number(nuclides_->at(n).GetAtomicMass());
        QString QElementName = QString::fromStdString(PeriodicTable::GetAtomicName(atomicNumber_));
        QString QQBeta = QString::number(nuclides_->at(n).GetQBeta());
        QString QT12 = QString::number(levels_->at(0).GetHalfLifeTime());
        QString QSn = QString::number(nuclides_->at(n).GetSn());

        if(n == 0)
        {
            for (unsigned int i = 0; i!=levels_->size(); ++i)
            {
            QString QEnergy_ = QString::number(levels_->at(i).GetLevelEnergy());
            QString QIntensity_ = QString::number(levels_->at(i).GetLevelEnergy());
            QString QTransitionsFrom_ = QString::number(levels_->at(i).GetTransitions()->size());
            QString QTransitionsTo_ = QString::number(levels_->at(i).GetLevelEnergy());
            rowDataMother_.push_back(RowData(QEnergy_, QIntensity_, QTransitionsFrom_, QTransitionsTo_));
            }
            t1->uiT->labelMotherT12->setText("T1/2 : " + QT12 +" s");
            t1->uiT->labelMotherIsotope ->setText("Isotope : " +QElementName+"-"+QMassNumber + "(Z = " +QAtomicNumber+")");
            t1->uiT->labelMotherQvalue->setText("QBeta = " +QQBeta + " keV");
            t1->uiT->labelMotherSn->setText("Sn = "+QSn + " keV");

         } else if(n == 2)
             {
            for (unsigned int i = 0; i!=levels_->size(); ++i)
            {
              QString QEnergy_ = QString::number(levels_->at(i).GetLevelEnergy());
              QString QIntensity_ = QString::number(levels_->at(i).GetLevelEnergy()); //to be corrected
              QString QTransitionsFrom_ = QString::number(levels_->at(i).GetTransitions()->size());
              QString QTransitionsTo_ = QString::number(levels_->at(i).GetLevelEnergy());
                rowDataGrandDaughter_.push_back(RowData(QEnergy_, QIntensity_, QTransitionsFrom_, QTransitionsTo_));
            }
            t1->uiT->labelGrandDaughterT12->setText("T1/2 : " + QT12 +" s");
            t1->uiT->labelGrandDaughterIsotope ->setText("Isotope : " +QElementName+"-"+QMassNumber + "(Z = " +QAtomicNumber+")");
            t1->uiT->labelGrandDaughterQValue->setText("QBeta = " +QQBeta + " keV");
            t1->uiT->labelGrandDaughterSn->setText("Sn = "+QSn + " keV");
         } else if (n==1)
             {
            std::vector <Level>* motherLevels = nuclides_->at(0).GetNuclideLevels();
            std::vector <Transition*>transitionsUsed;
            std::vector<Transition*>* transitions_ = motherLevels->at(0).GetTransitions();
            for(std::vector<Transition*>::iterator it = transitions_->begin(); it != transitions_->end(); ++it)
              {
                futureResults.push_back(true);
                transitionsUsed.push_back(*it);
               }
            { //mkstart
                for (unsigned int i = 0; i!=levels_->size(); ++i)
                {
                  QString QEnergy_ = QString::number(levels_->at(i).GetLevelEnergy());
 //                 QString QIntensity_ = QString::number(levels_->at(i).GetLevelEnergy());
                  QString QTransitionsFrom_ = QString::number(levels_->at(i).GetTransitions()->size());
                  QString QTransitionsTo_ = QString::number(levels_->at(i).GetLevelEnergy());
                  QString QFittingFlag_ = "n/a";
                  QString QIntensity_ ="n/a";
                  for(unsigned int ik = 0; ik != futureResults.size(); ++ik)
                    {
                      Transition* tmpTransition = transitionsUsed.at(ik);
                      Level* currentLevel = tmpTransition->GetPointerToFinalLevel();

//                      qDebug() << currentLevel->GetLevelEnergy() <<" ? " << levels_->at(i).GetLevelEnergy() << endl;

                    if(currentLevel->GetLevelEnergy() == levels_->at(i).GetLevelEnergy())
                    {
                      QFittingFlag_ = tmpTransition->GetIntensityFitFlag() ? "true" : "false";
                      qDebug() << tmpTransition->GetIntensity()*100 ;
                      QIntensity_ = QString::number(tmpTransition->GetIntensity()*100);
                      break;
                     }

                    }
                  rowDataDaughter_.push_back(RowData(QEnergy_, QIntensity_, QFittingFlag_, QTransitionsFrom_, QTransitionsTo_));
                }
            }  // mkstop

            qDebug() << "1 "  << " rowDataDaughter.size = " << rowDataDaughter_.size() ;
            /*           for(unsigned int i = 0; i != futureResults.size(); ++i)
              {
                Transition* tmpTransition = transitionsUsed.at(i);
                Level* currentLevel = tmpTransition->GetPointerToFinalLevel();
                QString QEnergy_ = QString::number(currentLevel->GetLevelEnergy());
                QString QIntensity_ = QString::number(tmpTransition->GetIntensity()*100);
                QString QFittingFlag_ = tmpTransition->GetIntensityFitFlag() ? "true" : "false";
                QString QTransitionsFrom_ = QString::number(levels_->at(i).GetTransitions()->size());
                QString QTransitionsTo_ = QString::number(levels_->at(i).GetLevelEnergy());
                rowDataDaughter_.push_back(RowData(QEnergy_, QIntensity_, QFittingFlag_, QTransitionsFrom_, QTransitionsTo_));
              }
   */         t1->uiT->labelDaughterT12->setText("T1/2 : " + QT12 +" s");
            t1->uiT->labelDaughterIsotope ->setText("Isotope : " +QElementName+"-"+QMassNumber + "(Z = " +QAtomicNumber+")");
            t1->uiT->labelDaughterQvalue->setText("QBeta = " +QQBeta + " keV");
            t1->uiT->labelDaughterSn->setText("Sn = "+QSn + " keV");

           }
        if(n > 2)qWarning() <<" TOO many nuclides cutting to 3" << endl;

    }
       tableController_->initializeTable(t1->uiT->tableMotherLevels,rowDataMother_);
       tableController_->initializeTable(t1->uiT->tableDaughterLevels,rowDataDaughter_);
       tableController_->initializeTable(t1->uiT->tableGrandDaughterLevels,rowDataGrandDaughter_);
       header << "Energy" << "Beta Feeding [%]" << "#Transitions from" << "# Transitions to";
       tableController_->setHeader(t1->uiT->tableMotherLevels,header);
       tableController_->setHeader(t1->uiT->tableGrandDaughterLevels,header);
       QStringList header2;
       header2 << "Energy" << "Beta Feeding [%]" << "Fit" << "#Transitions from" << "# Transitions to";
       tableController_->setHeader(t1->uiT->tableDaughterLevels,header2);
       t1->show();
}

void MainWindow::slotUpdateLevelEditor()
{
    qInfo() <<"MainWindow::slotUpdateLevelEditor" ;
    OpenLevelSchemeEditor();
}

void MainWindow::CloseLevelSchemeEditor(bool)
{
    ui->buttonEditLevelScheme->setText("Open decay path editor");
    ui->buttonEditLevelScheme->setChecked(false);
    t1->close();
}
void MainWindow::CloseExportFilesWindow(bool)
{
    e1->close();
}

void MainWindow::cleanHistogram(Histogram* hist)
{
    if(hist != 0L)
        delete hist;
}

void MainWindow::checkHistogram(Histogram* hist)
{
    if(hist == 0L)
    {
        int r = QMessageBox::warning(this, tr("Error"),
                                     tr("No histogram"),
                                     QMessageBox::Ok);
        if (r == QMessageBox::Ok)
            throw GenError("no histogram");
    }
}

void MainWindow::slotDecayPathEdited()
{
    ui->buttonUpdateResponseContainer->setEnabled(true);
    ui->buttonMakeSimulations->setEnabled(false);
    ui->buttonUploadAndCalculateResponse->setEnabled(false);
    //ui->buttonCalculateSIMSpec->setEnabled(false);
    ui->buttonMakeDirsAndCheckFiles->setEnabled(false);
    ui->actionCompare_Spectra->setEnabled(false);
}

void MainWindow::slotMakeDirsAndCheckFiles()
{
    qInfo() << "MainWindow::slotMakeDirsAndCheckFiles";
    ui->buttonCalculateSIMSpec->setEnabled(false);

    DecaySpectrum* decaySpectrum = new DecaySpectrum();
    decaySpectrum->CheckAndCreateDirectories();

    ResponseFunction* responseFunction = ResponseFunction::get();
    if( responseFunction->GetAllFilesReadyFlag() && responseFunction->GetResponseFunctionReady() )
        ui->buttonCalculateSIMSpec->setEnabled(true);

    else if( responseFunction->GetAllFilesReadyFlag() )
        ui->buttonUploadAndCalculateResponse->setEnabled(true);
    else
        ui->buttonMakeSimulations->setEnabled(true);
}

void MainWindow::slotMakeSimulations()
{
    qInfo() << "MainWindow::slotMakeSimulations()";
    Project* myProject = Project::get();
    bool ok;
    QString s =   QString::number(  myProject->getNumberOfSimulations());
    QString text = QInputDialog::getText(this, tr("Response Functions to be calculated"),
                                          tr("Please give a total numner of simulations"), QLineEdit::Normal,
                                          s , &ok);

    myProject->setNumberOfSimulations(text.toInt());


    DecaySpectrum* decaySpectrum = new DecaySpectrum();
    decaySpectrum->MakeSimulations();

    ResponseFunction* responseFunction = ResponseFunction::get();
    if( responseFunction->GetAllFilesReadyFlag() )
    {
        ui->buttonMakeSimulations->setEnabled(false);
        ui->buttonUploadAndCalculateResponse->setEnabled(true);
    }
}

void MainWindow::slotUploadAndCalculateResponse()
{
    qInfo() <<"MainWindow::slotUploadAndCalculateResponse";

    ResponseFunction* responseFunction = ResponseFunction::get();
    responseFunction->CalculateAllLevelsRespFunction();

    if( responseFunction->GetResponseFunctionReady() )
    {
        ui->buttonUploadAndCalculateResponse->setEnabled(false);
        ui->buttonCalculateSIMSpec->setEnabled(true);
        ui->comboBox_ExpSpecID->setEnabled(true);
    }
}

void MainWindow::slotUpdateResponseContainer()
{
    qInfo() << "MainWindow::slotUpdateResponseContainer";

    ResponseFunction* responseFunction = ResponseFunction::get();
    responseFunction->UpdateStructure();

    ui->buttonMakeSimulations->setEnabled(false);
    ui->buttonUploadAndCalculateResponse->setEnabled(false);
    ui->buttonCalculateSIMSpec->setEnabled(false);
    ui->buttonUpdateResponseContainer->setEnabled(false);
    ui->buttonMakeDirsAndCheckFiles->setEnabled(true);
    ui->actionCompare_Spectra->setEnabled(false);
}

void MainWindow::slotCalculateDECSpectrum()
{
    qInfo() << "MainWindow::slotCalculateDECSpectrum";
    qWarning() << "---------------DECAY spectrum calculation---------------";
    std::cout << "---------------DECAY spectrum calculation---------------" << std::endl;

    Project* myProject = Project::get();
    int expSpectrumID = std::stoi(myProject->getExpSpecID());
    ResponseFunction* responseFunction = ResponseFunction::get();
    DecayPath* decayPath = DecayPath::get();

    // 20230807 Observe if this can be commented
    if( !(responseFunction->GetResponseFunctionReady()) )
        responseFunction->CalculateAllLevelsRespFunction();

    int histId = std::stoi(myProject->getExpSpecID());
    Level* motherLevel = &decayPath->GetAllNuclides()->at(0).GetNuclideLevels()->at(0);

    Histogram tmpDecHist = new Histogram();
    tmpDecHist = *responseFunction->GetResponseFunction(motherLevel, histId);

    tmpDecHist.Rebin(myProject->getBinning1Dfactor());

    ui->frameFit->setEnabled(true);
    ui->buttonAutoFit->setEnabled(true);
    ui->buttonAnalysis2D->setEnabled(true);
    ui->buttonManualFit->setEnabled(true);
    ui->actionCompare_Spectra->setEnabled(true);
    ui->comboBoxFit1D->setEnabled(true);
    ui->labelFitMethod->setEnabled(true);

       std::vector< std::pair<int, Contamination> >* contaminations = myProject->getContaminations();
        float sumNormCont = 0.0;
        for (unsigned int i = 0; i !=  contaminations->size(); i++)
        {
            if(contaminations->at(i).first == expSpectrumID)
                sumNormCont += contaminations->at(i).second.intensity;
        }
        float norm = myProject->getExpHist()->GetNrOfCounts()*(1-sumNormCont);
        std::cout << " Normalize DECAY spectrum to: " << norm <<std::endl;

        tmpDecHist.Normalize(norm);

        myProject->setDecHist( tmpDecHist );
        myProject->replaceSimHistInMap(expSpectrumID,tmpDecHist);

     slotCalculateTotalRECSpectrum();
}


void MainWindow::slotCalculateTotalRECSpectrum()
{

    Project* myProject = Project::get();
    int expSpectrumID = std::stoi(myProject->getExpSpecID());

    cout << "---------------Calculating TOTAL reconstructed spectrum--------------- " << endl;

    Histogram* recHist = Histogram::GetEmptyHistogram(0, 100, 100 / myProject->getBinning1Dfactor());
    Histogram* difHist = Histogram::GetEmptyHistogram(0, 100, 100 / myProject->getBinning1Dfactor());
    Histogram* decHist = myProject->getDecHist();

    if(decHist != 0L)
    {
        recHist->Add( decHist, 1.);
    }

    std::vector< std::pair<int, Contamination> > contaminations = *(myProject->getContaminations());
    cout << "Contaminations to be added = " << contaminations.size() <<endl;
    for (unsigned int i = 0; i !=  contaminations.size(); i++)
    {
    qDebug() << "i= " << i << " " << QString::fromStdString(contaminations.at(i).second.filename)
                 << " " << &contaminations.at(i).second.hist
                 << " " << contaminations.at(i).second.intensity
                 << " " << contaminations.at(i).second.normalization
                 << " " << contaminations.at(i).second.hist.GetNrOfCounts();


        if(contaminations.at(i).first == expSpectrumID)
        {
            double fact = contaminations.at(i).second.intensity * myProject->getExpHist()->GetNrOfCounts();
            Histogram *tmpCont = new Histogram(contaminations.at(i).second.hist);
            tmpCont->Normalize(fact);
            recHist->Add(tmpCont, 1.0);
            delete tmpCont;
        }
    }

    float norm = myProject->getExpHist()->GetNrOfCounts();
    recHist->Normalize(norm);


    std::cout<<"---------Total RECONSTRUCTED spectrum has been calculated "
             << " Xmin: " << recHist -> GetXMin()
             << " Xmax: "<< recHist -> GetXMax()
             << " NrOfBins: " << recHist -> GetNrOfBins()
             << " NrOfCounts: " << recHist -> GetNrOfCounts() <<"-----"<< std::endl;

      ui->specPlot->clearGraphs();
    difHist ->Add(myProject->getExpHist(), 1.);
    difHist ->Add(recHist, -1.);

    myProject->setRecHist(*recHist);
    myProject->replaceRecHistInMap(expSpectrumID,recHist);
    myProject->setDifHist(*difHist);
    emit signalUpdateSpecPlot();


}

void MainWindow::slotSetMainLogScale(bool checked)
{
    if (checked) {
        if(ui->lineEditYAxisMin->text().toDouble() == 0.0){
            ui->lineEditYAxisMin->setText("1");
        }
    ui->specPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    } else {
        if(ui->lineEditYAxisMin->text().toDouble()==1.0){
            ui->lineEditYAxisMin->setText("0");
        }
        ui->specPlot->yAxis->setScaleType(QCPAxis::stLinear);
    }
    ui->specPlot->replot();
}
void MainWindow::slotSetNormRange()
{
    Project* myProject = Project::get();

    double NormMin =  ui -> lineEditNormMin->text().toDouble();
    double NormMax =  ui -> lineEditNormMax->text().toDouble();

    if (NormMin == NormMax)
    {
        NormMin = myProject->getExpHist()->GetXMin();
        NormMax= myProject->getExpHist()->GetXMax();
     }
    myProject->setNormMin(NormMin);
    myProject->setNormMax(NormMax);

   emit signalUpdateSpecPlot();
}
void MainWindow::slotSetAxisRange()
{
    Project* myProject = Project::get();

    double xMinEn =  ui -> lineEditXAxisMin->text().toDouble();
    double xMaxEn =  ui -> lineEditXAxisMax->text().toDouble();
 //   double size = xMaxEn-xMinEn;
    ui->specPlot->xAxis->setRange(xMinEn,xMaxEn);
    double yMin = ui->lineEditYAxisMin->text().toDouble();
    double yMax = ui->lineEditYAxisMax->text().toDouble();
//    double ymax = static_cast<double>( expHist->GetMaxValue(xMinEn,xMaxEn));
//    ymax = ymax*1.1; //adding 10% to the scale
    ui->specPlot->yAxis->setRange(yMin,yMax);
    // calculating SUM of displayed exp spectrum
    QString qstr = "NoC in Exp. spec. within limits: "+ QString::number(myProject->getExpHist()->GetNrOfCounts(xMinEn,xMaxEn));
    ui->labelExpCount->setText(qstr);

    ui->specPlot->replot();

}


string MainWindow::slotReadTableData(QTableWidget* table, int xtab, int ytab)
{
    QString itabtext = table->item(xtab,ytab)->text();
    string str = itabtext.toUtf8().constData();
//    std::cout << "slotReadTableData - Przeczytane" << xtab <<" , "<<ytab<< " : " << str << std::endl;
    return str;
}

void MainWindow::slotContaminationTableChanged(int xtab, int ytab)
{
    slotUpdateContaminationData(xtab,ytab);
    slotCalculateTotalRECSpectrum();
    emit signalUpdateSpecPlot();
}

void MainWindow::WriteTableData(QTableWidget* table, int row, int column, string item)
{
//    std::cout << "In WriteTableData " <<row <<" "<< column <<" "<<item << std::endl;
    const QSignalBlocker blocker(table);
    if(row >= table->rowCount())table->insertRow(table->rowCount());
    QTableWidgetItem *newItem = new QTableWidgetItem;(tr("%1").arg(pow(row, column+1)));
                table->setItem(row, column, newItem);
                table->item(row, column)->setText(QString::fromStdString(item));
}

void MainWindow::ClearTable(QTableWidget* table)
{
    const QSignalBlocker blocker(table);
    table->clear();

}
void MainWindow::slotUpdateContaminationPanel()
{
    qInfo() <<"MainWindow::slotUpdateContaminationPanel";
    Project* myProject = Project::get();
    ClearTable(ui->tableContamination);
    int expSpectrumID = std::stoi(myProject->getExpSpecID());

    QStringList header;
    header << "File Name" << "Norm (%)" << "free param" << "histID" ;
    ui->tableContamination->setHorizontalHeaderLabels(header);


    std::vector<Contamination> contaminations = myProject->getContaminationsSpecID(expSpectrumID);
    string str;
  //  int row = 0;
    for(unsigned int con = 0; con != contaminations.size(); ++con )
    {
            for (int column = 0; column != 4; ++column)
            {
                if (column == 0) {
//                    string fileName = contaminations.at(con).second.filename;
                    string fileName = contaminations.at(con).filename;
                    int dirLength = myProject->getWorkingDir().length();
                    if (fileName.substr(0,dirLength) == myProject->getWorkingDir()){
                        int a = dirLength -   fileName.length();
//                    str = contaminations.at(con).second.filename.substr(dirLength,a) + ".his" ;
                    str = contaminations.at(con).filename.substr(dirLength,a) + ".his" ;
    //            cout << "fileName : " <<fileName;
    //            cout << "dirLength: " << dirLength;
    //            cout << "filename-str: " << str;
                    }
                    else {str = contaminations.at(con).filename;}
                }

               if (column == 3) str = std::to_string(contaminations.at(con).id);
                if (column == 1) str = std::to_string(contaminations.at(con).intensity*100) ;
                if (column == 2) str =contaminations.at(con).GetIntensityFitFlag() ? "true" : "false";
      //          cout << "string to be added" << str << endl;
                WriteTableData(ui->tableContamination, con, column, str);
    //            row++;
            }
    }

/*    //It probably works only if expSpectrumID contaminations are first in .tas file!!!!
    int k=0;
    for(unsigned int row = 0; row != contaminations.size(); ++row )
//        if(contaminations.at(row).first == expSpectrumID)
            for (int column = 0; column != 4; ++column){
                slotUpdateContaminationData(row, column);
            k++;
            }
*/
}


void MainWindow::slotUpdateContaminationData(int xtab, int ytab)
{
    Project* myProject = Project::get();

    int row = xtab;
    int column = ytab;
    string str = slotReadTableData(ui->tableContamination, row, column);
    std::vector< std::pair<int, Contamination> > contaminations = *(myProject->getContaminations());
    bool readNewSpectrum_ = false;
    if (column == 0)
    {
        if (str != contaminations.at(row).second.filename)
        {
          contaminations.at(row).second.filename = str;
          readNewSpectrum_ = true;
         }
    }
    else if (column == 1)
    {
        if (str != std::to_string(contaminations.at(row).second.intensity*100))
        {
           contaminations.at(row).second.intensity = stof(str)/100;
           double value = contaminations.at(row).second.intensity * myProject->getExpHist()->GetNrOfCounts();
           contaminations.at(row).second.normalization =  value;
        }
    }
    else if (column == 3)
        {
           if (str != std::to_string(contaminations.at(row).second.id))
           {
            contaminations.at(row).second.id = stoi(str);
            readNewSpectrum_ = true;
           }
        }
    else if (column == 2)
        {
            if(str != std::to_string(contaminations.at(row).second.GetIntensityFitFlag()))
            {
                bool flag;
                if (str == "true") flag = true;
                if (str == "false") flag = false;
                contaminations.at(row).second.SetIntensityFitFlag(flag);
            }
        }
     else   qWarning() << "ERROR IN CONTAMINATION TABLE ";


    myProject->setOneContamination(row,contaminations.at(row).second); // zmiana contaminations w myProject

    if (readNewSpectrum_)
    {
       string fileName = contaminations.at(row).second.filename;
        int histId =contaminations.at(row).second.id;
        Histogram *tmpHist = new Histogram(fileName,histId);
        contaminations.at(row).second.hist = *tmpHist;
        myProject->setOneContamination(row,contaminations.at(row).second); // efektywniezmiana widma w contaminations w myProject
        delete tmpHist;
        readNewSpectrum_ =false;
    }
}

void MainWindow::slotContaminationTableClicked(int row, int column)
{
    qInfo() << "MainWindow::slotContaminationTableClicked";
//    std::cout << "MainWindow::ContaminationTableClicked row " << row << " column " << column << std::endl;
    if(column != 2) return;
    if(column == 2){
//        int row = xtab;
//        int column = ytab;
        Project* myProject = Project::get();

        string str = slotReadTableData(ui->tableContamination, row, column);
        std::vector< std::pair<int, Contamination> >* contaminations_ = myProject->getContaminations();
        bool flag;
        if (str == "true") flag = true;
        if (str == "false") flag = false;
        flag = !flag;
        contaminations_->at(row).second.SetIntensityFitFlag(flag);
    const QSignalBlocker blocker(ui->tableContamination);
    ui->tableContamination->setItem(row, 2, new QTableWidgetItem(flag ? "true" : "false"));
//    ui->tableContamination->setItem(row, 2, new QTableWidgetItem(flag ? "1" : "0"));
    ui->tableContamination->show();
    }
}
void MainWindow::slotUpdateProjectPanel(bool checked)
{
    Project* myProject = Project::get();

    ui->lineProjectName->setText(QString::fromStdString(myProject->getProjectName()));
    ui->lineWorkingDir->setText(QString::fromStdString(myProject->getWorkingDir()));
    ui->lineExpFile->setText(QString::fromStdString(myProject->getExpFile()));
//    ui->lineExpSpecID->setText(QString::fromStdString(myProject->getExpSpecID()));
    ui->comboBox_ExpSpecID->setCurrentText(QString::fromStdString(myProject->getExpSpecID()));
    ui->lineExp2DSpecID->setText(QString::fromStdString(myProject->getExp2DSpecID()));
    ui->lineInputDecayFile->setText(QString::fromStdString(myProject->getInputDecayFile()));
    ui->lineOutputDecayFile->setText(QString::fromStdString(myProject->getOutputDecayFile()));
    ui->lineOutputSIMFile->setText(QString::fromStdString(myProject->getOutputSIMFile()));
    ui->lineOutputLEVFile->setText(QString::fromStdString(myProject->getOutputLEVFile()));
    ui->lineCodeGEANT->setText(QString::fromStdString(myProject->getCodeGEANTName()));
//    ui->lineEditFitHistId->setText(QString::fromStdString(myProject->getExpSpecID()));
    ui->lineEditFitEnergyFrom->setText(QString::number(myProject->getFitEnergyFrom()));
    ui->lineEditFitEnergyTo->setText(QString::number(myProject->getFitEnergyTo()));
    ui->lineEditFitLevelsFrom->setText(QString::number(myProject->getFitLevelsFrom()));
    ui->lineEditFitLevelsTo->setText(QString::number(myProject->getFitLevelsTo()));
    ui->lineEditFitLambda->setText(QString::number(myProject->getFitLambda()));
    ui->lineEditNoFitIterations->setText(QString::number(myProject->getNoFitIterations()));
    ui->lineEditFitBinning->setText(QString::number(myProject->getFitBinning()));

}

void MainWindow::slotProjectNameUpdate(){Project* myProject = Project::get(); myProject->setProjectName(ui->lineProjectName->text().toUtf8().constData()); }
void MainWindow::slotProjectWorkingDirUpdate()  {Project* myProject = Project::get(); myProject->setWorkingDir(ui->lineWorkingDir->text().toUtf8().constData()); }
void MainWindow::slotProjectExpFileUpdate()  {Project* myProject = Project::get();
                                              myProject->setExpFile(ui->lineExpFile->text().toUtf8().constData());}
void MainWindow::slotProjectExpSpecIDUpdate(int ID) {slotProjectExpSpecIDUpdate(); }
void MainWindow::slotProjectExpSpecIDUpdate()  {Project* myProject = Project::get();
//                                                myProject->setExpSpecID(ui->lineExpSpecID->text().toUtf8().constData());
                                                myProject->setExpSpecID(ui->comboBox_ExpSpecID->currentText().toUtf8().constData());
                                                myProject->setExpHist();
                                                emit signalUpdateContaminationPanel();
                                                emit signalUpdateDecSpec();
                                                slotCalculateTotalRECSpectrum();
                                                emit signalUpdateSpecPlot();}
void MainWindow::slotProjectExp2DSpecIDUpdate()  {Project* myProject = Project::get(); myProject->setExp2DSpecID(ui->lineExp2DSpecID->text().toUtf8().constData()); }
void MainWindow::slotProjectInputDecayFileUpdate()  {Project* myProject = Project::get(); myProject->setInputDecayFile(ui->lineInputDecayFile->text().toUtf8().constData()); }
void MainWindow::slotProjectOutputDecayFileUpdate()  {Project* myProject = Project::get(); myProject->setOutputDecayFile(ui->lineOutputDecayFile->text().toUtf8().constData()); }
void MainWindow::slotProjectOutputSIMFileUpdate()  {Project* myProject = Project::get(); myProject->setOutputSIMFile(ui->lineOutputSIMFile->text().toUtf8().constData()); }
void MainWindow::slotProjectOutputLEVFileUpdate()  {Project* myProject = Project::get(); myProject->setOutputLEVFile(ui->lineOutputLEVFile->text().toUtf8().constData()); }
void MainWindow::slotProjectCodeGEANTUpdate()  {Project* myProject = Project::get(); myProject->setCodeGEANTName(ui->lineCodeGEANT->text().toUtf8().constData());}
void MainWindow::slotProjectFitEnergyFrom() {Project* myProject = Project::get(); myProject->setFitEnergyFrom(ui->lineEditFitEnergyFrom->text().toDouble()); }
void MainWindow::slotProjectFitEnergyTo() {Project* myProject = Project::get(); myProject->setFitEnergyTo(ui->lineEditFitEnergyTo->text().toDouble()); }
void MainWindow::slotProjectFitLevelsFrom() {Project* myProject = Project::get(); myProject->setFitLevelsFrom(ui->lineEditFitLevelsFrom->text().toDouble()); }
void MainWindow::slotProjectFitLevelsTo() {Project* myProject = Project::get(); myProject->setFitLevelsTo(ui->lineEditFitLevelsTo->text().toDouble()); }
void MainWindow::slotProjectFitLambda() {Project* myProject = Project::get(); myProject->setFitLambda(ui->lineEditFitLambda->text().toDouble()); }
void MainWindow::slotProjectNoFitIterations() {Project* myProject = Project::get(); myProject->setNoFitIterations(ui->lineEditNoFitIterations->text().toInt()); }
void MainWindow::slotProjectFitBinning()
{
    Project* myProject = Project::get();
    myProject->setFitBinning(ui->lineEditFitBinning->text().toInt());
}

void MainWindow::slotUpdateSpecPlot()
{
    Project* myProject = Project::get();

    QPen blueDotPen;
    blueDotPen.setColor(QColor(0, 0, 255, 150));
    blueDotPen.setStyle(Qt::DotLine);
    blueDotPen.setWidthF(1);
    QPen bluePen;
    bluePen.setColor(QColor(0, 0, 255, 150));
    bluePen.setWidthF(1);
    QPen greenPen;
    greenPen.setColor(QColor(0, 255, 0, 150));
    greenPen.setWidthF(1);

    QPen redDotPen;
    redDotPen.setColor(QColor(255, 0, 0, 250));
    redDotPen.setStyle(Qt::DotLine);
    redDotPen.setWidthF(2);
    QPen blackDotPen;
    blackDotPen.setColor(QColor(0, 0, 0, 250));
    blackDotPen.setStyle(Qt::DotLine);
    blackDotPen.setWidthF(2);
    QPen blackPen;
    blackPen.setColor(QColor(0, 0, 0, 250));
    blackPen.setWidthF(1);
    QPen redPen;
    redPen.setColor(QColor(255, 0, 0, 250));
    redPen.setWidthF(1);
   QFont legendFont = font();  // start out with MainWindow's font..
    legendFont.setPointSize(9); // and make a bit smaller for legend

    ui->specPlot->clearGraphs();
    ui->specPlot->clearItems();
    ui->specPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);

    ui->specPlot->legend->setVisible(true);
    ui->specPlot->legend->setBrush(QBrush(QColor(255,255,255,150)));
    ui->specPlot->legend->setFont(legendFont);


//  setting up boundaries of the plot
    ui->specPlot->xAxis->setLabel("Channels/Energy (keV)");
    ui->specPlot->yAxis->setLabel("Counts");
    double xEMin =  ui -> lineEditXAxisMin->text().toDouble();
    double xEMax =  ui -> lineEditXAxisMax->text().toDouble();
    double yMin =  ui -> lineEditYAxisMin->text().toDouble();
    double yMax =  ui -> lineEditYAxisMax->text().toDouble();
    ui->specPlot->xAxis->setRange(xEMin,xEMax);
//    double ymax = static_cast<double>( myProject->getExpHist()->GetMaxValue(xEMin,xEMax));
//    ymax = ymax*1.1; //adding 10% to the scale
    ui->specPlot->yAxis->setRange(yMin,yMax);

// getting Q value from NNDC input data and making a line
        QCPItemLine *QValueLine = new QCPItemLine(ui->specPlot);
        QCPItemLine *SnValueLine = new QCPItemLine(ui->specPlot);
        DecayPath* decayPath= DecayPath::get();
        if(decayPath != 0L)
        {
            double QValue = decayPath->GetAllNuclides()->at(0).GetQBeta();

            // add the text label at the top:
            QCPItemText *textLabel = new QCPItemText(ui->specPlot);
            textLabel->setPositionAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    //        textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
            textLabel->position->setCoords(QValue, yMax/4);
            QString qstr = "Q=" + QString::number(QValue) + " keV";
            textLabel->setText(qstr);
            textLabel->setFont(QFont(font().family(), 16)); // make font a bit larger
            textLabel->setPen(QPen(Qt::black)); // show black border around text
            textLabel->setRotation(-90);
            QValueLine->start->setParentAnchor(textLabel->left);
    //        QValueLine->start->setCoords(QValue,ymax/2);
            QValueLine->end->setCoords(QValue, 1);
            QValueLine->setHead(QCPLineEnding::esSpikeArrow);
            QValueLine->setPen(QPen(Qt::red));
            // time for SN
            double Sn = decayPath->GetAllNuclides()->at(1).GetSn();
            if (Sn < QValue)
            {
            QCPItemText *textLabel2 = new QCPItemText(ui->specPlot);
            textLabel2->setPositionAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            textLabel2->position->setCoords(Sn, yMax/4);
            QString Snstr = "Sn=" + QString::number(Sn) + " keV";
            textLabel2->setText(Snstr);
            textLabel2->setFont(QFont(font().family(), 16)); // make font a bit larger
            textLabel2->setPen(QPen(Qt::black)); // show black border around text
            textLabel2->setRotation(-90);
            SnValueLine->start->setParentAnchor(textLabel2->left);
            SnValueLine->end->setCoords(Sn, 1);
            SnValueLine->setHead(QCPLineEnding::esSpikeArrow);
            SnValueLine->setPen(QPen(Qt::red));
            }

        }

        QVector<double> x = QVector<double>::fromStdVector(myProject->getExpHist()->GetEnergyVectorD());
        QVector<double> y1 = QVector<double>::fromStdVector(myProject->getExpHist()->GetAllDataD());
 // calculating SUM of displayed exp spectrum
       double y1Sum = std::accumulate(y1.begin(),y1.end(),0.0);
       QString qstr = "NoC in Exp. spec. within limits: "+ QString::number(y1Sum);
       ui->labelExpCount->setText(qstr);


    ui->specPlot->addGraph();
    ui->specPlot->graph(0)->setName("Experiment");
    ui->specPlot->graph(0)->setPen(blackPen);
    ui->specPlot->graph(0)->setData(x, y1);
    ui->specPlot->graph(0)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
    ui->specPlot->graph(0)->selectable();
    ui->specPlot->graph(0)->selectionDecorator()->setPen(QPen(Qt::yellow));

    ui->specPlot->addGraph();
    ui->specPlot->graph(1)->setName("Decay");
    ui->specPlot->graph(1)->setPen(QPen(Qt::green));
    ui->specPlot->graph(1)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
    ui->specPlot->graph(1)->selectable();
    ui->specPlot->graph(1)->selectionDecorator()->setPen(QPen(Qt::yellow));


    if(myProject->getDecHist()!= 0L)
    {
        //Histogram* decHist1 = myProject->getDecHist();
        QVector<double> x3 = QVector<double>::fromStdVector(myProject->getDecHist()->GetEnergyVectorD());
        QVector<double> y3 = QVector<double>::fromStdVector(myProject->getDecHist()->GetAllDataD());
        ui->specPlot->graph(1)->setData(x3, y3);
    }



    if(myProject->getRecHist()!= 0L)
    {
        Histogram *recHist1 = myProject->getRecHist();
        double normMin = myProject->getNormMin();
        double normMax = myProject->getNormMax();
        double norm = myProject->getExpHist()->GetNrOfCounts(normMin,normMax);
        recHist1->Normalize(norm,normMin,normMax);
        QVector<double> x2 = QVector<double>::fromStdVector(recHist1->GetEnergyVectorD());
        QVector<double> y2 = QVector<double>::fromStdVector(recHist1->GetAllDataD());
//        QVector<double> x2 = QVector<double>::fromStdVector(myProject->getRecHist()->GetEnergyVectorD());
//        QVector<double> y2 = QVector<double>::fromStdVector(myProject->getRecHist()->GetAllDataD());

        ui->specPlot->addGraph();
        ui->specPlot->graph(2)->setName("Reconstruction");
        ui->specPlot->graph(2)->setPen(redPen);
    //    cout << " size x,y1,x2,y2 " << x.size() << " " << y1.size() << " "
    //        << x2.size() << " " << y2.size() <<  endl;
        ui->specPlot->graph(2)->setData(x2, y2);
        ui->specPlot->graph(2)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
        ui->specPlot->graph(2)->selectable();
        ui->specPlot->graph(2)->selectionDecorator()->setPen(QPen(Qt::yellow));
    }
    std::vector< std::pair<int, Contamination> >* contaminations = myProject->getContaminations();
    int expSpectrumID = std::stoi(myProject->getExpSpecID());
    int k =0;
    for (unsigned int i = 0; i !=  contaminations->size(); i++)
    {
        if(contaminations->at(i).first == expSpectrumID)
        {
            Histogram tmpHist = contaminations->at(i).second.hist;
            double norm = contaminations->at(i).second.intensity * myProject->getExpHist()->GetNrOfCounts();
            tmpHist.Normalize(norm); // ,xMin,xMax);
            QVector<double> x4 = QVector<double>::fromStdVector(tmpHist.GetEnergyVectorD());
            QVector<double> y4 = QVector<double>::fromStdVector(tmpHist.GetAllDataD());

            ui->specPlot->addGraph();
    // set style for the graph
            string conName ="Contamination " + std::to_string(k+1);
            ui->specPlot->graph(k+3)->setName(QString::fromStdString(conName));
            ui->specPlot->graph(k+3)->setPen(bluePen);
            ui->specPlot->graph(k+3)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
            ui->specPlot->graph(k+3)->selectable();
            ui->specPlot->graph(k+3)->selectionDecorator()->setPen(QPen(Qt::yellow));
    // set data for the graph
            ui->specPlot->graph(k+3)->setData(x4, y4);
            k++;
        }
    }

    // synchronize selection of graphs with selection of corresponding legend items:

    ui->specPlot->replot();
}

void MainWindow::slotSelectionChanged()
{
    for (int i=0; i<ui->specPlot->graphCount(); ++i)
    {
         QCPGraph *graph = ui->specPlot->graph(i);
          QCPPlottableLegendItem *item = ui->specPlot->legend->itemWithPlottable(graph);
          if (item->selected() || graph->selected())
            {
               item->setSelected(true);
               graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
            }
    }
}

void MainWindow::slotClearContaminations()
{
    Project* myProject = Project::get();

    myProject->removeAllContaminations();
    ui->tableContamination->clear();
    slotUpdateContaminationPanel();
}

void MainWindow::newProject(bool triggered)
{
    Project* myProject = Project::get();

    if(!triggered)
    {
    if(myProject->getInputContaminations().size() != 0){
      slotClearContaminations();
    }
    myProject->New();   //clear all the fields
    slotUpdateProjectPanel(true);
    // filling project fields updates automaticaly the data
    QMessageBox::information(this, "New Project", "Please fill in Project Frame data", QMessageBox::Ok, QMessageBox::Ok);

    }
}
void MainWindow::saveProject(bool trigered)
{
    Project* myProject = Project::get();
    e1= new ExportFiles;
    std::string path = e1->checkAndPreparePath();
    QString Qpath_ ;
    if(path.size() < 40)
    {
        Qpath_ = QString::fromStdString(path);
    } else {
        Qpath_ = QString::fromStdString(path.substr(0,15)) + "..."  + QString::fromStdString(path.substr(path.size() - 25,path.size()));
    }

    if(!trigered)
    {

    QString qfileName;
    qfileName = Qpath_ + "/" + QString::fromStdString(myProject->getProjectName()) + ".tas" ;
 //   qfileName = QFileDialog::getSaveFileName(this, tr("Save project to file") );
    QFile file(qfileName);
            file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << "ProjectName: " << QString::fromStdString(myProject->getProjectName()) << "\n" ;
    out << "WorkingDir: " << QString::fromStdString(myProject->getWorkingDir()) <<"\n" ;
    out << "ExpFile: " << QString::fromStdString(myProject->getExpFile()) <<"\n" ;
    out << "ExpSpecID: " << QString::fromStdString(myProject->getExpSpecID()) <<"\n" ;
    out << "Exp2DSpecID: " << QString::fromStdString(myProject->getExp2DSpecID()) <<"\n" ;
    out << "InputDecayFile: " << QString::fromStdString(myProject->getInputDecayFile()) <<"\n" ;
    out << "OutputDecayFile: " << QString::fromStdString(myProject->getOutputDecayFile()) <<"\n" ;
    out << "OutputSIMFile: " << QString::fromStdString(myProject->getOutputSIMFile()) <<"\n" ;
    out << "OutputLEVFile: " << QString::fromStdString(myProject->getOutputLEVFile()) <<"\n" ;
    out << "CodeGEANT: " << QString::fromStdString(myProject->getCodeGEANTName()) <<"\n" ;
    out << "CodeGEANTver: " << QString::fromStdString(myProject->getCodeGEANTver()) <<"\n" ;
    out << "NumberOfSimulations: " << QString::number(myProject->getNumberOfSimulations()) <<"\n" ;
//---Contaminations --
// lpop po mapie
    vector<vector <string> > inputContaminations = myProject->getInputContaminations();
    qDebug() << "--------------------------------" ;
    qDebug() << "size" <<    inputContaminations.size() ;
    for(unsigned long i=0; i<inputContaminations.size(); i++)
    {
          qDebug() << "size" <<    inputContaminations.at(i).size() ;
      //  qDebug() << QString::fromStdString(inputContaminations.at(i)) ;
          qDebug() <<"i= " <<i;
        out << "Contamination: " ;
            for(unsigned long ii=0; ii<inputContaminations.at(i).size(); ii++){
            qDebug() <<"ii= " <<ii << " "
                         << QString::fromStdString(inputContaminations.at(i).at(ii));
            out << " " << QString::fromStdString(inputContaminations.at(i).at(ii)) ;
        } out << "\n";
    }
    //   out << "Contamination: " << QString::fromStdString(myProject->getCodeGEANTver()) <<"\n" ;
//Contami-end
    out << "SortProgramName: " << myProject->getSortProgramName() <<"\n" ;
    out << "SortInputFileName: " << myProject->getSortInputFileName() <<"\n" ;
    out << "SiliconThreshold: " << QString::number(myProject->getSiliThreshold()) <<"\n" ;
    out << "IMOThreshold: " << QString::number(myProject->getIMOThreshold()) <<"\n" ;
    out << "Binning1D: " << QString::number(myProject->getBinning1Dfactor()) <<"\n" ;
    out << "Binning2D: " << QString::number(myProject->getBinning2Dfactor()) <<"\n" ;
    out << "SimulationActiveCores: " << QString::number(myProject->getActiveCoresForSimulation()) <<"\n" ;
    out << "NormalizeBetaIntensities: " << myProject->getNormalizeBetaIntensities()  <<"\n" ;
    out << "SaveRaoundedBetaIntensities: " << myProject->getSaveRoundedBetaIntensities() <<"\n" ;
    out << "2DNormalizeRange: " << QString::number(get<0>(myProject->getTwoDimNormalizeRange())) << " "
        << QString::fromStdString(get<1>(myProject->getTwoDimNormalizeRange())) <<"\n" ;
    out << "RoughUncertaintyModifierInPercets: " << QString::number(myProject->getUncertModifierInPercents()) <<"\n" ;

    //    out << "Contamination: " << QString::fromStdString(myProject->getWorkingDir()) <<"\n" ;

// consider saving also HERE decay path OutputENSFile
    file.close();
    }
}

void MainWindow::openProject(bool trigered)
{
    Project* myProject = Project::get();

    if(projectOpen_){
        int r = QMessageBox::warning(this, tr("Error"),
                                     tr("Project already exists"),
                                     QMessageBox::Cancel,
                                     QMessageBox::Ignore);
        if (r == QMessageBox::Cancel)
        {
            return;
        } else if (r == QMessageBox::Ignore){
//            myProject->New();
//            slotClearContaminations();
        }

    }

    string sfileName = myProject->getProjectInputFileName();
    if(sfileName == "")
    {

    QString qfileName;
    qfileName = QFileDialog::getOpenFileName(this,
        tr("Load MTAS Project File"), QString::fromStdString(myProject->getWorkingDir()), tr("MTAS project files (*.tas)"));

    if(qfileName.isEmpty()&& qfileName.isNull())
        return;
    sfileName = qfileName.toStdString();

    }
    myProject->New();
    //slotClearContaminations();


    bool OK = myProject->Open(sfileName);
    if(!OK) return;
    qDebug() << "Current working directory is: " << QDir::currentPath();
    if(QDir::currentPath() != QString::fromStdString(myProject->getWorkingDir()))
    {
        if (!QDir::setCurrent(QString::fromStdString(myProject->getWorkingDir())))
        qDebug() << "Could not change the current working directory ";
    qDebug() << "Working directory changed to: " << QDir::currentPath();
    }
    slotUpdateProjectPanel(true);

    vector<string> IDVec = myProject->getExpSpecIDVec();
    for(unsigned int i=0; i<IDVec.size() ; i++)
    {
      int histId = stoi(IDVec.at(i));
      Histogram tmpHis = new Histogram(myProject->getExpFile(),histId);
      myProject->addExpHist(histId,tmpHis);
    }
    SetcomboBoxExpSpecID();
    myProject->setExpHist();
    myProject->getExpHist()->Rebin( myProject->getBinning1Dfactor() );

    double xEMax = myProject->getExpHist()->GetNrOfBins();
    double ymax = static_cast<double>( myProject->getExpHist()->GetMaxValue(1,xEMax));
    double counts = myProject->getExpHist()->GetNrOfCounts();
    qDebug() << "Total number of counts in experimental spectrum: " << counts;
    std::cout << "Total number of counts in experimental spectrum: " << std::setprecision(9) << counts << std::endl;
    ui->lineEditYAxisMax->setText(QString::number(ymax*1.1));
    ui->lineEditYAxisMin->setText("0");
    //Histogram *tmphis2D = new Histogram( myProject->getExpFile(),std::stoi(myProject->getExp2DSpecID()) );
    //Dopisac automatyczne ustalanie wielkosci os Xi Y czyli Ysize i Xsize zeby
    // dobrze odczytywac widma
    Histogram tmp2DHist = Histogram( myProject->getExpFile(),std::stoi(myProject->getExp2DSpecID()) );
     myProject->setExp2DHist( tmp2DHist );
//Eva SPRAWDZIC    //mk    expHist->Rebin(BinningController::getBinningFactor());//10keV/ch

     ContaminationController *contaminationController = new ContaminationController();
     for (unsigned int i = 0; i !=  myProject->getInputContaminations().size(); i++)
     {
         int expId = std::stoi(myProject->getInputContaminations().at(i).at(0));
        string fileName = myProject->getInputContaminations().at(i).at(1);
        int hisId = std::stoi(myProject->getInputContaminations().at(i).at(3));
        double inten = std::stod(myProject->getInputContaminations().at(i).at(2));
        //additional binning/smoothening factor
        int smoothFactor = 0;
        if(myProject->getInputContaminations().at(i).size() > 4)
            smoothFactor = std::stoi(myProject->getInputContaminations().at(i).at(4));
        std::cout << "contam(file,ID): (" << fileName <<","<< hisId<< ")" << endl;

        contaminationController->addContamination(QString::number(expId),QString::fromStdString(fileName),
                                                  QString::number(hisId), QString::number(inten), smoothFactor);
     }
     contaminationController->SaveAll();
     delete contaminationController;

    slotUpdateContaminationPanel();     //filling contaminationTable withdata from the *.tas file

    // loading Decay path file
    string ss = myProject->getWorkingDir() + "/"+myProject->getInputDecayFile();  // full path has to be given

// polaczenie zprogramem p.Oli zamiast bezposrednim czytaniem z pliku
    createDecayInstance(ss);

    ResponseFunction* responseFunction = ResponseFunction::get();
    responseFunction->UpdateStructure();

    slotCalculateTotalRECSpectrum();
    emit signalUpdateSpecPlot();
    projectOpen_ = true;

  //  }
}

void MainWindow::loadDecayFile(bool trigered)
{
    Project* myProject = Project::get();
 if(!trigered)
 {
  QString fileName;
  fileName = QFileDialog::getOpenFileName(this,
      tr("Load Decay File"), QString::fromStdString(myProject->getWorkingDir()), tr("Decay path Files (*.ens,*.xml)"));
  qDebug() << fileName;
   ui->lineInputDecayFile->setText(fileName);   //update Project Panel
//MK-ADD cutting of directory name
   myProject->setInputDecayFile(fileName.toUtf8().constData()); // update global variable myProject

   // loading Decay file
   string ss = myProject->getWorkingDir() + myProject->getInputDecayFile();  // full path has to be given
   createDecayInstance(ss);
  }
}


void MainWindow::loadHISFile(bool trigered)
{
    Project* myProject = Project::get();

  qDebug() << "load experimental HIS file";
  QString expFileName;
  expFileName = QFileDialog::getOpenFileName(this,
      tr("Load histograms"), QString::fromStdString(myProject->getWorkingDir()), tr("HIS Files (*.his)"));
  qDebug() << expFileName;
  ui->lineExpFile->setText(expFileName);   //update Project Panel
  myProject->setExpFile(expFileName.toUtf8().constData());      // update singleton class myProject
  bool ok;
  QString qHistId = QInputDialog::getText(this, tr("Histogram ID to be read"), tr("HistID"),QLineEdit::Normal, "HistID",&ok);
  int hisId = qHistId.toInt();
  if(ok & (hisId!=0))
  {
      myProject->setExpSpecID(qHistId.toStdString());
  }

//  int hisId = std::stoi(myProject->getExpSpecID());

  //Histogram* tmphis = new Histogram(myProject->getExpFile(),hisId);
  myProject->setExpHist( Histogram(myProject->getExpFile(),hisId) );
//MS July 2020  expHist=myProject->getExpHist();
  emit signalUpdateSpecPlot();
}

/*Eva
void MainWindow::exportENSFile(bool trigered)
{
    qDebug() << " Exporting ENS file";
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save ENS file"), "",
        tr("ENS file (*.ens);;All Files (*)"));

    std::string filenameString = fileName.toUtf8().constData();
    Decay* decay= Decay::get();
    OutputFileController* outputFileController = new OutputFileController(decay);
    outputFileController->saveToFile(filenameString);

}
*/


void MainWindow::slotExportFiles(bool triggered)
{
    e1 = new ExportFiles();
    saveProject(false);    // always saves <ProjectName.tas> file
    e1->show();
}


void MainWindow::slotCalculate_Uncertainties(bool triggered)
{
    cout << "Calculate uncertainties started." << endl;
    Project* myProject = Project::get();
    Histogram* tempExpHist = myProject->getExpHist();
    double minEnergy = 0.;
    double maxEnergy = 16000.;
    double expSum = tempExpHist->GetNrOfCounts(minEnergy,maxEnergy);

    DecayPath* decayPath = DecayPath::get();
    std::vector<Nuclide>* nuclidesVector = decayPath->GetAllNuclides();
    Nuclide* motherNuclide = &nuclidesVector->at(0);
    motherNuclide->GetNuclideLevels()->at(0).CalculateBetaTransitionsUncertainties(expSum);
    cout << "Calculate uncertainties finished." << endl;
}


void MainWindow::slotPileupSignalBackground()
{
    PileupController *p1 = new PileupController(2);
    p1->show();
    connect(p1, SIGNAL(signalSavedAndAdded(bool)), this, SLOT(slotPileupAddContam(bool)));

}

void MainWindow::slotPileupAddContam(bool flag)
{
    slotUpdateContaminationPanel();
    slotCalculateTotalRECSpectrum();
    emit signalUpdateSpecPlot();
}

void MainWindow::slotPileupSignalSignal()
{
    PileupController *p1 = new PileupController(1);
    p1->show();
    connect(p1, SIGNAL(signalSavedAndAdded(bool)), this, SLOT(slotPileupAddContam(bool)));
}


void MainWindow::slotManualFit()
{
    Project* myProject = Project::get();

//    m1 = new ManualFitGraph();
// change background color if opened
//---------Table-------------

    std::vector <bool>  futureResults;
    std::vector <bool> displayStatus;
    QStringList header;
    std::vector<RowData> rowData_;

    header << "Display ?" << "Energy" << "Fitting Flag" << "Intensity[%]" ;
    m1->ManualFitGraph::setHeader(header);
    m1->show();

    DecayPath* decayPath= DecayPath::get();
//assuming one isotopes = nuclides
    std::vector <Level>* motherLevels = decayPath->GetAllNuclides()->at(0).GetNuclideLevels();
    std::vector<Transition*>transitionsUsed;
    std::vector<Transition*>* transitions_ = motherLevels->at(0).GetTransitions();
   for(std::vector<Transition*>::iterator it = transitions_->begin(); it != transitions_->end(); ++it)
    {
        futureResults.push_back(true);
        displayStatus.push_back(true);
        transitionsUsed.push_back(*it);
    }

    for(unsigned int i = 0; i != futureResults.size(); ++i)
    {
    QString QDisplayStatus_ = displayStatus.at(i) ? "true" : "false";
    Transition* tmpTransition = transitionsUsed.at(i);
    Level* currentLevel = tmpTransition->GetPointerToFinalLevel();
    QString QEnergy_ = QString::number(currentLevel->GetLevelEnergy());
    QString QIntensity_ = QString::number(tmpTransition->GetIntensity()*100);
    QString QFittingFlag_ = tmpTransition->GetIntensityFitFlag() ? "true" : "false";
    rowData_.push_back(RowData(QDisplayStatus_, QEnergy_, QFittingFlag_, QIntensity_));
    }
    m1->ManualFitGraph::initializeTable(rowData_);

//-------end of Table related code ---
    //calling to get the pointer again as it could havechange during recalculations
    //Histogram *recHist1 = myProject->getRecHist();
    //Histogram *difHist1 = myProject->getDifHist();

    QVector<double> x = QVector<double>::fromStdVector(myProject->getExpHist()->GetEnergyVectorD());
    QVector<double> y1 = QVector<double>::fromStdVector(myProject->getExpHist()->GetAllDataD());
    QVector<double> y2 = QVector<double>::fromStdVector(myProject->getRecHist()->GetAllDataD());
    QVector<double> x2 = QVector<double>::fromStdVector(myProject->getRecHist()->GetEnergyVectorD());
    QVector<double> d12 = QVector<double>::fromStdVector(myProject->getDifHist()->GetAllDataD());

    string respType = "l";
    m1->setResponseType(respType);
    m1->initializeGraphs();
    m1->showDataExpSimDiff(x, y1, x2, y2, d12);
    m1->showResponseFunctions();

    m1->show();

}

void MainWindow::updateHistogram()
{
    Project* myProject = Project::get();

    if(m1->isVisible())
    {
        QVector<double> x = QVector<double>::fromStdVector(myProject->getExpHist()->GetEnergyVectorD());
        QVector<double> y1 = QVector<double>::fromStdVector(myProject->getExpHist()->GetAllDataD());
        QVector<double> x2 = QVector<double>::fromStdVector(myProject->getRecHist()->GetEnergyVectorD());
        QVector<double> y2 = QVector<double>::fromStdVector(myProject->getRecHist()->GetAllDataD());
        QVector<double> d12 = QVector<double>::fromStdVector(myProject->getDifHist()->GetAllDataD());

        string respType = "l";
        m1->setResponseType(respType);
        m1->initializeGraphs();
        m1->showDataExpSimDiff(x, y1, x2, y2, d12);
        m1->showResponseFunctions();

    //-----spectra display code ------


    //----- end of spectra display code ------
        m1->show();
    }
}


void MainWindow::openHistogramGraph()
{
    Project* myProject = Project::get();
    w1 = new HistogramGraph();

//    float xMin = 0;
//    float xMax = expHist->GetNrOfBins();

    //Histogram *recHist1 = myProject->getRecHist();
    //Histogram *difHist1 = myProject->getDifHist();

    //MS July 2020    QVector<double> x = QVector<double>::fromStdVector(expHist->GetEnergyVectorD());
        QVector<double> x = QVector<double>::fromStdVector(myProject->getExpHist()->GetEnergyVectorD());
    //MS July 2020    QVector<double> y1 = QVector<double>::fromStdVector(expHist->GetAllDataD());
        QVector<double> y1 = QVector<double>::fromStdVector(myProject->getExpHist()->GetAllDataD());
    QVector<double> y2 = QVector<double>::fromStdVector(myProject->getRecHist()->GetAllDataD());
    QVector<double> d12 = QVector<double>::fromStdVector(myProject->getDifHist()->GetAllDataD());

    w1->showDataExpSimDiff(x, y1, y2, d12);
    w1->show();
}


std::vector<float> MainWindow::cutVector (std::vector<float> data, int min, int max)
{
//    if(max >= data.size())
//        throw Exception(":(");
    std::vector<float> cutData;
    cutData.insert (cutData.begin(),data.begin()+ min, data.begin()+ max );
    return cutData;
}
/*
void MainWindow::showLevelSchemePDF()
{
    QString levelSchemePDFFile;
    levelSchemePDFFile = QFileDialog::getOpenFileName(this,
        tr("Load Level Scheme PDF file"), QString::fromStdString(myProject->workingDir()), tr("HIS Files (*.pdf)"));
    qDebug() << levelSchemePDFFile;
    QDesktopServices::openUrl(QUrl::fromLocalFile(levelSchemePDFFile));
}
*/
void MainWindow::SetcomboBoxExpSpecID()
{
    Project* myProject = Project::get();
    vector<string> expSpecIDVec_ = myProject->getExpSpecIDVec();
    ui->comboBox_ExpSpecID->setToolTip("Only spectra defined in int input *.tas file");

    for(int i=0; i != expSpecIDVec_.size(); ++i)
    {
       ui->comboBox_ExpSpecID->addItem(QString::fromStdString(expSpecIDVec_.at(i)));
    }
}

void MainWindow::SetcomboBoxFit()
{
    ui->comboBoxFit1D->setToolTip("Please select fitting method for 1D spectra");
    ui->labelFitMethod->setToolTip("Please select fitting method for 1D spectra");

    for(int i = 0; i != fittingMethodList_.size(); ++i)
    {
        ui->comboBoxFit1D->addItem(fittingMethodList_.at(i));
        ui->comboBoxFit1D->setItemData(i, fittingMethodToolTips_.at(i), Qt::ToolTipRole);
    }
}

void MainWindow::slot1DFittingMethod(QString method)
{
    ui->frameFitParam->setEnabled(true);
    ui->lineEditFitLambda->setEnabled(false);
    ui->labelFitLambda->setEnabled(false);
    ui->lineEditFitHistId->setEnabled(false);
    ui->labelFitHistId->setEnabled(false);
    ui->labelFittingMethod->clear();
    ui->labelFitLambda->setToolTip("Fitting procedure parameter. Set close to 1 for small changes and far away from 1 for larger jumps");
    ui->labelFitHistId->setToolTip("Give semicolon (;) separated hist IDs for multi spectra fit.");
    set1DFittingMethod(method);
    switch(fittingMethod_)
    {
    case 1 : {
        ui->labelFittingMethod->setText("Maximum LikelyHood");
        ui->lineEditFitLambda->setEnabled(true);
        ui->labelFitLambda->setEnabled(true);
        break;
    }
    case 2 : {
        Project* myProject = Project::get();
        ui->labelFittingMethod->setText("Bayesian multi spec");
        ui->lineEditFitHistId->setEnabled(true);
        ui->labelFitHistId->setEnabled(true);
        ui->lineEditFitHistId->setText( QString::fromStdString(myProject->getExpSpecID()) );
        break;
    }
    default : {
        ui->labelFittingMethod->setText("--to be selected--");
        break;
    }
    }
}
