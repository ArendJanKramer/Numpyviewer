#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define app_version 6

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

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

    void load_numpy_file(const string &path, bool by_user = false);

    template<class T>
    void load_and_convert_vector(cnpy::NpyArray *arr);

    virtual void closeEvent(QCloseEvent *event);

    static char const *nameOfType(char type);


protected:

private slots:

    void render_channel(int batch_index, int bandnum);

    void updateTextInToolbar();

    void resizeEvent(QResizeEvent *event);

    void on_actionOpen_triggered();

    void on_actionExport_as_PNG_triggered();

    void on_channelSlider_valueChanged(int value);

    void mouseMovedEvent(QMouseEvent *event);

    void mousePressedEvent(QMouseEvent *event);

    void on_actionHistogram_triggered(bool checked);

    void on_actionconvert_triggered();

    void version_downloaded();

    void updateSettingsMenu();

    void comboboxItemChanged(const QString &text);

    void on_order_C_H_W_triggered();

    void on_order_H_W_C_triggered();

    void on_color_Colormap_triggered();

    void on_color_RGB_triggered();

    void on_color_BGR_triggered();

    void on_color_Grayscale_triggered();

    void on_color_Seismic_triggered();

    void loadSibling(int offset);

    void loadNext();

    void loadPrevious();

    void on_batchSlider_valueChanged(int value);

    void on_order_automatic_triggered();

    void auto_set_channel_order(cnpy::NpyArray &arr);

    void on_contrast_array_triggered();

    void on_contrast_canvas_triggered();

private:
    Ui::MainWindow *ui;
    //cnpy::NpyArray arr;
    QImage image;
    //    float *float_ptr;
    int height{};
    int width{};
    int num_channels{};
    int batch_size{};

    ChannelOrder user_selected_order;
    ChannelOrder used_channel_order;
    ColorMode colorMode;
    ContrastMode contrastMode;

    vector<float> loaded_data;
    cnpy::npz_t npz_pairs;
    std::string loaded_path;
    float max_pixel_in_file{};
    float min_pixel_in_file{};
    HistoGram histoGram;
    ConvertWindow convertWindow;
    FileDownloader *update_checker;
    QLabel *dimensionLabel;
    QComboBox *comboBox;
    QAction* comboBoxAction;

};

#endif // MAINWINDOW_H
