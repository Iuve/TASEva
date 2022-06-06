#ifndef TABLEINPUT_H
#define TABLEINPUT_H

#include "rowData.h"

#include <QObject>
#include <QWidget>
#include <QTableWidget>

namespace Ui {
class TableInput;
}

class TableInput : public QWidget
{
    Q_OBJECT

public:
    explicit TableInput(QWidget *parent = 0);
    ~TableInput();

    void setLabelMain(QString label);
    void setLabelDescription(QString label);
    void setLabelDown(QString label);
    void setLabelUp(QString label);
    void setTableUpHeader(QStringList header);
    void setTableDownHeader(QStringList header);
    void setNumberofColumnsUp(int n);
    void setNumberofColumnsDown(int n);
    void setTableDawnDataInitialised(bool a){TableDownDataInitialised_ = a;}
    void setTableUpDataInitialised(bool a){TableUpDataInitialised_ = a;}

    void initializeTable(QString tab, std::vector<RowData> rowData);
    void CellClicked(QTableWidget* table, int row, int column);
    void CellChanged(QTableWidget* table, int row, int column);

public slots:
    void slotUpCellClicked(int row, int column);
    void slotDownCellClicked(int row, int column);
    void slotUpCellChanged(int row, int column);
    void slotDownCellChanged(int row, int column);

signals:
    void recalibrate(QString spectype, int histID, std::vector<double> low, int glue, std::vector<double> high);
    void recalibrate(int histID);

private:
    Ui::TableInput *ui;
    bool TableDownDataInitialised_;
    bool TableUpDataInitialised_;
    bool error_ = false;

};

#endif // TABLEINPUT_H
