//#include <linux/if.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

typedef struct
{
	
	int tipo;
	// 1 = com porta
	// 2 = sem porta
	
	//modos:
	bool verbose; // mostra portas fechadas
	bool AllPort; // scan de 1 ate 65355
	bool cli; // retorna valores
	
	//tipos de scan:
	//bool ping;
	//bool arp
} config;

int mini_falmef(const char* ip,const int port)
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
		// status da porta
		if(so_error == 0) {resultado = 1;}
		else if(so_error == 111) {resultado = 0;}
		else {resultado = 2;}
		// debug basico printf("codigo do so_error: %d", so_error);
    }
    else if(status_sock == 0) {resultado = 2;}
	else {resultado = 0;}
	
    close(sock);
    return resultado;
	/* 
	Anotações
	so_error 111 = porta fechada
	so_error 0 = porta aberta
	2 seria o firewall
	*/
}

bool valid_ip(const char ip[]);
void post(int port, int status, bool verbose);

int main(int argc, char *argv[])
{
	char ip[16] = {0};
    int port = 0;
	
	config cf = {0};
	cf.verbose = false;
	cf.AllPort = false;
	cf.cli = false;
	
	const int PortSensi[29] =
	{
		//PORTAS WEB
		80, 422, 8080, 8443,
		
		//PORTAS SSH
		22, 2222,
		
		//PORTAS FTP
		21, 20, 990,
		
		//PORTAS DE EMAIL
		25, 110, 143, 465, 587, 993, 995,
		
		//PORTAS do BANCO DE DADOS
		3306, 5432, 1433, 27017, 6379,
		
		23, // telnet
		53, // dns
		161, // snmp
		389, // ldap
		445, // smp
		3309, // rdp
		5900, // vnc
		5555, // adb wifi
	}; // adicionar mais portas precisa aumentar o 28
	
	for(int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "--cli") == 0)
		{
			cf.cli = true;
		}
		else if(strcmp(argv[i], "-v") == 0)
		{
			cf.verbose = true;
		}
		else if(strcmp(argv[i], "-a") == 0)
		{
			cf.AllPort = true;
		}
		else if(strlen(argv[i]) >= 5)
		{
			if(sscanf(argv[i], "%[^:]:%d", ip, &port) == 2)
			{
				if(!valid_ip(ip))
				{
					printf("IP INVALIDO\n exemplo: 000.000.000.000");
					return -5;
				}
				cf.tipo = 1;
			}
			else if(sscanf(argv[i], "%[^:]", ip) == 1)
			{
				cf.tipo = 2;
			}
			//**/printf("ip: %s, porta: %d", ip, port);
		}
		printf("arg: %s", argv[i]);
	}
	printf("\n Falmef feito por zaikadev45-tech \n");
	printf("	https://github.com/zaikadev45-tech \n");

	/* debug = */printf("porta: %d \n\n", port);
	
	if(cf.AllPort == true)
	{
		for(port = 1; port < 65355; port++)
		{
			int status = mini_falmef(ip, port);
			post(port, status, cf.verbose);
		}
	}
	else if(cf.tipo == 1)
	{
		int status = mini_falmef(ip, port);
		post(port, status, cf.verbose);
		if(cf.cli)
		{
			if(status == 1) return 1;
			if(status == 0) return 0;
			if(status == 2) return 2;
		}
	}
	else if(cf.tipo == 2)
	{
		for(int i = 0; i < 29; i++) //29 = PortSensi[] <<
		{
			int status = mini_falmef(ip, PortSensi[i]);
			post(PortSensi[i], status, cf.verbose);
		}
	}
}

void post(int port, int status, bool verbose)
{
	if(status == 1)
	{
		printf("[+] Porta %d Aberta\n", port);
	}
	else if(status == 0 && verbose == true)
	{
		printf("[-] Porta %d Fechada\n", port);
	}
	else if(status == 2)
	{
		printf("[=] Porta %d Filtrada\n", port);
	}
}

bool valid_ip(const char ip[])
{
	int a, b, c, d;
	if(sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) == 4)
	{
		if(a >= 0 && a <= 255 &&
		   b >= 0 && b <= 255 &&
		   c >= 0 && c <= 255 &&
		   d >= 0 && d <= 255)
		{
			return true;
		}
	}
	return false;
}

