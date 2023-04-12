#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define PAGE_SIZE 4
#define PAGE_TABLE_SIZE 16
#define NUM_THREADS 4

// Definição de uma entrada da tabela de páginas
typedef struct
{
    int page_number;   // Número da página armazenada nessa entrada
    int reference_bit; // Bit de referência usado pelo algoritmo NUR
    int dirty_bit;     // Bit de sujeira para indicar se a página foi modificada
} page_table_entry;

// Declaração da tabela de páginas e contadores de falhas de página
page_table_entry page_table[PAGE_TABLE_SIZE];
int page_fault_count = 0;

// Função que é executada por cada thread para acessar páginas
void *access_page(void *thread_id)
{
    int id = *((int *)thread_id);
    int i, j, page_number;
    for (i = 0; i < 10; i++)
    {
        // Seleciona uma página aleatória na tabela de páginas
        page_number = rand() % PAGE_TABLE_SIZE;
        printf("Thread %d acessando página %d\n", id, page_number);
        for (j = 0; j < PAGE_TABLE_SIZE; j++)
        {
            // Verifica se a página pode ser substituída
            if (page_table[j].reference_bit == 0)
            {
                if (page_table[j].dirty_bit == 0)
                {
                    // Encontrou uma página para substituir
                    printf("Thread %d substituindo página %d\n", id, j);
                    // Carrega a nova página na entrada selecionada
                    page_table[j].page_number = page_number;
                    page_table[j].reference_bit = 1;
                    page_table[j].dirty_bit = 0;
                    break;
                }
            }
            else
            {
                // A página foi recentemente referenciada, limpa o bit de referência
                page_table[j].reference_bit = 0;
            }
        }
        if (j == PAGE_TABLE_SIZE)
        {
            // Não encontrou uma página para substituir, incrementa o contador de falhas de página
            printf("Thread %d encontrou uma falha de página\n", id);
            page_fault_count++;
        }
    }
    pthread_exit(NULL);
}

int main()
{
    int i, thread_ids[NUM_THREADS];
    pthread_t threads[NUM_THREADS];
    // Inicializa a tabela de páginas com valores padrão
    for (i = 0; i < PAGE_TABLE_SIZE; i++)
    {
        page_table[i].page_number = -1;
        page_table[i].reference_bit = 0;
        page_table[i].dirty_bit = 0;
    }
    // Cria as threads que irão acessar as páginas
    for (i = 0; i < NUM_THREADS; i++)
    {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, access_page, &thread_ids[i]);
    }
    // Aguarda todas as threads terminarem
    for (i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    // Imprime o número total de falhas de página
    printf("Total de falhas de página: %d\n", page_fault_count);
    return 0;
}
