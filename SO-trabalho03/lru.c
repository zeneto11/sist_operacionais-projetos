#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MEM_SIZE 32  // tamanho da memória em bytes
#define FRAME_SIZE 4  // tamanho do frame em bytes
#define NUM_FRAMES MEM_SIZE/FRAME_SIZE  // quantidade de frames na memória
#define MAX_PROCESSOS 4  // quantidade máxima de processos a serem criados
#define MAX_SIZE 32  // tamanho máximo de um processo em bytes

struct Process{
    int processo_id; //ID do processo
    int num_pages; //número de páginas do processo
};

typedef struct Page {
    struct Process process;  // processo a que a página pertence
    int pag_num;  // número da página dentro do processo
} Page;

typedef struct Frame {
    int ocupado;  // indica se o frame está ocupado (1) ou livre (0)
    Page page;  // página armazenada no frame
} Frame;

struct Process processos[MAX_PROCESSOS];
int num_processos = 0;  // quantidade de processos criados

Frame memory[NUM_FRAMES];  // memória principal

// função para criar um novo processo com um tamanho aleatório
void criar_processo() {

    int tam = rand() % MAX_SIZE + 1; //gerando um tamanho aleatório para o proceso
    int num_pages = tam / FRAME_SIZE + (tam % FRAME_SIZE != 0);  // calcula o número de páginas
    struct Process processo = {
        .processo_id = num_processos+1,
        .num_pages = num_pages
    };
    num_processos++;

    processos[num_processos] = processo;

    printf("Criando processo %d com tamanho %d bytes (%d páginas)\n", processo.processo_id, tam, processo.num_pages);

}

// função para exibir as páginas na memória
void gerenciador_memoria() {
    int i;
    printf("\n");
    for (i = 0; i < NUM_FRAMES; i++) {
        if (memory[i].ocupado) {
            printf("Frame %d: processo %d, página %d\n", i, memory[i].page.process.processo_id, memory[i].page.pag_num);
        } else {
            printf("Frame %d: vazio\n", i);
        }
    }
    printf("\n");
}

// função para carregar uma página na memória
void load_page(Page page) {
    int i, j;
    //verifica se a página do processo já está na memória
    for (i = 0; i < NUM_FRAMES; i++) {
        if (memory[i].ocupado && memory[i].page.process.processo_id == page.process.processo_id && memory[i].page.pag_num == page.pag_num) {
            printf("Página %d do processo %d já está na memória no frame %d\n", page.pag_num, page.process.processo_id, i);
            
            //agora, se a página já estiver na memória, colocamos ela no último frame desocupado e removemos ela da sua posição antiga
            
            printf("Tirando a página %d do processo %d no frame %d\n", memory[i].page.pag_num, memory[i].page.process.processo_id, i);
            
            //atualizo a posição dos outros itens
            for (j = i; j < NUM_FRAMES - 1; j++) {
                memory[j] = memory[j+1];
            }

            //colocando o item no primeiro frame desocupado
            for (i = 0; i < NUM_FRAMES; i++) {
                if (!memory[i].ocupado) {
                    memory[i].ocupado = 1;
                    memory[i].page = page;
                    printf("Página %d do processo %d está no frame %d\n", page.pag_num, page.process.processo_id, i);
                    return;
                }
            }    

            gerenciador_memoria();      

            return;
        }
    }

    //colocando o item no primeiro frame desocupado
    for (i = 0; i < NUM_FRAMES; i++) {
        if (!memory[i].ocupado) {
            memory[i].ocupado = 1;
            memory[i].page = page;
            printf("Página %d do processo %d está no frame %d\n", page.pag_num, page.process.processo_id, i);
            return;
        }
    }

    // se chegou aqui, é porque a memória está cheia
    printf("\n");
    printf("Memória cheia.\n");
    gerenciador_memoria();

    int frame = 0;

    //removendo o item no frame 0 e atualizando todos os outros frames
    printf("Tirando a página %d do processo %d no frame %d\n", memory[frame].page.pag_num, memory[frame].page.process.processo_id, frame);
    for (i = 0; i < NUM_FRAMES - 1; i++) {
        memory[i] = memory[i+1];
    }

    //colcando o item no último frame
    memory[NUM_FRAMES - 1].ocupado = 1;
    memory[NUM_FRAMES - 1].page = page;
    printf("Página %d do processo %d colocada no frame %d\n", page.pag_num, page.process.processo_id, NUM_FRAMES - 1);
}


int main() {

    int i, k;
    for (i = 0; i < NUM_FRAMES; i++) {
        memory[i].ocupado = 0;
    }
    

    for (i = 0; i<MAX_PROCESSOS; i++){
        criar_processo();
    }
    
    printf("\n");

    for (i = 0; i < 10; i++) {  

        printf("Iteração %d:\n", i+1);

        //escolhendo aleatoriamente o processo a ser referenciado
        int random_processo_id = rand() % MAX_PROCESSOS + 1;
        printf("processo escolhido: %d com %d paginas\n", processos[random_processo_id].processo_id, processos[random_processo_id].num_pages);

        // gerando uma quantidade aleatória de páginas por processo a serem referenciadas   
        int num_pages = rand() % processos[random_processo_id].num_pages + 1; 
        printf("Número de páginas do processo que vão ser adicionadas: %d\n", num_pages);

        for (k = 0; k < num_pages; k++) {
            Page page = {processos[random_processo_id], k};
            load_page(page);
        }

        gerenciador_memoria();
    }
    return 0;
}