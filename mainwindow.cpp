#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this); // Это QT добавил автоматически

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

    ui->comboBoxFlowControl->addItem("NO"); // Заполнение настройки окончания строки
    ui->comboBoxFlowControl->addItem("SOFT");
    ui->comboBoxFlowControl->addItem("HARD");
    ui->comboBoxFlowControl->setCurrentText("NO"); // Указание "дефолтного" значения

    ui->comboBoxInput->addItem("2A");
    ui->comboBoxInput->addItem("2B");
    ui->comboBoxInput->addItem("2C");
    ui->comboBoxInput->addItem("2D");

    ui->comboBoxInput->addItem("3A");
    ui->comboBoxInput->addItem("3B");
    ui->comboBoxInput->addItem("3C");
    ui->comboBoxInput->addItem("3D");

    ui->comboBoxOutput->addItem("Out1");

    ui->comboBoxInput->setCurrentText("2A");
    ui->comboBoxOutput->setCurrentText("Out1");

    ui->doubleSpinBoxP->clear();
    ui->doubleSpinBoxI->clear();
    ui->doubleSpinBoxD->clear();
    ui->doubleSpinBoxSetpoint->clear();
    ui->doubleSpinBoxOutputValue->clear();

    on_actionCOMUPD_triggered(); // Обнновить список доступных портов

    serial = new QSerialPort();  // Создать новый объект класса "SerialPort"

    connect(this, SIGNAL(response(QString)),
            this, SLOT(get_response(QString))); // Подключить сигнал получения ответа к слоту
    timer.setInterval(200);
    connect(&timer, SIGNAL(timeout()), this, SLOT(Send_request()));
}

MainWindow::~MainWindow()
{
    delete serial; // Удалить ненужные переменные
    delete ui;
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

        if (ui->comboBoxFlowControl->currentText() == "NO") // Указание метода контроля передачи данных
            serial->setFlowControl(QSerialPort::NoFlowControl);
        if (ui->comboBoxFlowControl->currentText() == "SOFT")
            serial->setFlowControl(QSerialPort::SoftwareControl);
        if (ui->comboBoxFlowControl->currentText() == "HARD")
            serial->setFlowControl(QSerialPort::HardwareControl);

        if (!serial->open(QIODevice::ReadWrite)) { // Если попытка открыть порт для ввода\вывода не получилось
            QSerialPort::SerialPortError getError = QSerialPort::NoError; // Ошибка открытия порта
            serial->error(getError); // Получить номер ошибки
            emit response(tr("Can't open %1, error code %2").arg(ui->comboBoxCOM->currentText()).arg(serial->error())); // Выдать сообщение об ошибке

            return;
        }

        ui->pushButtonAction->setText("Disconnect"); // Перевести кнопку в режим "Отключение"

        ui->lineEditResponse->setText(""); // Опустошить строку с последним текущим ответом
    } else { // Если нам нужно отключиться
        this->serial->close(); // Закрыть открытый порт

        ui->pushButtonAction->setText("Connect"); // Перевести кнопку в режим "Подключение"
    }

    // Блокировка или разблокировка переключателей настроек порта
    ui->comboBoxCOM->setEnabled(ui->pushButtonAction->text() == "Connect");
    ui->comboBoxBauds->setEnabled(ui->pushButtonAction->text() == "Connect");
    ui->comboBoxFlowControl->setEnabled(ui->pushButtonAction->text() == "Connect");

    // Блокировка или разблокировка кнопок отправки и приёма сообщений
    ui->lineEditCommand->setEnabled(ui->pushButtonAction->text() == "Disconnect");
    ui->pushButtonSend->setEnabled(ui->pushButtonAction->text() == "Disconnect");
    ui->pushButtonRecieve->setEnabled(ui->pushButtonAction->text() == "Disconnect");
    ui->pushButtonHeat->setEnabled(ui->pushButtonAction->text() == "Disconnect");
    ui->pushButtonDisable->setEnabled(ui->pushButtonAction->text() == "Disconnect");
    ui->checkBoxPID->setEnabled(ui->pushButtonAction->text() == "Disconnect");
    ui->doubleSpinBoxOutputValue->setEnabled(ui->pushButtonAction->text() == "Disconnect");

    return;
}

void MainWindow::Send_request()
{
    QString str;
    QByteArray arr;

    serial->write((ui->comboBoxOutput->currentText() + ".value?\n").toLocal8Bit());
    QTest::qWait(60);
    ui->doubleSpinBoxOutputValue->setValue(QString(serial->readAll()).toDouble());

    serial->write((ui->comboBoxInput->currentText() + ".value?\n").toLocal8Bit());
    QTest::qWait(60);
    ui->lineEditInputValue->setText(QString(serial->readAll()));

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

void MainWindow::on_pushButtonSet_clicked()
{
    serial->write((this->CMD1 + ui->comboBoxInput->currentText() + "\n").toLocal8Bit());
    serial->write((this->CMD2 + QString("%1").arg(ui->doubleSpinBoxP->value(),0,'g',6) + "\n").toLocal8Bit());
    serial->write((this->CMD3 + QString("%1").arg(ui->doubleSpinBoxI->value(),0,'g',6) + "\n").toLocal8Bit());
    serial->write((this->CMD4 + QString("%1").arg(ui->doubleSpinBoxD->value(),0,'g',6) + "\n").toLocal8Bit());
    serial->write((this->CMD5 + QString("%1").arg(ui->doubleSpinBoxSetpoint->value(),0,'g',6) + "\n").toLocal8Bit());
}

void MainWindow::on_pushButtonHeat_clicked()
{
    serial->write((this->CMD6 + "\n").toLocal8Bit());
    timer.start();
}

void MainWindow::on_pushButtonDisable_clicked()
{
    serial->write((this->CMD7 + "\n").toLocal8Bit());
    timer.stop();
}

void MainWindow::on_checkBoxPID_stateChanged(int arg1)
{
    QString str;
    if(arg1 > 0)
        serial->write(QByteArray("Out1.PID.Mode = on\n"));
    else
        serial->write(QByteArray("Out1.PID.Mode = off\n"));

    ui->doubleSpinBoxP->setEnabled(arg1 > 0);
    ui->doubleSpinBoxI->setEnabled(arg1 > 0);
    ui->doubleSpinBoxD->setEnabled(arg1 > 0);
    ui->doubleSpinBoxSetpoint->setEnabled(arg1 > 0);
    ui->pushButtonSet->setEnabled(arg1 > 0);
    if(arg1 > 0)
    {
        serial->write(QByteArray("Out1.PID.P?\n"));
        QTest::qWait(100);
        str = QString(serial->readAll());
        ui->doubleSpinBoxP->setValue(str.toDouble());
        serial->write(QByteArray("Out1.PID.I?\n"));
        QTest::qWait(100);
        str = QString(serial->readAll());
        ui->doubleSpinBoxI->setValue(str.toDouble());
        serial->write(QByteArray("Out1.PID.D?\n"));
        QTest::qWait(100);
        str = QString(serial->readAll());
        ui->doubleSpinBoxD->setValue(str.toDouble());
        serial->write(QByteArray("Out1.PID.Setpoint?\n"));
        QTest::qWait(100);
        str = QString(serial->readAll());
        ui->doubleSpinBoxSetpoint->setValue(str.toDouble());
    }
    else
    {
        ui->doubleSpinBoxP->clear();
        ui->doubleSpinBoxI->clear();
        ui->doubleSpinBoxD->clear();
        ui->doubleSpinBoxSetpoint->clear();
    }

    emit response("Nice!");

}
