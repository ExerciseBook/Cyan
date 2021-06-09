//
// Created by Eric on 2021-05-08 0008.
//

#ifndef CYAN_LEXER_HPP
#define CYAN_LEXER_HPP

#include <fstream>
#include "token.hpp"
#include "../exception/eof_exception.hpp"
#include "../exception/token_unexpected_exception.hpp"

class lexer {
private:

    // 原始数据
    std::wistream *input_stream;

    // 当前扫描到原始数据的位置
    int now_position = 0;

    // 当前行数
    int now_line = 0;

    // 当前列数
    int now_column = 0;

    // 当前字符
    wchar_t *now_char = nullptr;

    // 下一个字符
    wchar_t *next_char = nullptr;

public:
    explicit lexer(std::wistream &in) {
        this->input_stream = &in;
        get_next_char();
        now_line = 1;
        now_column = 0;
    }

    token next_token() {
        auto _now_char = get_next_char();
        if (_now_char == nullptr) {
            return {token_type::END_OF_FILE, now_position, now_line, now_column + 1};
        }

        token ret;

        int _start_line = now_line;
        int _start_column = now_column;

        switch (*_now_char) {
            case '\r' : {
                if ((next_char != nullptr) && (*next_char == '\n')) {
                    get_next_char();
                }
                ret = token(token_type::WHITE_SPACE, now_position, _start_line, _start_column, *_now_char);
                now_line++;
                now_column = 0;
                break;
            }
            case '\n' : {
                ret = token(token_type::WHITE_SPACE, now_position, _start_line, _start_column, *_now_char);
                now_line++;
                now_column = 0;
                break;
            }
            case '\t' :
            case ' ' : {
                ret = token(token_type::WHITE_SPACE, now_position, _start_line, _start_column, *_now_char);
                break;
            }
            case ',' : {
                ret = token(token_type::COMMA, now_position, _start_line, _start_column, *_now_char);
                break;
            }
            case ';' : {
                ret = token(token_type::SEMICOLON, now_position, _start_line, _start_column, *_now_char);
                break;
            }
            case '[' : {
                ret = token(token_type::SQUARE_BRACKET_OPEN, now_position, _start_line, _start_column, *_now_char);
                break;
            }
            case ']' : {
                ret = token(token_type::SQUARE_BRACKET_CLOSE, now_position, _start_line, _start_column, *_now_char);
                break;
            }
            case '{' : {
                ret = token(token_type::PARENTHESES_OPEN, now_position, _start_line, _start_column, *_now_char);
                break;
            }
            case '}' : {
                ret = token(token_type::PARENTHESES_CLOSE, now_position, _start_line, _start_column, *_now_char);
                break;
            }
            case '(' : {
                ret = token(token_type::ROUND_BRACKET_OPEN, now_position, _start_line, _start_column, *_now_char);
                break;
            }
            case ')' : {
                ret = token(token_type::ROUND_BRACKET_CLOSE, now_position, _start_line, _start_column, *_now_char);
                break;
            }
            case '=' : {
                if ((next_char != nullptr) && (*next_char == '=')) {
                    ret = token(token_type::EQUAL, now_position, _start_line, _start_column, L"==");
                    free(next_char);
                    get_next_char();
                } else {
                    ret = token(token_type::ASSIGNMENT_SYMBOL, now_position, _start_line, _start_column, *_now_char);
                }
                break;
            }
            case '*' : {
                ret = token(token_type::MUL, now_position, _start_line, _start_column, *_now_char);
                break;
            }
            case '/' : {
                if (next_char == nullptr) {
                    ret = token(token_type::DIV, now_position, _start_line, _start_column, *_now_char);
                } else if (*next_char == '/') {
                    // 单行注释

                    std::wstring text = L"/";
                    text += *next_char;

                    free(next_char);
                    get_next_char();

                    while (
                            (next_char != nullptr) &&
                            (
                                ((*next_char) != '\r') && ((*next_char) != '\n')
                            )
                    ) {
                        get_next_char();

                        text += *now_char;
                        free(now_char);
                    }

                    ret = token(token_type::COMMENT_LINE, now_position, _start_line, _start_column, text);
                } else if (*next_char == '*') {
                    // 多行注释
                    std::wstring text = L"/";
                    text += *next_char;

                    free(next_char);
                    get_next_char();
                    get_next_char();

                    while (
                        (now_char != nullptr) && (next_char != nullptr) &&
                        (
                            ((*now_char) != '*') || ((*next_char) != '/')
                        )
                    ) {
                        if ((*now_char) == '\n') {
                            now_line++;
                            now_column = 0;

                            text += *now_char;
                            free(now_char);
                            get_next_char();
                        } else if (((*now_char) == '\r') && ((*now_char) == '\n')) {
                            now_line++;
                            now_column = 0;

                            text += *now_char;
                            free(now_char);
                            get_next_char();

                            text += *now_char;
                            free(now_char);
                            get_next_char();
                        } else if (((*now_char) == '\r') && ((*now_char) != '\n')) {
                            now_line++;
                            now_column = 0;

                            text += *now_char;
                            free(now_char);
                            get_next_char();
                        } else {
                            text += *now_char;
                            free(now_char);
                            get_next_char();
                        }
                    }

                    free(now_char);
                    free(next_char);
                    get_next_char();
                    text += L"*/";

                    ret = token(token_type::COMMENT_BLOCK, now_position, _start_line, _start_column, text);
                } else {
                    ret = token(token_type::DIV, now_position, _start_line, _start_column, *_now_char);
                }
                break;
            }
            case '+' : {
                ret = token(token_type::ADD, now_position, _start_line, _start_column, *_now_char);
                break;
            }
            case '-' : {
                ret = token(token_type::SUB, now_position, _start_line, _start_column, *_now_char);
                break;
            }
            case '%' : {
                ret = token(token_type::MOD, now_position, _start_line, _start_column, *_now_char);
                break;
            }
            case '<' : {
                if (next_char == nullptr) {
                    ret = token(token_type::LESS, now_position, _start_line, _start_column, *_now_char);
                } else if (*next_char == '=') {
                    free(next_char);
                    get_next_char();
                    ret = token(token_type::LESS_OR_EQUAL, now_position, _start_line, _start_column, L"<=");
                } else {
                    ret = token(token_type::LESS, now_position, _start_line, _start_column, *_now_char);
                }
                break;
            }
            case '>' : {
                if (next_char == nullptr) {
                    ret = token(token_type::GREATER, now_position, _start_line, _start_column, *_now_char);
                } else if (*next_char == '=') {
                    free(next_char);
                    get_next_char();
                    ret = token(token_type::GREATER_OR_EQUAL, now_position, _start_line, _start_column, L">=");
                } else {
                    ret = token(token_type::GREATER, now_position, _start_line, _start_column, *_now_char);
                }
                break;
            }
            case '!' : {
                if (next_char == nullptr) {
                    ret = token(token_type::NOT, now_position, _start_line, _start_column, *_now_char);
                } else if (*next_char == '=') {
                    free(next_char);
                    get_next_char();
                    ret = token(token_type::NOTEQUAL, now_position, _start_line, _start_column, L"!=");
                } else {
                    ret = token(token_type::NOT, now_position, _start_line, _start_column, *_now_char);
                }
                break;
            }
            case '&' : {
                if (next_char == nullptr) {
                    free(_now_char);
                    throw token_unexpected_exception(now_line, now_column, L"&&", L"&");
                } else if (*next_char == '&') {
                    free(next_char);
                    get_next_char();
                    ret = token(token_type::AND, now_position, _start_line, _start_column, L"&&");
                } else {
                    free(_now_char);
                    throw token_unexpected_exception(now_line, now_column, L"&&", L"&");
                }
                break;
            }
            case '|' : {
                if (next_char == nullptr) {
                    free(_now_char);
                    throw token_unexpected_exception(now_line, now_column, L"||", L"|");
                } else if (*next_char == '|') {
                    free(next_char);
                    get_next_char();
                    ret = token(token_type::OR, now_position, _start_line, _start_column, L"||");
                } else {
                    free(_now_char);
                    throw token_unexpected_exception(now_line, now_column, L"||", L"|");
                }
                break;
            }
            case '0':
                if (next_char == nullptr) {
                    // 0
                    ret = token(token_type::INT_CONST, now_position, _start_line, _start_column, L"0");
                } else if ((*next_char == 'x') || (*next_char == 'X')) {
                    std::wstring text = L"0";
                    text += *next_char;

                    // HEX
                    free(next_char);
                    get_next_char();

                    while (
                            (next_char != nullptr) &&
                            (
                                    (('0' <= (*next_char)) && ((*next_char) <= '9')) ||
                                    (('a' <= (*next_char)) && ((*next_char) <= 'f')) ||
                                    (('A' <= (*next_char)) && ((*next_char) <= 'F'))
                            )
                    ) {
                        get_next_char();

                        text += *now_char;
                        free(now_char);
                    }

                    if (text.length() == 2) {
                        free(_now_char);
                        throw token_unexpected_exception(now_line, now_column, L"hex integer", L"invalid found");
                    }

                    ret = token(token_type::INT_CONST, now_position, _start_line, _start_column, text);
                } else if (('0' <= (*next_char)) && ((*next_char) <= '7')) {
                    std::wstring text = L"0";
                    text += *next_char;

                    // HEX
                    free(next_char);
                    get_next_char();

                    while (
                            (next_char != nullptr) &&
                            (
                                    (('0' <= (*next_char)) && ((*next_char) <= '9')) ||
                                    (('a' <= (*next_char)) && ((*next_char) <= 'f')) ||
                                    (('A' <= (*next_char)) && ((*next_char) <= 'F'))
                            )
                    ) {
                        if (('0' <= (*next_char)) && ((*next_char) <= '7')) {
                            get_next_char();

                            text += *now_char;
                            free(now_char);
                        } else {
                            free(_now_char);
                            throw token_unexpected_exception(now_line, now_column, L"oct integer", L"invalid found");
                        }
                    }

                    ret = token(token_type::INT_CONST, now_position, _start_line, _start_column, text);
                } else {
                    // 0
                    ret = token(token_type::INT_CONST, now_position, _start_line, _start_column, L"0");
                }
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                // DEC

                std::wstring text;
                text += *now_char;

                while (
                        (next_char != nullptr) &&
                        (
                                (('0' <= (*next_char)) && ((*next_char) <= '9')) ||
                                (('a' <= (*next_char)) && ((*next_char) <= 'f')) ||
                                (('A' <= (*next_char)) && ((*next_char) <= 'F'))
                        )
                        ) {
                    if (('0' <= (*next_char)) && ((*next_char) <= '9')) {
                        get_next_char();

                        text += *now_char;
                        free(now_char);
                    } else {
                        free(_now_char);
                        throw token_unexpected_exception(now_line, now_column, L"dec integer", L"invalid found");
                    }
                }

                ret = token(token_type::INT_CONST, now_position, _start_line, _start_column, text);

                break;
            }
            case '\0' : {
                ret = token(token_type::END_OF_FILE, now_position, _start_line, _start_column, *_now_char);
                break;
            }
            default: {
                std::wstring text;
                text += *now_char;

                while (
                        (next_char != nullptr) &&
                        (
                            (*next_char  != '\r') && (*next_char != '\n') &&
                            (*next_char  != '\t') && (*next_char != ' ') &&
                            (*next_char  != ',') && (*next_char != ';') &&
                            (*next_char  != '[') && (*next_char != ']') &&
                            (*next_char  != '{') && (*next_char != '}') &&
                            (*next_char  != '(') && (*next_char != ')') &&
                            (*next_char  != '<') && (*next_char != '>') &&
                            (*next_char  != '*') && (*next_char != '/') &&
                            (*next_char  != '+') && (*next_char != '-') &&
                            (*next_char  != '!') && (*next_char != '%') &&
                            (*next_char  != '&') && (*next_char != '|')
                        )
                ) {
                    get_next_char();

                    text += *now_char;
                    free(now_char);
                }

                if (text == L"const") {
                    ret = token(token_type::CONST, now_position, _start_line, _start_column, text);
                } else if (text == L"if") {
                    ret = token(token_type::IF, now_position, _start_line, _start_column, text);
                } else if (text == L"else") {
                    ret = token(token_type::ELSE, now_position, _start_line, _start_column, text);
                } else if (text == L"while") {
                    ret = token(token_type::WHILE, now_position, _start_line, _start_column, text);
                } else if (text == L"break") {
                    ret = token(token_type::BREAK, now_position, _start_line, _start_column, text);
                } else if (text == L"continue") {
                    ret = token(token_type::CONTINUE, now_position, _start_line, _start_column, text);
                } else if (text == L"return") {
                    ret = token(token_type::RETURN, now_position, _start_line, _start_column, text);
                } else if (text == L"int") {
                    ret = token(token_type::INT, now_position, _start_line, _start_column, text);
                } else if (text == L"void") {
                    ret = token(token_type::VOID, now_position, _start_line, _start_column, text);
                } else {
                    ret = token(token_type::IDENT, now_position, _start_line, _start_column, text);
                }

                break;
            }
        }

        free(_now_char);

        this->now_token = ret;
        return ret;
    }

    token get_now_token() {
        return this->now_token;
    }

    token next_token_with_skip() {
        do {
            next_token();
        } while (
                get_now_token().get_type() == token_type::COMMENT_BLOCK ||
                get_now_token().get_type() == token_type::COMMENT_LINE ||
                get_now_token().get_type() == token_type::WHITE_SPACE
        );
        return get_now_token();
    }

private:
    wchar_t *get_next_char() {
        if (input_stream->eof()) {
            now_char = next_char;
            next_char = nullptr;
            return now_char;
        }

        now_position++;
        now_column++;

        auto *ret = new wchar_t();
        input_stream->get(*ret);

        now_char = next_char;
        next_char = ret;
        return now_char;
    }

    token now_token;
};


#endif //CYAN_LEXER_HPP
