#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

typedef struct
{
	int modo;
	bool Error;
	// 1 = com porta
	// 2 = sem porta
	
	//modos:
	bool verbose; // mostra portas fechadas
	bool AllPort; // scan de 1 ate 65355
	bool cli; // retorna valores
	
	//tipos de scan:
	bool TCP_CONNECT;
	bool SYN;
	//bool ping;
	//bool arp
	
	//info:
	char ip[16];

	int *port;
	int cout_port;
} config;

config processInput(int argc, char *argv[]);

bool valid_ip(const char ip[]);
bool RootAcesse(config *cf);
void help(void);

#endif
