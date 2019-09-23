#include "widget.h"

Widget::Widget(const QString &title, QWidget *parent)
    : QWidget(parent), action_(nullptr), pvlayout_(new QVBoxLayout(this)), pselect_list_(nullptr), phelper_(nullptr) {
    create_titlebar();
    setWindowTitle(title);
    load_style_sheet(":qdarkstyle/style.qss");
}

Widget::~Widget() {
    if (in_view_ != nullptr) delete in_view_;
    if (in_model_ != nullptr) delete in_model_;
    if (pselect_list_ != nullptr) delete pselect_list_;
}

void Widget::bind_action(QAction *action) {
    action_ = action;
    connect(this, &Widget::sig_close, action_, &QAction::setChecked);
    connect(this, &Widget::sig_view, action_, &QAction::setChecked);
}

void Widget::set_layout(QLayout *layout) { pcentral_window_->setLayout(layout); }

void Widget::set_view(QTableView *view, bool state) {
    if (!state) {
        QVBoxLayout *l = new QVBoxLayout(pcentral_window_);
        pcentral_window_->setLayout(l);
        l->addWidget(view);
        l->setSpacing(0);
        l->setMargin(0);
    }
    in_view_ = view;
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void Widget::set_model(TableModel *model, ElementType type, HeadLocal local) {
    if (in_view_ != nullptr) {
        in_view_->setModel(model);
        in_model_ = model;
        model->set_head_data(type, local);
    }
    widget_type_ = type;
}

void Widget::set_title_visib(bool state) {
    if (!state) {
        ptitlebar_->set_title_visib();
    }
}

QVBoxLayout *Widget::central_layout() { return pvlayout_; }

void Widget::closeEvent(QCloseEvent *e) {
    this->setVisible(false);
    emit sig_close(false);

    Q_UNUSED(e)
}

bool Widget::eventFilter(QObject *o, QEvent *e) {
    if (o == pselect_list_ && e->type() == QEvent::Close) {
        QWidget *w = qobject_cast<QWidget *>(o);
        w->setVisible(false);
        return true;
    }

    return QWidget::eventFilter(o, e);
}

void Widget::create_titlebar() {
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    ptitlebar_ = new TitleBar(this);
    installEventFilter(ptitlebar_);
    pvlayout_->addWidget(ptitlebar_);

    //添加中心窗体
    pcentral_window_ = new QWidget(this);
    pvlayout_->addWidget(pcentral_window_);

    pvlayout_->setSpacing(0);
    pvlayout_->setMargin(0);

    in_select_ = ptitlebar_->get_selec_list();
    connect(in_select_, &QPushButton::clicked, this, &Widget::show_select_list); //
}

void Widget::create_select_list() {
    pselect_list_ = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(pselect_list_);
    QListWidget *list_w = new QListWidget(pselect_list_);
    QStringList *string_list = in_model_->get_row_name(widget_type_);

    // 0.添加筛选功能和全选/全不选
    QHBoxLayout *hbl = new QHBoxLayout(pselect_list_);
    QPushButton *on_check = new QPushButton(tr("全选"), pselect_list_);
    QPushButton *un_check = new QPushButton(tr("全不选"), pselect_list_);
    connect(on_check, &QPushButton::clicked, [=] {
        for (auto var : list_head_checkbox_) {
            var->setCheckState(Qt::CheckState::Checked);
        }
        for (auto var : list_widget_checkbox_) {
            var->setCheckState(Qt::CheckState::Checked);
        }
    });
    connect(un_check, &QPushButton::clicked, [=] {
        for (auto var : list_head_checkbox_) {
            var->setCheckState(Qt::CheckState::Unchecked);
        }
        for (auto var : list_widget_checkbox_) {
            var->setCheckState(Qt::CheckState::Unchecked);
        }
    });

    hbl->addWidget(on_check);
    hbl->addWidget(un_check);
    layout->addLayout(hbl);

    // 1.添加垂直表头
    for (auto s : *string_list) {
        QListWidgetItem *item = new QListWidgetItem(list_w);
        QCheckBox *check_box = new QCheckBox(list_w);
        check_box->setCheckState(Qt::CheckState::Checked);
        list_head_checkbox_.push_back(check_box);
        list_w->addItem(item);
        list_w->setItemWidget(item, check_box);
        check_box->setText(s);
    }

    // 2.添加窗体
    if (!list_sub_widgets_.isEmpty()) {
        for (auto var : list_sub_widgets_) {
            QString s = var->windowTitle();
            QListWidgetItem *item = new QListWidgetItem(list_w);
            QCheckBox *check_box = new QCheckBox(list_w);
            check_box->setCheckState(Qt::CheckState::Checked);
            list_widget_checkbox_.push_back(check_box);
            list_w->addItem(item);
            list_w->setItemWidget(item, check_box);
            check_box->setText(s);
        }
    }

    layout->addWidget(list_w);

    // 3.添加按钮
    hbl = new QHBoxLayout(pselect_list_);
    QPushButton *ok_button = new QPushButton(tr("确认"), pselect_list_);
    QPushButton *cancel_button = new QPushButton(tr("取消"), pselect_list_);
    connect(ok_button, &QPushButton::clicked, this, &Widget::on_setting_visible);
    connect(cancel_button, &QPushButton::clicked, pselect_list_, &QWidget::close);

    hbl->addWidget(ok_button);
    hbl->addWidget(cancel_button);
    layout->addLayout(hbl);

    pselect_list_->setLayout(layout);
    pselect_list_->setWindowModality(Qt::ApplicationModal);
    pselect_list_->setWindowFlag(Qt::WindowStaysOnTopHint);
    pselect_list_->installEventFilter(this);
    pselect_list_->show();
}

void Widget::show_select_list() {
    if (pselect_list_ != nullptr)
        pselect_list_->setVisible(true);
    else
        create_select_list();
}

void Widget::show_sub_widget() {
    if (!list_sub_widgets_.empty()) {
        for (auto var : list_sub_widgets_) {
            var->setVisible(true);
        }
    }
}

void Widget::set_top_from() {
    phelper_ = new FramelessHelper(this);
    phelper_->activate_on(this);
    phelper_->set_title_height(35);
    phelper_->set_widget_movable(true);
    phelper_->set_widget_esizable(true);
    phelper_->set_ubber_band_on_move(true);
    phelper_->set_rubber_band_on_resize(true);
}

bool Widget::load_style_sheet(const QString &path) {
    QFile file(path);
    file.open(QFile::ReadOnly);
    if (!file.isOpen()) return false;

    QString style = this->styleSheet();
    style += QLatin1String(file.readAll());
    this->setStyleSheet(style);
    file.close();
}

void Widget::on_setting_visible() {
    for (int i = 0; i < list_head_checkbox_.size(); i++) {
        if (list_head_checkbox_[i]->checkState() == Qt::CheckState::Checked) {
            in_view_->setRowHidden(i, false);
        } else {
            in_view_->setRowHidden(i, true);
        }
    }

    for (int i = 0; i < list_widget_checkbox_.size(); i++) {
        QWidget *w = list_sub_widgets_[i];
        if (list_widget_checkbox_[i]->checkState() == Qt::CheckState::Checked) {
            if (!w->isVisible()) w->setVisible(true);
        } else {
            if (w->isVisible()) w->setVisible(false);
        }
    }

    pselect_list_->close();
}
