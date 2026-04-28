#include "../include/modulos.h"

#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>

int TCP_connect(const char* ip,const int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {return 0;}

    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in alvo;
    alvo.sin_family = AF_INET;
    alvo.sin_port = htons(port);
    alvo.sin_addr.s_addr = inet_addr(ip);

    connect(sock, (struct sockaddr*)&alvo, sizeof(alvo));

    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);

    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    int resultado = 0;
    int status_sock = select(sock + 1, NULL, &fdset, NULL, &tv);

    if(status_sock == 1)
    {
        int so_error = {0};
        socklen_t len = sizeof(so_error);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
        // status da cf.porta
        if(so_error == 0) {resultado = 1;}
        else if(so_error == 111) {resultado = 0;}
        else {resultado = 2;}
    }
    else if(status_sock == 0) {resultado = 2;}
    else {resultado = 0;}

    close(sock);
    return resultado;
    // ===== Anotações =====
    // so_error 111 = cf.porta fechada
    // so_error 0 = cf.porta aberta
    // timeouf 2 = filtrada
}
