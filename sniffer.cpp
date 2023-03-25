#include "sniffer.h"

sniffer::sniffer() : all_devs(nullptr), dev(nullptr), index(0), cur_index(0), handle(nullptr), mode_index(0) {
    header_info = QVector<QString>(6);
}

sniffer::~sniffer() {

}

char* sniffer::iptos(u_long in) {
    static char output[IPTOSBUFFERS][3*4+3+1];
    static short which;
    u_char *p;

    p = (u_char *)&in;
    which = (which + 1 == IPTOSBUFFERS ? 0 : which + 1);
    sprintf(output[which], "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
    return output[which];
}

QString sniffer::ucharToQString(const u_char* data, int length) {
    QString temp, msg;
    int i = 0;
    while (i < length) {
        temp = QString("%1").arg((int)data[i], 2, 16, QLatin1Char('0'));
        msg.append(temp);
        i++;
    }
    return msg;
}

void sniffer::_get_adapter_info(pcap_if_t* dev) {
    pcap_addr_t* addr;
    QVector<QString> info;
    QVector<QString> ip_addr;
    // 设备名
    info.push_back(dev->name);
    // 设备描述
    if(dev->description)
        info.push_back(dev->description);
    else
        info.push_back("");

    for(addr = dev->addresses; addr; addr = addr->next) {
        switch (addr->addr->sa_family) {
        case AF_INET:
            if (addr->addr)
                info.push_back(iptos(((struct sockaddr_in *)addr->addr)->sin_addr.s_addr));
            else
                info.push_back("");
            if (addr->netmask)
                info.push_back(iptos(((struct sockaddr_in *)addr->netmask)->sin_addr.s_addr));
            else
                info.push_back("");
            if (addr->broadaddr)
                info.push_back(iptos(((struct sockaddr_in *)addr->broadaddr)->sin_addr.s_addr));
            else
                info.push_back("");
            if (addr->dstaddr)
                info.push_back(iptos(((struct sockaddr_in *)addr->dstaddr)->sin_addr.s_addr));
            else
                info.push_back("");
            break;

        case AF_INET6:
            break;

        default:
            break;
        }
    }
    adapter_info.push_back(info);
}

int sniffer::get_adapter_info() {
    // 如果查找失败，程序直接终止
    if (pcap_findalldevs(&all_devs, err_buf) == -1)
        return -1;
    for(dev = all_devs; dev; dev = dev->next) {
        ++index;
        if(index == 0) {
            qDebug("没有有效接口");
            return -1;
        }
        _get_adapter_info(dev);
    }
    //pcap_freealldevs(all_devs);
    return 0;
}

void sniffer::set_adapter_info(int index) {
    cur_index = index;
    int i = 0;
    for(dev = all_devs; dev && i < cur_index; dev = dev->next, ++i);
    if((handle = pcap_open_live(dev->name, 65535, PCAP_OPENFLAG_PROMISCUOUS, 1000, err_buf)) == nullptr) {
        pcap_freealldevs(all_devs);
    }
}

void sniffer::set_filter() {
    if(all_devs == nullptr)
        return;
    if (dev->addresses != NULL)
        /* 获取接口第一个地址的掩码 */
        netmask = ((struct sockaddr_in *)(dev->addresses->netmask))->sin_addr.S_un.S_addr;
    else
        /* 如果这个接口没有地址，那么我们假设这个接口在C类网络中 */
        netmask = 0xffffff;
    if(pcap_compile(handle, &fcode, filter_mode[mode_index].c_str(), 1, netmask) >= 0) {
        //设置过滤器
        if (pcap_setfilter(handle, &fcode) < 0)
        {
            fprintf(stderr,"\nError setting the filter.\n");
            /* 释放设备列表 */
            pcap_freealldevs(all_devs);
            return;
        }
    }
    else {
        fprintf(stderr,"\nError setting the filter.\n");
        /* 释放设备列表 */
        pcap_freealldevs(all_devs);
        return;
    }
}

QString sniffer::get_ip_address(ip_v4_address address) {
    return QString::number(address.byte1) + "." + QString::number(address.byte2) + "." +
            QString::number(address.byte3) + "." + QString::number(address.byte4);
}

QString sniffer::get_ip_address(ip_v6_address address) {
    QString byte1 = QString("%1").arg(address.part1, 4, 16, QLatin1Char('0'));
    QString byte2 = QString("%1").arg(address.part2, 4, 16, QLatin1Char('0'));
    QString byte3 = QString("%1").arg(address.part3, 4, 16, QLatin1Char('0'));
    QString byte4 = QString("%1").arg(address.part4, 4, 16, QLatin1Char('0'));
    QString byte5 = QString("%1").arg(address.part5, 4, 16, QLatin1Char('0'));
    QString byte6 = QString("%1").arg(address.part6, 4, 16, QLatin1Char('0'));
    QString byte7 = QString("%1").arg(address.part7, 4, 16, QLatin1Char('0'));
    QString byte8 = QString("%1").arg(address.part8, 4, 16, QLatin1Char('0'));
    return byte1 + ":" + byte2 + ":" + byte3 + ":" + byte4 + ":" +
            byte5 + ":" + byte6 + ":" + byte7 + ":" + byte8;
}

QString sniffer::parse_mac_header() {
    qDebug() << __FUNCTION__;
    QString info;
    ethernet_header* eh = (ethernet_header*) pkt_data;
    u_short type = ntohs(eh->type);
    switch (type) {
    case 0x0800:
        info += parse_ipv4_header();
        break;
    case 0x0806:
        info += parse_arp_header();
        break;
    case 0x86DD:
        info += parse_ipv6_header();
        break;
    default:
        break;
    }
    return info;
}

QString sniffer::parse_arp_header() {
    qDebug() << __FUNCTION__;
    header_info[3] = "Arp";
    QString info;
    arp_header* ah;
    ah = (arp_header*)(pkt_data + 14);
    u_short operation_code = ntohs(ah->operation_code);
    switch (operation_code) {
    case 1:
        info += "ARP Request Protocol";
        break;
    case 2:
        info += "ARP Response Protocol";
        break;
    case 3:
        info += "ARP Request Protocol";
            break;
    case 4:
        info += "ARP Request Protocol";
        break;
    default:
        break;
    }
    return info;
}

QString sniffer::parse_ipv4_header() {
    qDebug() << __FUNCTION__;
    QString info;
    ip_v4_header *ih;
    ih = (ip_v4_header*)(pkt_data + 14); //14 measn the length of ethernet header
    header_info[0] = get_ip_address(ih->src_ip_addr);
    header_info[1] = get_ip_address(ih->des_ip_addr);
    switch (ih->proto) {
    case 6:
        info += parse_tcp_header();
        break;
    case 17:
        info += parse_udp_header();
        break;
    case 1:
        info += parse_icmp_header();
    default:
        break;
    }
    return info;
}

QString sniffer::parse_ipv6_header() {
    QString info;
    ip_v6_header *ih;
    ih = (ip_v6_header*)(pkt_data + 14);
    header_info[0] = get_ip_address(ih->src_ip_addr);
    header_info[1] = get_ip_address(ih->dst_ip_addr);
    int version = (ih->ver_trafficclass_flowlabel & 0xf0000000) >> 28;
    int traffic_class = ntohs((ih->ver_trafficclass_flowlabel & 0x0ff00000) >> 20);
    int flow_label = ih->ver_trafficclass_flowlabel & 0x000fffff;
    switch (ih->next_head) {
    case 6:
        info += parse_tcp_header();
            break;
    case 17:
        info += parse_udp_header();
        break;
    case 58:
        info += parse_icmp_header();
        break;
    default:
        break;
    }
    return info;
}

QString sniffer::parse_tcp_header() {
    qDebug() << __FUNCTION__;
    header_info[3] = "Tcp";
    QString info;
    tcp_header* th;
    th = (tcp_header*)(pkt_data + 14 + 20);
    info += "src port = " + QString::number(th->sport) + " ";
    info += "dst port = " + QString::number(th->dport) + " ";
    if (th->flags & 0x01)
        info += "FIN = 1 ";
    if (th->flags & 0x02)
        info += "SYN = 1 ";
    if (th->flags & 0x04)
        info += "RST = 1 ";
    if (th->flags & 0x08)
        info += "PSH = 1 ";
    if (th->flags & 0x10)
        info += "ACK = 1 ";
    if (th->flags & 0x20)
        info += "URG = 1 ";
    info += "seq = " + QString::number(th->sequence) + " ";
    info += "ack = " + QString::number(th->acknowledgement);
    return info;
}

QString sniffer::parse_udp_header() {
    qDebug() << __FUNCTION__;
    header_info[3] = "Udp";
    QString info;
    udp_header *uh;
    uh = (udp_header *)(pkt_data + 20 + 14);
    info += "src port = " + QString::number(uh->sport) + " ";
    info += "dst port = " + QString::number(uh->dport) + " ";
    info += "length = " + QString::number(uh->len) + " ";
    info += "checksum = " + QString::number(uh->checksum);
    return info;
}

QString sniffer::parse_icmp_header() {
    qDebug() << __FUNCTION__;
    header_info[3] = "Icmp";
    QString info;
    icmp_header* ih;
    ih = (icmp_header*)(pkt_data + 14 + 20);
    info += "type = " + QString::number(ih->type) + " ";
    info += "seq = " + QString::number(ih->sequence) + " ";
    info += "checksum = " + QString::number(ih->checksum);
    return info;
}
