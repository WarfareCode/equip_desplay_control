#ifndef __ELEMENTS_H__
#define __ELEMENTS_H__

#include <QMetaType>
#include <QObject>
#include <QStringList>

//雷达系统状态
struct SystemState {
    int power_off;
    int control_state;
    int scanning_mode;
    double antenna_eleva_angle;
    double beam_eleva_angle;
    int eccm_measures;
    QString clutter_map;
    double emi_intensity;
    double time_alloca_state;
    double track_data_rate;
};
Q_DECLARE_METATYPE(SystemState);

//工作模式
struct WorkPattern {
    int id;
    double start_yaw;
    double end_yaw;
};
Q_DECLARE_METATYPE(WorkPattern);

//辐射状态
struct RadiationState {
    int equipment_id;
    int radiation_state;
};
Q_DECLARE_METATYPE(RadiationState);

//工作频点
struct WorkFrequency {
    int id;
    double frequency_point;
};
Q_DECLARE_METATYPE(WorkFrequency);

//有源干扰方向
struct DisturbDirection {
    int id;
    double eleva_angle;
    double pitch;
    double power;
};
Q_DECLARE_METATYPE(DisturbDirection);

//搜索区域
struct RegionOfSearch {
    double max_lon;
    double min_lon;
    double max_lat;
    double min_lat;
};
Q_DECLARE_METATYPE(RegionOfSearch);

//指控系统状态数据描述
struct ChainOfCommand {
    int work_state;
    int war_preparedness_lv;
    int equip_state;
    int combat_permissions;
    int command_mode;
};
Q_DECLARE_METATYPE(ChainOfCommand);

//光电装备状态显示
struct PhotoelectricityEquipment {
    int id;
    double lon;
    double lat;
    double alt;
    double elevation_angle;
    double pitch_angle;
    int trace_status;
};
Q_DECLARE_METATYPE(PhotoelectricityEquipment);

//拦截武器装备数据描述
struct DescriptionOfInterceptorWeapon {
    double status;             //工作状态
    int war_readiness_lv;      //战备值班等级
    int operational_authority; //作战权限
    int command_mode;          //指挥方式
    int app_mode;              //传感器应用方式
    int run_status;            //系统运行状态
};
Q_DECLARE_METATYPE(DescriptionOfInterceptorWeapon);

//制导雷达
struct GuidanceRadar {
    int id;
    double res_occu_rate;
};
Q_DECLARE_METATYPE(GuidanceRadar);

//拦截弹资源
struct GBIResources {
    int id;
    double bullet_quantity; //可用弹量
};
Q_DECLARE_METATYPE(GBIResources);

//火力单元
struct FirepowerUnit {
    int lon;                     //工作方式
    int command_mode;            //指挥方式
    int oper_task;               //作战任务
    int inter_ception_mode;      //拦截方式
    int frequency_point_id;      //频点号
    double sector_central_angle; //责任扇区中心角
};
Q_DECLARE_METATYPE(FirepowerUnit);

//火力单元通道状态数据描述
struct FirepowerUnitAisle {
    int unit_id;   //火力单元编号
    int target_id; //跟踪目标编号
    int status;    //状态
};
Q_DECLARE_METATYPE(FirepowerUnitAisle);

enum ElementType {
    SYSTEM_STATE = 0,
    WORK_PATTERN,
    RADIATION_STATE,
    WORK_FREQUENCY,
    DISTURB_DIRECTION,
    REGION_OF_SEARCH,
    CHAIN_OF_COMMAND,
    PHOTOELECTRICITY_EQUIPMENT,
    DESCRIPTION_OF_INTERCEPTOR_WEAPON,
    GBI_RESOURCES,
    GUIDANCE_RADAR,
    FIREPOWER_UNIT,
    FIREPOWER_UNIT_AISLE,
};
Q_DECLARE_METATYPE(ElementType);

#endif //__ELEMENTS_H__
