mod config;
mod modulos;

use std::net::Ipv4Addr;
use std::sync::Arc;

use config::{Args, Ip};
use config::varrer_paralelo;
use clap::Parser;

const MAX_THREADS: usize = 100;

fn ip_range(rede: &Ipv4Addr, mask: u8) -> (u32, u32) {
    let ip_u32 = u32::from(*rede);
    let mascara_bits = if mask == 0 { 0 } else { !0u32 << (32 - mask) };
    let primeiro = ip_u32 & mascara_bits;
    let ultimo = primeiro | !mascara_bits;
    (primeiro, ultimo)
}

fn varrer(ip: &str, ports: &[u16], verbose: bool) {
    if !modulos::ping_tcp(ip) {
        if verbose {
            println!("[=] ip:{ip} Host Offline!");
        }
        return;
    }

    let mut output = format!("Ip: {ip} Online\n");

    for port in ports {
        let status = modulos::full_tcp(ip, *port);
        match status {
            modulos::StatusPort::Aberto   => output.push_str(&format!("  [+] {port} <=> ABERTO\n")),
            modulos::StatusPort::Fechado  => { if verbose { output.push_str(&format!("  [-] {port} <=> FECHADO\n")); } }
            modulos::StatusPort::Filtrado => output.push_str(&format!("  [!] {port} <=> Filtrada\n")),
            modulos::StatusPort::Erro     => output.push_str(&format!("  [ERRO NA REDE]\n")),
        }
    }

    print!("{output}");
}

fn main() {
    let args = Args::parse();

    println!("criador: https://github.com/zaikadev45-tech");
    println!("source code: https://github.com/zaikadev45-tech/Falmef  \n");

    match &args.ip {
        Ip::Unico(ip) => {
            if !modulos::ping_tcp(&ip.to_string()) {
                println!("[=] Host Offline!");
                std::process::exit(1);
            }
            println!("[+] Host Online!");
            for port in &args.port {
                let status = modulos::full_tcp(&ip.to_string(), *port);
                status.exibir(*port, args.verbose);
            }
        }
        Ip::SubRede { rede, mask } => {
            let (inicio, fim) = ip_range(rede, *mask);
            
            let total = fim - inicio + 1;
            println!("Varrendo {}/{} ({} hosts)", rede, mask, total);
            
            let ports = Arc::new(args.port.clone());
            let verbose = args.verbose;
            let ips = (inicio..=fim).map(|n| Ipv4Addr::from(n).to_string());

            varrer_paralelo(ips, MAX_THREADS, move |ip: String| {
                let ports = Arc::clone(&ports);
                varrer(&ip, &ports, verbose);
            });
        }
    }
    println!("\n Finalizado");
}

impl modulos::StatusPort {
    fn exibir(&self, port: u16, verbose: bool) {
        match self {
            modulos::StatusPort::Aberto   => println!("[+] {port} <=> ABERTO"),
            modulos::StatusPort::Fechado  => { if verbose { println!("[-] {port} <=> FECHADO"); } }
            modulos::StatusPort::Filtrado => println!("[!] {port} <=> Filtrada"),
            modulos::StatusPort::Erro     => println!("[ERRO NA REDE]"),
        }
    }
}
