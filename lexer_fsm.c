// deciding to go with a cleaner approach to our input processing logic from before
// which was creating uncessary/ duplciated functions whne there is an obvious simpler solution
// ie an FSM
// will likely use a buffer
//token has type and a value - so split input into tokens that can be dealt with by our shell logic
// then state can change based on current charcter ..stay, switch or restart , state
//so our shell can parse command easier 

#include "s3.h"
#include "lexer_fsm.h"
#include <ctype.h>

const char* token_type_name(TokenType type){
    switch (type){
        case TOKEN_WORD:      return "WORD";
        case TOKEN_PIPE:      return "PIPE";
        case TOKEN_REDIR_IN:  return "REDIR_IN";
        case TOKEN_REDIR_OUT: return "REDIR_OUT";
        case TOKEN_REDIR_APP: return "REDIR_APP";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_LPAREN:    return "LPAREN";
        case TOKEN_RPAREN:    return "RPAREN";
        case TOKEN_EOF:       return "EOF";
        case TOKEN_ERROR:     return "ERROR";
        default:              return "UNKNOWN";
    }
}

// lexer FSM
typedef enum{
    STATE_START,          // Beginning or after whitespace
    STATE_IN_WORD,        // Reading a normal word
    STATE_IN_SQUOTE,      // Inside single quotes 'text'
    STATE_IN_DQUOTE,      // Inside double quotes "text"
    STATE_AFTER_LESS,     // <
    STATE_AFTER_GREATER,  // >
    STATE_ESCAPE_NORMAL,  // After backslash normal 
    STATE_ESCAPE_DQUOTE   // After backslash " "
} LexerState;

struct Lexer{
    const char *input;    // Original input string
    const char *current;  // Current position
    char *token_buf;      // Buffer for building current token
    size_t buf_size;
    size_t buf_pos;       // Current position in buffer
    LexerState state;
};

Lexer* lexer_create(const char *input){
    Lexer *lex = malloc(sizeof(Lexer));
    if (!lex) return NULL;  //dont deref a null pointer !
    
    lex->input = input;
    lex->current = input;
    lex->buf_size = MAX_LINE;
    lex->token_buf = malloc(lex->buf_size);
    if (!lex->token_buf){
        free(lex);
        return NULL;
    }
    
    lex->buf_pos = 0;
    lex->state = STATE_START;
    
    return lex;
}

void lexer_destroy(Lexer *lex){
    if (lex) {
        free(lex->token_buf);
        free(lex);
    }
}

// buff stuff - utilities - internal

static void buf_reset(Lexer *lex){
    lex->buf_pos = 0;
    lex->token_buf[0] = '\0';
}

static void buf_append(Lexer *lex, char c){
    if (lex->buf_pos < lex->buf_size - 1){
        lex->token_buf[lex->buf_pos++] = c;
        lex->token_buf[lex->buf_pos] = '\0';
    }
}

static char buf_peek_last(Lexer *lex){
    return (lex->buf_pos > 0) ? lex->token_buf[lex->buf_pos - 1] : '\0';
}

// helpers for charcters 

static char peek(Lexer *lex){
    return *lex->current;
}

static char peek_next(Lexer *lex){
    return lex->current[1];
}

//move as long as not at end 
static void advance(Lexer *lex){
    if (*lex->current != '\0'){
        lex->current++;
    }
}

static int is_operator_char(char c){
    return c == '|' || c == '<' || c == '>' || c == ';' || c == '(' || c == ')';
}

static int is_word_char(char c){
    return c != '\0' && !isspace(c) && !is_operator_char(c) && c != '\'' && c != '"';
}