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
    } /*else {
        QMessageBox::information(this, "Information", "No file selected.");
    }
*/
}


void MainWindow::on_readPushButton_clicked()
{
    bool ok;
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
    QList<QByteArray> lines = output.split('\n');

    for (const QByteArray& line : lines) {
        QByteArray trimmedLine = line.trimmed();
        if ((trimmedLine.startsWith("Reply from") || trimmedLine.startsWith("Pinging"))) {
            outputBuffer.append(trimmedLine);
        }
    }

    if (!timer.isActive()) {
        displayNextLine();
    }
}

void MainWindow::readError() {
    if (!outputBuffer.isEmpty()) {
        ui->textBrowser->append(QString::fromUtf8(outputBuffer.takeFirst()));
        timer.start(1000);
    }
}

void MainWindow::displayNextLine() {
    if (!outputBuffer.isEmpty()) {
        ui->textBrowser->append(QString::fromUtf8(outputBuffer.takeFirst()));
        timer.start(1000);
    }
}



void MainWindow::on_pushButton_clicked()
{
    ui->textBrowser->setPlainText(ui->filePathLabel->text().remove("File: ")+"\n");
    QStringList arguments;
    arguments << "/c" << "ping 127.0.0.1 -n 1";
    process.start("cmd.exe", {"/c", "ping 127.0.0.1 -n 1"});
    timer.start(1000);
}



