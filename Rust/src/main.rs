mod config;
mod modulos;

use config::Args;

use clap::Parser;

fn main() {
    let args = Args::parse();

    for port in args.port {
        match modulos::full_tcp(args.ip.to_string(), port) {
            modulos::StatusPort::Aberto => println!("[+]p: {port} <=> ABERTO"),
            modulos::StatusPort::Fechado => {
                if args.verbose {
                    println!("[-]p: {port} <=> FECHADO");
                }
            }
            modulos::StatusPort::Filtrado => println!("[!]p {port} <=> Filtrado"),
            modulos::StatusPort::Erro => println!("[RRO]"),
        }

    }

    /*
    match modulos::full_tcp(args.ip.to_string(), args.port) {
        modulos::StatusPort::Aberto => println!("[+] {} <=> ABERTO", args.port),
        modulos::StatusPort::Fechado => println!("[-] {} <=> FECHADO", args.port),
        modulos::StatusPort::Filtrado => println!("[!] {} <=> Filtrada", args.port),
        _ => println!("ERRO"),
    };*/


}
