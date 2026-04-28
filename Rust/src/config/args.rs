use clap::Parser;
use std::net::IpAddr;

#[derive(Parser)]
#[command(name = "falmef-rs")]
#[command(version = "0.1")]
#[command(about = "falmef = https://github.com/zaikadev45-tech/Falmef", long_about = None)]

pub struct Args {
    /// Endereço IP do alvo
    #[arg(value_parser = clap::value_parser!(IpAddr))]
    pub ip: IpAddr,

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


