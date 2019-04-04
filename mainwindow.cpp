#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this); // Это QT добавил автоматически

    serial = new QSerialPort();  // Создать новый объект класса "SerialPort"

    ui->pushButtonAction->setText("Connect"); // Кнопка работает в режиме "Подключить"

    QStringList Bauds; // Заполнение настройки частоты передачи данных
    Bauds.push_back(   "300");
    Bauds.push_back(   "600");
    Bauds.push_back(  "1200");
    Bauds.push_back(  "2400");
    Bauds.push_back(  "4800");
    Bauds.push_back(  "9600");
    Bauds.push_back( "19200");
    Bauds.push_back( "38400");
    Bauds.push_back( "57600");
    Bauds.push_back("115200");
    ui->comboBoxBauds->addItems(Bauds);
    ui->comboBoxBauds->setCurrentText("9600"); // Указание "дефолтного" значения

    ui->comboBoxInput->addItem("2A");
    ui->comboBoxInput->addItem("2B");
    ui->comboBoxInput->addItem("2C");
    ui->comboBoxInput->addItem("2D");

    ui->comboBoxInput->addItem("3A");
    ui->comboBoxInput->addItem("3B");
    ui->comboBoxInput->addItem("3C");
    ui->comboBoxInput->addItem("3D");

    ui->comboBoxOutput->addItem("Out1");

    ui->comboBoxInput->setCurrentText("3A");
    ui->comboBoxOutput->setCurrentText("Out1");

    ui->doubleSpinBoxP->clear();
    ui->doubleSpinBoxI->clear();
    ui->doubleSpinBoxD->clear();
    ui->doubleSpinBoxSetpoint->clear();
    ui->doubleSpinBoxOutputValue->clear();

    on_actionCOMUPD_triggered(); // Обнновить список доступных портов

    connect(this, SIGNAL(response(QString)),
            this, SLOT(get_response(QString))); // Подключить сигнал получения ответа к слоту
    timer.setInterval(200);
    connect(&timer, SIGNAL(timeout()), this, SLOT(Send_request()));
    ui->Plot->addGraph();
    ui->Plot->xAxis->setRange(-1, 1);
    ui->Plot->yAxis->setRange(0, 1);

    ui->Plot->xAxis->setLabel("Time, s");
    ui->Plot->yAxis->setLabel("Temperature, °C");


    ui->Plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    ui->Plot2->addGraph();
    ui->Plot2->xAxis->setRange(-1, 1);
    ui->Plot2->yAxis->setRange(0, 1);

    ui->Plot2->xAxis->setLabel("Time, s");
    ui->Plot2->yAxis->setLabel("Power, W");


    ui->Plot2->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

MainWindow::~MainWindow()
{
    delete serial; // Удалить ненужные переменные
    delete ui;
}

QString MainWindow::GetResponse(const QString &str)
{
    QByteArray array;
    QString temp, cmd;
    int res = 0;

    this->firstWaitTime = ui->spinBoxTimeout->value();

    if(str == "T")
        res = 1;
    else if(str == "Output")
        res = 2;
    else if(str == "P")
        res = 3;
    else if(str == "I")
        res = 4;
    else if(str == "D")
        res = 5;
    else if(str == "Setpoint")
        res = 6;
    else
        emit response("Wrong string!!!");

    switch (res) {
    case 1:
        cmd = ui->comboBoxInput->currentText() + ".value?\n"; break;
    case 2:
        cmd = ui->comboBoxOutput->currentText() + ".value?\n"; break;
    case 3:
        cmd = "Out1.PID.P?\n"; break;
    case 4:
        cmd = "Out1.PID.I?\n"; break;
    case 5:
        cmd = "Out1.PID.D?\n"; break;
    case 6:
        cmd = "Out1.PID.Setpoint?\n"; break;
    default:
        emit response("Error motherfucker!!!");
    }

    serial->write(cmd.toLocal8Bit());

    if (serial->waitForReadyRead(this->firstWaitTime)) { // Если за данное число миллисекунд что-то пришло
        array = serial->readAll(); // Прочитать полученные данные
        while (serial->waitForReadyRead(this->additionalWaitTime)) // Если пришло что-то ещё
            array += serial->readAll(); // Дописать новые данные
        temp = QString(array); // Преобразовать полученные данные в строку
    }
    return temp;
}

void MainWindow::SendRequest(const QString &str)
{
    if(str == "Init")
        serial->write((this->CMD1 + ui->comboBoxInput->currentText() + "\n").toLocal8Bit());
    else if(str == "P")
        serial->write((this->CMD2 + QString("%1").arg(ui->doubleSpinBoxP->value(),0,'g',6) + "\n").toLocal8Bit());
    else if(str == "I")
        serial->write((this->CMD3 + QString("%1").arg(ui->doubleSpinBoxI->value(),0,'g',6) + "\n").toLocal8Bit());
    else if(str == "D")
        serial->write((this->CMD4 + QString("%1").arg(ui->doubleSpinBoxD->value(),0,'g',6) + "\n").toLocal8Bit());
    else if(str == "Setpoint")
        serial->write((this->CMD5 + QString("%1").arg(ui->doubleSpinBoxSetpoint->value(),0,'g',6) + "\n").toLocal8Bit());
    else if(str == "Enable")
        serial->write((this->CMD6 + "\n").toLocal8Bit());
    else if(str == "Disable")
        serial->write((this->CMD7 + "\n").toLocal8Bit());
    else if(str == "PIDOn")
        serial->write(QByteArray("Out1.PID.Mode = on\n"));
    else if(str == "PIDOff")
        serial->write(QByteArray("Out1.PID.Mode = off\n"));
    else
        emit response("Wrong string!!!");
    return;
}

void MainWindow::on_actionCOMUPD_triggered()
{
    ui->comboBoxCOM->clear(); // Стереть все имевшиеся старые порты

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) { // Добавление доступных в системе портов
        ui->comboBoxCOM->addItem(info.portName());
    }
    return;
}

void MainWindow::on_pushButtonAction_clicked()
{
    if (ui->pushButtonAction->text() == "Connect") { // Если нам нужно подключиться
        serial->setPortName(ui->comboBoxCOM->currentText()); // Указание имени порта

        serial->setBaudRate(ui->comboBoxBauds->currentText().toInt()); // Указание частоты передачи порта

        serial->setFlowControl(QSerialPort::HardwareControl);

        if (!serial->open(QIODevice::ReadWrite)) { // Если попытка открыть порт для ввода\вывода не получилось
            QSerialPort::SerialPortError getError = QSerialPort::NoError; // Ошибка открытия порта
            serial->error(getError); // Получить номер ошибки
            emit response(tr("Can't open %1, error code %2").arg(ui->comboBoxCOM->currentText()).arg(serial->error())); // Выдать сообщение об ошибке

            return;
        }

        ui->pushButtonAction->setText("Disconnect"); // Перевести кнопку в режим "Отключение"
        timer.start();

        ui->lineEditResponse->setText(""); // Опустошить строку с последним текущим ответом
    } else { // Если нам нужно отключиться
        ui->checkBoxPID->setCheckState(Qt::Unchecked);

        QTest::qWait(100);

        this->serial->close(); // Закрыть открытый порт
        timer.stop();

        ui->pushButtonAction->setText("Connect"); // Перевести кнопку в режим "Подключение"
    }

    // Блокировка или разблокировка переключателей настроек порта
    ui->comboBoxCOM->setEnabled(ui->pushButtonAction->text() == "Connect");
    ui->comboBoxBauds->setEnabled(ui->pushButtonAction->text() == "Connect");

    // Блокировка или разблокировка кнопок отправки и приёма сообщений
    ui->lineEditCommand->setEnabled(ui->pushButtonAction->text() == "Disconnect");
    ui->pushButtonSend->setEnabled(ui->pushButtonAction->text() == "Disconnect");
    ui->pushButtonRecieve->setEnabled(ui->pushButtonAction->text() == "Disconnect");
    ui->pushButtonHeat->setEnabled(ui->pushButtonAction->text() == "Disconnect");
    ui->pushButtonDisable->setEnabled(ui->pushButtonAction->text() == "Disconnect");
    ui->checkBoxPID->setEnabled(ui->pushButtonAction->text() == "Disconnect");
    ui->doubleSpinBoxOutputValue->setEnabled(ui->pushButtonAction->text() == "Disconnect");
    ui->comboBoxInput->setEnabled(ui->pushButtonAction->text() == "Disconnect");

    return;
}

void MainWindow::Send_request()
{
    QString str;
    QByteArray arr;

    QString T = GetResponse("T");
    QString P = GetResponse("Output");

    ui->doubleSpinBoxOutputValue->setValue(P.toDouble());
    ui->lineEditInputValue->setText(T);

    //emit response("May be good");

    ui->Plot->graph(0)->addData(this->timeplot.elapsed()/1000.0, T.toDouble());
    ui->Plot2->graph(0)->addData(this->timeplot.elapsed()/1000.0, P.toDouble());

    ui->Plot->rescaleAxes();
    ui->Plot2->rescaleAxes();

    ui->Plot->replot();
    ui->Plot2->replot();

    return;
}

void MainWindow::on_pushButtonSend_clicked()
{
    QString command = ui->lineEditCommand->text(); // Получить команду из текстового поля

    command += "\n";

    serial->write(command.toLocal8Bit()); // Передать данные по порту в битовом представлении

    on_pushButtonRecieve_clicked(); // Получить ответ

    return;
}

void MainWindow::on_pushButtonRecieve_clicked()
{
    QString temp; // Строка для записи ответа
    QByteArray array; // Массив данных ответа

    this->firstWaitTime = ui->spinBoxTimeout->value(); // Задать выбранный пользователем таймаут

    this->time = QTime(0, 0, 0, 0); // Обнуление времени
    time.start(); // Начала счётчика

    if (serial->waitForReadyRead(this->firstWaitTime)) { // Если за данное число миллисекунд что-то пришло
        array = serial->readAll(); // Прочитать полученные данные
        while (serial->waitForReadyRead(this->additionalWaitTime)) // Если пришло что-то ещё
            array += serial->readAll(); // Дописать новые данные
        temp = QString(array); // Преобразовать полученные данные в строку

        ui->lineEditTTR->setText(QString::number(time.elapsed()-this->additionalWaitTime)); // Записать время отклика
    } else { // Если не пришло ничего
        temp = "Wait write request timeout"; // Сформировать отчёт об ошибке

        ui->lineEditTTR->setText(">" + QString::number(this->firstWaitTime)); // Время отклика не определено точно
    }

    emit response(temp); // Вывести полученный ответ на экран

    return;
}

void MainWindow::get_response(const QString &s)
{
    ui->lineEditResponse->setText(s); // Вывести сообщение в текстовое поле

    return;
}

void MainWindow::on_pushButtonHeat_clicked()
{
    SendRequest("Enable");
    this->timeplot.start();
}

void MainWindow::on_pushButtonDisable_clicked()
{
    SendRequest("Disable");
}

void MainWindow::on_checkBoxPID_stateChanged(int arg1)
{
    QString str;
    if(arg1 > 0) {
        SendRequest("PIDOn");
        SendRequest("Init");
    }
    else {
        SendRequest("PIDOff");
    }

    ui->doubleSpinBoxP->setEnabled(arg1 > 0);
    ui->doubleSpinBoxI->setEnabled(arg1 > 0);
    ui->doubleSpinBoxD->setEnabled(arg1 > 0);
    ui->doubleSpinBoxSetpoint->setEnabled(arg1 > 0);

    if(arg1 > 0)
    {
        ui->doubleSpinBoxP->setValue(GetResponse("P").toDouble());
        ui->doubleSpinBoxI->setValue(GetResponse("I").toDouble());
        ui->doubleSpinBoxD->setValue(GetResponse("D").toDouble());
        ui->doubleSpinBoxSetpoint->setValue(GetResponse("Setpoint").toDouble());
    }
    else
    {
        ui->doubleSpinBoxP->clear();
        ui->doubleSpinBoxI->clear();
        ui->doubleSpinBoxD->clear();
        ui->doubleSpinBoxSetpoint->clear();
    }
}

void MainWindow::on_doubleSpinBoxP_valueChanged(const QString &arg1)
{
    SendRequest("P");
}

void MainWindow::on_doubleSpinBoxI_valueChanged(const QString &arg1)
{
    SendRequest("I");
}

void MainWindow::on_doubleSpinBoxD_valueChanged(const QString &arg1)
{
    SendRequest("D");
}

void MainWindow::on_doubleSpinBoxSetpoint_valueChanged(const QString &arg1)
{
    SendRequest("Setpoint");
}

void MainWindow::on_comboBoxInput_currentTextChanged(const QString &arg1)
{
    SendRequest("Init");
}
