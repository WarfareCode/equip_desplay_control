#ifndef __TABLEMODEL_H__
#define __TABLEMODEL_H__

#include <QAbstractTableModel>
#include <QTableView>

#include "elements.h"

enum HeadLocal { VERTICAL_HEAD = 0, HORIZONTAL_HEAD };

class TableModel : public QAbstractTableModel {
public:
    TableModel(QObject *parent = Q_NULLPTR);
    virtual ~TableModel() override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void add_data(void *pdata, ElementType type);
    bool set_head_data(ElementType type, HeadLocal local);
    QStringList *get_row_name(int type) { return map_headnames_[type]; }
    void update();

protected:
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool view_data(QVariant *pother_data);

private:
    bool init_text_data();

private:
    QList<QList<QVariant>> data_;
    QMap<int, QStringList *> map_headnames_;
    QMap<int, QVariant *> map_variants_;
    QMap<QString, QStringList *> map_para_choose_;
    QStringList *phor_head_data_ = Q_NULLPTR;
    QStringList *pver_head_data_ = Q_NULLPTR;
};

#endif //__TABLEMODEL_H__
