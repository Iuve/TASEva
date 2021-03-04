#ifndef MANUALFITGRAPH_H
#define MANUALFITGRAPH_H

#include <QWidget>
#include <rowData.h>
#include <project.h>

namespace Ui {
class ManualFitGraph;
}

class ManualFitGraph : public QWidget
{
    Q_OBJECT

public:
    explicit ManualFitGraph(QWidget *parent = 0);
    ~ManualFitGraph();
    void showDataExpSimDiff(QVector<double> x, QVector<double> y, QVector<double> y2, QVector<double> diff);
    double vectorMax(QVector<double> yM, double minEn, double maxEn);
    void setHeader(QStringList header);
    void setResponseType(string respType){respType_ =  respType;}

    string getResponseType(){return respType_;}
    void initializeTable(std::vector<RowData> rowData);
    void initializeRow(int rowNumber, RowData rowData);
    void setCurrentLevelIndex(int currentLevelIndex){currentLevelIndex_ = currentLevelIndex;}
    void setCurrentLevel(double energy);
    void setxMin(QString qstr);
    void setxMax(QString qstr);
    void setXValues(QVector<double> X){ x = X;}
    void setYValues(QVector<double> Y){ y = Y;}
    void setY2Values(QVector<double> Y){ y2 = Y;}
    void setDiffValues(QVector<double> Y){ diff = Y;}
    void initializeGraphs();

    Ui::ManualFitGraph *uiM;

public slots:
    void slotShowPointToolTip(QMouseEvent *event);
    void slotSetLogScale(bool checked);
    void slotSetAxisRange();
    void slotUpdateTableData(int row, int column);
    void slotDisplayStatusClicked(int row, int column);
    void showResponseFunctionsLevels(int i);
    void showResponseFunctionsGammas(int i);
    void slotShowOtherLevelsContribution(bool status);
    void changeGammaTable(int row, int column);
    void changeLevelTable(int row, int column);
    void changeSpectraTable(int row, int column);
    void slotRecalculate();
    void slotNormalise(bool ok);
    void slotNormaliseBetaFeeding(bool ok);
    void slotNormaliseTransitionsFeeding(bool ok);
    void slotShowSimSpec(bool checked);
    void slotFittingStatusTrue(){setColumnStatus(true, 2);}
    void slotFittingStatusFalse(){setColumnStatus(false, 2);}
    void setColumnStatus(bool status, int column);
    void slotDisplayStatusTrue(){setColumnStatus(true, 0);}
    void slotDisplayStatusFalse(){setColumnStatus(false, 0);}
    void slotStatusClicked(bool status, int row, int column);
    void showSpectra(int start);
    void showResponseFunctions();

    signals:
    void signalRecalculateGamma(bool);
    void signalRecalculateLevel();

private:
    QVector<double> x, y, y2, diff;
    std::vector<bool> displayStatus;
    bool  boolTableDataInitialised;
    Project* myProject;
    string respType_;      // "l" - levels; "g" - gammas in 2D; "c" -
    int currentLevelIndex_;
        int lastGraph_;
     void ShowOtherLevelsContribution(int last);

     QAction* FittingStatusTrueAction_;
     QAction* FittingStatusFalseAction_;
     QAction* DisplayStatusTrueAction_;
     QAction* DisplayStatusFalseAction_;

      QPen blueDotPen,redDotPen,blackDotPen,blackPen;

};

#endif // MANUALFITGRAPH_H
