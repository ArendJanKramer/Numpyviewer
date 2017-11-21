#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <stdint.h>
#include "keyeventhandler.h"
#include <QtCharts>

using namespace QtCharts;
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    ui->bandView->setMouseTracking(true);

    KeyEventHandler *handler = new KeyEventHandler(ui->bandView);
    ui->bandView->installEventFilter(handler);
    connect(handler, &KeyEventHandler::mousePositionChanged, this, &MainWindow::mouseMovedEvent);
    connect(handler, &KeyEventHandler::mousePressed, this, &MainWindow::mousePressedEvent);
}

MainWindow::~MainWindow()
{
    histoGram.close();
    delete ui;
}

void MainWindow::mousePressedEvent(QMouseEvent *event){
    if (!image.isNull() && ui->actionHistogram->isChecked()){
        QPoint  local_pt = event->pos();//ui->bandView->mapFromGlobal(event->globalPos());
        QPointF img_coord_pt = ui->bandView->mapToScene(local_pt);

        short graphNum = 0;
        if(event->buttons() == Qt::RightButton)
            graphNum = 1;

        int x = (int)img_coord_pt.x();
        int y = (int)img_coord_pt.y();

        if (x <= width && y <= height && y>=0 && x >= 0){
            histoGram.setData(&loaded_data, graphNum, x, y, width, height, bands);
            histoGram.show();
            histoGram.activateWindow();
        }

    }
}

void MainWindow::mouseMovedEvent(QMouseEvent *event)
{
    if (!image.isNull()){

        QPoint  local_pt = event->pos();//ui->bandView->mapFromGlobal(event->globalPos());
        QPointF img_coord_pt = ui->bandView->mapToScene(local_pt);

        int x = (int)img_coord_pt.x();
        int y = (int)img_coord_pt.y();
        int index = (x+(y*image.width()))*ui->bandSelector->value();

        if (index <= (int)loaded_data.size() && index >= 0 && x <= width && y <= height && y>=0 && x >= 0){
            float value = (float)loaded_data.at(index);

            QString message;
            message.sprintf("(%d, %d) [%f]",x,y,value);
            ui->statusBar->showMessage(message);

            if(event->buttons() == Qt::LeftButton || event->buttons() == Qt::RightButton)
                mousePressedEvent(event);

        }
    }
}


void MainWindow::renderView(long bandnum){

    long offset = bandnum * width * height;

    long imageSize = loaded_data.size()/bands;
    QByteArray data(imageSize, '\0');

    float gain = 255.0/maxPixel;

    for (int i = 0; i < (int)imageSize; i++) {
        if (bandnum == 0 && bands >= 225){
            data[i] = (uint8_t)(loaded_data[i+offset]*255);
        }else{
            data[i] = (uint8_t)loaded_data[i+offset]*gain;
        }
    }

    QImage img = QImage(width, height, QImage::Format_Grayscale8);

    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            int val = static_cast<uint8_t>(data.at(x+(y*width)));
            QColor res(QColor(val, val, val));
            if (!res.isValid())
                res = QColor(255, 0, 0);

            img.setPixelColor(x, y, res);
        }
    }

    image = img.copy();

    QPixmap item(QPixmap::fromImage(img));
    QGraphicsScene* scene = new QGraphicsScene;
    scene->addPixmap(item);
    ui->bandView->setScene(scene);
    resizeEvent(NULL);

}

void MainWindow::loadCube(string path){

    qInfo("Load cube");

    try{
        cnpy::NpyArray arr = cnpy::npy_load(path);
        int wordSize = (int)arr.word_size;

        float *float_ptr = arr.data<float>();

        height = (long)arr.shape[1];
        width = (long)arr.shape[2];
        bands = (long)arr.shape[0];

        loaded_data = vector<float>(float_ptr, float_ptr + (width*height*bands));
        maxPixel = *max_element(loaded_data.begin(), loaded_data.end());

        qInfo("max : %f", maxPixel);

        QString message;
        message.sprintf("Bands : %lu Width : %lu Height : %lu Wordsize : %i", bands, width, height, wordSize);
        ui->statusBar->showMessage(message);

        histoGram.setMax(maxPixel);
        ui->bandSelector->setMaximum(bands-1);
        ui->bandSelector->setEnabled(true);

        renderView(ui->bandSelector->value());
        qInfo("Done!");
        updateTextInToolbar();
    } catch(std::exception e){
        QMessageBox msgBox;
        msgBox.setText(e.what());
        msgBox.exec();

        close();
    }

}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QGraphicsScene *scene = ui->bandView->scene();
    if (scene != NULL){
        QRectF bounds = ui->bandView->scene()->itemsBoundingRect();
        ui->bandView->fitInView(bounds, Qt::KeepAspectRatio);
        ui->bandView->centerOn(0, 0);
    }
}

void MainWindow::updateTextInToolbar(){
    ui->actionSelected_band->setText(QString("Selected band : %1").arg(ui->bandSelector->value()));
}

void MainWindow::on_bandSelector_valueChanged(int value)
{
    renderView(value);
    updateTextInToolbar();
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Numpy Files (*.npy)"));
    if (fileName != NULL){
        loadCube(fileName.toStdString());
    }
}

void MainWindow::on_actionExport_as_PNG_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to PNG"), "", tr("PNG image (*.png)"));
    qInfo("%s", fileName.toUtf8().constData());
    if (fileName != NULL){
        image.save(fileName);
    }
}

void MainWindow::closeEvent( QCloseEvent *event )
{
    histoGram.close();
}

void MainWindow::on_actionHistogram_triggered(bool checked)
{
    if (!checked)
        histoGram.close();
}

void MainWindow::on_actionconvert_triggered()
{
    convertWindow.show();
}
