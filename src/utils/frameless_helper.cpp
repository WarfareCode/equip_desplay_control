#include "frameless_helper.h"

int CursorPosCalculator::border_width_ = 5;
int CursorPosCalculator::title_height_ = 30;

/***** CursorPosCalculator *****/
CursorPosCalculator::CursorPosCalculator() { reset(); }

void CursorPosCalculator::reset() {
    on_edges_ = false;
    on_left_edge_ = false;
    on_right_edge_ = false;
    on_top_edge_ = false;
    on_bottom_edge_ = false;
    on_top_left_edge_ = false;
    on_bottom_left_edge_ = false;
    on_top_right_edge_ = false;
    on_bottom_right_edge_ = false;
}

void CursorPosCalculator::recalculate(const QPoint &mouse_pos, const QRect &frame_rect) {
    int global_mouse_x = mouse_pos.x();
    int global_mouse_y = mouse_pos.y();

    int frame_x = frame_rect.x();
    int frame_y = frame_rect.y();

    int frame_width = frame_rect.width();
    int frame_height = frame_rect.height();

    on_left_edge_ = (global_mouse_x >= frame_x && global_mouse_x <= frame_x + border_width_);

    on_right_edge_ =
        (global_mouse_x >= frame_x + frame_width - border_width_ && global_mouse_x <= frame_x + frame_width);

    on_top_edge_ = (global_mouse_y >= frame_y && global_mouse_y <= frame_y + border_width_);

    on_bottom_edge_ =
        (global_mouse_y >= frame_y + frame_height - border_width_ && global_mouse_y <= frame_y + frame_height);

    on_top_left_edge_ = on_top_edge_ && on_left_edge_;
    on_bottom_left_edge_ = on_bottom_edge_ && on_left_edge_;
    on_top_right_edge_ = on_top_edge_ && on_right_edge_;
    on_bottom_right_edge_ = on_bottom_edge_ && on_right_edge_;

    on_edges_ = on_left_edge_ || on_right_edge_ || on_top_edge_ || on_bottom_edge_;
}

/***** WidgetData *****/
WidgetData::WidgetData(FramelessHelperPrivate *d, QWidget *ptop_level_widget) {
    d_ = d;
    pwidget_ = ptop_level_widget;
    left_button_pressed_ = false;
    cursor_shape_changed_ = false;
    left_button_title_pressed_ = false;
    prubber_band_ = nullptr;

    window_flags_ = pwidget_->windowFlags();
    pwidget_->setMouseTracking(true);
    pwidget_->setAttribute(Qt::WA_Hover, true);

    update_rubber_band_status();
}

WidgetData::~WidgetData() {
    pwidget_->setMouseTracking(false);
    pwidget_->setWindowFlags(window_flags_);
    pwidget_->setAttribute(Qt::WA_Hover, false);

    delete prubber_band_;
    prubber_band_ = nullptr;
}

QWidget *WidgetData::widget() { return pwidget_; }

void WidgetData::handle_widget_event(QEvent *event) {
    switch (event->type()) {
        default:
            break;
        case QEvent::MouseButtonPress:
            handle_mouse_press_event(static_cast<QMouseEvent *>(event));
            break;
        case QEvent::MouseButtonRelease:
            handleMouseReleaseEvent(static_cast<QMouseEvent *>(event));
            break;
        case QEvent::MouseMove:
            handleMouseMoveEvent(static_cast<QMouseEvent *>(event));
            break;
        case QEvent::Leave:
            handleLeaveEvent(static_cast<QMouseEvent *>(event));
            break;
        case QEvent::HoverMove:
            handleHoverMoveEvent(static_cast<QHoverEvent *>(event));
            break;
    }
}

void WidgetData::update_rubber_band_status() {
    if (d_->rubber_band_on_move_ || d_->rubber_band_on_resize_) {
        if (nullptr == prubber_band_) prubber_band_ = new QRubberBand(QRubberBand::Rectangle);
    } else {
        delete prubber_band_;
        prubber_band_ = nullptr;
    }
}

void WidgetData::update_cursor_shape(const QPoint &mouse_pos) {
    if (pwidget_->isFullScreen() || pwidget_->isMaximized()) {
        if (cursor_shape_changed_) {
            pwidget_->unsetCursor();
        }
        return;
    }

    move_mouse_pos_.recalculate(mouse_pos, pwidget_->frameGeometry());

    if (move_mouse_pos_.on_top_left_edge_ || move_mouse_pos_.on_bottom_right_edge_) {
        pwidget_->setCursor(Qt::SizeFDiagCursor);
        cursor_shape_changed_ = true;
    } else if (move_mouse_pos_.on_top_right_edge_ || move_mouse_pos_.on_bottom_left_edge_) {
        pwidget_->setCursor(Qt::SizeBDiagCursor);
        cursor_shape_changed_ = true;
    } else if (move_mouse_pos_.on_left_edge_ || move_mouse_pos_.on_right_edge_) {
        pwidget_->setCursor(Qt::SizeHorCursor);
        cursor_shape_changed_ = true;
    } else if (move_mouse_pos_.on_top_edge_ || move_mouse_pos_.on_bottom_edge_) {
        pwidget_->setCursor(Qt::SizeVerCursor);
        cursor_shape_changed_ = true;
    } else {
        if (cursor_shape_changed_) {
            pwidget_->unsetCursor();
            cursor_shape_changed_ = false;
        }
    }
}

void WidgetData::resize_widget(const QPoint &mouse_pos) {
    QRect orig_rect;

    if (d_->rubber_band_on_resize_)
        orig_rect = prubber_band_->frameGeometry();
    else
        orig_rect = pwidget_->frameGeometry();

    int left = orig_rect.left();
    int top = orig_rect.top();
    int right = orig_rect.right();
    int bottom = orig_rect.bottom();
    orig_rect.getCoords(&left, &top, &right, &bottom);

    int min_width = pwidget_->minimumWidth();
    int min_height = pwidget_->minimumHeight();

    if (pressed_mouse_pos_.on_top_left_edge_) {
        left = mouse_pos.x();
        top = mouse_pos.y();
    } else if (pressed_mouse_pos_.on_bottom_left_edge_) {
        left = mouse_pos.x();
        bottom = mouse_pos.y();
    } else if (pressed_mouse_pos_.on_top_right_edge_) {
        right = mouse_pos.x();
        top = mouse_pos.y();
    } else if (pressed_mouse_pos_.on_bottom_right_edge_) {
        right = mouse_pos.x();
        bottom = mouse_pos.y();
    } else if (pressed_mouse_pos_.on_left_edge_) {
        left = mouse_pos.x();
    } else if (pressed_mouse_pos_.on_right_edge_) {
        right = mouse_pos.x();
    } else if (pressed_mouse_pos_.on_top_edge_) {
        top = mouse_pos.y();
    } else if (pressed_mouse_pos_.on_bottom_edge_) {
        bottom = mouse_pos.y();
    }

    QRect newRect(QPoint(left, top), QPoint(right, bottom));

    if (newRect.isValid()) {
        if (min_width > newRect.width()) {
            if (left != orig_rect.left())
                newRect.setLeft(orig_rect.left());
            else
                newRect.setRight(orig_rect.right());
        }
        if (min_height > newRect.height()) {
            if (top != orig_rect.top())
                newRect.setTop(orig_rect.top());
            else
                newRect.setBottom(orig_rect.bottom());
        }

        if (d_->rubber_band_on_resize_) {
            prubber_band_->setGeometry(newRect);
        } else {
            pwidget_->setGeometry(newRect);
        }
    }
}

void WidgetData::move_widget(const QPoint &mouse_pos) {
    if (d_->rubber_band_on_move_) {
        prubber_band_->move(mouse_pos - drag_pos_);
    } else {
        pwidget_->move(mouse_pos - drag_pos_);
    }
}

void WidgetData::handle_mouse_press_event(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        left_button_pressed_ = true;
        left_button_title_pressed_ = event->pos().y() < move_mouse_pos_.title_height_;

        QRect frameRect = pwidget_->frameGeometry();
        pressed_mouse_pos_.recalculate(event->globalPos(), frameRect);

        drag_pos_ = event->globalPos() - frameRect.topLeft();

        if (pressed_mouse_pos_.on_edges_) {
            if (d_->rubber_band_on_resize_) {
                prubber_band_->setGeometry(frameRect);
                prubber_band_->show();
            }
        } else if (d_->rubber_band_on_move_) {
            prubber_band_->setGeometry(frameRect);
            prubber_band_->show();
        }
    }
}

void WidgetData::handleMouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        left_button_pressed_ = false;
        left_button_title_pressed_ = false;
        pressed_mouse_pos_.reset();
        if (prubber_band_ && prubber_band_->isVisible()) {
            prubber_band_->hide();
            pwidget_->setGeometry(prubber_band_->geometry());
        }
    }
}

void WidgetData::handleMouseMoveEvent(QMouseEvent *event) {
    if (left_button_pressed_) {
        if (d_->widget_resizable_ && pressed_mouse_pos_.on_edges_) {
            resize_widget(event->globalPos());
        } else if (d_->widget_movable_ && left_button_title_pressed_) {
            move_widget(event->globalPos());
        }
    } else if (d_->widget_resizable_) {
        update_cursor_shape(event->globalPos());
    }
}

void WidgetData::handleLeaveEvent(QEvent *event) {
    Q_UNUSED(event)
    if (!left_button_pressed_) {
        pwidget_->unsetCursor();
    }
}

void WidgetData::handleHoverMoveEvent(QHoverEvent *event) {
    if (d_->widget_resizable_) {
        update_cursor_shape(pwidget_->mapToGlobal(event->pos()));
    }
}

class WidgetData;

/*****FramelessHelper*****/
FramelessHelper::FramelessHelper(QObject *parent) : QObject(parent), d(new FramelessHelperPrivate()) {
    d->widget_movable_ = true;
    d->widget_resizable_ = true;
    d->rubber_band_on_resize_ = false;
    d->rubber_band_on_move_ = false;
}

FramelessHelper::~FramelessHelper() {
    QList<QWidget *> keys = d->widget_data_hash_.keys();
    int size = keys.size();
    for (int i = 0; i < size; ++i) {
        delete d->widget_data_hash_.take(keys[i]);
    }

    delete d;
}

bool FramelessHelper::eventFilter(QObject *obj, QEvent *event) {
    switch (event->type()) {
        case QEvent::MouseMove:
        case QEvent::HoverMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::Leave: {
            WidgetData *data = d->widget_data_hash_.value(static_cast<QWidget *>(obj));
            if (data) {
                data->handle_widget_event(event);
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void FramelessHelper::activate_on(QWidget *top_level_widget) {
    if (!d->widget_data_hash_.contains(top_level_widget)) {
        WidgetData *data = new WidgetData(d, top_level_widget);
        d->widget_data_hash_.insert(top_level_widget, data);

        top_level_widget->installEventFilter(this);
    }
}

void FramelessHelper::remove_from(QWidget *top_level_idget) {
    WidgetData *data = d->widget_data_hash_.take(top_level_idget);
    if (data) {
        top_level_idget->removeEventFilter(this);
        delete data;
    }
}

void FramelessHelper::set_ubber_band_on_move(bool movable) {
    d->rubber_band_on_move_ = movable;
    QList<WidgetData *> list = d->widget_data_hash_.values();
    foreach (WidgetData *data, list) { data->update_rubber_band_status(); }
}

void FramelessHelper::set_widget_movable(bool movable) { d->widget_movable_ = movable; }

void FramelessHelper::set_widget_esizable(bool resizable) { d->widget_resizable_ = resizable; }

void FramelessHelper::set_rubber_band_on_resize(bool resizable) {
    d->rubber_band_on_resize_ = resizable;
    QList<WidgetData *> list = d->widget_data_hash_.values();
    foreach (WidgetData *data, list) { data->update_rubber_band_status(); }
}

void FramelessHelper::set_border_width(uint width) {
    if (width > 0) {
        CursorPosCalculator::border_width_ = width;
    }
}

void FramelessHelper::set_title_height(uint height) {
    if (height > 0) {
        CursorPosCalculator::title_height_ = height;
    }
}

bool FramelessHelper::widget_movable() { return d->widget_movable_; }

bool FramelessHelper::widget_resizable() { return d->widget_resizable_; }

bool FramelessHelper::rubber_band_on_move() { return d->rubber_band_on_move_; }

bool FramelessHelper::rubber_band_on_resisze() { return d->rubber_band_on_resize_; }

uint FramelessHelper::border_width() { return CursorPosCalculator::border_width_; }

uint FramelessHelper::title_height() { return CursorPosCalculator::title_height_; }
