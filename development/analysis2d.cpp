#include "analysis2d.h"
#include "ui_analysis2d.h"

#include "project.h"
#include "DeclareHeaders.hh"
#include "twoDimFitController.h"
#include "manualfitgraph.h"
//#include "histogramGraphController.h"

#include <string>
#include <iostream>


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
        connect(ui2D->buttonClose, SIGNAL(clicked()), this, SLOT(close()));
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

    std::cout << " ============== Analysis 2D =================" << std::endl;

    windowOpen_ = true;

    Histogram *histSim = Histogram::GetEmptyHistogram();
    fit2DController_->setSimulatedHistogram(histSim);
    delete histSim;


}

Analysis2D::~Analysis2D()
{
    delete ui2D;
    delete fit2DController_;
    delete m1;
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
    ui2D->lineYGate2High->insert(QString::number(gateOnYHigh_));
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

    QCPColorMap *colorMap2D = new QCPColorMap(ui2D->Plot2D->xAxis, ui2D->Plot2D->yAxis);
    colorMap2D->data()->setRange(QCPRange(Display2DXmin, Display2DXmax), QCPRange(Display2DYmin, Display2DYmax));
    colorMap2D->data()->setSize(Display2DXmax,Display2DYmax);
    colorMap2D->setBrush(Qt::NoBrush);
    colorMap2D->setPen(QPen(Qt::black));
    colorMap2D->setDataScaleType(QCPAxis::stLogarithmic);  //stLinear
    colorMap2D->setGradient(QCPColorGradient::gpPolar);
    colorMap2D->setInterpolate(true);
    cout << "number of bins : " << myProject->getExp2DHist()->GetNrOfBins() << endl;
    cout << " Xmax: " << Display2DXmax << " Ymax: " <<Display2DYmax << " iloczyn: " <<  (Display2DXmax+1)*(Display2DYmax+1)<< endl;


    for (unsigned int x=0; x<xSize_; ++x){
        unsigned int xpos = x*(xSize_+1);
        for (unsigned int y=0; y<ySize_; ++y){
          colorMap2D->data()->setCell(x, y, myProject->getExp2DHist()->GetBinValue(xpos+y));
        }
    }
      colorMap2D->rescaleDataRange(true);
      ui2D->Plot2D->rescaleAxes();
      ui2D->Plot2D->replot();

// ======= MTAS projection ===========

      ui2D->label_15->setStyleSheet("color: magenta");
      ui2D->label_4->setStyleSheet("color: green");
      ui2D->labelGate1Counts->setStyleSheet("color: green");
      ui2D->labelGate2Counts->setStyleSheet("color: magenta");
      projectionOnX =  gateOnY(gateOnYLow_,gateOnYHigh_);
      projection2OnX_ = gateOnY(gateOnYLow_,gateOnYHigh_);
      //projectionOnX =  gateOnY(gateOnYLow_,gateOnYLow_ + 10);
      //projection2OnX_ = gateOnY(gateOnYLow_,gateOnYLow_ + 10);

      projectionOnY =  gateOnX(gateOnXLow_,gateOnXHigh_);
      //projectionOnY =  gateOnX(gateOnXLow_,gateOnXLow_ + 10);

      for(unsigned int x=0; x<xSize_; x++)   channelProjectionOnX.push_back(static_cast<double>(x));

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
}

void Analysis2D::slot2DFitControler()
{
    cout << "Analysis2D:slot2DFitControler  STARTED"  << endl;
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
     prepareRestLevelResponseDoneFlag_=true;
 }
 cout << "=========================== NOW FIT================================" << endl;
 fit2DController_->makeXGammaFit();
 fit2DController_->calculateSimulatedHistogram();
 fit2DController_->calculateDiffHistogram();
 cout << "Analysis2D:slot2DFitControler  FINISHED"  << endl;

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
    double high = myProject->get2DGate1High();
    double low = myProject->get2DGate1Low();
    cout << "slotMakeGate " << "low: " <<low << " high: " << high << endl;
    projectionOnX = gateOnY(low,high);


    float min = 0.0;
    float max = high;
    cout << "slotMakeGate: min = " << min << ", max = " << max << endl;
    std::vector<float> fvect;
    for(unsigned k = 0; k < high; k++)
        fvect.push_back(static_cast<float>(projectionOnX.at(k)));
    Histogram* tmpHist = new Histogram(min,max,fvect);
    tmpHist->Normalize(1.);
    myProject->setExpGate( *tmpHist );
    fit2DController_->setExperimentalHistogram( myProject->getExpGate() );

    ui2D->PlotProjOnX->graph(0)->setData(QVector<double>::fromStdVector(channelProjectionOnX),QVector<double>::fromStdVector(projectionOnX));
    double high2 = ui2D->lineYGate2High->text().toDouble();
    double low2 = ui2D->lineYGate2Low->text().toDouble();
    projection2OnX_ = gateOnY(low2,high2);
    ui2D->PlotProjOnX->graph(1)->setData(QVector<double>::fromStdVector(channelProjectionOnX),QVector<double>::fromStdVector(projection2OnX_));

    yRectGate1Item_ = new QCPItemRect(ui2D->PlotProjOnY );

                    yRectGate1Item_->setVisible(true);

                    QColor green50 = Qt::green;
                    green50.setAlphaF( 0.5 );
                    yRectGate1Item_->setBrush(green50);
                    yRectGate1Item_->topLeft     ->setType(QCPItemPosition::ptPlotCoords);
                    yRectGate1Item_->topLeft->setCoords( 1, high );
                    yRectGate1Item_->bottomRight ->setType(QCPItemPosition::ptPlotCoords);
                    yRectGate1Item_->bottomRight->setCoords( 1e4, low );

   yRectGate2Item_ = new QCPItemRect(ui2D->PlotProjOnY );
                   yRectGate2Item_->setVisible(true);

                   QColor magenta50 = Qt::magenta;
                   magenta50.setAlphaF( 0.5 );
                   yRectGate2Item_->setBrush(magenta50);
                   yRectGate2Item_->topLeft->setType(QCPItemPosition::ptPlotCoords);
                   yRectGate2Item_->topLeft->setCoords( 1, high2 );
                   yRectGate2Item_->bottomRight ->setType(QCPItemPosition::ptPlotCoords);
                   yRectGate2Item_->bottomRight->setCoords( 1e4, low2 );

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

//    cout << "Analysis2D::slotMakeGate: Total NrOfCount Gate-1: " << fit2DController_->getExperimentalHistogram()->GetNrOfCounts() << endl;
    cout << "Analysis2D::slotMakeGate: Total NrOfCount Gate-1: " << std::accumulate(projectionOnX.begin(), projectionOnX.end(), 0.0)<< endl;
    QString qstr1 = "Number of counts in Gate # 1: " + QString::number(std::accumulate(projectionOnX.begin(), projectionOnX.end(), 0.0));
    ui2D->labelGate1Counts->setText(qstr1);
    QString qstr2 = "Number of counts in Gate # 2: " + QString::number(std::accumulate(projection2OnX_.begin(), projection2OnX_.end(), 0.0));
    ui2D->labelGate2Counts->setText(qstr2);
    cout << " before replot" << endl;
    slotReplotGraphics();
//    ui2D->Plot2D->replot();
//    ui2D->PlotProjOnX->replot();
//    ui2D->PlotProjOnY->replot();

}

void Analysis2D::slotReplotGraphics()
{
    if(windowOpen_){

        Display2DXmax = ui2D->lineDisplayXmax->text().toDouble();
        Display2DXmin = ui2D->lineDisplayXmin->text().toDouble();
        Display2DYmax = ui2D->lineDisplayYmax->text().toDouble();
        Display2DYmin = ui2D->lineDisplayYmin->text().toDouble();
        cout << "ReplotGraphics-1" << endl;

        ui2D->Plot2D->xAxis->setRange(Display2DXmin, Display2DXmax);
        ui2D->Plot2D->yAxis->setRange(Display2DYmin, Display2DYmax);
        cout << "ReplotGraphics-1a" << endl;
        ui2D->PlotProjOnX->xAxis->setRange(Display2DXmin,Display2DXmax);
        ui2D->PlotProjOnY->yAxis->setRange(Display2DYmin,Display2DYmax);
        cout << "ReplotGraphics-1b" << endl;
        cout << "min: " << Display2DXmin << "max: " << Display2DXmax << endl;
        double yMax = vectorMax(projectionOnX,Display2DXmin,Display2DXmax);
        cout << " granice" << Display2DXmin << " : " << Display2DXmax << endl;
        ui2D->PlotProjOnX->yAxis->setRange(1,yMax);
        yMax = vectorMax(projectionOnY,Display2DYmin,Display2DYmax);
        cout << "ReplotGraphics-2" << endl;

        ui2D->PlotProjOnY->xAxis->setRange(1,yMax);
    }

    cout << "ReplotGraphics-3" << endl;
    QCPColorMap *colorMap2D = new QCPColorMap(ui2D->Plot2D->xAxis, ui2D->Plot2D->yAxis);

    colorMap2D->rescaleDataRange(true);

    ui2D->Plot2D->replot();
    ui2D->PlotProjOnX->replot();
    ui2D->PlotProjOnY->replot();
    cout << "ReplotGraphics-4" << endl;

}

void Analysis2D::slotSetLevelEnergy(QString qLevelEnergy)
{

    fit2DController_->setLevelEnergy(qLevelEnergy.toDouble());
    fit2DController_->setHistId(ui2D->lineHistID->text().toInt());
    fit2DController_->setLeftLimit(ui2D->lineYGate1Low->text().toInt());
    fit2DController_->setRightLimit(ui2D->lineYGate1High->text().toInt());

//    cout << "energy: " << fit2DController_->getLevelEnergy() << endl;
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

    header << "Display ?" << "Final Level Energy" << "Intensity[%]" ;
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
        rowData_.push_back(RowData(QDisplayStatus_, QEnergy_, QIntensity_));
    }
    m1->ManualFitGraph::initializeTable(rowData_);

//-------end of Table related code ---
    //float xMin = 0;
    //float xMax = ui2D->lineDisplayXmax->text().toDouble() ;  //expHist->GetNrOfBins();
//    float xMax = 8000.0;
    if( recalculateTransitions )
    {
       fit2DController_->prepareFeedings();
       fit2DController_->calculateSimulatedHistogram();
       fit2DController_->calculateDiffHistogram();
    }

    QVector<double> x1 = QVector<double>::fromStdVector(fit2DController_->getExperimentalHistogram()->GetEnergyVectorD());
    QVector<double> y1 = QVector<double>::fromStdVector(fit2DController_->getExperimentalHistogram()->GetAllDataD());
    QVector<double> y2 = QVector<double>::fromStdVector(fit2DController_->getRecalculatedHistogram()->GetAllDataD());
    QVector<double> d12 = QVector<double>::fromStdVector(fit2DController_->getDiffHistogram()->GetAllDataD());
    //cout << "Analysis2D::slotShowGate1ExpVsSim: Total NrOfCount ExpHist: " << fit2DController_->getExperimentalHistogram()->GetNrOfCounts() << endl;
    //cout << "Analysis2D::slotShowGate1ExpVsSim: Total NrOfCount SimHist: " << fit2DController_->getSimulatedHistogram()->GetNrOfCounts() << endl;
    //cout << "Analysis2D::slotShowGate1ExpVsSim: Total NrOfCount RecHist: " << fit2DController_->getRecalculatedHistogram()->GetNrOfCounts() << endl;
    //cout << "Analysis2D::slotShowGate1ExpVsSim: Total NrOfCount LevelsRespHist: " << fit2DController_->getLevelsRespHistogram()->GetNrOfCounts() << endl;


     string respType = "g";
     m1->setResponseType(respType);
    m1->setxMax(ui2D->lineDisplayXmax->text());
    m1->setxMin(ui2D->lineDisplayXmin->text());
    m1->initializeGraphs();
    m1->showDataExpSimDiff(x1, y1, y2, d12);
    m1->showResponseFunctions();

//-----spectra display code ------


//----- end of spectra display code ------
    m1->show();

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


