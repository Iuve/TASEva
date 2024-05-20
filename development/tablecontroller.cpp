#include "tablecontroller.h"

#include "QTableWidget"
#include <iostream>
using namespace std;

TableController::TableController()
{

}

void TableController::setHeader(QTableWidget *table, QStringList header)
{
    table->setHorizontalHeaderLabels(header);
}

void TableController::initializeTable(QTableWidget *table,std::vector<RowData> rowData)
{
    if(rowData.size() <= 0)
        return;
    unsigned int nrOfColumn = rowData.at(0).GetNumberOfCells();
    unsigned int nrOfRow = rowData.size();
   qInfo() << "TableController::initializeTable - START"; // << std::endl;
    table->clear();
    qDebug() << "nrOfColumn: " << nrOfColumn ;// << std::endl;
    qDebug() << "nrOfColumn: " << rowData.back().GetNumberOfCells();// << std::endl;
    qDebug() << "nrOfRow: " << nrOfRow  ;//std::end;
    table->setColumnCount(nrOfColumn);
    table->setMinimumWidth(nrOfColumn*117);
    table->setRowCount(nrOfRow);
    table->setMinimumHeight(150);

//    for(unsigned int i= nrOfRow; i-- >0;) {
        for(unsigned int i=0; i!=nrOfRow; ++i) {
        qDebug() << "i= " << i;
        qDebug() << "i= " << i <<  rowData.at(i).GetCellData(0) << " "
                 << rowData.at(i).GetCellData(1) <<  " "
                 << rowData.at(i).GetCellData(2) <<  " "
                 << rowData.at(i).GetCellData(3) <<  " "
                    ;
        if (nrOfColumn == 5) qDebug() << rowData.at(i).GetCellData(4) ;
        this->initializeRow(table, i, rowData.at(i));
        qDebug() << "Initiated";
    }
    qDebug() << "TableController::initializeTable - END"; // << std::endl;

}

void TableController::initializeRow(QTableWidget *table, int rowNumber, RowData rowData)
{
    for(unsigned int i = 0; i != rowData.GetNumberOfCells(); ++i){
        QTableWidgetItem*   tmp = new QTableWidgetItem(rowData.GetCellData(i));
        if(rowData.GetCellData(i) == "n/a")
        {  // blocking the row for changes
            tmp->setFlags(Qt::ItemFlag(!Qt::ItemIsEnabled));  //No-Enabled
            tmp->setFlags(Qt::ItemFlag(!Qt::ItemIsEditable));  //No-Editable ItemIsEditable
            tmp->setFlags(Qt::ItemFlag(!Qt::ItemIsSelectable));  //No-Selectable
            qDebug() <<"Item("<<rowNumber<<","<<i<<") set false";
        }
        table->setItem(rowNumber, i, tmp);
//        table->setItem(rowNumber, i, new QTableWidgetItem(rowData.GetCellData(i)));
    }
}

void TableController::updateTable(QTableWidget *table, std::vector<RowData> rowData)
{
    table->clear();
    initializeTable(table, rowData);
}
