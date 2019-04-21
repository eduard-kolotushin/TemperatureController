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
    term.SetPortname(ui->comboBoxInput->currentText());
    ui->comboBoxOutput->setCurrentText("Out1");

    ui->doubleSpinBoxP->clear();
    ui->doubleSpinBoxI->clear();
    ui->doubleSpinBoxD->clear();
    ui->doubleSpinBoxSetpoint->clear();
    ui->doubleSpinBoxOutputValue->clear();

    on_actionCOMUPD_triggered(); // Обнновить список доступных портов

    connect(ui->actionCOM_update, SIGNAL(triggered()), this, SLOT(on_actionCOMUPD_triggered()));

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
    ui->groupBoxComunication->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::Send_request()
{
    QString str;
    QByteArray arr;

    QString T = term.GetResponse("T");
    QString P = term.GetResponse("Output");

    ui->doubleSpinBoxOutputValue->setValue(P.toDouble());
    ui->lineEditInputValue->setText(T);

    t.push_back(this->timeplot.elapsed()/1000.0);
    Temp.push_back(T.toDouble());

    ui->Plot->graph(0)->addData(t.back(), Temp.back());
    ui->Plot2->graph(0)->addData(t.back(), P.toDouble());

    ui->Plot->rescaleAxes();
    ui->Plot2->rescaleAxes();

    ui->Plot->replot();
    ui->Plot2->replot();

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
        term.serial->setPortName(ui->comboBoxCOM->currentText()); // Указание имени порта

        term.serial->setBaudRate(ui->comboBoxBauds->currentText().toInt()); // Указание частоты передачи порта

        term.serial->setFlowControl(QSerialPort::HardwareControl);

        if (!term.serial->open(QIODevice::ReadWrite)) { // Если попытка открыть порт для ввода\вывода не получилось
            QSerialPort::SerialPortError getError = QSerialPort::NoError; // Ошибка открытия порта
            term.serial->error(getError); // Получить номер ошибки
            emit response(tr("Can't open %1, error code %2").arg(ui->comboBoxCOM->currentText()).arg(term.serial->error())); // Выдать сообщение об ошибке

            return;
        }

        ui->pushButtonAction->setText("Disconnect"); // Перевести кнопку в режим "Отключение"

        ui->lineEditResponse->setText(""); // Опустошить строку с последним текущим ответом
    } else { // Если нам нужно отключиться
        ui->checkBoxPID->setCheckState(Qt::Unchecked);

        QTest::qWait(100);

        this->term.serial->close(); // Закрыть открытый порт
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

void MainWindow::on_pushButtonSend_clicked()
{
    QString command = ui->lineEditCommand->text(); // Получить команду из текстового поля

    command += "\n";

    term.serial->write(command.toLocal8Bit()); // Передать данные по порту в битовом представлении

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

    if (term.serial->waitForReadyRead(this->firstWaitTime)) { // Если за данное число миллисекунд что-то пришло
        array = term.serial->readAll(); // Прочитать полученные данные
        while (term.serial->waitForReadyRead(this->additionalWaitTime)) // Если пришло что-то ещё
            array += term.serial->readAll(); // Дописать новые данные
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
    if(ui->pushButtonHeat->text() == ("Heat"))
    {
        timer.start();
        term.SendRequest("Enable");
        if(this->timeplot.elapsed() == 0)
            this->timeplot.start();
        ui->pushButtonHeat->setText("Disable");
    }
    else
    {
        term.SendRequest("Disable");
        ui->pushButtonHeat->setText("Heat");
    }
    return;
}

void MainWindow::on_pushButtonDisable_clicked()
{
    timer.stop();
    return;
}

void MainWindow::on_checkBoxPID_stateChanged(int arg1)
{
    QString str;
    if(arg1 > 0) {
        term.SendRequest("PIDOn");
        term.SetPortname(ui->comboBoxInput->currentText());
    }
    else {
        term.SendRequest("PIDOff");
    }

    ui->doubleSpinBoxP->setEnabled(arg1 > 0);
    ui->doubleSpinBoxI->setEnabled(arg1 > 0);
    ui->doubleSpinBoxD->setEnabled(arg1 > 0);
    ui->doubleSpinBoxSetpoint->setEnabled(arg1 > 0);

    if(arg1 > 0)
    {
        ui->doubleSpinBoxP->setValue(term.GetP());
        ui->doubleSpinBoxI->setValue(term.GetI());
        ui->doubleSpinBoxD->setValue(term.GetD());
        ui->doubleSpinBoxSetpoint->setValue(term.GetSetpoint());
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
    term.SetP(ui->doubleSpinBoxP->value());
}

void MainWindow::on_doubleSpinBoxI_valueChanged(const QString &arg1)
{
    term.SetI(ui->doubleSpinBoxI->value());
}

void MainWindow::on_doubleSpinBoxD_valueChanged(const QString &arg1)
{
    term.SetD(ui->doubleSpinBoxD->value());
}

void MainWindow::on_doubleSpinBoxSetpoint_valueChanged(const QString &arg1)
{
    term.SetSetpoint(ui->doubleSpinBoxSetpoint->value());
}

void MainWindow::on_comboBoxInput_currentTextChanged(const QString &arg1)
{
    term.SetPortname(arg1);
}

void MainWindow::on_pushButtonSave_clicked()
{
    if(ui->lineEditFilename->text() == nullptr)
        emit response("Wrong!!!");
    else
    {
        QFile file(ui->lineEditFilename->text());
        file.open(QIODevice::WriteOnly);
        QTextStream out(&file);
        for(int i = 0; i < t.size(); i++)
        {
            out << QString(tr("%1").arg(t[i])) << " " << QString(tr("%1").arg(Temp[i])) << "\n";
        }
        file.close();
    }
}

void MainWindow::on_pushButtonChoose_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File with Data"), "",
                                                    tr("Text Files (*.txt);; All Files (*.*)"));
    ui->lineEditImport->setText(fileName);
}

void MainWindow::on_pushButtonImport_clicked()
{
    QFile file(ui->lineEditImport->text());
    if(!file.open(QFile::ReadOnly | QFile::Text)) {
        emit response("Can't open!");
        return;
    }
    QTextStream in(&file);
    QVector<double> xTemp;
    QVector<double> yTemp;
    while(!in.atEnd()) {
        QString temp = in.readLine();
        QStringList temp2 = temp.split(" ");
        xTemp.push_back(temp2[0].toDouble());
        yTemp.push_back(temp2[1].toDouble());
    }
    ui->Plot->graph(0)->setData(xTemp,yTemp);
    ui->Plot->rescaleAxes();
    ui->Plot->replot();
}

void MainWindow::on_actionCommands_triggered()
{
    ui->groupBoxComunication->setVisible(true);
}
