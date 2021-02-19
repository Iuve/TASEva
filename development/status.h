#ifndef STATUS_H
#define STATUS_H

#include <QWidget>
#include <QString>
#include <QTreeWidget>
#include <QBrush>
#include <QColor>



namespace Ui {
class Status;
}

class Status : public QWidget
{
    Q_OBJECT

public:
    explicit Status(QWidget *parent = 0);
    ~Status();
    void update();

signals:
    void signalUpdate();

private:
    Ui::Status *ui;

    void addResponseFunctionRoot();
    void addTreeChild(QTreeWidgetItem *parent,
                      QString name, QString description);


};

#endif // STATUS_H
