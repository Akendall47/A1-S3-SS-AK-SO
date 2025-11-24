#ifndef _LEXER_FSM_H_
#define _LEXER_FSM_H_

#include <stddef.h>

typedef enum{
    TOKEN_WORD,       // Command, argument, or filename
    TOKEN_PIPE,       // |
    TOKEN_REDIR_IN,   // <
    TOKEN_REDIR_OUT,  // >
    TOKEN_REDIR_APP,  // >>
    TOKEN_SEMICOLON,  // ;
    TOKEN_LPAREN,     // (
    TOKEN_RPAREN,     // )
    TOKEN_EOF,        // end
    TOKEN_ERROR       // error
} TokenType;


typedef struct{
    TokenType type;
    char *value;
} Token;

// lexer can only be accessed with our functions 
typedef struct Lexer Lexer;

//returns Lexer pointer, or NULL on allocation failure
Lexer* lexer_create(const char *input);

void lexer_destroy(Lexer *lex);


#endif 
