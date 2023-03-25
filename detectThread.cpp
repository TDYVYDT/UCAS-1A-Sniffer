#include "detectThread.h"

detectThread::detectThread(QObject* parent) : QThread(parent), is_lock(false), close(false) {

}

detectThread::~detectThread() {
    this->exit();
    this->quit();
    this->terminate();
}

void detectThread::run() {
    int ret;
    while((ret = pcap_next_ex(my_sniffer.handle, &my_sniffer.header, &my_sniffer.pkt_data)) >= 0) {
        if(ret == 0)
            continue;
        if(close)
            break;
        if(is_lock)
            my_mutex.lock();
        struct tm* ltime;
        char time_str[16];
        time_t local_tv_sec;
        local_tv_sec = my_sniffer.header->ts.tv_sec;
        ltime = localtime(&local_tv_sec);
        strftime(time_str, sizeof (time_str), "%H:%M:%S", ltime);
        my_sniffer.header_info[2] = QString(time_str);
        my_sniffer.header_info[4] = QString::number(my_sniffer.header->len);
        my_sniffer.header_info[5] = my_sniffer.parse_mac_header();
        emit send_packet_info(my_sniffer.header_info);
        msleep(50);
    }
}
