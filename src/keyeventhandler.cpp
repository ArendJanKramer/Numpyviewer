#include "keyeventhandler.h"

KeyEventHandler::KeyEventHandler(QObject *parent) : QObject(parent) {

}

bool KeyEventHandler::eventFilter(QObject *obj, QEvent *event) {
    bool res = false;
    switch (event->type()) {
    case QEvent::MouseButtonDblClick:
        emit mouseDoubleClicked(static_cast<QMouseEvent *>(event));
        res = true;
        break;
    case QEvent::MouseButtonPress:
        emit mousePressed(static_cast<QMouseEvent *>(event));
        res = true;
        break;
    case QEvent::MouseButtonRelease:
        emit mouseReleased(static_cast<QMouseEvent *>(event));
        res = true;
        break;
    case QEvent::MouseMove:
        emit mousePositionChanged(static_cast<QMouseEvent *>(event));
        res = true;
        break;
    default:
        res = QObject::eventFilter(obj, event);
    }
    return res;
}
