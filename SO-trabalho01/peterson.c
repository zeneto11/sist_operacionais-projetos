// Exclusão mútua com espera ocupada: Solução de Peterson

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <stdbool.h>

// declarando as chaves e variaveis compartilhadas
int *buffer;// buffer compartilhado pelo produtor e pelo consumidor
int *flag;// usada para indicar quem esta interessado em acessar a regiao critica
int *turn;// indica de quem e a vez de acessar o buffer compartilhado
int shmid1, shmid2, shmid3;
key_t key1 = 1234, key2 = 9999, key3 = 3456;

// funcao produtor que adiciona um item no buffer em um loop
void produzir()
{
    int produtor = 1;
    int consumidor;

    consumidor = 1 - produtor; // produtor = 1

    // REGIÃO CRITICA (ENTER_REGION)
    *turn = 1;// indica que o processo quer usar o buffer compartilhado
    flag[produtor] = 1;// indica que o processo esta entrando na sua regiao critica(ENTER_REGION)

    // verifica se o outro(consumidor) esta na sua regiao critica
    while (*turn == produtor && flag[consumidor])
    {
        printf("Não pode produzir, pois o consumidor está na sua região crítica.\n");
        sleep(1);
    }

    // se ele nao entra no while, ele esta livre para acessar o buffer
    printf("%p", turn);
    printf("Produtor entrando na região crítica.\n");
    *buffer += 10;
    printf("O valor no buffer: %d.\n", *buffer);
    printf("Produtor saindo da região crítica.\n");
    printf("\n");
    flag[produtor] = 0; // indica que esta saindo de sua regiao critica(LEAVE_REGION)
    sleep(1);
}

// funciona da mesma forma que o produtor
void consumir()
{
    int consumidor = 0;
    int produtor;

    produtor = 1 - consumidor; // produtor = 1

    // REGIÃO CRITICA(ENTER_REGION)
    flag[consumidor] = 1;
    *turn = 0;

    // verifica se o outro esta na sua regiao critica
    while (*turn == consumidor && flag[produtor])
    {
        printf("Não pode consumir, pois o produtor está na sua região crítica.\n");
        sleep(2);
    }

    // verifica se o buffer nao esta vazio
    if (*buffer > 0)
    {
        printf("%p", turn);
        printf("Consumidor entrando na região crítica.\n");
        *buffer -= 5;
        printf("O valor no buffer: %d.\n", *buffer);
        printf("Consumidor saindo da região crítica.\n");
        printf("\n");
        flag[consumidor] = 0;
        sleep(2);
    }
}

int main(){

    // alocando memoria compartilhada, especificando a chave e o tamanho da memoria, alem da permissao de acesso
    shmid1 = shmget(key1, sizeof(int), 0666 | IPC_CREAT); 
    shmid2 = shmget(key2, sizeof(int) * 2, 0666 | IPC_CREAT);
    shmid3 = shmget(key3, sizeof(int), 0666 | IPC_CREAT);

    //conectando a memoria compartilhada ao processo, que retorna um ponteiro
    turn = (int *)shmat(shmid1, NULL, 0);
    flag = (int *)shmat(shmid2, NULL, 0);
    buffer = (int *)shmat(shmid3, NULL, 0);
       
    *buffer = 0;

    printf("Valor no buffer %d\n", *buffer);
    printf("\n");

    // criando processo pai e processo filho
    pid_t pid = fork();
    int count = 0;
    while (count < 10)
    {
        if (pid == 0)
        {
            produzir();
        }
        else
        {
            consumir();
        }
        count++;
    }

    // desconectando a memoria compartilhada
    shmdt(buffer);
    shmdt(turn);
    shmdt(flag);

    // liberando a memoria compartilhada
    shmctl(shmid3, IPC_RMID, NULL);
    shmctl(shmid2, IPC_RMID, NULL);
    shmctl(shmid1, IPC_RMID, NULL);

    return 0;
}