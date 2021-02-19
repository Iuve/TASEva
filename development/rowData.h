#ifndef ROWDATA_H
#define ROWDATA_H
#include<vector>
#include <QtGui>

class RowData
{
public:
    RowData(QString firstCell, QString secondCell, QString thirdCell);
    RowData(QString firstCell, QString secondCell, QString thirdCell, QString fourthCell);
    RowData(QString firstCell, QString secondCell, QString thirdCell, QString fourthCell, QString fifthCell);
    RowData(std::vector<QString> &allCells);

    unsigned int GetNumberOfCells() {
        return allCellsData.size();
    }
    QString GetCellData(unsigned int cellNumber);
private:
    std::vector<QString> allCellsData;
};

#endif // ROWDATA_H
