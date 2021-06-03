//
// Created by Eric on 2021-05-08 0008.
//

#ifndef CYAN_TOKEN_TYPE_HPP
#define CYAN_TOKEN_TYPE_HPP

enum token_type {
    CONST,
    COMMA,
    SEMICOLON,
    SQUARE_BRACKET_OPEN,
    SQUARE_BRACKET_CLOSE,
    ASSIGNMENT_SYMBOL,
    PARENTHESES_OPEN,
    PARENTHESES_CLOSE,
    ROUND_BRACKET_OPEN,
    ROUND_BRACKET_CLOSE,
    IF,
    ELSE,
    WHILE,
    BREAK,
    CONTINUE,
    RETURN,
    INT,
    VOID,
    MUL,
    DIV,
    ADD,
    SUB,
    MOD,
    LESS,
    LESS_OR_EQUAL,
    EQUAL,
    GREATER_OR_EQUAL,
    GREATER,
    NOTEQUAL,
    NOT,
    AND,
    OR,
    INT_CONST,
    IDENT,
    WHITE_SPACE,
    COMMENT_LINE,
    COMMENT_BLOCK,

    END_OF_FILE,
    DEFAULT,
};

static const std::wstring token_map[] = {
             L"CONST",
             L"COMMA",
             L"SEMICOLON",
             L"SQUARE_BRACKET_OPEN",
             L"SQUARE_BRACKET_CLOSE",
             L"ASSIGNMENT_SYMBOL",
             L"PARENTHESES_OPEN",
             L"PARENTHESES_CLOSE",
             L"ROUND_BRACKET_OPEN",
             L"ROUND_BRACKET_CLOSE",
             L"IF",
             L"ELSE",
             L"WHILE",
             L"BREAK",
             L"CONTINUE",
             L"RETURN",
             L"INT",
             L"VOID",
             L"MUL",
             L"DIV",
             L"ADD",
             L"SUB",
             L"MOD",
             L"LESS",
             L"LESS_OR_EQUAL",
             L"EQUAL",
             L"GREATER_OR_EQUAL",
             L"GREATER",
             L"NOTEQUAL",
             L"NOT",
             L"AND",
             L"OR",
             L"INT_CONST",
             L"IDENT",
             L"WHITE_SPACE",
             L"COMMENT_LINE",
             L"COMMENT_BLOCK",

             L"END_OF_FILE",
             L"DEFAULT"};

std::wstring token_type_get_name(token_type t) {
    return token_map[(int) t];
}

#endif //CYAN_TOKEN_TYPE_HPP
