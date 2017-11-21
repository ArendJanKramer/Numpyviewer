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
    void loadCube(string path);
    virtual void closeEvent ( QCloseEvent * event );

protected:

private slots:
    void renderView(long bandnum);

    void updateTextInToolbar();

    void resizeEvent(QResizeEvent* event);

    void on_actionOpen_triggered();

    void on_actionExport_as_PNG_triggered();

    void on_bandSelector_valueChanged(int value);

    void mouseMovedEvent(QMouseEvent *event);

    void mousePressedEvent(QMouseEvent *event);

    void on_actionHistogram_triggered(bool checked);

    void on_actionconvert_triggered();

private:
    Ui::MainWindow *ui;
    //cnpy::NpyArray arr;
    QImage image;
//    float *float_ptr;
    long height;
    long width;
    long bands;
    vector<float> loaded_data;
    float maxPixel;
    HistoGram histoGram;
    ConvertWindow convertWindow;

};

#endif // MAINWINDOW_H
