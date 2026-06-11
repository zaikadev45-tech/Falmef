use std::sync::{Arc, Mutex, Condvar};
use std::thread;

pub struct Semaforo {
    slots: Mutex<usize>,
    cond: Condvar,
}

impl Semaforo {
    pub fn new(max: usize) -> Arc<Self> {
        Arc::new(Self {
            slots: Mutex::new(max),
            cond: Condvar::new(),
        })
    }

    pub fn adquirir(self: &Arc<Self>) {
        let mut slots = self.slots.lock().unwrap();
        while *slots == 0 {
            slots = self.cond.wait(slots).unwrap();
        }
        *slots -= 1;
    }

    pub fn liberar(&self) {
        let mut slots = self.slots.lock().unwrap();
        *slots += 1;
        self.cond.notify_one();
    }
}

pub fn varrer_paralelo<F>(
    ips: impl Iterator<Item = String>,
    max_threads: usize,
    tarefa: F,
) where
    F: Fn(String) + Send + Sync + 'static,
{
    let sem = Semaforo::new(max_threads);
    let tarefa = Arc::new(tarefa);
    let mut handles = vec![];

    for ip in ips {
        let sem = Arc::clone(&sem);
        let tarefa = Arc::clone(&tarefa);

        sem.adquirir();

        let handle = thread::spawn(move || {
            tarefa(ip);
            sem.liberar();
        });

        handles.push(handle);
    }

    for h in handles {
        h.join().unwrap();
    }
}