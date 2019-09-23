#ifndef __WIDGET_H__
#define __WIDGET_H__

#include <QAction>
#include <QCheckBox>
#include <QCloseEvent>
#include <QEvent>
#include <QFile>
#include <QHeaderView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

#include "src/models/tablemodel.h"
#include "src/utils/frameless_helper.h"
#include "src/views/titlebar.h"

class Widget : public QWidget {
    Q_OBJECT

public:
    Widget(const QString &title = "", QWidget *parent = nullptr);
    virtual ~Widget() override;
    void bind_action(QAction *action);
    void set_layout(QLayout *layout);
    void set_view(QTableView *view, bool state = false);
    void set_model(TableModel *model, ElementType type, HeadLocal local);
    void set_title_visib(bool state);
    void show_sub_widget();
    void set_top_from();
    void add_sub_widget(Widget *w) { list_sub_widgets_.push_back(w); }
    QTableView *get_view() { return in_view_; }
    TableModel *get_model() { return in_model_; }
    QVBoxLayout *central_layout();

protected:
    virtual void closeEvent(QCloseEvent *e) override;
    virtual bool eventFilter(QObject *o, QEvent *e) override;

private slots:
    void create_titlebar();
    void create_select_list();
    void show_select_list();

signals:
    void sig_close(bool);
    void sig_view(bool);

public slots:
    bool load_style_sheet(const QString &path);
    void on_setting_visible();

protected:
    QAction *action_; //菜单栏关联的action

private:
    int widget_type_ = -1;
    TitleBar *ptitlebar_;
    QVBoxLayout *pvlayout_;
    QWidget *pcentral_window_;
    QWidget *pselect_list_;
    FramelessHelper *phelper_;

    TableModel *in_model_;
    QTableView *in_view_;
    QPushButton *in_select_;

    QList<Widget *> list_sub_widgets_;
    QList<QCheckBox *> list_head_checkbox_;
    QList<QCheckBox *> list_widget_checkbox_;
};

#endif // __WIDGET_H__
