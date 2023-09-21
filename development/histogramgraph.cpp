#include "histogramgraph.h"
#include "ui_histogramgraph.h"
#include  "DeclareHeaders.hh"



HistogramGraph::HistogramGraph(QWidget *parent) :
    QWidget(parent),
    uiH(new Ui::HistogramGraph)
{
    uiH->setupUi(this);

    uiH->histogramPlot->setInteraction(QCP::iRangeDrag, true);
    uiH->histogramPlot->setInteraction(QCP::iRangeZoom, true);
    uiH->histogramPlot->setInteraction(QCP::iSelectPlottables, true);
    uiH->histogramPlot->xAxis->setLabel("Channels/Energy (keV)");
    uiH->histogramPlot->yAxis->setLabel("Counts");

    uiH->histogramDiffPlot->xAxis->setLabel("Channels/Energy (keV)");
    uiH->histogramDiffPlot->yAxis->setLabel("(Exp - Sim)");


    connect(uiH->histogramPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(slotShowPointToolTip(QMouseEvent*)));
    connect(uiH->LogScaleCheck, SIGNAL(clicked(bool)), this, SLOT(slotSetLogScale(bool)));
    connect(uiH->lineEditXAxisMax, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(uiH->lineEditXAxisMin, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(uiH->lineEditYAxisMin, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(uiH->lineEditYAxisMax, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));

}

HistogramGraph::~HistogramGraph()
{
    delete uiH;
}
void HistogramGraph::slotShowPointToolTip(QMouseEvent *event)
    {
    double x = uiH->histogramPlot->xAxis->pixelToCoord(event->pos().x());
    double y = uiH->histogramPlot->yAxis->pixelToCoord(event->pos().y());
    setToolTip(QString("%1 , %2").arg(x).arg(y));

    }
void HistogramGraph::slotSetLogScale(bool checked)
{
    if (checked) {
    uiH->histogramPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    uiH->lineEditYAxisMin->setText("1");
    } else {
        uiH->histogramPlot->yAxis->setScaleType(QCPAxis::stLinear);
        uiH->lineEditYAxisMin->setText("0");
    }
    uiH->histogramPlot->replot();
}


double HistogramGraph::vectorMax(QVector<double> yM, double minEn, double maxEn)
{
    int xposition = 0;
    double yMax=1;
    unsigned int xposMin = 0;
    unsigned int xposMax = 16000;
    int j=0;
    while(x.at(j) < minEn)
    {
     j++;
    }
            xposMin=j;
    while(x.at(j) < maxEn)
    {
        j++;
    }
            xposMax=j;
    for(unsigned int i = xposMin;  i <= xposMax; ++i)
    {
            yMax=std::max(yMax,std::abs(yM.at(i)));
    }

    return yMax;
}



void HistogramGraph::slotSetAxisRange()
{
    double xMinEn =  uiH -> lineEditXAxisMin->text().toDouble();
    double xMaxEn =  uiH -> lineEditXAxisMax->text().toDouble();
    double size = xMaxEn-xMinEn;
    uiH->histogramPlot->xAxis->setRange(xMinEn,xMaxEn);
    uiH->histogramDiffPlot->xAxis->setRange(xMinEn,xMaxEn);
    double yMin = uiH->lineEditYAxisMin->text().toDouble();
//    double yMax = vectorMax(y,xMinEn,xMaxEn);
//    yMax = yMax*1.1; //adding 10% to the scale
    double yMax = uiH->lineEditYAxisMax->text().toDouble();

    uiH->histogramPlot->yAxis->setRange(yMin,yMax);
    double dyMax = vectorMax(diff,xMinEn,xMaxEn);
    uiH->histogramDiffPlot->yAxis->setRange(-dyMax,dyMax);
 /*   // calculating SUM of displayed exp spectrum
           double value1DSum =     expHist->GetNrOfCounts(xMinEn,xMaxEn);
           QString ExpSpecSum_ = QString::number(value1DSum);
           QString qstr = "NoC in Exp. spec. within limits: "+ ExpSpecSum_;
           ui->labelExpCount->setText(qstr);
*/
    uiH->histogramPlot->replot();
    uiH->histogramDiffPlot->replot();

}
void HistogramGraph::showDataExpSimDiff(QVector<double> xx, QVector<double> yy, QVector<double> yy2, QVector<double> ddiff)
{
    x = xx;
    y = yy;
    y2 = yy2;
    diff =ddiff;
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

    double yMin=1;
    double yMax;
    double xEMin =  uiH -> lineEditXAxisMin->text().toDouble();
    double xEMax =  uiH -> lineEditXAxisMax->text().toDouble();
    yMax = *std::max_element(y.begin(),y.end());
    yMax = yMax*1.1; //adding 10% to the scale
    uiH->lineEditYAxisMax->setText(QString::number(yMax));

    uiH->histogramPlot->yAxis->setRange(yMin,yMax);


    // getting Q value from NNDC input data and making a line
            QCPItemLine *QValueLine = new QCPItemLine(uiH->histogramPlot);
            QCPItemLine *SnValueLine = new QCPItemLine(uiH->histogramPlot);
            DecayPath* decayPath= DecayPath::get();
            if(decayPath != 0L)
            {
            double QValue = decayPath->GetAllNuclides()->at(0).GetQBeta();
            xEMax= QValue* 1.2;
     // add the text label at the top:
            QCPItemText *textLabel = new QCPItemText(uiH->histogramPlot);
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
            QCPItemText *textLabel2 = new QCPItemText(uiH->histogramPlot);
            textLabel2->setPositionAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    //        textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
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


    uiH->histogramPlot->addGraph();
    uiH->histogramPlot->graph(0)->setName("Experiment");
    uiH->histogramPlot->graph(0)->setData(x, y);
     uiH->histogramPlot->graph(0)->setPen(blackPen);
     uiH->histogramPlot->graph(0)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
//     uiH->histogramPlot->yAxis->setRange(yMin,yMax);
     uiH->histogramPlot->xAxis->setRange(xEMin,xEMax);
     uiH->histogramDiffPlot->xAxis->setRange(xEMin,xEMax);

//     uiH->histogramPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
//     plot->xAxis->setLabel("Channels/Energy (keV)");
//     plot->yAxis->setLabel("Counts");



     uiH->histogramPlot->addGraph();
     uiH->histogramPlot->graph(1)->setName("Simulation");
     uiH->histogramPlot->graph(1)->setData(x, y2);
     uiH->histogramPlot->graph(1)->setPen(redDotPen);
     uiH->histogramPlot->graph(1)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
     uiH->histogramPlot->graph()->selectable();
     uiH->histogramPlot->graph()->selectionDecorator()->setPen(QPen(Qt::green));


     uiH->histogramDiffPlot->addGraph();
     uiH->histogramDiffPlot->graph(0)->setName("(Exp - Sim)");
     uiH->histogramDiffPlot->graph(0)->setData(x, diff);
//     plotDiff->graph(0)->setPen(blackPen);
     uiH->histogramDiffPlot->graph(0)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
     uiH->histogramDiffPlot->xAxis->setRange(xEMin,xEMax);
     uiH->histogramDiffPlot->yAxis->setRange(-yMax/20,yMax/20);
//     uiH->histogramDiffPlot->xAxis2->setRange(0,100);

//     foreach (QCustomPlot *plot, m_charts2)
     uiH->histogramPlot->replot();
     uiH->histogramDiffPlot->replot();

}
