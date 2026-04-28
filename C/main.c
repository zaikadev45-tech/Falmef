#include "include/input.h"
#include "include/modulos.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

void post(int port, int status, bool verbose);
int select_modulo(const char *ip, const int port, config *cf);

int main(int argc, char *argv[])
{
	/* ===== Processar argumentos =====*/
	config cf = processInput(argc, argv);

	/* ===== Verificar os argumentos ===== */
	if(cf.Error == true)
	{
		printf("\n Erro no input do usuário \n");
		return -5;
	}
	
	const int PortSensi[] =
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
	};

	const int PortSensi_num = sizeof(PortSensi) / sizeof(PortSensi[0]);
	
	int status = 0;
	if(cf.modo == 1)
	{
		if(cf.AllPort == true)
		{
			for(int i = 1; i <= 65535; i++)
			{
				status = select_modulo(cf.ip, i, &cf);
				post(i, status, cf.verbose);
			}
			return 0;
		} else
		{
			for(int i = 0; i < PortSensi_num; i++)
			{
				status = select_modulo(cf.ip, PortSensi[i], &cf);
				post(PortSensi[i], status, cf.verbose);
			}
		}
	}
	else if(cf.modo == 2)
	{
		for(int i = 0; i < cf.cout_port; i++)
		{
			status = select_modulo(cf.ip, cf.port[i], &cf);
			if(cf.cli == true)
			{
				if(status == 0) return 0;
				else if(status == 1) return 1;
				else if(status == 2) return 2;
			}
			post(cf.port[i], status, cf.verbose);
		}
		free(cf.port);
	}
	return -1;
}

void post(const int port, const int status, const bool verbose)
{
    if(status == 1) {
        printf("[+] Porta %d: ABERTA\n", port);
    } 
    else if(status == 0 && verbose) {
        printf("[-] Porta %d: FECHADA\n", port);
    }
    else if(status == 2) {
        printf("[?] Porta %d: FILTRADA\n", port);
    }
}

int select_modulo(const char * ip, const int port, config *cf) 
{
	usleep(300000);
	if(cf->TCP_CONNECT)
	{
		return TCP_connect(ip, port);
	}
	else if(cf->SYN)
	{
		return SYN_stealth(ip, port);
	}
	return -5;
}
