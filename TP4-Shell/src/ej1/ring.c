#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    int start, status, pid, n, c;
    int buffer[1]; // buffer de tamaño 1 para almacenar mensaje
    int **pipes;

    if (argc != 4) {
        printf("Uso: anillo <n> <c> <s> \n");
        exit(0);
    }

    /* Parsing de argumentos */
    n = atoi(argv[1]);   // cantidad de procesos hijos
    c = atoi(argv[2]);   // mensaje inicial
    start = atoi(argv[3]); // proceso que inicia la comunicación

    // Crear pipes para la comunicación
    pipes = malloc(n * sizeof(int *)); // creo las pipes
    for (int i = 0; i < n; i++) {
        pipes[i] = malloc(2 * sizeof(int)); // los 2 extremos de las pipes
        pipe(pipes[i]); // se inicializan las pipes
    }

    printf("Se crearán %i procesos, se enviará el valor %i desde proceso %i \n", n, c, start);

    // crea procesos hijos
    for (int i = 0; i < n; i++) {
        pid = fork();  // creo un nuevo hijo

        if (pid == 0) {
            int next = (i + 1) % n;  // índice del proceso sucesor, con aritmética modular para tener circularidad

            // lee del proceso anterior o del padre
            read(pipes[i][0], buffer, sizeof(buffer));

            int valor_recibido = buffer[0];
            buffer[0]++; 

            // si soy el hijo (start - 2), devuelvo el valor al padre (porque esta indexado con 0 y sos el anterior al start)
            if (i == (start - 2 + n) % n) {
                printf("Soy el hijo %i, recibí el valor %i y lo devuelvo al padre aumentado en 1 \n", i + 1, valor_recibido);
                write(pipes[start - 1][1], buffer, sizeof(buffer)); // mandar al padre (hijo start - 1)
            } else {
                // si no, paso el mensaje al siguiente proceso
                printf("Soy el hijo %i, recibí el valor %i y envié el mensaje %i al hijo %i \n", i + 1, valor_recibido, buffer[0], next + 1);
                write(pipes[next][1], buffer, sizeof(buffer));
            }

            exit(0); // terminar el proceso hijo
        }
    }

    buffer[0] = c; // el padre toma el valor inicial
    write(pipes[start - 1][1], buffer, sizeof(buffer)); //se lo pasa a start-1 que es s (indexado con 0)

    // esperar a todos los hijos
    for (int i = 0; i < n; i++) {
        waitpid(-1, &status, 0);
    }

    // el padre recibe el mensaje final desde el hijo `start-2`
    read(pipes[start - 1][0], buffer, sizeof(buffer));
    printf("El proceso padre recibe el valor final: %i \n", buffer[0]);

    // liberar los pipes
    for (int i = 0; i < n; i++) {
        free(pipes[i]);
    }
    free(pipes);

    return 0;
}
