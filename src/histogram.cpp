#include "histogram.h"
#include "ui_histogram.h"
#include "keyeventhandler.h"
#include "colormap.h"

// Create views, initialise data and ui
HistoGram::HistoGram(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HistoGram) {
    ui->setupUi(this);

    chartView = new QChartView(this);
    this->setCentralWidget(chartView);

    chart = new QChart();
    chart->legend()->hide();
    chart->setTitle("Spectogram");

    axisX = new QValueAxis();
    axisY = new QValueAxis();
    axisY2 = new QValueAxis();

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    chart->addAxis(axisY2, Qt::AlignRight);

    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    base_data = new QLineSeries();
    differentiated_data = new QLineSeries();
    base_data_2 = new QLineSeries();
    differentiated_data_2 = new QLineSeries();

    chart->addSeries(base_data);
    chart->addSeries(differentiated_data);
    chart->addSeries(base_data_2);
    chart->addSeries(differentiated_data_2);

    auto *handler = new KeyEventHandler(chartView);
    chartView->installEventFilter(handler);
    connect(handler, &KeyEventHandler::mouseDoubleClicked, this, &HistoGram::mouseDoubleClickedEvent);

}

void HistoGram::mouseDoubleClickedEvent(QMouseEvent *event) {

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to PNG"), "", tr("PNG image (*.png)"));
    qInfo("%s", fileName.toUtf8().constData());
    if (fileName != nullptr) {
        QImage image(chartView->sceneRect().size().toSize(), QImage::Format_ARGB32);  // Create the image with the exact size of the shrunk scene
        image.fill(Qt::transparent);                                              // Start all pixels transparent

        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        chartView->render(&painter);

        image.save(fileName);
    }

    if (event != nullptr)
        event->accept();

}

void HistoGram::setMax(float max) {
    maxValue = max;
}


void HistoGram::setMin(float min) {
    minValue = min;
}

// Update the data in histogram
void HistoGram::setData(vector<float> *dataPtr, short graphNum, int n, int x, int y, int width, int height, int num_channels, ChannelOrder channelOrder) {

    auto *data = new QLineSeries();
    auto *differentiated = new QLineSeries();

    double max_base = 0.0;
    double max_diff = 0.0;

    unsigned long index = 0;
    unsigned long index_2 = 0;

    for (int i = 0; i < num_channels; i++) {

        index = index_in_vector(channelOrder, n, x, y, i, width, height, num_channels);

        //        int index = x+(y*width)+(i*width*height);

        if (index >= dataPtr->size())
            break;

        auto base = static_cast<double>(dataPtr->at(index));
        data->append(i, base);

        if (base > max_base)
            max_base = base;

        if (i > 1) {
            index_2 = index_in_vector(channelOrder, n, x, y, (i - 1), width, height, num_channels);

            if (index_2 >= dataPtr->size())
                break;

            auto diff = static_cast<double>(dataPtr->at(index) / dataPtr->at(index_2));
            differentiated->append(i, diff);
            if (diff > max_diff)
                max_diff = diff;
        }
    }

    axisX->setRange(0, num_channels - 1);
    axisY->setRange(0 - static_cast<double>(maxValue), static_cast<double>(maxValue));
    axisY2->setRange(0, 2);


    if (graphNum == 0) {
        chart->removeSeries(base_data);
        chart->removeSeries(differentiated_data);

        base_data = data;
        differentiated_data = differentiated;

        chart->addSeries(base_data);
        chart->addSeries(differentiated_data);

    } else if (graphNum == 1) {
        chart->removeSeries(base_data_2);
        chart->removeSeries(differentiated_data_2);

        base_data_2 = data;
        differentiated_data_2 = differentiated;

        chart->addSeries(base_data_2);
        chart->addSeries(differentiated_data_2);

    }

    data->attachAxis(axisX);
    data->attachAxis(axisY);
    differentiated->attachAxis(axisX);
    differentiated->attachAxis(axisY2);

}

HistoGram::~HistoGram() {
    delete ui;
}
