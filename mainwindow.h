#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTime>
#include <QDebug>
#include <QTest>
#include <QTimer>
#include <QVector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    int firstWaitTime = 500; // Первичное время ожидания новых данных, мс
    int additionalWaitTime = 50; // Дополнительное время ожидания новых данных, мс
    QSerialPort *serial; // SerialPort
    QTime time; // Для измерения времени отклика
    QTime timeplot;
    QTimer timer;


    QString CMD1 = "Out1.PID.Input = ";
    QString CMD2 = "Out1.PID.P = ";
    QString CMD3 = "Out1.PID.I = ";
    QString CMD4 = "Out1.PID.D = ";
    QString CMD5 = "Out1.PID.Setpoint = ";
    QString CMD6 = "outputEnable = on";
    QString CMD7 = "outputEnable = off";


public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void SendRequest(const QString &str);
    QString GetResponse(const QString &str);

private slots:
    void on_actionCOMUPD_triggered(); // Обновление списка портов

    void on_pushButtonAction_clicked(); // Подключение и отключение

    void on_pushButtonSend_clicked(); // Отправка команды
    void on_pushButtonRecieve_clicked(); // Получение ответа

    void get_response(const QString &s); // Выведение ответа или отчёта об ошибки

    void on_pushButtonHeat_clicked();

    void Send_request();

    void on_pushButtonDisable_clicked();

    void on_checkBoxPID_stateChanged(int arg1);

    void on_doubleSpinBoxP_valueChanged(const QString &arg1);

    void on_doubleSpinBoxI_valueChanged(const QString &arg1);

    void on_doubleSpinBoxD_valueChanged(const QString &arg1);

    void on_doubleSpinBoxSetpoint_valueChanged(const QString &arg1);

    void on_comboBoxInput_currentTextChanged(const QString &arg1);

signals:
    void response(const QString &s); // Отправка ответа в слот

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
