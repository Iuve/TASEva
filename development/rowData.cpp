#include "rowData.h"
//EVa #include "exception.h"

RowData::RowData(QString firstCell, QString secondCell, QString thirdCell)
{
    allCellsData.push_back(firstCell);
    allCellsData.push_back(secondCell);
    allCellsData.push_back(thirdCell);
}

RowData::RowData(QString firstCell, QString secondCell, QString thirdCell, QString fourthCell)
{
    allCellsData.push_back(firstCell);
    allCellsData.push_back(secondCell);
    allCellsData.push_back(thirdCell);
    allCellsData.push_back(fourthCell);
}

RowData::RowData(QString firstCell, QString secondCell, QString thirdCell, QString fourthCell, QString fifthCell)
{
    allCellsData.push_back(firstCell);
    allCellsData.push_back(secondCell);
    allCellsData.push_back(thirdCell);
    allCellsData.push_back(fourthCell);
    allCellsData.push_back(fifthCell);
}

RowData::RowData(std::vector<QString> &allCells)
{
    //allCellsData.resize(allCells.size());
    //copy(v1.begin(), v1.end(), v2.begin());
    allCellsData = allCells;
}

QString RowData::GetCellData(unsigned int cellNumber)
{
//EVa    if (cellNumber >= allCellsData.size())
//        throw OutOfRangeException("RowData::GetCellData, cellNumber outside of the available range");
    return allCellsData.at(cellNumber);
}
