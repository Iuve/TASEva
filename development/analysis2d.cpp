#include "analysis2d.h"
#include "ui_analysis2d.h"

#include "project.h"
#include "DeclareHeaders.hh"
#include "twoDimFitController.h"
#include "manualfitgraph.h"
//#include "histogramGraphController.h"

#include <string>
#include <iostream>
#include <fstream>


Analysis2D::Analysis2D(QWidget *parent) :
    QWidget(parent),
    ui2D(new Ui::Analysis2D)
{
    ui2D->setupUi(this);

    xSize_ = 8191;  // should be read from *.drr file
    ySize_ = 8191;
    windowOpen_ = false;
    setInitialValues();
    setGraphics();
    setComboBoxLevel();
    fit2DController_ = new TwoDimFitController();
    m1 = new ManualFitGraph();
    prepareRestLevelResponseDoneFlag_=false;

    ui2D->buttonShowGate->setDisabled(true);
        connect(ui2D->buttonClose, SIGNAL(clicked()), this, SLOT(slotClose()));
        connect(this, SIGNAL(signalClose2dUI()), this, SLOT(close()));

        connect(ui2D->checkLogXProj, SIGNAL(clicked(bool)), this, SLOT(slotSetLogScaleXProj(bool)));
        connect(ui2D->checkLogYProj, SIGNAL(clicked(bool)), this, SLOT(slotSetLogScaleYProj(bool)));
    //Display
    connect(ui2D->lineDisplayXmax, SIGNAL(returnPressed()), this, SLOT(slotReplotGraphics()));
    connect(ui2D->lineDisplayXmin, SIGNAL(returnPressed()), this, SLOT(slotReplotGraphics()));
    connect(ui2D->lineDisplayYmax, SIGNAL(returnPressed()), this, SLOT(slotReplotGraphics()));
    connect(ui2D->lineDisplayYmin, SIGNAL(returnPressed()), this, SLOT(slotReplotGraphics()));
    // Gate
    connect(ui2D->buttonCreateGate, SIGNAL(clicked(bool)), this, SLOT(slotMakeGate()));
//    connect(ui2D->buttonCreateGate, SIGNAL(clicked(bool)), this, SLOT(slotGate1Changed()));
 //   connect(ui2D->buttonCreateGate, SIGNAL(clicked(bool)), this, SLOT(slotGate2Changed()));
    connect(ui2D->buttonShowGate, SIGNAL(clicked(bool)), this, SLOT(slotShowGate1ExpVsSim(bool)));
    connect(ui2D->lineYGate1Low, SIGNAL(returnPressed()), this, SLOT(slotGate1Changed()));
    connect(ui2D->lineYGate1High, SIGNAL(returnPressed()), this, SLOT(slotGate1Changed()));
    connect(ui2D->lineYGate2Low, SIGNAL(returnPressed()), this, SLOT(slotGate2Changed()));
    connect(ui2D->lineYGate2High, SIGNAL(returnPressed()), this, SLOT(slotGate2Changed()));
    connect(ui2D->comboBoxLevel, SIGNAL(currentTextChanged(QString)), this, SLOT(slotSetLevelEnergy(QString)));
//
// FIt
    connect(ui2D->buttonFit, SIGNAL(clicked(bool)), this, SLOT(slot2DFitControler()));
    connect(m1, SIGNAL(signalRecalculateGamma(bool)), this, SLOT(slotShowGate1ExpVsSim(bool)));
//  Whole spectrum Chi2 calculation
    connect(ui2D->buttonChi2Calc, SIGNAL(clicked(bool)), this, SLOT(slotChi2Calculation()));

    std::cout << " ============== Analysis 2D =================" << std::endl;

    windowOpen_ = true;

    Project *myProject = Project::get();
    Histogram *histSim = Histogram::GetEmptyHistogram(0, 100, 100 / myProject->getBinning2Dfactor());
    fit2DController_->setSimulatedHistogram(histSim);
    delete histSim;


}

Analysis2D::~Analysis2D()
{ 
    delete yRectGate1Item_;
    delete yRectGate2Item_;
    delete yRectGate12DItem_;
    delete yRectGate22DItem_;
    //delete colorMap2D_;
    delete fit2DController_;
    delete m1;
    delete ui2D;
}

std::vector<double> Analysis2D::gateOnY(int low, int high)
{
    Project *myProject = Project::get();
    Histogram* exp2DHist = myProject->getExp2DHist();
    vector<double> tmp;
    for(unsigned int i=0; i!=xSize_; i++) tmp.push_back(0.0);
    cout << "Creating Gate on Y between: " << low <<" and " << high << " : spectrum length= " << xSize_ ;
    for(unsigned int x=0; x<xSize_; x++)
    {
        for(int y=low; y<high; y++ )
            tmp[x] += exp2DHist->GetBinValue(x*(xSize_+1)+y);
    }
    cout << " DONE" << endl;
    return tmp;
}

std::vector<double> Analysis2D::gateOnX(int low, int high)
{
    Project *myProject = Project::get();
    Histogram* exp2DHist = myProject->getExp2DHist();
    vector<double> tmp1;
    for(unsigned int i=0; i!=ySize_; i++) tmp1.push_back(0.0);
    cout << "Creating Gate on X between: " << low <<" and " << high ;

        for( int x=low; x<high; x++) {
            for(unsigned int y=0; y<ySize_; y++ )
                tmp1[y] += exp2DHist->GetBinValue(x*(xSize_+1)+y);
    }
    cout << " DONE" << endl;
    return tmp1;
}

void Analysis2D::setComboBoxLevel()
{
    DecayPath* decayPath_= DecayPath::get();
    std::vector<Transition*>* betaTransitions_ = decayPath_->GetBetaTransitionsFromFirstNuclide();
    std::vector <bool>  futureResults_;
    for(auto it = betaTransitions_->begin(); it != betaTransitions_->end(); ++it)
    {
        futureResults_.push_back(true);
    }

    for(unsigned int i = 0; i != futureResults_.size(); ++i)
    {
        QString QEnergy_ = QString::number(betaTransitions_->at(i)->GetFinalLevelEnergy());
        ui2D->comboBoxLevel->addItem(QEnergy_);
    }


}

void Analysis2D::setInitialValues()
{
    Project *myProject = Project::get();
    string HistID = myProject->getExp2DSpecID();
    ui2D->lineHistID->insert(QString::fromStdString(myProject->getExp2DSpecID()));
    Display2DXmin = 10.0;
    Display2DXmax = static_cast<double>(xSize_);
    Display2DYmin = 10.0;
    Display2DYmax = static_cast<double>(ySize_);
    gateOnXLow_ = Display2DXmin;
    gateOnXHigh_ = Display2DXmax;
    gateOnYLow_ = Display2DYmin;
    gateOnYHigh_ = Display2DYmax;
    ui2D->lineDisplayXmin->insert(QString::number(Display2DXmin));
    ui2D->lineDisplayXmax->insert(QString::number(Display2DXmax));
    ui2D->lineDisplayYmin->insert(QString::number(Display2DYmin));
    ui2D->lineDisplayYmax->insert(QString::number(Display2DYmax));
    ui2D->lineYGate1Low->insert(QString::number(gateOnYLow_));
    ui2D->lineYGate1High->insert(QString::number(gateOnYHigh_));
    ui2D->lineYGate2Low->insert(QString::number(gateOnYLow_));
    ui2D->lineYGate2High->insert(QString::number(gateOnYLow_ + 5));
}

void Analysis2D::setGraphics()
{
    Project *myProject = Project::get();
    ui2D->Plot2D->xAxis->setLabel("I,M,O single gamma Channels/Energy (keV)");
    ui2D->Plot2D->yAxis->setLabel("MTAS (full) Channels/Energy (kev)");

    ui2D->PlotProjOnX->xAxis->setLabel("I,M,O Channels/Energy (keV)");
    ui2D->PlotProjOnX->yAxis->setLabel("Counts");
    ui2D->PlotProjOnY->yAxis->setLabel(" MTAS Channels/Energy (keV)");
    ui2D->PlotProjOnY->xAxis->setLabel("Counts");

    Display2DXmax = ui2D->lineDisplayXmax->text().toDouble();
    Display2DXmin = ui2D->lineDisplayXmin->text().toDouble();
    Display2DYmax = ui2D->lineDisplayYmax->text().toDouble();
    Display2DXmin = ui2D->lineDisplayXmin->text().toDouble();
    gateOnYLow_ = ui2D->lineYGate1Low->text().toDouble();
    gateOnYHigh_ = ui2D->lineYGate1High->text().toDouble();
    ui2D->Plot2D->xAxis->setRange(Display2DXmin, Display2DXmax);
    ui2D->Plot2D->yAxis->setRange(Display2DYmin, Display2DYmax);
    ui2D->PlotProjOnX->xAxis->setRange(Display2DXmin,Display2DXmax);
    ui2D->PlotProjOnY->yAxis->setRange(Display2DYmin,Display2DYmax);

    colorMap2D_ = new QCPColorMap(ui2D->Plot2D->xAxis, ui2D->Plot2D->yAxis);
    colorMap2D_->data()->setRange(QCPRange(Display2DXmin, Display2DXmax), QCPRange(Display2DYmin, Display2DYmax));
    colorMap2D_->data()->setSize(Display2DXmax,Display2DYmax);
    colorMap2D_->setBrush(Qt::NoBrush);
    colorMap2D_->setPen(QPen(Qt::black));
    colorMap2D_->setDataScaleType(QCPAxis::stLogarithmic);  //stLinear or stLogarithmic
    colorMap2D_->setGradient(QCPColorGradient::gpPolar);
    colorMap2D_->setInterpolate(true);
    cout << "number of bins : " << myProject->getExp2DHist()->GetNrOfBins() << endl;
    cout << " Xmax: " << Display2DXmax << " Ymax: " <<Display2DYmax << " iloczyn: " <<  (Display2DXmax+1)*(Display2DYmax+1)<< endl;

    //=========== MK <-> MS place to change ==============
    //int expSpectra2Dbinning = 1;
    int expSpectra2Dbinning = myProject->getBinning2Dfactor();

    for (unsigned int x=0; x<xSize_; ++x)
    {
        unsigned int xpos = (x / expSpectra2Dbinning) * (xSize_+1);
        for (unsigned int y=0; y<ySize_; ++y)
        {
            double cellValue = myProject->getExp2DHist()->GetBinValue(xpos + y / expSpectra2Dbinning);
            //colorMap2D_->data()->setCell(x, y, cellValue / (expSpectra2Dbinning^2 ));
            colorMap2D_->data()->setCell(x, y, cellValue);
        }
    }

      colorMap2D_->rescaleDataRange(true);
      ui2D->Plot2D->rescaleAxes();
      ui2D->Plot2D->replot();

// ======= MTAS projection ===========

      ui2D->label_15->setStyleSheet("color: magenta");
      ui2D->label_4->setStyleSheet("color: green");
      ui2D->labelGate1Counts->setStyleSheet("color: green");
      ui2D->labelGate2Counts->setStyleSheet("color: magenta");
      projectionOnX =  gateOnY(gateOnYLow_ / expSpectra2Dbinning, gateOnYHigh_ / expSpectra2Dbinning);
      projection2OnX_ = gateOnY(gateOnYLow_ / expSpectra2Dbinning, gateOnYHigh_ / expSpectra2Dbinning);
      projectionOnY =  gateOnX(gateOnXLow_ / expSpectra2Dbinning, gateOnXHigh_ / expSpectra2Dbinning);

      for(unsigned int x=0; x<xSize_; x++)
          channelProjectionOnX.push_back(static_cast<double>(x * expSpectra2Dbinning));

      ui2D->PlotProjOnX->addGraph();
      ui2D->PlotProjOnX->graph(0)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
      ui2D->PlotProjOnX->graph(0)->setPen(QPen(Qt::green));
      ui2D->PlotProjOnX->graph(0)->setData(QVector<double>::fromStdVector(channelProjectionOnX),QVector<double>::fromStdVector(projectionOnX));
      ui2D->PlotProjOnX->addGraph();
      ui2D->PlotProjOnX->graph(1)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
      ui2D->PlotProjOnX->graph(1)->setPen(QPen(Qt::magenta));
      ui2D->PlotProjOnX->graph(1)->setData(QVector<double>::fromStdVector(channelProjectionOnX),QVector<double>::fromStdVector(projection2OnX_));

     double yMax = 1e4;
//      yMax = *max_element(projectionOnX.begin(),projectionOnX.end()) * 1.1;
      ui2D->PlotProjOnX->yAxis->setRange(1,yMax);
      ui2D->PlotProjOnX->replot();

      ui2D->PlotProjOnY->addGraph(ui2D->PlotProjOnY->yAxis, ui2D->PlotProjOnY->xAxis);
      ui2D->PlotProjOnY->graph(0)->setPen(QPen(Qt::black));
      ui2D->PlotProjOnY->graph(0)->setData(QVector<double>::fromStdVector(channelProjectionOnX),QVector<double>::fromStdVector(projectionOnY));
      ui2D->PlotProjOnY->graph(0)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
      yMax = *max_element(projectionOnY.begin(),projectionOnY.end()) * 1.1;
      ui2D->PlotProjOnY->yAxis->setRange(Display2DYmin,Display2DYmax);
      ui2D->PlotProjOnY->xAxis->setRange(1,yMax);
      ui2D->PlotProjOnY->replot();

      //delete colorMap2D;
}

void Analysis2D::slot2DFitControler()
{
// slot exacutes after Fit button is clicked
 fit2DController_->setHistId(ui2D->lineHistID->text().toInt());
 fit2DController_->setLambda_(ui2D->lineLambda->text().toDouble());
 fit2DController_->setNrOfIter(ui2D->lineNrOfIterations->text().toInt());
 fit2DController_->setBinningFactor(ui2D->lineBinForFit->text().toInt());
 fit2DController_->setLevelEnergy(ui2D->comboBoxLevel->currentText().toDouble());
 fit2DController_->setLeftLimit(ui2D->lineYGate1Low->text().toInt());
 fit2DController_->setRightLimit(ui2D->lineYGate1High->text().toInt());
 cout << " number of count in exp spec: " << fit2DController_->getExperimentalHistogram()->GetNrOfCounts() << endl;

 if(!prepareRestLevelResponseDoneFlag_)
 {
     fit2DController_->prepareRestLevelsResponseFromOutside();
//     prepareRestLevelResponseDoneFlag_=true;
 }
 cout << "=========================== NOW FIT================================" << endl;
 fit2DController_->makeXGammaFit();
 fit2DController_->calculateSimulatedHistogram();
 fit2DController_->calculateDiffHistogram();
}

void Analysis2D::slotGate1Changed()
{
    ui2D->PlotProjOnY->removeItem(yRectGate1Item_) ;
    ui2D->Plot2D->removeItem(yRectGate12DItem_) ;
    myProject->set2DGate1Low(ui2D->lineYGate1Low->text().toDouble());
    myProject->set2DGate1High(ui2D->lineYGate1High->text().toDouble());

}

void Analysis2D::slotGate2Changed()
{
    ui2D->PlotProjOnY->removeItem(yRectGate2Item_) ;
    ui2D->Plot2D->removeItem(yRectGate22DItem_) ;

}

void Analysis2D::slotMakeGate()
{
    slotGate1Changed();
    slotGate2Changed();
    int expSpectra2Dbinning = myProject->getBinning2Dfactor();
    double high = myProject->get2DGate1High();
    double low = myProject->get2DGate1Low();
    cout << "slotMakeGate " << "low: " <<low << " high: " << high << endl;
    projectionOnX = gateOnY(low / expSpectra2Dbinning, high / expSpectra2Dbinning);


    int min = 0;
    float max = high;
    cout << "slotMakeGate: min = " << min << ", max = " << max << endl;
    std::vector<float> fvect;
    for(unsigned k = min; k < high / expSpectra2Dbinning; k++)
        fvect.push_back(static_cast<float>(projectionOnX.at(k)));
    Histogram* tmpHist = new Histogram(min,max,fvect);
    //normalization here should depend on 2D binning probably
    tmpHist->Normalize(10. / expSpectra2Dbinning);
    myProject->setExpGate( *tmpHist );
    delete tmpHist;

    fit2DController_->setExperimentalHistogram( myProject->getExpGate() );

    ui2D->PlotProjOnX->graph(0)->setData(QVector<double>::fromStdVector(channelProjectionOnX),QVector<double>::fromStdVector(projectionOnX));
    double high2 = ui2D->lineYGate2High->text().toDouble();
    double low2 = ui2D->lineYGate2Low->text().toDouble();
    projection2OnX_ = gateOnY(low2 / expSpectra2Dbinning, high2 / expSpectra2Dbinning);
    ui2D->PlotProjOnX->graph(1)->setData(QVector<double>::fromStdVector(channelProjectionOnX),QVector<double>::fromStdVector(projection2OnX_));

    double yMax = *max_element(projectionOnY.begin(),projectionOnY.end()) * 1.1;

    yRectGate1Item_ = new QCPItemRect(ui2D->PlotProjOnY );

                    yRectGate1Item_->setVisible(true);

                    QColor green50 = Qt::green;
                    green50.setAlphaF( 0.5 );
                    yRectGate1Item_->setBrush(green50);
                    yRectGate1Item_->topLeft     ->setType(QCPItemPosition::ptPlotCoords);
                    yRectGate1Item_->topLeft->setCoords( 1, high );
                    yRectGate1Item_->bottomRight ->setType(QCPItemPosition::ptPlotCoords);
                    yRectGate1Item_->bottomRight->setCoords( yMax, low );

   yRectGate2Item_ = new QCPItemRect(ui2D->PlotProjOnY );
                   yRectGate2Item_->setVisible(true);

                   QColor magenta50 = Qt::magenta;
                   magenta50.setAlphaF( 0.5 );
                   yRectGate2Item_->setBrush(magenta50);
                   yRectGate2Item_->topLeft->setType(QCPItemPosition::ptPlotCoords);
                   yRectGate2Item_->topLeft->setCoords( 1, high2 );
                   yRectGate2Item_->bottomRight ->setType(QCPItemPosition::ptPlotCoords);
                   yRectGate2Item_->bottomRight->setCoords( yMax, low2 );

   yRectGate12DItem_ = new QCPItemRect(ui2D->Plot2D );
                    yRectGate12DItem_->setVisible(true);
                    yRectGate12DItem_->setBrush(green50);
                    yRectGate12DItem_->topLeft->setType(QCPItemPosition::ptPlotCoords);
                    yRectGate12DItem_->topLeft->setCoords(1, high);
                    yRectGate12DItem_->bottomRight->setType(QCPItemPosition::ptPlotCoords);
                    yRectGate12DItem_->bottomRight->setCoords(xSize_, low);


   yRectGate22DItem_ = new QCPItemRect(ui2D->Plot2D );
                     yRectGate22DItem_->setVisible(true);
                     yRectGate22DItem_->setBrush(magenta50);
                     yRectGate22DItem_->topLeft->setType(QCPItemPosition::ptPlotCoords);
                     yRectGate22DItem_->topLeft->setCoords(1, high2);
                     yRectGate22DItem_->bottomRight->setType(QCPItemPosition::ptPlotCoords);
                     yRectGate22DItem_->bottomRight->setCoords(xSize_, low2);

    cout << "Analysis2D::slotMakeGate: Total NrOfCount Gate-1: " << std::accumulate(projectionOnX.begin(), projectionOnX.end(), 0.0)<< endl;
    QString qstr1 = "Number of counts in Gate # 1: " + QString::number(std::accumulate(projectionOnX.begin(), projectionOnX.end(), 0.0));
    ui2D->labelGate1Counts->setText(qstr1);
    QString qstr2 = "Number of counts in Gate # 2: " + QString::number(std::accumulate(projection2OnX_.begin(), projection2OnX_.end(), 0.0));
    ui2D->labelGate2Counts->setText(qstr2);
    slotReplotGraphics();

}

void Analysis2D::slotReplotGraphics()
{
    if(windowOpen_){

        Display2DXmax = ui2D->lineDisplayXmax->text().toDouble();
        Display2DXmin = ui2D->lineDisplayXmin->text().toDouble();
        Display2DYmax = ui2D->lineDisplayYmax->text().toDouble();
        Display2DYmin = ui2D->lineDisplayYmin->text().toDouble();

        ui2D->Plot2D->xAxis->setRange(Display2DXmin, Display2DXmax);
        ui2D->Plot2D->yAxis->setRange(Display2DYmin, Display2DYmax);
        ui2D->PlotProjOnX->xAxis->setRange(Display2DXmin,Display2DXmax);
        ui2D->PlotProjOnY->yAxis->setRange(Display2DYmin,Display2DYmax);
        double yMax = vectorMax(projectionOnX,Display2DXmin,Display2DXmax);
        ui2D->PlotProjOnX->yAxis->setRange(1,yMax);
        yMax = vectorMax(projectionOnY,Display2DYmin,Display2DYmax);

        ui2D->PlotProjOnY->xAxis->setRange(1,yMax);
    }

    colorMap2D_->rescaleDataRange(true);

    ui2D->Plot2D->replot();
    ui2D->PlotProjOnX->replot();
    ui2D->PlotProjOnY->replot();

}

void Analysis2D::slotSetLevelEnergy(QString qLevelEnergy)
{

    fit2DController_->setLevelEnergy(qLevelEnergy.toDouble());
    fit2DController_->setHistId(ui2D->lineHistID->text().toInt());
    fit2DController_->setLeftLimit(ui2D->lineYGate1Low->text().toInt());
    fit2DController_->setRightLimit(ui2D->lineYGate1High->text().toInt());

    fit2DController_->findCorrespondingLevel();

    fit2DController_->prepareRestLevelsResponseFromOutside();
    prepareRestLevelResponseDoneFlag_=true;

    fit2DController_->prepareTransitionResponses();
    fit2DController_->prepareFeedings();
    fit2DController_->calculateSimulatedHistogram();
    fit2DController_->calculateDiffHistogram();

    ui2D->buttonShowGate->setEnabled(true);

QMessageBox msgBox;
QString info;
info = "Energy of level to be fitted: " + qLevelEnergy;
msgBox.setText(info);
msgBox.exec();

}

void Analysis2D::slotSetLogScaleXProj(bool checked)
{
    if (checked) {
      ui2D->PlotProjOnX->yAxis->setScaleType(QCPAxis::stLogarithmic);
    } else {
        ui2D->PlotProjOnX->yAxis->setScaleType(QCPAxis::stLinear);
    }
    ui2D->PlotProjOnX->replot();
}

void Analysis2D::slotSetLogScaleYProj(bool checked)
{
    if (checked) {
      ui2D->PlotProjOnY->xAxis->setScaleType(QCPAxis::stLogarithmic);
    } else {
        ui2D->PlotProjOnY->xAxis->setScaleType(QCPAxis::stLinear);
    }
    ui2D->PlotProjOnY->replot();
}

void Analysis2D::slotShowGate1ExpVsSim(bool recalculateTransitions)
{
//    ManualFitGraph *m1 = new ManualFitGraph();

// change background color if opened
//---------Table-------------

    cout << "Analysis2D::slotShowGate1ExpVsSim: recalculateTransitions = " << recalculateTransitions << endl;

    std::vector <bool> futureResults;
    std::vector <bool> displayStatus;
    QStringList header;
    std::vector<RowData> rowData_;

    header << "Display ?" << "Final Level Energy" <<"Fitting flag" << "Intensity[%]" ;
    m1->ManualFitGraph::setHeader(header);
    m1->show();

    Level* level_ = myProject->GetCurrent2DFitLevel();
    DecayPath* decayPath_= DecayPath::get();
    std::vector<Transition*>* transitions_ = level_->GetTransitions();
    for(auto it = transitions_->begin(); it != transitions_->end(); ++it)
    {
        futureResults.push_back(true);
        displayStatus.push_back(true);
    }

    for(unsigned int i = 0; i != futureResults.size(); ++i)
    {
        QString QDisplayStatus_ = displayStatus.at(i) ? "true" : "false";
        QString QEnergy_ = QString::number(transitions_->at(i)->GetFinalLevelEnergy());
        QString QIntensity_ = QString::number(transitions_->at(i)->GetIntensity()*100);
        QString QFittingFlag_ = transitions_->at(i)->GetIntensityFitFlag() ? "true" : "false";
        rowData_.push_back(RowData(QDisplayStatus_, QEnergy_, QFittingFlag_, QIntensity_));
    }
    m1->ManualFitGraph::initializeTable(rowData_);

//-------end of Table related code ---
    //float xMin = 0;
    //float xMax = ui2D->lineDisplayXmax->text().toDouble() ;  //expHist->GetNrOfBins();
//    float xMax = 8000.0;
//    if( recalculateTransitions )
//    {
       fit2DController_->prepareFeedings();
       fit2DController_->calculateSimulatedHistogram();
       fit2DController_->calculateDiffHistogram();
//    }

    QVector<double> x1 = QVector<double>::fromStdVector(fit2DController_->getExperimentalHistogram()->GetEnergyVectorD());
    QVector<double> y1 = QVector<double>::fromStdVector(fit2DController_->getExperimentalHistogram()->GetAllDataD());
    QVector<double> x2 = QVector<double>::fromStdVector(fit2DController_->getRecalculatedHistogram()->GetEnergyVectorD());
    QVector<double> y2 = QVector<double>::fromStdVector(fit2DController_->getRecalculatedHistogram()->GetAllDataD());
    QVector<double> d12 = QVector<double>::fromStdVector(fit2DController_->getDiffHistogram()->GetAllDataD());

     string respType = "g";
     m1->setResponseType(respType);
    m1->setxMax(ui2D->lineDisplayXmax->text());
    m1->setxMin(ui2D->lineDisplayXmin->text());
    m1->initializeGraphs();
    m1->showDataExpSimDiff(x1, y1, x2, y2, d12);
    m1->showResponseFunctions();

//-----spectra display code ------


//----- end of spectra display code ------
    m1->show();

}

void Analysis2D::slotChi2Calculation()
{
    // path creation copy-pasted checkAndPreparePath from exportFIles.cpp
    QDateTime now = QDateTime::currentDateTime();
    const QString timestamp = now.toString(QLatin1String("yyyyMMdd-hhmm"));
    QDir newDir(timestamp);
    QDir().mkdir(timestamp);
    QDir currentDir = QDir::current();
    string currentPath = currentDir.absolutePath().toStdString();
    string path = currentPath + "/" + timestamp.toStdString() + "/";

    string generalFilename = "WholeSpectrumChi2.txt";
    string outputFilename = path + generalFilename;
    ofstream outputFile(outputFilename.c_str());
    if (!outputFile.is_open())
        cout << "Warning message: The file " + (string) outputFilename + " is not open!" << endl;

    int binWidth = ui2D->lineChi2Bin->text().toInt();
    int startE = ui2D->lineChi2StartE->text().toInt();
    int stopE = ui2D->lineChi2StopE->text().toInt();
    int startChi2 = ui2D->lineChi2StartChi2->text().toInt();

    double chi2WholeGate = 0.;
    outputFile << "StartE | StopE | Chi2WholeGate from " + to_string(startChi2) << endl;

    for(int i = startE; i < stopE; i += binWidth)
    {
        string specificFilename = to_string(i) + "_" + to_string(i + binWidth) + ".txt";
        string outputSpecificFilename = path + specificFilename;
        ofstream outputSpecificFile(outputSpecificFilename.c_str());
        if (!outputSpecificFile.is_open())
            cout << "Warning message: The file " + (string) outputSpecificFilename + " is not open!" << endl;

        if(fit2DController_ == 0L)
            fit2DController_ = new TwoDimFitController();

        int expSpectra2Dbinning = myProject->getBinning2Dfactor();
        projectionOnX = gateOnY(i / expSpectra2Dbinning, (i + binWidth) / expSpectra2Dbinning);
        //next section is probably overcomplicated
        int min = 0;
        int max = i + binWidth;
        std::vector<float> fvect;
        for(int k = min; k < max / expSpectra2Dbinning; k++)
            fvect.push_back(static_cast<float>(projectionOnX.at(k)));
        Histogram* tmpHist = new Histogram(min, max, fvect);
        tmpHist->Normalize(10. / expSpectra2Dbinning);
        myProject->setExpGate( *tmpHist );
        delete tmpHist;
        fit2DController_->setExperimentalHistogram( myProject->getExpGate() );
        // overcomlication ends here

        fit2DController_->setHistId(ui2D->lineHistID->text().toInt());
        fit2DController_->setLeftLimit( i );
        fit2DController_->setRightLimit( i + binWidth );

        fit2DController_->prepareRestLevelsResponseFromOutside();
        fit2DController_->calculateSimulatedHistogram();

        vector<double> expEnergyVector = fit2DController_->getExperimentalHistogram()->GetEnergyVectorD();
        vector<double> expDataVector = fit2DController_->getExperimentalHistogram()->GetAllDataD();
        vector<double> simEnergyVector = fit2DController_->getRecalculatedHistogram()->GetEnergyVectorD();
        vector<double> simDataVector = fit2DController_->getRecalculatedHistogram()->GetAllDataD();
        //outputSpecificFile << "expEnergyVector.size() = " << expEnergyVector.size() << endl;
        //outputSpecificFile << "expDataVector.size() = " << expDataVector.size() << endl;
        //outputSpecificFile << "simEnergyVector.size() = " << simEnergyVector.size() << endl;
        //outputSpecificFile << "simDataVector.size() = " << simDataVector.size() << endl;
        outputSpecificFile << "expEnergy | expData | simEnergy | simData | Chi2" << endl;

        double chi2EachBin = 0.;
        int maxVectorSize = 0;
        if(expEnergyVector.size() <= simEnergyVector.size())
            maxVectorSize = expEnergyVector.size();
        else
            maxVectorSize = simEnergyVector.size();

        for(int j = 0; j < maxVectorSize; j++)
        {
            if(expEnergyVector.at(j) == 0)
            {
                outputSpecificFile << "0 0 0 0 0" << endl;
                continue;
            }
            double simCounts = simDataVector.at(j);
            double expCounts = expDataVector.at(j);
            if(expEnergyVector.at(j) >= startChi2 && expCounts > 0)
                chi2EachBin = (simCounts - expCounts) * (simCounts - expCounts) / expCounts;
            else
                chi2EachBin = 0.;
            outputSpecificFile << expEnergyVector.at(j) << " " << expDataVector.at(j) << " " <<
                                  simEnergyVector.at(j) << " " << simDataVector.at(j) << " " << chi2EachBin << endl;

            chi2WholeGate += chi2EachBin;
        }


// Chi2 as in Pearson's chi-square test https://en.wikipedia.org/wiki/Goodness_of_fit
        outputFile << i << " " << i + binWidth << " " << chi2WholeGate << endl;

        chi2WholeGate = 0.;
        delete fit2DController_;
        fit2DController_ = 0L;
        outputSpecificFile.close();
    }
    outputFile.close();
}

//===== functions repeated from HistogramGraph
double Analysis2D::vectorMax(std::vector<double> yM, double minEn, double maxEn)
{
//    int xposition = 0;
    double yMax=1;
    unsigned int xposMin = 0;
    unsigned int xposMax = 16000;
    int j=0;
   while(channelProjectionOnX.at(j) < minEn)
    {
     j++;
    }
            xposMin=j;
    while(channelProjectionOnX.at(j) < maxEn-1)
    {
        j++;
    }
            xposMax=j;

    for(unsigned int i = xposMin;  i < xposMax; ++i)
    {
            yMax=max(yMax,abs(yM.at(i)));
    }

    return yMax;
}

void Analysis2D::slotClose()
{
    emit signalClose2dUI();
    delete colorMap2D_;
}
