# APS - Linguagens & Paradigmas (2025/2)

### Tema da Linguagem
A linguagem é inspirada em futebol, permitindo representar jogadores, passes, jogadas e ações típicas em um programa.

### Gramática em EBNF

```ebnf
program         : { stmt } ;

stmt            : decl_stmt
                | assign_stmt
                | print_stmt
                | if_stmt
                | while_stmt ;

decl_stmt       : "jogador" NAME ";" ;
assign_stmt     : NAME "passe" expr ";" ;
print_stmt      : "torcida" "(" expr ")" ";" ;

if_stmt         : "ataque" "(" expr ")" block ["defesa" block] ;
while_stmt      : "enquanto" "partida" "(" expr ")" block ;

block           : "{" { stmt } "}" ;

expr            : term { ("+" | "-") term } ;
term            : factor { ("*" | "/") factor } ;
factor          : NUMBER
                | NAME
                | "(" expr ")" ;

NAME            : LETTER { LETTER | DIGIT } ;
NUMBER          : DIGIT { DIGIT } ;
LETTER          : "a" | … | "z" | "A" | … | "Z" ;
DIGIT           : "0" | … | "9" ;
