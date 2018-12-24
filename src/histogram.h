#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QMainWindow>
#include <QtCharts>
#include "colormap.h"

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
    void setMin(float min);
    void setData(vector<float> *dataPtr, short graphNum, int x, int y, int width, int height, int num_channels, ChannelOrder channelOrder);
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
    float minValue;

};

#endif // HISTOGRAM_H
