#ifndef ANALYSIS2D_H
#define ANALYSIS2D_H
#include <qcustomplot.h>
//#include <twoDimFitController.h>
#include "histogram.h"
#include "project.h"
#include "manualfitgraph.h"

#include <QWidget>

namespace Ui {
class Analysis2D;
}

class TwoDimFitController;
class Analysis2D : public QWidget
{
    Q_OBJECT

public:
    explicit Analysis2D(QWidget *parent = 0);
    ~Analysis2D();
public slots:
     void slotReplotGraphics();
     void slotSetLogScaleYProj(bool checked);
     void slotSetLogScaleXProj(bool checked);
     void slotMakeGate();
     void slotGate1Changed();
     void slotGate2Changed();
     void slotSetLevelEnergy(QString qLevelEnergy);
     void slot2DFitControler();
     void slotShowGate1ExpVsSim(bool recalculateTransitions);
     void slotChi2Calculation();
    void slotClose();
signals:
    void signalClose2dUI();

private:
     Ui::Analysis2D *ui2D;
     ManualFitGraph *m1;
//    ManualFitGraph *m1 = new ManualFitGraph();
    Project *myProject = Project::get();
    void setInitialValues();
    void setGraphics();
    void setComboBoxLevel();
    std::vector<double> gateOnX(int low, int high);
    std::vector<double> gateOnY(int low, int high);

    double Display2DXmin, Display2DXmax;
    double Display2DYmin, Display2DYmax;
    bool windowOpen_ = false;
    double gateOnXLow_, gateOnXHigh_;
    double gateOnYLow_, gateOnYHigh_;
    std::vector<double> projectionOnX;  //means gating on Y axis
    std::vector<double> projection2OnX_;  //means gating on Y axis  additional (2) gate
    std::vector<double> projectionOnY;  //means gating on X axis
    std::vector<double> channelProjectionOnX;
    unsigned int xSize_;
    unsigned int ySize_;   // size of the 2D spectrum

    QCPColorMap* colorMap2D_;
     QCPItemRect *yRectGate1Item_;
     QCPItemRect *yRectGate12DItem_;
     QCPItemRect *yRectGate2Item_;
     QCPItemRect *yRectGate22DItem_;

     TwoDimFitController* fit2DController_ ;//= new TwoDimFitController();
     bool prepareRestLevelResponseDoneFlag_;
    // functions used in other classes too ;(
    double vectorMax(std::vector<double> vec, double minEn, double maxEn);  //similar to HistogramGraph
};

#endif // ANALYSIS2D_H
