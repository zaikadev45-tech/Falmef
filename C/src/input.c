#include "../include/input.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
config processInput(int argc, char *argv[])
{
	config cf = {0};

	/* ====== dados default ====== */

    cf.verbose = false;
	cf.AllPort = false;
	cf.cli = false;

	//Scan default
	cf.TCP_CONNECT = true;
	cf.SYN = false;

	/* ===== verificar input ===== */
	for(int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			help();
			return cf;
		}
		else if(strcmp(argv[i], "--cli") == 0)
		{
			cf.cli = true;
		}
		// Modo Verbose
		else if(strcmp(argv[i], "-v") == 0)
		{
			cf.verbose = true;
		}
		//modo All Port
		else if(strcmp(argv[i], "-a") == 0)
		{
			cf.AllPort = true;
		}
		// == Tipos de Scan == //
		//
		// Modo SYN-Stealth
		else if(strcmp(argv[i], "-sY") == 0)
		{
			if(RootAcesse(&cf) == false)
			{
				cf.TCP_CONNECT = false;
				cf.SYN = true;
			}
		}
		// específicar porta
		else if(strcmp(argv[i], "-p") == 0)
		{
			cf.modo = 2;
			cf.verbose = true;

			int cout = 0; //quantas portas
			int j = i + 1;
			while(j < argc && argv[j][0] != '=')
			{
				cout++;
				j++;
			}

			//alocar portas para memoria
			cf.port = malloc(cout * sizeof(int));
			cf.cout_port = 0;

			//guarda portas
			while(i + 1 < argc && argv[i+1][0] != '-')
			{
				i++;
				cf.port[cf.cout_port] = atoi(argv[i]);
				cf.cout_port++;
			}
		}
		//Varificar o ip
		else if(valid_ip(argv[i]) == true)
		{
			strcpy(cf.ip, argv[i]);
			if(cf.modo != 2) cf.modo = 1;
		} else {
			printf("\n argumento desconhecido: %s \n", argv[i]);
		}
	}

	if(argc < 2)
	{
		help();
	}

	return cf;
}

bool valid_ip(const char ip[])
{
	int a, b, c, d;
	char ERRO_IP;
	
	int matches = sscanf(ip, "%d.%d.%d.%d%c", &a, &b, &c, &d, &ERRO_IP);

	if(matches != 4) return false;

	if(a < 0 || a > 255) return false;
	if(b < 0 || b > 255) return false;
	if(c < 0 || c > 255) return false;
	if(d < 0 || d > 255) return false;
	return true;
}

bool RootAcesse(config *cf)
{
	if(geteuid() != 0)
	{
		printf("\n[!] usuário sem permissão Root...\n");
		return cf->Error = true;
	}
	return cf->Error = false;
}

void help(void)
{
	printf("\nFalmef V2.0\n");
	printf("\nfeito por zaikadev45-tech \n");
	printf("https://github.com/zaikadev45-tech \n\n Modos do felmef:\n");
	printf("-v\n   Modo Verbose: Mostra Portas Fechadas \n");
	printf("-a\n   Modo AllPort: faz scan da porta 1 > 65535\n");
	printf("--cli\n   Modo CLI: ativa o modo CLI que retorna Valores,\n   0 = porta aberta\n   1 = porta fechada\n   2 = porta filtrada\n");
	printf("-sY\n   Usa modo SYN-Stealth (mais furtivo)\n");
}
