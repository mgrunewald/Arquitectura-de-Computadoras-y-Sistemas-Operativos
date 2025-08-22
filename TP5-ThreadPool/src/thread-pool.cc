/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include "thread-pool.h"
using namespace std;

// se inicializa el ThreadPool con un número fijo de hilos y done como false
ThreadPool::ThreadPool(size_t numThreads) : wts(numThreads), done(false) {
    // se crean  e inicializan los worker threads asociados a la función worker 
    for (size_t i = 0; i < numThreads; ++i) {
        wts.push_back(worker_t{
            thread(&ThreadPool::worker, this, i), //se crea el thread asociado a la función worker
            nullptr  // inicialmente no tiene tarea asignada
        });
    }
    dt = thread(&ThreadPool::dispatcher, this); // se crea e inicializa el dispatcher thread
}

// schedule: agregar una nueva tarea a la cola de tareas
void ThreadPool::schedule(const function<void(void)>& thunk) { 
    unique_lock<mutex> lock(queueLock); // bloquea el acceso a la queue (FIFO) mientras se esté ejecutando
    tasks.push(thunk); // agrega la tarea a la queue
    taskIsAvailable.notify_one(); // avisa que hay una nueva tarea disponible/available
}

// worker: ejecuta cada worker thread
void ThreadPool::worker(int id) {  
    while (true) { // itera infinito hasta que se rompa explícitamente (cuando hacía (!done) había problemas con las condition variables) así se debe romper explícitamente el loop
        function<void(void)> task; // acá se almacena la task
        {
            unique_lock<mutex> lock(queueLock);
            taskIsAvailable.wait(lock, [&] { 
                return !tasks.empty() || done; // que haya una task available o que el threadpool esté done
            }); 
            if (done && tasks.empty()) break; // si solo se cumple que el threadPool está done y no quedan tareas pendientes en tasks
            if (!tasks.empty()) {
                task = tasks.front(); // toma la task al frente de la queue
                tasks.pop(); // la popea/saca de la queue
            }
            if (tasks.empty()) waitForEmpty.notify_all(); // avisa si la cola está vacía
        }
        if (task){
        task(); //ejecuta la task una vez fuera del bloqueo para evitar conflictos de sincronización
        }
    }
}

 // dispatcher: ejecuta el dispatcher thread que monitoriza la cola de tareas y coordina el procesamiento
void ThreadPool::dispatcher() {
    while (!done) { // lo que sucede abajo es muy parecido al caso anterior
        unique_lock<mutex> lock(queueLock); 
        taskIsAvailable.wait(lock, [&] { 
            return !tasks.empty() || done; 
        });
        if (done && tasks.empty()) break; 
        workerIsAvailable.notify_one(); // le avisa al worker thread hay una tarea disponible
    }
}

// wait: bloquea hasta que todas las tareas de la queue hayan sido procesadas y la cola esté vacía
void ThreadPool::wait() {
    unique_lock<mutex> lock(queueLock); 
    waitForEmpty.wait(lock, [&] { // bloqueado hata que la cola de tasks esté vacía
        return tasks.empty(); 
    });
    taskIsAvailable.notify_all(); // avisa al dispatcher y workers que todavía pueden aceptar nuevas tareas
}

// destructor: libera todos los recursos del threadPool (finalizando los worker threads y dispatcher)
ThreadPool::~ThreadPool() {
    {
        unique_lock<mutex> lock(queueLock);
        done = true; // establece quie el threadpool esta terminado/done
    }
    taskIsAvailable.notify_all(); // le avisa a TODOS los worker threads que estén esperando una tarea disponible
    waitForEmpty.notify_all();   // se fija de que nadie quede bloqueado en la cola vacía
    //dt.join();
    if (dt.joinable()) { // espera que el dispatcher termine 
        dt.join();
    }
    
    for (auto& worker : wts) { // para todos los worker threads espera que terminen si están activos
        if (worker.ts.joinable()) worker.ts.join(); 
    }
}
