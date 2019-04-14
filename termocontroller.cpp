#include "termocontroller.h"

Termocontroller::Termocontroller()
{
    serial = new QSerialPort();
    this->firstWaitTime = 100;
}

Termocontroller::~Termocontroller()
{
    delete serial;
}

QString Termocontroller::GetResponse(const QString &str)
{
    QByteArray array;
    QString temp, cmd;

    if(str == "T")
        cmd = portname + ".value?\n";
    else if(str == "Output")
        cmd = "Out1.value?\n";
    else if(str == "P")
        cmd = "Out1.PID.P?\n";
    else if(str == "I")
        cmd = "Out1.PID.I?\n";
    else if(str == "D")
        cmd = "Out1.PID.D?\n";
    else if(str == "Setpoint")
        cmd = "Out1.PID.Setpoint?\n";
    else
        cmd = "";

    serial->write(cmd.toLocal8Bit());

    if (serial->waitForReadyRead(this->firstWaitTime)) { // Если за данное число миллисекунд что-то пришло
        array = serial->readAll(); // Прочитать полученные данные
        while (serial->waitForReadyRead(this->additionalWaitTime)) // Если пришло что-то ещё
            array += serial->readAll(); // Дописать новые данные
        temp = QString(array); // Преобразовать полученные данные в строку
    }
    return temp;
}

void Termocontroller::SendRequest(const QString &str)
{
    if(str == "Init")
        serial->write((this->CMD1 + portname + "\n").toLocal8Bit());
    else if(str == "P")
        serial->write((this->CMD2 + QString("%1").arg(P,0,'g',6) + "\n").toLocal8Bit());
    else if(str == "I")
        serial->write((this->CMD3 + QString("%1").arg(I,0,'g',6) + "\n").toLocal8Bit());
    else if(str == "D")
        serial->write((this->CMD4 + QString("%1").arg(D,0,'g',6) + "\n").toLocal8Bit());
    else if(str == "Setpoint")
        serial->write((this->CMD5 + QString("%1").arg(Setpoint,0,'g',6) + "\n").toLocal8Bit());
    else if(str == "Enable")
        serial->write((this->CMD6 + "\n").toLocal8Bit());
    else if(str == "Disable")
        serial->write((this->CMD7 + "\n").toLocal8Bit());
    else if(str == "PIDOn")
        serial->write(QByteArray("Out1.PID.Mode = on\n"));
    else if(str == "PIDOff")
        serial->write(QByteArray("Out1.PID.Mode = off\n"));
    else
        return;
    return;
}

void Termocontroller::SetPortname(const QString &str)
{
    portname = str;
    SendRequest("Init");
}

void Termocontroller::SetP(const double str)
{
    P = str;
    SendRequest("P");
}

void Termocontroller::SetI(const double str)
{
    I = str;
    SendRequest("I");
}

void Termocontroller::SetD(const double str)
{
    D = str;
    SendRequest("D");
}

void Termocontroller::SetSetpoint(const double str)
{
    Setpoint = str;
    SendRequest("Setpoint");
}

double Termocontroller::GetP()
{
    P = GetResponse("P").toDouble();
    return P;
}

double Termocontroller::GetI()
{
    I = GetResponse("I").toDouble();
    return I;
}
double Termocontroller::GetD()
{
    D = GetResponse("D").toDouble();
    return D;
}
double Termocontroller::GetSetpoint()
{
    Setpoint = GetResponse("Setpoint").toDouble();
    return Setpoint;
}
