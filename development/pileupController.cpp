#include "pileupController.h"
#include "ui_pileupController.h"
//#include "histogramController.h"
#include "histogramOutputController.h"
#include "contaminationController.h"
//eva #include "decay.h"



PileupController::PileupController(int flag, QWidget *parent) :
//PileupController::PileupController(QWidget *parent) :
    QWidget(parent),
    uiPileup(new Ui::PileupController)
{
    uiPileup->setupUi(this);

    myProject = Project::get();
    setInitialValues(flag);
    loadHistograms();
    slotPlotInputHistograms();
//    emit signalPlotInputHistograms();

//    plotInputHistograms();

 //   connect(uiH->histogramPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(slotShowPointToolTip(QMouseEvent*)));
    connect(uiPileup->LogScaleCheck, SIGNAL(clicked(bool)), this, SLOT(slotSetLogScale(bool)));
    connect(uiPileup->lineEditXAxisMax, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(uiPileup->lineEditXAxisMin, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(uiPileup->lineEditYAxisMax, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(uiPileup->lineEditYAxisMin, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(uiPileup->pushButtonCalculate, SIGNAL(clicked(bool)), this, SLOT(slotCalculate()));
    connect(uiPileup->pushButtonShowInputs, SIGNAL(clicked(bool)), this, SLOT(slotPlotInputHistograms()));
//    connect(this, SIGNAL(signalPlotInputHistograms()), this, SLOT(slotPlotInputHistograms()));
    connect(uiPileup->lineEditInputFile_1, SIGNAL(returnPressed()), this, SLOT(slotInputChanged()));
    connect(uiPileup->lineEditHistID_1, SIGNAL(returnPressed()), this, SLOT(slotInputChanged()));
    connect(uiPileup->lineEditFrom_1, SIGNAL(returnPressed()), this, SLOT(slotInputChanged()));
    connect(uiPileup->lineEditTo_1, SIGNAL(returnPressed()), this, SLOT(slotInputChanged()));
    connect(uiPileup->lineEditInputFile_2, SIGNAL(returnPressed()), this, SLOT(slotInputChanged()));
    connect(uiPileup->lineEditHistID_2, SIGNAL(returnPressed()), this, SLOT(slotInputChanged()));
    connect(uiPileup->lineEditFrom_2, SIGNAL(returnPressed()), this, SLOT(slotInputChanged()));
    connect(uiPileup->lineEditTo_2, SIGNAL(returnPressed()), this, SLOT(slotInputChanged()));
    connect(uiPileup->pushButtonShow, SIGNAL(clicked(bool)), this, SLOT(slotNormalizeAndShow()));
    connect(uiPileup->pushButtonSave, SIGNAL(clicked(bool)), this, SLOT(slotSave()));
    connect(uiPileup->pushButtonSaveAndAddCont, SIGNAL(clicked(bool)), this, SLOT(slotSaveAndAdd()));
}

PileupController::~PileupController()
{
    delete uiPileup;

}

void PileupController::slotSaveAndAdd()
{
    slotSave();
    QString inten = "1";   // 1% intensity of the pileup in the total spectrum
     ContaminationController *contaminationController_ = new  ContaminationController();
     int expSpectrumID = std::stoi(myProject->getExpSpecID());
     contaminationController_->addContamination(QString::number(expSpectrumID), outputFile_, QHistOutId_, inten, 0);
     delete contaminationController_;
     emit signalSavedAndAdded(false);
}

void PileupController::slotSave(){

    int histId = QHistOutId_.toInt(); //  8300;
    int Norm_ = pileupNorm_.toInt(); // 1e6;
//    int Norm_ = 1e6;
//    QString outputFile_ = "piletest";
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save pileUp spectrum HIS file"), QString::fromStdString(myProject->getWorkingDir()),
        tr(";;All Files (*)"));
//    tr("HIS file (*.his);;All Files (*)"));
    qDebug() << "1 " << outputFile_;
    if(fileName.lastIndexOf("/") != -1)
    {
        outputFile_ = fileName.section('/',-1);
    } else
    {
        outputFile_ =fileName;
    }
    qDebug() << "2 " <<outputFile_;

    vector<float> FdataYPN_;
    for(int i=0; i< dataYP.size(); i++){
        FdataYPN_.push_back(static_cast<float>(Norm_ * dataYP.at(i)));
    }
    Histogram *pileup = new Histogram(0,dataYP.size(),FdataYPN_);
    pileup->Normalize(1e6);
    HistogramOutputController *pileupOutController = new HistogramOutputController(outputFile_.toStdString(), 1, histId);
    pileupOutController->saveHistogram(pileup,histId);

}

void PileupController::slotNormalizeAndShow()
{
    if(dataYP.size() < 2)
    {
        int r = QMessageBox::warning(this, tr("Error"),
                                     tr("First calculate pileup"),
                                     QMessageBox::Ok);
        if (r == QMessageBox::Ok)
        {
            return;
        }
    }
    totalExp_ = 0.0;
    totalPileup_ = 0.0;
    int start_ = uiPileup->lineEditNormFrom->text().toInt();
//            NormFrom_.toInt();
    int end_ = uiPileup->lineEditNormTo->text().toInt();
//  NormTo_.toInt();

    for(int i=start_; i< end_; i++)
    {
        totalExp_ = totalExp_ + dataY1.at(i);
        totalPileup_ =totalPileup_ + dataYP.at(i);
    }
    dataYPN = dataYP;
    normFactor_ = totalExp_ / totalPileup_;
//    normFactor_ =1;
    for(int i=0; i < dataYP.size(); i++)
    {
        dataYPN.replace(i, dataYP.at(i)*normFactor_);
    }
    slotPlotOutputHistograms();

}

void PileupController::slotInputChanged()
{
    if(!InputChanged_)InputChanged_= true;

    InputFileName1_ = uiPileup->lineEditInputFile_1->text();
    histID1_ = uiPileup->lineEditHistID_1->text();
    Data1From_ = uiPileup->lineEditFrom_1->text();
    Data1To_ = uiPileup->lineEditTo_1->text();

    InputFileName2_ = uiPileup->lineEditInputFile_2->text();
    histID2_ = uiPileup->lineEditHistID_2->text();
    Data2From_ = uiPileup->lineEditFrom_2->text();
    Data2To_ = uiPileup->lineEditTo_2->text();

}

void PileupController::setGraphics()
{
    uiPileup->histogramInput->xAxis->setLabel("Channels/Energy (keV)");
    uiPileup->histogramInput->yAxis->setLabel("Counts");
    uiPileup->histogramOutput->xAxis->setLabel("Channels/Energy (keV)");
    uiPileup->histogramOutput->yAxis->setLabel("Counts");


    /*    uiH->histogramPlot->setInteraction(QCP::iRangeDrag, true);
    uiH->histogramPlot->setInteraction(QCP::iRangeZoom, true);
    uiH->histogramPlot->setInteraction(QCP::iSelectPlottables, true);

*/
}

void PileupController::setInitialValues(int flag)
{

    if(flag == 1) //Signal-Signal
    {
    InputFileName1_ = QString::fromStdString(myProject->getExpFile());
    histID1_ = QString::fromStdString(myProject->getExpSpecID());
    InputFileName2_ = InputFileName1_;
    histID2_ = histID1_;
    }
    if(flag == 2) // Signal-Background
    {
        InputFileName1_ = QString::fromStdString(myProject->getExpFile());
        histID1_ = QString::fromStdString(myProject->getExpSpecID());
        InputFileName2_ = InputFileName1_;
        histID2_ = "6201";

    } else {

    }

    expFileName_ = QString::fromStdString(myProject->getExpFile());
    uiPileup->lineEditExpFileName->insert(expFileName_);
    uiPileup->lineEditNormFrom->insert(NormFrom_);
    uiPileup->lineEditNormTo->insert(NormTo_);
    uiPileup->lineEditInputFile_1->insert(InputFileName1_);
    uiPileup->lineEditHistID_1->insert(histID1_);
    uiPileup->lineEditFrom_1->insert(Data1From_);
    uiPileup->lineEditTo_1->insert(Data1To_);

    uiPileup->lineEditInputFile_2->insert(InputFileName2_);
    uiPileup->lineEditHistID_2->insert(histID2_);
    uiPileup->lineEditFrom_2->insert(Data2From_);
    uiPileup->lineEditTo_2->insert(Data2To_);
    InputChanged_ = true;


}
void PileupController::slotPlotOutputHistograms()
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
    blackDotPen.setWidthF(4);
    QPen blackPen;
    blackPen.setColor(QColor(0, 0, 0, 250));
    blackPen.setWidthF(1);

    QFont legendFont = font();  // start out with MainWindow's font..
    legendFont.setPointSize(9); // and make a bit smaller for legend



    QVector<double> x = dataX1;
    QVector<double> y = dataY1;
    QVector<double> x2 = dataXP;
    QVector<double> y2 = dataYPN;
    QVector<double> diff = dataYP;

    uiPileup->histogramOutput->clearGraphs();
    uiPileup->histogramOutput->clearItems();
    uiPileup->histogramOutput->setInteraction(QCP::iRangeDrag, true);
    uiPileup->histogramOutput->setInteraction(QCP::iRangeZoom, true);
    uiPileup->histogramOutput->setInteraction(QCP::iSelectPlottables, true);
    uiPileup->histogramOutput->legend->setVisible(true);
    uiPileup->histogramOutput->legend->setBrush(QBrush(QColor(255,255,255,150)));
    uiPileup->histogramOutput->legend->setFont(legendFont);

    double yMin= uiPileup->lineEditYAxisMin->text().toDouble();
    double yMax= uiPileup->lineEditYAxisMax->text().toDouble();
    double xEMin =  uiPileup -> lineEditXAxisMin->text().toDouble();
    double xEMax =  uiPileup -> lineEditXAxisMax->text().toDouble();
//    yMax = *max_element(y.begin(),y.end());
//    yMax = yMax*1.1; //adding 10% to the scale

    uiPileup->histogramOutput->yAxis->setRange(yMin,yMax);
    uiPileup->histogramOutput->xAxis->setRange(xEMin,xEMax);


    uiPileup->histogramOutput->addGraph();
    uiPileup->histogramOutput->graph(0)->setName("Signal 1");
    uiPileup->histogramOutput->graph(0)->setData(dataX1, dataY1);
    uiPileup->histogramOutput->graph(0)->setPen(blackPen);
    uiPileup->histogramOutput->graph(0)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);

//     uiH->histogramPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
//     plot->xAxis->setLabel("Channels/Energy (keV)");
//     plot->yAxis->setLabel("Counts");

     uiPileup->histogramOutput->addGraph();
     uiPileup->histogramOutput->graph(1)->setName("Pileup normalized");
     uiPileup->histogramOutput->graph(1)->setData(x2, y2);
     uiPileup->histogramOutput->graph(1)->setPen(redDotPen);
     uiPileup->histogramOutput->graph(1)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);

     uiPileup->histogramOutput->graph()->selectable();
     uiPileup->histogramOutput->graph()->selectionDecorator()->setPen(QPen(Qt::green));

     uiPileup->histogramOutput->replot();

}
void PileupController::slotPlotInputHistograms()
{

    if(InputChanged_)loadHistograms();
    QVector<double> x = dataX1;
    QVector<double> y = dataY1;
    QVector<double> y2 = dataY2;
    QVector<double> diff = dataYP;
//    dataYP.resize(dataX1.size());
    qDebug() << dataXP.size() << dataYP.size() ;

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
    blackDotPen.setWidthF(4);
    QPen blackPen;
    blackPen.setColor(QColor(0, 0, 0, 250));
    blackPen.setWidthF(1);

    QFont legendFont = font();  // start out with MainWindow's font..
    legendFont.setPointSize(9); // and make a bit smaller for legend

    uiPileup->histogramInput->clearGraphs();
    uiPileup->histogramInput->clearItems();
    uiPileup->histogramInput->setInteraction(QCP::iRangeDrag, true);
    uiPileup->histogramInput->setInteraction(QCP::iRangeZoom, true);
    uiPileup->histogramInput->setInteraction(QCP::iSelectPlottables, true);
    uiPileup->histogramInput->legend->setVisible(true);
    uiPileup->histogramInput->legend->setBrush(QBrush(QColor(255,255,255,150)));
    uiPileup->histogramInput->legend->setFont(legendFont);

    double yMin= uiPileup->lineEditYAxisMin->text().toDouble();
    double yMax= uiPileup->lineEditYAxisMax->text().toDouble();;
    double xEMin =  uiPileup -> lineEditXAxisMin->text().toDouble();
    double xEMax =  uiPileup -> lineEditXAxisMax->text().toDouble();
    double yMax1 = *max_element(y.begin(),y.end());
    double yMax2 = *max_element(y2.begin(),y2.end());
    yMax = std::max(yMax1,yMax2);
    yMax = yMax*1.1; //adding 10% to the scale

    uiPileup->histogramInput->yAxis->setRange(yMin,yMax);
    uiPileup->histogramInput->xAxis->setRange(xEMin,xEMax);
/*
    // getting Q value from NNDC input data and making a line
            QCPItemLine *QValueLine = new QCPItemLine(uiPileup->histogramInput);
            Decay* decay= Decay::get();
            if(decay != 0L)
            {
            double QValue = decay->GetQVal();
            xEMax= QValue* 1.2;
     // add the text label at the top:
            QCPItemText *textLabel = new QCPItemText(uiPileup->histogramInput);
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

*/

    uiPileup->histogramInput->addGraph();
    uiPileup->histogramInput->graph(0)->setName("Signal 1");
    uiPileup->histogramInput->graph(0)->setData(dataX1, dataY1);
    uiPileup->histogramInput->graph(0)->setPen(blackPen);
    uiPileup->histogramInput->graph(0)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);

//     uiH->histogramPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
//     plot->xAxis->setLabel("Channels/Energy (keV)");
//     plot->yAxis->setLabel("Counts");

     uiPileup->histogramInput->addGraph();
     uiPileup->histogramInput->graph(1)->setName("Signal 2");
     uiPileup->histogramInput->graph(1)->setData(x, y2);
     uiPileup->histogramInput->graph(1)->setPen(redDotPen);
     uiPileup->histogramInput->graph(1)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);

     if(dataXP.size() > 0 & dataYP.size() > 0)
     {
     uiPileup->histogramInput->addGraph();
     uiPileup->histogramInput->graph(2)->setName("PileUp 1");
     uiPileup->histogramInput->graph(2)->setData(dataXP, dataYP);
     uiPileup->histogramInput->graph(2)->setPen(blackPen);
     uiPileup->histogramInput->graph(2)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
    }

     uiPileup->histogramInput->graph()->selectable();
     uiPileup->histogramInput->graph()->selectionDecorator()->setPen(QPen(Qt::green));

     uiPileup->histogramInput->replot();


}
void PileupController::loadHistograms()
{
    if(InputChanged_){
    Histogram *firstHistogram = new Histogram(InputFileName1_.toStdString(),histID1_.toInt() );
    Histogram *secondHistogram = new Histogram(InputFileName2_.toStdString(),histID2_.toInt() );
    float x1Min = 0;
    float x1Max = firstHistogram->GetNrOfBins();
    dataY1 = QVector<double>::fromStdVector(firstHistogram->GetAllDataD());
    dataX1 = QVector<double>::fromStdVector(firstHistogram->GetEnergyVectorD());
    double yMax1 = *max_element(dataY1.begin(),dataY1.end());

    float x2Min = 0;
    float x2Max = secondHistogram->GetNrOfBins();
    dataY2 = QVector<double>::fromStdVector(secondHistogram->GetAllDataD());
    dataX2 = QVector<double>::fromStdVector(secondHistogram->GetEnergyVectorD());

    double yMax2 = *max_element(dataY2.begin(),dataY2.end());
    double yMax =  std::max(yMax1,yMax2);
    yMax = yMax*1.1; //adding 10% to the scale
    uiPileup->lineEditYAxisMax->setText(QString::number(yMax));

    }
    InputChanged_= false;
}

void PileupController::slotCalculate()
{
// based on Charlie Rasco's ROOT file

    if(InputChanged_)loadHistograms();

    double binEff = 0.0;
    double binE = 0.0;
    double pilupEff = 0.0;
    double pilup2Eff = 0.0;
    double pilupE = 0.0;
    double dataY2Total = 0.0; // secondHistogram->Integral(0,nBins);
    double dataY1Total = 0.0;

    // Undetected part of spectrum
    const double undetectedFract = 0.0;// 0 to 1. 1 = 100% detection. 50% ~ beta groundstate to groundstate efficiency.
    const double undetected2Fract = 1.00;// 0 to 1. 1 = 100% detection. 50% ~ beta groundstate to groundstate efficiency.

    for(int i=0; i < dataY2.size(); i++)
    {
        dataY2Total = dataY2Total + dataY2.at(i);
    }
    for(int i=0; i < dataY1.size(); i++)
    {
        dataY1Total = dataY1Total + dataY1.at(i);
    }

    dataXP.clear();
    dataYP.clear();
    dataYP2.clear();
    dataXP.resize(dataX2.size()+dataX1.size());
    dataYP.resize(dataXP.size());
    dataYP2.resize(dataXP.size());

    qDebug() <<" sizeX1 : " << dataX1.size();
    qDebug() <<" sizeX2 : " << dataX2.size();
    qDebug() <<" sizeXP : " << dataXP.size();
    qDebug() <<" sizeYP : " << dataXP.size();

    int End1_ =   Data1To_.toInt();
    int End2_ = Data1To_.toInt();

    int numTasks = End1_;
        QProgressDialog progress(" Pileup calculation in progress... ", "Cancel", 0, numTasks, this);
        progress.setFixedWidth(512);
        progress.setWindowModality(Qt::WindowModal);

    for( int i = 0; i < End1_ ; i++ )
    {
//     std::cout << "Running channel # " << i << "\r" << std::flush;
     progress.setValue(i);
     if (progress.wasCanceled())  break;

     if( i == 0 )// unmeasured data in MTAS but with trigger.
      {

        for( int j = 1; j < End2_; j++ )
        {
//          pilupE = secondHistogram->GetBinCenter(j);
//          pilupEff = undetectedFract * secondHistogram->GetBinContent(j) / integral2Total;
//          pilup2Eff = undetected2Fract * secondHistogram->GetBinContent(j) / integral2Total;
          dataXP.replace(j, dataX2.at(j));
          dataYP.replace(j, dataYP.at(j) + undetectedFract *  dataY2.at(j) / dataY2Total);
//          dataYP2.insert(j, undetected2Fract *  dataY2.at(j)/dataY2Total);

//          pileupHis->Fill( pilupE, pilupEff );
//          pileup2His->Fill( pilupE, pilup2Eff );
        }
    //    continue;
      }
      else
      {
    //    continue;
//      qDebug() << i <<" przed druga petla";
      double value1 = dataY1.at(i);
      double pos1 = dataX1.at(i);
        for( Int_t j = 1; j < dataX2.size(); j++ )
        {
//          pilupE = binE + secondHistogram->GetBinCenter(j);
          dataXP.replace(i+j, pos1 + dataX2.at(j));
          int k;
          k=i+j;
          pilupEff = dataYP.at(k) + (1.0 - undetectedFract) * value1 * dataY2.at(j) /dataY2Total ;
          dataYP.replace(k,pilupEff);
//          if(i+j == 80)qDebug() << dataXP.at(i+j) << dataYP.at(k) << pilupEff;
     //     dataYP.insert(i+j,  dataYP.at(i+j) + (1.0 - undetectedFract) * value1 * dataY2.at(j)); // /dataY2Total);
  //        if(i+j == 80) qDebug() << i << "+"<<j<<"="<<i+j << dataYP.at(i+j) << value1 << dataY2.at(j) ;
//          dataYP2.insert(i+j, dataYP.at(i+j) + (1.0 - undetected2Fract) * value1 * dataY2.at(j)/dataY2Total);

//          pilupEff = ( 1.0 - undetectedFract ) * binEff * secondHistogram->GetBinContent(j) / integral2Total;
//          pilup2Eff = ( 1.0 - undetected2Fract ) * binEff * secondHistogram->GetBinContent(j) / integral2Total;

        }
      }
    }

    slotPlotInputHistograms();
}

/* void PilupController::slotShowPointToolTip(QMouseEvent *event)
    {
    double x = uiH->histogramPlot->xAxis->pixelToCoord(event->pos().x());
    double y = uiH->histogramPlot->yAxis->pixelToCoord(event->pos().y());
    setToolTip(QString("%1 , %2").arg(x).arg(y));

    }
*/
void PileupController::slotSetLogScale(bool checked)
{
    if (checked) {
        if(uiPileup->lineEditYAxisMin->text().toDouble() == 0){
            uiPileup->lineEditYAxisMin->setText("1");
        }
        uiPileup->histogramInput->yAxis->setScaleType(QCPAxis::stLogarithmic);
        uiPileup->histogramOutput->yAxis->setScaleType(QCPAxis::stLogarithmic);
    } else {
        if(uiPileup->lineEditYAxisMin->text().toDouble() == 1){
            uiPileup->lineEditYAxisMin->setText("0");
        }
        uiPileup->histogramInput->yAxis->setScaleType(QCPAxis::stLinear);
        uiPileup->histogramOutput->yAxis->setScaleType(QCPAxis::stLinear);
    }
    uiPileup->histogramInput->replot();
    uiPileup->histogramOutput->replot();
}



void PileupController::slotSetAxisRange()
{
    double xMinEn =  uiPileup -> lineEditXAxisMin->text().toDouble();
    double xMaxEn =  uiPileup -> lineEditXAxisMax->text().toDouble();
    uiPileup->histogramInput->xAxis->setRange(xMinEn,xMaxEn);
    uiPileup->histogramOutput->xAxis->setRange(xMinEn,xMaxEn);
    double yMax = uiPileup->lineEditYAxisMax->text().toDouble();
    double yMin = uiPileup->lineEditYAxisMin->text().toDouble();
    uiPileup->histogramInput->yAxis->setRange(yMin,yMax);
    uiPileup->histogramOutput->yAxis->setRange(yMin,yMax);
   // calculating SUM of displayed exp spectrum
//           double value1DSum =     expHist->GetNrOfCounts(xMinEn,xMaxEn);
//           QString ExpSpecSum_ = QString::number(value1DSum);
//           QString qstr = "NoC in Exp. spec. within limits: "+ ExpSpecSum_;
//           ui->labelExpCount->setText(qstr);

    uiPileup->histogramOutput->replot();
    uiPileup->histogramInput->replot();

}
/*
double PileupController::vectorMax(QVector<double> yM, double minEn, double maxEn)
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
            yMax=max(yMax,abs(yM.at(i)));
    }

    return yMax;
}
*/
