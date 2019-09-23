#ifndef __TITLEBAR_H__
#define __TITLEBAR_H__

#include <QLabel>
#include <QWidget>

#include "src/utils/macro.h"

class QPushButton;
class QHBoxLayout;

class TitleBar : public QWidget {
    Q_OBJECT

public:
    TitleBar(QWidget *parent = nullptr);
    virtual ~TitleBar() override;
    void set_title_visib();
    QPushButton *get_selec_list() { return pselec_list_; }

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    //    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_clicked();

private:
    void update_maximize();
    void init_control();

private:
    int title_height_;
    QLabel *picon_label_;
    QLabel *ptitle_label_;
    QHBoxLayout *playout_;
    QPushButton *pselec_list_;
    QPushButton *pminimize_btn_;
    QPushButton *pmaximize_btn_;
    QPushButton *pclose_button_;
};

#endif // __TITLEBAR_H__
