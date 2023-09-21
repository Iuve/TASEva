#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <project.h>
//#include <DeclareHeaders.hh>
#include <histogramgraph.h>
#include <manualfitgraph.h>
#include <calibrateenergy.h>
#include <decayPathEditor.h>
#include "decaySpectrum.h"
#include "analysis2d.h"
#include "status.h"
#include "exportFiles.h"

#include <QMainWindow>
#include <QtWidgets>
#include <QDebug>
#include <QLineEdit>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();
    static MainWindow* GetInstance(QWidget* parent = 0);
//    Histogram* expHist;  /**<  Holds experimental spectrum*/
//     Histogram* decHist;  /**<  Holds simulated spectrum*/
//    Histogram* simHist;  /**<  Holds simulated plus contaminations spectrum*/
//    Histogram* difHist;  /**<  Holds differential spectrum expHist-simHist */
//    vector<Histogram*> conHist;  /**<  Holds contaminations spectra*/
//    vector<double> conNorm;  /**<  Holds required normalization for the contamination spectra */
//mk-singleton    Decay* decay;

public slots:
    void openHistogramGraph();
    void openProject(bool trigered);
    void newProject(bool trigered);
    void saveProject(bool trigered);
//Eva    void slotEnergyCalibration(bool trigered); /**< SLOT to input calibration data and do the recalibration */
    void slotFWHMCalibration(bool trigered);    /** SLOT to recalibrate FWHM */
    void loadDecayFile(bool trigered);
    void loadHISFile(bool trigered);
    void slotShowStatus(bool);
//Eva    void exportDecayFile(bool trigered);
// mk   void exportXMLDecayFiles(bool triggered);
// mk   string checkAndPreparePath();
//mk    void exportRecSpec(bool trigered);
//mk    void exportDecayInfo(bool triggered);
 //mk   void exportResponseFunctions(bool triggered);
    void slotExportFiles(bool triggered);
//Eva    void showLevelSchemePDF();
    void slotProjectNameUpdate();
    void slotProjectExpFileUpdate();
    void slotProjectExpSpecIDUpdate();
    void slotProjectExpSpecIDUpdate(int ID);
    void slotProjectExp2DSpecIDUpdate();
    void slotProjectWorkingDirUpdate();
    void slotProjectInputDecayFileUpdate();
    void slotProjectOutputDecayFileUpdate();
    void slotProjectOutputSIMFileUpdate();
    void slotProjectOutputLEVFileUpdate();
    void slotProjectCodeGEANTUpdate();
    void slotProjectFitEnergyFrom();
    void slotProjectFitEnergyTo();
    void slotProjectFitLevelsFrom();
    void slotProjectFitLevelsTo();
    void slotProjectFitLambda();
    void slotProjectNoFitIterations();
    void slotProjectFitBinning();
    void slotPileupSignalSignal();
    void slotPileupSignalBackground();
    void slotCompareSpectra(bool);
    void slotEnergyCalibration(bool);


    void slotSetMainLogScale(bool checked);
    void slotSetAxisRange();
    void slotSetNormRange();

    void slotEditLevelSchemeClicked(bool checked); /**< SLOT to open/close LevelScheme Editor */
    void slotCalculateDECSpectrum();               /**< SLOT to start calculations of simulated spectrum */
    void slotCalculateTotalRECSpectrum();               /**< SLOT to calculate total "simulated" spectrum DECAY + CONTAMINATIONS*/
    void slotUpdateProjectPanel(bool checked);
    void slotRemoveContamination();             /**< SLOT to remove contamination from the list and table*/
    void slotAddContamination();                /**< SLOT to add a contamination to the table and list*/
//    void slotEditContamination();               /**< SLOT to edit parameters of the contamination entry  */
    void slotContaminationTableChanged(int xtab, int ytab);
    void slotContaminationTableClicked(int row, int column);
    void slotUpdateContaminationPanel();
    void slotUpdateContaminationData(int xtab, int ytab);  /**<  Updates data and plot after changes to the Contamination table has been made*/
    void slotClearContaminations();
    void slotPileupAddContam(bool flag);
    void slotUpdateSpecPlot();                             /**<  Updates spectrum on the main GUI panel */
    void slotSelectionChanged();
    string slotReadTableData(QTableWidget *table, int row, int column);
    void WriteTableData(QTableWidget *table, int row, int column, string item);
    void ClearTable( QTableWidget *table);
// Analisis 2D panel
    void slotOpen2DAnalysis();     /**< Opens 2D analysis panel*/
    void set1DFittingMethod(QString method){ fittingMethod_ = fittingMethodList_.indexOf(method);}
    void slot1DFittingMethod(QString method);
    //Fitting panel
    void slotAutoFit();         /**< Starts automatic fitting routine */
    void slotManualFit();      /**< Displays spectra with all level response functions and allows to change feedings with mouse clicks*/
    //Fitting panel -end
    void updateHistogram();

   void createDecayInstance(string DecayFileName); /**<  Creates a base for all the decay calculations*/
//    Histogram*  readHISFile(string hisfilename, int hisId);
   void cleanHistogram(Histogram* hist);
   void checkHistogram(Histogram* hist);
//mk   void saveLevelResp(int histId, QString outputFile);
   void CloseLevelSchemeEditor(bool); /**<  Closes level scheme editor - no data deleted from the memory*/
   void slotUpdateLevelEditor();
   void CloseExportFilesWindow(bool);
   void slotMakeDirsAndCheckFiles();
   void slotMakeSimulations();
   void slotUploadAndCalculateResponse();
   void slotUpdateResponseContainer();
   void slotDecayPathEdited();

private:
    Ui::MainWindow *ui;
    static MainWindow* mainInstance;
    HistogramGraph *w1;
    ManualFitGraph *m1;
    ManualFitGraph *specComp_ui;
    CalibrateEnergy *calE;
//Evaout    PileupController *p1;
    DecayPathEditor *t1;
    Analysis2D *a2D_ui;
    Status *s_ui;
    ExportFiles *e1;
//Eva    Calibration* cal_ui;
//Eva    HisDrr* inputFile;
    std::vector<float> *histDataFloat;
    int respFunNorm_;   /**< Keeps normalisation for the output response functions*/
//    DecaySpectrum *decaySpectrum;
//    Project *myProject = Project::get();
//    Histogram *expHist = myProject->getExpHist();  /**<  Holds experimental spectrum*/
//    Histogram *decHist = myProject->getDecHist();     /**<  Holds simulated spectrum*/
//    Histogram *recHist = myProject->getRecHist();  /**<  Holds simulated plus contaminations spectrum*/
//    Histogram *difHist = myProject->getDifHist();  /**<  Holds differential spectrum expHist-simHist */


    bool levelEditorOpen_;
    bool projectOpen_;
    QStringList fittingMethodList_ = {"select", "MaxLikelyHood" , "BayesianMultiSpec"};
    int fittingMethod_;
    QStringList fittingMethodToolTips_ = {"Please select a method",
                                          "Maximum Likelyhood method as used by A.Fijalkowska",
                                          "Bayesian method as described in P.Shuai paper on 88Rb"};

    std::vector<float> cutVector (std::vector<float> data, int min, int max);

    void OpenLevelSchemeEditor(); /**<  Opens new window with the level scheme editor*/
    void SetcomboBoxFit();  /**<  Sets table of 1D fitting methods to choose from*/
    void SetcomboBoxExpSpecID(); /**< Sets values of Experimental Spectrum combo BOX */
signals:
    void signalUpdateSpecPlot();
    void signalUpdateDecSpec();
    void signalXAxisSet();
    void signalUpdateContaminationPanel();
};

#endif // MAINWINDOW_H
