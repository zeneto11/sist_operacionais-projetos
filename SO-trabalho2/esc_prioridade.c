#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define QUANTUM 2        // Tempo de quantum
#define MAX_PROCESSOS 10 // Número máximo de processos

// Estrutura de processo
struct processo
{
    int id;
    int tempo_restante;
    int prioridade;
    pthread_t thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

// Array de processos
struct processo processos[MAX_PROCESSOS];
int num_processos = 0;

// Função para executar o processo
void *executar_processo(void *arg)
{
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
        if (p->prioridade>1){
                    p->prioridade -= 1;
        }
        printf("Processo %d executado por %d segundos (prioridade: %d)\n", p->id, tempo_executado, p->prioridade);
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

// Função para adicionar um processo
void adicionar_processo(int tempo, int prioridade)
{
    if (num_processos >= MAX_PROCESSOS)
    {
        printf("Numero maximo de processos atingido\n");
        return;
    }
    struct processo p = {
        .id = num_processos + 1,
        .tempo_restante = tempo,
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .cond = PTHREAD_COND_INITIALIZER,
        .prioridade = prioridade};
    processos[num_processos] = p;
    num_processos++;
}

int maior_prioridade()
{
    int maior = -1;
    for (int i = 0; i < num_processos; i++)
    {
        if (processos[i].prioridade > maior)
        {
            maior = processos[i].prioridade;
        }
    }
    return maior;
}
// Função para executar o escalonamento Round Robin
void *executar_escalonamento()
{
    while (1)
    {
        int maior = maior_prioridade();
        for (int i = 0; i < num_processos; i++)
        {
            if (processos[i].prioridade == maior)
            {
                // Bloqueia o mutex do processo
                pthread_mutex_lock(&processos[i].mutex);
                // Sinaliza a condição do processo
                pthread_cond_signal(&processos[i].cond);
                // Desbloqueia o mutex do processo
                pthread_mutex_unlock(&processos[i].mutex);
                // Aguarda o tempo de quantum antes de executar o próximo processo
                usleep(QUANTUM * 150000);
            }
        }
    }
}

int main()
{
    // Adiciona alguns processos
    adicionar_processo(7, 7);
    adicionar_processo(8, 5);
    adicionar_processo(3, 4);
    adicionar_processo(10, 1);
    adicionar_processo(1, 2);

    // Inicia as threads para executar cada processo
    for (int i = 0; i < num_processos; i++)
    {
        pthread_create(&processos[i].thread_id, NULL, executar_processo, &processos[i]);
    }

    // Executa o escalonamento Round Robin em uma thread separada
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