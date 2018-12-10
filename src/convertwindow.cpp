#include "convertwindow.h"
#include "ui_convertwindow.h"
#include <QFileDialog>
#include <QDirIterator>
#include <QMessageBox>
#include "INIReader.h"

ConvertWindow::ConvertWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ConvertWindow)
{
    ui->setupUi(this);
}

ConvertWindow::~ConvertWindow()
{
    delete ui;
}

void ConvertWindow::on_btnCaptureBrowse_clicked()
{
    QString fileName = QFileDialog::getExistingDirectory(this, tr("Open Image"), "", QFileDialog::ShowDirsOnly
                                                         | QFileDialog::DontResolveSymlinks);
    if (fileName != nullptr){
        QFileInfo info1(fileName);
        QDir info2 = info1.absoluteDir();
        ui->captureFolder->setText(fileName);
        ui->outputFile->setText(info2.absoluteFilePath("file.npy"));
    }
}

void ConvertWindow::on_btnNumpyBrowser_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to numpy"), "", tr("Numpy file (*.npy)"));
    qInfo("%s", fileName.toUtf8().constData());
    if (fileName != nullptr){
        ui->outputFile->setText(fileName);
    }
}
bool ConvertWindow::convertFolder(QString source, QString destination, bool normalize, float pixelGain){
    qint64 biggestSize = 0;
    std::string biggestFilename = "";
    std::string biggestFile = "";
    std::string whiteFile = "";
    std::string darkFile = "";
    std::string hdrFile = "";
    std::string destinationFile = destination.toStdString();

    QDirIterator it(source, QStringList() << "*.RAW", QDir::Files);
    while (it.hasNext()){
        QFileInfo fileInfo(it.next());
        if (fileInfo.size() > biggestSize && fileInfo.isFile()){
            biggestFile = fileInfo.absoluteFilePath().toStdString();
            biggestFilename = fileInfo.fileName().toStdString();
            biggestSize = fileInfo.size();
        }
        if (fileInfo.fileName().contains("DARKREF")){
            darkFile = fileInfo.absoluteFilePath().toStdString();
        }
        if (fileInfo.fileName().contains("WHITEREF")){
            whiteFile = fileInfo.absoluteFilePath().toStdString();
        }
    }

    size_t lastindex = biggestFilename.find_last_of(".");
    std::string rawname = biggestFilename.substr(0, lastindex);

    qInfo("RAW : %s",rawname.c_str());

    QDirIterator it2(source, QStringList() << "*.HDR", QDir::Files);
    while (it2.hasNext()){
        QFileInfo fileInfo(it2.next());
        if (fileInfo.fileName().startsWith(QString::fromUtf8(rawname.c_str())))
            hdrFile = fileInfo.absoluteFilePath().toStdString();
    }


    qInfo("1. %s\n\n2. %s\n\n3. %s\n\n4. %s\n\n",biggestFile.c_str(), whiteFile.c_str(), darkFile.c_str(), hdrFile.c_str());
    std::fflush(stdout);

    INIReader reader(hdrFile);

    long width = reader.GetInteger("NV", "samples", -1);
    long height = reader.GetInteger("NV", "lines", -1);
    long bands = reader.GetInteger("NV", "bands", -1);

    if (width < 0){
        QMessageBox Msgbox;
            Msgbox.setText("Could not read HDR file");
            Msgbox.exec();
        return false;
    }

    if (normalize){
        return (enviParser.processNormalizedCapture(biggestFile, darkFile, whiteFile, destinationFile, width, bands));
    }else{
        return (enviParser.processCapture(biggestFile, darkFile, whiteFile, destinationFile, width, bands, pixelGain));
    }
}
void ConvertWindow::on_btnConvert_clicked()
{
    if (convertFolder(ui->captureFolder->text(), ui->outputFile->text(), ui->normalize->isChecked(), ui->inpGain->value())){
        QMessageBox Msgbox;
            Msgbox.setText("Conversion successful!");
            Msgbox.exec();
    }else{
        QMessageBox Msgbox;
            Msgbox.setText("Conversion to numpy failed!");
            Msgbox.exec();
    }
}

void ConvertWindow::on_normalize_toggled(bool checked)
{
    ui->inpGain->setEnabled(!checked);
}

void ConvertWindow::on_btnBatch_clicked()
{
    QDirIterator it(ui->captureFolder->text(), QStringList() << "DARKREF*.raw", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()){
        QFileInfo fileInfo(it.next());
        QDir capFolder = fileInfo.absoluteDir();
        QDir baseFolder = fileInfo.absoluteDir();
        baseFolder.cdUp();

        QString destinationFile = baseFolder.absoluteFilePath("file.npy");
        if (ui->normalize->isChecked())
            destinationFile = baseFolder.absoluteFilePath("file-norm.npy");

        QString source = capFolder.absolutePath();

        qInfo(" - %s\n", source.toStdString().c_str());
        qInfo(" = %s\n", destinationFile.toStdString().c_str());

        if (convertFolder(source, destinationFile, ui->normalize->isChecked(), ui->inpGain->value()))
            ui->statusBar->showMessage(baseFolder.dirName() + " converted");
        else
            ui->statusBar->showMessage(baseFolder.dirName() + " failed to convert");

        qApp->processEvents();

    }
    ui->statusBar->showMessage("Done!");

}
