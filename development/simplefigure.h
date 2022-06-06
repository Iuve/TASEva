#ifndef SIMPLEFIGURE_H
#define SIMPLEFIGURE_H
#include "histogram.h"

#include <QWidget>

namespace Ui {
class SimpleFigure;
}

class SimpleFigure : public QWidget
{
    Q_OBJECT

public:
    explicit SimpleFigure(QWidget *parent = nullptr);
    ~SimpleFigure();

    void setxMin(QString qstr);
    void setxMax(QString qstr);
    void setyMin(QString qstr);
    void setyMax(QString qstr);
    void initializeGraphs();
    void showData(QVector<double>xx,QVector<double>yy,QVector<double>xx2,QVector<double>yy2);
    void showData(Histogram *y1, Histogram *y2);
    void showData(vector<Histogram> vecHistogram);
    void showData(vector<QVector<double>> vecQVector);

    Ui::SimpleFigure *ui;

public slots:
    void slotShowPointToolTip(QMouseEvent *event);
    void slotSetLogScale(bool checked);
    void slotSetAxisRange();
    void slotAccept();
    void slotCancel();

signals:
    void accept();
    void cancel();

private:

   double vectorMax(QVector<double> yM, double minEn, double maxEn);

   QVector<double> x, y, y2, x2;

//   QPen blueDotPen,redDotPen,blackDotPen,blackPen;
};

#endif // SIMPLEFIGURE_H
