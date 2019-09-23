#include "tablemodel.h"

TableModel::TableModel(QObject *parent) {
    if (!init_text_data()) exit(0);
}

TableModel::~TableModel() {
    if (Q_NULLPTR != phor_head_data_) delete phor_head_data_;

    for (auto var : map_headnames_) {
        delete var;
    }

    for (auto var : map_variants_) {
        delete var;
    }

    for (auto var : map_para_choose_) {
        delete var;
    }
}

#define STRING_PUSH(var)      \
    {                         \
        val0.append(var);     \
        val1.push_back(val0); \
        val0.clear();         \
    }
void TableModel::add_data(void *pdata, ElementType type) {
    QVariant *pvar = Q_NULLPTR;
    QVariantList val0;
    QVariantList val1;

    switch (type) {
        case SYSTEM_STATE: {
            SystemState *data = static_cast<SystemState *>(pdata);

            if (map_para_choose_.contains("power_off")) {
                auto var = map_para_choose_.find("power_off");
                STRING_PUSH(var.value()->at(data->power_off))
            } else {
                STRING_PUSH("")
            }

            //受控状态
            if (map_para_choose_.contains("control_status")) {
                auto var = map_para_choose_.find("control_status");
                STRING_PUSH(var.value()->at(data->control_state))
            } else {
                STRING_PUSH("")
            }

            //天线扫描方式
            if (map_para_choose_.contains("scanning_mode")) {
                auto var = map_para_choose_.find("scanning_mode");
                STRING_PUSH(var.value()->at(data->scanning_mode))
            } else {
                STRING_PUSH("")
            }

            //其他
            STRING_PUSH(data->antenna_eleva_angle)
            STRING_PUSH(data->beam_eleva_angle)
            STRING_PUSH(data->eccm_measures)
            STRING_PUSH(data->clutter_map)
            STRING_PUSH(data->emi_intensity)
            STRING_PUSH(data->time_alloca_state)
            STRING_PUSH(data->track_data_rate)

            if (!map_variants_.contains(SYSTEM_STATE)) {
                pvar = new QVariant();
                map_variants_.insert(SYSTEM_STATE, pvar);
            } else {
                pvar = map_variants_[SYSTEM_STATE];
            }
            pvar->setValue(val1);
            break;
        }
        case WORK_PATTERN: {
            WorkPattern *data = static_cast<WorkPattern *>(pdata);

            if (!map_variants_.contains(WORK_PATTERN)) {
                pvar = new QVariant();
                map_variants_.insert(WORK_PATTERN, pvar);
            } else {
                pvar = map_variants_[WORK_PATTERN];
            }

            val0.append(data->id);
            val0.append(data->start_yaw);
            val0.append(data->end_yaw);
            val1.push_back(val0);

            pvar->setValue(val1);
            break;
        }
        case RADIATION_STATE: {
            RadiationState *data = static_cast<RadiationState *>(pdata);

            if (!map_variants_.contains(RADIATION_STATE)) {
                pvar = new QVariant();
                map_variants_.insert(RADIATION_STATE, pvar);
            } else {
                pvar = map_variants_[RADIATION_STATE];
            }

            val0.append(data->equipment_id);
            if (map_para_choose_.contains("radiation_state")) {
                auto var = map_para_choose_.find("radiation_state");
                val0.append(var.value()->at(data->radiation_state));
            } else {
                val0.append("");
            }
            val1.push_back(val0);

            pvar->setValue(val1);
            break;
        }
        case WORK_FREQUENCY: {
            WorkFrequency *data = static_cast<WorkFrequency *>(pdata);

            if (!map_variants_.contains(WORK_FREQUENCY)) {
                pvar = new QVariant();
                map_variants_.insert(WORK_FREQUENCY, pvar);
            } else {
                pvar = map_variants_[WORK_FREQUENCY];
            }

            val0.append(data->id);
            val0.append(data->frequency_point);
            val1.push_back(val0);

            pvar->setValue(val1);
            break;
        }
        case DISTURB_DIRECTION: {
            DisturbDirection *data = static_cast<DisturbDirection *>(pdata);

            if (!map_variants_.contains(DISTURB_DIRECTION)) {
                pvar = new QVariant();
                map_variants_.insert(DISTURB_DIRECTION, pvar);
            } else {
                pvar = map_variants_[DISTURB_DIRECTION];
            }

            val0.append(data->id);
            val0.append(data->eleva_angle);
            val0.append(data->pitch);
            val0.append(data->power);
            val1.push_back(val0);

            pvar->setValue(val1);
            break;
        }
        case REGION_OF_SEARCH: {
            RegionOfSearch *data = static_cast<RegionOfSearch *>(pdata);

            if (!map_variants_.contains(REGION_OF_SEARCH)) {
                pvar = new QVariant();
                map_variants_.insert(REGION_OF_SEARCH, pvar);
            } else {
                pvar = map_variants_[REGION_OF_SEARCH];
            }

            //设置表格数据
            pvar->setValue(val1);
            break;
        }
        case CHAIN_OF_COMMAND: {
            ChainOfCommand *data = static_cast<ChainOfCommand *>(pdata);

            if (!map_variants_.contains(CHAIN_OF_COMMAND)) {
                pvar = new QVariant();
                map_variants_.insert(CHAIN_OF_COMMAND, pvar);
            } else {
                pvar = map_variants_[CHAIN_OF_COMMAND];
            }

            if (map_para_choose_.contains("work_state")) {
                auto var = map_para_choose_.find("work_state");
                STRING_PUSH(var.value()->at(data->work_state))
            } else {
                STRING_PUSH("")
            }

            STRING_PUSH(data->war_preparedness_lv)

            if (map_para_choose_.contains("equip_statu")) {
                auto var = map_para_choose_.find("equip_statu");
                STRING_PUSH(var.value()->at(data->equip_state))
            } else {
                STRING_PUSH("")
            }
            if (map_para_choose_.contains("combat_permissions")) {
                auto var = map_para_choose_.find("combat_permissions");
                STRING_PUSH(var.value()->at(data->combat_permissions))
            } else {
                STRING_PUSH("")
            }
            if (map_para_choose_.contains("command_mode")) {
                auto var = map_para_choose_.find("command_mode");
                STRING_PUSH(var.value()->at(data->command_mode))
            } else {
                STRING_PUSH("")
            }

            pvar->setValue(val1);
            break;
        }
        case PHOTOELECTRICITY_EQUIPMENT: {
            PhotoelectricityEquipment *data = static_cast<PhotoelectricityEquipment *>(pdata);

            STRING_PUSH(data->id)
            STRING_PUSH(data->lon)
            STRING_PUSH(data->lat)
            STRING_PUSH(data->alt)
            STRING_PUSH(data->elevation_angle)
            STRING_PUSH(data->pitch_angle)
            if (map_para_choose_.contains("trace_status")) {
                auto var = map_para_choose_.find("trace_status");
                STRING_PUSH(var.value()->at(data->trace_status))
            } else {
                STRING_PUSH("")
            }

            if (!map_variants_.contains(PHOTOELECTRICITY_EQUIPMENT)) {
                pvar = new QVariant();
                map_variants_.insert(PHOTOELECTRICITY_EQUIPMENT, pvar);
            } else {
                pvar = map_variants_[PHOTOELECTRICITY_EQUIPMENT];
            }
            pvar->setValue(val1);
            break;
        }
        case DESCRIPTION_OF_INTERCEPTOR_WEAPON: {
            DescriptionOfInterceptorWeapon *data = static_cast<DescriptionOfInterceptorWeapon *>(pdata);

            if (map_para_choose_.contains("work_state")) {
                auto var = map_para_choose_.find("work_state");
                STRING_PUSH(var.value()->at(data->status));
            } else {
                STRING_PUSH("")
            }
            STRING_PUSH(data->war_readiness_lv);
            if (map_para_choose_.contains("combat_permissions")) {
                auto var = map_para_choose_.find("combat_permissions");
                STRING_PUSH(var.value()->at(data->operational_authority));
            } else {
                STRING_PUSH("")
            }
            if (map_para_choose_.contains("command_mode")) {
                auto var = map_para_choose_.find("command_mode");
                STRING_PUSH(var.value()->at(data->command_mode));
            } else {
                STRING_PUSH("")
            }
            STRING_PUSH(data->app_mode)
            if (map_para_choose_.contains("power_off")) {
                auto var = map_para_choose_.find("power_off");
                STRING_PUSH(var.value()->at(data->run_status));
            } else {
                STRING_PUSH("")
            }

            if (!map_variants_.contains(DESCRIPTION_OF_INTERCEPTOR_WEAPON)) {
                pvar = new QVariant();
                map_variants_.insert(DESCRIPTION_OF_INTERCEPTOR_WEAPON, pvar);
            } else {
                pvar = map_variants_[DESCRIPTION_OF_INTERCEPTOR_WEAPON];
            }
            pvar->setValue(val1);
            break;
        }
        case GBI_RESOURCES: {
            //拦截弹资源
            GBIResources *data = static_cast<GBIResources *>(pdata);

            val0.push_back(data->id);
            val0.push_back(data->bullet_quantity);
            val1.push_back(val0);

            if (!map_variants_.contains(GBI_RESOURCES)) {
                pvar = new QVariant();
                map_variants_.insert(GBI_RESOURCES, pvar);
            } else {
                pvar = map_variants_[GBI_RESOURCES];
            }
            pvar->setValue(val1);
            break;
        }
        case GUIDANCE_RADAR: {
            //制导雷达
            GuidanceRadar *data = static_cast<GuidanceRadar *>(pdata);

            val0.push_back(data->id);
            val0.push_back(data->res_occu_rate);
            val1.push_back(val0);

            if (!map_variants_.contains(GUIDANCE_RADAR)) {
                pvar = new QVariant();
                map_variants_.insert(GUIDANCE_RADAR, pvar);
            } else {
                pvar = map_variants_[GUIDANCE_RADAR];
            }
            pvar->setValue(val1);
            break;
        }
        case FIREPOWER_UNIT: {
            FirepowerUnit *data = static_cast<FirepowerUnit *>(pdata);

            if (map_para_choose_.contains("work_state")) {
                auto var = map_para_choose_.find("work_state");
                STRING_PUSH(var.value()->at(data->lon));
            } else {
                STRING_PUSH("")
            }

            if (map_para_choose_.contains("command_mode")) {
                auto var = map_para_choose_.find("command_mode");
                STRING_PUSH(var.value()->at(data->command_mode));
            } else {
                STRING_PUSH("")
            }

            STRING_PUSH(data->oper_task);

            if (map_para_choose_.contains("inter_ception_mode")) {
                auto var = map_para_choose_.find("inter_ception_mode");
                STRING_PUSH(var.value()->at(data->inter_ception_mode));
            } else {
                STRING_PUSH("")
            }

            STRING_PUSH(data->frequency_point_id);

            STRING_PUSH(data->sector_central_angle);

            if (!map_variants_.contains(FIREPOWER_UNIT)) {
                pvar = new QVariant();
                map_variants_.insert(FIREPOWER_UNIT, pvar);
            } else {
                pvar = map_variants_[FIREPOWER_UNIT];
            }
            pvar->setValue(val1);
            break;
        }
        case FIREPOWER_UNIT_AISLE: {
            FirepowerUnitAisle *data = static_cast<FirepowerUnitAisle *>(pdata);

            val0.append(data->unit_id);
            val0.append(data->target_id);
            if (map_para_choose_.contains("firepower_status")) {
                auto var = map_para_choose_.find("firepower_status");
                val0.append(var.value()->at(data->status));
            } else {
                val0.append("");
            }
            val1.push_back(val0);

            if (!map_variants_.contains(FIREPOWER_UNIT_AISLE)) {
                pvar = new QVariant();
                map_variants_.insert(FIREPOWER_UNIT_AISLE, pvar);
            } else {
                pvar = map_variants_[FIREPOWER_UNIT_AISLE];
            }
            pvar->setValue(val1);
            break;
        }
        default:
            break;
    }

    view_data(pvar);
}

bool TableModel::set_head_data(ElementType type, HeadLocal local) {
    if (type < 0) return false;

    pver_head_data_ = Q_NULLPTR;
    phor_head_data_ = Q_NULLPTR;

    if (local == VERTICAL_HEAD && map_headnames_.contains(type))
        pver_head_data_ = map_headnames_[type];
    else if (local == HORIZONTAL_HEAD && map_headnames_.contains(type))
        phor_head_data_ = map_headnames_[type];
    else
        return false;

    return true;
}

void TableModel::update() {
    this->beginResetModel();
    this->endResetModel();
}

QVariant TableModel::data(const QModelIndex &index, int role) const {
    int row = index.row();
    int column = index.column();

    if (!index.isValid()) return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (row >= data_.size()) return QVariant();
        if (column >= data_.at(row).size()) return QVariant();

        return data_.at(row).at(column);
    } else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    } else {
        return QVariant();
    }
}

int TableModel::rowCount(const QModelIndex &parent) const { return data_.size(); }

int TableModel::columnCount(const QModelIndex &parent) const {
    if (data_.isEmpty())
        return -1;
    else
        return data_.at(0).size();
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Vertical && Q_NULLPTR != pver_head_data_)
        return pver_head_data_->at(section);
    else if (orientation == Qt::Horizontal && Q_NULLPTR != phor_head_data_)
        return phor_head_data_->at(section);

    return QVariant();
}

bool TableModel::view_data(QVariant *pother_data) {
    QVariantList var_rows = pother_data->toList(); //获得二维数组
    if (var_rows.isEmpty()) return false;

    for (int i = 0; i < var_rows.size(); i++) {
        QVariantList buf = var_rows.at(i).toList();
        data_.push_back(buf);
    }

    return true;
}

bool TableModel::init_text_data() {
    QStringList *sl = new QStringList();

    // 1.初始化表头信息
    //雷达系统状态数据描述
    *sl << "开关机:"
        << "受控状态:"
        << "天线扫描方式:"
        << "天线机械俯仰角:"
        << "波束起始扫描仰角:"
        << "抗干扰措施:"
        << "杂波图:"
        << "电磁干扰强度:"
        << "时间资源分配状态:"
        << "跟踪数据率:";
    map_headnames_.insert(SYSTEM_STATE, sl);

    //工作模式
    sl = new QStringList();
    *sl << "编号"
        << "起始方位"
        << "结束方位";
    map_headnames_.insert(WORK_PATTERN, sl);

    //辐射状态
    sl = new QStringList();
    *sl << "设备名称"
        << "状态";
    map_headnames_.insert(RADIATION_STATE, sl);

    //工作频点
    sl = new QStringList();
    *sl << "编号"
        << "频点";
    map_headnames_.insert(WORK_FREQUENCY, sl);

    //有源干扰方向
    sl = new QStringList();
    *sl << "编号"
        << "俯仰角"
        << "偏航角"
        << "功率";
    map_headnames_.insert(DISTURB_DIRECTION, sl);

    //指控系统状态数据描述
    sl = new QStringList();
    *sl << "系统工作状态:"
        << "战备值班等级:"
        << "装备状态:"
        << "作战权限:"
        << "指挥方式:";
    map_headnames_.insert(CHAIN_OF_COMMAND, sl);

    //光电装备状态显示
    sl = new QStringList();
    *sl << "编号:"
        << "经度:"
        << "维度:"
        << "高度:"
        << "光学中心指向(俯仰角):"
        << "光学中心指向(偏航角):"
        << "搜索跟踪状态:";
    map_headnames_.insert(PHOTOELECTRICITY_EQUIPMENT, sl);

    //拦截武器显示
    sl = new QStringList();
    *sl << "工作状态:"
        << "战备值班等级:"
        << "作战权限:"
        << "指挥方式:"
        << "传感器应用方式:"
        << "系统运行状态:"
        << "传感器资源占用百分比:";
    map_headnames_.insert(DESCRIPTION_OF_INTERCEPTOR_WEAPON, sl);

    //拦截武器显示
    sl = new QStringList();
    *sl << "发射车id"
        << "可用弹量";
    map_headnames_.insert(GBI_RESOURCES, sl);

    //制导雷达
    sl = new QStringList();
    *sl << "编号"
        << "传感器资源占用百分比";
    map_headnames_.insert(GUIDANCE_RADAR, sl);

    //火力单元状态显示
    sl = new QStringList();
    *sl << "工作方式:"
        << "指挥方式:"
        << "作战任务:"
        << "拦截方式:"
        << "频点号:"
        << "责任扇区中心角:";
    map_headnames_.insert(FIREPOWER_UNIT, sl);

    //火力单元通道状态
    sl = new QStringList();
    *sl << "火力单元编号"
        << "跟踪目标编号"
        << "状态";
    map_headnames_.insert(FIREPOWER_UNIT_AISLE, sl);

    ////////////////////////////////////////////////////////////////////

    // 2.初始化成员状态信息
    sl = new QStringList();
    *sl << "开机"
        << "待机"
        << "关机";
    map_para_choose_.insert("power_off", sl);

    sl = new QStringList();
    *sl << "本控"
        << "遥控"
        << "其他";
    map_para_choose_.insert("control_status", sl);

    sl = new QStringList();
    *sl << "辐射"
        << "静默"
        << "闪烁";
    map_para_choose_.insert("radiation_state", sl);

    sl = new QStringList();
    *sl << "圆圈顺时针"
        << "圆圈逆时针"
        << "扇扫"
        << "驻留";
    map_para_choose_.insert("scanning_mode", sl);

    sl = new QStringList();
    *sl << "作战"
        << "训练"
        << "试验"
        << "值班";
    map_para_choose_.insert("work_state", sl);

    sl = new QStringList();
    *sl << "正常"
        << "降级"
        << "故障";
    map_para_choose_.insert("equip_statu", sl);

    sl = new QStringList();
    *sl << "允许自主射击"
        << "人工射击";
    map_para_choose_.insert("combat_permissions", sl);

    sl = new QStringList();
    *sl << "按级"
        << "越级"
        << "接替";
    map_para_choose_.insert("command_mode", sl);

    sl = new QStringList();
    *sl << "手动"
        << "引导"
        << "闭环";
    map_para_choose_.insert("trace_status", sl);

    sl = new QStringList();
    *sl << "人工"
        << "自动";
    map_para_choose_.insert("inter_ception_mode", sl);

    sl = new QStringList();
    *sl << "无效"
        << "空闲"
        << "占用"
        << "拦截"
        << "已拦";
    map_para_choose_.insert("firepower_status", sl);
    return true;
}
