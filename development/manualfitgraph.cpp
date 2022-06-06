#include "DeclareHeaders.hh"
#include "manualfitgraph.h"
#include "ui_manualfitgraph.h"
#include  "responsefunction.h"

/*
class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:

    void signalUpdateSpecPlot(bool someValue);

}
*/


ManualFitGraph::ManualFitGraph(QWidget *parent) :
    QWidget(parent),
    uiM(new Ui::ManualFitGraph)
{
    myProject = Project::get();

    uiM->setupUi(this);

    uiM->histogramPlot->setInteraction(QCP::iRangeDrag, true);
    uiM->histogramPlot->setInteraction(QCP::iRangeZoom, true);
    uiM->histogramPlot->setInteraction(QCP::iSelectPlottables, true);
    uiM->histogramPlot->xAxis->setLabel("Channels/Energy (keV)");
    uiM->histogramPlot->yAxis->setLabel("Counts");

    uiM->histogramDiffPlot->xAxis->setLabel("Channels/Energy (keV)");
    uiM->histogramDiffPlot->yAxis->setLabel("(Exp - Sim)");

    QPalette ps = uiM->checkShowSIM->palette();
    ps.setColor(QPalette::Active,QPalette::WindowText,Qt::red);
    uiM->checkShowSIM->setPalette(ps);
    uiM->checkShowSIM->show();

    connect(uiM->histogramPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(slotShowPointToolTip(QMouseEvent*)));
    connect(uiM->LogScaleCheck, SIGNAL(clicked(bool)), this, SLOT(slotSetLogScale(bool)));
    connect(uiM->lineEditXAxisMax, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(uiM->lineEditXAxisMin, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(uiM->lineEditYAxisMax, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(uiM->lineEditYAxisMin, SIGNAL(returnPressed()), this, SLOT(slotSetAxisRange()));
    connect(uiM->tableLevels, SIGNAL(cellClicked(int,int)), this, SLOT(slotDisplayStatusClicked(int,int)));
    connect(uiM->tableLevels, SIGNAL(cellChanged(int,int)), this, SLOT(slotUpdateTableData(int,int)));
    connect(uiM->buttonRecalculate, SIGNAL(clicked(bool)), this, SLOT(slotRecalculate()));
    connect(uiM->checkContrOtherLevels, SIGNAL(clicked(bool)), this, SLOT(slotShowOtherLevelsContribution(bool)));
    connect(uiM->buttonClose, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(uiM->buttonNormalise, SIGNAL(clicked(bool)), this, SLOT(slotNormalise(bool)));
    connect(uiM->checkShowSIM, SIGNAL(clicked(bool)),this,SLOT(slotShowSimSpec(bool)));


    //Mouse right click actions
        setContextMenuPolicy(Qt::ActionsContextMenu);
        FittingStatusTrueAction_ = new QAction("Set Fitting status TRUE");
        FittingStatusFalseAction_ = new QAction("Set Fitting status FALSE");
        DisplayStatusTrueAction_ = new QAction("Set Display status TRUE");
        DisplayStatusFalseAction_ = new QAction("Set Display status FALSE");
        connect(FittingStatusTrueAction_, SIGNAL(triggered()), this, SLOT(slotFittingStatusTrue()));
        connect(FittingStatusFalseAction_, SIGNAL(triggered()), this, SLOT(slotFittingStatusFalse()));
        connect(DisplayStatusTrueAction_, SIGNAL(triggered()), this, SLOT(slotDisplayStatusTrue()));
        connect(DisplayStatusFalseAction_, SIGNAL(triggered()), this, SLOT(slotDisplayStatusFalse()));
        addAction(FittingStatusTrueAction_);
        addAction(FittingStatusFalseAction_);
        addAction(DisplayStatusTrueAction_);
        addAction(DisplayStatusFalseAction_);
    //Mouse right click acctions END

    boolTableDataInitialised = false;
//    xMaxInitial_ = 6000;
//    xMinInitial_ = 0;

    //delete DisplayStatusTrueAction;
    //delete DisplayStatusFalseAction;
}

ManualFitGraph::~ManualFitGraph()
{
    delete FittingStatusTrueAction_;
    delete FittingStatusFalseAction_;
    delete DisplayStatusTrueAction_;
    delete DisplayStatusFalseAction_;
    delete uiM;
}

void ManualFitGraph::setHeader(QStringList header)
{
    uiM->tableLevels->setHorizontalHeaderLabels(header);
    if(respType_ == "c"){   // for spectra comparision no Fitting status column
        removeAction(FittingStatusTrueAction_);
        removeAction(FittingStatusFalseAction_);
    }
}

void ManualFitGraph::slotDisplayStatusClicked( int row, int column)
{
    bool status;
    if((respType_ == "c") && (column == 2))return;
    if(column ==0)
    {
    displayStatus.at(row)= !displayStatus.at(row);
    slotStatusClicked(displayStatus.at(row), row, column);
    } else if(column ==2 ){
        QString qstatus = uiM->tableLevels->item(row,column)->text();
        if(qstatus == "true") status = false;
        if(qstatus == "false") status = true;
      slotStatusClicked(status, row, column);
    }

}
void ManualFitGraph::slotStatusClicked(bool status, int row, int column)
{
     QTableWidget *pointerToTable_ = 0L;
     QTableWidgetItem* tempQTableWidgetItem = new QTableWidgetItem(status ? "true" : "false");
     pointerToTable_ = uiM->tableLevels;
    if(column == 0){
    pointerToTable_->setItem(row, 0, tempQTableWidgetItem);
    pointerToTable_->show();
    int start = 0; //exp and simulation spectra
    if(respType_ == "l")start = 2;
    if(respType_ == "g")start = 2;
    if(respType_ == "c")start = 0;
    //mk    if(respType_ == "l")
//    {
     if(displayStatus.at(row))uiM->histogramPlot->graph(row+start)->setVisible(true);
     if(!displayStatus.at(row))uiM->histogramPlot->graph(row+start)->setVisible(false);
//    }
//    if(respType_ == "l")showResponseFunctionsLevels(2);
//    if(respType_ == "g")showResponseFunctionsGammas(2);
//    if(respType_ == "c")showSpectra(2);
    uiM->histogramPlot->replot();
    } else if(column == 2){
     pointerToTable_->setItem(row, 2, tempQTableWidgetItem);
     pointerToTable_->show();
    }

    //delete tempQTableWidgetItem;
}

void ManualFitGraph::slotRecalculate()
{
    uiM->checkShowSIM->setChecked(true);
    uiM->checkShowSIM->show();
    if(respType_ == "l") emit signalRecalculateLevel();
    if(respType_ == "g") emit signalRecalculateGamma(true);
}

void ManualFitGraph::slotNormalise(bool ok)
{
    if(respType_ == "l")slotNormaliseBetaFeeding(ok);
    if(respType_ == "g")slotNormaliseTransitionsFeeding(ok);
}

void ManualFitGraph::slotUpdateTableData(int row, int column)
{
    if(respType_ == "l")changeLevelTable(row,column);
    if(respType_ == "g")changeGammaTable(row,column);
    if(respType_ == "c")changeSpectraTable(row,column);

}

void ManualFitGraph::setCurrentLevel(double energy)
{

}

void ManualFitGraph::setxMin(QString qstr)
{
    uiM->lineEditXAxisMin->clear();
    uiM->lineEditXAxisMin->insert(qstr);
//mk    xMinInitial_ =qstr.toDouble();
}

void ManualFitGraph::setxMax(QString qstr)
{
    uiM->lineEditXAxisMax->clear();
    uiM->lineEditXAxisMax->insert(qstr);
//mk    xMaxInitial_ = qstr.toDouble();
}


void ManualFitGraph::changeGammaTable(int row, int column)
{
    if(boolTableDataInitialised){

        ResponseFunction* responseFunction = ResponseFunction::get();

//        std::cout << "In slotUpdateFeedingData : " << std::endl;
//        DecayPath* decayPath_= DecayPath::get();
//        std::vector<Nuclide>* nuclides_ = decayPath_->GetAllNuclides();
//        std::vector <Level*> levels = decayPath->GetAllLevels();
        std::vector<Transition*>* transitionsVec_ = myProject->GetCurrent2DFitLevel()->GetTransitions();
        QString energy;
//        Transition* transitionToBeChanged_;
        QString qstr = uiM->tableLevels->item(row,column)->text();
//        qDebug() << "przeczytano: " << qstr << "row: "<<row<<"column: "<<column;
        string str = qstr.toUtf8().constData();
        if(column==0){
            if (str == "true")displayStatus.at(row) = true;
            if (str == "false")displayStatus.at(row) = false;
            return;
         } else if (column==1) {
 //           std::cout << " nie zmieniamy energii gammy" << std::endl;
                    return;
        }  else if(column==2) {
            if(qstr=="true")transitionsVec_->at(row)->SetIntensityFitFlag(true);
            if(qstr=="false")transitionsVec_->at(row)->SetIntensityFitFlag(false);
        }  else if(column==3) {
             double newIntensity = qstr.toDouble()/100;
//             qDebug() << "przed ustawieniem: " << newIntensity <<"bylo " <<transitionsVec_->at(row)->GetIntensity();
             QTableWidgetItem* itm = uiM->tableLevels->item( row, 1 );
             if (itm) energy = itm->text();
             //double transitionEnergy = energy.toDouble();

             transitionsVec_->at(row)->ChangeIntensity(newIntensity);
             responseFunction->ChangeContainerTransitionIntensity( transitionsVec_->at(row), newIntensity );
             responseFunction->RefreshFlags();

             //responseFunction->GetPointerToCorrespondingStructure( transitionsVec_->at(row) )->transitionIntensity = newIntensity;
             //responseFunction->GetPointerToCorrespondingStructure( transitionsVec_->at(row) )->transitionResponseReady = false;

             cout<< "New Intensity: " <<  transitionsVec_->at(row)->GetIntensity();
        } else {
 //           std::cout << "inne columny" << std::endl;
            return;
        }
        }
}

void ManualFitGraph::changeSpectraTable(int row, int column)
{
    if(boolTableDataInitialised){


        QString qstr = uiM->tableLevels->item(row,column)->text();
        string str = qstr.toUtf8().constData();
        if(column==0){
            if (str == "true")displayStatus.at(row) = true;
            if (str == "false")displayStatus.at(row) = false;
            return;
         } else if (column==1){ //spectra histID
                return;
        }  else if(column==2){
                return;
        }  else if(column==3) {
        } else {
            std::cout << "inne columny" << std::endl;
            return;
        }
    }
}

void ManualFitGraph::changeLevelTable(int row, int column)
{
    if(boolTableDataInitialised){

        ResponseFunction* responseFunction = ResponseFunction::get();

    //    std::cout << "In slotUpdateFeedingData : " << std::endl;
        DecayPath* decayPath= DecayPath::get();
        std::vector<Nuclide>* nuclides_ = decayPath->GetAllNuclides();
        std::vector<Level>* motherLevels_ = nuclides_->at(0).GetNuclideLevels();
        std::vector<Transition*>* betaTransitions = motherLevels_->at(0).GetTransitions();

//--zmienic na Betatransitions
//        std::vector <Level*> levels = decayPath->GetAllLevels();

        QString qstr = uiM->tableLevels->item(row,column)->text();
 //       qDebug() << "przeczytano: " << qstr << "row: "<<row<<"column: "<<column;
        string str = qstr.toUtf8().constData();
        if(column==0){
            if (str == "true")displayStatus.at(row) = true;
            if (str == "false")displayStatus.at(row) = false;
            return;
         } else if (column==1){
            //        std::cout << " nie zmieniamy energii poziomu" << std::endl;
                    return;
        }  else if(column==2){
            if(qstr=="true")betaTransitions->at(row)->SetIntensityFitFlag(true);
            if(qstr=="false")betaTransitions->at(row)->SetIntensityFitFlag(false);
        }  else if(column==3) {
             double feeding = qstr.toDouble()/100;
    //         qDebug() << "przed ustawieniem: " << feeding <<"bylo " <<levels.at(row)->GetBetaFeedingFunction();

             betaTransitions->at(row)->ChangeIntensity(feeding);
             responseFunction->ChangeContainerDaughterLevelIntensity( betaTransitions->at(row)->GetPointerToFinalLevel(), feeding );
             responseFunction->RefreshFlags();

    //         std::cout << "ustawiono na : " << levels.at(row)->GetBetaFeedingFunction()<< std::endl;
            } else {
            std::cout << "inne columny" << std::endl;
            return;
        }
//        setTotalIntensityLabel();
    }
}

void ManualFitGraph::initializeTable(std::vector<RowData> rowData)
{
    if(rowData.size() <= 0)
        return;
    unsigned int nrOfColumn = rowData.at(0).GetNumberOfCells();
    unsigned int nrOfRow = rowData.size();
    uiM->tableLevels->setColumnCount(nrOfColumn);
   // tableOfData->setMinimumHeight(nrOfColumn*120);
    uiM->tableLevels->setMinimumWidth(nrOfColumn*117);
    uiM->tableLevels->setMinimumHeight(150);
    uiM->tableLevels->setRowCount(nrOfRow);
    for(unsigned int i=0; i != nrOfRow; ++i){
        this->initializeRow(i, rowData.at(i));
       if(rowData.at(i).GetCellData(0) == "true") displayStatus.push_back(true);
       if(rowData.at(i).GetCellData(0) == "false") displayStatus.push_back(false);
    };
    boolTableDataInitialised = "true";
}

void ManualFitGraph::initializeRow(int rowNumber, RowData rowData)
{
    for(unsigned int i = 0; i != rowData.GetNumberOfCells(); ++i)
    {
        QTableWidgetItem* tempQTableWidgetItem = new QTableWidgetItem(rowData.GetCellData(i));
        uiM->tableLevels->setItem(rowNumber, i, tempQTableWidgetItem);
        //delete tempQTableWidgetItem;
    }
}


void ManualFitGraph::slotShowPointToolTip(QMouseEvent *event)
    {
    double x = uiM->histogramPlot->xAxis->pixelToCoord(event->pos().x());
    double y = uiM->histogramPlot->yAxis->pixelToCoord(event->pos().y());
    setToolTip(QString("%1 , %2").arg(x).arg(y));

    }
void ManualFitGraph::slotSetLogScale(bool checked)
{
    if (checked) {
    uiM->histogramPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    uiM->lineEditYAxisMin->setText("1");
    } else {
        uiM->histogramPlot->yAxis->setScaleType(QCPAxis::stLinear);
        uiM->lineEditYAxisMin->setText("0");
    }
    uiM->histogramPlot->replot();
}


double ManualFitGraph::vectorMax(QVector<double> yM, double minEn, double maxEn)
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


void ManualFitGraph::slotSetAxisRange()
{

// this method responses to changes on GUI.

    double xMinEn =  uiM -> lineEditXAxisMin->text().toDouble();
    double xMaxEn =  uiM -> lineEditXAxisMax->text().toDouble();
//    double xMaxEn = x.size();
    double yMin =  uiM->lineEditYAxisMin->text().toDouble();
    double yMax =  uiM->lineEditYAxisMax->text().toDouble();
   if(xMaxEn > x.back())
    {
        xMaxEn = x.back();
        QMessageBox msgBox;
        QString info;
        QString qstrxMax = QString::number(xMaxEn);
        info = "X-Axis Max LARGER then spectrum size! Rescaling to: " + qstrxMax;
        uiM->lineEditXAxisMax->clear();
        uiM->lineEditXAxisMax->insert(QString::number(xMaxEn));
        msgBox.setText(info);
        msgBox.exec();

    }
    double size = xMaxEn-xMinEn;
    uiM->histogramPlot->xAxis->setRange(xMinEn,xMaxEn);
//    double yMax = vectorMax(y,xMinEn,xMaxEn);
//    yMax = yMax*1.1; //adding 10% to the scale
    uiM->histogramPlot->yAxis->setRange(yMin,yMax);
    if(respType_ != "c"){
        double dyMax = vectorMax(diff,xMinEn,xMaxEn);
        uiM->histogramDiffPlot->yAxis->setRange(-dyMax,dyMax);
        uiM->histogramDiffPlot->xAxis->setRange(xMinEn,xMaxEn);
        uiM->histogramDiffPlot->replot();
    }
    uiM->histogramPlot->replot();

}



void ManualFitGraph::slotShowSimSpec(bool checked)
{
    if(uiM->checkShowSIM->isChecked())
        uiM->histogramPlot->graph(1)->setVisible(true);
    if(!uiM->checkShowSIM->isChecked())
        uiM->histogramPlot->graph(1)->setVisible(false);

    uiM->histogramPlot->replot();

}

void ManualFitGraph::initializeGraphs()
{
//    QPen blueDotPen;
    blueDotPen.setColor(QColor(0, 0, 255, 150));
    blueDotPen.setStyle(Qt::DotLine);
    blueDotPen.setWidthF(1);
//    QPen redDotPen;
    redDotPen.setColor(QColor(255, 0, 0, 250));
    redDotPen.setStyle(Qt::DotLine);
    redDotPen.setWidthF(2);
//    QPen blackDotPen;
    blackDotPen.setColor(QColor(0, 0, 0, 250));
    blackDotPen.setStyle(Qt::DotLine);
    blackDotPen.setWidthF(2);
//    QPen blackPen;
    blackPen.setColor(QColor(0, 0, 0, 250));
    blackPen.setWidthF(1);


    double yMin= uiM->lineEditYAxisMin->text().toDouble();
    double yMaxUser = uiM->lineEditYAxisMax->text().toDouble();
    double xMinEn =  uiM -> lineEditXAxisMin->text().toDouble();
    double xMaxEn =  uiM -> lineEditXAxisMax->text().toDouble();

    double yMax=yMaxUser;

    uiM->histogramPlot->yAxis->setRange(yMin,yMaxUser);
    uiM->histogramPlot->xAxis->setRangeLower(xMinEn);
    uiM->histogramPlot->xAxis->setRangeUpper(xMaxEn);
    uiM->histogramDiffPlot->xAxis->setRangeLower(xMinEn);
    uiM->histogramDiffPlot->xAxis->setRangeUpper(xMaxEn);


    // getting Q value from NNDC input data and making a line
            QCPItemLine *QValueLine = new QCPItemLine(uiM->histogramPlot);
            DecayPath* decayPath= DecayPath::get();
            if(decayPath != 0L)
            {
            double QValue = decayPath->GetAllNuclides()->at(0).GetQBeta();
            xMaxEn= QValue* 1.2;
     // add the text label at the top:
            QCPItemText *textLabel = new QCPItemText(uiM->histogramPlot);
            textLabel->setPositionAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            textLabel->position->setCoords(QValue, yMax/4);
            QString qstr = "Q=" + QString::number(QValue) + " keV";
            textLabel->setText(qstr);
            textLabel->setFont(QFont(font().family(), 16)); // make font a bit larger
            textLabel->setPen(QPen(Qt::black)); // show black border around text
            textLabel->setRotation(-90);
            QValueLine->start->setParentAnchor(textLabel->left);
            QValueLine->end->setCoords(QValue, 1);
            QValueLine->setHead(QCPLineEnding::esSpikeArrow);
            QValueLine->setPen(QPen(Qt::red));
            delete textLabel;
            }

            delete QValueLine;

    cout<< "end of preparation phase" << endl;

}

void ManualFitGraph::showDataExpSimDiff(QVector<double> xx, QVector<double> yy, QVector<double> xx2, QVector<double> yy2, QVector<double> ddiff)
{
    cout <<"ManualFitGraph::showDataExpSimDiff - POCZATEK" << endl;
    x = xx;
    x2 = xx2;
    y = yy;
    y2 = yy2;
    diff =ddiff;

    double xMinEn =  uiM -> lineEditXAxisMin->text().toDouble();
    //double xMaxEn =  uiM -> lineEditXAxisMax->text().toDouble();
    double xMaxEn = x.at(x.size()-1);
    double yMin= uiM->lineEditYAxisMin->text().toDouble();
    double yMaxUser = uiM->lineEditYAxisMax->text().toDouble();
    if(xMinEn == 0.0)
        xMinEn = 1;
    double yMax = vectorMax(y,xMinEn,xMaxEn);
//    double yMax2 = vectorMax(y2,xMinEn,xMaxEn);
    yMax = std::min(yMax,yMaxUser);
    cout << "linia 510" << endl;
    //yMax = yMax*1.1; //adding 10% to the scale
    uiM->histogramPlot->yAxis->setRange(yMin,yMax);


    uiM->histogramPlot->addGraph();
    uiM->histogramPlot->graph(0)->setName("Experiment");
    uiM->histogramPlot->graph(0)->setData(x, y);
     uiM->histogramPlot->graph(0)->setPen(blackPen);
     uiM->histogramPlot->graph(0)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);


     uiM->histogramPlot->addGraph();
     uiM->histogramPlot->graph(1)->setName("Reconstruction");

//mkout     QVector<double> zero = y2;
//mkout     for(int ii=0; ii !=zero.size(); ii++)
//mkout         zero[ii]=0.0;
     if(uiM->checkShowSIM->isChecked())
     {
         uiM->histogramPlot->graph(1)->setData(x2, y2);
          uiM->histogramPlot->graph(1)->setVisible(true);
     }
     if(!uiM->checkShowSIM->isChecked())
         uiM->histogramPlot->graph(1)->setVisible(false);

     uiM->histogramPlot->graph(1)->setPen(redDotPen);
     uiM->histogramPlot->graph(1)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
     uiM->histogramPlot->graph()->selectable();
     uiM->histogramPlot->graph()->selectionDecorator()->setPen(QPen(Qt::green));


     uiM->histogramDiffPlot->addGraph();
     uiM->histogramDiffPlot->graph(0)->setName("(Exp - Rec)");
     uiM->histogramDiffPlot->graph(0)->setData(x, diff);
     uiM->histogramDiffPlot->graph(0)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
//     uiM->histogramDiffPlot->xAxis->setRange(xMinEn,xMaxEn);

     double dyMax = vectorMax(diff,xMinEn,xMaxEn);
     uiM->histogramDiffPlot->yAxis->setRange(-dyMax,dyMax);

//     uiM->histogramDiffPlot->yAxis->setRange(-yMax/20,yMax/20);


     // calculating SUM of displayed EXP,REC,DIFF spectra
     int startIndex = 100;
     QString ExpSpecSum_ = QString::number(accumulate(y.begin() + startIndex, y.end(), 0.0));
     QString SimSpecSum_ = QString::number(accumulate(y2.begin() + startIndex, y2.end(), 0.0))   ;
     QString DiffSpecSum_ = QString::number(accumulate(diff.begin(),diff.end(),0.0))   ;
     QString qstr = "Number of counts in: EXPspec "+ ExpSpecSum_ + ", SimSpec: " + SimSpecSum_ + ", DiffSpec: " + DiffSpecSum_;
     uiM->labelNoCounts->setText(qstr);
     uiM->histogramPlot->replot();
     uiM->histogramDiffPlot->replot();
     uiM->lineEditYAxisMax->setText(QString::number(yMax));

     std::cout << " ManualFitGraph::showDataExpSimDiff - KONIEC " << std::endl;

}

void ManualFitGraph::showResponseFunctions()
{
    if(respType_ == "l"){
        cout << "ManualFitGraph::showDataExpSimDiff - LEVELS: " << respType_ <<  endl;
    showResponseFunctionsLevels(2);
    } else if (respType_ == "g"){
        cout << "ManualFitGraph::showDataExpSimDiff - GAMMAS: " <<respType_ <<  endl;
    showResponseFunctionsGammas(2);
    } else if (respType_ == "c") {
      showSpectra(0);
     }   else {
        // do nothing
    }
    uiM->histogramPlot->replot();
}

void ManualFitGraph::slotNormaliseBetaFeeding(bool ok)
{
//   if(ok)
//   {
   cout << "ManualFitGraph::slotNormaliseBetaFeeding " << endl;
    DecayPath *decayPath_ = DecayPath::get();
    ResponseFunction* responseFunction = ResponseFunction::get();

    std::vector<Nuclide>* nuclides_ = decayPath_->GetAllNuclides();
    std::vector<Level>* motherLevels_ =nuclides_->at(0).GetNuclideLevels();
    //Level *motherLevel_ = &(motherLevels_->at(0));
    std::vector<Transition*>* betaTransitions = motherLevels_->at(0).GetTransitions();

    std::vector< std::pair<int,Contamination> >* contaminations = myProject->getContaminations();
    int expSpectrumID = std::stoi(myProject->getExpSpecID());
    float sumNormCont = 0.0;
    for (unsigned int i = 0; i !=  contaminations->size(); i++)
    {
        if(contaminations->at(i).first == expSpectrumID)
        sumNormCont += contaminations->at(i).second.intensity;
    }

    double IntensitySum = 0 ;
    for (std::vector<Transition*>::iterator it = betaTransitions->begin(); it !=  betaTransitions->end(); it++)
    {
       IntensitySum +=(*it)->GetIntensity();
    }
    cout << "IntensitySum " << IntensitySum << endl;
    int row =0;
    for (std::vector<Transition*>::iterator it = betaTransitions->begin(); it !=  betaTransitions->end(); it++)
    {
        double newIntensity = (*it)->GetIntensity()/IntensitySum;

       (*it)->ChangeIntensity(newIntensity);
        responseFunction->ChangeContainerDaughterLevelIntensity( (*it)->GetPointerToFinalLevel(), newIntensity );

        QString newIntensityString = QString::number(newIntensity*100);
        uiM->tableLevels->setItem(row, 3, new QTableWidgetItem(newIntensityString));
        row++;
    }
    responseFunction->RefreshFlags();
    uiM->tableLevels->show();
 //  }
}

void ManualFitGraph::slotNormaliseTransitionsFeeding(bool ok)
{
    ResponseFunction* responseFunction = ResponseFunction::get();

  Level* level_ = myProject->GetCurrent2DFitLevel();
  level_->NormalizeTransitionIntensities();

  responseFunction->UpdateWholeContainerIntensities();
  responseFunction->RefreshFlags();

  std::vector<bool> futureResults;
  std::vector<RowData> rowData_;
  std::vector<Transition*>* transitions_ = level_->GetTransitions();
  for(auto it = transitions_->begin(); it != transitions_->end(); ++it)
  {
      futureResults.push_back(true);
      displayStatus.push_back(true);
  }

  for(unsigned int i = 0; i != futureResults.size(); ++i)
  {
      QString QFittingFlag_ = transitions_->at(i)->GetIntensityFitFlag() ? "true" : "false";
      QString QDisplayStatus_ = displayStatus.at(i) ? "true" : "false";
      QString QEnergy_ = QString::number(transitions_->at(i)->GetFinalLevelEnergy());
      QString QIntensity_ = QString::number(transitions_->at(i)->GetIntensity()*100);
      rowData_.push_back(RowData(QDisplayStatus_, QEnergy_, QFittingFlag_, QIntensity_));
  }
  initializeTable(rowData_);
}

void ManualFitGraph::showSpectra(int start )
{

    float xMin = 0;
    float xMax = 10000; // expHist->GetNrOfBins();
      uiM->checkContrOtherLevels->hide();
      uiM->checkShowSIM->hide();

    DecayPath *decayPath_ = DecayPath::get();
    ResponseFunction* responseFunction_ = ResponseFunction::get();


    std::vector<Nuclide>* nuclides_ = decayPath_->GetAllNuclides();
    std::vector<Level>* motherLevels_ =nuclides_->at(0).GetNuclideLevels();
    Level *motherLevel_ = &(motherLevels_->at(0));


    //petal po liniach tabeli
   cout << "liczba graph " << uiM->histogramPlot->graphCount() << endl;
    for (unsigned int row = 0; row<uiM->tableLevels->rowCount(); row++)
    {
        uiM->histogramPlot->addGraph();
        uiM->histogramPlot->graph()->selectable();
        uiM->histogramPlot->graph()->selectionDecorator()->setPen(QPen(Qt::green));
        QString spectrumType = uiM->tableLevels->item(row,3)->text();
        Histogram tmpHist;
        int hisID = uiM->tableLevels->item(row,1)->text().toInt();
        if(spectrumType == "ExpMap"){
            tmpHist = Histogram(myProject->getHistFromExpMap(hisID));
            cout << "Ploting EXPMap spectrum:" << hisID << endl;
            uiM->histogramPlot->graph(row+start)->setPen(blackPen);
            uiM->histogramPlot->graph(row+start)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);

        } else if(spectrumType == "SimMap"){
            cout << "SIM not yet implemented";
            tmpHist = Histogram( *responseFunction_->GetResponseFunction(motherLevel_, hisID));
            double norm = myProject->getHistFromExpMap(hisID)->GetNrOfCounts();
            tmpHist.Normalize(norm);
            uiM->histogramPlot->graph(row+start)->setPen(redDotPen);
            uiM->histogramPlot->graph(row+start)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);

        } else if(spectrumType.contains(".his")){
            string fileName = spectrumType.toStdString();
 //           int hisID = uiM->tableLevels->item(row,1)->text().toInt();
            tmpHist = Histogram(fileName,hisID);
        } else continue;
 //       tmpHist = Histogram(responseFunction_->GetLevelRespFunction( (*it)->GetPointerToFinalLevel(), histID ));
        double xMax = tmpHist.GetXMax();
        double xMin = tmpHist.GetXMin();
 /*       cout << " number of counts in exp: " << myProject->getExpHist()->GetNrOfCounts() << endl;
        cout << " number of counts in resp fun: " << tmpHist->GetNrOfCounts() << endl;
        cout << " number of counts in dec: " << myProject->getDecHist()->GetNrOfCounts() << endl;

        double norm_ = (*it)->GetIntensity() * myProject->getExpHist()->GetNrOfCounts() *(1-sumNormCont);
        std::cout<< norm_ << " W spec PLOt" << std::endl;
        sumnorm +=norm_;
        cout << "sumn norm " << sumnorm <<  endl;
        tmpHist->Scale(norm_);
        cout << " number of counts in resp fun after NORM: " << tmpHist->GetNrOfCounts() << endl;
*/
        QVector<double> x2 = QVector<double>::fromStdVector(tmpHist.GetEnergyVectorD());
        QVector<double> y2 = QVector<double>::fromStdVector(tmpHist.GetAllDataD());
    // set style for the graph
           uiM->histogramPlot->graph(row+start)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
            uiM->histogramPlot->graph(row+start)->selectable();
            uiM->histogramPlot->graph(row+start)->selectionDecorator()->setPen(QPen(Qt::yellow));
    // set data for the graph
   if(displayStatus.at(row))
   {
       uiM->histogramPlot->graph(row+start)->setData(x2, y2);
       uiM->histogramPlot->graph(row+start)->setVisible(true);
   }
   if(!displayStatus.at(row))
       uiM->histogramPlot->graph(row+start)->setVisible(false);

    }

}

void ManualFitGraph::showResponseFunctionsLevels(int start)
{

    uiM->checkContrOtherLevels->hide();


    float xMin = 0;
    float xMax = 10000; // expHist->GetNrOfBins();
//evaout    HistogramGraphController *hisControler = new HistogramGraphController(xMin, xMax);
    DecayPath *decayPath_ = DecayPath::get();
    ResponseFunction* responseFunction_ = ResponseFunction::get();


    std::vector<Nuclide>* nuclides_ = decayPath_->GetAllNuclides();
    std::vector<Level>* motherLevels_ =nuclides_->at(0).GetNuclideLevels();
    Level *motherLevel_ = &(motherLevels_->at(0));
    std::vector<Transition*>* betaTransitions = motherLevels_->at(0).GetTransitions();

    std::vector< std::pair<int, Contamination> >* contaminations = myProject->getContaminations();
    int expSpectrumID = std::stoi(myProject->getExpSpecID());
    float sumNormCont = 0.0;
    for (unsigned int i = 0; i !=  contaminations->size(); i++)
    {
        if(contaminations->at(i).first == expSpectrumID)
        sumNormCont += contaminations->at(i).second.intensity;
    }
    cout <<"SumNormCount " << sumNormCont << endl;
    double IntenistySum = 0 ;
    for (std::vector<Transition*>::iterator it = betaTransitions->begin(); it !=  betaTransitions->end(); it++)
    {
       IntenistySum +=(*it)->GetIntensity();
    }
    cout << "IntensitySum " << IntenistySum << endl;
    double sumnorm = 0.0;
    double sumLevnorm = 0.0;

    int histID = std::stoi(myProject->getExpSpecID()); // 6300;
    int i=0;
    for (std::vector<Transition*>::iterator it = betaTransitions->begin(); it !=  betaTransitions->end(); it++)
    {
        Histogram* tmpHist = new Histogram(responseFunction_->GetLevelRespFunction( (*it)->GetPointerToFinalLevel(), histID ));
//        Histogram* tmpHist = new Histogram(levels.at(i)->GetResponseFunction(decay_,6300));
        double xMax = tmpHist->GetXMax();
        double xMin = tmpHist->GetXMin();
        cout << " number of counts in exp: " << myProject->getExpHist()->GetNrOfCounts() << endl;
        cout << " number of counts in resp fun: " << tmpHist->GetNrOfCounts() << endl;
        cout << " number of counts in dec: " << myProject->getDecHist()->GetNrOfCounts() << endl;

        double norm_ = (*it)->GetIntensity() * myProject->getExpHist()->GetNrOfCounts() *(1-sumNormCont);
        std::cout<< norm_ << " W spec PLOt" << std::endl;
        sumnorm +=norm_;
        cout << "sumn norm " << sumnorm <<  endl;
        tmpHist->Scale(norm_);
        cout << " number of counts in resp fun after NORM: " << tmpHist->GetNrOfCounts() << endl;

        QVector<double> x2 = QVector<double>::fromStdVector(tmpHist->GetEnergyVectorD());
        QVector<double> y2 = QVector<double>::fromStdVector(tmpHist->GetAllDataD());
        double dziwnasuma = 0.0;
/*mkout        for(int ii=0; ii<y2.size(); ii++)
        {
            dziwnasuma += y2.at(ii);
        }
        cout << " number of counts in resp fun after NORM: " << dziwnasuma << endl;
*/
 //mkout        QVector<double> zero = y2;
//mkout      for(int ii=0; ii !=zero.size(); ii++)zero[ii]=0.0;
        uiM->histogramPlot->addGraph();
    // set style for the graph
           uiM->histogramPlot->graph(i+start)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
            uiM->histogramPlot->graph(i+start)->selectable();
            uiM->histogramPlot->graph(i+start)->selectionDecorator()->setPen(QPen(Qt::yellow));
    // set data for the graph
   if(displayStatus.at(i))
   {
       uiM->histogramPlot->graph(i+start)->setData(x2, y2);
       uiM->histogramPlot->graph(i+start)->setVisible(true);
   }
   if(!displayStatus.at(i))
       uiM->histogramPlot->graph(i+start)->setVisible(false);
    i++;
    sumLevnorm += tmpHist->GetNrOfCounts();
    delete tmpHist;
    }
    cout << " total counts in REsp functinos " << sumLevnorm << endl;

}


void ManualFitGraph::slotShowOtherLevelsContribution(bool status)
{
    ShowOtherLevelsContribution(lastGraph_);
    uiM->histogramPlot->replot();
}

void ManualFitGraph::ShowOtherLevelsContribution(int last)
{
    cout << "ManualFitGraph::slotShowOtherLevelsContribution POCZATEK " << endl;

    Histogram otherLevelsContr =  *( myProject->getGate2DOtherLevelsContribution() );
    double scaleRatio = myProject->getGateNormFactor();
    otherLevelsContr.Scale( scaleRatio );

    QVector<double> x2 = QVector<double>::fromStdVector(otherLevelsContr.GetEnergyVectorD());
    QVector<double> y2 = QVector<double>::fromStdVector(otherLevelsContr.GetAllDataD());
//mkout    QVector<double> zero = y2;
//mkout  for(int ii=0; ii !=zero.size(); ii++)zero[ii]=0.0;
//  for(int ii=0; ii !=y2.size(); ii++)y2[ii] *=100.;

  //    uiM->histogramPlot->addGraph();
// set style for the graph
        uiM->histogramPlot->graph(last+1)->setName("Contribution from other levels");
        uiM->histogramPlot->graph(last+1)->setPen(QPen(Qt::green));
        uiM->histogramPlot->graph(last+1)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
        uiM->histogramPlot->graph(last+1)->selectable();
        uiM->histogramPlot->graph(last+1)->selectionDecorator()->setPen(QPen(Qt::yellow));
// set data for the graph
        cout << "isChecked otherlevels :" << uiM->checkContrOtherLevels->isChecked()
             << endl;
    if(uiM->checkContrOtherLevels->isChecked())
    {
        uiM->histogramPlot->graph(last+1)->setData(x2, y2);
        uiM->histogramPlot->graph(last+1)->setVisible(true);
    }
    if(!uiM->checkContrOtherLevels->isChecked())uiM->histogramPlot->graph(last+1)->setVisible(false);
cout << "ManualFitGraph::slotShowOtherLevelsContribution KONIEC " << endl;

}



void ManualFitGraph::showResponseFunctionsGammas(int start)
{
    QPalette p = uiM->checkContrOtherLevels->palette();
    p.setColor(QPalette::Active, QPalette::WindowText, Qt::green);
    uiM->checkContrOtherLevels->setPalette(p);
    uiM->checkContrOtherLevels->show();
    lastGraph_ = 0;
    //float xMin = 0;
    //float xMax = myProject->getExpGate()->GetNrOfBins();

    double scaleRatio = myProject->getGateNormFactor();
    Level* level_ = myProject->GetCurrent2DFitLevel();
    std::vector <Transition*>* transitions_ = level_->GetTransitions();
    //cout << " number of counts EXPGate_: " << myProject->getExpGate()->GetNrOfCounts() << endl;
    std::vector <Histogram>* gammaRespHist = myProject->getTransitionResponseHist();

    for (unsigned i = 0; i !=  transitions_->size(); i++)
    {
        Histogram tmpHist = gammaRespHist->at(i);

        //MS 2020.11.23 Important change for 2D fit, but not sure if correct for other manual fit purposes!!
        tmpHist.Scale( transitions_->at(i)->GetIntensity() * scaleRatio );

        //std::cout<< norm_ << " W spec PLOt " << "a po normalizacji " << tmpHist->GetNrOfCounts() << std::endl;
        QVector<double> x2 = QVector<double>::fromStdVector(tmpHist.GetEnergyVectorD());
        QVector<double> y2 = QVector<double>::fromStdVector(tmpHist.GetAllDataD());
//mkout        QVector<double> zero = y2;
//mkout        for(int ii=0; ii !=zero.size(); ii++)
//mkout            zero[ii]=0.0;

        uiM->histogramPlot->addGraph();

    // set style for the graph
//             string conName ="level " + std::to_string(i+1);
//             uiM->histogramPlot->graph(i+3)->setName(QString::fromStdString(conName));
//             uiM->histogramPlot->graph(i+3)->setPen(bluePen);
            uiM->histogramPlot->graph(i+start)->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
            uiM->histogramPlot->graph(i+start)->selectable();
            uiM->histogramPlot->graph(i+start)->selectionDecorator()->setPen(QPen(Qt::yellow));
    // set data for the graph
       if(displayStatus.at(i))
       {
           uiM->histogramPlot->graph(i+start)->setData(x2, y2);
           uiM->histogramPlot->graph(start+i)->setVisible(true);
       }
       if(!displayStatus.at(i))uiM->histogramPlot->graph(i+start)->setVisible(false);
       lastGraph_ =    i+start;
    }
    if(uiM->checkContrOtherLevels->isChecked())
    {
       uiM->histogramPlot->addGraph();
       slotShowOtherLevelsContribution(lastGraph_);
    }
}

void ManualFitGraph::setColumnStatus(bool status, int column)
{

    QTableWidget *pointerToTable_ = 0L;
    pointerToTable_ = uiM->tableLevels;

    QModelIndexList selection = pointerToTable_->selectionModel()->selectedRows();

    for(int i=0; i< selection.count(); i++)
    {
        QModelIndex index = selection.at(i);
//        pointerToTable_->setItem(index.row(),column, new QTableWidgetItem(status ? "true": "false"));
        slotStatusClicked(status, index.row(),column);
    }

}
