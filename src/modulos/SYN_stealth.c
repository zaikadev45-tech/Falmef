#include "../include/modulos.h"
//#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
//#include <errno.h>
#include <time.h>
#include <ifaddrs.h>
#include <net/if.h>

// Estrutura para pseudo-cabeçalho (checksum)
struct pseudo_tcp_header {
    u_int32_t src_addr;
    u_int32_t dst_addr;
    u_int8_t zero;
    u_int8_t protocol;
    u_int16_t tcp_len;
    struct tcphdr tcp;
};

// Calcula checksum
unsigned short checksum(unsigned short *ptr, int nbytes) {
    register long sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1) {
        sum += *(unsigned char*)ptr;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

// Função para obter IP local da interface ativa
char* get_local_ip(void) {
    struct ifaddrs *ifaddr, *ifa;
    static char ip[INET_ADDRSTRLEN];
    
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return NULL;
    }
    
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        
        // Pega apenas IPv4, não loopback e interfaces UP
        if (ifa->ifa_addr->sa_family == AF_INET && 
            !(ifa->ifa_flags & IFF_LOOPBACK) &&
            (ifa->ifa_flags & IFF_UP)) {
            
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);
            
            freeifaddrs(ifaddr);
            return ip;
        }
    }
    
    freeifaddrs(ifaddr);
    return "127.0.0.1";  // Fallback
}

int send_syn_packet(int send_sock, const char* src_ip, const char* dst_ip, 
                    int src_port, int dst_port) {
    struct iphdr ip_header;
    struct tcphdr tcp_header;
    struct tcp_mss_option {
        uint8_t kind;
        uint8_t len;
        uint16_t mss;
    } __attribute__((packed)) mss_opt;
    
    char packet[1000]= {0}; //valor padrao: 4096
    struct sockaddr_in dest;

    // Configura IP_HDRINCL
    int opt = 1;
    if (setsockopt(send_sock, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt)) < 0) {
        perror("setsockopt IP_HDRINCL");
        return -1;
    }
    
    // Preenche opção MSS
    mss_opt.kind = 2;
    mss_opt.len = 4;
    mss_opt.mss = htons(1460);
    
    // Preenche cabeçalho TCP (SYN)
    memset(&tcp_header, 0, sizeof(tcp_header));
    tcp_header.source = htons(src_port);
    tcp_header.dest = htons(dst_port);
    tcp_header.seq = htonl(rand() ^ (time(NULL) << 16));
    tcp_header.doff = (sizeof(tcp_header) + sizeof(mss_opt)) / 4;
    tcp_header.syn = 1;
    tcp_header.window = htons(65535);
    
    // Preenche cabeçalho IP
    memset(&ip_header, 0, sizeof(ip_header));
    ip_header.ihl = 5;
    ip_header.version = 4;
    ip_header.tos = 0;
    ip_header.tot_len = htons(sizeof(ip_header) + sizeof(tcp_header) + sizeof(mss_opt));
    ip_header.id = htons(rand() % 65535);
    ip_header.frag_off = 0;
    ip_header.ttl = 64;
    ip_header.protocol = IPPROTO_TCP;
    ip_header.saddr = inet_addr(src_ip); // IP da maquina
    ip_header.daddr = inet_addr(dst_ip);
    
    // Calcula checksum do IP
    ip_header.check = 0;
    ip_header.check = checksum((unsigned short*)&ip_header, sizeof(ip_header));
    
    // Calcula checksum TCP com pseudo-header
    struct {
        u_int32_t src_addr;
        u_int32_t dst_addr;
        u_int8_t zero;
        u_int8_t protocol;
        u_int16_t tcp_len;
    } __attribute__((packed)) pseudo_header;
    
    pseudo_header.src_addr = ip_header.saddr;
    pseudo_header.dst_addr = ip_header.daddr;
    pseudo_header.zero = 0;
    pseudo_header.protocol = IPPROTO_TCP;
    pseudo_header.tcp_len = htons(sizeof(tcp_header) + sizeof(mss_opt));
    
    unsigned long sum = 0;
    unsigned short *ptr;
    
    // Soma pseudo-header
    ptr = (unsigned short*)&pseudo_header;
    for(size_t i = 0; i < sizeof(pseudo_header)/2; i++) {
        sum += ptr[i];
    }
    
    // Soma TCP header
    ptr = (unsigned short*)&tcp_header;
    for(size_t i = 0; i < sizeof(tcp_header)/2; i++) {
        sum += ptr[i];
    }
    
    // Soma opção MSS
    ptr = (unsigned short*)&mss_opt;
    for(size_t i = 0; i < sizeof(mss_opt)/2; i++) {
        sum += ptr[i];
    }
    
    while(sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    tcp_header.check = (unsigned short)(~sum);
    
    // Monta pacote
    memset(packet, 0, sizeof(packet));
    memcpy(packet, &ip_header, sizeof(ip_header));
    memcpy(packet + sizeof(ip_header), &tcp_header, sizeof(tcp_header));
    memcpy(packet + sizeof(ip_header) + sizeof(tcp_header), &mss_opt, sizeof(mss_opt));
    
    // Configura destino
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr(dst_ip);
    
    // Envia pacote
    ssize_t sent = sendto(send_sock, packet, ntohs(ip_header.tot_len), 0,
                          (struct sockaddr*)&dest, sizeof(dest));
    
    if (sent < 0) {
        perror("sendto");
        return -1;
    }
    
    return 0;
}

int SYN_stealth(const char* ip, const int port) {
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL) ^ getpid());
        initialized = 1;
    }
    
    // Obtém IP local real
    const char* local_ip = get_local_ip();
    if (!local_ip) {
        fprintf(stderr, "Erro: Não foi possível obter IP local\n");
        return -1;
    }
    
    // Cria socket de envio
    int send_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (send_sock < 0) {
        perror("socket send_sock");
        return -1;
    }
    
    // Cria socket de recepção
    int recv_sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (recv_sock < 0) {
        perror("socket recv_sock");
        close(send_sock);
        return -1;
    }
    
    // Porta origem aleatória
    int src_port = 20000 + (rand() % 45535);
    
    // Envia SYN
    if (send_syn_packet(send_sock, local_ip, ip, src_port, port) < 0) {
        fprintf(stderr, "Erro ao enviar pacote SYN\n");
        close(send_sock);
        close(recv_sock);
        return -1;
    }
     
    // Configura timeout de 1 segundo
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000;
    
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(recv_sock, &fdset);
    
    char buffer[1000] = {0}; //valor padrao: 4092
    int result = 3;
    
    // Aguarda resposta
    int select_ret = select(recv_sock + 1, &fdset, NULL, NULL, &tv);
    
    if (select_ret > 0) {
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        ssize_t len = recvfrom(recv_sock, buffer, sizeof(buffer), 0,
                               (struct sockaddr*)&from, &fromlen);
        
        if (len > 0) {
            struct iphdr *ip_resp = (struct iphdr*)buffer;
            int ip_header_len = ip_resp->ihl * 4;
            
            // Verifica si é do alvo
            if ((unsigned int)ip_resp->saddr == (unsigned int)inet_addr(ip) && 
                ip_resp->protocol == IPPROTO_TCP) {
                
                struct tcphdr *tcp_resp = (struct tcphdr*)(buffer + ip_header_len);
                
                // Verifica se é resposta é para nossa porta origem
                if (ntohs(tcp_resp->dest) == src_port) {
                	
                    
                    if (tcp_resp->syn && tcp_resp->ack) {
                        result = 1;  // ABERTA
                        
                        
                        // Envia RST para fechar conexão
                        struct sockaddr_in dest;
                        dest.sin_family = AF_INET;
                        dest.sin_addr.s_addr = ip_resp->saddr;
                        
                        // Prepara pacote RST simples
                        struct iphdr rst_ip;
                        struct tcphdr rst_tcp;
                        
                        memset(&rst_ip, 0, sizeof(rst_ip));
                        rst_ip.ihl = 5;
                        rst_ip.version = 4;
                        rst_ip.tot_len = htons(sizeof(rst_ip) + sizeof(rst_tcp));
                        rst_ip.id = htons(rand() % 65535);
                        rst_ip.ttl = 64;
                        rst_ip.protocol = IPPROTO_TCP;
                        rst_ip.saddr = inet_addr(local_ip);
                        rst_ip.daddr = ip_resp->saddr;
                        rst_ip.check = checksum((unsigned short*)&rst_ip, sizeof(rst_ip));
                        
                        memset(&rst_tcp, 0, sizeof(rst_tcp));
                        rst_tcp.source = htons(src_port);
                        rst_tcp.dest = tcp_resp->source;
                        rst_tcp.seq = tcp_resp->ack_seq;
                        rst_tcp.ack_seq = 0;
                        rst_tcp.doff = 5;
                        rst_tcp.rst = 1;
                        
                        // Calcula checksum TCP do RST
                        struct {
                            u_int32_t src_addr;
                            u_int32_t dst_addr;
                            u_int8_t zero;
                            u_int8_t protocol;
                            u_int16_t tcp_len;
                        } __attribute__((packed)) rst_pseudo;
                        
                        rst_pseudo.src_addr = rst_ip.saddr;
                        rst_pseudo.dst_addr = rst_ip.daddr;
                        rst_pseudo.zero = 0;
                        rst_pseudo.protocol = IPPROTO_TCP;
                        rst_pseudo.tcp_len = htons(sizeof(rst_tcp));
                        
                        unsigned long rst_sum = 0;
                        unsigned short *ptr;
                        
                        ptr = (unsigned short*)&rst_pseudo;
                        for(size_t i = 0; i < sizeof(rst_pseudo)/2; i++) rst_sum += ptr[i];
                        ptr = (unsigned short*)&rst_tcp;
                        for(size_t i = 0; i < sizeof(rst_tcp)/2; i++) rst_sum += ptr[i];
                        while(rst_sum >> 16) rst_sum = (rst_sum & 0xffff) + (rst_sum >> 16);
                        rst_tcp.check = (unsigned short)(~rst_sum);
                        
                        // Monta e envia RST
                        char rst_packet[sizeof(rst_ip) + sizeof(rst_tcp)];
                        memcpy(rst_packet, &rst_ip, sizeof(rst_ip));
                        memcpy(rst_packet + sizeof(rst_ip), &rst_tcp, sizeof(rst_tcp));
                        sendto(send_sock, rst_packet, sizeof(rst_packet), 0,
                               (struct sockaddr*)&dest, sizeof(dest));
                        
                    } else if (tcp_resp->rst && tcp_resp->ack) {
                        result = 0;  // FECHADA
                    } else if (tcp_resp->rst) {
                        result = 0;  // FECHADA
                                           }
                }
            }
        }
    } else if (select_ret == 0) {
        result = 2; //FILTRADA
    } else {
        perror("select");
    }
    
    close(send_sock);
    close(recv_sock);
    return result;
}
