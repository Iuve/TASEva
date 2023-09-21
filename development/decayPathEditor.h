#ifndef DECAYPATHEDITOR_H
#define DECAYPATHEDITOR_H

#include "rowData.h"
#include "project.h"

#include <string>
#include <iostream>

#include <QMessageBox>
#include <QApplication>
#include <QWidget>
#include <QTableWidget>

class PseudoLevelsController;
class TransitionEditor;

class DialogOptionsWidget;
//class Level;

namespace Ui {
class DecayPathEditor;
class MainWindow;
}

class DecayPathEditor : public QWidget
{
    Q_OBJECT

public:
    explicit DecayPathEditor(QWidget *parent = 0);
    ~DecayPathEditor();
    Ui::DecayPathEditor *uiT;
  //EVa  void update();
    double getPseudoLevelEnergyStep() {return pseudoLevelEnergyStep_;}
    double getPseudoLevelEnergyMin() {return pseudoLevelEnergyMin_;}
    double getPseudoLevelEnergyMax() {return pseudoLevelEnergyMax_;}
    double getPseudoLevelTotInten() {return pseudoLevelTotInten_;}

    void setPseudoLevelEnergyStep(double d) {pseudoLevelEnergyStep_ = d;}
    void setPseudoLevelEnergyMin(double d) {pseudoLevelEnergyMin_ = d;}
    void setPseudoLevelEnergyMax(double d) {pseudoLevelEnergyMax_ = d;}
    void setPseudoLevelTotInten(double d) { pseudoLevelTotInten_ = d; }

    int getCurrentLevel(int i);




public slots:
    void slotEditTransitions(int tableIndex, int rowIndex);
    void slotEditMTransitions(int rowIndex){return slotEditTransitions(0, rowIndex);}
    void slotEditDTransitions(int rowIndex){return slotEditTransitions(1, rowIndex);}
    void slotEditGDTransitions(int rowIndex){return slotEditTransitions(2, rowIndex);}
//EVa    void slotUpdateFeedingData(int row, int column);
//EVa    void slotRecalculateDECSpectrum(bool);
   void slotDaughterTableCellClicked(int row, int column);
   void slotFittingStatusTrue(){setColumnStatus(true, 2);}
   void slotFittingStatusFalse(){setColumnStatus(false, 2);}
   void setColumnStatus(bool status, int column);
   void slotStatusClicked(bool status, int row, int column);
    void slotNormalizeBetaIntensity();
    void slotAddLevelEI();
    void slotRemoveLevel();
    void slotUpdatePseudoLevelData();
    void slotAddPseudoLevel();
    void slotSetGammaIntensityMethod(QString method);
    void slotSetGammaPathMethod(QString qmethod);
    void slotSetParticleIntensityMethod(QString);
    void slotSetParticlePathMethod(QString);
    void slotSetDeExcitationPath(QString);
    void slotSetGammaParticleIntensityRatio(QString);
    void slotUpdateTablesForward();
    void slotDaughterTableChanged( int row, int column);
    void slotApplyPathAndInten(bool);
    void InitLabels();

signals:
    void signalUpdateTables();
    void signalDecayPathEdited();

private:
    TransitionEditor* g1;
//         void initializeTable();
    void setComboBoxMethod();
    void setComboBoxDeExPath();
    void setTotalIntensityLabel();
//EVa    void checkData();

    int activeRow;
//Eva out    unsigned int nrOfRow;
//EVa out    unsigned int nrOfColumn;

//Eva    DecaySpectrum *decaySpectrum;
    Project* myProject;
    bool  boolDaughterTableDataInitialised;
   PseudoLevelsController* pseudoLevelsController_;

    double pseudoLevelEnergyStep_=100;
    double pseudoLevelEnergyMin_=3000;
    double pseudoLevelEnergyMax_=5000;
    double pseudoLevelTotInten_=10;
    double pseudoLevNeutronE_ = 0.;
//    double pseudoLevSn_ = 0.;
    bool ifStatModel_ = true;
//    string gammaIntensityMethod_;
//    string particleIntensityMethod_;
    string pathMain_;
};

#endif // DECAYPATHEDITOR_H
