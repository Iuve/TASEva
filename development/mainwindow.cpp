#include <DeclareHeaders.hh>
#include "mainwindow.h"
#include "contaminationController.h"
#include "pileupController.h"
#include "fitController.h"
#include "responsefunction.h"
#include "histogramgraph.h"
#include "histogramOutputController.h"

//Eva #include "levelEditor.h"
//Eva #include "hisdrr.h"
//Eva #include "Exceptions.h"
//Eva #include "histogram.h"
//Eva #include "qlogging.h"
//Eva #include "histogramGraphController.h"
//Eva #include "outputFileController.h"
//------------------------------------
//Eva MS start from loadDecayData
//Eva #include "loadDecayData.h"
//-----------------------------
//Eva #include "betaFeedingFdition.h"
//Eva #include "AF.h"

#include "tablecontroller.h"

// GUI includes
#include "ui_mainwindow.h"
#include "ui_histogramgraph.h"
#include "ui_pileupController.h"
#include "ui_DecayPathEditor.h"
#include "ui_analysis2d.h"
#include "ui_status.h"
#include "ui_manualfitgraph.h"


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
//Eva out   decay = 0L;
//Eva    simHist = Histogram::GetEmptyHistogram();
//Eva    decHist = Histogram::GetEmptyHistogram();
    t1 = new DecayPathEditor();
    m1 = new ManualFitGraph();
//    specComp_ui = new ManualFitGraph();

    levelEditorOpen_ = false;
    projectOpen_ = false;
    s_ui = 0L;

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

    connect(ui->export_ENS_File, SIGNAL(triggered(bool)), this, SLOT(exportENSFile(bool)));
    connect(ui->exportXML_Decay_Files, SIGNAL(triggered(bool)), this, SLOT(exportXMLDecayFiles(bool)));
    connect(ui->exportRecHis, SIGNAL(triggered(bool)), this, SLOT(exportRecSpec(bool)));
//contaminations
    connect(ui->actionSignal_Signal, SIGNAL(triggered(bool)), this, SLOT(slotPileupSignalSignal()));
    connect(ui->actionSignal_Bkg, SIGNAL(triggered(bool)), this, SLOT(slotPileupSignalBackground()));
    connect(this, SIGNAL(signalUpdateContaminationPanel()), this, SLOT(slotUpdateContaminationPanel()));

 // Project data panel
    connect(ui->lineProjectName, SIGNAL(returnPressed()), this, SLOT(slotProjectNameUpdate()));
    connect(ui->lineWorkingDir, SIGNAL(returnPressed()), this, SLOT(slotProjectWorkingDirUpdate()));
    connect(ui->lineExpFile, SIGNAL(returnPressed()), this, SLOT(slotProjectExpFileUpdate()));
    connect(ui->lineExpFile, SIGNAL(returnPressed()), this, SLOT(slotUpdateSpecPlot()));  // not working expHis not read in updateslot

    connect(ui->lineExpSpecID, SIGNAL(returnPressed()), this, SLOT(slotProjectExpSpecIDUpdate()));
    connect(ui->lineExp2DSpecID, SIGNAL(returnPressed()), this, SLOT(slotProjectExp2DSpecIDUpdate()));
    connect(ui->lineInputDecayFile, SIGNAL(returnPressed()), this, SLOT(slotProjectInputDecayFileUpdate()));
    connect(ui->lineOutputDecayFile, SIGNAL(returnPressed()), this, SLOT(slotProjectOutputDecayFileUpdate()));
    connect(ui->lineOutputSIMFile, SIGNAL(returnPressed()), this, SLOT(slotProjectOutputSIMFileUpdate()));
    connect(ui->lineOutputLEVFile, SIGNAL(returnPressed()), this, SLOT(slotProjectOutputLEVFileUpdate()));
    connect(ui->lineCodeGEANT, SIGNAL(returnPressed()), this, SLOT(slotProjectCodeGEANTUpdate(bool)));

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

    ui->buttonMakeDirsAndCheckFiles->setEnabled(false);
    ui->buttonMakeSimulations->setEnabled(false);
    ui->buttonUploadAndCalculateResponse->setEnabled(false);
    ui->buttonCalculateSIMSpec->setEnabled(false);
    ui->buttonUpdateResponseContainer->setEnabled(false);
    ui->frame_2->setEnabled(false);
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


// Contaminations Panel
    connect(ui->tableContamination,SIGNAL(cellChanged(int,int)), SLOT(slotContaminationTableChanged(int,int)));
    connect(ui->tableContamination,SIGNAL(cellClicked(int,int)), SLOT(slotContaminationTableClicked(int,int)));

//    connect(ui->tableContamination,SIGNAL(cellChanged(int,int)), SLOT(slotUpdateContaminationData(int,int)));


 //   QStringList header;
 //   header << "File Name" << "Norm (%)" << "free param" << "histID" ;
 //   ui->tableContamination->setHorizontalHeaderLabels(header);



    //2D Analysis panel
    connect(ui->buttonAnalysis2D, SIGNAL(clicked()), this, SLOT(slotOpen2DAnalysis()));

    connect(m1, SIGNAL(signalRecalculateLevel()), this, SLOT(slotCalculateDECSpectrum()));
//    connect(this, SIGNAL(signalUpdateSpecPlot()), m1, SLOT(takesMySignal()));
    connect(this, SIGNAL(signalUpdateSpecPlot()), this, SLOT(updateHistogram()));

    //Fitting Panel
    ui->buttonAutoFit->setEnabled(false);
    ui->buttonErrorCal->setEnabled(false);
    ui->buttonAnalysis2D->setEnabled(false);
    ui->buttonManualFit->setEnabled(false);
    connect(ui->buttonAutoFit, SIGNAL(clicked(bool)), this, SLOT(slotAutoFit()));
    connect(ui->buttonManualFit, SIGNAL(clicked(bool)), this, SLOT(slotManualFit()));
    connect(ui->lineEditFitEnergyFrom, SIGNAL(returnPressed()), this, SLOT(slotProjectFitEnergyFrom()));
    connect(ui->lineEditFitEnergyTo, SIGNAL(returnPressed()), this, SLOT(slotProjectFitEnergyTo()));
    connect(ui->lineEditFitLevelsFrom, SIGNAL(returnPressed()), this, SLOT(slotProjectFitLevelsFrom()));
    connect(ui->lineEditFitLevelsTo, SIGNAL(returnPressed()), this, SLOT(slotProjectFitLevelsTo()));
    connect(ui->lineEditFitLambda, SIGNAL(returnPressed()), this, SLOT(slotProjectFitLambda()));
    connect(ui->lineEditNoFitIterations, SIGNAL(returnPressed()), this, SLOT(slotProjectNoFitIterations()));
    connect(ui->lineEditFitEnergyFrom, SIGNAL(editingFinished()), this, SLOT(slotProjectFitEnergyFrom()));
    connect(ui->lineEditFitEnergyTo, SIGNAL(editingFinished()), this, SLOT(slotProjectFitEnergyTo()));
    connect(ui->lineEditFitLevelsFrom, SIGNAL(editingFinished()), this, SLOT(slotProjectFitLevelsFrom()));
    connect(ui->lineEditFitLevelsTo, SIGNAL(editingFinished()), this, SLOT(slotProjectFitLevelsTo()));
    connect(ui->lineEditFitLambda, SIGNAL(editingFinished()), this, SLOT(slotProjectFitLambda()));
    connect(ui->lineEditNoFitIterations, SIGNAL(editingFinished()),this, SLOT(slotProjectNoFitIterations()));
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
    std::cout << "MAINWINDOW INSTANCE  w kreowaniue" << mainInstance <<std::endl;
    return mainInstance;
}


void MainWindow::createDecayInstance(const string DecayFileName)
{
     std::cout<< "Created decay Instance..." << DecayFileName<<std::endl;
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
   cout << "QValue: "<< decayPath->GetAllNuclides()->at(0).GetNuclideLevels()->at(0).GetTransitions()->at(0)->GetTransitionQValue() << endl;
}


/* Eva
void MainWindow::slotEnergyCalibration(bool trigered)
{
   cal_ui = new Calibration();
   cal_ui->show();
//MainWindow::slotEnergyCalibration()
}

void MainWindow::slotFWHMCalibration(bool trigered)
{

}
*/

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
    std::vector<string> simID = {"6300","6310","6320","6330","6340"};

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
    specComp_ui->setYValues(y1);
    specComp_ui->setY2Values(y2);
    specComp_ui->setDiffValues(d12);
    //    specComp_ui->showDataExpSimDiff(x, y1, y2, d12);
    specComp_ui->showResponseFunctions();
//-----spectra display code ------

    specComp_ui->show();
}

void MainWindow::slotShowStatus(bool trigered)
{
//    cout << "-------start slotShowStatus--------" << endl;
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
//    std::cout << "MainWindow::slotAddContamination" << endl;
    QString qFileName;
    QString qHistId ;
    QString qIntensity;
    bool ok;
     QString text = QInputDialog::getText(this, tr("New Contamination"),
                                           tr("FileName : HistID : Intensity(%)"), QLineEdit::Normal,
                                           "FileName : HistID : Intensity", &ok);
     QStringList stringList= text.split(":",QString::SkipEmptyParts);
//     for (int i = 0; i < stringList.size(); i++) qDebug(stringList.at(i).toUtf8());
//     qDebug()<<stringList;
     if(ok && stringList.size() == 3)
     {
         qFileName = stringList.at(0);
         qHistId = stringList.at(1);
         qIntensity = stringList.at(2);
     } else
     {
         QMessageBox msgBox;
         msgBox.setText("Data not correct - no changes applied");
         msgBox.exec();
         return;
     }
    ContaminationController *contaminationController = new ContaminationController();
    contaminationController->addContamination(qFileName, qHistId, qIntensity);
    contaminationController->SaveAll();
    delete contaminationController;

    slotUpdateContaminationPanel();
    slotCalculateTotalRECSpectrum();
    emit signalUpdateSpecPlot();
}


void MainWindow::slotRemoveContamination()
{
    Project* myProject = Project::get();

    std::cout << "MainWindow::slotRemoveContamination" << endl;
    QString Qfilename_;
    QString QId_ ;
     QModelIndexList selection = ui->tableContamination->selectionModel()->selectedRows();

     // Multiple rows can be selected mk-not finnished implementation look at energy variable

     for(int i=0; i< selection.count(); i++)
     {
        cout << "in selection " << i << endl;
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
//    int     histId = ui->lineEditFitHistId->text().toInt();


     if(maxEnergy<=minEnergy)
     {
         int r = QMessageBox::warning(this, tr("Error"),
                                      tr("Max energy must be higher than min energy"),
                                      QMessageBox::Ok);
         if (r == QMessageBox::Ok)
             return;
     }

    try
     {
//MS June 2020         checkHistogram(expHist);
         checkHistogram( myProject->getExpHist() );
     }
     catch(GenError e)
     {
         return;
     }


     FitController* fitController = new FitController();
//MS June 2020     fitController->applyFit(expHist);
     fitController->applyFit( myProject->getExpHist() );
     ui->buttonErrorCal->setEnabled(true);
     std::vector <float> errors = fitController->getErrors();
     std::vector< std::pair<double, double> > fitResults;

//Eva     std::vector<Level*> levels = decay->GetAllLevels();

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
    int i=-1;
     cout << "Level energy  Feeding(%)  Uncertainty"<< endl;
     for(auto itt = transitions->begin(); itt != transitions->end(); ++itt)
     {
         i++;
         cout << (*itt) -> GetFinalLevelEnergy() << " "
         << (*itt) -> GetIntensity()*100
         << " " << errors.at(i) << endl;
         fitResults.emplace_back( (*itt) -> GetIntensity()*100, errors.at(i) );
     }
     myProject->SetLastAutofitResults(fitResults);
     cout << "FIT RESULT: CONTAMINATION" << endl;
     cout << "<File name>  :  intensity " << endl;
//    for(unsigned int i = 0; i < contaminationController_->getContaminations().size(); i++)

     std::vector<Contamination>* contaminations = myProject->getContaminations();
    for(unsigned int i = 0; i < contaminations->size(); i++)
    {
        std::cout << contaminations->at(i).filename << " : " <<
                     contaminations->at(i).intensity << std::endl;
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
    ui->buttonEditLevelScheme->setText("Decay path editor OPENED");

    QStringList header;
//    int tabIndex = t1->uiT->tabDecay->currentIndex();
    std::vector<RowData> rowDataMother_;
    std::vector<RowData> rowDataDaughter_;
    std::vector<RowData> rowDataGrandDaughter_;

    DecayPath* decayPath= DecayPath::get();
    TableController *tableController_ = new TableController();

//    cout << "QValue: "<< decayPath->GetAllNuclides()->at(tabIndex).GetNuclideLevels()->at(0).GetTransitions()->at(0)->GetTransitionQValue() << endl;

//    t1->show();

    std::vector<Nuclide>* nuclides_=decayPath->GetAllNuclides();
    for(unsigned int n =0; n !=nuclides_->size(); ++n)
    {
        std::vector<Level>* levels_ = nuclides_->at(n).GetNuclideLevels();
        for (unsigned int i = 0; i!=levels_->size(); ++i)
        {
            QString QEnergy_ = QString::number(levels_->at(i).GetLevelEnergy());
            QString QIntensity_ = QString::number(levels_->at(i).GetLevelEnergy());
            QString QTransitionsFrom_ = QString::number(levels_->at(i).GetTransitions()->size());
            QString QTransitionsTo_ = QString::number(levels_->at(i).GetLevelEnergy());

            if(n == 0)rowDataMother_.push_back(RowData(QEnergy_, QIntensity_, QTransitionsFrom_, QTransitionsTo_));
            if(n == 1)rowDataDaughter_.push_back(RowData(QEnergy_, QIntensity_, QTransitionsFrom_, QTransitionsTo_));
            if(n == 2)rowDataGrandDaughter_.push_back(RowData(QEnergy_, QIntensity_, QTransitionsFrom_, QTransitionsTo_));
            if(n > 2)cout <<" TOO many nuclides cutting to 3" << endl;

        }
    }
       tableController_->initializeTable(t1->uiT->tableMotherLevels,rowDataMother_);
       tableController_->initializeTable(t1->uiT->tableDaughterLevels,rowDataDaughter_);
       tableController_->initializeTable(t1->uiT->tableGrandDaughterLevels,rowDataGrandDaughter_);
       header << "Energy" << "Beta Feeding [%]" << "#Transitions from" << "# Transitions to";
       tableController_->setHeader(t1->uiT->tableMotherLevels,header);
       tableController_->setHeader(t1->uiT->tableDaughterLevels,header);
       tableController_->setHeader(t1->uiT->tableGrandDaughterLevels,header);

       t1->show();

}

void MainWindow::slotUpdateLevelEditor()
{
    cout <<"slotUpdateLevelEditor" << endl;
    OpenLevelSchemeEditor();
}

void MainWindow::CloseLevelSchemeEditor(bool)
{
    ui->buttonEditLevelScheme->setText("Open decay path editor");
    ui->buttonEditLevelScheme->setChecked(false);
    t1->close();
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
    std::cout << "Make directories and check files button pushed." << endl;
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
    std::cout << "Make simulations button pressed." << endl;

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
    std::cout << "Upload and calculate response button pushed." << endl;

    ResponseFunction* responseFunction = ResponseFunction::get();
    responseFunction->CalculateAllLevelsRespFunction();

    if( responseFunction->GetResponseFunctionReady() )
    {
        ui->buttonUploadAndCalculateResponse->setEnabled(false);
        ui->buttonCalculateSIMSpec->setEnabled(true);
    }
}

void MainWindow::slotUpdateResponseContainer()
{
    std::cout << "Update response container button pressed." << endl;

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
    std::cout << "---------------DECAY spectrum calculation---------------" << std::endl;

    Project* myProject = Project::get();
    ResponseFunction* responseFunction = ResponseFunction::get();
    DecayPath* decayPath = DecayPath::get();

    if( !(responseFunction->GetResponseFunctionReady()) )
        responseFunction->CalculateAllLevelsRespFunction();

    int histId = std::stoi(myProject->getExpSpecID());
    Level* motherLevel = &decayPath->GetAllNuclides()->at(0).GetNuclideLevels()->at(0);

    Histogram tmpDecHist = new Histogram();
    tmpDecHist = *responseFunction->GetResponseFunction(motherLevel, histId);

    ui->frame_2->setEnabled(true);
    ui->buttonAutoFit->setEnabled(true);
    ui->buttonAnalysis2D->setEnabled(true);
    ui->buttonManualFit->setEnabled(true);
    ui->actionCompare_Spectra->setEnabled(true);

       std::vector<Contamination>* contaminations = myProject->getContaminations();
        float sumNormCont = 0.0;
        for (unsigned int i = 0; i !=  contaminations->size(); i++)
        {
            sumNormCont += contaminations->at(i).intensity;
        }
        float norm = myProject->getExpHist()->GetNrOfCounts()*(1-sumNormCont);
        std::cout << " Normalize DECAY spectrum to: " << norm <<std::endl;

        tmpDecHist.Normalize(norm);

        myProject->setDecHist( tmpDecHist );

     slotCalculateTotalRECSpectrum();
}


void MainWindow::slotCalculateTotalRECSpectrum()
{

    Project* myProject = Project::get();

    cout << "---------------Calculating TOTAL reconstructed spectrum--------------- " << endl;
//Evaout    Histogram *recHist = Histogram::GetEmptyHistogram();  //why we crate rec his jest zdefiniowane w proj.h
//     recHist = Histogram::GetEmptyHistogram();

    Histogram* recHist = Histogram::GetEmptyHistogram();
    Histogram* difHist = Histogram::GetEmptyHistogram();
    Histogram* decHist = myProject->getDecHist();

    if(decHist != 0L)
    {
        recHist->Add( decHist, 1.);
    }

    std::vector<Contamination> contaminations = *(myProject->getContaminations());
    cout << "Contaminations to be added = " << contaminations.size() <<endl;
    for (unsigned int i = 0; i !=  contaminations.size(); i++)
    {
    std::cout << "i= " << i << " " << contaminations.at(i).filename
                 << " " << &contaminations.at(i).hist
                 << " " << contaminations.at(i).intensity
                 << " " << contaminations.at(i).normalization
                 << " " << contaminations.at(i).hist.GetNrOfCounts()
              << std::endl;

     double fact = contaminations.at(i).intensity * myProject->getExpHist()->GetNrOfCounts();
         Histogram *tmpCont = new Histogram(contaminations.at(i).hist);
         tmpCont->Normalize(fact);
         recHist->Add(tmpCont, 1.0);
         delete tmpCont;
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
//MS July 2020     QString qstr = "NoC in Exp. spec. within limits: "+ QString::number(expHist->GetNrOfCounts(xMinEn,xMaxEn));
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


void MainWindow::slotUpdateContaminationPanel()
{
    Project* myProject = Project::get();

    QStringList header;
    header << "File Name" << "Norm (%)" << "free param" << "histID" ;
    ui->tableContamination->setHorizontalHeaderLabels(header);


    std::vector<Contamination> contaminations = *(myProject->getContaminations());
    {
    string str;
//    std:: cout << "contaminations size " << contaminations.size() << endl;
    for(unsigned int row = 0; row != contaminations.size(); ++row )
    {
        for (int column = 0; column != 4; ++column)
        {
            if (column == 0) {
                string fileName = contaminations.at(row).filename;
                int dirLength = myProject->getWorkingDir().length();
                if (fileName.substr(0,dirLength) == myProject->getWorkingDir()){
                    int a = dirLength -   fileName.length();
                str = contaminations.at(row).filename.substr(dirLength,a) + ".his" ;
//            cout << "fileName : " <<fileName;
//            cout << "dirLength: " << dirLength;
//            cout << "filename-str: " << str;
                }
                else {str = contaminations.at(row).filename;}
            }

            if (column == 3) str = std::to_string(contaminations.at(row).id);
            if (column == 1) str = std::to_string(contaminations.at(row).intensity*100) ;
            if (column == 2) str =contaminations.at(row).GetIntensityFitFlag() ? "true" : "false";
            cout << "string to be added" << str << endl;
            WriteTableData(ui->tableContamination, row, column, str);
        }
    }
    }
    for(unsigned int row = 0; row != contaminations.size(); ++row )
     {
      for (int column = 0; column != 4; ++column)
        {
            slotUpdateContaminationData(row, column);
        }
     }

}


void MainWindow::slotUpdateContaminationData(int xtab, int ytab)
{
    Project* myProject = Project::get();

    int row = xtab;
    int column = ytab;
    string str = slotReadTableData(ui->tableContamination, row, column);
    std::vector<Contamination> contaminations = *(myProject->getContaminations());
    bool readNewSpectrum_ = false;
    if (column == 0)
    {
        if (str != contaminations.at(row).filename)
        {
          contaminations.at(row).filename = str;
          readNewSpectrum_ = true;
         }
    }
    else if (column == 1)
    {
        if (str != std::to_string(contaminations.at(row).intensity*100))
        {
           contaminations.at(row).intensity = stof(str)/100;
           double value = contaminations.at(row).intensity * myProject->getExpHist()->GetNrOfCounts();
           contaminations.at(row).normalization =  value;
        }
    }
    else if (column == 3)
        {
           if (str != std::to_string(contaminations.at(row).id))
           {
            contaminations.at(row).id = stoi(str);
            readNewSpectrum_ = true;
           }
        }
    else if (column == 2)
        {
            if(str != std::to_string(contaminations.at(row).GetIntensityFitFlag()))
            {
                bool flag;
                if (str == "true")flag = true;
                if (str == "flase")flag = false;
                contaminations.at(row).SetIntensityFitFlag(flag);
            }
        }
     else   std::cout << "ERROR IN TABLE " << std::endl;


    myProject->setOneContamination(row,contaminations.at(row)); // zmiana contaminations w myProject

    if (readNewSpectrum_)
    {
       string fileName = contaminations.at(row).filename;
        int histId =contaminations.at(row).id;
        Histogram *tmpHist = new Histogram(fileName,histId);
        contaminations.at(row).hist = *tmpHist;
        myProject->setOneContamination(row,contaminations.at(row)); // efektywniezmiana widma w contaminations w myProject
        delete tmpHist;
        readNewSpectrum_ =false;
    }
}

void MainWindow::slotContaminationTableClicked(int row, int column)
{
//    std::cout << "MainWindow::ContaminationTableClicked row " << row << " column " << column << std::endl;
    if(column != 2) return;
    if(column == 2){
//        int row = xtab;
//        int column = ytab;
        Project* myProject = Project::get();

        string str = slotReadTableData(ui->tableContamination, row, column);
        std::vector<Contamination>* contaminations_ = myProject->getContaminations();
        bool flag;
        if (str == "true")flag = true;
        if (str == "false")flag = false;
        flag = !flag;
        contaminations_->at(row).SetIntensityFitFlag(flag);
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
    ui->lineExpSpecID->setText(QString::fromStdString(myProject->getExpSpecID()));
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

}

void MainWindow::slotProjectNameUpdate(){Project* myProject = Project::get(); myProject->setProjectName(ui->lineProjectName->text().toUtf8().constData()); }
void MainWindow::slotProjectWorkingDirUpdate()  {Project* myProject = Project::get(); myProject->setWorkingDir(ui->lineWorkingDir->text().toUtf8().constData()); }
void MainWindow::slotProjectExpFileUpdate()  {Project* myProject = Project::get();
                                              myProject->setExpFile(ui->lineExpFile->text().toUtf8().constData());}
void MainWindow::slotProjectExpSpecIDUpdate()  {Project* myProject = Project::get();
                                                myProject->setExpSpecID(ui->lineExpSpecID->text().toUtf8().constData());
                                                myProject->setExpHist();
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
        }


//MS July 2020    QVector<double> x = QVector<double>::fromStdVector(expHist->GetEnergyVectorD());
        QVector<double> x = QVector<double>::fromStdVector(myProject->getExpHist()->GetEnergyVectorD());
//MS July 2020    QVector<double> y1 = QVector<double>::fromStdVector(expHist->GetAllDataD());
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
        ui->specPlot->graph(1)->setData(x, y3);
    }


//    cout << "before read " << endl;

    if(myProject->getRecHist()!= 0L)
    {
        //Histogram *recHist1 = myProject->getRecHist();
        QVector<double> x2 = QVector<double>::fromStdVector(myProject->getRecHist()->GetEnergyVectorD());
        QVector<double> y2 = QVector<double>::fromStdVector(myProject->getRecHist()->GetAllDataD());

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
    std::vector<Contamination>* contaminations = myProject->getContaminations();
    for (unsigned int i = 0; i !=  contaminations->size(); i++)
    {

        Histogram tmpHist = contaminations->at(i).hist;
        //double xMax = tmpHist.GetXMax();
        //double xMin = tmpHist.GetXMin();
        //double aa = contaminations->at(i).intensity * 1;
//        std::cout << "INtensity przed normalize w plot: " << aa << endl;
//MS July 2020        double norm = contaminations.at(i)->intensity * expHist->GetNrOfCounts();  //expHist->GetNrOfCounts(xMin,xMax);
        double norm = contaminations->at(i).intensity * myProject->getExpHist()->GetNrOfCounts();
        tmpHist.Normalize(norm); // ,xMin,xMax);
        QVector<double> x4 = QVector<double>::fromStdVector(tmpHist.GetEnergyVectorD());
        QVector<double> y4 = QVector<double>::fromStdVector(tmpHist.GetAllDataD());

        ui->specPlot->addGraph();
// set style for the graph
        string conName ="Contamination " + std::to_string(i+1);
        ui->specPlot->graph(i+3)->setName(QString::fromStdString(conName));
        ui->specPlot->graph(i+3)->setPen(bluePen);
        ui->specPlot->graph(i+3)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
        ui->specPlot->graph(i+3)->selectable();
        ui->specPlot->graph(i+3)->selectionDecorator()->setPen(QPen(Qt::yellow));
// set data for the graph
        ui->specPlot->graph(i+3)->setData(x4, y4);

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

    if(!trigered)
    {
//    QMessageBox::information(this, "New Project", "Please fill in Project Frame data", QMessageBox::Ok, QMessageBox::Ok);

    QString qfileName;
    qfileName = QFileDialog::getSaveFileName(this, tr("Save project to file") );
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

    if(!trigered)
    {
//        const QSignalBlocker blocker(ui->tableContamination);
    QString qfileName;
    qfileName = QFileDialog::getOpenFileName(this,
        tr("Load MTAS Project File"), QString::fromStdString(myProject->getWorkingDir()), tr("MTAS project files (*.tas)"));
//    qDebug() << "Project file name: " << qfileName;

    if(qfileName.isEmpty()&& qfileName.isNull())
    {
    return;
    }

    myProject->New();
    slotClearContaminations();

    string sfileName = qfileName.toStdString();
    myProject->Open(sfileName);

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

//    int hisId = std::stoi(myProject->getExpSpecID());
//    Histogram tmp = myProject->getHistFromExpMap(hisId);
//    myProject->setExpHist( tmp );
    myProject->setExpHist();

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
     for (unsigned int i = 0; i !=  myProject->getInputContaminations().size(); i++){
        string fileName = myProject->getInputContaminations().at(i).at(0) ;
        int hisId = std::stoi(myProject->getInputContaminations().at(i).at(2));
        double inten = std::stod(myProject->getInputContaminations().at(i).at(1));
        std::cout << "contam(file,ID): (" << fileName <<","<< hisId<< ")" << endl;

        contaminationController->addContamination(QString::fromStdString(fileName), QString::number(hisId), QString::number(inten));
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

    }
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
  if(ok & hisId!=0)
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

string MainWindow::checkAndPreparePath()
{
    QDateTime now = QDateTime::currentDateTime();
    const QString timestamp = now.toString(QLatin1String("yyyyMMdd-hhmm"));
    QDir newDir(timestamp);
    qDebug() << newDir.exists();
//   if(!QDir::exists(timestamp))
//    {
    QDir().mkdir(timestamp);
    qDebug() << timestamp;
//    }
    QDir currentDir = QDir::current();
    string currentPath = currentDir.absolutePath().toStdString();
    cout << currentPath << endl;
    string path = currentPath + "/" + timestamp.toStdString();
    cout << path << endl;
    return path;
}

void MainWindow::exportXMLDecayFiles(bool triggered)
{
    string path = checkAndPreparePath();
    cout << path << endl;
    SaveDecayData* outputXMLfiles = new SaveDecayData(path);
    outputXMLfiles->SaveDecayStructure();
    delete outputXMLfiles;

}


void MainWindow::exportRecSpec(bool trigered)
{
    Project* myProject = Project::get();
    qDebug() << "Exporting Reconstructed HIS spectrum";
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Simulated spectrum HIS file"), "",
        tr("HIS file (*.his);;All Files (*)"));

    cout << "pozycja .his" << fileName.lastIndexOf(".his",-1, Qt::CaseInsensitive) << endl;

    int p =  fileName.lastIndexOf(".his",-1, Qt::CaseInsensitive);
    qDebug() << "filename: " << fileName.left(p) ;
    fileName = fileName.left(p); //It is a full path not just a filename.

    int histId = std::stoi(myProject->getExpSpecID());
    saveLevelResp(histId, fileName);  // All components should be included not ONLY simulated

}


void MainWindow::saveLevelResp(int histId, QString outputFile)
{
    DecayPath* decayPath_= DecayPath::get();
    ResponseFunction* responseFunction_ = ResponseFunction::get();
    Project* myProject = Project::get();
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
    cout << "saveLevelResp 1" << endl;


    std::vector<Nuclide>* nuclides_ = decayPath_->GetAllNuclides();
    std::vector<Level>* motherLevels_ =nuclides_->at(0).GetNuclideLevels();
    Level *motherLevel_ = &(motherLevels_->at(0));
    std::vector<Transition*>* betaTransitions = motherLevels_->at(0).GetTransitions();

    cout << "saveLevelResp 2" << endl;
    int nrOfTransitions = betaTransitions->size();
    int nrOfSpectra;
    nrOfSpectra = 1 + nrOfTransitions;
    HistogramOutputController *levelsOutController = new HistogramOutputController(outputFile.toStdString(), nrOfSpectra, histId);
    cout << "saveLevelResp 3" << endl;
//mk  add  calculateHistRespFunc(histId);
    cout << "saveLevelResp 4" << endl;

    levelsOutController->saveHistogram(myProject->getRecHist(),histId);
//    float norm_=myProject->getExpHist()->GetNrOfCounts();
    cout << "saveLevelResp 5" << endl;
//    cout << "norm = " << norm_ << endl;
    int i=1;
    int sumNormCont = 0; // normalizacja calkowita kontaminacji
    for (std::vector<Transition*>::iterator it = betaTransitions->begin(); it !=  betaTransitions->end(); it++)
    {
        cout << "saveLevelResp 6 " << i << endl;
        Histogram* tmpHist = new Histogram(responseFunction_->GetLevelRespFunction( (*it)->GetPointerToFinalLevel(), histId ));
        double xMax = tmpHist->GetXMax();
        double xMin = tmpHist->GetXMin();
//        cout << " number of counts in exp: " << myProject->getExpHist()->GetNrOfCounts() << endl;
//        cout << " number of counts in resp fun: " << tmpHist->GetNrOfCounts() << endl;
//        cout << " number of counts in dec: " << myProject->getDecHist()->GetNrOfCounts() << endl;
        cout << "saveLevelResp 7 " << i << endl;
        double norm_ = (*it)->GetIntensity() * myProject->getExpHist()->GetNrOfCounts() *(1-sumNormCont);
        std::cout<< norm_ << " W SAve" << std::endl;
       tmpHist->Scale(norm_);
        cout << " number of counts in resp fun after NORM: " << tmpHist->GetNrOfCounts() << endl;
        cout << "saveLevelResp 8 " << i << endl;
        levelsOutController->saveHistogram(tmpHist,i);
        i=i+1;
        cout << "saveLevelResp 9 " << i << endl;
    }

   cout << "ALL levels saved" << endl;

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
//out    for(std::vector<Level*>::iterator it = motherLevels.begin(); it != levels.end(); ++it)
    for(std::vector<Transition*>::iterator it = transitions_->begin(); it != transitions_->end(); ++it)
    {
        futureResults.push_back(true);
        displayStatus.push_back(true);
        transitionsUsed.push_back(*it);
    }

    for(unsigned int i = 0; i != futureResults.size(); ++i)
    {
//       std::cout<< "test levelschemeEditor Energy, feeding " << levels.at(i)->GetEnergy() << " ; " <<
//           levels.at(i)->GetBetaFeedingFunction() <<std::endl;
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

//MS July 2020    QVector<double> x = QVector<double>::fromStdVector(expHist->GetEnergyVectorD());
    QVector<double> x = QVector<double>::fromStdVector(myProject->getExpHist()->GetEnergyVectorD());
//MS July 2020    QVector<double> y1 = QVector<double>::fromStdVector(expHist->GetAllDataD());
    QVector<double> y1 = QVector<double>::fromStdVector(myProject->getExpHist()->GetAllDataD());
    QVector<double> y2 = QVector<double>::fromStdVector(myProject->getRecHist()->GetAllDataD());
    QVector<double> d12 = QVector<double>::fromStdVector(myProject->getDifHist()->GetAllDataD());

    string respType = "l";
    m1->setResponseType(respType);
    m1->initializeGraphs();
    m1->showDataExpSimDiff(x, y1, y2, d12);
    m1->showResponseFunctions();

    m1->show();

}

void MainWindow::updateHistogram()
{
    Project* myProject = Project::get();

    if(m1->isVisible())
    {
        //Histogram *recHist1 = myProject->getRecHist();
        //Histogram *difHist1 = myProject->getDifHist();

    //MS July 2020    QVector<double> x = QVector<double>::fromStdVector(expHist->GetEnergyVectorD());
        QVector<double> x = QVector<double>::fromStdVector(myProject->getExpHist()->GetEnergyVectorD());
    //MS July 2020    QVector<double> y1 = QVector<double>::fromStdVector(expHist->GetAllDataD());
        QVector<double> y1 = QVector<double>::fromStdVector(myProject->getExpHist()->GetAllDataD());
        QVector<double> y2 = QVector<double>::fromStdVector(myProject->getRecHist()->GetAllDataD());
        QVector<double> d12 = QVector<double>::fromStdVector(myProject->getDifHist()->GetAllDataD());

        string respType = "l";
        m1->setResponseType(respType);
        m1->initializeGraphs();
        m1->showDataExpSimDiff(x, y1, y2, d12);
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
    std::cout<< "cutvector" << std::endl;
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


