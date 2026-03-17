%{
#include<stdio.h>

%}

DIGIT       [0-9]
ID          [a-z_][a-z0-9_]*

%%

{DIGIT}+    {
            printf("digit %s" ,yytext);
            }

%%
int main()
{
    yylex();
    return 0;
}