#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QMainWindow>
#include <QtCharts>

using namespace QtCharts;
using namespace std;

namespace Ui {
class HistoGram;
}

class HistoGram : public QMainWindow
{
    Q_OBJECT

public:
    explicit HistoGram(QWidget *parent = 0);
    ~HistoGram();
    void setMax(float max);
    void setData(vector<float> *dataPtr, short graphNum, int x, int y, int width, int height, int numbands);
    void mouseDoubleClickedEvent(QMouseEvent *event);

private:
    Ui::HistoGram *ui;
    QChartView *chartView;
    QChart *chart;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QValueAxis *axisY2;

    QLineSeries *base_data;
    QLineSeries *differentiated_data;

    QLineSeries *base_data_2;
    QLineSeries *differentiated_data_2;

    float maxValue;

};

#endif // HISTOGRAM_H
