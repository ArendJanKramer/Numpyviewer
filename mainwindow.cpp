#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <stdint.h>
#include "keyeventhandler.h"
#include <QtCharts>
#include "colormap.h"

using namespace QtCharts;
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    channelsfirst = false;
    use_colormap = false;
    loaded_path = "";

    ui->setupUi(this);

    ui->bandView->setMouseTracking(true);
    ui->actionChannels_first->setChecked(channelsfirst);
    ui->bandSelector->setHidden(true);

    ui->actionUse_colormap_instead_of_grayscale->setChecked(use_colormap);

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
            histoGram.setData(&loaded_data, graphNum, x, y, width, height, num_channels, channelsfirst);
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

        unsigned long index = 0;

        if (channelsfirst){
            index = (unsigned long)(y*height+x)+(ui->bandSelector->value() * width * height);
        }else{
            index = (unsigned long)(x*width+y)*num_channels+ui->bandSelector->value();
        }

        if (index <= loaded_data.size() && x <= width && y <= height && y>=0 && x >= 0){
            float value = (float)loaded_data.at(index);

            QString message;
            message.sprintf("(%d, %d) [%f]",x,y,value);
            ui->statusBar->showMessage(message);

            if(event->buttons() == Qt::LeftButton || event->buttons() == Qt::RightButton)
                mousePressedEvent(event);

        }
    }
}


void MainWindow::render_channel(long channel_index){

    long imageSize = loaded_data.size()/num_channels;
    QByteArray bitmap(imageSize, '\0');

    float slope = (255.0) / (max_pixel_in_file - min_pixel_in_file);

    // Build bitmap array
    unsigned long pixel_position = 0;
    int i = 0;
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            i = (int)(y*height+x);

            if (channelsfirst){
                pixel_position = (unsigned long)(y*height+x)+(channel_index * width * height);
            }else{
                pixel_position = (unsigned long)(x*width+y)*num_channels+channel_index;
            }

            if (pixel_position > loaded_data.size()){
                qInfo("Index ran out of bounds");
            }else{
                bitmap[i] = (uint8_t)((loaded_data[pixel_position]-min_pixel_in_file)*slope);
            }
        }
    }

    // Draw all bitmap pixels
    QImage img = QImage(width, height, QImage::Format_RGB888);
    QColor res;
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            int val = static_cast<uint8_t>(bitmap.at(x+(y*width)));

            if (use_colormap){
            // TODO clip val to 255
                res = QColor(cmap_red[val], cmap_green[val], cmap_blue[val]);
            }else{
                res = QColor(val, val, val);
            }
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

template <class T>
void MainWindow::load_and_convert_vector(cnpy::NpyArray *arr){
    T *data_ptr = arr->data<T>();
    vector<T> type_data = vector<T>(data_ptr, data_ptr + (width*height*num_channels));
    loaded_data = std::vector<float>(type_data.begin(), type_data.end());
}

void MainWindow::load_cube(string path){

    qInfo("Load cube");
    try{
        cnpy::NpyArray arr = cnpy::npy_load(path);
        int wordSize = (int)arr.word_size;
        char data_type = (char)arr.data_type;

        if (channelsfirst){
            qInfo("Expecting channels*height*width shape");
            height = (long)arr.shape[1];
            width = (long)arr.shape[2];
            num_channels = (long)arr.shape[0];
        }else{
            qInfo("Expecting height*width*channels shape");
            height = (long)arr.shape[0];
            width = (long)arr.shape[1];
            num_channels = (long)arr.shape[2];
        }

        /* Data_types in numpy files
        '?' 	boolean
        'b' 	(signed) byte
        'B' 	unsigned byte
        'i' 	(signed) integer
        'u' 	unsigned integer
        'f' 	floating-point
        'c' 	complex-floating point
        'm' 	timedelta
        'M' 	datetime
        'O' 	(Python) objects
        'S', 'a' zero-terminated bytes (not recommended)
        'U' 	Unicode string
        'V' 	raw data (void)
        */
        if (data_type == 'f'){
            qInfo("Creating array of datatype [float] and wordsize %d", wordSize);
            if (wordSize == 4)
                this->load_and_convert_vector<float>(&arr);
            else if (wordSize == 8)
                this->load_and_convert_vector<double>(&arr);
            else
                return;
        }else if (data_type == 'u' || data_type == 'B'){
            qInfo("Creating array of datatype [unsigned int] and wordsize %d", wordSize);
            if (wordSize == 1)
                this->load_and_convert_vector<uint8_t>(&arr);
            else if (wordSize == 2)
                this->load_and_convert_vector<uint16_t>(&arr);
            else if (wordSize == 4)
                this->load_and_convert_vector<uint32_t>(&arr);
            else if (wordSize == 8)
                this->load_and_convert_vector<uint64_t>(&arr);
            else
                return;
        }else{
            qInfo("Creating array of datatype [signed int] and wordsize %d", wordSize);
            if (wordSize == 1)
                this->load_and_convert_vector<int8_t>(&arr);
            else if (wordSize == 2)
                this->load_and_convert_vector<int16_t>(&arr);
            else if (wordSize == 4)
                this->load_and_convert_vector<int32_t>(&arr);
            else if (wordSize == 8)
                this->load_and_convert_vector<int64_t>(&arr);
            else
                return;
        }

        // Calculate for contrast stretch
        max_pixel_in_file = *max_element(loaded_data.begin(), loaded_data.end());
        min_pixel_in_file = *min_element(loaded_data.begin(), loaded_data.end());

        // Put stats in GUI
        qInfo("Max pixel value in file: %f Min pixel value in file : %f", max_pixel_in_file, min_pixel_in_file);
        QString message;
        message.sprintf("Bands : %lu Width : %lu Height : %lu Wordsize : %i", num_channels, width, height, wordSize);
        qInfo("%s",message.toStdString().c_str());
        ui->statusBar->showMessage(message);

        // Setup GUI constraints
        histoGram.setMax(max_pixel_in_file);
        histoGram.setMin(min_pixel_in_file);

        ui->bandSelector->setHidden(num_channels <= 1);
        ui->bandSelector->setMaximum(num_channels-1);
        ui->bandSelector->setEnabled(true);

        QFileInfo info(QString::fromUtf8(path.c_str()));
        this->setWindowTitle(info.baseName());
        render_channel(ui->bandSelector->value());
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
    ui->actionSelected_band->setText(QString("Channel : %1").arg(ui->bandSelector->value()));
}

void MainWindow::on_bandSelector_valueChanged(int value)
{
    render_channel(value);
    updateTextInToolbar();
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Numpy Files (*.npy)"));
    if (fileName != NULL){
        loaded_path = fileName.toStdString();
        load_cube(loaded_path);
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

void MainWindow::on_actionChannels_first_triggered()
{
    qInfo("Channel order triggered");
    channelsfirst = ui->actionChannels_first->isChecked();
    if (loaded_path.length() > 3){
        load_cube(loaded_path);
    }
}

void MainWindow::on_actionUse_colormap_instead_of_grayscale_triggered()
{
    qInfo("Colormap triggered");
    use_colormap = ui->actionUse_colormap_instead_of_grayscale->isChecked();
    if (loaded_path.length() > 3){
       render_channel(ui->bandSelector->value());
    }
}
