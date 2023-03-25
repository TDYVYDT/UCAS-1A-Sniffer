#ifndef SNIFFER_H
#define SNIFFER_H

#include <QDebug>
#include <ws2tcpip.h>
#include "header.h"
#include <QByteArray>
#include <winsock2.h>

#define IPTOSBUFFERS    12

#pragma comment(lib, "ws2_32.lib")

class sniffer {
public:

    sniffer();
    ~sniffer();
    char* iptos(u_long);
    QString ucharToQString(const u_char*, int);
    void _get_adapter_info(pcap_if_t*);
    int get_adapter_info();
    void set_adapter_info(int);
    void set_filter();
    QString get_ip_address(ip_v4_address);
    QString get_ip_address(ip_v6_address);
    QString parse_mac_header();
    QString parse_ipv4_header();
    QString parse_ipv6_header();
    QString parse_arp_header();
    QString parse_tcp_header();
    QString parse_udp_header();
    QString parse_icmp_header();

    QVector<std::string> filter_mode = {
        "ip or arp",
        "tcp",
        "udp",
        "arp",
        "icmp"
    };

public:
    QVector<QVector<QString>> adapter_info;     // 网卡信息
    pcap_if_t* all_devs;                        // all_devs存储查找到的设备链表的链表头
    pcap_if_t* dev;                             // dev为遍历all_devs链表时的游标
    int index;                                  // 设备序号，在后面的循环中会用到
    int cur_index;                              // 当前设备序号
    char err_buf[PCAP_ERRBUF_SIZE];             // 存储错误信息
    pcap_t* handle;
    struct pcap_pkthdr* header;
    const u_char* pkt_data;
    struct bpf_program fcode;
    unsigned int netmask;
    char packet_filter[100];
    QVector<QString> header_info;               // 0: src ip, 1: dst ip, 2: time, 3:protocol, 4:size, 5:info
    int mode_index;
};

#endif // SNIFFER_H
