# ⚽ FutebolLang — APS Linguagens & Paradigmas (2025/2)

**Rafael Ken Reis Miyamoto**  
Turma: Linguagens e Paradigmas — 2025/2  

---

## Tema da Linguagem
A **FutebolLang** é uma linguagem inspirada no universo do futebol.  
Ela permite representar jogadores, passes, jogadas e ações típicas dentro de um programa.

Os comandos da linguagem utilizam termos como:
- `jogador` — declaração de variáveis
- `passe` — atribuição
- `torcida` — saída (impressão)
- `ataque` e `defesa` — estrutura condicional (if / else)
- `enquanto` — laço de repetição (while)

A linguagem é compilada para uma **Stack-based VM (SimpleStackVM)**, uma máquina virtual baseada em pilha com instruções como `PUSH`, `LOAD`, `STORE`, `ADD`, `SUB`, `JZ`, `JMP`, etc.

---

## Estrutura Gramatical (EBNF)

```ebnf
program     ::= { stmt } ;

stmt        ::= decl_stmt ";" 
              | assign_stmt ";" 
              | print_stmt ";" 
              | if_stmt 
              | while_stmt 
              ;

decl_stmt   ::= "jogador" NAME ;
assign_stmt ::= NAME "passe" expr ;
print_stmt  ::= "torcida" "(" expr ")" ;

if_stmt     ::= "ataque" "(" expr ")" block [ "defesa" block ] ;
while_stmt  ::= "enquanto" "(" expr ")" block ;

block       ::= "{" { stmt } "}" ;

expr        ::= logic_or ;
logic_or    ::= logic_and { "||" logic_and } ;
logic_and   ::= equality { "&&" equality } ;
equality    ::= relational { ("==" | "!=") relational } ;
relational  ::= additive { ("<" | ">" | "<=" | ">=") additive } ;
additive    ::= term { ("+" | "-") term } ;
term        ::= factor { ("*" | "/") factor } ;
factor      ::= "-" factor
              | NUMBER
              | NAME
              | "(" expr ")"
