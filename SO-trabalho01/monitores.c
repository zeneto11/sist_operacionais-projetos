// Monitores

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 7891 // Chave para operações com memória compartilhada
#define BUF_SIZE 1024 // Tamanho do buffer de memória compartilhado
#define NUM_MESSAGES 10 // Número de mensagens enviadas entre os processos

typedef enum { false, true } bool; // Define o tipo booleano

bool flag = false; // Variável para o controle de acesso da mem. compartilhada

int shmid; // ID do segmento de memória compartilhada
char *shm_buf; // Ponteiro para o buffer de memória compartilhada

// Função para entrar no monitor e garantir que apenas um processo acesse a mem. compartilhada
void enter_monitor(bool *flag) {
while (*flag) {
sleep(1);
}
*flag = true;
}

// Função para sair do monitor e liberar o acesso da mem. compartilhada
void exit_monitor(bool *flag) {
*flag = false;
}

// Função produtor
void produzir() {
    enter_monitor(&flag);

    printf("Emissor está escrevendo a mensagem......\n");

    // Escrever na memória compartilhada
    strcpy(shm_buf, "Quando te esforças, a vida também se esforça pra te ajudar.");

    exit_monitor(&flag);

    sleep(1);
}

// Função consumidor
void consumir() {
    enter_monitor(&flag);

    // Ler a memória compartilhada
    printf("Receptor leu: %s\n\n", shm_buf);

    exit_monitor(&flag);

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

        // Remover segmento de memória compartilhada
        shmctl(shmid, IPC_RMID, 0);
    }

    // Desconectar-se do segmento de memória compartilhada
    if (shmdt(shm_buf) < 0) {
            perror("shmdt");
            exit(1);
    }
    printf("Desconectado do segmento de memória compartilhada com id: %d\n", shmid);


    return 0;
} 
