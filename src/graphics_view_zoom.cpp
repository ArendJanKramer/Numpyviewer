#include "graphics_view_zoom.h"
#include <QMouseEvent>
#include <QApplication>
#include <QScrollBar>
#include <qmath.h>

Graphics_view_zoom::Graphics_view_zoom(QGraphicsView *view)
    : QObject(view), _view(view) {
    _view->viewport()->installEventFilter(this);
    _view->setMouseTracking(true);
    _modifiers = Qt::ControlModifier;
    _zoom_factor_base = 1.0015;
    _horizontal_scroll_accumulator = 0;
}

void Graphics_view_zoom::gentle_zoom(double factor) {
    _view->scale(factor, factor);
    _view->centerOn(target_scene_pos);
    QPointF delta_viewport_pos = target_viewport_pos - QPointF(_view->viewport()->width() / 2.0,
                                                               _view->viewport()->height() / 2.0);
    QPointF viewport_center = _view->mapFromScene(target_scene_pos) - delta_viewport_pos;
    _view->centerOn(_view->mapToScene(viewport_center.toPoint()));
    emit zoomed();
}

void Graphics_view_zoom::set_modifiers(Qt::KeyboardModifiers modifiers) {
    _modifiers = modifiers;

}

void Graphics_view_zoom::set_zoom_factor_base(double value) {
    _zoom_factor_base = value;
}

bool Graphics_view_zoom::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
        QPointF delta = target_viewport_pos - mouse_event->pos();
        if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5) {
            target_viewport_pos = mouse_event->pos();
            target_scene_pos = _view->mapToScene(mouse_event->pos());
        }
    } else if (event->type() == QEvent::Wheel) {
        QWheelEvent *wheel_event = static_cast<QWheelEvent *>(event);
        auto keyboard_modifiers = QApplication::keyboardModifiers();

        double vertical_angle = wheel_event->angleDelta().y();
        double horizontal_angle = wheel_event->angleDelta().x();

        if (QApplication::keyboardModifiers() == _modifiers) {
            if (abs(vertical_angle) > 0) {
                double factor = qPow(_zoom_factor_base, vertical_angle);
                gentle_zoom(factor);
                return true;
            }
        }

        // -- added Horizontal scroll over the Canvas to scroll through examples in the batch (or stack).
        if (abs(horizontal_angle) > 0) {

            if (QApplication::keyboardModifiers() & Qt::ShiftModifier) {
                horizontal_angle *= 3;
            }

            auto parentWidget = this->_view->parent();
            auto batchSlider = parentWidget->findChild<QSlider*>("batchSlider");
            int oldBatchValue = batchSlider->value();

            int degrees_per_item = 15;
            int scroll_pseudovalue_per_item = 8 * degrees_per_item;

            // -- the input from scroll wheel will go into the accumulator...
            _horizontal_scroll_accumulator += horizontal_angle;
            // ... then, the value in accumulator will influence the diffValue (= the number of batch samples to increment)
            int diffValue = (_horizontal_scroll_accumulator / scroll_pseudovalue_per_item);
            // ... and finally, we will exlude already-used value from accumulator.
            _horizontal_scroll_accumulator -= diffValue * scroll_pseudovalue_per_item;

            int newValue = oldBatchValue + diffValue;
            batchSlider->setValue(newValue);

            qDebug() << "BatchSlider: " << batchSlider << "; " << "Horizontal Angle: " << horizontal_angle
                     << "; oldValue: " << oldBatchValue << "; newValue: " << newValue << "; remaining accumulator value: " << _horizontal_scroll_accumulator
                     << "; Keyboard Modifier: " << keyboard_modifiers;

            return true;
        }

    }
    Q_UNUSED(object)
    return false;
}
