#pragma once

#include <QDialog>

#include <QStack>
#include <QDebug>
#include <QScrollBar>

#include <QKeyEvent>
#include <QJsonValue>

QT_BEGIN_NAMESPACE
namespace Ui { class RpnKeypadDialog; }
QT_END_NAMESPACE

class RpnKeypadDialog : public QDialog
{
    Q_OBJECT

public:
    RpnKeypadDialog(QStack<QJsonValue> &stack, QWidget* parent = nullptr);
    ~RpnKeypadDialog();

private slots:
    void on_button_0_clicked();
    void on_button_1_clicked();
    void on_button_2_clicked();
    void on_button_3_clicked();
    void on_button_4_clicked();
    void on_button_5_clicked();
    void on_button_6_clicked();
    void on_button_7_clicked();
    void on_button_8_clicked();
    void on_button_9_clicked();
    void on_button_dot_clicked();
    void on_button_chs_clicked();
    void on_button_enter_clicked();
    void on_button_back_clicked();

    void on_button_add_clicked();
    void on_button_subtract_clicked();
    void on_button_multiply_clicked();
    void on_button_divide_clicked();
    //    void on_button_ok_clicked();

    void on_pb_1_1_clicked();
    void on_pb_2_1_clicked();
    void on_pb_3_1_clicked();
    void on_pb_4_1_clicked();
    void on_pb_5_1_clicked();
    void on_pb_6_1_clicked();
    void on_pb_7_1_clicked();
    void on_pb_8_1_clicked();
    void on_pb_9_1_clicked();

    void on_pb_1_2_clicked();
    void on_pb_2_2_clicked();
    void on_pb_3_2_clicked();
    void on_pb_4_2_clicked();
    void on_pb_5_2_clicked();
    void on_pb_6_2_clicked();
    void on_pb_7_2_clicked();
    void on_pb_8_2_clicked();
    void on_pb_9_2_clicked();
    void on_pb_10_2_clicked();

    void on_pb_1_3_clicked();
    void on_pb_2_3_clicked();
    void on_pb_3_3_clicked();
    void on_pb_4_3_clicked();
    void on_pb_5_3_clicked();
    void on_pb_6_3_clicked();
    void on_pb_7_3_clicked();
    void on_pb_8_3_clicked();
    void on_pb_9_3_clicked();
    void on_pb_10_3_clicked();

    void on_pb_1_4_clicked();
    void on_pb_2_4_clicked();
    void on_pb_3_4_clicked();
    void on_pb_4_4_clicked();
    void on_pb_5_4_clicked();
    void on_pb_6_4_clicked();
    void on_pb_7_4_clicked();
    void on_pb_8_4_clicked();
    void on_pb_9_4_clicked();
    void on_pb_10_4_clicked();

    void on_pb_7_0_clicked();
    void on_pb_8_0_clicked();
    void on_pb_9_0_clicked();

    void on_toggle_work_machine_clicked();
    void on_toggle_relative_absolute_clicked();
    void on_toggle_jog_cut_clicked();

public:
    struct Privates;
 private:
    Privates *_p;
};
