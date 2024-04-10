#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void readOutput();
    void readError();
    void displayNextLine();

private slots:

    void validateSpinBoxValue(int value);
    void on_openPushButton_clicked();

    void on_readPushButton_clicked();

    void on_savePushButton_clicked();

    void on_pushButton_clicked();     

private:
    QProcess process;
    QList<QByteArray> outputBuffer;
    QTimer timer;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
