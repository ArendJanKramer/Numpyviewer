#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <stdint.h>
#include "keyeventhandler.h"
#include <QtCharts>
#include "graphics_view_zoom.h"

using namespace QtCharts;
using namespace std;

// Initizializer
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    colorMode = ColorMode::Grayscale;
    channelOrder = ChannelOrder::H_W_C;
    loaded_path = "";

    ui->setupUi(this);

    Graphics_view_zoom *z = new Graphics_view_zoom(ui->imageCanvas);
    z->set_modifiers(Qt::NoModifier);

    ui->imageCanvas->setMouseTracking(true);

    updateSettingsMenu();
    ui->channelSlider->setHidden(true);

    dimensionLabel = new QLabel(this);
    dimensionLabel->setText("");
    ui->statusBar->addPermanentWidget(dimensionLabel, 0);

    KeyEventHandler *handler = new KeyEventHandler(ui->imageCanvas);
    ui->imageCanvas->installEventFilter(handler);
    connect(handler, &KeyEventHandler::mousePositionChanged, this, &MainWindow::mouseMovedEvent);
    connect(handler, &KeyEventHandler::mousePressed, this, &MainWindow::mousePressedEvent);

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left), this, SLOT(loadPrevious()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right), this, SLOT(loadNext()));

    QUrl imageUrl("https://raw.githubusercontent.com/ArendJanKramer/Qt-ENVI-Numpy-Viewer/master/version.txt");
    update_checker = new FileDownloader(imageUrl, this);
    connect(update_checker, SIGNAL (downloaded()), this, SLOT (version_downloaded()));
}

void MainWindow::updateSettingsMenu(){
    ui->order_C_H_W->setChecked(channelOrder==ChannelOrder::C_H_W);
    ui->order_H_W_C->setChecked(channelOrder==ChannelOrder::H_W_C);

    ui->color_RGB->setEnabled(num_channels>=3);
    ui->color_BGR->setEnabled(num_channels>=3);

    ui->color_Grayscale->setChecked(colorMode==ColorMode::Grayscale);
    ui->color_Colormap->setChecked(colorMode==ColorMode::Colormap);
    ui->color_RGB->setChecked(colorMode==ColorMode::RGB);
    ui->color_BGR->setChecked(colorMode==ColorMode::BGR);
}

void MainWindow::version_downloaded(){
    try {
        QString data = update_checker->downloadedData();
        if (data.length() > 0){
            QStringList list1 = data.split('|');
            if (list1.length() == 2){
                int version = list1.at(0).toInt();
                if (version > app_version){
                    QString text = "New version available!<br>New in this version:";
                    text.append(list1.at(1));
                    QMessageBox msgBox;
                    msgBox.setWindowTitle("New version available");
                    msgBox.setText(text);
                    msgBox.exec();
                }
            }
        }
    } catch (exception e) {

    }
}

MainWindow::~MainWindow() {
    histoGram.close();
    delete ui;
}

// This function handles the mouse clicks and drags in the canvas, and communicates with the histogram window
void MainWindow::mousePressedEvent(QMouseEvent *event) {
    if (!image.isNull() && ui->actionHistogram->isChecked()) {
        QPoint local_pt = event->pos();//ui->imageCanvas->mapFromGlobal(event->globalPos());
        QPointF img_coord_pt = ui->imageCanvas->mapToScene(local_pt);

        short graphNum = 0;
        if (event->buttons() == Qt::RightButton)
            graphNum = 1;

        int x = static_cast<int>(img_coord_pt.x());
        int y = static_cast<int>(img_coord_pt.y());

        if (x <= width && y <= height && y >= 0 && x >= 0) {
            histoGram.setData(&loaded_data, graphNum, x, y, width, height, num_channels, channelOrder);
            histoGram.show();
            histoGram.activateWindow();
        }

    }
}

// This function display pixel values in the statusbar on mouse moves
void MainWindow::mouseMovedEvent(QMouseEvent *event) {
    if (!image.isNull()) {

        QPoint local_pt = event->pos();//ui->imageCanvas->mapFromGlobal(event->globalPos());
        QPointF img_coord_pt = ui->imageCanvas->mapToScene(local_pt);

        int x = static_cast<int>(img_coord_pt.x());
        int y = static_cast<int>(img_coord_pt.y());

        bool channels_first = (channelOrder==ChannelOrder::C_H_W);
        unsigned long index = index_in_vector(channels_first, x, y, ui->channelSlider->value(), width, height, num_channels);

        if (index <= loaded_data.size() && x <= width && y <= height && y >= 0 && x >= 0) {
            float value = static_cast<float>(loaded_data.at(index));

            QString message;
            message.sprintf("(%d, %d) [%g]", x, y, value);
            ui->statusBar->showMessage(message);

            if (event->buttons() == Qt::LeftButton || event->buttons() == Qt::RightButton)
                mousePressedEvent(event);

        }
    }
}

// This function draws a channel onto the canvas
void MainWindow::render_channel(int channel_index) {

    unsigned long imageSize = loaded_data.size() / static_cast<unsigned long>(num_channels);
    QByteArray bitmap_ch1(static_cast<int>(imageSize), '\0');
    QByteArray bitmap_ch2(static_cast<int>(imageSize), '\0');
    QByteArray bitmap_ch3(static_cast<int>(imageSize), '\0');

    float slope = (255.0f) / (max_pixel_in_file - min_pixel_in_file);

    // Build bitmap array
    unsigned long pixel_index1 = 0;
    unsigned long pixel_index2 = 0;
    unsigned long pixel_index3 = 0;

    int i = 0;
    bool channels_first = (channelOrder==ChannelOrder::C_H_W);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            i = y * width + x;
            if (pixel_index1 < loaded_data.size()){
                pixel_index1 = index_in_vector(channels_first, x, y, channel_index, width, height, num_channels);

                if ((colorMode == ColorMode::RGB || colorMode == ColorMode::BGR) && num_channels >= 3){
                    // Get RGB values
                    pixel_index1 = index_in_vector(channels_first, x, y, 0, width, height, num_channels);
                    pixel_index2 = index_in_vector(channels_first, x, y, 1, width, height, num_channels);
                    pixel_index3 = index_in_vector(channels_first, x, y, 2, width, height, num_channels);

                    bitmap_ch1[i] = static_cast<char>((loaded_data[pixel_index1] - min_pixel_in_file) * slope);
                    bitmap_ch2[i] = static_cast<char>((loaded_data[pixel_index2] - min_pixel_in_file) * slope);
                    bitmap_ch3[i] = static_cast<char>((loaded_data[pixel_index3] - min_pixel_in_file) * slope);
                }else{
                    bitmap_ch1[i] = static_cast<char>((loaded_data[pixel_index1] - min_pixel_in_file) * slope);
                }
            }
        }
    }

    // Draw all bitmap pixels
    QImage img = QImage(width, height, QImage::Format_RGB888);
    QColor res;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int val1 = static_cast<uint8_t>(bitmap_ch1.at(x + (y * width)));

            if (colorMode==ColorMode::Colormap) {
                while (val1 > 20)
                    val1 = val1 - 20;
                res = QColor(cmap_red[val1], cmap_green[val1], cmap_blue[val1]);
            } else {
                if (colorMode == ColorMode::RGB && num_channels >= 3){
                    int val2 = static_cast<uint8_t>(bitmap_ch2.at(x + (y * width)));
                    int val3 = static_cast<uint8_t>(bitmap_ch3.at(x + (y * width)));
                    res = QColor(val1, val2, val3);
                }else if (colorMode == ColorMode::BGR && num_channels >= 3){
                    int val2 = static_cast<uint8_t>(bitmap_ch2.at(x + (y * width)));
                    int val3 = static_cast<uint8_t>(bitmap_ch3.at(x + (y * width)));
                    res = QColor(val3, val2, val1);
                }else{
                    res = QColor(val1, val1, val1);
                }
            }
            if (!res.isValid())
                res = QColor(255, 0, 0);

            img.setPixelColor(x, y, res);
        }
    }

    image = img.copy();

    QPixmap item(QPixmap::fromImage(img));
    QGraphicsScene *scene = new QGraphicsScene;
    scene->addPixmap(item);
    ui->imageCanvas->setScene(scene);
    resizeEvent(nullptr);

}

// Load numpy data into vector of correct type, so data is interpret correctly
template<class T>
void MainWindow::load_and_convert_vector(cnpy::NpyArray *arr) {
    T *data_ptr = arr->data<T>();
    vector <T> type_data = vector<T>(data_ptr, data_ptr + (width * height * num_channels));
    loaded_data = std::vector<float>(type_data.begin(), type_data.end());
}

// For display purposes
char const *MainWindow::nameOfType(char type) {
    switch (type) {
    case '?':
        return "bool";
    case 'b':
        return "byte";
    case 'B':
        return "ubyte";
    case 'i':
        return "int";
    case 'u':
        return "uint";
    case 'f':
        return "float";
    case 'c':
        return "Complex-float";
    case 'm':
        return "Timedelta";
    case 'M':
        return "Datetime";
    case 'O':
        return "(Python) objects";
    case 'S':
        return "Zero-terminated bytes";
    case 'a':
        return "Zero-terminated bytes";
    case 'U':
        return "Unicode";
    case 'V':
        return "Raw data";
    default:
        return "";
    }
}

// This function tries to load the file, and sets all corresponding settings
void MainWindow::load_numpy_file(string path) {

    qInfo("Load numpy");
    try {
        cnpy::NpyArray arr = cnpy::npy_load(path);
        int wordSize = static_cast<int>(arr.word_size);
        char data_type = static_cast<char>(arr.data_type);

        if (arr.shape.size() < 2) {
            QMessageBox msgBox;
            msgBox.setText("This numpy array does not have enough dimensions, expecting at least 2");
            msgBox.exec();
            return;
        } else if (arr.shape.size() == 2) {
            qInfo("Array is 2d, assuming 1 channel");
            height = static_cast<int>(arr.shape[0]);
            width = static_cast<int>(arr.shape[1]);
            num_channels = 1;
        } else if (channelOrder==ChannelOrder::C_H_W) {
            qInfo("Expecting channels*height*width shape");
            height = static_cast<int>(arr.shape[1]);
            width = static_cast<int>(arr.shape[2]);
            num_channels = static_cast<int>(arr.shape[0]);
        } else {
            qInfo("Expecting height*width*channels shape");
            height = static_cast<int>(arr.shape[0]);
            width = static_cast<int>(arr.shape[1]);
            num_channels = static_cast<int>(arr.shape[2]);
        }

        string shapeStr(nameOfType(data_type));
        shapeStr.append(to_string(wordSize * 8));
        if (data_type == 'f') {
            qInfo("Creating array of datatype [float] and wordsize %d", wordSize);
            if (wordSize == 4)
                this->load_and_convert_vector<float>(&arr);
            else if (wordSize == 8)
                this->load_and_convert_vector<double>(&arr);
            else
                return;
        } else if (data_type == 'u' || data_type == 'B') {
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
        } else { // Try to parse as signed ints
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

        shapeStr.append(" (");
        for (unsigned long i = 0; i < arr.shape.size(); i++) {
            shapeStr.append(to_string(arr.shape.at(i)));
            if (i < arr.shape.size() - 1)
                shapeStr.append(", ");
        }
        shapeStr.append(")");
        dimensionLabel->setText(QString::fromUtf8(shapeStr.c_str()));

        // Calculate for contrast stretch
        max_pixel_in_file = *max_element(loaded_data.begin(), loaded_data.end());
        min_pixel_in_file = *min_element(loaded_data.begin(), loaded_data.end());

        // Put stats in GUI
        qInfo("Max pixel value in file: %f Min pixel value in file : %f", max_pixel_in_file, min_pixel_in_file);
        QString message;
        message.sprintf("Bands : %i Width : %i Height : %i Wordsize : %i", num_channels, width, height, wordSize);
        qInfo("%s", message.toStdString().c_str());
        ui->statusBar->showMessage(message);

        // Setup histogram
        histoGram.setMax(max_pixel_in_file);
        histoGram.setMin(min_pixel_in_file);

        // Switch back to garyscale if needed
        if (colorMode == ColorMode::RGB || colorMode == ColorMode::BGR)
            colorMode = num_channels >= 3 ? colorMode : ColorMode::Grayscale;
        updateSettingsMenu();

        // Channel slider
        ui->channelSlider->setHidden(num_channels <= 1 || colorMode == ColorMode::RGB || colorMode == ColorMode::BGR);
        ui->channelSlider->setMaximum(num_channels - 1);
        ui->channelSlider->setEnabled(true);

        QFileInfo info(QString::fromUtf8(path.c_str()));
        this->setWindowTitle(info.baseName());
        render_channel(ui->channelSlider->value());
        qInfo("Done!");
        loaded_path = path;
        updateTextInToolbar();
    } catch (std::exception e) {
        QMessageBox msgBox;
        msgBox.setText(e.what());
        msgBox.exec();
        close();
    }

}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QGraphicsScene *scene = ui->imageCanvas->scene();
    if (scene != nullptr) {
        QRectF bounds = ui->imageCanvas->scene()->itemsBoundingRect();
        ui->imageCanvas->fitInView(bounds, Qt::KeepAspectRatio);
        ui->imageCanvas->centerOn(0, 0);
    }
    if (event != nullptr)
        event->accept();
}

void MainWindow::updateTextInToolbar() {
    ui->actionSelected_band->setText(QString("Channel : %1").arg(ui->channelSlider->value()));
}

void MainWindow::on_channelSlider_valueChanged(int value) {
    render_channel(value);
    updateTextInToolbar();
}

void MainWindow::on_actionOpen_triggered() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Numpy Files (*.npy)"));
    if (fileName != nullptr) {
        load_numpy_file(fileName.toStdString());
    }
}

void MainWindow::loadSibling(int offset){
    if (loaded_path.length() > 3){
        QFileInfo info(QString::fromUtf8(loaded_path.c_str()));
        QDir directory = info.dir();
        QStringList files = directory.entryList(QStringList() << "*.npy" << "*.NPY",QDir::Files, QDir::Name);
        int i = 0;
        for (i = 0; i < files.length(); i++) {
            if (files[i] == info.fileName())
                break;
        }
        i = i + offset;

        if (i >= files.length())
            i = 0;
        if (i < 0)
            i = files.length() - 1;

        if (i >= 0 && i < files.length() && files.length() >= 1){
            QString path = directory.filePath(files[i]);
            load_numpy_file(path.toStdString());
        }
    }
}

void MainWindow::loadPrevious(){
    loadSibling(-1);
}

void MainWindow::loadNext(){
    loadSibling(1);
}

void MainWindow::on_actionExport_as_PNG_triggered() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to PNG"), "", tr("PNG image (*.png)"));
    qInfo("%s", fileName.toUtf8().constData());
    if (fileName != nullptr) {
        image.save(fileName);
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    histoGram.close();
    if (event != nullptr)
        event->accept();
}

void MainWindow::on_actionHistogram_triggered(bool checked) {
    if (!checked)
        histoGram.close();
}

void MainWindow::on_actionconvert_triggered() {
    convertWindow.show();
}

void MainWindow::on_order_C_H_W_triggered()
{
    qInfo("Set to mode C*H*W");
    channelOrder = ChannelOrder::C_H_W;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        load_numpy_file(loaded_path);
    }
}

void MainWindow::on_order_H_W_C_triggered()
{
    qInfo("Set to mode H*W*C");
    channelOrder = ChannelOrder::H_W_C;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        load_numpy_file(loaded_path);
    }
}

void MainWindow::on_color_Grayscale_triggered()
{
    colorMode = ColorMode::Grayscale;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        load_numpy_file(loaded_path);
    }
}

void MainWindow::on_color_Colormap_triggered()
{
    qInfo("Colormap triggered");
    colorMode = ColorMode::Colormap;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        render_channel(ui->channelSlider->value());
    }
}

void MainWindow::on_color_RGB_triggered()
{
    colorMode = num_channels >= 3 ? ColorMode::RGB : colorMode;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        load_numpy_file(loaded_path);
    }
}

void MainWindow::on_color_BGR_triggered()
{
    colorMode = num_channels >= 3 ? ColorMode::BGR : colorMode;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        load_numpy_file(loaded_path);
    }
}

