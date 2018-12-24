#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define app_version 2

#include <QMainWindow>
#include <QtDebug>
#include "cnpy.h"
#include <QMouseEvent>
#include "histogram.h"
#include "convertwindow.h"
#include "filedownloader.h"
#include "colormap.h"

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void load_numpy_file(string path);

    template <class T>
    void load_and_convert_vector(cnpy::NpyArray *arr);

    virtual void closeEvent ( QCloseEvent * event );

    char const* nameOfType(char type);


protected:

private slots:
    void render_channel(int bandnum);

    void updateTextInToolbar();

    void resizeEvent(QResizeEvent* event);

    void on_actionOpen_triggered();

    void on_actionExport_as_PNG_triggered();

    void on_channelSlider_valueChanged(int value);

    void mouseMovedEvent(QMouseEvent *event);

    void mousePressedEvent(QMouseEvent *event);

    void on_actionHistogram_triggered(bool checked);

    void on_actionconvert_triggered();

    void version_downloaded();

    void updateSettingsMenu();

    void on_order_C_H_W_triggered();

    void on_order_H_W_C_triggered();

    void on_color_Colormap_triggered();

    void on_color_RGB_triggered();

    void on_color_BGR_triggered();

    void on_color_Grayscale_triggered();

    void loadSibling(int offset);
    void loadNext();
    void loadPrevious();

private:
    Ui::MainWindow *ui;
    //cnpy::NpyArray arr;
    QImage image;
    //    float *float_ptr;
    int height;
    int width;
    int num_channels;

    ChannelOrder channelOrder;
    ColorMode colorMode;

    vector<float> loaded_data;
    std::string loaded_path;
    float max_pixel_in_file;
    float min_pixel_in_file;
    HistoGram histoGram;
    ConvertWindow convertWindow;
    FileDownloader *update_checker;
    QLabel *dimensionLabel;

};

#endif // MAINWINDOW_H
