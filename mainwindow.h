#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "detectThread.h"
#include <QStringList>
#include <QStyleFactory>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void init_sniffer();
    void init_ui();
    void close_sniffer();

signals:
    void send_filter_mode_index(int);

private slots:
    void on_adapterWidget_itemDoubleClicked(QTableWidgetItem *item);
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void receive_packet_info(QVector<QString>);
    void on_filterModeBox_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    detectThread* sniffer_thread;
};
#endif // MAINWINDOW_H
