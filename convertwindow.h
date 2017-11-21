#ifndef CONVERTWINDOW_H
#define CONVERTWINDOW_H

#include <QMainWindow>
#include "envi_parser.h"

namespace Ui {
class ConvertWindow;
}

class ConvertWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConvertWindow(QWidget *parent = 0);
    ~ConvertWindow();

private slots:
    void on_btnCaptureBrowse_clicked();

    void on_btnCubeBrowser_clicked();

    void on_btnConvert_clicked();

    void on_normalize_toggled(bool checked);

    void on_btnBatch_clicked();

private:
    Ui::ConvertWindow *ui;
    envi_parser enviParser;
    bool convertFolder(QString source, QString destination, bool normalize, float pixelGain);

};

#endif // CONVERTWINDOW_H
