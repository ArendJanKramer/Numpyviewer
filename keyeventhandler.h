#ifndef KEYEVENTHANDLER_H
#define KEYEVENTHANDLER_H

#include <QObject>
#include <QtDebug>
#include <QEvent>
#include <QMouseEvent>

class KeyEventHandler : public QObject
{
    Q_OBJECT
public:
    explicit KeyEventHandler(QObject *parent = 0);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

signals:

    void mouseDoubleClicked(QMouseEvent *event);
    void mousePressed(QMouseEvent *event);
    void mouseReleased(QMouseEvent *event);
    void mousePositionChanged(QMouseEvent *event);

public slots:
};

#endif // KEYEVENTHANDLER_H
