#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MEM_SIZE 32  // tamanho da memória em bytes
#define FRAME_SIZE 4  // tamanho do frame em bytes
#define NUM_FRAMES MEM_SIZE/FRAME_SIZE  // quantidade de frames na memória
#define MAX_PROCESSOS 4  // quantidade máxima de processos a serem criados
#define MAX_SIZE 32  // tamanho máximo de um processo em bytes

//estrutura do processo
struct Process{
    int processo_id;
    int num_pages;
};

typedef struct Page {
    struct Process process;  // processo a que a página pertence
    int pag_num;  // número da página dentro do processo
    int bit; //bit que mostra se o algoritmo merece uma segunda chance
} Page;

typedef struct Frame {
    int ocupado;  // indica se o frame está ocupado (1) ou livre (0)
    Page page;  // página armazenada no frame
} Frame;

struct Process processos[MAX_PROCESSOS]; //vetor de processos
int num_processos = 0;  // quantidade de processos criados

Frame memory[NUM_FRAMES];  // memória principal

// função para criar um novo processo com um tamanho aleatório
void criar_processo() {

    int tam = rand() % MAX_SIZE + 1; //gerando um tamanho aleatório para cada processo
    int num_pages = tam / FRAME_SIZE + (tam % FRAME_SIZE != 0);  // calcula o número de páginas
    struct Process processo ={
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
            printf("Frame %d: processo %d, página %d (bit: %d)\n", i, memory[i].page.process.processo_id, memory[i].page.pag_num, memory[i].page.bit);
        } else {
            printf("Frame %d: vazio\n", i);
        }
    }
    printf("\n");
}

// função para carregar uma página na memória
void load_page(Page page) {
    int i;
    //verifica se a página do processo já está na memória
    for (i = 0; i < NUM_FRAMES; i++) {
        if (memory[i].ocupado && memory[i].page.process.processo_id == page.process.processo_id && memory[i].page.pag_num == page.pag_num) {
            printf("Página %d do processo %d já está na memória no frame %d\n", page.pag_num, page.process.processo_id, i);
            return;
        }
    }
    //coloca uma página em um frame vazio(se existir)
    for (i = 0; i < NUM_FRAMES; i++) {
        if (!memory[i].ocupado) {
            memory[i].ocupado = 1;
            memory[i].page = page;
            printf("Página %d do processo %d colocada no frame %d\n", page.pag_num, page.process.processo_id, i);
            return;
        }
    }
    
    // se chegou aqui, é porque a memória está cheia
    printf("\n");
    printf("Memória cheia\n");
    gerenciador_memoria();

    int frame = 0; //frame para ser substituído

    //verifica o bit de cada frame até achar o que tenha 0
    while (memory[frame].page.bit==1){
        printf("Página %d do processo %d no frame %d tem bit %d e merece segunda chance\n", memory[frame].page.pag_num, memory[frame].page.process.processo_id, frame,  memory[frame].page.bit);
        
        //muda o bit dele para 0
        memory[frame].page.bit=0;

        //armazena a página antiga
        Page antiga = memory[frame].page;

        //vai mudar a posição das páginas a partir do frame atual
        for (i = frame; i < NUM_FRAMES - 1; i++) {
            memory[i] = memory[i+1];
        }

        //coloca novamente a página removida na memória
        memory[NUM_FRAMES - 1].ocupado = 1;
        memory[NUM_FRAMES - 1].page = antiga;
        printf("Página %d do processo %d colocada no frame %d\n", antiga.pag_num, antiga.process.processo_id, NUM_FRAMES - 1);
        gerenciador_memoria();

    }

    //se chegar aqui é porque o bit da página é 0
    printf("Página %d do processo %d no frame %d tem bit %d e não merece segunda chance\n", memory[frame].page.pag_num, memory[frame].page.process.processo_id, frame,  memory[frame].page.bit);  
    printf("Tirando a página %d do processo %d do frame %d\n", memory[frame].page.pag_num, memory[frame].page.process.processo_id, frame);
    
    //vai mudar a posição das páginas a partir do frame com bit 0
    for (i = frame; i < NUM_FRAMES - 1; i++) {
        memory[i] = memory[i+1];
    }

    //adiciona a nova página na memória
    memory[NUM_FRAMES - 1].ocupado = 1;
    memory[NUM_FRAMES - 1].page = page;
    printf("Página %d do processo %d colocada no frame %d\n", page.pag_num, page.process.processo_id, NUM_FRAMES - 1);
    gerenciador_memoria();
}


int main() {

    int i, k;
    for (i = 0; i < NUM_FRAMES; i++) {
        memory[i].ocupado = 0;
    }

    //criando processos
    for (i = 0; i<MAX_PROCESSOS; i++){
        criar_processo();
    }
    
    printf("\n");

    for (i = 0; i < 10; i++) {  

        printf("Iteração %d:\n", i+1);

        //escolhendo aleatoriamente o processo a ser referenciado
        int random_processo_id = rand() % MAX_PROCESSOS + 1;
        printf("Processo escolhido: %d com %d páginas\n", processos[random_processo_id].processo_id, processos[random_processo_id].num_pages);

        // gerando uma quantidade aleatória de páginas por processo a serem referenciadas
        int num_pages = rand() % processos[random_processo_id].num_pages + 1;
        printf("Número de páginas do processo que vão ser adicionadas: %d\n", num_pages);

        for (k = 0; k < num_pages; k++) {

            //gerando um bit aleatoriamente para cada página criada
            int bit = rand() % 3;
            if (bit!=0){
                bit = 1;
            }

            Page page = {processos[random_processo_id], k, bit};
            load_page(page);
        }

        gerenciador_memoria();
    }

    return 0;
}