#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define MIN_BLOCK_SIZE 8     // Tamanho mínimo de bloco
#define MAX_MEMORY_SIZE 1024 // Tamanho máximo de memória

typedef struct block_t
{
    struct block_t *next; // Próximo bloco na lista de blocos livres
    size_t size;          // Tamanho do bloco (em bytes)
    bool free;            // Indica se o bloco está livre ou alocado
} block_t;

static block_t *free_blocks[MAX_MEMORY_SIZE + 1];         // Lista de blocos livres por tamanho
static size_t total_allocated = 0;                        // Total de memória alocada
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para proteger acesso concorrente

// Função que divide um bloco em dois blocos buddies, atualizando as listas de blocos livres
static void split_block(block_t *block, size_t new_size)
{
    while (block->size > new_size * 2)
    {
        block_t *buddy = (block_t *)((char *)block + new_size);
        buddy->next = free_blocks[new_size];
        buddy->size = new_size;
        buddy->free = true;
        free_blocks[new_size] = buddy;
        block->size = new_size;
        block = buddy;
        new_size *= 2;
    }
    block->free = false;
    free_blocks[new_size] = block->next;
}

// Função que une dois blocos buddies, criando um bloco maior
static block_t *merge_blocks(block_t *block1, block_t *block2)
{
    block_t *buddy;
    if (block1 < block2)
    {
        buddy = (block_t *)((char *)block1 + block1->size);
    }
    else
    {
        buddy = (block_t *)((char *)block2 + block2->size);
    }
    buddy->next = free_blocks[buddy->size];
    buddy->size = block1->size * 2;
    buddy->free = true;
    free_blocks[buddy->size] = buddy;
    return buddy;
}

// Função que aloca um bloco de memória de determinado tamanho
static void *allocate_memory(size_t size)
{
    size_t block_size = MIN_BLOCK_SIZE;
    while (block_size < size + sizeof(block_t))
    {
        block_size *= 2;
    }
    pthread_mutex_lock(&mutex);
    block_t *block = free_blocks[block_size];
    if (block == NULL)
    {
        split_block(free_blocks[block_size * 2], block_size);
        block = free_blocks[block_size];
    }
    block->free = false;
    free_blocks[block_size] = block->next;
    total_allocated += block->size;
    pthread_mutex_unlock(&mutex);
    return (void *)(block + 1);
}

// Função que desaloca um bloco de memória previamente alocado
static void deallocate_memory(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }
    block_t *block = (block_t *)ptr - 1;
    pthread_mutex_lock(&mutex);
    block->free = true;
    while (block->size < MAX_MEMORY_SIZE)
    {
        block_t *buddy = (block_t *)((size_t)block ^ block->size);
        if (buddy->free && buddy->size == block->size)
        {
            free_blocks[block->size] = buddy->next;
            block = merge_blocks(block, buddy);
        }
        else
        {
            block->next = free_blocks[block->size];
            free_blocks[block->size] = block;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
    total_allocated -= block->size;
}

// Função que imprime informações sobre os blocos livres na memória
static void print_free_blocks()
{
    printf("Blocos livres:\n");
    for (int i = MIN_BLOCK_SIZE; i <= MAX_MEMORY_SIZE; i *= 2)
    {
        printf("Tamanho %d:", (int)i);
        block_t *block = free_blocks[i];
        while (block != NULL)
        {
            printf(" [%p, %d]", block, (int)block->size);
            block = block->next;
        }
        printf("\n");
    }
}

// Função que imprime informações sobre os blocos alocados na memória
static void print_allocated_blocks()
{
    printf("Blocos alocados:\n");
    block_t *block = (block_t *)sbrk(0) - 1;
    while (block >= (block_t *)sbrk(0) - total_allocated)
    {
        printf(" [%p, %d]", block + 1, (int)block->size);
        block = (block_t *)((char *)block - block->size);
    }
    printf("\n");
}

int main()
{
    printf("Alocando 16 bytes...\n");
    void *p1 = allocate_memory(16);
    printf("p1 = %p\n", p1);
    print_free_blocks();
    print_allocated_blocks();
    printf("\nAlocando 32 bytes...\n");
    void *p2 = allocate_memory(32);
    printf("p2 = %p\n", p2);
    print_free_blocks();
    print_allocated_blocks();

    printf("\nDesalocando 16 bytes...\n");
    deallocate_memory(p1);
    print_free_blocks();
    print_allocated_blocks();

    printf("\nDesalocando 32 bytes...\n");
    deallocate_memory(p2);
    print_free_blocks();
    print_allocated_blocks();

    return 0;
}
