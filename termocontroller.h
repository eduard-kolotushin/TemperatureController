#ifndef TERMOCONTROLLER_H
#define TERMOCONTROLLER_H

#include <QString>
#include <QSerialPort>
#include <QSerialPortInfo>


class Termocontroller
{
public:
    Termocontroller();
    ~Termocontroller();
    QSerialPort *serial; // SerialPort
    QString GetResponse(const QString &str);
    void SetPortname(const QString &str);
    void SetP(const double str);
    void SetI(const double str);
    void SetD(const double str);
    void SetSetpoint(const double str);
    double GetP();
    double GetI();
    double GetD();
    double GetSetpoint();
    void SendRequest(const QString &str);
private:
    int firstWaitTime = 500;
    int additionalWaitTime = 50;

    QString portname = "2A";
    double P = 0;
    double I = 0;
    double D = 0;
    double Setpoint = 0;

    QString CMD1 = "Out1.PID.Input = ";
    QString CMD2 = "Out1.PID.P = ";
    QString CMD3 = "Out1.PID.I = ";
    QString CMD4 = "Out1.PID.D = ";
    QString CMD5 = "Out1.PID.Setpoint = ";
    QString CMD6 = "outputEnable = on";
    QString CMD7 = "outputEnable = off";
};

#endif // TERMOCONTROLLER_H
