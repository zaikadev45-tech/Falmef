use clap::Parser;

use std::net::{IpAddr, Ipv4Addr};
use std::str::FromStr;
use std::fmt;

#[derive(Parser)]
#[command(name = "falmef-rs")]
#[command(version = "0.1")]
#[command(about = "falmef = https://github.com/zaikadev45-tech/Falmef", long_about = None)]

pub struct Args {
    /// Endereço IP do alvo
    #[arg(value_parser = parse_ip)]
    pub ip: Ip,

    /// Portas especificas
    #[arg(short, long,
        num_args = 1..,
        default_values =
        ["21", "23", "25","53", "80", "110",
        "143", "443", "3306", "5555", "8080"])]
    pub port: Vec<u16>,

    /// Modo verbose (mostra portas fechadas também)
    #[arg(short, long, default_value_t = false)]
    pub verbose: bool,
}

impl fmt::Display for Ip {
    fn fmt(&self, f:  &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Ip::Unico(ip) => write!(f, "{}", ip),
            Ip::SubRede {rede, mask} => write!(f, "{}/{}", rede, mask),
        }
    }
}

#[derive(Debug, Clone)]
pub enum Ip {
    Unico(IpAddr),
    SubRede {rede: Ipv4Addr, mask: u8},
}

fn parse_ip(s: &str) -> Result<Ip, String> {
    if s.contains('/') {
        let partes: Vec<&str> = s.split('/').collect();
        let rede = Ipv4Addr::from_str(partes[0])
            .map_err(|e| format!("IP inválido: {e}"))?;
        let mask: u8 = partes[1]
            .parse()
            .map_err(|_| "máscara inválida".to_string())?;
        if mask == 8 || mask == 16 || mask == 24 || mask == 28 || mask == 30 || mask == 32 {
            Ok(Ip::SubRede { rede, mask})
        } else {
            Err("máscara disponíveis: 8/16/24/28/30/32".to_string())
        }
    } else {
        let ip = IpAddr::from_str(s)
            .map_err(|e| format!("IP inválido: {e}"))?;
        Ok(Ip::Unico(ip))

    }
}
