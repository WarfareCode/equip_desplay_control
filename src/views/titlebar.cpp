#include "titlebar.h"

#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPushButton>

#define BTN_W title_height_
#define BTN_H title_height_
#define ICON_W title_height_
#define ICON_H title_height_

#ifdef Q_OS_WIN
#pragma comment(lib, "user32.lib")
#include <qt_windows.h>
#endif

TitleBar::TitleBar(QWidget *parent) : QWidget(parent), title_height_(35) {
    setFixedHeight(title_height_); // 设置窗体固定高度

    init_control();
}

TitleBar::~TitleBar() {
    DELETE_PTR(picon_label_);
    DELETE_PTR(ptitle_label_);
    DELETE_PTR(pselec_list_);
    DELETE_PTR(pminimize_btn_);
    DELETE_PTR(pmaximize_btn_);
    DELETE_PTR(pclose_button_);
}

void TitleBar::set_title_visib() {
    pclose_button_->setVisible(false);
    pminimize_btn_->setVisible(false);
    pmaximize_btn_->setVisible(false);
    pselec_list_->setVisible(false);
}

void TitleBar::init_control() {
    /* 初始化控件 */
    picon_label_ = new QLabel(this);
    ptitle_label_ = new QLabel(this);
    pselec_list_ = new QPushButton(this);
    pminimize_btn_ = new QPushButton(this);
    pmaximize_btn_ = new QPushButton(this);
    pclose_button_ = new QPushButton(this);

    /* 设置标题栏图标控件 */
    picon_label_->setFixedSize(ICON_W, ICON_H); // 设置控件固定大小
    picon_label_->setScaledContents(true);      // 设置标签缩放内容填充

    /* 设置标题栏文本控件 */
    ptitle_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // 将窗口小部件设置为可拉伸
    ptitle_label_->setAlignment(Qt::AlignCenter);

    /* 设置标题栏按钮控件 */
    pselec_list_->setFixedSize(BTN_W, BTN_H);
    pminimize_btn_->setFixedSize(BTN_W, BTN_H);
    pmaximize_btn_->setFixedSize(BTN_W, BTN_H);
    pclose_button_->setFixedSize(BTN_W, BTN_H);

    /* 设置按钮控件图标 */
    pmaximize_btn_->setIcon(QIcon(":qss_icons/rc/window_undock_focus@2x.png"));
    pminimize_btn_->setIcon(QIcon(":qss_icons/rc/window_minimize_focus@2x.png"));
    pclose_button_->setIcon(QIcon(":qss_icons/rc/window_close_focus@2x.png"));

    /* 设置各种对象名称 */
    ptitle_label_->setObjectName("whiteLabel");
    pminimize_btn_->setObjectName("minimizeButton");
    pmaximize_btn_->setObjectName("maximizeButton");
    pclose_button_->setObjectName("closeButton");

    /* 设置标题栏按钮的提示文本 */
    pselec_list_->setToolTip(tr("选项"));
    pminimize_btn_->setToolTip(tr("最小化"));
    pmaximize_btn_->setToolTip(tr("最大化"));
    pclose_button_->setToolTip(tr("关闭"));

    /* 设置控件布局 */
    playout_ = new QHBoxLayout(this);
    playout_->addWidget(picon_label_);
    playout_->addSpacing(5); // 设置布局的间距
    playout_->addWidget(ptitle_label_);
    playout_->addWidget(pselec_list_);
    playout_->addWidget(pminimize_btn_);
    playout_->addWidget(pmaximize_btn_);
    playout_->addWidget(pclose_button_);
    playout_->setSpacing(0);
    playout_->setContentsMargins(5, 0, 5, 0); // 设置布局周围边距
    setLayout(playout_);

    /* 链接按钮点击信号槽 */
    connect(pminimize_btn_, SIGNAL(clicked()), this, SLOT(on_clicked()));
    connect(pmaximize_btn_, SIGNAL(clicked()), this, SLOT(on_clicked()));
    connect(pclose_button_, SIGNAL(clicked()), this, SLOT(on_clicked()));
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_UNUSED(event);

    emit pmaximize_btn_->clicked();
}

// void TitleBar::mousePressEvent( QMouseEvent *event )
//{
//#ifdef Q_OS_WIN
//    if (ReleaseCapture()) {
//        QWidget *pwindow = this->window();
//        if (pwindow->isTopLevel())
//            SendMessage(HWND(pwindow->winId()), \
//			WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
//	}

//	event->ignore();
//#else
//#endif
//}

bool TitleBar::eventFilter(QObject *obj, QEvent *event) {
    switch (event->type()) {
        case QEvent::WindowTitleChange: // 窗体标题发生改变
        {
            QWidget *pwidget = qobject_cast<QWidget *>(obj);
            if (pwidget) { // 保存当前标题
                ptitle_label_->setText(pwidget->windowTitle());
                return true;
            }
        }
        case QEvent::WindowIconChange: // 窗体图标发生改变
        {
            QWidget *pwidget = qobject_cast<QWidget *>(obj);
            if (pwidget) { // 保存当前图标
                QIcon icon = pwidget->windowIcon();
                picon_label_->setPixmap(icon.pixmap(picon_label_->size()));
                return true;
            }
        }
        case QEvent::WindowStateChange: // 当窗口状态发生改变
        case QEvent::Resize:            // 部件大小发生改变
            update_maximize();
            return true;
    }
    return QWidget::eventFilter(obj, event);
}

void TitleBar::on_clicked() {
    QPushButton *pbutton = qobject_cast<QPushButton *>(sender());
    QWidget *pwindow = this->window();
    if (pwindow->isWindow()) {
        if (pbutton == pminimize_btn_) // 最小化
            pwindow->showMinimized();
        else if (pbutton == pmaximize_btn_) //最大化/还原
            pwindow->isMaximized() ? pwindow->showNormal() : pwindow->showMaximized();
        else if (pbutton == pclose_button_) // 关闭
            pwindow->close();
    }
}

void TitleBar::update_maximize() {
    QWidget *pwindow = this->window();
    if (pwindow->isWindow()) {
        bool bmaximize = pwindow->isMaximized();
        if (bmaximize) {
            pmaximize_btn_->setToolTip("还原");
            pmaximize_btn_->setProperty("maximizeProperty", "restore");
        } else {
            pmaximize_btn_->setProperty("maximizeProperty", "maximize");
            pmaximize_btn_->setToolTip("最大化");
        }

        pmaximize_btn_->setStyle(QApplication::style());
    }
}
