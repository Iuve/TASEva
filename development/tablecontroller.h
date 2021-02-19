#ifndef TABLECONTROLLER_H
#define TABLECONTROLLER_H

#include "rowData.h"

#include <QObject>
#include <QWidget>
#include <QTableWidget>

class TableController
{
public:
    TableController();
    void setHeader(QTableWidget *table, QStringList header);
    void initializeTable(QTableWidget *table,std::vector<RowData> rowData);
    void initializeRow(QTableWidget *table, int rowNumber, RowData rowData);
    void updateTable(QTableWidget *table, std::vector<RowData> rowData);
};

#endif // TABLECONTROLLER_H
