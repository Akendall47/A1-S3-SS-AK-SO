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
    TOKEN_AMPERSAND,  // &
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

//return token -- caller must free token.value
Token lexer_next_token(Lexer *lex);

int lexer_tokenize_all(const char *input, Token **tokens_out, int *count_out);

void lexer_free_tokens(Token *tokens, int count);

//get an udnerstandbale name for token type
const char* token_type_name(TokenType type);

#endif
