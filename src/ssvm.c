#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    OP_PUSH, OP_LOAD, OP_STORE,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_NEG,
    OP_EQ, OP_NEQ, OP_LT, OP_GT, OP_LE, OP_GE,
    OP_AND, OP_OR,
    OP_JZ, OP_JMP, OP_LABEL, OP_PRINT,
    OP_NOP
} OpCode;

typedef struct {
    OpCode op;
    int arg;         // usado para números, slots ou índice alvo (resolvido)
    char *label;     // usado inicialmente para JZ/JMP targets e LABEL name
    char *raw;       // linha original (debug)
} Instr;

typedef struct {
    Instr *a;
    int n, cap;
} InstrArray;

void ia_init(InstrArray *ia){ ia->a=NULL; ia->n=0; ia->cap=0; }
void ia_push(InstrArray *ia, Instr it){
    if(ia->n==ia->cap){
        ia->cap = ia->cap? ia->cap*2 : 128;
        ia->a = realloc(ia->a, ia->cap * sizeof(Instr));
    }
    ia->a[ia->n++] = it;
}

typedef struct {
    long long *a;
    int n, cap;
} Stack;
void st_init(Stack *s){ s->a=NULL; s->n=0; s->cap=0; }
void st_push(Stack *s, long long v){
    if(s->n==s->cap){ s->cap = s->cap? s->cap*2 : 128; s->a = realloc(s->a, s->cap * sizeof(long long)); }
    s->a[s->n++] = v;
}
long long st_pop(Stack *s){ if(s->n==0){ fprintf(stderr,"Runtime error: pop empty stack\n"); exit(1);} return s->a[--s->n]; }
long long st_top(Stack *s){ if(s->n==0){ fprintf(stderr,"Runtime error: top empty stack\n"); exit(1);} return s->a[s->n-1]; }

char *trim(char *s){
    while(*s && isspace((unsigned char)*s)) s++;
    if(*s==0) return s;
    char *e = s + strlen(s)-1;
    while(e>s && isspace((unsigned char)*e)) *e-- = 0;
    return s;
}

int opcode_from_token(const char *t){
    if(strcmp(t,"PUSH")==0) return OP_PUSH;
    if(strcmp(t,"LOAD")==0) return OP_LOAD;
    if(strcmp(t,"STORE")==0) return OP_STORE;
    if(strcmp(t,"ADD")==0) return OP_ADD;
    if(strcmp(t,"SUB")==0) return OP_SUB;
    if(strcmp(t,"MUL")==0) return OP_MUL;
    if(strcmp(t,"DIV")==0) return OP_DIV;
    if(strcmp(t,"NEG")==0) return OP_NEG;
    if(strcmp(t,"EQ")==0) return OP_EQ;
    if(strcmp(t,"NEQ")==0) return OP_NEQ;
    if(strcmp(t,"LT")==0) return OP_LT;
    if(strcmp(t,"GT")==0) return OP_GT;
    if(strcmp(t,"LE")==0) return OP_LE;
    if(strcmp(t,"GE")==0) return OP_GE;
    if(strcmp(t,"AND")==0) return OP_AND;
    if(strcmp(t,"OR")==0) return OP_OR;
    if(strcmp(t,"JZ")==0) return OP_JZ;
    if(strcmp(t,"JMP")==0) return OP_JMP;
    if(strcmp(t,"LABEL")==0) return OP_LABEL;
    if(strcmp(t,"PRINT")==0) return OP_PRINT;
    return OP_NOP;
}

int main(int argc, char **argv){
    if(argc < 2){ fprintf(stderr,"Usage: %s program.asm\n", argv[0]); return 1; }
    FILE *f = fopen(argv[1],"r");
    if(!f){ perror("fopen"); return 1; }

    InstrArray instrs; ia_init(&instrs);
    char line[1024];
    int lineno=0;
    while(fgets(line,sizeof(line),f)){
        lineno++;
        char *s = trim(line);
        if(*s==0) continue;
        if(s[0]=='#') continue;
        // tokenizar a linha (token e possivel argumento)
        char *tok = strtok(s," \t\n\r");
        if(!tok) continue;
        Instr it = {OP_NOP,0,NULL,NULL};
        it.raw = strdup(line);
        int code = opcode_from_token(tok);
        it.op = code;
        if(code==OP_PUSH || code==OP_LOAD || code==OP_STORE){
            char *arg = strtok(NULL," \t\n\r");
            if(!arg){ fprintf(stderr,"Parse error line %d: missing arg for %s\n", lineno, tok); return 1; }
            it.arg = atoi(arg);
        } else if(code==OP_JZ || code==OP_JMP){
            char *arg = strtok(NULL," \t\n\r");
            if(!arg){ fprintf(stderr,"Parse error line %d: missing label for %s\n", lineno, tok); return 1; }
            it.label = strdup(arg);
        } else if(code==OP_LABEL){
            char *arg = strtok(NULL," \t\n\r");
            if(!arg){ fprintf(stderr,"Parse error line %d: missing label for LABEL\n", lineno); return 1; }
            it.label = strdup(arg);
        }
        ia_push(&instrs,it);
    }
    fclose(f);

    // primeira passada: mapear labels para indices
    int n = instrs.n;
    // simple mapping arrays
    char **labels = NULL; int *lab_pos = NULL; int labs = 0; int labs_cap=0;
    for(int i=0;i<n;i++){
        Instr *it = &instrs.a[i];
        if(it->op == OP_LABEL && it->label){
            // store label -> i (next instruction after label)
            if(labs==labs_cap){ labs_cap = labs_cap? labs_cap*2 : 64; labels = realloc(labels, labs_cap*sizeof(char*)); lab_pos = realloc(lab_pos, labs_cap*sizeof(int)); }
            labels[labs] = strdup(it->label);
            lab_pos[labs] = i;
            labs++;
        }
    }
    // resolve JZ/JMP label names to indices (index of instruction AFTER label)
    for(int i=0;i<n;i++){
        Instr *it = &instrs.a[i];
        if((it->op==OP_JZ || it->op==OP_JMP) && it->label){
            int found = -1;
            for(int j=0;j<labs;j++){
                if(strcmp(labels[j], it->label)==0){
                    found = lab_pos[j];
                    break;
                }
            }
            if(found==-1){
                fprintf(stderr,"Runtime error: unknown label '%s'\n", it->label);
                return 1;
            }
            // jump to the instruction index after the LABEL line
            it->arg = found + 1;
        }
    }

    // runtime structures
    Stack st; st_init(&st);
    long long mem[1024]; memset(mem,0,sizeof(mem));

    // execute
    int pc = 0;
    while(pc < n){
        Instr *it = &instrs.a[pc];
        switch(it->op){
            case OP_PUSH:
                st_push(&st, it->arg); pc++; break;
            case OP_LOAD:
                if(it->arg < 0 || it->arg >= 1024){ fprintf(stderr,"Runtime error: LOAD invalid slot %d\n", it->arg); return 1; }
                st_push(&st, mem[it->arg]); pc++; break;
            case OP_STORE:{
                if(it->arg < 0 || it->arg >= 1024){ fprintf(stderr,"Runtime error: STORE invalid slot %d\n", it->arg); return 1; }
                long long v = st_pop(&st);
                mem[it->arg] = v;
                pc++; break;
            }
            case OP_ADD:{
                long long b = st_pop(&st); long long a = st_pop(&st); st_push(&st, a + b); pc++; break;
            }
            case OP_SUB:{
                long long b = st_pop(&st); long long a = st_pop(&st); st_push(&st, a - b); pc++; break;
            }
            case OP_MUL:{
                long long b = st_pop(&st); long long a = st_pop(&st); st_push(&st, a * b); pc++; break;
            }
            case OP_DIV:{
                long long b = st_pop(&st); long long a = st_pop(&st);
                if(b==0){ fprintf(stderr,"Runtime error: division by zero\n"); return 1; }
                st_push(&st, a / b); pc++; break;
            }
            case OP_NEG:{
                long long a = st_pop(&st); st_push(&st, -a); pc++; break;
            }
            case OP_EQ:{
                long long b = st_pop(&st); long long a = st_pop(&st); st_push(&st, a==b?1:0); pc++; break;
            }
            case OP_NEQ:{
                long long b = st_pop(&st); long long a = st_pop(&st); st_push(&st, a!=b?1:0); pc++; break;
            }
            case OP_LT:{
                long long b = st_pop(&st); long long a = st_pop(&st); st_push(&st, a < b ? 1 : 0); pc++; break;
            }
            case OP_GT:{
                long long b = st_pop(&st); long long a = st_pop(&st); st_push(&st, a > b ? 1 : 0); pc++; break;
            }
            case OP_LE:{
                long long b = st_pop(&st); long long a = st_pop(&st); st_push(&st, a <= b ? 1 : 0); pc++; break;
            }
            case OP_GE:{
                long long b = st_pop(&st); long long a = st_pop(&st); st_push(&st, a >= b ? 1 : 0); pc++; break;
            }
            case OP_AND:{
                long long b = st_pop(&st); long long a = st_pop(&st); st_push(&st, (a && b) ? 1 : 0); pc++; break;
            }
            case OP_OR:{
                long long b = st_pop(&st); long long a = st_pop(&st); st_push(&st, (a || b) ? 1 : 0); pc++; break;
            }
            case OP_JZ:{
                long long v = st_pop(&st);
                if(v == 0) pc = it->arg;
                else pc++;
                break;
            }
            case OP_JMP:{
                pc = it->arg;
                break;
            }
            case OP_LABEL:
                pc++; break;
            case OP_PRINT:{
                long long v = st_pop(&st);
                printf("%lld\n", v);
                pc++; break;
            }
            case OP_NOP:
            default:
                pc++; break;
        }
    }

    // cleanup
    for(int i=0;i<instrs.n;i++){
        free(instrs.a[i].raw);
        if(instrs.a[i].label) free(instrs.a[i].label);
    }
    free(instrs.a);
    for(int j=0;j<labs;j++) free(labels[j]);
    free(labels); free(lab_pos);
    free(st.a);
    return 0;
}
