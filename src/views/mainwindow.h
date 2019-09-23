#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include<qmath.h>
#include <QCloseEvent>
#include <QHeaderView>
#include <QLabel>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QStatusBar>
#include <QTableWidget>
#include <QVBoxLayout>

#include<qgsmapcanvas.h>

#include "src/models/tablemodel.h"
#include "src/utils/macro.h"
#include "src/views/widget.h"
#include "titlebar.h"

enum Menus {
    MENU_FILE = 0,
    MENU_VIEW,
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = Q_NULLPTR);
    virtual ~MainWindow() override;

protected:
    virtual void closeEvent(QCloseEvent *e) override;
    virtual bool eventFilter(QObject *o, QEvent *e) override;

signals:
    void sig_change_wid_statu(int);

public slots:
    void slot_change_wid_statu(int type);
    void from_arranged(QAction *action);

private:
    void init_window();
	void create_centwindow();
    void create_title();
    void create_menubar();
    void create_toolbar();
    void create_status_bar();
    void load_style_sheet(const QString &path);

    QTableView *create_tablewindow(TableModel *pmodel, bool head_v = true, bool head_h = true,
                                   QWidget *parent = nullptr);

    //浮动窗体创建
    void create_radar_state();
    void create_chain_of_command();
    void create_photoelectricity();
    void create_description(); //创建拦截武器状态数据描述
    void create_firepower();

    void create_data();
private:
    QWidget *pcentral_window_;
    QVBoxLayout *playout_;
    TitleBar *ptitlebar_;
    QStatusBar *pstatus_bar_;

    QString simu_start_time_;
    QString simu_end_time_;
    QString curr_reality_time_;

    QMap<int, Widget *> map_widgets_;
    QMap<int, Widget *> map_parent_widgets_;
    QList<QStringList *> list_headnames_;

    QMap<int, QMenu *> map_menus_;
    QList<QLabel *> list_labels_;

    QAction *action_overlaping_;
    QAction *action_horizontal_;
    QAction *action_vertical_;
	
	//======================================
	QWidget *qgis_w_ = nullptr;
	QList<QgsMapLayer *> layers_;
	QgsMapCanvas *map_canvas_;
};

#endif // MAINWINDOW_H
