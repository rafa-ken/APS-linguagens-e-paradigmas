# APS - Linguagens & Paradigmas (2025/2)

### Tema da Linguagem
A linguagem será inspirada em **futebol**, permitindo escrever programas que simulam ações típicas de uma partida (passes, chutes, gols, etc.).

### Gramática (versão inicial em EBNF)

```ebnf
program     = { statement } ;

statement   = assignment | action | loop | conditional ;

assignment  = "jogador" identifier "=" number ;
action      = "chutar" "(" identifier ")" 
            | "passar" "(" identifier "," identifier ")" 
            | "gol" "(" identifier ")" ;

loop        = "enquanto" condition "faça" { statement } "fim" ;
conditional = "se" condition "então" { statement } [ "senão" { statement } ] "fim" ;

condition   = identifier ("==" | "!=" | ">" | "<") number ;

identifier  = letter { letter | digit } ;
number      = digit { digit } ;
letter      = "a" | ... | "z" | "A" | ... | "Z" ;
digit       = "0" | ... | "9" ;
