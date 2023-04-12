// Produtor/Consumidor com Semáforo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SEM_KEY 1234 // Chave para operações com semáforos
#define SHM_KEY 7891 // Chave para operações com memória compartilhada
#define BUF_SIZE 1024 // Tamanho do buffer de memória compartilhado
#define NUM_MESSAGES 10 // Número de mensagens enviadas entre os processos

int shmid, semid; // ID do segmento de memória compartilhada e semáforo
char *shm_buf; // Ponteiro para o buffer de memória compartilhada
struct sembuf sb; // Estrutura usada para controlar o semáforo

// Função produtor
void produzir() {
    // Esperar pelo semáforo
    sb.sem_num = 0; // Define o número do semáforo 
    sb.sem_op = -1; // Define a operação de decremento no valor do semáforo (down)
    semop(semid, &sb, 1); // Executa o decremento (entrada na seção crítica)

    printf("Emissor está escrevendo a mensagem...\n");
            
    // Escrever na memória compartilhada
    strcpy(shm_buf, "Crê em ti mesmo, age e verá os resultados.");

    // Liberar semáforo
    sb.sem_op = 1; // Define a liberação do recurso (up)
    semop(semid, &sb, 1); // Executa a liberação do recurso

    sleep(1);

}

// Função consumidor
void consumir() {
    // Esperar pelo semáforo
    sb.sem_num = 0; // Define o número do semáforo 
    sb.sem_op = -1; // Define a operação de decremento no valor do semáforo (down)
    semop(semid, &sb, 1); // Executa o decremento (entrada na seção crítica)

    // Ler a memória compartilhada
    printf("Receptor leu: %s\n\n", shm_buf);

    // Liberar semáforo
    sb.sem_op = 1; // Define a liberação do recurso (up)
    semop(semid, &sb, 1); // Executa a liberação do recurso

    sleep(1);

}

int main() {
    // Criar segmento de memória compartilhada
    shmid = shmget(SHM_KEY, BUF_SIZE, IPC_CREAT | 0666);
    // Caso não tenha sucesso retorna -1
    if (shmid < 0) { 
        perror("shmget");
        exit(1);
    }
    printf("Segmento de memória compartilhada criado com id: %d\n", shmid);


    // Conectar-se ao segmento de memória compartilhada
    shm_buf = shmat(shmid, NULL, 0);
    // Caso não tenha sucesso retorna -1
    if (shm_buf == (char *) -1) {
        perror("shmat");
        exit(1);
    }
    printf("Conectado ao segmento de memória compartilhada com id: %d\n\n", shmid);


    // Criar conjunto de semáforos
    semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    // Caso não tenha sucesso retorna -1
    if (semid < 0) {
        perror("semget");
        exit(1);
    }

    // Inicializar semáforo
    if (semctl(semid, 0, SETVAL, 1) < 0) {
        perror("semctl"); // Caso semctl retornar um valor menor que 0
        exit(3);
    }

    if (fork() == 0) {
        // Consumidor (filho)
        for (int i = 0; i < NUM_MESSAGES; i++) {
            consumir();
        }

    } else {
        // Produtor (pai)        
        for (int i = 0; i < NUM_MESSAGES; i++) {
            produzir();
        }

        wait(NULL);

        // Remover segmento de memória compartilhada e conjunto de semáforos
        shmctl(shmid, IPC_RMID, 0);
        semctl(semid, 0, IPC_RMID);
    }

    // Desconectar-se do segmento de memória compartilhada
    if (shmdt(shm_buf) < 0) {
        perror("shmdt");
        exit(1);
    }
    printf("Desconectado do segmento de memória compartilhada com id: %d\n", shmid);


    return 0;
}
