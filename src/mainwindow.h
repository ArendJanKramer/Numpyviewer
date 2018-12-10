#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtDebug>
#include "cnpy.h"
#include <QMouseEvent>
#include "histogram.h"
#include "convertwindow.h"

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


protected:

private slots:
    void render_channel(long bandnum);

    void updateTextInToolbar();

    void resizeEvent(QResizeEvent* event);

    void on_actionOpen_triggered();

    void on_actionExport_as_PNG_triggered();

    void on_channelSlider_valueChanged(int value);

    void mouseMovedEvent(QMouseEvent *event);

    void mousePressedEvent(QMouseEvent *event);

    void on_actionHistogram_triggered(bool checked);

    void on_actionconvert_triggered();

    void on_actionChannels_first_triggered();

    void on_actionUse_colormap_instead_of_grayscale_triggered();

private:
    Ui::MainWindow *ui;
    //cnpy::NpyArray arr;
    QImage image;
//    float *float_ptr;
    int height;
    int width;
    int num_channels;
    bool channelsfirst;
    bool use_colormap;

    vector<float> loaded_data;
    std::string loaded_path;
    float max_pixel_in_file;
    float min_pixel_in_file;
    HistoGram histoGram;
    ConvertWindow convertWindow;

};

#endif // MAINWINDOW_H
