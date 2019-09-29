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
    NumpyViewer(int &argc, char **argv) : QApplication(argc, argv) {

        if (argc <= 1) {
            auto *w = new MainWindow;
            w->show();
        }

        for (int i = 0; i < argc - 1; i++) {
            auto *w = new MainWindow;
            w->show();
            w->load_numpy_file(argv[i + 1]);
        }

        exec();
    }

    bool event(QEvent *event) override {
        if (event->type() == QEvent::FileOpen) {
            auto *openEvent = dynamic_cast<QFileOpenEvent *>(event);

            qDebug() << "Open file" << openEvent->file();

            auto *w = new MainWindow;
            w->show();
            w->load_numpy_file(openEvent->file().toStdString());
        }

        return QApplication::event(event);
    }
};


int main(int argc, char *argv[]) {
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    NumpyViewer a(argc, argv);
}
