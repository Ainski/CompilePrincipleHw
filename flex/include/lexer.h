#ifndef LEXER_H
#define LEXER_H

class Lexer {
    public:
        static int line_number;
        Lexer();
        ~Lexer();
        int read();
};
#endif