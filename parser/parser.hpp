//
// Created by Eric on 2021-05-20 0020.
//

#ifndef CYAN_PARSER_HPP
#define CYAN_PARSER_HPP


#include "../visitor/visitor.hpp"

class parser {

public:
    explicit parser(lexer *lexer) {
        this->lexer = lexer;

        // 拿第一个 Token
        lexer->next_token_with_skip();
    }

    void comp_unit() {
        while (lexer->get_now_token().get_type() != token_type::END_OF_FILE) {
            if (lexer->get_now_token().get_type() == token_type::CONST) {
                this->const_decl();
            } else if (lexer->get_now_token().get_type() == token_type::INT) {
                this->comp_unit$2();
            } else if (lexer->get_now_token().get_type() == token_type::VOID) {
                this->func_def();
            }
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
            id.error(L"( or [ or , or =, ;");
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
            if (this->lexer->get_now_token().get_type() == token_type::SEMICOLON) {
                this->lexer->next_token_with_skip();
                break;
            } else if (this->lexer->get_now_token().get_type() != token_type::COMMA) {
                this->lexer->get_now_token().error(L"COMMA");
            }
        }

    }

    void b_type() {
        token type = lexer->next_token_with_skip();
        type.assert(token_type::INT, L"INT");
    }

    void const_def() {
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

    void const_exp$2(const token &next) {
        this->add_exp();
    }

    void const_init_val() {
        token array_start = lexer->next_token_with_skip();
        if (array_start.get_type() == token_type::PARENTHESES_OPEN) {
            // { }
            // { constInitVal (, constInitVal)* }
            while (true) {
                this->const_init_val();
                if (this->lexer->get_now_token().get_type() == token_type::COMMA) {
                    // next element
                } else if (this->lexer->get_now_token().get_type() == token_type::PARENTHESES_CLOSE) {
                    this->lexer->next_token_with_skip();
                    break;
                }
            }
        } else if (array_start.get_type() == token_type::PARENTHESES_CLOSE) {
            // break
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

    void var_decl(const token &type, const token &id, const token &third) {
//        this->b_type();

        this->var_def$2(id, third);

        while (true) {
            if (this->lexer->get_now_token().get_type() == token_type::SEMICOLON) {
                this->lexer->next_token_with_skip();
                break;
            } else if (this->lexer->get_now_token().get_type() != token_type::COMMA) {
                this->lexer->get_now_token().error(L"COMMA");
            }

            this->var_def();
        }
    }

    void var_def$2(const token &id, const token &third) {
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
            return;
        }

        if (next.get_type() == token_type::SEMICOLON) {
            return;
        }

        next.error(L", or ;");
    }

    void var_def() {
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
            return;
        }

        if (next.get_type() == token_type::SEMICOLON) {
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
            this->exp();
        }
    }

    void func_def() {
        token type = lexer->get_now_token();
        token id = lexer->next_token_with_skip();
        id.assert(token_type::IDENT, L"IDENT");
        token round_bracket = lexer->next_token_with_skip();
        round_bracket.assert(token_type::ROUND_BRACKET_OPEN, L"(");
        func_def(type, id, round_bracket);
    }

    void func_def(const token &type, const token &id, const token &round_bracket) {
        token next = this->lexer->next_token_with_skip();
        if (next.get_type() != token_type::ROUND_BRACKET_CLOSE) {
            this->func_fparams(next);
        }
        this->block();
    }

    void func_fparams(const token &first) {
        this->func_fparam(first);


        while (this->lexer->get_now_token().get_type() != token_type::ROUND_BRACKET_CLOSE) {
            this->lexer->get_now_token().assert(token_type::COMMA, L", or )");
            token t = this->lexer->next_token_with_skip();
            this->func_fparam(t);
        }
    }

    void func_fparam(const token &type) {
        type.assert(token_type::INT, L"INT");

        token ident = this->lexer->next_token_with_skip();
        ident.assert(token_type::IDENT, L"IDENT");

        // 判断掉第一个 []
        token next = this->lexer->next_token_with_skip();
        if (next.get_type() == token_type::SQUARE_BRACKET_OPEN) {
            token first_close = this->lexer->next_token_with_skip();
            first_close.assert(token_type::SQUARE_BRACKET_CLOSE, L"]");
        } else {
            return;
        }

        // 判断掉多个 [ exp ]，最后以 ） 结束
        next = this->lexer->next_token_with_skip();
        while (next.get_type() != token_type::ROUND_BRACKET_CLOSE) {
            next.assert(token_type::SQUARE_BRACKET_OPEN, L"[");
            this->exp();
            token square_close = this->lexer->next_token_with_skip();

            next = this->lexer->next_token_with_skip();
        }
    }

    void block() {
        token open = this->lexer->next_token_with_skip();
        open.assert(token_type::PARENTHESES_OPEN, L"{");
        this->block$2();
    }

    void block$2() {
        this->lexer->get_now_token().assert(token_type::PARENTHESES_OPEN, L"{");

        this->lexer->next_token_with_skip();
        while (this->lexer->get_now_token().get_type() != token_type::PARENTHESES_CLOSE) {
            this->block_item();
        }
        this->lexer->next_token_with_skip();
    }

    void block_item() {
        if (this->lexer->get_now_token().get_type() == token_type::CONST ||
            this->lexer->get_now_token().get_type() == token_type::INT) {
            this->decl();
        } else {
            this->stmt();
        }
    }

    void decl() {
        if (this->lexer->get_now_token().get_type() == token_type::CONST) {
            this->const_decl();
        } else if (this->lexer->get_now_token().get_type() == token_type::INT) {
            this->var_decl();
        } else {
            this->stmt();
        }
    }

    void stmt() {
        if (this->lexer->get_now_token().get_type() == token_type::IDENT) {
            // stat -> lVal '=' exp ';'
            // stat -> exp? ';'

            bool flag = true;
            this->exp(flag);

            if (flag) {
                // TODO 拆解
                // stat -> lVal '=' exp ';'
                token assign = this->lexer->get_now_token();
                assign.assert(token_type::ASSIGNMENT_SYMBOL, L"=");

                this->exp();
            }

            token end = this->lexer->get_now_token();
            end.assert(token_type::SEMICOLON, L";");
            this->lexer->next_token_with_skip();
        } else if (
                this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                this->lexer->get_now_token().get_type() == token_type::ADD ||
                this->lexer->get_now_token().get_type() == token_type::SUB ||
                this->lexer->get_now_token().get_type() == token_type::NOT ||
                this->lexer->get_now_token().get_type() == token_type::INT_CONST
                ) {
            this->exp();
            token end = this->lexer->get_now_token();
            end.assert(token_type::SEMICOLON, L";");
            this->lexer->next_token_with_skip();
        } else if (this->lexer->get_now_token().get_type() == token_type::SEMICOLON) {
            this->lexer->next_token_with_skip();
        } else if (this->lexer->get_now_token().get_type() == token_type::PARENTHESES_OPEN) {
            this->block$2();
        } else if (this->lexer->get_now_token().get_type() == token_type::IF) {
            this->lexer->next_token_with_skip();
            this->lexer->get_now_token().assert(token_type::ROUND_BRACKET_OPEN, L"(");

            this->cond();

            this->lexer->next_token_with_skip();
            this->lexer->get_now_token().assert(token_type::ROUND_BRACKET_CLOSE, L")");

            this->stmt();

            this->lexer->next_token_with_skip();
            if (this->lexer->get_now_token().get_type() == token_type::ELSE) {
                this->stmt();
            }
        } else if (this->lexer->get_now_token().get_type() == token_type::WHILE) {
            this->lexer->next_token_with_skip();
            this->lexer->get_now_token().assert(token_type::ROUND_BRACKET_OPEN, L"(");

            this->cond();

            this->lexer->next_token_with_skip();
            this->lexer->get_now_token().assert(token_type::ROUND_BRACKET_CLOSE, L")");

            this->stmt();
        } else if (this->lexer->get_now_token().get_type() == token_type::BREAK) {
            this->lexer->next_token_with_skip();
            this->lexer->get_now_token().assert(token_type::SEMICOLON, L";");
            this->lexer->next_token_with_skip();
        } else if (this->lexer->get_now_token().get_type() == token_type::CONTINUE) {
            this->lexer->next_token_with_skip();
            this->lexer->get_now_token().assert(token_type::SEMICOLON, L";");
            this->lexer->next_token_with_skip();
        } else if (this->lexer->get_now_token().get_type() == token_type::RETURN) {
            this->lexer->next_token_with_skip();

            if (this->lexer->get_now_token().get_type() != token_type::SEMICOLON) {
                this->add_exp();
            }

            this->lexer->get_now_token().assert(token_type::SEMICOLON, L";");
            this->lexer->next_token_with_skip();
        }
    }

    void exp() {
        bool tmp = true;
        this->exp(tmp);
    }

    void exp(bool &is_only_lval) {
        this->add_exp(is_only_lval);
    }

    void add_exp() {
        bool tmp = true;
        this->add_exp(tmp);
    }

    void add_exp(bool &is_only_lval) {
        if (
                this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                this->lexer->get_now_token().get_type() == token_type::ADD ||
                this->lexer->get_now_token().get_type() == token_type::SUB ||
                this->lexer->get_now_token().get_type() == token_type::NOT ||
                this->lexer->get_now_token().get_type() == token_type::INT_CONST ||
                this->lexer->get_now_token().get_type() == token_type::IDENT
                ) {
            this->mul_exp(is_only_lval);
        }

        while (true) {
            token op = this->lexer->get_now_token();
            if (op.get_type() != token_type::ADD && op.get_type() != token_type::SUB) {
                break;
            }

            this->lexer->next_token_with_skip();
            if (
                    this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                    this->lexer->get_now_token().get_type() == token_type::ADD ||
                    this->lexer->get_now_token().get_type() == token_type::SUB ||
                    this->lexer->get_now_token().get_type() == token_type::NOT ||
                    this->lexer->get_now_token().get_type() == token_type::INT_CONST ||
                    this->lexer->get_now_token().get_type() == token_type::IDENT
                    ) {
                this->mul_exp(is_only_lval);
            }
        }
    }

    void mul_exp(bool &is_only_lval) {
        if (
                this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                this->lexer->get_now_token().get_type() == token_type::ADD ||
                this->lexer->get_now_token().get_type() == token_type::SUB ||
                this->lexer->get_now_token().get_type() == token_type::NOT ||
                this->lexer->get_now_token().get_type() == token_type::INT_CONST ||
                this->lexer->get_now_token().get_type() == token_type::IDENT
                ) {
            this->unary_exp(is_only_lval);
        }

        while (true) {
            token op = this->lexer->get_now_token();
            if (
                    op.get_type() != token_type::MUL && op.get_type() != token_type::DIV &&
                    op.get_type() != token_type::MOD
                    ) {
                break;
            }

            this->lexer->next_token_with_skip();
            if (
                    this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                    this->lexer->get_now_token().get_type() == token_type::ADD ||
                    this->lexer->get_now_token().get_type() == token_type::SUB ||
                    this->lexer->get_now_token().get_type() == token_type::NOT ||
                    this->lexer->get_now_token().get_type() == token_type::INT_CONST ||
                    this->lexer->get_now_token().get_type() == token_type::IDENT
                    ) {
                this->unary_exp(is_only_lval);
            }
        }
    }

    void unary_exp(bool &is_only_lval) {
        if (
                this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                this->lexer->get_now_token().get_type() == token_type::INT_CONST
                ) {
            this->primary_exp(is_only_lval);
        } else if (this->lexer->get_now_token().get_type() == token_type::IDENT) {
            token now = this->lexer->get_now_token();
            token next =  this->lexer->next_token_with_skip();

            if (next.get_type() == token_type::ROUND_BRACKET_OPEN) {
                token close = this->lexer->next_token_with_skip();
                if (close.get_type() != token_type::ROUND_BRACKET_CLOSE) {
                    this->func_rparam();
                    close = this->lexer->next_token_with_skip();
                }
            } else {
                this->primary_exp$2(is_only_lval, now);
            }
        } else if (this->lexer->get_now_token().get_type() == token_type::ADD ||
                   this->lexer->get_now_token().get_type() == token_type::SUB ||
                   this->lexer->get_now_token().get_type() == token_type::NOT) {

            token op = this->lexer->get_now_token();
            this->lexer->next_token_with_skip();
            this->unary_exp(is_only_lval);
        }
    }

    void func_rparam() {
        while (this->lexer->get_now_token().get_type() != token_type::ROUND_BRACKET_CLOSE) {
            this->exp();
            token split = this->lexer->get_now_token();
            split.assert(token_type::COMMA, L",");
            this->lexer->next_token_with_skip();
        }
    }

    void primary_exp(bool &is_only_lval) {
        if (this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN) {
            is_only_lval = false;
            token first = this->lexer->get_now_token();
            this->exp();
            token third = this->lexer->get_now_token();
            third.assert(token_type::ROUND_BRACKET_CLOSE, L")");
        } else if (this->lexer->get_now_token().get_type() == token_type::IDENT) {
            is_only_lval = is_only_lval & true;
            this->l_val();
        } else if (this->lexer->get_now_token().get_type() == token_type::INT_CONST) {
            is_only_lval = false;
            this->number();
        }
    }

    void primary_exp$2(bool &is_only_lval, const token &now) {
        if (now.get_type() == token_type::ROUND_BRACKET_OPEN) {
            is_only_lval = false;
            token first = now;
            this->exp();
            token third = this->lexer->get_now_token();
            third.assert(token_type::ROUND_BRACKET_CLOSE, L")");
        } else if (now.get_type() == token_type::IDENT) {
            is_only_lval = is_only_lval & true;
            this->l_val$2(now);
        } else if (now.get_type() == token_type::INT_CONST) {
            is_only_lval = false;
            this->number$2(now);
        }
    }

    void l_val() {
        token now = this->lexer->get_now_token();
        this->lexer->next_token_with_skip();
        this->l_val$2(now);
    }

    void l_val$2(const token &ident) {
        while (this->lexer->get_now_token().get_type() == token_type::SQUARE_BRACKET_OPEN) {
            token open = this->lexer->get_now_token();
            this->exp();
            token close = this->lexer->get_now_token();
            close.assert(token_type::SQUARE_BRACKET_CLOSE, L"]");
            this->lexer->next_token_with_skip();
        }
    }

    void cond() {
        // TODO
    }

    void number() {
        token number = this->lexer->get_now_token();
        number.assert(token_type::INT_CONST, L"INT_CONST");
        this->lexer->next_token_with_skip();
    }

    void number$2(const token &number) {
        number.assert(token_type::INT_CONST, L"INT_CONST");
        this->lexer->next_token_with_skip();
    }

private:
    lexer *lexer;
};


#endif //CYAN_PARSER_HPP
