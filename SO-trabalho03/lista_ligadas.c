#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

typedef struct Page
{
    int data;          // Dados da página
    int pid;           // ID do processo que está utilizando a página
    struct Page *next; // Ponteiro para o próximo nó da lista
} Page;

typedef struct Swap
{
    int page_id;       // ID da página correspondente na memória principal
    struct Swap *next; // Ponteiro para o próximo nó da lista de swap
} Swap;

Page *memory_head; // Ponteiro para o primeiro nó da lista de memória
Swap *swap_head;   // Ponteiro para o primeiro nó da lista de swap

pthread_mutex_t memory_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para garantir a exclusão mútua na lista de memória
pthread_mutex_t swap_mutex = PTHREAD_MUTEX_INITIALIZER;   // Mutex para garantir a exclusão mútua na lista de swap

bool all_pages_loaded = false; // Variável de controle para indicar que todas as páginas foram carregadas na memória

void *memory_manager(void *arg) {
    while (!all_pages_loaded)
    {
        // Percorre a lista de memória e verifica se a página está presente
        pthread_mutex_lock(&memory_mutex);
        Page *current = memory_head;
        while (current != NULL)
        {
            if (current->data == arg)
            {
                printf("Thread %ld acessou página %d com sucesso!\n", pthread_self(), current->data);
                pthread_mutex_unlock(&memory_mutex);
                return NULL;
            }
            current = current->next;
        }
        pthread_mutex_unlock(&memory_mutex);

        // Caso a página não esteja presente, busca a página no disco e a adiciona à lista de memória
        printf("A página não está na memória, buscando no disco...\n");
        pthread_mutex_lock(&swap_mutex);
        Swap *swap_current = swap_head;
        while (swap_current != NULL)
        {
            if (swap_current->page_id == arg)
            {
                break;
            }
            swap_current = swap_current->next;
        }
        pthread_mutex_unlock(&swap_mutex);

        if (swap_current == NULL)
        {
            printf("A página não foi encontrada no disco!\n");
            sleep(2); // Aguarda alguns segundos antes de tentar buscar
            break;
        }

        // Aloca uma nova página na memória
        Page *new_page = (Page *)malloc(sizeof(Page));
        new_page->data = arg;
        new_page->pid = rand() % 1000; // Simula o ID do processo
        new_page->next = NULL;

        // Adiciona a nova página à lista de memória
        pthread_mutex_lock(&memory_mutex);
        if (memory_head == NULL)
        {
            memory_head = new_page;
        }
        else
        {
            Page *last = memory_head;
            while (last->next != NULL)
            {
                last = last->next;
            }
            last->next = new_page;
        }
        pthread_mutex_unlock(&memory_mutex);

        // Remove a página correspondente da lista de swap
        pthread_mutex_lock(&swap_mutex);
        if (swap_current == swap_head)
        {
            swap_head = swap_head->next;
        }
        else
        {
            Swap *last = swap_head;
            while (last->next != swap_current)
            {
                last = last->next;
            }
            last->next = swap_current->next;
        }
        free(swap_current);
        pthread_mutex_unlock(&swap_mutex);
        printf("Thread %ld carregou página %d na memória com sucesso!\n", pthread_self(), new_page->data);
    }
    return NULL;
}

int main()
{
    // Inicializa a lista de swap com 10 páginas
    swap_head = (Swap *)malloc(sizeof(Swap));
    swap_head->page_id = 0;
    Swap *current = swap_head;
    for (int i = 1; i < 10; i++)
    {
        current->next = (Swap *)malloc(sizeof(Swap));
        current = current->next;
        current->page_id = i;
    }
    current->next = NULL;

    // Inicializa 5 threads para simular os processos que acessam as páginas
    pthread_t threads[5];
    int page_ids[] = {2, 5, 1, 7, 4};
    for (int i = 0; i < 5; i++)
    {
        pthread_create(&threads[i], NULL, memory_manager, (void *)page_ids[i]);
    }

    // Aguarda as threads finalizarem
    for (int i = 0; i < 5; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Libera a memória alocada para a lista de swap
    while (swap_head != NULL)
    {
        Swap *next = swap_head->next;
        free(swap_head);
        swap_head = next;
    }

    // Libera a memória alocada para a lista de memória
    while (memory_head != NULL)
    {
        Page *next = memory_head->next;
        free(memory_head);
        memory_head = next;
    }

    return 0;
}