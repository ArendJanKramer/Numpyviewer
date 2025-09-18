#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <cstdint>
#include "keyeventhandler.h"
#include <QtCharts>
#include "graphics_view_zoom.h"
#include <locale>
#include "colormap.h"
using namespace QtCharts;
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow) {
    colorMode = ColorMode::Grayscale;
    contrastMode = ContrastMode::Array;
    user_selected_order = ChannelOrder::Auto;
    used_channel_order = ChannelOrder::H_W_C;
    loaded_path = "";

    ui->setupUi(this);

    auto* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->toolBar->addWidget(spacer);

    this->comboBox = new QComboBox();
    comboBox->setFixedWidth(200);
    connect(comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(comboboxItemChanged(const QString &)));
    comboBoxAction = ui->toolBar->addWidget(this->comboBox);
    comboBoxAction->setVisible(false);


    auto *z = new Graphics_view_zoom(ui->imageCanvas);
    z->set_modifiers(Qt::NoModifier);

    ui->imageCanvas->setMouseTracking(true);

    updateSettingsMenu();
    ui->channelSlider->setHidden(true);
    ui->channelLabel->setHidden(true);

    ui->batchSlider->setHidden(true);
    ui->batchLabel->setHidden(true);

    dimensionLabel = new QLabel(this);
    dimensionLabel->setText("");
    ui->statusBar->addPermanentWidget(dimensionLabel, 0);


    auto *handler = new KeyEventHandler(ui->imageCanvas);
    ui->imageCanvas->installEventFilter(handler);
    connect(handler, &KeyEventHandler::mousePositionChanged, this, &MainWindow::mouseMovedEvent);
    connect(handler, &KeyEventHandler::mousePressed, this, &MainWindow::mousePressedEvent);

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left), this, SLOT(loadPrevious()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right), this, SLOT(loadNext()));

    QUrl imageUrl("https://raw.githubusercontent.com/ArendJanKramer/Qt-ENVI-Numpy-Viewer/master/version.txt");
    update_checker = new FileDownloader(imageUrl, this);
    connect(update_checker, SIGNAL(downloaded()), this, SLOT(version_downloaded()));
}

void MainWindow::updateSettingsMenu() {
    ui->order_C_H_W->setChecked(user_selected_order == ChannelOrder::C_H_W);
    ui->order_H_W_C->setChecked(user_selected_order == ChannelOrder::H_W_C);
    ui->order_automatic->setChecked(user_selected_order == ChannelOrder::Auto);

    ui->color_RGB->setEnabled(num_channels >= 3);
    ui->color_BGR->setEnabled(num_channels >= 3);

    ui->color_Grayscale->setChecked(colorMode == ColorMode::Grayscale);
    ui->color_Colormap->setChecked(colorMode == ColorMode::Colormap);
    ui->color_RGB->setChecked(colorMode == ColorMode::RGB);
    ui->color_BGR->setChecked(colorMode == ColorMode::BGR);
    ui->color_Seismic->setChecked(colorMode == ColorMode::Seismic);
    ui->color_Viridis->setChecked(colorMode == ColorMode::Viridis);

    ui->contrast_array->setChecked(contrastMode == ContrastMode::Array);
    ui->contrast_canvas->setChecked(contrastMode == ContrastMode::Canvas);
}

void MainWindow::version_downloaded() {
    try {
        QString data = update_checker->downloadedData();
        if (data.length() > 0) {
            QStringList list1 = data.split('|');
            if (list1.length() == 2) {
                int version = list1.at(0).toInt();
                if (version > app_version) {
                    QString text = "New version available!<br>New in this version:";
                    text.append(list1.at(1));
                    QMessageBox msgBox;
                    msgBox.setWindowTitle("New version available");
                    msgBox.setText(text);
                    msgBox.exec();
                }
            }
        }
    } catch (exception &e) {
        qWarning("Could not check for updates");
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
        int n = ui->batchSlider->value();

        if (x <= width && y <= height && y >= 0 && x >= 0) {
            histoGram.setData(&loaded_data, graphNum, n, x, y, width, height, num_channels, used_channel_order);
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
        int n = ui->batchSlider->value();

        int channelSelected = ui->channelSlider->value();
        bool isMultiChannel = (colorMode == ColorMode::RGB) || (colorMode == ColorMode::BGR);

        unsigned long index = index_in_vector(used_channel_order, n, x, y, channelSelected, width, height, num_channels);

        if (index < loaded_data.size() && x >= 0 && x <= width && y >= 0 && y <= height) {
            QString message;

            if (isMultiChannel) {
                // RGB/BGR display mode is on, so we will extract values for first 3 channels:
                unsigned long indices[] = {
                                    index_in_vector(used_channel_order, n, x, y, 0, width, height, num_channels),
                                    index_in_vector(used_channel_order, n, x, y, 1, width, height, num_channels),
                                    index_in_vector(used_channel_order, n, x, y, 2, width, height, num_channels)
                              };

                auto value_0 = static_cast<float>(loaded_data.at(indices[0]));
                auto value_1 = static_cast<float>(loaded_data.at(indices[1]));
                auto value_2 = static_cast<float>(loaded_data.at(indices[2]));

                message.sprintf("(%d, %d) [%g, %g, %g]", x, y, value_0, value_1, value_2);

            } else {
                // Extracting only the specified channel value, because only a single-channel is being displayed
                auto value = static_cast<float>(loaded_data.at(index));
                message.sprintf("(%d, %d) [%g] /ch: %d/", x, y, value, channelSelected);
            }

            ui->statusBar->showMessage(message);

            if (event->buttons() == Qt::LeftButton || event->buttons() == Qt::RightButton)
                mousePressedEvent(event);

        }
    }
}

// This function draws a channel onto the canvas
void MainWindow::render_channel(int batch_index, int channel_index) {

    unsigned long imageSize = loaded_data.size() / static_cast<unsigned long>(num_channels * batch_size);
    std::vector<float> bitmap_ch1(static_cast<int>(imageSize), '\0');
    std::vector<float> bitmap_ch2(static_cast<int>(imageSize), '\0');
    std::vector<float> bitmap_ch3(static_cast<int>(imageSize), '\0');

    float max_pixel = max_pixel_in_file;
    float min_pixel = min_pixel_in_file;

    float slope = (255.0f) / (max_pixel - min_pixel);

    // Build bitmap array
    unsigned long pixel_index1 = 0;
    unsigned long pixel_index2 = 0;
    unsigned long pixel_index3 = 0;

    int i = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            i = y * width + x;
            if (pixel_index1 < loaded_data.size()) {
                pixel_index1 = index_in_vector(used_channel_order, batch_index, x, y, channel_index, width, height,
                                               num_channels);

                if ((colorMode == ColorMode::RGB || colorMode == ColorMode::BGR) && num_channels >= 3) {
                    // Get RGB values
                    pixel_index1 = index_in_vector(used_channel_order, batch_index, x, y, 0, width, height,
                                                   num_channels);
                    pixel_index2 = index_in_vector(used_channel_order, batch_index, x, y, 1, width, height,
                                                   num_channels);
                    pixel_index3 = index_in_vector(used_channel_order, batch_index, x, y, 2, width, height,
                                                   num_channels);

                    bitmap_ch1[i] = (loaded_data[pixel_index1] - min_pixel) * slope;
                    bitmap_ch2[i] = (loaded_data[pixel_index2] - min_pixel) * slope;
                    bitmap_ch3[i] = (loaded_data[pixel_index3] - min_pixel) * slope;
                } else {
                    bitmap_ch1[i] = (loaded_data[pixel_index1] - min_pixel) * slope;
                }
            }
        }
    }

    // Re-stretch contrast for current canvas
    if (contrastMode == ContrastMode::Canvas) {
        auto mm = std::minmax_element(bitmap_ch1.begin(), bitmap_ch1.end());
        min_pixel = static_cast<float>(*mm.first);
        max_pixel = static_cast<float >(*mm.second);
        slope = (255.0f) / (max_pixel - min_pixel);
//        qInfo("Max pixel value in canvas: %f Min pixel value in canvas : %f", max_pixel, min_pixel);
    }

    // Draw all bitmap pixels
    QImage img = QImage(width, height, QImage::Format_RGB888);
    QColor res;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int val1 = static_cast<uint8_t>(bitmap_ch1.at(x + (y * width)));

            if (colorMode == ColorMode::Colormap) {
                while (val1 > 20)
                    val1 = val1 - 20;
                res = QColor(cmap_red[val1], cmap_green[val1], cmap_blue[val1]);
            } else if (colorMode == ColorMode::Seismic){
                uint8_t result[3];
                pixel_index1 = index_in_vector(used_channel_order, batch_index, x, y, 0, width, height,num_channels);
                getSeismicColormap(loaded_data[pixel_index1], max_pixel, min_pixel, result);
                res = QColor(result[0], result[1], result[2]);
            } else if (colorMode == ColorMode::Viridis){
                uint8_t result[3];
                pixel_index1 = index_in_vector(used_channel_order, batch_index, x, y, 0, width, height,num_channels);
                getViridisColormap(loaded_data[pixel_index1], max_pixel, min_pixel, result);
                res = QColor(result[0], result[1], result[2]);
            } else {
                if (colorMode == ColorMode::RGB && num_channels >= 3) {
                    int val2 = static_cast<uint8_t>(bitmap_ch2.at(x + (y * width)));
                    int val3 = static_cast<uint8_t>(bitmap_ch3.at(x + (y * width)));
                    res = QColor(val1, val2, val3);
                } else if (colorMode == ColorMode::BGR && num_channels >= 3) {
                    int val2 = static_cast<uint8_t>(bitmap_ch2.at(x + (y * width)));
                    int val3 = static_cast<uint8_t>(bitmap_ch3.at(x + (y * width)));
                    res = QColor(val3, val2, val1);
                } else {
                    if (contrastMode == ContrastMode::Canvas) {
                        val1 = static_cast<uint8_t>((bitmap_ch1.at(x + (y * width)) - min_pixel) * slope);
                    }
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
    auto *scene = new QGraphicsScene;
    scene->addPixmap(item);
    ui->imageCanvas->setScene(scene);
    resizeEvent(nullptr);

}

int biggestOutlier(const std::vector<size_t>& shape){
    // Finds the axis that outlies the most from the others. This is generally speaking the channel axis
    size_t average = accumulate( shape.begin(), shape.end(), 0.0)/shape.size();
    size_t abs_diff = 0;
    int axis = 0;
    int i = 0;
    for (size_t a : shape){
        size_t diff = std::abs(int(a - average));
        if (diff > abs_diff) {
            abs_diff = diff;
            axis = i;
        }
        i++;
    }
    return axis;
}

void MainWindow::auto_set_channel_order(cnpy::NpyArray &arr) {
    qInfo() << "Determining best channel order for shape " << QString::fromStdString(arr.shape_str());
    int num_dimensions = arr.shape.size();
    int biggest_outlier = biggestOutlier(arr.shape);
    if (num_dimensions == 3) {
        if (biggest_outlier != 2) {
            used_channel_order = ChannelOrder::C_H_W;
        } else {
            used_channel_order = ChannelOrder::H_W_C;
        }
    } else if (num_dimensions == 4) {
        if (biggest_outlier != 3) {
            used_channel_order = ChannelOrder::C_H_W;
        } else {
            used_channel_order = ChannelOrder::H_W_C;
        }
    } else {
        used_channel_order = ChannelOrder::H_W_C;
    }
}

// Load numpy data into vector of correct type, so data is interpret correctly
template<class T>
void MainWindow::load_and_convert_vector(cnpy::NpyArray *arr) {
    T *data_ptr = arr->data<T>();
    vector<T> type_data = vector<T>(data_ptr, data_ptr + (width * height * num_channels * batch_size));
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

bool isSuffix(string s1, string s2) {
    int n1 = s1.length(), n2 = s2.length();
    if (n1 > n2)
        return false;
    for (int i = 0; i < n1; i++)
        if (s1[n1 - i - 1] != s2[n2 - i - 1])
            return false;
    return true;
}


// This function tries to load the file, and sets all corresponding settings
void MainWindow::load_numpy_file(const string &path, bool by_user) {
    // Make lower case of path
    string lower = path;
    transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    cnpy::NpyArray arr;

    qInfo("Load numpy");
    try {

        // If npz file
        if (isSuffix("npz", lower)) {

            if (by_user) { // User loaded a file
                qInfo("Reloading npz file from disk");
                npz_pairs = cnpy::npz_load(path);
                loaded_path = path;

                // Hide, disable and clear combobox
                comboBox->setEnabled(false);
                comboBoxAction->setVisible(false);
                comboBox->clear();

                for (const auto &pair : npz_pairs) {
                    comboBoxAction->setVisible(true);

                    auto name = pair.first;
                    auto array = pair.second;
                    string title = name + " " + array.shape_str();
                    comboBox->addItem(title.c_str(), QVariant(name.c_str()));
                }

                comboBox->setEnabled(true);
                // The combobox will automatically call comboboxItemChanged()
                // This function will be called with the corresponding array
                return;
            }
            else {
                // If the combobox is disabled, we are constructing it
                if (!comboBox->isEnabled())
                    return;

                QString value = comboBox->currentData().toString();
                auto p = npz_pairs.at(value.toUtf8().constData());
                arr = p;
            }

        } else {
            npz_pairs.clear();
            comboBoxAction->setVisible(false);
            arr = cnpy::npy_load(path);
        }

        int wordSize = static_cast<int>(arr.word_size);
        char data_type = static_cast<char>(arr.data_type);
        int num_dimensions = arr.shape.size();

        ui->batchSlider->setValue(0);
        ui->channelSlider->setValue(0);
        this->updateTextInToolbar();

        if (user_selected_order == ChannelOrder::Auto) {
            auto_set_channel_order(arr);
        }

        if (num_dimensions < 1) {
            QMessageBox msgBox;
            msgBox.setText("This numpy array does not have any dimensions, expecting at least 1");
            msgBox.exec();
            return;
        }
        else if (num_dimensions < 2) {
            height = 1;
            width = 1;
            num_channels = static_cast<int>(arr.shape[0]);
            batch_size = 1;

//            QMessageBox msgBox;
//            msgBox.setText("This numpy array does not have enough dimensions, expecting at least 2");
//            msgBox.exec();
//            return;
        } else if (num_dimensions == 2) {
            qInfo("Array is 2d, assuming 1 channel");
            height = static_cast<int>(arr.shape[0]);
            width = static_cast<int>(arr.shape[1]);
            num_channels = 1;
            batch_size = 1;
        } else if (used_channel_order == ChannelOrder::C_H_W && num_dimensions == 3) {
            qInfo("Rendering as channels*height*width");
            height = static_cast<int>(arr.shape[1]);
            width = static_cast<int>(arr.shape[2]);
            num_channels = static_cast<int>(arr.shape[0]);
            batch_size = 1;
        } else if (used_channel_order == ChannelOrder::C_H_W && num_dimensions >= 4) {
            qInfo("Rendering as n*channels*height*width");
            height = static_cast<int>(arr.shape[2]);
            width = static_cast<int>(arr.shape[3]);
            num_channels = static_cast<int>(arr.shape[1]);
            batch_size = static_cast<int>(arr.shape[0]);
        } else if (used_channel_order == ChannelOrder::H_W_C && num_dimensions == 3) {
            qInfo("Rendering as height*width*channels");
            height = static_cast<int>(arr.shape[0]);
            width = static_cast<int>(arr.shape[1]);
            num_channels = static_cast<int>(arr.shape[2]);
            batch_size = 1;
        } else if (used_channel_order == ChannelOrder::H_W_C && num_dimensions >= 4) {
            qInfo("Rendering as n*height*width*channels");
            height = static_cast<int>(arr.shape[1]);
            width = static_cast<int>(arr.shape[2]);
            num_channels = static_cast<int>(arr.shape[3]);
            batch_size = static_cast<int>(arr.shape[0]);
        } else {
            QMessageBox msgBox;
            msgBox.setText("This numpy array is not understood");
            msgBox.exec();
            return;
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

        shapeStr.append(" ");
        shapeStr.append(arr.shape_str());

        dimensionLabel->setText(QString::fromUtf8(shapeStr.c_str()));

        // Calculate for contrast stretch
        auto mm = std::minmax_element(loaded_data.begin(), loaded_data.end());
        min_pixel_in_file = *mm.first;
        max_pixel_in_file = *mm.second;

        // Put stats in GUI
        qInfo("Max pixel value in file: %f Min pixel value in file : %f", max_pixel_in_file, min_pixel_in_file);
        QString message;
        message.sprintf("Channels : %i Width : %i Height : %i Wordsize : %i", num_channels, width, height, wordSize);
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
        ui->channelLabel->setHidden(ui->channelSlider->isHidden());

        ui->channelSlider->setMaximum(num_channels - 1);
        ui->channelSlider->setEnabled(true);
        ui->batchSlider->setEnabled(true);

        ui->batchSlider->setHidden(num_dimensions <= 3 || batch_size <= 1);
        ui->batchLabel->setHidden(ui->batchSlider->isHidden());
        ui->batchSlider->setMaximum(batch_size - 1);

        QFileInfo info(QString::fromUtf8(path.c_str()));
        this->setWindowTitle(info.fileName());
        render_channel(ui->batchSlider->value(), ui->channelSlider->value());
        qInfo("Done!");
        loaded_path = path;
        updateTextInToolbar();
    } catch (std::exception &e) {
        QMessageBox msgBox;
        msgBox.setText(e.what());
        msgBox.exec();
//        close();
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
    if (batch_size > 1 && num_channels <= 1) {
        ui->actionSelected_band->setText(QString("Batch: %1").arg(ui->batchSlider->value()));
    } else if (batch_size > 1 && num_channels > 1) {
        ui->actionSelected_band->setText(
                QString("Batch: %1, Channel: %2").arg(ui->batchSlider->value()).arg(ui->channelSlider->value()));
    } else {
        ui->actionSelected_band->setText(QString("Channel: %1").arg(ui->channelSlider->value()));
    }
}

void MainWindow::on_channelSlider_valueChanged(int value) {
    render_channel(ui->batchSlider->value(), value);
    updateTextInToolbar();
}


void MainWindow::on_batchSlider_valueChanged(int value) {
    render_channel(value, ui->channelSlider->value());
    updateTextInToolbar();
}

void MainWindow::on_actionOpen_triggered() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Numpy Files (*.npy *.npz)"));
    if (fileName != nullptr) {
        load_numpy_file(fileName.toStdString(), true);
    }
}

void MainWindow::loadSibling(int offset) {
    if (loaded_path.length() > 3) {
        QFileInfo info(QString::fromUtf8(loaded_path.c_str()));
        QDir directory = info.dir();
        QStringList files = directory.entryList(QStringList() << "*.npy" << "*.NPY", QDir::Files, QDir::Name);
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

        if (i >= 0 && i < files.length() && files.length() >= 1) {
            QString path = directory.filePath(files[i]);
            load_numpy_file(path.toStdString());
        }
    }
}

void MainWindow::loadPrevious() {
    loadSibling(-1);
}

void MainWindow::loadNext() {
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

void MainWindow::comboboxItemChanged(const QString &text){
    load_numpy_file(loaded_path);
}

void MainWindow::on_actionHistogram_triggered(bool checked) {
    if (!checked)
        histoGram.close();
}

void MainWindow::on_actionconvert_triggered() {
    convertWindow.show();
}

void MainWindow::on_order_C_H_W_triggered() {
    qInfo("Set to mode C*H*W");
    user_selected_order = ChannelOrder::C_H_W;
    used_channel_order = ChannelOrder::C_H_W;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        load_numpy_file(loaded_path);
    }
}

void MainWindow::on_order_H_W_C_triggered() {
    qInfo("Set to mode H*W*C");
    user_selected_order = ChannelOrder::H_W_C;
    used_channel_order = ChannelOrder::H_W_C;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        load_numpy_file(loaded_path);
    }
}


void MainWindow::on_order_automatic_triggered() {
    qInfo("Set to mode auto");
    user_selected_order = ChannelOrder::Auto;
    used_channel_order = ChannelOrder::Auto;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        load_numpy_file(loaded_path);
    }
}


void MainWindow::on_color_Grayscale_triggered() {
    colorMode = ColorMode::Grayscale;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        load_numpy_file(loaded_path);
    }
}

void MainWindow::on_color_Colormap_triggered() {
    qInfo("Colormap triggered");
    colorMode = ColorMode::Colormap;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        render_channel(ui->batchSlider->value(), ui->channelSlider->value());
    }
}

void MainWindow::on_color_RGB_triggered() {
    colorMode = num_channels >= 3 ? ColorMode::RGB : colorMode;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        load_numpy_file(loaded_path);
    }
}

void MainWindow::on_color_BGR_triggered() {
    colorMode = num_channels >= 3 ? ColorMode::BGR : colorMode;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        load_numpy_file(loaded_path);
    }
}

void MainWindow::on_color_Seismic_triggered() {
    qInfo("Seismic Colormap triggered");
    colorMode = ColorMode::Seismic;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        load_numpy_file(loaded_path);
    }
}

void MainWindow::on_color_Viridis_triggered() {
    qInfo("Viridis Colormap triggered");
    colorMode = ColorMode::Viridis;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        load_numpy_file(loaded_path);
    }
}

void MainWindow::on_contrast_array_triggered() {
    qInfo("Contrast mode triggered");
    contrastMode = ContrastMode::Array;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        render_channel(ui->batchSlider->value(), ui->channelSlider->value());
    }
}

void MainWindow::on_contrast_canvas_triggered() {
    qInfo("Contrast mode triggered");
    contrastMode = ContrastMode::Canvas;
    updateSettingsMenu();
    if (loaded_path.length() > 3) {
        render_channel(ui->batchSlider->value(), ui->channelSlider->value());
    }
}
