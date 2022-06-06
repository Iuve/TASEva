#ifndef CALIBRATEENERGY_H
#define CALIBRATEENERGY_H

#include "tableinput.h"
#include "histogram.h"
#include "project.h"

class CalibrateEnergy : public QWidget
{
    Q_OBJECT
public:
    explicit CalibrateEnergy(QWidget *parent = 0);
    ~CalibrateEnergy();

    void InitTableUp();
    void InitTableDown();

public slots:
    void slotRecalibrate(int ID);
    void slotRecalibrate(QString spectype, int histID, std::vector<double> low, int glue, std::vector<double> high);
    void slotAccept();
    void slotCancel();

private:
    TableInput *calE_ui;
    Histogram histogramIN_;
    Histogram histogramOUT_;
    Histogram histogramREF_;
    Project *myProject = Project::get();
    vector<Histogram> vecHistogram_;
    int currentHistID_;
    QString currentSpecType_;

};

#endif // CALIBRATEENERGY_H
