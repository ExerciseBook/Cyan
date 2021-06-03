//
// Created by Eric on 2021-05-08 0008.
//

#ifndef CYAN_TOKEN_HPP
#define CYAN_TOKEN_HPP

#include <iostream>
#include <utility>
#include "token_type.hpp"
#include "../exception/token_unexpected_exception.hpp"

class token {
public:
    token(token_type type = token_type::DEFAULT, int line = 0, int column = 0) {
        this->type = type;
        this->line = line;
        this->column = column;
    }

    token(token_type type, int line, int column, wchar_t text) {
        this->type = type;
        this->line = line;
        this->column = column;

        wchar_t arr[2] = {text, 0};
        this->text = std::wstring(arr);
    }

    token(token_type type, int line, int column, std::wstring text) {
        this->type = type;
        this->line = line;
        this->column = column;
        this->text = std::move(text);
    }

    token_type get_type() const {
        return type;
    }

    std::wstring get_type_name() const {
        return token_type_get_name(type);
    }

    int get_line() const {
        return line;
    }

    int get_column() const {
        return column;
    }

    std::wstring get_text() const {
        return text;
    }

    void inline error(const std::wstring& excepted) const {
        throw token_unexpected_exception(this->get_line(), this->get_column(), excepted, token_type_get_name(this->get_type()));
    }

    void inline assert(int token_type, const std::wstring& excepted) const {
        if (this->get_type() != token_type) {
            this->error(excepted);
        }
    }

private:
    token_type type;
    int line = 0;
    int column = 0;
    std::wstring text;
};


#endif //CYAN_TOKEN_HPP
