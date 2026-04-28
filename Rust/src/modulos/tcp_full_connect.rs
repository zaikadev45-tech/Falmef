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

pub fn full_tcp(ip: String, port: u16) -> StatusPort {
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
