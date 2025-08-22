#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 100

void interpret_escape_sequences(char *str) { // procesa /n y /t y lo convierte en caracteres para el string
    char *src = str; // lectura
    char *dst = str; // escritura

    while (*src) { // itera sobre todo el string, hasta que no haya nada más que leer
        if (*src == '\\' && *(src + 1)) {
            switch (*(src + 1)) {
                case 'n': *dst++ = '\n'; src += 2; break; // reemplaza por saldo de línea
                case 't': *dst++ = '\t'; src += 2; break; // reemplaza por tab
                default: *dst++ = *src++; break; 
            }
        } else { *dst++ = *src++; } // caso normal, deja todo igual
    }
    *dst = '\0';
}

void parse_arguments(char *command, char **args) { // divide al input de comandos en los diferentes argumentos, dejando estos en un array
    int arg_count = 0;
    char *arg_start = command;
    int in_quotes = 0;
    
    while (*command) { //itera todo el input
        if (*command == '"' || *command == '\'') {  // caso ""
            in_quotes = !in_quotes;
            if (!in_quotes) {  // termina "", entonces lo que está dentro de las comillas es un argumento
                *command = '\0';
                args[arg_count++] = arg_start; // lo agrega al array
                arg_start = command + 1; // actualiza el siguiente argumento
            } else { arg_start = command + 1; }
        } else if (*command == ' ' && !in_quotes) {  // separa con respecto a los espacios
            *command = '\0';
            if (arg_start != command) { args[arg_count++] = arg_start; }
            arg_start = command + 1;
        }
        command++; //siguiente caracter
    }
    if (*arg_start) { args[arg_count++] = arg_start; } // agrega último argumento
    args[arg_count] = NULL; // termina con un último elemento nulo
}

int main() {
    char command[256];
    pid_t pids[MAX_COMMANDS];
    char *commands[MAX_COMMANDS];
    int command_count = 0;

    while (1) {
        printf("Shell> ");
        fgets(command, sizeof(command), stdin); // lee el input del usuario en stdin
        command[strcspn(command, "\n")] = '\0'; // saca el último salto de línea

        char *token = strtok(command, "|"); // separa los comandos por '|' 
        command_count = 0;

        while (token != NULL) { // mientras estén separados por '|' 
            commands[command_count++] = token; // guada el command
            token = strtok(NULL, "|"); // busca el siguiente
        }

        // pipes
        int pipes[command_count - 1][2]; // crea el array de pipes
        for (int i = 0; i < command_count - 1; i++) { // pipe entre comandos
            if (pipe(pipes[i]) == -1) {
                perror("Error creating pipe");
                exit(1);
            }
        }

        for (int i = 0; i < command_count; i++) {
            pid_t pid = fork(); // crea un proceso hijo apra cada comando
            if (pid == -1) {
                perror("Error in fork");
                exit(1);
            } else { pids[i] = pid; } // guardo el pid

            if (pid == 0) { // para los procesos hijos
                if (i > 0) { // si no es el hijo del primer comando
                    if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1) { // cambia la entrada
                        perror("Error in dup2 for STDIN");
                        exit(1);
                    }
                }

                if (i < command_count - 1) { // si no es el último
                    if (dup2(pipes[i][1], STDOUT_FILENO) == -1) { // cambia la salida
                        perror("Error in dup2 for STDOUT");
                        exit(1);
                    }
                }

                // cierra los pipes del hijo
                for (int j = 0; j < command_count - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                char *args[MAX_ARGS];
                parse_arguments(commands[i], args); // convierte los comandos en argumentos con la función que creamos

                // en el caso que tengamos echo lo maneja con la función creada (los comandos con echo generaban error antes)
                if (strcmp(args[0], "echo") == 0 && args[1] && strcmp(args[1], "-e") == 0) {
                    interpret_escape_sequences(args[2]);
                    
                    // desplaza todos los elementos de args hacia la izquierda para eliminar "-e" del argumento entre comillas
                    for (int j = 1; args[j] != NULL; j++) {
                        args[j] = args[j + 1];
                    }
                }

                if (execvp(args[0], args) == -1) {
                    perror("Error executing command");
                    exit(1);
                }
            }
        }

        // cierra las pipes del proceso padre
        for (int i = 0; i < command_count - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        // hace waitpid hasta que temrinen los hijos
        for (int i = 0; i < command_count; i++) { waitpid(pids[i], NULL, 0); }
    }
    return 0;
}