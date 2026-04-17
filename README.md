# **Falmef**
uma ferramenta que faz conexão com portas de um ip

## comandos:
- `--cli`
modo CLI, retorna valores
  > 0 = porta aberta
  > 
  > 1 = porta fechada
  > 
  > 2 = porta filtrada
- `-v`
modo Verbose, mostra as portas fechadas 
- `-a`
Escaneia as portas de 1 até 65355
- `-p`
faz scan em portas especificadas -p 80 8080 22
- `-sY`
Faz scan em modo SYN-Stealth

## para compilar:
`make`

##### exemplos:
- felmef 127.0.0.1 -a
- felmef 127.0.0.1:80
