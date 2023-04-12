#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 2
#define PAGE_SIZE 4096
#define PAGE_COUNT 128

// Estrutura de página
typedef struct
{
    char data[PAGE_SIZE];
    int present; // Bit de presença da página (0 - não presente, 1 - presente)
} page_t;

// Variáveis globais
page_t pages[PAGE_COUNT]; // Vetor de páginas
pthread_mutex_t lock;     // Mutex para sincronização

// Função executada pelas threads
void *thread_func(void *thread_id)
{
    int id = *(int *)thread_id;

    // Simulação de acesso às páginas
    int i, j;
    for (i = id * PAGE_COUNT / NUM_THREADS; i < (id + 1) * PAGE_COUNT / NUM_THREADS; i++)
    {
        pthread_mutex_lock(&lock);
        // Verifica se a página não está presente na memória principal
        if (!pages[i].present)
        {
            printf("Thread %d: carregando página %d\n", id, i);
            // Simulação de leitura da página da memória secundária
            for (j = 0; j < PAGE_SIZE; j++)
            {
                pages[i].data[j] = rand() % 256;
            }
            pages[i].present = 1; // Define o bit de presença da página como 1, indicando que está presente na memória principal
        }
        pthread_mutex_unlock(&lock);
        // Simulação de acesso à página
        printf("Thread %d: acessando página %d\n\n", id, i);
        // Simulação de escrita na página
        pages[i].data[rand() % PAGE_SIZE] = rand() % 256;
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int i;

    // Inicialização do vetor de páginas
    for (i = 0; i < PAGE_COUNT; i++)
    {
        pages[i].present = 0; // Define o bit de presença de todas as páginas como 0, indicando que não estão presentes na memória principal
    }

    // Inicialização do mutex
    pthread_mutex_init(&lock, NULL);

    // Criação das threads
    for (i = 0; i < NUM_THREADS; i++)
    {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_func, (void *)&thread_ids[i]);
    }

    // Espera as threads terminarem
    for (i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Liberação do mutex
    pthread_mutex_destroy(&lock);

    return 0;
}
