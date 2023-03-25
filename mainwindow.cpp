#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , sniffer_thread(new detectThread(this)) {
    ui->setupUi(this);

    init_ui();
    init_sniffer();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::init_ui() {
    QApplication::setStyle( QStyleFactory::create("Fusion"));
    QStringList labels;
    labels << "dev name" << "dev discription" << "IP address" << "netmask" << "broadcast address" << "destination IP address";
    ui->adapterWidget->setColumnCount(6);
    ui->adapterWidget->setRowCount(10);
    ui->adapterWidget->setHorizontalHeaderLabels(labels);
    ui->adapterWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);     //横向先自适应宽度
    ui->adapterWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);  //然后设置要根据内容使用宽度的列
    ui->adapterWidget->setSelectionBehavior(QTableWidget::SelectRows);
    ui->adapterWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QStringList packet_labels;
    packet_labels << "source IP address" << "destination IP address" << "time" << "protocol" << "length" << "info";
    ui->analyseWidget->setColumnCount(6);
    ui->analyseWidget->setRowCount(0);
    ui->analyseWidget->setHorizontalHeaderLabels(packet_labels);
    ui->analyseWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);     //横向先自适应宽度
    ui->analyseWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);  //然后设置要根据内容使用宽度的列
    ui->analyseWidget->setSelectionBehavior(QTableWidget::SelectRows);
    ui->analyseWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->filterModeBox->addItem("Default");
    ui->filterModeBox->addItem("Tcp");
    ui->filterModeBox->addItem("Udp");
    ui->filterModeBox->addItem("Arp");
    ui->filterModeBox->addItem("Icmp");
}

void MainWindow::init_sniffer() {
    if(sniffer_thread->my_sniffer.get_adapter_info() == -1)
        exit(0);
    for(int i = 0; i < sniffer_thread->my_sniffer.index; ++i) {
        for(int j = 0; j < sniffer_thread->my_sniffer.adapter_info[i].size(); ++j) {
            QTableWidgetItem* item = new QTableWidgetItem;
            item->setText(sniffer_thread->my_sniffer.adapter_info[i][j]);
            ui->adapterWidget->setItem(i, j, item);
        }
    }
    connect(sniffer_thread, SIGNAL(send_packet_info(QVector<QString>)), this, SLOT(receive_packet_info(QVector<QString>)), Qt::BlockingQueuedConnection);
}

void MainWindow::close_sniffer() {
    sniffer_thread->close = true;
    sniffer_thread->exit();
    sniffer_thread->deleteLater();
}

void MainWindow::on_adapterWidget_itemDoubleClicked(QTableWidgetItem *item) {
    int row = item->row();
    sniffer_thread->my_sniffer.set_adapter_info(row);
    QString text = "dev name:\t" + sniffer_thread->my_sniffer.adapter_info[row][0] + "\n"
            "dev description:\t" + sniffer_thread->my_sniffer.adapter_info[row][1] + "\n";
    ui->currentAdapterEdit->setPlainText(text);
}

void MainWindow::on_pushButton_clicked() {
    sniffer_thread->my_sniffer.mode_index = ui->filterModeBox->currentIndex();
    sniffer_thread->my_sniffer.set_filter();
    sniffer_thread->start();
}

void MainWindow::on_pushButton_2_clicked() {
    if(ui->pushButton_2->text() == "stop") {
        sniffer_thread->is_lock = true;
        ui->pushButton_2->setText("continue");
    }
    else {
        sniffer_thread->is_lock = false;
        sniffer_thread->my_mutex.unlock();
        ui->pushButton_2->setText("stop");
    }
}

void MainWindow::on_pushButton_3_clicked() {
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_pushButton_4_clicked() {
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::receive_packet_info(QVector<QString> info) {
    qDebug() << __FUNCTION__;
    int row_count = ui->analyseWidget->rowCount();
    ui->analyseWidget->insertRow(row_count);
    for(int i = 0; i < info.size() ; ++i) {

        QTableWidgetItem* item = new QTableWidgetItem;
        item->setText(info[i]);
        if(info[3] == "Tcp")
            item->setBackground(QBrush(QColor("#FFFF99")));
        else if(info[3] == "Udp")
            item->setBackground(QBrush(QColor("#FF6666")));
        else if(info[3] == "Arp")
            item->setBackground(QBrush(QColor("#99FF99")));
        else
            item->setBackground(QBrush(QColor("#FFA07A")));
        ui->analyseWidget->setItem(row_count, i, item);
    }
}

void MainWindow::on_filterModeBox_currentIndexChanged(int index) {
    sniffer_thread->my_sniffer.mode_index = index;
    sniffer_thread->my_sniffer.set_filter();
}

