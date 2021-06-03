//
// Created by Eric on 2021-05-10 0010.
//

#ifndef CYAN_TOKEN_UNEXPECTED_EXCEPTION_HPP
#define CYAN_TOKEN_UNEXPECTED_EXCEPTION_HPP

#include <sstream>
#include <string>
#include <iostream>

class token_unexpected_exception : public std::exception {
public :
    token_unexpected_exception(int line, int column, std::wstring excepted, std::wstring found) {
        this->line = line;
        this->column = column;

        this->excepted = excepted;
        this->found = found;
    }

    int get_line() const {
        return line;
    }

    int get_column() const {
        return column;
    }

    std::wstring get_message() {
        std::wostringstream stringStream;
        stringStream << '[' << line << " : " << column + ']';
        stringStream << ' ' << excepted << " expected, but " << found << " found.";
        std::wstring copyOfStr = stringStream.str();

        return copyOfStr;
    }

private :
    int line;
    int column;

    std::wstring excepted;
    std::wstring found;
};


#endif //CYAN_TOKEN_UNEXPECTED_EXCEPTION_HPP
