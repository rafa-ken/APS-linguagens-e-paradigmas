// vm.c
// Interpretador simples de assembly textual para a VM do APS
// Compilar: gcc -O2 -o vm vm.c
//
// Formato de instrução: cada instrução em uma linha. Labels terminam com ':'.
// Comentários iniciam com '#' e linhas vazias são ignoradas.
//
// Instruções suportadas:
//  - LDI <reg> <imm>        ; reg = valor imediato
//  - LOAD <reg> <addr>      ; reg = MEM[addr]
//  - STORE <reg> <addr>     ; MEM[addr] = reg
//  - ADD <rA> <rB>          ; rA = rA + rB
//  - SUB <rA> <rB>          ; rA = rA - rB
//  - MUL <rA> <rB>
//  - DIV <rA> <rB>
//  - INC <reg>
//  - DEC <reg>
//  - JMP <label>
//  - JZ <reg> <label>       ; se reg == 0 pula
//  - JNZ <reg> <label>      ; se reg != 0 pula
//  - PRINT_REG <reg>        ; imprime valor do registrador
//  - PRINT_MEM <addr>       ; imprime MEM[addr]
//  - PRINT_VAL <imm>        ; imprime valor imediato
//  - READ_SENSOR <reg> <id> ; lê sensor readonly para registrador
//  - HALT
//
// Registradores: R0, R1, R2, R3
// Memória: MEM[0..1023] inteiros
// Sensores (readonly): ID 0 = tempo simulado desde início (segundos),
//                      ID 1 = constante (ex.: 42), ID 2 = valor pseudo-aleatório
//
// Exemplo de uso: ./vm sample.asm

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_LINES 10000
#define MAX_TOK 8
#define MEM_SIZE 1024
#define MAX_LABELS 10000

typedef struct {
    char *line;
    char *tokens[MAX_TOK];
    int ntok;
} Instr;

typedef struct {
    char *name;
    int idx;
} Label;

Instr program[MAX_LINES];
int nlines = 0;

Label labels[MAX_LABELS];
int nlabels = 0;

int REG[4];
int MEM[MEM_SIZE];

double vm_start_time;

void trim(char *s) {
    char *p = s;
    while(isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p)+1);
    char *end = s + strlen(s) - 1;
    while(end >= s && isspace((unsigned char)*end)) { *end = '\0'; end--; }
}

int reg_index(const char *r) {
    if (strcmp(r,"R0")==0) return 0;
    if (strcmp(r,"R1")==0) return 1;
    if (strcmp(r,"R2")==0) return 2;
    if (strcmp(r,"R3")==0) return 3;
    return -1;
}

int find_label(const char *name) {
    for (int i=0;i<nlabels;i++) if (strcmp(labels[i].name,name)==0) return labels[i].idx;
    return -1;
}

void parse_file(const char *path) {
    FILE *f = fopen(path,"r");
    if (!f) { perror("fopen"); exit(1); }
    char *line = NULL;
    size_t len=0;
    ssize_t read;
    while ((read = getline(&line, &len, f)) != -1) {
        if (nlines >= MAX_LINES) { fprintf(stderr,"Programa muito longo\n"); exit(1); }
        if (read>0 && (line[read-1]=='\n' || line[read-1]=='\r')) line[--read]=0;
        char *s = strdup(line);
        trim(s);
        if (s[0]==0 || s[0]=='#') { free(s); continue; }
        program[nlines].line = s;
        program[nlines].ntok = 0;
        char *p = s;
        if (strchr(p,':')) {
            char *col = strchr(p,':');
            int pos = col - p;
            char *lab = strndup(p,pos);
            trim(lab);
            labels[nlabels].name = lab;
            labels[nlabels].idx = nlines;
            nlabels++;
            p = col + 1;
            trim(p);
            if (*p == 0) {
                program[nlines].ntok = 0;
                nlines++;
                continue;
            }
        }
        char *tok = strtok(p," \t,");
        while(tok && program[nlines].ntok < MAX_TOK) {
            program[nlines].tokens[program[nlines].ntok++] = strdup(tok);
            tok = strtok(NULL," \t,");
        }
        nlines++;
    }
    fclose(f);
    if (line) free(line);
}

int sensor_read(int id) {
    if (id == 0) {
        double now = ((double)clock() - vm_start_time) / CLOCKS_PER_SEC;
        return (int) now;
    } else if (id == 1) {
        return 42;
    } else {
        return rand() % 100;
    }
}

int to_int(const char *s) {
    return atoi(s);
}

void run() {
    vm_start_time = clock();
    srand((unsigned int)time(NULL));
    int pc = 0;
    int steps = 0;
    while (pc < nlines) {
        Instr *ins = &program[pc];
        if (ins->ntok == 0) { pc++; continue; }
        char *op = ins->tokens[0];
        for (char *c=op; *c; ++c) *c = toupper((unsigned char)*c);
        if (strcmp(op,"LDI")==0) {
            if (ins->ntok < 3) { fprintf(stderr,"LDI requer 2 argumentos na linha %d\n",pc+1); exit(1); }
            int r = reg_index(ins->tokens[1]);
            if (r<0) { fprintf(stderr,"Registrador inválido %s\n",ins->tokens[1]); exit(1); }
            int v = to_int(ins->tokens[2]);
            REG[r] = v;
            pc++;
        } else if (strcmp(op,"LOAD")==0) {
            if (ins->ntok < 3) { fprintf(stderr,"LOAD requer 2 argumentos\n"); exit(1); }
            int r = reg_index(ins->tokens[1]);
            int addr = to_int(ins->tokens[2]);
            if (addr < 0 || addr >= MEM_SIZE) { fprintf(stderr,"LOAD endereço inválido %d\n",addr); exit(1); }
            REG[r] = MEM[addr];
            pc++;
        } else if (strcmp(op,"STORE")==0) {
            if (ins->ntok < 3) { fprintf(stderr,"STORE requer 2 argumentos\n"); exit(1); }
            int r = reg_index(ins->tokens[1]);
            int addr = to_int(ins->tokens[2]);
            if (addr < 0 || addr >= MEM_SIZE) { fprintf(stderr,"STORE endereço inválido %d\n",addr); exit(1); }
            MEM[addr] = REG[r];
            pc++;
        } else if (strcmp(op,"ADD")==0) {
            int a = reg_index(ins->tokens[1]);
            int b = reg_index(ins->tokens[2]);
            REG[a] = REG[a] + REG[b];
            pc++;
        } else if (strcmp(op,"SUB")==0) {
            int a = reg_index(ins->tokens[1]);
            int b = reg_index(ins->tokens[2]);
            REG[a] = REG[a] - REG[b];
            pc++;
        } else if (strcmp(op,"MUL")==0) {
            int a = reg_index(ins->tokens[1]);
            int b = reg_index(ins->tokens[2]);
            REG[a] = REG[a] * REG[b];
            pc++;
        } else if (strcmp(op,"DIV")==0) {
            int a = reg_index(ins->tokens[1]);
            int b = reg_index(ins->tokens[2]);
            if (REG[b] == 0) { fprintf(stderr,"DIV por zero\n"); exit(1); }
            REG[a] = REG[a] / REG[b];
            pc++;
        } else if (strcmp(op,"INC")==0) {
            int r = reg_index(ins->tokens[1]);
            REG[r]++; pc++;
        } else if (strcmp(op,"DEC")==0) {
            int r = reg_index(ins->tokens[1]);
            REG[r]--; pc++;
        } else if (strcmp(op,"JMP")==0) {
            if (ins->ntok < 2) { fprintf(stderr,"JMP precisa de label\n"); exit(1); }
            int idx = find_label(ins->tokens[1]);
            if (idx < 0) { fprintf(stderr,"Label desconhecido %s\n",ins->tokens[1]); exit(1); }
            pc = idx;
        } else if (strcmp(op,"JZ")==0 || strcmp(op,"JNZ")==0) {
            if (ins->ntok < 3) { fprintf(stderr,"%s precisa de registrador e label\n", op); exit(1); }
            int r = reg_index(ins->tokens[1]);
            int idx = find_label(ins->tokens[2]);
            if (idx < 0) { fprintf(stderr,"Label desconhecido %s\n",ins->tokens[2]); exit(1); }
            int cond = (REG[r] == 0);
            if (strcmp(op,"JNZ")==0) cond = !cond;
            if (cond) pc = idx; else pc++;
        } else if (strcmp(op,"PRINT_REG")==0) {
            int r = reg_index(ins->tokens[1]);
            printf("%d\n", REG[r]); pc++;
        } else if (strcmp(op,"PRINT_MEM")==0) {
            int addr = to_int(ins->tokens[1]);
            printf("%d\n", MEM[addr]); pc++;
        } else if (strcmp(op,"PRINT_VAL")==0) {
            int v = to_int(ins->tokens[1]);
            printf("%d\n", v); pc++;
        } else if (strcmp(op,"READ_SENSOR")==0) {
            int r = reg_index(ins->tokens[1]);
            int id = to_int(ins->tokens[2]);
            REG[r] = sensor_read(id);
            pc++;
        } else if (strcmp(op,"HALT")==0) {
            break;
        } else {
            fprintf(stderr,"Operação desconhecida '%s' na linha %d\n", op, pc+1);
            exit(1);
        }
        steps++;
        if (steps > 10000000) { fprintf(stderr,"Limite de passos de execução excedido\n"); exit(1); }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,"Uso: %s programa.asm\n", argv[0]);
        return 1;
    }
    for (int i=0;i<4;i++) REG[i]=0;
    for (int i=0;i<MEM_SIZE;i++) MEM[i]=0;
    parse_file(argv[1]);
    run();
    return 0;
}
