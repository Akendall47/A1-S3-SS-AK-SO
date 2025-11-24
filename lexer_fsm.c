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


//Token Makers 
static Token make_token(Lexer *lex, TokenType type){
    Token tok;
    tok.type = type;
    tok.value = strdup(lex->token_buf);
    return tok;
}

static Token make_error_token(const char *message){
    Token tok;
    tok.type = TOKEN_ERROR;
    tok.value = strdup(message);
    return tok;
}

static Token make_simple_token(Lexer *lex, TokenType type, const char *value){
    buf_reset(lex);
    buf_append(lex, *value);
    return make_token(lex, type);
}


/// main FSM LOGIC

Token lexer_next_token(Lexer *lex){
    buf_reset(lex);
    
    while (1){
        char c = peek(lex);

        switch (lex->state){
        case STATE_START:
            if (isspace(c)){
                advance(lex);
                continue;
            }
            if (c == '\0'){
                return make_token(lex, TOKEN_EOF);
            }
            if (c == '|'){
                advance(lex);
                return make_simple_token(lex, TOKEN_PIPE, "|");
            }
            if (c == ';'){
                advance(lex);
                return make_simple_token(lex, TOKEN_SEMICOLON, ";");
            }
            if (c == '('){
                advance(lex);
                return make_simple_token(lex, TOKEN_LPAREN, "(");
            }
            if (c == ')'){
                advance(lex);
                return make_simple_token(lex, TOKEN_RPAREN, ")");
            }
            
            if (c == '<'){
                advance(lex);
                lex->state = STATE_AFTER_LESS;
                buf_append(lex, c);
                continue;
            }
            if (c == '>'){
                advance(lex);
                lex->state = STATE_AFTER_GREATER;
                buf_append(lex, c);
                continue;
            }
            
            if (c == '\''){
                advance(lex);
                lex->state = STATE_IN_SQUOTE;
                continue;
            }
            if (c == '"'){
                advance(lex);
                lex->state = STATE_IN_DQUOTE;
                continue;
            }
            
            if (c == '\\'){
                advance(lex);
                lex->state = STATE_ESCAPE_NORMAL;
                continue;
            }
            
            if (is_word_char(c)){
                buf_append(lex, c);
                advance(lex);
                lex->state = STATE_IN_WORD;
                continue;
            }
            
            advance(lex);
            return make_error_token("Unexpected character");
            
        case STATE_IN_WORD:
            if (is_word_char(c)){
                buf_append(lex, c);
                advance(lex);
                continue;
            }
            if (c == '\\'){
                advance(lex);
                lex->state = STATE_ESCAPE_NORMAL;
                continue;
            }
            if (c == '"'){
                advance(lex);
                lex->state = STATE_IN_DQUOTE;
                continue;
            }
            if (c == '\''){
                advance(lex);
                lex->state = STATE_IN_SQUOTE;
                continue;
            }
            lex->state = STATE_START;
            return make_token(lex, TOKEN_WORD);
            
        case STATE_IN_SQUOTE:
            if (c == '\0'){
                return make_error_token("Unterminated single quote");
            }
            if (c == '\''){
                advance(lex);
                if (is_word_char(peek(lex)) || peek(lex) == '"' || peek(lex) == '\''){
                    lex->state = STATE_IN_WORD;
                } else {
                    lex->state = STATE_START;
                    return make_token(lex, TOKEN_WORD);
                }
                continue;
            }
            buf_append(lex, c);
            advance(lex);
            continue;
            
        case STATE_IN_DQUOTE:
            if (c == '\0'){
                return make_error_token("Unterminated double quote");
            }
            if (c == '"'){
                advance(lex);
                if (is_word_char(peek(lex)) || peek(lex) == '\'' || peek(lex) == '"'){
                    lex->state = STATE_IN_WORD;
                } else {
                    lex->state = STATE_START;
                    return make_token(lex, TOKEN_WORD);
                }
                continue;
            }
            if (c == '\\'){
                advance(lex);
                lex->state = STATE_ESCAPE_DQUOTE;
                continue;
            }
            buf_append(lex, c);
            advance(lex);
            continue;
            
        case STATE_AFTER_LESS:
            lex->state = STATE_START;
            return make_token(lex, TOKEN_REDIR_IN);
            
        case STATE_AFTER_GREATER:
            if (c == '>'){
                buf_append(lex, c);
                advance(lex);
                lex->state = STATE_START;
                return make_token(lex, TOKEN_REDIR_APP);
            }
            lex->state = STATE_START;
            return make_token(lex, TOKEN_REDIR_OUT);
            
        case STATE_ESCAPE_NORMAL:
            if (c == '\0'){
                return make_error_token("Backslash at end of input");
            }
            buf_append(lex, c);
            advance(lex);
            lex->state = STATE_IN_WORD;
            continue;
            
        case STATE_ESCAPE_DQUOTE:
            if (c == '\0'){
                return make_error_token("Backslash at end of input");
            }
            if (c == '"' || c == '\\' || c == '$' || c == '`' || c == '\n'){
                buf_append(lex, c);
            } else {
                buf_append(lex, '\\');
                buf_append(lex, c);
            }
            advance(lex);
            lex->state = STATE_IN_DQUOTE;
            continue;
        }
    }
}

int lexer_tokenize_all(const char *input, Token **tokens_out, int *count_out){
    Lexer *lex = lexer_create(input);
    if (!lex) return -1;
    
    int capacity = 32;
    int count = 0;
    Token *tokens = malloc(sizeof(Token) * capacity);
    if (!tokens){
        lexer_destroy(lex);
        return -1;
    }
    
    while (1){
        Token tok = lexer_next_token(lex);
        
        if (tok.type == TOKEN_ERROR){
            fprintf(stderr, "Lexer error: %s\n", tok.value);
            free(tok.value);
            for (int i = 0; i < count; i++){
                free(tokens[i].value);
            }
            free(tokens);
            lexer_destroy(lex);
            return -1;
        }
        
        if (count >= capacity){
            capacity *= 2;
            Token *new_tokens = realloc(tokens, sizeof(Token) * capacity);
            if (!new_tokens) {
                for (int i = 0; i < count; i++){
                    free(tokens[i].value);
                }
                free(tokens);
                free(tok.value);
                lexer_destroy(lex);
                return -1;
            }
            tokens = new_tokens;
        }
        
        tokens[count++] = tok;
        
        if (tok.type == TOKEN_EOF){
            break;
        }
    }
    
    lexer_destroy(lex);
    *tokens_out = tokens;
    *count_out = count;
    return 0;
}

void lexer_free_tokens(Token *tokens, int count){
    for (int i = 0; i < count; i++) {
        free(tokens[i].value);
    }
    free(tokens);
}