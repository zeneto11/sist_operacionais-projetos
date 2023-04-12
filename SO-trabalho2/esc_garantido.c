#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_PROCESSOS 10 // Número máximo de processos

// Variável global para definir o cpu time
double CPU_TIME = 10;

// Estrutura de processo
struct processo {
    int id;
    double tempo_restante;
    pthread_t thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int finalizado; // Flag para indicar se o processo foi finalizado
};

// Array de processos
struct processo processos[MAX_PROCESSOS];
int num_processos = 0;

// Tempo de execução para cada processo
double tempo_execucao = 0;

// Contador de processos
int cont_processos = 5;

// Função para executar um processo pelo tempo de execução calculado
void* executar_processo(void* arg) {
    struct processo* p = (struct processo*) arg;
    double tempo_executado = 0;
    while(p->tempo_restante > 0) {
        // Bloqueia o mutex do processo
        pthread_mutex_lock(&p->mutex);
        // Espera pela condição do processo
        pthread_cond_wait(&p->cond, &p->mutex);
        // Cálculo do tempo de execução
        tempo_execucao = CPU_TIME/cont_processos;
        // Executa o processo por tempo de execução
        if(p->tempo_restante > tempo_execucao) {
            tempo_executado = tempo_execucao;
            p->tempo_restante -= tempo_execucao;
        } else {
            tempo_executado = p->tempo_restante;
            p->tempo_restante = 0;
            cont_processos -= 1;
        }
        printf("Processo %d executado por %f segundos\n", p->id, tempo_executado);
        // Desbloqueia o mutex do processo
        pthread_mutex_unlock(&p->mutex);
        // Aguarda um tempo antes de executar o próximo processo
        usleep(3000);
        // Verifica se o processo foi finalizado
        if (p->tempo_restante == 0) {
            p->finalizado = 1;
            printf("Processo %d finalizado e removido do array\n", p->id);
            return NULL;
        }
    }
    // Retorna NULL para finalizar a thread
    return NULL;
}

// Função para adicionar um processo
void adicionar_processo(int tempo) {
    if(num_processos >= MAX_PROCESSOS) {
        printf("Numero maximo de processos atingido\n");
        return;
    }
    struct processo p = {
        .id = num_processos + 1,
        .tempo_restante = tempo,
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .cond = PTHREAD_COND_INITIALIZER,
        .finalizado = 0 // Inicializa a flag como não finalizado
    };
    processos[num_processos] = p;
    num_processos++;
}

// Função para executar o escalonamento garantido
void* executar_escalonamento() {
    while(1) {
        for(int i = 0; i < num_processos; i++) {
            // Bloqueia o mutex do processo
            pthread_mutex_lock(&processos[i].mutex);
            // Sinaliza a condição do processo
            pthread_cond_signal(&processos[i].cond);
            // Desbloqueia o mutex do processo
            pthread_mutex_unlock(&processos[i].mutex);
            // Aguarda o tempo de quantum antes de executar o próximo processo
            usleep(1000000);
        }
    }
}

int main() {
    // Adiciona alguns processos
    adicionar_processo(4);
    adicionar_processo(20);
    adicionar_processo(25);
    adicionar_processo(50);
    adicionar_processo(10);

    // Inicia as threads para executar cada processo
    for(int i = 0; i < num_processos; i++) {
        pthread_create(&processos[i].thread_id, NULL, executar_processo, &processos[i]);
    }

    // Executa o escalonamento garantido em uma thread separada
    pthread_t thread_escalonamento;
    pthread_create(&thread_escalonamento, NULL, executar_escalonamento, NULL);

    // Aguarda a finalização das threads de cada processo
    for(int i = 0; i < num_processos; i++) {
        pthread_join(processos[i].thread_id, NULL);
    }

    // Finaliza a thread de escalonamento
    pthread_cancel(thread_escalonamento);

    return 0;
}