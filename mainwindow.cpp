#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QLabel>
#include <QSlider>
#include <QObject>
#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <QThread>
#include <QByteArray>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
   //
    //setFixedSize(sizeHint());
   // setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
    //setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint);
    setFixedSize(size());
    ui->slider->setMinimum(0);
    ui->slider->setMaximum(247);
    ui->spinBox->setMinimum(0);
    ui->spinBox->setMaximum(247);
    ui->spinBox->setSingleStep(19);
    ui->slider->setSingleStep(19);
    QObject::connect(ui->spinBox, QOverload<int>::of(&QSpinBox::valueChanged), ui->slider, &QSlider::setValue);
    QObject::connect(ui->slider, &QSlider::valueChanged, ui->spinBox, &QSpinBox::setValue);
    QObject::connect(ui->spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::validateSpinBoxValue);
    connect(&process, &QProcess::readyReadStandardOutput, this, &MainWindow::readOutput);
    connect(&process, &QProcess::readyReadStandardError, this, &MainWindow::readError);
    connect(&timer, &QTimer::timeout, this, &MainWindow::displayNextLine);
}

MainWindow::~MainWindow()
{


    delete ui;
}

void MainWindow::validateSpinBoxValue(int value) {
    if (value % 19 != 0) {
        int nearestMultiple = (value / 19) * 19;
        ui->spinBox->setValue(nearestMultiple);
    }
}

QString fileContents;
QString lastNumber;
QString firstNumber;

void MainWindow::on_openPushButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Text File", "", "Text files (*.txt)");

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Error", "Could not open file.");
            return;
        }

        QTextStream in(&file);
        fileContents = in.readAll();
        file.close();


        ui->filePathLabel->setText("File: " + filePath);
    } else {
        QMessageBox::information(this, "Information", "No file selected.");
    }
}


void MainWindow::on_readPushButton_clicked()
{
    bool ok;
    //ui->textBrowser->setPlainText(fileContents);
    firstNumber = fileContents.mid(306, 2);
    lastNumber = fileContents.mid(363, 2);
    qDebug() << "first Number "<<firstNumber;
    qDebug() << "last Number "<<lastNumber;
    qDebug() << "check";
    ui->slider->setValue(lastNumber.toInt(&ok, 16));
    ui->spinBox->setValue(lastNumber.toInt(&ok, 16));
}


void MainWindow::on_savePushButton_clicked()
{
    bool ok;
    int result;
    QString hexResult;
    int Ksb;
    int Kn;
    int i;
    int decValue;
    int k=0;
    Ksb = ui->spinBox->value()/19;
    Kn = fileContents.mid(309,2).toInt(&ok, 16) - fileContents.mid(306,2).toInt(&ok, 16);
    if (Kn!=Ksb) {
        if (Ksb==0) {
            for (i=0;i<=3*19; i+=3){
                fileContents.replace(306 + i, 2, "00");
            }
        } else
        if (Kn != 0) {
            for (i=0;i<=3*19; i+=3){
                decValue = fileContents.mid(306+i, 2).toInt(&ok, 16);
                hexResult = QString::number(decValue/Kn*Ksb, 16).toUpper();
                qDebug()<<"hexResult is = "<< hexResult;
                if (hexResult.length() == 1) {
                    hexResult.prepend('0');
                }
                fileContents.replace(306 + i, 2, hexResult);
            }

        } else {
            for (i=0; i<=3*19; i+=3) {

                hexResult = QString::number(k*Ksb, 16).toUpper();
                if (hexResult.length() == 1) {
                    hexResult.prepend('0');
                }
                fileContents.replace(306 + i, 2, hexResult);
                k++;
            }
        }

    }

   /* for (i = 0; i<=3*19; i+=3) {
        qDebug() << "i = " << i <<", fileContents.mid("<<306+i<<", 2) = " << fileContents.mid(306+i, 2)<<")";
        decValue = fileContents.mid(306+i, 2).toInt(&ok, 16);
        result = decValue * ui->spinBox->value()/19;
        qDebug()<<"decValue var is "<<decValue<<", result is "<<result;
        hexResult = QString::number(result, 16).toUpper();
        qDebug()<<"result in hex is "<< hexResult;
        if (hexResult.length() == 1) {
            hexResult.prepend('0');
        }
        fileContents.replace(306 + i, 2, hexResult);
        qDebug()<<"after all, fileContents.mid("<<306+i<<", 2) = "<< fileContents.mid(306+i,2);
        qDebug()<<"and, fileContents.mid("<<306+i<<", 5) = "<< fileContents.mid(306+i,5);
    }
    */
    //ui->textBrowser->setPlainText(fileContents);


    QString filePath = ui->filePathLabel->text().remove("File: ");

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);
    out << fileContents;

    file.close();

    QMessageBox::information(this, "Information", "File saved successfully.");

}

void MainWindow::readOutput() {
    QByteArray output = process.readAllStandardOutput();
    outputBuffer.append(output.split('\n'));

    // Start displaying lines if not already started
    if (!timer.isActive()) {
        displayNextLine();
    }
}

void MainWindow::readError() {
    // Check if there are lines to display
    if (!outputBuffer.isEmpty()) {
        // Display the next line in the QTextBrowser
        ui->textBrowser->append(QString::fromUtf8(outputBuffer.takeFirst()));

        // Start the timer again for the next line
        timer.start(1000);
    }
}

void MainWindow::displayNextLine() {
    // Check if there are lines to display
    if (!outputBuffer.isEmpty()) {
        // Display the next line in the QTextBrowser
        ui->textBrowser->append(QString::fromUtf8(outputBuffer.takeFirst()));

        // Start the timer again for the next line
        timer.start(1000);
    }
}



void MainWindow::on_pushButton_clicked()
{
    ui->textBrowser->setPlainText(ui->filePathLabel->text().remove("File: ")+"\n");
    // Create a QProcess instance
   /* QProcess process;

    // Set the command to start cmd.exe and run the ping command
    QStringList arguments;
    arguments << "/c" << "ping 127.0.0.1 -n 1"; // "-n 1" means ping only once
    process.start("cmd.exe", arguments);

    // Start reading output
    connect(&process, &QProcess::readyReadStandardOutput, [=, &process]() {
        QByteArray output = process.readAllStandardOutput();
        ui->textBrowser->append(QString::fromLocal8Bit(output));
    });

    // Start reading error output
    connect(&process, &QProcess::readyReadStandardError, [=, &process]() {
        QByteArray error = process.readAllStandardError();
        ui->textBrowser->append(QString::fromLocal8Bit(error));
    });

    // Wait for the process to finish
    process.waitForFinished(-1);

    // Check the exit code
    if (process.exitCode() == 0) {
        ui->textBrowser->append("Ping completed successfully.");
    } else {
        ui->textBrowser->append("Ping failed.");
    }
    */

    QString program = "cmd.exe";
    QStringList arguments;
    arguments << "/c" << "ping 127.0.0.1 -n 1"; // "-n 1" means ping only once

    // Start the process
    process.start(program, arguments);


    // Start a timer with a 1-second interval
    timer.start(1000);
}



