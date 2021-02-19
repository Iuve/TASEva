#include "tablecontroller.h"

#include "QTableWidget"

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
    table->clear();
    table->setColumnCount(nrOfColumn);
    table->setMinimumWidth(nrOfColumn*117);
    table->setRowCount(nrOfRow);
    table->setMinimumHeight(150);

    for(unsigned int i=0; i != nrOfRow; ++i)
        this->initializeRow(table, i, rowData.at(i));
}

void TableController::initializeRow(QTableWidget *table, int rowNumber, RowData rowData)
{
    for(unsigned int i = 0; i != rowData.GetNumberOfCells(); ++i)
        table->setItem(rowNumber, i, new QTableWidgetItem(rowData.GetCellData(i)));
}


void TableController::updateTable(QTableWidget *table, std::vector<RowData> rowData)
{
    table->clear();
    initializeTable(table, rowData);
}
