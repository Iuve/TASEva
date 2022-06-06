#include "simplefigure.h"
#include "ui_simplefigure.h"

SimpleFigure::SimpleFigure(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SimpleFigure)
{
    ui->setupUi(this);

    ui->dataPlot->setInteraction(QCP::iRangeDrag, true);
    ui->dataPlot->setInteraction(QCP::iRangeZoom, true);
    ui->dataPlot->setInteraction(QCP::iSelectPlottables, true);
    ui->dataPlot->xAxis->setLabel("Channels/Energy (keV)");
    ui->dataPlot->yAxis->setLabel("Counts");

    connect(ui->dataPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(slotShowPointToolTip(QMouseEvent*)));
    connect(ui->checkBox_LogScale, SIGNAL(clicked(bool)), this, SLOT(slotSetLogScale(bool)));
    connect(ui->lineEdit_XAxisMax, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(ui->lineEdit_XAxisMin, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(ui->lineEdit_YAxisMax, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(ui->lineEdit_YAxisMin, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));

    connect(ui->btn_Close, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(ui->btn_Accept, SIGNAL(clicked(bool)), this, SLOT(slotAccept()));
    connect(ui->btn_Reject, SIGNAL(clicked(bool)), this, SLOT(slotCancel()));

}

SimpleFigure::~SimpleFigure()
{
    delete ui;
}

void SimpleFigure::slotAccept(){
    emit accept();
}
void SimpleFigure::slotCancel(){
    emit cancel();
}
void SimpleFigure::setxMin(QString qstr)
{
    ui->lineEdit_XAxisMin->clear();
    ui->lineEdit_XAxisMin->insert(qstr);
}
void SimpleFigure::setxMax(QString qstr)
{
    ui->lineEdit_XAxisMax->clear();
    ui->lineEdit_XAxisMax->insert(qstr);
}
void SimpleFigure::setyMin(QString qstr)
{
    ui->lineEdit_YAxisMin->clear();
    ui->lineEdit_YAxisMin->insert(qstr);
}
void SimpleFigure::setyMax(QString qstr)
{
    ui->lineEdit_YAxisMax->clear();
    ui->lineEdit_YAxisMax->insert(qstr);
}
void SimpleFigure::slotShowPointToolTip(QMouseEvent *event)
    {
    double x = ui->dataPlot->xAxis->pixelToCoord(event->pos().x());
    double y = ui->dataPlot->yAxis->pixelToCoord(event->pos().y());
    setToolTip(QString("%1 , %2").arg(x).arg(y));

    }
void SimpleFigure::slotSetLogScale(bool checked)
{
    if (checked) {
    ui->dataPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->lineEdit_YAxisMin->setText("1");
    } else {
        ui->dataPlot->yAxis->setScaleType(QCPAxis::stLinear);
        ui->lineEdit_YAxisMin->setText("0");
    }
    ui->dataPlot->replot();
}

void SimpleFigure::slotSetAxisRange()
{

// this method responses to changes on GUI.

    double xMinEn =  ui -> lineEdit_XAxisMin->text().toDouble();
    double xMaxEn =  ui -> lineEdit_XAxisMax->text().toDouble();
//    double xMaxEn = x.size();
    double yMin =  ui->lineEdit_YAxisMin->text().toDouble();
    double yMax =  ui->lineEdit_YAxisMax->text().toDouble();
/*   if(xMaxEn > x.back())
    {
        xMaxEn = x.back();
        QMessageBox msgBox;
        QString info;
        QString qstrxMax = QString::number(xMaxEn);
        info = "X-Axis Max LARGER then spectrum size! Rescaling to: " + qstrxMax;
        ui->lineEdit_XAxisMax->clear();
        ui->lineEdit_XAxisMax->insert(QString::number(xMaxEn));
        msgBox.setText(info);
        msgBox.exec();

    }
  */  double size = xMaxEn-xMinEn;
    ui->dataPlot->xAxis->setRange(xMinEn,xMaxEn);
    ui->dataPlot->yAxis->setRange(yMin,yMax);
    ui->dataPlot->replot();

}

void SimpleFigure::initializeGraphs()
{
    QPen blueDotPen;
    blueDotPen.setColor(QColor(0, 0, 255, 150));
    blueDotPen.setStyle(Qt::DotLine);
    blueDotPen.setWidthF(1);
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


    double yMin= ui->lineEdit_YAxisMin->text().toDouble();
    double yMaxUser = ui->lineEdit_YAxisMax->text().toDouble();
    double xMinEn =  ui -> lineEdit_XAxisMin->text().toDouble();
    double xMaxEn =  ui -> lineEdit_XAxisMax->text().toDouble();

    double yMax=yMaxUser;

    ui->dataPlot->yAxis->setRange(yMin,yMaxUser);
    ui->dataPlot->xAxis->setRangeLower(xMinEn);
    ui->dataPlot->xAxis->setRangeUpper(xMaxEn);

}



void SimpleFigure::showData(vector<Histogram> vecHistogram)
{
    qDebug() << "W showdata(histogram) ";

    vector<QVector<double>> vecQVector;
    for (unsigned i=0; i!=vecHistogram.size(); i++){
        QVector<double> x = QVector<double>::fromStdVector(vecHistogram.at(i).GetEnergyVectorD());
        QVector<double> y = QVector<double>::fromStdVector(vecHistogram.at(i).GetAllDataD());
        vecQVector.push_back(x);
        vecQVector.push_back(y);
    }

    showData(vecQVector);
}
void SimpleFigure::showData(vector<QVector<double>> vecQVector)
{

    qDebug() << " " << vecQVector.size() << " % 2" << vecQVector.size() % 2;

    for (int i=2800; i != 2850;i++){
        qDebug() << i << vecQVector.at(0).at(i) << vecQVector.at(2).at(i) << vecQVector.at(4).at(i) ;
    }
    if((vecQVector.size() % 2) == 0){
       int k =0;
       for (unsigned i=0; i !=vecQVector.size(); i=i+2){
           qDebug() << "W showdata(vector) petla" << i;
        ui->dataPlot->addGraph();
        ui->dataPlot->graph(k)->setData(vecQVector.at(i),vecQVector.at(i+1));
        ui->dataPlot->graph(k)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
        if(k==0)ui->dataPlot->graph(0)->setPen(QPen(Qt::black));
        if(k==1)ui->dataPlot->graph(1)->setPen(QPen(Qt::red));
        if(k==2)ui->dataPlot->graph(2)->setPen(QPen(Qt::green));
        k++;
       }
       ui->dataPlot->replot();
    } else {
        qDebug() << "VEctor values for plotting have no even (x,y) data " << vecQVector.size();
        return;
    }
 //   ui->dataPlot->replot();

}

double SimpleFigure::vectorMax(QVector<double> yM, double minEn, double maxEn)
{
    int xposition = 0;
    double yMax=1;
    unsigned int xposMin = 0;
    unsigned int xposMax = 16000;
    int j=0;
    if(x.size() != yM.size()) cout << " voctorMax:: X.size not equal Y.size" << endl;
    cout << "x.size  = "<<x.size() <<"  y.size = " << yM.size() << endl;
    while(x.at(j) < minEn)
    {
     j++;
    }
            xposMin=j;
            cout << "xposMin = " << xposMin << endl;
    while(x.at(j) < maxEn-1)
    {
        j++;
    }
            xposMax=j;
            cout << "xposMax = " << xposMax << endl;
    for(unsigned int i = xposMin;  i <= xposMax; ++i)
    {
            yMax=max(yMax,abs(yM.at(i)));
    }

    return yMax;
}
