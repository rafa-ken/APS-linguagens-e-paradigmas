%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE *yyin;
extern int yylineno;

#ifdef USE_PARSER
  #include "parser.tab.h"
#else
  #define JOGADOR 256
  #define PASSE   257
  #define TORCIDA 258
  #define ATAQUE  259
  #define DEFESA  260
  #define ENQUANTO 261
  #define EQ  262
  #define NEQ 263
  #define LE  264
  #define GE  265
  #define AND 266
  #define OR  267
  #define NUMBER 268
  #define NAME   269
#endif

int print_mode = 0;

void yyerror(const char *s) {
    fprintf(stderr, "Erro léxico (linha %d): %s\n", yylineno, s);
}
void print_name_token(const char *kind, const char *text) {
    printf("%s(%s)\n", kind, text);
}
%}

DIGIT   [0-9]
ID      [A-Za-z_][A-Za-z0-9_]*
NUMBER  {DIGIT}+

%%

"jogador"        { if(print_mode) printf("JOGADOR\n"); else return JOGADOR; }
"passe"          { if(print_mode) printf("PASSE\n");   else return PASSE; }
"torcida"        { if(print_mode) printf("TORCIDA\n"); else return TORCIDA; }
"ataque"         { if(print_mode) printf("ATAQUE\n");  else return ATAQUE; }
"defesa"         { if(print_mode) printf("DEFESA\n");  else return DEFESA; }
"enquanto"       { if(print_mode) printf("ENQUANTO\n");else return ENQUANTO; }

"=="             { if(print_mode) printf("EQ\n");  else return EQ; }
"!="             { if(print_mode) printf("NEQ\n"); else return NEQ; }
"<="             { if(print_mode) printf("LE\n");  else return LE; }
">="             { if(print_mode) printf("GE\n");  else return GE; }
"&&"             { if(print_mode) printf("AND\n"); else return AND; }
"||"             { if(print_mode) printf("OR\n");  else return OR; }

"("              { if(print_mode) printf("LPAREN\n");    else return '('; }
")"              { if(print_mode) printf("RPAREN\n");    else return ')'; }
"{"              { if(print_mode) printf("LBRACE\n");    else return '{'; }
"}"              { if(print_mode) printf("RBRACE\n");    else return '}'; }
";"              { if(print_mode) printf("SEMICOLON\n"); else return ';'; }
","              { if(print_mode) printf("COMMA\n");     else return ','; }
"+"              { if(print_mode) printf("PLUS\n");      else return '+'; }
"-"              { if(print_mode) printf("MINUS\n");     else return '-'; }
"*"              { if(print_mode) printf("MUL\n");       else return '*'; }
"/"              { if(print_mode) printf("DIV\n");       else return '/'; }
"<"              { if(print_mode) printf("LT\n");        else return '<'; }
">"              { if(print_mode) printf("GT\n");        else return '>'; }
"!"              { if(print_mode) printf("NOT\n");       else return '!'; }
"="              { if(print_mode) printf("ASSIGN\n");    else return '='; }

{NUMBER}         {
                    if (print_mode) print_name_token("NUMBER", yytext);
                    else {
#ifdef USE_PARSER
                      yylval.num = atoi(yytext);
#endif
                      return NUMBER;
                    }
                 }

{ID}             {
                    if (print_mode) print_name_token("NAME", yytext);
                    else {
#ifdef USE_PARSER
                      yylval.str = strdup(yytext);
#endif
                      return NAME;
                    }
                 }

[ \t\r]+         { }
\n               { yylineno++; }

.                { char msg[128]; snprintf(msg,sizeof(msg),"caractere inválido: '%s'", yytext); yyerror(msg); }

%%

#ifdef TEST_LEXER
int main(int argc, char **argv) {
  if (argc < 2) { fprintf(stderr, "Uso: %s <arquivo.src>\n", argv[0]); return 1; }
  yyin = fopen(argv[1], "r");
  if (!yyin) { perror("fopen"); return 1; }
  print_mode = 1;
  yylex();
  fclose(yyin);
  return 0;
}
#endif
