// Bloqueio e desbloqueio de processo: primitiva SLEEP/WAKEUP

#include <signal.h>
#include <unistd.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

#define LIMITE 10

// declarando as chaves e variaveis compartilhadas
key_t key1 = 3333, key2 = 4444, key3 = 5555;
int *buffer;// buffer que sera incrementado
pid_t *pid_consumidor;// ID do consumidor
pid_t *pid_produtor;// ID do produtor
//pid_t process_id;// ID do processo atual
int shmid1, shmid2, shmid3;


void DORMIR(int processo)
{
    pid_t process_id = getpid();// pega o ID do processo atual
    printf("\n");
    printf("processo %d dormindo\n", processo);
    kill(process_id, SIGSTOP);// sinal não-capturável que pode ser enviado a um processo para pausá-lo
}

void ACORDAR(pid_t pid)
{
    kill(pid, SIGCONT);// sinal enviado para um processo continuar sua execucao apos ela ser pausada(pelo sinal SIGSTOP)
}

//funcao que incrementa o valor do buffer
void produzir()
{
    while (true)
    {
        //verifica se o buffer esta no seu limite, se estiver o processo vai dormir
        if (*buffer == LIMITE)
        {
            printf("Não consegui produzir!\n");
            DORMIR(1);
        }
        printf("\n");
        printf("Estou no produtor!\n");
        *buffer = *buffer + 2;
        printf("Valor no buffer: %d\n", *buffer);
        
        if (*buffer >= 2)// se o valor no buffer for maior ou igual a 2, ele vai acordar o consumidor
        {
            ACORDAR(*pid_consumidor);
        }

        sleep(1);
    }
}

// funciona da mesma forma que a funcao produzir, mas decrementando o valor do buffer
void consumir()
{
    while (true)
    {
        if (*buffer == 0)
        {   
            printf("Não consegui consumir!\n");
            DORMIR(0);
        }
        printf("\n");
        printf("Estou no consumidor!\n");
        *buffer = *buffer - 2;
        printf("Valor no buffer: %d\n", *buffer);

        if (*buffer <= LIMITE - 2)
        {
            ACORDAR(*pid_produtor);
        }

        sleep(2);
    }
}

int main()
{
    // alocando memoria compartilhada, especificando a chave e o tamanho da memoria, alem da permissao de acesso
    shmid1 = shmget(key1, sizeof(int), 0666 | IPC_CREAT); 
    shmid2 = shmget(key2, sizeof(int), 0666 | IPC_CREAT);
    shmid3 = shmget(key3, sizeof(int), 0666 | IPC_CREAT);

    // conectando a memoria compartilhada ao processo, que retorna um ponteiro
    buffer = (int *)shmat(shmid1, NULL, 0);
    pid_consumidor = (int *)shmat(shmid2, NULL, 0);
    pid_produtor = (int *)shmat(shmid3, NULL, 0);

    *buffer = 0;

    // criando processo pai e processo filho
    pid_t pid = fork();

    if (pid == 0)
    {   
        *pid_consumidor = getppid();
        produzir();
    }
    else
    {
        *pid_produtor = pid;
        consumir();
    }

    // desconectando a memoria compartilhada
    shmdt(buffer);
    shmdt(pid_consumidor);
    shmdt(pid_produtor);

    // liberando a memoria compartilhada
    shmctl(shmid1, IPC_RMID, NULL);
    shmctl(shmid2, IPC_RMID, NULL);
    shmctl(shmid3, IPC_RMID, NULL);


    return 0;
}