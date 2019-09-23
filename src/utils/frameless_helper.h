#ifndef FRAMELESS_HELPER_H
#define FRAMELESS_HELPER_H

#include <QApplication>
#include <QHoverEvent>
#include <QMouseEvent>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QRubberBand>

class QWidget;
class FramelessHelperPrivate;

class FramelessHelper : public QObject {
    Q_OBJECT

public:
    explicit FramelessHelper(QObject *parent = nullptr);
    ~FramelessHelper();
    // 激活窗体
    void activate_on(QWidget *top_lever_widget);
    // 移除窗体
    void remove_from(QWidget *top_lever_widget);
    // 设置窗体移动
    void set_widget_movable(bool movable);
    // 设置窗体缩放
    void set_widget_esizable(bool resizable);
    // 设置橡皮筋移动
    void set_ubber_band_on_move(bool movable);
    // 设置橡皮筋缩放
    void set_rubber_band_on_resize(bool resizable);
    // 设置边框的宽度
    void set_border_width(uint width);
    // 设置标题栏高度
    void set_title_height(uint height);
    bool widget_resizable();
    bool widget_movable();
    bool rubber_band_on_move();
    bool rubber_band_on_resisze();
    uint border_width();
    uint title_height();

protected:
    // 事件过滤，进行移动、缩放等
    virtual bool eventFilter(QObject *obj, QEvent *event);

private:
    FramelessHelperPrivate *d;
};

/*****
 * CursorPosCalculator
 * 计算鼠标是否位于左、上、右、下、左上角、左下角、右上角、右下角
 *****/
class CursorPosCalculator {
public:
    explicit CursorPosCalculator();
    void reset();
    void recalculate(const QPoint &global_mouse_pos, const QRect &frame_rect);

public:
    bool on_edges_ : true;
    bool on_left_edge_ : true;
    bool on_right_edge_ : true;
    bool on_top_edge_ : true;
    bool on_bottom_edge_ : true;
    bool on_top_left_edge_ : true;
    bool on_bottom_left_edge_ : true;
    bool on_top_right_edge_ : true;
    bool on_bottom_right_edge_ : true;

    static int border_width_;
    static int title_height_;
};

/*****
 * WidgetData
 * 更新鼠标样式、移动窗体、缩放窗体
 *****/
class WidgetData {
public:
    explicit WidgetData(FramelessHelperPrivate *d_, QWidget *ptop_level_widget);
    ~WidgetData();
    QWidget *widget();
    // 处理鼠标事件-划过、按下、释放、移动
    void handle_widget_event(QEvent *event);
    // 更新橡皮筋状态
    void update_rubber_band_status();

private:
    // 更新鼠标样式
    void update_cursor_shape(const QPoint &mouse_pos);
    // 重置窗体大小
    void resize_widget(const QPoint &mouse_pos);
    // 移动窗体
    void move_widget(const QPoint &mouse_pos);
    // 处理鼠标按下
    void handle_mouse_press_event(QMouseEvent *event);
    // 处理鼠标释放
    void handleMouseReleaseEvent(QMouseEvent *event);
    // 处理鼠标移动
    void handleMouseMoveEvent(QMouseEvent *event);
    // 处理鼠标离开
    void handleLeaveEvent(QEvent *event);
    // 处理鼠标进入
    void handleHoverMoveEvent(QHoverEvent *event);

private:
    FramelessHelperPrivate *d_;
    QRubberBand *prubber_band_;
    QWidget *pwidget_;
    QPoint drag_pos_;
    CursorPosCalculator pressed_mouse_pos_;
    CursorPosCalculator move_mouse_pos_;
    bool left_button_pressed_;
    bool cursor_shape_changed_;
    bool left_button_title_pressed_;
    Qt::WindowFlags window_flags_;
};

/*****
 * FramelessHelperPrivate
 * 存储界面对应的数据集合，以及是否可移动、可缩放属性
 *****/
class FramelessHelperPrivate {
public:
    QHash<QWidget *, WidgetData *> widget_data_hash_;
    bool widget_movable_ : true;
    bool widget_resizable_ : true;
    bool rubber_band_on_resize_ : true;
    bool rubber_band_on_move_ : true;
};

#endif // FRAMELESS_HELPER_H
