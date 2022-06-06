#include "calibrateenergy.h"
#include "rowData.h"
//#include "project.h"
#include "simplefigure.h"
#include "responsefunction.h"

#include "QString"

CalibrateEnergy::CalibrateEnergy(QWidget *parent):
    QWidget(parent)
{



    calE_ui = new TableInput();
    calE_ui->setLabelMain("Calibration coefficients tables");
    calE_ui->setLabelDescription("Low energy polynomial calibration meets high energy polinomial calibration at the glue point");

    calE_ui->setNumberofColumnsUp(5);
    calE_ui->setNumberofColumnsDown(5);
    QStringList header;
    header << "Recalib" << "ID" << "Low energy coeff." << "GluePoint" << "High energy coeff" ;
    calE_ui->setTableUpHeader(header);
    calE_ui->setTableDownHeader(header);
    calE_ui->setLabelUp("Experimental");
    calE_ui->setLabelDown("Simulated");
//    calE_ui->setTableUpDataInitialised(false);
//    calE_ui->setTableDawnDataInitialised(false);

     InitTableUp();
     InitTableDown();

     connect(calE_ui,SIGNAL(recalibrate(int)),this,SLOT(slotRecalibrate(int)));
     connect(calE_ui,SIGNAL(recalibrate(QString, int , std::vector<double>, int, std::vector<double> )),
             this,SLOT(slotRecalibrate(QString, int , std::vector<double>, int, std::vector<double>)));

     calE_ui->show();

}

CalibrateEnergy::~CalibrateEnergy()
{
    delete calE_ui;
}

void CalibrateEnergy::InitTableUp()
{
    Project* myProject = Project::get();
    std::vector<string> expID = myProject->getExpSpecIDVec();

    cout << "expID. size= " << expID.size() << std::endl;
    //-------- Tables ----------//
    std::vector<RowData> rowData_;
    for(unsigned int i = 0; i != expID.size(); i++)
    {
        QString QReCal = "NO";
        QString QID_ = QString::fromStdString(expID.at(i));
        QString QCalLow = "0.0;1.0;0.0";
        QString QGluePoint = "0";
        QString QCalHigh = "0.1;1.1;0.1";
        rowData_.push_back(RowData(QReCal,QID_,QCalLow,QGluePoint,QCalHigh));
    }

    calE_ui->TableInput::initializeTable("Up", rowData_);
    calE_ui->TableInput::setTableUpDataInitialised(true);
}

void CalibrateEnergy::InitTableDown()
{
//    Project* myProject = Project::get();
//    std::vector<string> expID = myProject->getExpSpecIDVec();
    std::vector<string> simID = {"6300", "6310", "6320", "6330"};

    //-------- Tables ----------//
    std::vector<RowData> rowData_;
    for(unsigned int i = 0; i != simID.size(); i++)
    {
        QString QReCal = "NO";
        QString QID_ = QString::fromStdString(simID.at(i));
        QString QCalLow = "0.0;1.0;0.0";
        QString QGluePoint = "0";
        QString QCalHigh = "0.1;1.1;0.1";
        rowData_.push_back(RowData(QReCal,QID_,QCalLow,QGluePoint,QCalHigh));
    }

    calE_ui->TableInput::initializeTable("Down", rowData_);
    calE_ui->TableInput::setTableDawnDataInitialised(true);

}

void CalibrateEnergy::slotRecalibrate(int ID)
{
    qDebug() << "Got signal ID only";
    qDebug() << ID ;
}

void CalibrateEnergy::slotRecalibrate(QString spectype, int histID, std::vector<double> low, int glue, std::vector<double> high)
{
    qDebug() << "Got signal";
    qDebug() << "spectype" << spectype;
    qDebug() << "ID" << histID;
    qDebug() << "low" << low;
    qDebug() << "glue" << glue;
    qDebug() << "high" << high;
    currentHistID_ = histID;
    currentSpecType_ = spectype;
    DecayPath *decayPath_ = DecayPath::get();
    ResponseFunction* responseFunction_ = ResponseFunction::get();


    std::vector<Nuclide>* nuclides_ = decayPath_->GetAllNuclides();
    std::vector<Level>* motherLevels_ =nuclides_->at(0).GetNuclideLevels();
    Level *motherLevel_ = &(motherLevels_->at(0));


    vector<Histogram> vecHistogram;
    if(spectype == "EXP"){  //for experimental spectra recalibration
        qDebug() << " przed read histogramow <<";

        histogramIN_ = Histogram(myProject->getHistFromExpMap(currentHistID_));
        histogramOUT_ = Histogram(myProject->getHistFromExpMap(currentHistID_));
        histogramOUT_.Recalibrate(high);
        double xMin =100;
        double xMax = 5000;
        double norm = histogramIN_.GetNrOfCounts(xMin,xMax);

        histogramREF_ = Histogram( *responseFunction_->GetResponseFunction(motherLevel_, currentHistID_));
        histogramREF_.NormalizeV(norm, xMin, xMax);

        vecHistogram.push_back(histogramIN_);
        vecHistogram.push_back(histogramREF_);
        vecHistogram.push_back(histogramOUT_);

        SimpleFigure *figure = new SimpleFigure();
        figure->initializeGraphs();
        figure->showData(vecHistogram);
        figure->show();

     connect(figure, SIGNAL(accept()), this, SLOT(slotAccept()));
     connect(figure, SIGNAL(cancel()), this, SLOT(slotCancel()));

    } else if (spectype == "SIM"){
        return;
    } else {
        return;
    }
}

void CalibrateEnergy::slotAccept()
{
    qDebug() << "got slot accept";
    if (currentSpecType_ == "EXP"){
    myProject->replaceExpHistInMap(currentHistID_, histogramOUT_);
       if(stoi(myProject->getExpSpecID()) == currentHistID_){
         myProject->setExpHist();
       }
    } else if (currentSpecType_ == "SIM"){

    } else {
        return;
    }
}

void CalibrateEnergy::slotCancel()
{

}
