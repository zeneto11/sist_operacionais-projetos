#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define QUANTUM 2        // Tempo de quantum
#define MAX_PROCESSOS 10 // Número máximo de processos
#define MAX_FILAS 3 //número máximo de filas

// Estrutura de processo
struct processo
{
    int id;
    int tempo_restante;
    int prioridade;
    int fila;
    pthread_t thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

//Estrutura da fila
struct fila
{
    struct processo *processos[MAX_PROCESSOS];
    int num_processos;
    int quantum;
};

// Array de processos
struct processo processos[MAX_PROCESSOS];
int num_processos = 0;

//Array de filas
struct fila filas[MAX_FILAS] = {
    {.quantum = QUANTUM},
    {.quantum = QUANTUM * 2},
    {.quantum = QUANTUM * 4},
};

// Função para executar o processo
void *executar_processo(void *arg)
{
    printf("executando processo\n");
    struct processo *p = (struct processo *)arg;
    int tempo_executado = 0;
    
    while (p->tempo_restante > 0)
    {
        // Bloqueia o mutex do processo
        pthread_mutex_lock(&p->mutex);
        // Espera pela condição do processo
        pthread_cond_wait(&p->cond, &p->mutex);
        // Executa o processo por um quantum
        if (p->tempo_restante > QUANTUM)
        {
            tempo_executado = QUANTUM;
            p->tempo_restante -= QUANTUM;
        }
        else
        {
            tempo_executado = p->tempo_restante;
            p->tempo_restante = 0;
        }
        //Diminuindo a prioridade a cada vez que o processo é executado
        p->prioridade -= 1;
        printf("Processo %d executado por %d segundos na fila %d\n", p->id, tempo_executado, p->fila);
        // Desbloqueia o mutex do processo
        pthread_mutex_unlock(&p->mutex);
        // Aguarda um tempo antes de executar o próximo processo
        usleep(1000);
    }
    //Garantir que o processo terá uma prioridade muito baixa para que ele não seja executado novamente
    p->prioridade = -9999;
    // Retorna NULL para finalizar a thread
    return NULL;
}

// Função para adicionar o processo na fila
void adicionar_processo_fila(struct processo *p){
    printf("adicionando processo na fila\n");
    int prioridade = p->prioridade;
    if (prioridade>6) {
        filas[0].processos[filas[0].num_processos] = p;
        filas[0].num_processos++;
        p->fila = 0;
    } else if (prioridade>3) {
        filas[1].processos[filas[1].num_processos] = p;
        filas[1].num_processos++;
        p->fila = 1;
    } else{
        filas[2].processos[filas[2].num_processos] = p;
        filas[2].num_processos++;
        p->fila = 2;
    }
    printf("processo adicionado na fila\n");
}

// Função para adicionar um processo
void adicionar_processo(int tempo, int prioridade)
{
    printf("Adicionando processo\n");
    if (num_processos >= MAX_PROCESSOS)
    {
        printf("Numero maximo de processos atingido\n");
        return;
    }
    
    struct processo *p = malloc(sizeof(struct processo));
 /*  
    struct processo *p = {
        .id = num_processos + 1,
        .tempo_restante = tempo,
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .cond = PTHREAD_COND_INITIALIZER,
        .prioridade = prioridade};
*/ 

    p->id = num_processos+1;
    p->tempo_restante = tempo;
    p->mutex = PTHREAD_MUTEX_INITIALIZER;
    p->cond = PTHREAD_COND_INITIALIZER;
    p->prioridade = prioridade;
    
    processos[num_processos] = *p;
    num_processos++;
    printf("processo adicionado\n");
}

// Função para verificar a maior prioridade
int maior_prioridade()
{
    for (int i = 0; i < MAX_FILAS; i++)
    {
        if (filas[i].num_processos > 0)
        {
            return i;
        }
    }
    return -1;
}

// Função para executar o filas múltiplas
void *executar_escalonamento()
{
    while (1)
    {
        printf("executando escalonamento\n");
        int i = maior_prioridade();
        if (i == -1){
            break;
        }
        struct processo *p = filas[i].processos[0];

        //Bloqueando o mutex do processo
        //pthread_mutex_lock(&p->mutex);
        //Sinalizando a condição do processo
        //pthread_cond_signal(&p->cond);
        //Desbloqueando o mutex do processo
        //pthread_mutex_unlock(&p->mutex);
        //Da um sleep pelo tempo do quantum antes de executar o próximo processo
        usleep(filas[i].quantum * 1000000);
        //Removendo o processo da fila
        for (int j = 0; j < filas[i].num_processos-1; j++) {
            filas[i].processos[j] = filas[i].processos[j+1];
        }
        filas[i].num_processos--;
        //Por fim, vamos colocar o processo na sua nova fila de prioridade
        adicionar_processo_fila(p);
    }
    return NULL;
}

int main()
{
    // Adiciona alguns processos
    adicionar_processo(5, 7);
    adicionar_processo(8, 5);
    adicionar_processo(3, 4);
    adicionar_processo(10, 1);
    adicionar_processo(1, 2);

    // Inicia as threads para executar cada processo
    for (int i = 0; i < num_processos; i++)
    {
        pthread_create(&processos[i].thread_id, NULL, executar_processo, &processos[i]);
    }

    // Executa o filas múltiplas em uma thread separada
    pthread_t thread_escalonamento;
    pthread_create(&thread_escalonamento, NULL, executar_escalonamento, NULL);

    // Aguarda a finalização das threads de cada processo
    for (int i = 0; i < num_processos; i++)
    {
        pthread_join(processos[i].thread_id, NULL);
    }

    // Finaliza a thread de escalonamento
    pthread_cancel(thread_escalonamento);

    return 0;
}