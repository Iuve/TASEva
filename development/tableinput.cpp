#include "tableinput.h"
#include "ui_tableinput.h"

#include <QMessageBox>
TableInput::TableInput(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TableInput)
{
    ui->setupUi(this);
    TableUpDataInitialised_ = false;
    TableDownDataInitialised_ = false;

connect(ui->buttonClose  , SIGNAL(clicked(bool)), this, SLOT(close()));
connect(ui->tableUp, SIGNAL(cellClicked(int,int)), this, SLOT(slotUpCellClicked(int,int)));
connect(ui->tableDown, SIGNAL(cellClicked(int,int)), this, SLOT(slotDownCellClicked(int,int)));
connect(ui->tableUp, SIGNAL(cellChanged(int,int)), this, SLOT(slotUpCellChanged(int,int)));
connect(ui->tableDown, SIGNAL(cellChanged(int,int)), this, SLOT(slotDownCellChanged(int,int)));
}

TableInput::~TableInput()
{
delete ui;
}

void TableInput::setLabelMain(QString label){ui->labelMain->setText(label); }
void TableInput::setLabelDescription(QString label){ui->labelDescription->setText(label);}
void TableInput::setLabelDown(QString label){ui->labelDown->setText(label);}
void TableInput::setLabelUp(QString label){ui->labelUp->setText(label);}
void TableInput::setTableUpHeader(QStringList header){ui->tableUp->setHorizontalHeaderLabels(header);}
void TableInput::setTableDownHeader(QStringList header){ui->tableDown->setHorizontalHeaderLabels(header);}
void TableInput::setNumberofColumnsUp(int n){ui->tableUp->setColumnCount(n);}
void TableInput::setNumberofColumnsDown(int n){ui->tableDown->setColumnCount(n);}

void TableInput::initializeTable(QString tab, std::vector<RowData> rowData)
{
    if(rowData.size() <= 0)  return;
    QTableWidget *pointerToTable_ = 0l;
    if (tab == "Up")pointerToTable_ = ui->tableUp;
    if (tab == "Down")pointerToTable_ = ui->tableDown;
    unsigned int nrOfColumn = rowData.at(0).GetNumberOfCells();
    unsigned int nrOfRow = rowData.size();
    pointerToTable_->setColumnCount(nrOfColumn);
// tableOfData->setMinimumHeight(nrOfColumn*120);
    pointerToTable_->setMinimumWidth(nrOfColumn*117);
    pointerToTable_->setMinimumHeight(150);
    pointerToTable_->setRowCount(nrOfRow);
    for(unsigned int i=0; i != nrOfRow; ++i)
    {
        for(unsigned int k = 0; k != nrOfColumn; ++k)
        {
        QTableWidgetItem* tempQTableWidgetItem = new QTableWidgetItem(rowData.at(i).GetCellData(k));
        if(k == 0){tempQTableWidgetItem->setFlags(tempQTableWidgetItem->flags() & (~Qt::ItemIsEditable));}
        if(k == 1){tempQTableWidgetItem->setFlags(tempQTableWidgetItem->flags() & (~Qt::ItemIsEditable));}
//temporary set until low energy recalibration will be implemented
        if(k == 2){tempQTableWidgetItem->setFlags(tempQTableWidgetItem->flags() & (~Qt::ItemIsEditable));}
        if(k == 3){tempQTableWidgetItem->setFlags(tempQTableWidgetItem->flags() & (~Qt::ItemIsEditable));}
        pointerToTable_->setItem(i, k, tempQTableWidgetItem);
        }
    };
}

void TableInput::slotUpCellClicked(int row ,int column)
{
    QTableWidget * pointerToTable_ = 0l;
    pointerToTable_ = ui->tableUp;
    CellClicked( pointerToTable_, row, column);
}
void TableInput::slotDownCellClicked(int row ,int column)
{
    QTableWidget * pointerToTable_ = 0l;
    pointerToTable_ = ui->tableDown;
    CellClicked(pointerToTable_, row, column);
}

void TableInput::CellClicked(QTableWidget* table, int row, int column)
{
    bool status_;
    QString  specType_;
    if(table == ui->tableUp)specType_="EXP";
    if(table == ui->tableDown)specType_="SIM";
    QString qCellValue = table->item(row,column)->text();

     if (column == 0)
    {
     if(qCellValue == "YES") status_ = false;
     if(qCellValue == "NO") {return;  }
     QTableWidgetItem* tempQTableWidgetItem = new QTableWidgetItem(status_ ? "YES" : "NO");
     tempQTableWidgetItem->setFlags(tempQTableWidgetItem->flags() & (~Qt::ItemIsEditable));
     table->setItem(row, 0, tempQTableWidgetItem);
     table->show();
     int ID = table->item(row,1)->text().toInt();
     int glue = table->item(row,3)->text().toInt();
     std::vector<double> low;
     QString qStr = table->item(row,2)->text();
     QStringList qStrListH = qStr.split(";");
     for(int i=0; i!= qStrListH.size(); i++){
          double value = qStrListH.at(i).toDouble();
       low.push_back(value);
     }
     std::vector<double> high;
     qStr = table->item(row,4)->text();
     QStringList qStrListL = qStr.split(";");
     for(int i=0; i!= qStrListL.size(); i++){
         double value = qStrListL.at(i).toDouble();
       high.push_back(value);
     }
    emit recalibrate(specType_, ID, low, glue, high);
//    emit recalibrate(ID);
     }
}


void TableInput::slotUpCellChanged(int row ,int column)
{
    QTableWidget * pointerToTable_ = 0l;
    pointerToTable_ = ui->tableUp;
    CellChanged( pointerToTable_, row, column);
}
void TableInput::slotDownCellChanged(int row ,int column)
{
    QTableWidget * pointerToTable_ = 0l;
    pointerToTable_ = ui->tableDown;
    CellChanged(pointerToTable_, row, column);
}
void TableInput::CellChanged(QTableWidget* table, int row, int column)
{

        bool ok;
    if(TableUpDataInitialised_ & TableDownDataInitialised_)
    {
       if(column == 0){
           return;
       } else if (column == 1){  // Hist ID
            qDebug() << "We do not change HistID" ;
            return;
       } else if (column ==2){   //low energy calibration
           qDebug() << "We do not change low energy calibration at this moement" ;
           return;
       } else if (column == 3){  //Glue point
           qDebug() << "We do not change glue point in this moment, as we use ONLY one calibration" ;
           return;
       } else if (column == 4){  // high energy calibration

           QString qStr = table->item(row,column)->text();
           QStringList qStrList = qStr.split(";");
//           qDebug()<< qStrList;

           bool ok0,ok1,ok2;
           ok = true;
           double a0 = qStrList.at(0).toDouble(&ok0);
           double a1 = qStrList.at(1).toDouble(&ok1);
           double a2 = qStrList.at(2).toDouble(&ok2);
           bool ok = ok0 && ok1 && ok2;

           if ((qStrList.size() != 3) || (ok==false)){
               QMessageBox msgBox;
               msgBox.setText("Wrong coefficients. Setting values to 0.0;1.0,0.0");
               msgBox.exec();
               QTableWidgetItem* tempQTableWidgetItem = new QTableWidgetItem("0.0;1.0;0.0");
               table->setItem(row, column, tempQTableWidgetItem);
               table->show();
               error_ = true;
               return;
           }
            if(error_== false){
           QTableWidgetItem* tempQTableWidgetItem = new QTableWidgetItem("YES");
           tempQTableWidgetItem->setFlags(tempQTableWidgetItem->flags() & (~Qt::ItemIsEditable));
           table->setItem(row, 0, tempQTableWidgetItem);
           table->show();
           }
       } else {
           return;
       }
    }
}

