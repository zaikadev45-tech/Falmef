mod config;
mod modulos;

use config::Args;
use config::Ip;

use clap::Parser;


fn main() {
    let args = Args::parse(); 

    match &args.ip {
        Ip::Unico(ip) => {
            if !modulos::ping_tcp(&ip.to_string()) {
                println!("[=] Host Offline!");
                std::process::exit(1);
            }
            for port in &args.port {
                let status = modulos::full_tcp(&ip.to_string(), *port);
                status.exibir(*port, args.verbose);
            }

        }
        Ip::SubRede{rede, mask} => {
            let rede_string = rede.to_string();
            let ip_base: Vec<&str> = rede_string.split('.').collect();
            match mask {
                8 => {

                }
                16 => {

                }
                24 => {
                    let base24 = format!("{}.{}.{}",
                        ip_base[0], ip_base[1], ip_base[2]);
                    for ultimo in 0..255 {
                        let ip24 = format!("{}.{}", base24, ultimo);
                        if !modulos::ping_tcp(&ip24.to_string()) {
                            println!("[=] ip:{ip24} Host Offline!");
                            continue;
                        }
                        println!("Ip: {ip24} Online");
                        for port in &args.port {
                            let status = modulos::full_tcp(&ip24, *port);
                            status.exibir(*port, args.verbose);
                            
                        }
                    }
                }
                _ => println!("erro"),
            }
        }
    }

}

//exibir status da porta
impl modulos::StatusPort {
    fn exibir(&self, port: u16, verbose: bool) {
        match self {
            modulos::StatusPort::Aberto => println!("[+] {port} <=> ABERTO"),
            modulos::StatusPort::Fechado => {
                if verbose {
                    println!("[-] {port} <=> FECHADO");
                }
            }
            modulos::StatusPort::Filtrado => println!("[!] {port} <=> Filtrada"),
            modulos::StatusPort::Erro => println!("[ERRO NA REDE"),
        }
    }
}
