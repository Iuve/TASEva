#ifndef TRANSITIONEDITOR_H
#define TRANSITIONEDITOR_H

#include <QWidget>
#include <QTableWidget>
#include "rowData.h"

#include <string>
#include <iostream>

//class Transition;

namespace Ui {
class TransitionEditor;
class DecayPathEditor;
class MainWindow;
}

class TransitionEditor : public QWidget
{
    Q_OBJECT

public:
    explicit TransitionEditor(QWidget *parent = 0);
    ~TransitionEditor();
    Ui::TransitionEditor *uiL;
    QTableWidget* getUiPointer() {return pointerToTable_;}
//EVa    void setNrOfColumn(int nrOfColumn){this->nrOfColumn = nrOfColumn;}
    void setTransitionLabel(QString Label);
    void setTotalIntensityLabel();
    void setCurrentLevel(int currentLevelIndex){currentLevelIndex_ = currentLevelIndex;}
    int  getCurrentLevel(){return  currentLevelIndex_;}
    void setCurrentNuclide(int currentNuclideIndex){currentNuclideIndex_ = currentNuclideIndex;}
    void setTransitionEditorOpen(bool b){transitionEditorOpen_ = b;}
public slots:
//    void slotEditGammas(int rowIndex);
    void slotAddTransition();
    void slotRemoveTransition();
    void slotNormalizeTransitionIntensity();
     void slotFittingStatusClicked(int row, int column);
     void slotTableChanged(int row, int column);
signals:
    void signalUpdateTransitionTable(int currentNuclideIndex_, int currentLevelIndex_);
    void signalUpdateLevelTable();
    void signalTransitionsEdited();


private:
    void initializeRow(int rowNumber, RowData rowData);
    void initializeTable();
    void checkData();
    QTableWidget *pointerToTable_;
    int activeRow;
    unsigned int nrOfColumn;
    int currentLevelIndex_;
    int currentNuclideIndex_;
    bool transitionEditorOpen_;
};

#endif // TRANSITIONEDITOR_H
