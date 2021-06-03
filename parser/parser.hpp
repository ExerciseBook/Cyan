//
// Created by Eric on 2021-05-20 0020.
//

#ifndef CYAN_PARSER_HPP
#define CYAN_PARSER_HPP


#include "../visitor/visitor.hpp"

class parser {

public:
    explicit parser(lexer* lexer) {
        this->lexer = lexer;

        // 拿第一个 Token
        lexer->next_token_with_skip();
    }

    void comp_unit() {
        while (lexer->get_now_token().get_type() != token_type::END_OF_FILE) {
            if (lexer->get_now_token().get_type() == token_type::CONST) {
                this->const_decl();
            } else
            if (lexer->get_now_token().get_type() == token_type::INT) {
                this->comp_unit$2();
            } else
            if (lexer->get_now_token().get_type() == token_type::VOID) {
                this->func_def();
            }

            lexer->next_token_with_skip();
        }
    }

    /**
     * 区分 int 到底是函数定义还是变量定义的地方
     */
    void comp_unit$2() {
        // int a;
        // int a, b, c;
        // int a = 1;
        // int a = 2;
        // int a[] = 3;
        // int fun();
        token type = lexer->get_now_token();
        token id = lexer->next_token_with_skip();
        id.assert(token_type::IDENT, L"IDENT");
        token branch = lexer->next_token_with_skip();
        if (
                branch.get_type() != token_type::ROUND_BRACKET_OPEN &&
                branch.get_type() != token_type::SQUARE_BRACKET_OPEN &&
                branch.get_type() != token_type::COMMA &&
                branch.get_type() != token_type::SEMICOLON &&
                branch.get_type() != token_type::ASSIGNMENT_SYMBOL
        ) {
            id.error( L"( or [ or , or =, ;");
        }
        if (branch.get_type() == token_type::ROUND_BRACKET_OPEN) {
            this->func_def(type, id, branch);
        } else {
            this->var_decl(type, id, branch);
        }
    }

    void const_decl() {
        token const_token = lexer->get_now_token();
        const_token.assert(token_type::CONST, L"CONST");

        this->b_type();
        this->const_def();

        while (true) {
            token next = lexer->next_token_with_skip();
            if (next.get_type() == token_type::SEMICOLON) {
                break;
            } else if (next.get_type() != token_type::COMMA){
                next.error(L"COMMA");
            }
        }

        this->const_def();
    }

    void b_type(){
        token type = lexer->next_token_with_skip();
        type.assert(token_type::INT, L"INT");
    }

    void const_def(){
        token id = lexer->next_token_with_skip();
        id.assert(token_type::IDENT, L"IDENT");

        while (true) {
            token next = lexer->next_token_with_skip();
            if (next.get_type() == token_type::ASSIGNMENT_SYMBOL) {
                break;
            } else if (next.get_type() == token_type::SQUARE_BRACKET_OPEN) {
                this->const_exp();
                token square_bracket_close = lexer->next_token_with_skip();
                square_bracket_close.assert(token_type::SQUARE_BRACKET_CLOSE, L"]");
            } else {
                next.error(L"[, =");
            }
        }

        this->const_init_val();
    }

    void const_exp() {
        token next = lexer->next_token_with_skip();
        const_exp$2(next);
    }

    void const_exp$2(const token& next) {
        this->add_exp();
    }

    void const_init_val() {
        token array_start = lexer->next_token_with_skip();
        if (array_start.get_type() == token_type::PARENTHESES_OPEN) {
            // { }
            // { constInitVal (, constInitVal)* }
            while (true) {
                this->const_init_val();
                token split = lexer->next_token_with_skip();
                if (split.get_type() == token_type::COMMA) {
                    // next element
                } else if (split.get_type() == token_type::PARENTHESES_CLOSE) {
                    break;
                }
            }
        } else {
            // 那就不是数组了
            this->const_exp$2(array_start);
        }
    }

    void var_decl() {
        token type = lexer->get_now_token();
        token id = lexer->next_token_with_skip();
        id.assert(token_type::IDENT, L"IDENT");
        token branch = lexer->next_token_with_skip();
        if (
                branch.get_type() != token_type::SQUARE_BRACKET_OPEN &&
                branch.get_type() != token_type::COMMA &&
                branch.get_type() != token_type::SEMICOLON &&
                branch.get_type() != token_type::ASSIGNMENT_SYMBOL
                ) {
            id.error(L"[ or , or =, ;");
        }
        var_decl(type, id, branch);
    }

    void var_decl(const token& type, const token& id, const token& third) {
//        this->b_type();

        token next;
        this->var_def$2(next, id, third);

        while (true) {
            if (next.get_type() == token_type::SEMICOLON) {
                break;
            } else if (next.get_type() != token_type::COMMA){
                next.error(L"COMMA");
            }

            this->var_def(next);
        }
    }

    void var_def$2(token &ret, const token& id, const token& third) {
        // b [3][4] = {} ;
        // b = 1 ,
        // b [3] = {} ,
        // b ,
        // b ;

        id.assert(token_type::IDENT, L"IDENT");

        token next = third;
        if (next.get_type() == token_type::SQUARE_BRACKET_OPEN) {
            // ([ const_exp ])+

            // [ 被吃了

            while (true) {
                this->const_exp();                          // 吃掉 const_exp
                /* next = */ lexer->next_token_with_skip();       // 吃掉 ]
                next = lexer->next_token_with_skip();       // 拿出下一个
                if (next.get_type() != token_type::SQUARE_BRACKET_OPEN) break;
            }
        }

        if (next.get_type() == token_type::ASSIGNMENT_SYMBOL) {
            this->init_val();
            next = lexer->next_token_with_skip();
        }

        if (next.get_type() == token_type::COMMA) {
            ret = next;
            return;
        }

        if (next.get_type() == token_type::SEMICOLON) {
            ret = next;
            return;
        }

        next.error(L", or ;");
    }

    void var_def(token &ret) {
        // b [3][4] = {} ;
        // b = 1 ,
        // b [3] = {} ,
        // b ,
        // b ;

        token id = lexer->next_token_with_skip();
        id.assert(token_type::IDENT, L"IDENT");

        token next = lexer->next_token_with_skip();
        if (next.get_type() == token_type::SQUARE_BRACKET_OPEN) {
            // ([ const_exp ])+

            // [ 被吃了

            while (true) {
                this->const_exp();                          // 吃掉 const_exp
                /* next = */ lexer->next_token_with_skip();       // 吃掉 ]
                next = lexer->next_token_with_skip();       // 拿出下一个
                if (next.get_type() != token_type::SQUARE_BRACKET_OPEN) break;
            }
        }

        if (next.get_type() == token_type::ASSIGNMENT_SYMBOL) {
            this->init_val();
            next = lexer->next_token_with_skip();
        }

        if (next.get_type() == token_type::COMMA) {
            ret = next;
            return;
        }

        if (next.get_type() == token_type::SEMICOLON) {
            ret = next;
            return;
        }

        next.error(L", or ;");
    }

    void init_val() {
        token array_start = lexer->next_token_with_skip();
        if (array_start.get_type() == token_type::PARENTHESES_OPEN) {
            // { }
            // { constInitVal (, constInitVal)* }
            while (true) {
                this->const_init_val();
                token split = lexer->next_token_with_skip();
                if (split.get_type() == token_type::COMMA) {
                    // next element
                } else if (split.get_type() == token_type::PARENTHESES_CLOSE) {
                    break;
                }
            }
        } else {
            // 那就不是数组了
            this->exp$2(array_start);
        }
    }

    void exp() {
        token next = lexer->next_token_with_skip();
        this->exp$2(next);
    }

    void exp$2(const token& next) {
        this->add_exp$2(next);
    }

    void func_def() {
        token type = lexer->get_now_token();
        token id = lexer->next_token_with_skip();
        id.assert(token_type::IDENT, L"IDENT");
        token round_bracket = lexer->next_token_with_skip();
        round_bracket.assert(token_type::ROUND_BRACKET_OPEN, L"(");
        func_def(type, id, round_bracket);
    }

    void func_def(const token& type, const token& id, const token& round_bracket) {
        // TODO
    }

    void add_exp() {
        token next = lexer->next_token_with_skip();
        this->add_exp$2(next);
    }

    void add_exp$2(const token& next) {
        // TODO
    }

private:
    lexer* lexer;
};


#endif //CYAN_PARSER_HPP