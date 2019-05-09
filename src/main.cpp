#include "mainwindow.h"
#include <QApplication>
#include <QFileOpenEvent>
#include <QtDebug>

#ifdef __APPLE__
#define apple true
#else
#define apple false
#endif


class NumpyViewer : public QApplication {
public:
    NumpyViewer(int &argc, char **argv)
        : QApplication(argc, argv) {

        if (argc <= 1) {
            MainWindow *w = new MainWindow;
            w->show();
        }

        for (int i = 0; i < argc - 1; i++) {
            MainWindow *w = new MainWindow;
            w->show();
            w->load_numpy_file(argv[i + 1]);
        }

        exec();
    }

    bool event(QEvent *event) {
        if (event->type() == QEvent::FileOpen) {
            QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);

            qDebug() << "Open file" << openEvent->file();

            MainWindow *w = new MainWindow;
            w->show();
            w->load_numpy_file(openEvent->file().toStdString().c_str());

        }

        return QApplication::event(event);
    }
};


int main(int argc, char *argv[]) {
    NumpyViewer a(argc, argv);
}
