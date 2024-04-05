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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->slider->setMinimum(0);
    ui->slider->setMaximum(255);
    ui->spinBox->setMinimum(0);
    ui->spinBox->setMaximum(255);
    QObject::connect(ui->spinBox, QOverload<int>::of(&QSpinBox::valueChanged), ui->slider, &QSlider::setValue);
    QObject::connect(ui->slider, &QSlider::valueChanged, ui->spinBox, &QSpinBox::setValue);

}

MainWindow::~MainWindow()
{

    delete ui;
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
    ui->textBrowser->setPlainText(fileContents);
    firstNumber = fileContents.mid(309, 2);
    lastNumber = fileContents.mid(363, 2);
    qDebug() << "first Number "<<firstNumber;
    qDebug() << "last Number "<<lastNumber;
    ui->slider->setValue(lastNumber.toInt(&ok, 16));
    ui->spinBox->setValue(lastNumber.toInt(&ok, 16));
}

