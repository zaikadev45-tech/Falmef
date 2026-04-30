use std::io;
use std::net::{TcpStream, ToSocketAddrs};
use std::time::Duration;

#[derive(Debug, PartialEq)]
pub enum StatusPort {
    Aberto,
    Fechado,
    Filtrado,
    Erro,
}

pub fn full_tcp(ip: &str, port: u16) -> StatusPort {
    let addr = format!("{}:{}", ip, port);

    let addrs = match addr.to_socket_addrs() {
        Ok(addrs) => addrs,
        Err(_) => return StatusPort::Erro,
    };

    for socket_addr in  addrs {
        match TcpStream::connect_timeout(&socket_addr, Duration::from_secs(2)) {
            Ok(_) => return StatusPort::Aberto,
            Err(ref e) if e.kind() == io::ErrorKind::ConnectionRefused => {
                return StatusPort::Fechado;
            }
            Err(ref e) if e.kind() == io::ErrorKind::TimedOut => {
                return StatusPort::Filtrado;
            }
            Err(_) => return StatusPort::Erro,
        };        
    }
    StatusPort::Erro
}

pub fn ping_tcp(host: &str) -> bool {
    let mut erros: u8 = 0;
    let test: [u16; 3] = [64499, 65530, 65535];

    for &porta in &test {
        match full_tcp(host, porta) {
            StatusPort::Erro | StatusPort::Filtrado => {
                erros += 1;
                if erros == 3 {
                    return false;
                }
            }
            StatusPort::Aberto | StatusPort::Fechado => {
                return true;
            }
        } 
    }

    erros < 3
}
