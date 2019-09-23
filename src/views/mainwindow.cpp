#include "mainwindow.h"
//===================
#include <qfiledialog.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsmaptoolpan.h>
#include <qgsmaptoolzoom.h>
#include <qgsrenderer.h>
#include <qgssymbollayer.h>
#include <qgssymbol.h>
#include <qgsrasterlayer.h>
#include <qgsproject.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) { init_window(); }

MainWindow::~MainWindow() {
    for (auto var : map_widgets_) {
        delete var;
    }

    for (auto var : map_menus_) delete var;

    for (auto var : list_headnames_) delete var;

    DELETE_PTR(pcentral_window_);
    DELETE_PTR(playout_);
    DELETE_PTR(ptitlebar_);
}

void MainWindow::closeEvent(QCloseEvent *e) {
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(500);
    animation->setStartValue(1);
    animation->setEndValue(0.7);
    animation->start();

    switch (QMessageBox::information(this, tr("提示"), tr("是否关闭？"), tr("确认"), tr("返回"), Q_NULLPTR, 1)) {
        case 0: {
            e->accept();
            exit(0);
        }
        case 1:
        default: {
            animation->setDuration(500);
            animation->setStartValue(0.5);
            animation->setEndValue(1);
            animation->start();
            e->ignore();
            break;
        }
    }
}

bool MainWindow::eventFilter(QObject *o, QEvent *e) { return QWidget::eventFilter(o, e); }

void MainWindow::slot_change_wid_statu(int type) {
    auto var = map_widgets_[type];
    bool statu = var->isVisible();

    if (var == Q_NULLPTR) return;

    if (statu == true) {
        var->setVisible(false);
    } else {
        var->setVisible(true);
        var->show_sub_widget();
    }
}

void MainWindow::from_arranged(QAction *action) {
    int h = this->height(), w = this->width();
    int count = 0;

    if (action == action_overlaping_) {
        for (auto var : map_parent_widgets_) {
            if (var->isVisible()) {
                var->setGeometry(w / 3 + count, h / 3 + count, var->width(), var->height());
                count += 30;
            }
        }
    } else if (action == action_horizontal_) {
        for (auto var : map_parent_widgets_) {
            if (var->isVisible()) {
                var->setGeometry(0 + count, h / 3, var->width(), var->height());
                count += var->width() + 10;
            }
        }
    } else if (action == action_vertical_) {
        for (auto var : map_parent_widgets_) {
            if (var->isVisible()) {
                var->setGeometry(0, 0 + count, var->width(), var->height());
                count += var->height();
            }
        }
    }
}

void MainWindow::init_window() {
    //初始化控件
    pcentral_window_ = new QWidget(this);
    this->setCentralWidget(pcentral_window_);
    playout_ = new QVBoxLayout(pcentral_window_);
	
    //创建sub窗体
    create_radar_state();
    create_chain_of_command();
    create_photoelectricity();
    create_description();
    create_firepower();

    //创建自定义标题栏
    create_title();
    create_menubar();
    create_toolbar();
    load_style_sheet(":qdarkstyle/style.qss");

	//创建二维地图主窗体
	create_centwindow();

	playout_->setSpacing(0);
    pcentral_window_->setLayout(playout_);

    //添加状态栏
    create_status_bar();

    //添加数据
    create_data();

    //默认隐藏所有窗体
    for (auto var : map_widgets_) {
        var->setVisible(false);
    }
}

void MainWindow::create_centwindow() {
	//qgis_w_ = new QWidget();
	//playout_->addWidget(qgis_w_, 3, 0, 47, 1);
	map_canvas_ = new QgsMapCanvas();
	playout_->addWidget(map_canvas_);
	map_canvas_->enableAntiAliasing(true);
	map_canvas_->setCanvasColor(QColor(0, 0, 0));
	map_canvas_->setAcceptDrops(true);
	map_canvas_->setMouseTracking(true);
	map_canvas_->setMapTool(new QgsMapToolPan(map_canvas_));
	//map_canvas_->setMinimumSize(QSize(1920, 1080));

	QString fileName = "tmsforuser.xml";
	QStringList temp = fileName.split('/');
	QString basename = temp.at(temp.size() - 1);

	QgsRasterLayer *rasterLayser = new QgsRasterLayer(fileName, basename, "gdal");

	if (!rasterLayser->isValid())
	{
		QMessageBox::critical(this, "error", QStringLiteral("图层无效: \n") + fileName);
		return;
	}

	QgsProject::instance()->addMapLayer(rasterLayser);
	//渲染线条;
	map_canvas_->setExtent(rasterLayser->extent());//设置区域
	layers_.append(rasterLayser);//装载图层
	map_canvas_->setLayers(layers_);//设置图层集合
	map_canvas_->zoomToFullExtent();//全屏展示

	map_canvas_->setVisible(true);
	map_canvas_->refresh();//更新画布	
}

void MainWindow::create_title() {
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    ptitlebar_ = new TitleBar(this);
    installEventFilter(ptitlebar_);
    this->setWindowIcon(QIcon(":qss_icons/rc/LOGO.png"));
    this->setWindowTitle(tr("装备显控软件"));
    playout_->addWidget(ptitlebar_);
}

void MainWindow::create_menubar() {
    QMenuBar *menubar = this->menuBar();
    playout_->addWidget(reinterpret_cast<QWidget *>(menubar));

    //创建文件菜单项
    QMenu *menu = menubar->addMenu(tr("文件"));
    map_menus_.insert(MENU_FILE, menu);
    QAction *action = menu->addAction(tr("打开文件"));

    //创建视图菜单项
    menu = menubar->addMenu(tr("视图"));
    map_menus_.insert(MENU_VIEW, menu);

    action = menu->addAction(QIcon(":qss_icons/rc/window_close_focus@2x.png"), tr("关闭所有"));
    connect(action, &QAction::triggered, [=] {
        for (auto var : map_widgets_) var->close();
    });
    action = menu->addAction(QIcon(":qss_icons/rc/window_undock_focus@2x.png"), tr("打开所有"));
    connect(action, &QAction::triggered, [=] {
        for (auto var : map_widgets_) {
            var->setVisible(true);
            emit var->sig_view(true);
        }
    });
    menu->addSection("on_off");

    action_overlaping_ = menu->addAction(tr("重叠"));
    action_horizontal_ = menu->addAction(tr("水平平铺"));
    action_vertical_ = menu->addAction(tr("垂直平铺"));
    menu->addSection("view");
    connect(menu, &QMenu::triggered, this, &MainWindow::from_arranged);

    action = menu->addAction(tr("雷达系统状态显示"));
    action->setCheckable(true);
    action->setChecked(false);
    Widget *w = map_widgets_[SYSTEM_STATE];
    w->bind_action(action);
    connect(action, &QAction::triggered, [=] { emit sig_change_wid_statu(SYSTEM_STATE); });

    action = menu->addAction(tr("指控系统装备状态显示"));
    action->setCheckable(true);
    action->setChecked(false);
    w = map_widgets_[CHAIN_OF_COMMAND];
    w->bind_action(action);
    connect(action, &QAction::triggered, [=] { emit sig_change_wid_statu(CHAIN_OF_COMMAND); });

    action = menu->addAction(tr("光电装备状态显示"));
    action->setCheckable(true);
    action->setChecked(false);
    w = map_widgets_[PHOTOELECTRICITY_EQUIPMENT];
    w->bind_action(action);
    connect(action, &QAction::triggered, [=] { emit sig_change_wid_statu(PHOTOELECTRICITY_EQUIPMENT); });

    action = menu->addAction(tr("拦截武器显示"));
    action->setCheckable(true);
    action->setChecked(false);
    w = map_widgets_[DESCRIPTION_OF_INTERCEPTOR_WEAPON];
    w->bind_action(action);
    connect(action, &QAction::triggered, [=] { emit sig_change_wid_statu(DESCRIPTION_OF_INTERCEPTOR_WEAPON); });

    action = menu->addAction(tr("火力单元状态显示"));
    action->setCheckable(true);
    action->setChecked(false);
    w = map_widgets_[FIREPOWER_UNIT];
    w->bind_action(action);
    connect(action, &QAction::triggered, [=] { emit sig_change_wid_statu(FIREPOWER_UNIT); });

    connect(this, &MainWindow::sig_change_wid_statu, this, &MainWindow::slot_change_wid_statu);
}

void MainWindow::create_toolbar() {}

void MainWindow::create_status_bar() {
    pstatus_bar_ = this->statusBar();
    simu_start_time_ = tr("仿真开始时间:");
    simu_end_time_ = tr("仿真结束时间:");
    curr_reality_time_ = tr("真实时间:");

    QLabel *l = new QLabel(simu_start_time_, pstatus_bar_);
    pstatus_bar_->addWidget(l);
    l = new QLabel(simu_end_time_, pstatus_bar_);
    pstatus_bar_->addWidget(l);
    l = new QLabel(curr_reality_time_, pstatus_bar_);
    pstatus_bar_->addWidget(l);

    playout_->addWidget(pstatus_bar_);
}

void MainWindow::load_style_sheet(const QString &path) {
    QFile file(path);
    file.open(QFile::ReadOnly);
    if (!file.isOpen()) return;

    QString styleSheet = this->styleSheet();
    styleSheet += QLatin1String(file.readAll());
    this->setStyleSheet(styleSheet);
    file.close();
}

QTableView *MainWindow::create_tablewindow(TableModel *pmodel, bool head_v, bool head_h, QWidget *parent) {
    QTableView *view = new QTableView(parent);
    view->installEventFilter(this);

    if (!head_h) {
        view->horizontalHeader()->setVisible(false);
    }
    if (!head_v) {
        view->verticalHeader()->setVisible(false);
    }

    view->setModel(pmodel);

    return view;
}

void MainWindow::create_radar_state() {
    Widget *w = new Widget(tr("雷达系统状态显示"));
    QVBoxLayout *vblayout = new QVBoxLayout(w);
    vblayout->setSpacing(0);
    w->set_layout(vblayout);
    w->setWindowFlag(Qt::WindowStaysOnTopHint);
    w->set_top_from();

    TableModel *model = Q_NULLPTR;
    QTableView *view = Q_NULLPTR;

    //创建雷达系统状态显示窗体
    model = new TableModel();
    view = create_tablewindow(model, true, false, w);
    w->set_view(view, true);
    w->set_model(model, SYSTEM_STATE, VERTICAL_HEAD);
    vblayout->addWidget(view);
    vblayout->setStretchFactor(view, 5);
    map_widgets_.insert(SYSTEM_STATE, w);
    map_parent_widgets_.insert(SYSTEM_STATE, w);

    QHBoxLayout *hblayout = new QHBoxLayout(w);
    Widget *sub_w = nullptr;
    //创建工作模式窗体
    sub_w = new Widget(tr("工作模式"));
    model = new TableModel();
    view = create_tablewindow(model, false, true);
    sub_w->set_title_visib(false);
    sub_w->set_view(view);
    sub_w->set_model(model, WORK_PATTERN, HORIZONTAL_HEAD);
    hblayout->addWidget(sub_w);
    w->add_sub_widget(sub_w);
    map_widgets_.insert(WORK_PATTERN, sub_w);

    //创建辐射状态窗体
    sub_w = new Widget(tr("辐射状态"));
    model = new TableModel();
    view = create_tablewindow(model, false, true);
    sub_w->set_title_visib(false);
    sub_w->set_view(view);
    sub_w->set_model(model, RADIATION_STATE, HORIZONTAL_HEAD);
    hblayout->addWidget(sub_w);
    w->add_sub_widget(sub_w);
    map_widgets_.insert(RADIATION_STATE, sub_w);
    vblayout->addLayout(hblayout);

    hblayout = new QHBoxLayout(w);
    //创建工作频点窗体
    sub_w = new Widget(tr("工作频点"));
    model = new TableModel();
    view = create_tablewindow(model, false, true);
    sub_w->set_title_visib(false);
    sub_w->set_view(view);
    sub_w->set_model(model, WORK_FREQUENCY, HORIZONTAL_HEAD);
    hblayout->addWidget(sub_w);
    w->add_sub_widget(sub_w);
    map_widgets_.insert(WORK_FREQUENCY, sub_w);

    //创建有源干扰方向窗体
    sub_w = new Widget(tr("有源干扰方向"));
    model = new TableModel();
    view = create_tablewindow(model, false, true);
    sub_w->set_title_visib(false);
    sub_w->set_view(view);
    sub_w->set_model(model, DISTURB_DIRECTION, HORIZONTAL_HEAD);
    hblayout->addWidget(sub_w);
    w->add_sub_widget(sub_w);
    map_widgets_.insert(DISTURB_DIRECTION, sub_w);
    vblayout->addLayout(hblayout);

    w->show();
}

void MainWindow::create_chain_of_command() {
    Widget *w = new Widget(tr("指控系统装备状态显示"));
    TableModel *model = new TableModel();
    QTableView *view = create_tablewindow(model, true, false);
    w->setWindowFlag(Qt::WindowStaysOnTopHint);
    w->set_top_from();
    w->set_view(view);
    w->set_model(model, CHAIN_OF_COMMAND, VERTICAL_HEAD);

    w->show();
    map_widgets_.insert(CHAIN_OF_COMMAND, w);
    map_parent_widgets_.insert(CHAIN_OF_COMMAND, w);
}

void MainWindow::create_photoelectricity() {
    Widget *w = new Widget(tr("光电装备状态显示"));
    TableModel *model = new TableModel();
    QTableView *view = create_tablewindow(model, true, false);
    w->setWindowFlag(Qt::WindowStaysOnTopHint);
    w->set_top_from();
    w->set_view(view);
    w->set_model(model, PHOTOELECTRICITY_EQUIPMENT, VERTICAL_HEAD);

    w->show();
    map_widgets_.insert(PHOTOELECTRICITY_EQUIPMENT, w);
    map_parent_widgets_.insert(PHOTOELECTRICITY_EQUIPMENT, w);
}

void MainWindow::create_description() {
    Widget *w = new Widget(tr("拦截武器显示"));
    QVBoxLayout *vblayout = new QVBoxLayout(w);
    vblayout->setSpacing(0);
    w->set_layout(vblayout);
    w->setWindowFlag(Qt::WindowStaysOnTopHint);
    w->set_top_from();

    TableModel *model = Q_NULLPTR;
    QTableView *view = Q_NULLPTR;

    //创建拦截武器显示
    model = new TableModel();
    view = create_tablewindow(model, true, false, w);
    w->set_view(view, true);
    w->set_model(model, DESCRIPTION_OF_INTERCEPTOR_WEAPON, VERTICAL_HEAD);
    vblayout->addWidget(view);
    vblayout->setStretchFactor(view, 5);
    map_widgets_.insert(DESCRIPTION_OF_INTERCEPTOR_WEAPON, w);
    map_parent_widgets_.insert(DESCRIPTION_OF_INTERCEPTOR_WEAPON, w);

    Widget *sub_w = nullptr;
    //创建拦截弹资源
    sub_w = new Widget(tr("拦截弹资源"));
    model = new TableModel();
    view = create_tablewindow(model, false, true);
    sub_w->set_title_visib(false);
    sub_w->set_view(view);
    sub_w->set_model(model, GBI_RESOURCES, HORIZONTAL_HEAD);
    vblayout->addWidget(sub_w);
    w->add_sub_widget(sub_w);
    map_widgets_.insert(GBI_RESOURCES, sub_w);

    //创建制导雷达
    sub_w = new Widget(tr("制导雷达"));
    model = new TableModel();
    view = create_tablewindow(model, false, true);
    sub_w->set_title_visib(false);
    sub_w->set_view(view);
    sub_w->set_model(model, GUIDANCE_RADAR, HORIZONTAL_HEAD);
    vblayout->addWidget(sub_w);
    w->add_sub_widget(sub_w);
    map_widgets_.insert(GUIDANCE_RADAR, sub_w);

    w->show();
}

void MainWindow::create_firepower() {
    Widget *w = new Widget(tr("火力单元状态显示"));
    QVBoxLayout *vblayout = new QVBoxLayout(w);
    vblayout->setSpacing(0);
    w->set_layout(vblayout);
    w->setWindowFlag(Qt::WindowStaysOnTopHint);
    w->set_top_from();

    TableModel *model = Q_NULLPTR;
    QTableView *view = Q_NULLPTR;

    //创建火力单元状态显示
    model = new TableModel();
    view = create_tablewindow(model, true, false, w);
    w->set_view(view, true);
    w->set_model(model, FIREPOWER_UNIT, VERTICAL_HEAD);
    vblayout->addWidget(view);
    vblayout->setStretchFactor(view, 5);
    map_widgets_.insert(FIREPOWER_UNIT, w);
    map_parent_widgets_.insert(FIREPOWER_UNIT, w);

    Widget *sub_w = nullptr;
    //创建火力单元通道状态
    sub_w = new Widget(tr("火力单元通道状态"));
    model = new TableModel();
    view = create_tablewindow(model, false, true);
    sub_w->set_title_visib(false);
    sub_w->set_view(view);
    sub_w->set_model(model, FIREPOWER_UNIT_AISLE, HORIZONTAL_HEAD);
    vblayout->addWidget(sub_w);
    w->add_sub_widget(sub_w);
    map_widgets_.insert(FIREPOWER_UNIT_AISLE, sub_w);

    w->show();
}

void MainWindow::create_data() {
    TableModel *model = Q_NULLPTR;

    //系统状态显示
    SystemState *sys = new SystemState;
    sys->power_off = 0;
    sys->control_state = 0;
    sys->scanning_mode = 0;

    model = map_widgets_[SYSTEM_STATE]->get_model();
    model->add_data(sys, SYSTEM_STATE);

    //工作模式窗体
    WorkPattern *workpat = new WorkPattern;
    workpat->id = 1;
    workpat->start_yaw = 0;
    workpat->end_yaw = 10;

    model = map_widgets_[WORK_PATTERN]->get_model();
    model->add_data(workpat, WORK_PATTERN);

    //辐射状态
    RadiationState *radiat = new RadiationState;
    radiat->equipment_id = 0;
    radiat->radiation_state = 0;

    model = map_widgets_[RADIATION_STATE]->get_model();
    model->add_data(radiat, RADIATION_STATE);

    //工作频点
    WorkFrequency *work_freq = new WorkFrequency;
    work_freq->id = 0;
    work_freq->frequency_point = 0;

    model = map_widgets_[WORK_FREQUENCY]->get_model();
    model->add_data(work_freq, WORK_FREQUENCY);

    //有源干扰方向
    DisturbDirection *disturb = new DisturbDirection;
    disturb->id = 0;
    disturb->eleva_angle = 0;
    disturb->pitch = 0;
    disturb->power = 0;

    model = map_widgets_[DISTURB_DIRECTION]->get_model();
    model->add_data(disturb, DISTURB_DIRECTION);

    //指挥系统状态数据描述
    ChainOfCommand *chain = new ChainOfCommand;
    chain->work_state = 0;
    chain->war_preparedness_lv = 0;
    chain->equip_state = 0;
    chain->combat_permissions = 0;
    chain->command_mode = 0;

    model = map_widgets_[CHAIN_OF_COMMAND]->get_model();
    model->add_data(chain, CHAIN_OF_COMMAND);

    //光电装备状态显示
    PhotoelectricityEquipment *photo = new PhotoelectricityEquipment;
    photo->id = 0;
    photo->lon = 0;
    photo->lat = 0;
    photo->alt = 0;
    photo->elevation_angle = 0;
    photo->pitch_angle = 0;
    photo->trace_status = 0;

    model = map_widgets_[PHOTOELECTRICITY_EQUIPMENT]->get_model();
    model->add_data(photo, PHOTOELECTRICITY_EQUIPMENT);

    //拦截武器显示
    DescriptionOfInterceptorWeapon *doiw = new DescriptionOfInterceptorWeapon;
    doiw->status = 0;
    doiw->war_readiness_lv = 0;
    doiw->operational_authority = 0;
    doiw->command_mode = 0;
    doiw->app_mode = 0;
    doiw->run_status = 0;
    model = map_widgets_[DESCRIPTION_OF_INTERCEPTOR_WEAPON]->get_model();
    model->add_data(doiw, DESCRIPTION_OF_INTERCEPTOR_WEAPON);

    //拦截弹资源
    GBIResources *gbi = new GBIResources;
    gbi->id = 0;
    gbi->bullet_quantity = 0;

    model = map_widgets_[GBI_RESOURCES]->get_model();
    model->add_data(gbi, GBI_RESOURCES);

    //制导雷达
    GuidanceRadar *gui = new GuidanceRadar;
    gui->id = 0;
    gui->res_occu_rate = 0;

    model = map_widgets_[GUIDANCE_RADAR]->get_model();
    model->add_data(gui, GUIDANCE_RADAR);

    //火力单元状态显示
    FirepowerUnit *fir = new FirepowerUnit;
    fir->lon = 0;
    fir->command_mode = 0;
    fir->oper_task = 0;
    fir->inter_ception_mode = 0;
    fir->frequency_point_id = 0;
    fir->sector_central_angle = 0;

    model = map_widgets_[FIREPOWER_UNIT]->get_model();
    model->add_data(fir, FIREPOWER_UNIT);

    //火力单元通道状态数据描述
    FirepowerUnitAisle *fir_ais = new FirepowerUnitAisle;
    fir_ais->unit_id = 0;
    fir_ais->target_id = 0;
    fir_ais->status = 0;

    model = map_widgets_[FIREPOWER_UNIT_AISLE]->get_model();
    model->add_data(fir_ais, FIREPOWER_UNIT_AISLE);

    for (auto var : map_widgets_) {
        var->update();
        TableModel *model = var->get_model();
        if (model != nullptr) model->update();
    }
}
