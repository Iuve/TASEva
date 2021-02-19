#ifndef PILEUPCONTROLLER_H
#define PILEUPCONTROLLER_H

//eva #include "project.h"
#include "histogram.h"
//eva #include "histogramGraphController.h"
#include "project.h"
#include <QWidget>

namespace Ui {
class PileupController;
}

class PileupController : public QWidget
{
    Q_OBJECT

public:
    explicit PileupController(int flag, QWidget *parent = 0);
    ~PileupController();
    void showDataExpSimDiff(QVector<double> x, QVector<double> y, QVector<double> y2, QVector<double> diff);
    double vectorMax(QVector<double> yM, double minEn, double maxEn);
    void setInitialValues(int flag);

public slots:
//    void slotShowPointToolTip(QMouseEvent *event);
      void slotSetLogScale(bool checked);
      void slotSetAxisRange();
      void slotCalculate();
      void slotPlotInputHistograms();
      void slotPlotOutputHistograms();
      void slotInputChanged();
      void slotNormalizeAndShow();
      void slotSave();
      void slotSaveAndAdd();

signals:
    void signalPlotInputHistograms();
    void signalContaminationToAdd(QString outputFile_, QString QHistOutId_, QString inten);
    void signalSavedAndAdded(bool flag);

private:
      void loadHistograms();
      void setGraphics();
//      void plotInputHistograms();
    Ui::PileupController *uiPileup;
    Project* myProject;
       bool InputChanged_;

    Histogram *firstHistogram;
    Histogram *secondHistogram;
    QVector<double> dataX1, dataY1, dataX2, dataY2;
    QVector<double> dataXP, dataYP;
    QVector<double> dataYP2;
    QVector<double> dataYPN;
    double totalExp_, totalPileup_ , normFactor_;

    QString InputFileName1_ = "not assign ";
    QString histID1_ = "";
    QString Data1From_ = "0";
    QString Data1To_ = "4000";
    QString InputFileName2_ = "not assign";
    QString histID2_ = "";
    QString Data2From_ = "0";
    QString Data2To_ = "8000";
    QString expFileName_ = "expFileName";
    QString NormFrom_ = "4000";
    QString NormTo_ = "6000";
//    QVector<double> x, y, y2, diff;
    QString QHistOutId_ = "8300";
    QString outputFile_ = "pileTest";
    QString pileupNorm_ = "1000000";

};

#endif // PILEUPCONTROLLER_H
