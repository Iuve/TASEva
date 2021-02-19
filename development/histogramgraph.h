#ifndef HISTOGRAMGRAPH_H
#define HISTOGRAMGRAPH_H

#include <QWidget>

namespace Ui {
class HistogramGraph;
}

class HistogramGraph : public QWidget
{
    Q_OBJECT

public:
    explicit HistogramGraph(QWidget *parent = 0);
    ~HistogramGraph();
    void showDataExpSimDiff(QVector<double> x, QVector<double> y, QVector<double> y2, QVector<double> diff);
    double vectorMax(QVector<double> yM, double minEn, double maxEn);
public slots:
    void slotShowPointToolTip(QMouseEvent *event);
    void slotSetLogScale(bool checked);
    void slotSetAxisRange();
private:
    Ui::HistogramGraph *uiH;
    QVector<double> x, y, y2, diff;

};

#endif // HISTOGRAMGRAPH_H
