//
// Created by Eric on 2021-05-20 0020.
//

#ifndef CYAN_PARSER_HPP
#define CYAN_PARSER_HPP


#include "../visitor/visitor.hpp"
#include "../ast/node.hpp"

class parser {

public:
    explicit parser(lexer *lexer) {
        this->lexer = lexer;

        // 拿第一个 Token
        lexer->next_token_with_skip();
    }

    ast::comp_unit* comp_unit() {
        auto ret = new ast::comp_unit();

        while (lexer->get_now_token().get_type() != token_type::END_OF_FILE) {
            if (lexer->get_now_token().get_type() == token_type::CONST) {
                auto decl = new ast::decl();
                auto p = this->const_decl();
                decl->append_children(p);
                ret->append_children(decl);
            } else if (lexer->get_now_token().get_type() == token_type::INT) {
                auto p = this->comp_unit$2();
                ret->append_children(p);
            } else if (lexer->get_now_token().get_type() == token_type::VOID) {
                auto p = this->func_def();
                ret->append_children(p);
            }
        }

        return ret;
    }

    /**
     * 区分 int 到底是函数定义还是变量定义的地方
     */
    ast::node* comp_unit$2() {
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
            return this->func_def(type, id, branch);
        } else {
            auto decl = new ast::decl();
            auto var_decl = this->var_decl(type, id, branch);
            decl->append_children(var_decl);
            return decl;
        }
    }

    ast::const_decl* const_decl() {
        auto ret = new ast::const_decl();

        token const_token = lexer->get_now_token();
        const_token.assert(token_type::CONST, L"CONST");
        lexer->next_token_with_skip();
        ret->append_children(new ast::_const(const_token));

        auto b_type = this->b_type();
        ret->append_children(b_type);

        auto const_def = this->const_def();
        ret->append_children(const_def);

        while (true) {
            if (this->lexer->get_now_token().get_type() == token_type::SEMICOLON) {
                ret->append_children(new ast::_semicolon(this->lexer->get_now_token()));
                this->lexer->next_token_with_skip();
                break;
            } else if (this->lexer->get_now_token().get_type() != token_type::COMMA) {
                this->lexer->get_now_token().error(L"COMMA");
            }
            ret->append_children(new ast::_semicolon(this->lexer->get_now_token()));
            this->lexer->next_token_with_skip();

            auto mult_const_def = this->const_def();
            ret->append_children(mult_const_def);
        }

        return ret;
    }

    ast::b_type* b_type() {
        auto ret = new ast::b_type();

        token type = lexer->get_now_token();
        type.assert(token_type::INT, L"INT");
        ret->append_children(new ast::_int(type));
        lexer->next_token_with_skip();

        return ret;
    }

    ast::const_def* const_def() {
        auto ret = new ast::const_def();

        token id = lexer->get_now_token();
        id.assert(token_type::IDENT, L"IDENT");
        ret->append_children(new ast::_ident(id));
        lexer->next_token_with_skip();

        while (true) {
            token now = lexer->get_now_token();
            if (now.get_type() == token_type::ASSIGNMENT_SYMBOL) {
                ret->append_children(new ast::_assignment_symbol(now));
                lexer->next_token_with_skip();
                break;
            } else if (now.get_type() == token_type::SQUARE_BRACKET_OPEN) {
                ret->append_children(new ast::_square_bracket_open(now));
                lexer->next_token_with_skip();

                ret->append_children(this->const_exp());

                token square_bracket_close = lexer->get_now_token();
                square_bracket_close.assert(token_type::SQUARE_BRACKET_CLOSE, L"]");
                ret->append_children(new ast::_square_bracket_close(square_bracket_close));

                lexer->next_token_with_skip();
            } else {
                now.error(L"[, =");
            }
        }

        this->const_init_val();

        return ret;
    }

    ast::const_exp* const_exp() {
        auto ret = new ast::const_exp();
        ret->append_children(this->add_exp());
        return ret;
    }

    ast::const_init_val* const_init_val() {
        auto ret = new ast::const_init_val();

        token array_start = lexer->get_now_token();
        if (array_start.get_type() == token_type::PARENTHESES_OPEN) {
            ret->append_children(new ast::_parentheses_open(array_start));
            lexer->next_token_with_skip();

            // { }
            // { constInitVal (, constInitVal)* }
            while (true) {
                auto const_init_val = this->const_init_val();
                ret->append_children(const_init_val);

                if (this->lexer->get_now_token().get_type() == token_type::COMMA) {
                    ret->append_children(new ast::_comma(this->lexer->get_now_token()));
                    lexer->next_token_with_skip();
                    // next element
                } else if (this->lexer->get_now_token().get_type() == token_type::PARENTHESES_CLOSE) {
                    ret->append_children(new ast::_parentheses_close(this->lexer->get_now_token()));
                    this->lexer->next_token_with_skip();
                    break;
                }
            }
        } else if (array_start.get_type() == token_type::PARENTHESES_CLOSE) {
            // break
        } else {
            // 那就不是数组了
            auto const_exp = this->const_exp();
            ret->append_children(const_exp);
        }

        return ret;
    }

    ast::var_decl* var_decl() {
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
        return var_decl(type, id, branch);
    }

    ast::var_decl* var_decl(const token &type, const token &id, const token &third) {
        auto ret = new ast::var_decl();

//        this->b_type();
        auto b_type = new ast::b_type();
        b_type->append_children(new ast::_int(type));
        ret->append_children(b_type);

        auto var_def = this->var_def$2(id, third);
        ret->append_children(var_def);

        while (true) {
            if (this->lexer->get_now_token().get_type() == token_type::SEMICOLON) {
                ret->append_children(new ast::_semicolon(this->lexer->get_now_token()));
                this->lexer->next_token_with_skip();
                break;
            } else if (this->lexer->get_now_token().get_type() != token_type::COMMA) {
                this->lexer->get_now_token().error(L"COMMA");
            }
            ret->append_children(new ast::_comma(this->lexer->get_now_token()));
            this->lexer->next_token_with_skip();

            auto mult_var_def = this->var_def();
            ret->append_children(mult_var_def);
        }

        return ret;
    }

    ast::var_def* var_def$2(const token &id, const token &third) {
        auto ret = new ast::var_def();

        // b [3][4] = {} ;
        // b = 1 ,
        // b [3] = {} ,
        // b ,
        // b ;

        id.assert(token_type::IDENT, L"IDENT");
        ret->append_children(new ast::_ident(id));

        token next = third;
        if (next.get_type() == token_type::SQUARE_BRACKET_OPEN) {
            // ([ const_exp ])+

            while (true) {
                ret->append_children(new ast::_square_bracket_open(lexer->get_now_token()));
                lexer->next_token_with_skip(); // 吃掉 [

                auto const_exp = this->const_exp();                          // 吃掉 const_exp
                ret->append_children(const_exp);

                next = lexer->get_now_token();       // 吃掉 ]
                next.assert(token_type::SQUARE_BRACKET_CLOSE, L"]");
                ret->append_children(new ast::_square_bracket_close(next));
                next = lexer->next_token_with_skip();

                if (next.get_type() != token_type::SQUARE_BRACKET_OPEN) break;
            }
        }

        if (next.get_type() == token_type::ASSIGNMENT_SYMBOL) {
            ret->append_children(new ast::_assignment_symbol(next));
            lexer->next_token_with_skip(); // 吃掉 =

            auto init_val = this->init_val();
            ret->append_children(init_val);

            next = lexer->get_now_token();
        }

        if (next.get_type() == token_type::COMMA) {
            return ret;
        }

        if (next.get_type() == token_type::SEMICOLON) {
            return ret;
        }

        next.error(L", or ;");
        return nullptr;
    }

    ast::var_def* var_def() {
        auto ret = new ast::var_def();

        // b [3][4] = {} ;
        // b = 1 ,
        // b [3] = {} ,
        // b ,
        // b ;

        token id = lexer->get_now_token();
        id.assert(token_type::IDENT, L"IDENT");
        ret->append_children(new ast::_ident(id));

        token next = lexer->next_token_with_skip();
        if (next.get_type() == token_type::SQUARE_BRACKET_OPEN) {
            // ([ const_exp ])+

            while (true) {
                ret->append_children(new ast::_square_bracket_open(lexer->get_now_token()));
                lexer->next_token_with_skip(); // 吃掉 [

                auto const_exp = this->const_exp();                          // 吃掉 const_exp
                ret->append_children(const_exp);

                next = lexer->get_now_token();       // 吃掉 ]
                next.assert(token_type::SQUARE_BRACKET_CLOSE, L"]");
                ret->append_children(new ast::_square_bracket_close(next));
                next = lexer->next_token_with_skip();

                if (next.get_type() != token_type::SQUARE_BRACKET_OPEN) break;
            }
        }

        if (next.get_type() == token_type::ASSIGNMENT_SYMBOL) {
            ret->append_children(new ast::_assignment_symbol(next));
            lexer->next_token_with_skip(); // 吃掉 =

            auto init_val = this->init_val();
            ret->append_children(init_val);

            next = lexer->get_now_token();
        }

        if (next.get_type() == token_type::COMMA) {
            return ret;
        }

        if (next.get_type() == token_type::SEMICOLON) {
            return ret;
        }

        next.error(L", or ;");
        return nullptr;
    }

    ast::init_val* init_val() {
        auto ret = new ast::init_val();

        token array_start = lexer->get_now_token();
        if (array_start.get_type() == token_type::PARENTHESES_OPEN) {
            ret->append_children(new ast::_parentheses_open(array_start));
            lexer->next_token_with_skip();

            // { }
            if (lexer->get_now_token().get_type() == token_type::PARENTHESES_CLOSE) {
                ret->append_children(new ast::_parentheses_close(lexer->get_now_token()));
                lexer->next_token_with_skip();
                return ret;
            }

            // { initVal (, initVal)* }
            while (true) {
                auto init_val = this->init_val();
                ret->append_children(init_val);

                token split = lexer->get_now_token();
                if (split.get_type() == token_type::COMMA) {
                    ret->append_children(new ast::_comma(split));
                    lexer->next_token_with_skip();
                    // next element
                } else if (split.get_type() == token_type::PARENTHESES_CLOSE) {
                    ret->append_children(new ast::_parentheses_close(split));
                    lexer->next_token_with_skip();
                    break;
                }
            }
        } else {
            // 那就不是数组了
            ret->append_children(this->exp());
        }

        return ret;
    }

    ast::func_def* func_def() {
        token type = lexer->get_now_token();
        token id = lexer->next_token_with_skip();
        id.assert(token_type::IDENT, L"IDENT");
        token round_bracket = lexer->next_token_with_skip();
        round_bracket.assert(token_type::ROUND_BRACKET_OPEN, L"(");
        return func_def(type, id, round_bracket);
    }

    ast::func_def* func_def(const token &type, const token &id, const token &round_bracket_open) {
        auto ret = new ast::func_def();

        auto func_type = new ast::func_type();
        if (type.get_type() == token_type::VOID) {
            func_type->append_children(new ast::_void(type));
        } else if (type.get_type() == token_type::INT) {
            func_type->append_children(new ast::_int(type));
        } else {
            type.error(L"void, int");
        }
        ret->append_children(func_type);

        ret->append_children(new ast::_ident(id));
        ret->append_children(new ast::_round_bracket_open(round_bracket_open));

        token next = this->lexer->next_token_with_skip();
        if (next.get_type() != token_type::ROUND_BRACKET_CLOSE) {
            auto func_fparams = this->func_fparams(next);
            ret->append_children(func_fparams);
        }
        auto round_bracket_close = this->lexer->get_now_token();
        ret->append_children(new ast::_round_bracket_close(round_bracket_close));
        this->lexer->next_token_with_skip();

        auto block = this->block();
        ret->append_children(block);

        return ret;
    }

    ast::func_fparams* func_fparams(const token &first) {
        auto ret = new ast::func_fparams();

        auto func_fparam = this->func_fparam(first);
        ret->append_children(func_fparam);

        while (this->lexer->get_now_token().get_type() != token_type::ROUND_BRACKET_CLOSE) {
            token comma = this->lexer->get_now_token();
            comma.assert(token_type::COMMA, L", or )");
            ret->append_children(new ast::_comma(comma));

            token t = this->lexer->next_token_with_skip();
            auto mult_func_fparam = this->func_fparam(t);
            ret->append_children(mult_func_fparam);
        }

        return ret;
    }

    ast::func_fparam* func_fparam(const token &type) {
        auto ret = new ast::func_fparam();

        type.assert(token_type::INT, L"INT");
        ret->append_children(new ast::_int(type));

        token ident = this->lexer->next_token_with_skip();
        ident.assert(token_type::IDENT, L"IDENT");
        ret->append_children(new ast::_ident(ident));

        // 判断掉第一个 []
        token next = this->lexer->next_token_with_skip();
        if (next.get_type() == token_type::SQUARE_BRACKET_OPEN) {
            ret->append_children(new ast::_square_bracket_open(next));

            token first_close = this->lexer->next_token_with_skip();
            ret->append_children(new ast::_square_bracket_close(first_close));

            first_close.assert(token_type::SQUARE_BRACKET_CLOSE, L"]");
        } else {
            return ret;
        }

        // 判断掉多个 [ exp ]，最后以 ) 或 , 结束
        next = this->lexer->next_token_with_skip();
        while (next.get_type() != token_type::ROUND_BRACKET_CLOSE && next.get_type() != token_type::COMMA) {
            next.assert(token_type::SQUARE_BRACKET_OPEN, L"[");
            ret->append_children(new ast::_square_bracket_open(next));
            this->lexer->next_token_with_skip();

            auto exp = this->exp();
            ret->append_children(exp);

            token square_close = this->lexer->get_now_token();
            square_close.assert(token_type::SQUARE_BRACKET_CLOSE, L"]");
            ret->append_children(new ast::_square_bracket_close(square_close));

            next = this->lexer->next_token_with_skip();
        }

        return ret;
    }

    ast::block* block() {
        auto ret = new ast::block();

        token open = this->lexer->get_now_token();
        open.assert(token_type::PARENTHESES_OPEN, L"{");
        ret->append_children(new ast::_parentheses_open(open));
        this->lexer->next_token_with_skip();

        while (this->lexer->get_now_token().get_type() != token_type::PARENTHESES_CLOSE) {
            auto item = this->block_item();
            ret->append_children(item);
        }

        token close = this->lexer->get_now_token();
        ret->append_children(new ast::_parentheses_close(close));
        this->lexer->next_token_with_skip();

        return ret;
    }

    ast::block_item* block_item() {
        auto ret = new ast::block_item();

        if (this->lexer->get_now_token().get_type() == token_type::CONST ||
            this->lexer->get_now_token().get_type() == token_type::INT) {
            ret->append_children(this->decl());
        } else {
            ret->append_children(this->stmt());
        }

        return ret;
    }

    ast::decl* decl() {
        auto ret = new ast::decl();

        if (this->lexer->get_now_token().get_type() == token_type::CONST) {
            ret->append_children(this->const_decl());
        } else if (this->lexer->get_now_token().get_type() == token_type::INT) {
            ret->append_children(this->var_decl());
        } else {
            this->lexer->get_now_token().error(L"variables or constance declaration");
        }

        return ret;
    }

    ast::stmt* stmt() {
        auto ret = new ast::stmt();

        if (this->lexer->get_now_token().get_type() == token_type::IDENT) {
            // stat -> lVal '=' exp ';'
            // stat -> exp? ';'

            bool flag = true;
            auto first = this->exp(flag);

            if (flag && (this->lexer->get_now_token().get_type() != token_type::SEMICOLON)) {
                // TODO
                auto add_exp = (ast::add_exp*) (first->children[0]);
                auto mul_exp = (ast::mul_exp*) (add_exp->children[0]);
                auto unary_exp = (ast::unary_exp*) (mul_exp->children[0]);
                auto primary_exp = (ast::primary_exp*) (unary_exp->children[0]);
                auto lval = (ast::l_val*) (primary_exp->children[0]);

                free(primary_exp);
                free(unary_exp);
                free(mul_exp);
                free(add_exp);

                // stat -> lVal '=' exp ';'
                ret->append_children(lval);

                token assign = this->lexer->get_now_token();
                assign.assert(token_type::ASSIGNMENT_SYMBOL, L"=");
                ret->append_children(new ast::_assignment_symbol(assign));
                this->lexer->next_token_with_skip();

                auto exp = this->exp();
                ret->append_children(exp);
            } else {
                ret->append_children(first);
            }

            token end = this->lexer->get_now_token();
            end.assert(token_type::SEMICOLON, L";");
            ret->append_children(new ast::_semicolon(end));

            this->lexer->next_token_with_skip();
        } else if (
                this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                this->lexer->get_now_token().get_type() == token_type::ADD ||
                this->lexer->get_now_token().get_type() == token_type::SUB ||
                this->lexer->get_now_token().get_type() == token_type::NOT ||
                this->lexer->get_now_token().get_type() == token_type::INT_CONST
                ) {
            auto exp = this->exp();
            ret->append_children(exp);

            token end = this->lexer->get_now_token();
            end.assert(token_type::SEMICOLON, L";");
            ret->append_children(new ast::_semicolon(end));
            this->lexer->next_token_with_skip();
        } else if (this->lexer->get_now_token().get_type() == token_type::SEMICOLON) {
            ret->append_children(new ast::_semicolon(this->lexer->get_now_token()));
            this->lexer->next_token_with_skip();
        } else if (this->lexer->get_now_token().get_type() == token_type::PARENTHESES_OPEN) {
            auto block = this->block();
            ret->append_children(block);
        } else if (this->lexer->get_now_token().get_type() == token_type::IF) {
            ret->append_children(new ast::_if(this->lexer->get_now_token()));
            this->lexer->next_token_with_skip();

            this->lexer->get_now_token().assert(token_type::ROUND_BRACKET_OPEN, L"(");
            ret->append_children(new ast::_round_bracket_open(this->lexer->get_now_token()));
            this->lexer->next_token_with_skip();

            auto cond = this->cond();
            ret->append_children(cond);

            this->lexer->get_now_token().assert(token_type::ROUND_BRACKET_CLOSE, L")");
            ret->append_children(new ast::_round_bracket_close(this->lexer->get_now_token()));
            this->lexer->next_token_with_skip();

            auto stmt = this->stmt();
            ret->append_children(stmt);

            if (this->lexer->get_now_token().get_type() == token_type::ELSE) {
                ret->append_children(new ast::_else(this->lexer->get_now_token()));
                this->lexer->next_token_with_skip();

                auto else_stmt = this->stmt();
                ret->append_children(else_stmt);
            }
        } else if (this->lexer->get_now_token().get_type() == token_type::WHILE) {
            ret->append_children(new ast::_while(this->lexer->get_now_token()));
            this->lexer->next_token_with_skip();

            this->lexer->get_now_token().assert(token_type::ROUND_BRACKET_OPEN, L"(");
            ret->append_children(new ast::_round_bracket_open(this->lexer->get_now_token()));
            this->lexer->next_token_with_skip();

            auto cond = this->cond();
            ret->append_children(cond);

            this->lexer->get_now_token().assert(token_type::ROUND_BRACKET_CLOSE, L")");
            ret->append_children(new ast::_round_bracket_close(this->lexer->get_now_token()));
            this->lexer->next_token_with_skip();

            auto stmt = this->stmt();
            ret->append_children(stmt);
        } else if (this->lexer->get_now_token().get_type() == token_type::BREAK) {
            ret->append_children(new ast::_break(this->lexer->get_now_token()));
            this->lexer->next_token_with_skip();

            this->lexer->get_now_token().assert(token_type::SEMICOLON, L";");
            ret->append_children(new ast::_semicolon(this->lexer->get_now_token()));
            this->lexer->next_token_with_skip();
        } else if (this->lexer->get_now_token().get_type() == token_type::CONTINUE) {
            ret->append_children(new ast::_continue(this->lexer->get_now_token()));
            this->lexer->next_token_with_skip();

            this->lexer->get_now_token().assert(token_type::SEMICOLON, L";");
            ret->append_children(new ast::_semicolon(this->lexer->get_now_token()));
            this->lexer->next_token_with_skip();
        } else if (this->lexer->get_now_token().get_type() == token_type::RETURN) {
            ret->append_children(new ast::_return(this->lexer->get_now_token()));
            this->lexer->next_token_with_skip();

            if (this->lexer->get_now_token().get_type() != token_type::SEMICOLON) {
                auto exp = this->add_exp();
                ret->append_children(exp);
            }

            this->lexer->get_now_token().assert(token_type::SEMICOLON, L";");
            ret->append_children(new ast::_semicolon(this->lexer->get_now_token()));
            this->lexer->next_token_with_skip();
        }

        return ret;
    }

    ast::exp* exp() {
        bool tmp = true;
        return this->exp(tmp);
    }

    ast::exp* exp(bool &is_only_lval) {
        auto ret = new ast::exp();
        auto add_exp = this->add_exp(is_only_lval);
        ret->append_children(add_exp);
        return ret;
    }

    ast::add_exp* add_exp() {
        bool tmp = true;
        return this->add_exp(tmp);
    }

    ast::add_exp* add_exp(bool &is_only_lval) {
        auto ret = new ast::add_exp();

        token p1 = this->lexer->get_now_token();
        if (
                this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                this->lexer->get_now_token().get_type() == token_type::ADD ||
                this->lexer->get_now_token().get_type() == token_type::SUB ||
                this->lexer->get_now_token().get_type() == token_type::NOT ||
                this->lexer->get_now_token().get_type() == token_type::INT_CONST ||
                this->lexer->get_now_token().get_type() == token_type::IDENT
                ) {
            auto mul_exp = this->mul_exp(is_only_lval);
            ret->append_children(mul_exp);
        }
        token p2 = this->lexer->get_now_token();
        if (p1.get_position() == p2.get_position()) {
            throw token_unexpected_exception(p1.get_line(), p1.get_column(), L"(, +, -, !, NUMBER, IDENT", token_type_get_name(p1.get_type()));
        }

        bool expend_flag = false;

        while (true) {
            token op = this->lexer->get_now_token();
            if (op.get_type() != token_type::ADD && op.get_type() != token_type::SUB) {
                break;
            }

            if (expend_flag) {
                auto new_node = new ast::add_exp();
                new_node->append_children(ret);
                ret = new_node;
            }

            if (op.get_type() == token_type::ADD) {
                ret->append_children(new ast::_add(op));
            } else if (op.get_type() == token_type::SUB) {
                ret->append_children(new ast::_sub(op));
            } else {
                break;
            }

            p1 = this->lexer->next_token_with_skip();
            if (
                    this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                    this->lexer->get_now_token().get_type() == token_type::ADD ||
                    this->lexer->get_now_token().get_type() == token_type::SUB ||
                    this->lexer->get_now_token().get_type() == token_type::NOT ||
                    this->lexer->get_now_token().get_type() == token_type::INT_CONST ||
                    this->lexer->get_now_token().get_type() == token_type::IDENT
                    ) {
                auto mul_exp = this->mul_exp(is_only_lval);
                ret->append_children(mul_exp);
            }
            p2 = this->lexer->get_now_token();
            if (p1.get_position() == p2.get_position()) {
                throw token_unexpected_exception(p1.get_line(), p1.get_column(), L"(, +, -, !, NUMBER, IDENT", token_type_get_name(p1.get_type()));
            }

            expend_flag = true;
        }

        return ret;
    }

    ast::mul_exp* mul_exp(bool &is_only_lval) {
        auto ret = new ast::mul_exp();

        token p1 = this->lexer->get_now_token();
        if (
                this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                this->lexer->get_now_token().get_type() == token_type::ADD ||
                this->lexer->get_now_token().get_type() == token_type::SUB ||
                this->lexer->get_now_token().get_type() == token_type::NOT ||
                this->lexer->get_now_token().get_type() == token_type::INT_CONST ||
                this->lexer->get_now_token().get_type() == token_type::IDENT
                ) {
            auto unary_exp = this->unary_exp(is_only_lval);
            ret->append_children(unary_exp);
        }
        token p2 = this->lexer->get_now_token();
        if (p1.get_position() == p2.get_position()) {
            throw token_unexpected_exception(p1.get_line(), p1.get_column(), L"(, +, -, !, NUMBER, IDENT", token_type_get_name(p1.get_type()));
        }

        bool expend_flag = false;

        while (true) {
            token op = this->lexer->get_now_token();
            if (
                    op.get_type() != token_type::MUL && op.get_type() != token_type::DIV &&
                    op.get_type() != token_type::MOD
                    ) {
                break;
            }

            if (expend_flag) {
                auto new_node = new ast::mul_exp();
                new_node->append_children(ret);
                ret = new_node;
            }

            if (op.get_type() == token_type::MUL) {
                ret->append_children(new ast::_mul(op));
            } else if (op.get_type() == token_type::DIV) {
                ret->append_children(new ast::_div(op));
            } else if (op.get_type() == token_type::MOD) {
                ret->append_children(new ast::_mod(op));
            } else {
                break;
            }

            p1 = this->lexer->next_token_with_skip();
            if (
                    this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                    this->lexer->get_now_token().get_type() == token_type::ADD ||
                    this->lexer->get_now_token().get_type() == token_type::SUB ||
                    this->lexer->get_now_token().get_type() == token_type::NOT ||
                    this->lexer->get_now_token().get_type() == token_type::INT_CONST ||
                    this->lexer->get_now_token().get_type() == token_type::IDENT
                    ) {
                auto unary_exp = this->unary_exp(is_only_lval);
                ret->append_children(unary_exp);
            }
            p2 = this->lexer->get_now_token();
            if (p1.get_position() == p2.get_position()) {
                throw token_unexpected_exception(p1.get_line(), p1.get_column(), L"(, +, -, !, NUMBER, IDENT", token_type_get_name(p1.get_type()));
            }

            expend_flag = true;
        }

        return ret;
    }

    ast::unary_exp* unary_exp(bool &is_only_lval) {
        auto ret = new ast::unary_exp();

        if (
                this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                this->lexer->get_now_token().get_type() == token_type::INT_CONST
                ) {
            auto primary_exp = this->primary_exp(is_only_lval);
            ret->append_children(primary_exp);
        } else if (this->lexer->get_now_token().get_type() == token_type::IDENT) {
            token now = this->lexer->get_now_token();
            token next =  this->lexer->next_token_with_skip();

            if (next.get_type() == token_type::ROUND_BRACKET_OPEN) {
                is_only_lval = false;

                ret->append_children(new ast::_ident(now));
                ret->append_children(new ast::_round_bracket_open(next));

                token close = this->lexer->next_token_with_skip();
                if (close.get_type() != token_type::ROUND_BRACKET_CLOSE) {
                    this->func_rparams();
                }
                close = this->lexer->get_now_token();
                ret->append_children(new ast::_round_bracket_close(close));
                this->lexer->next_token_with_skip();
            } else {
                auto primary_exp = this->primary_exp$2(is_only_lval, now);
                ret->append_children(primary_exp);
            }
        } else if (this->lexer->get_now_token().get_type() == token_type::ADD ||
                   this->lexer->get_now_token().get_type() == token_type::SUB ||
                   this->lexer->get_now_token().get_type() == token_type::NOT) {
            is_only_lval = false;

            token op = this->lexer->get_now_token();
            if (op.get_type() == token_type::ADD) {
                ret->append_children(new ast::_add(op));
            } else if (op.get_type() == token_type::NOT) {
                ret->append_children(new ast::_sub(op));
            } else if (op.get_type() == token_type::NOT) {
                ret->append_children(new ast::_not(op));
            }
            this->lexer->next_token_with_skip();

            auto unary_exp = this->unary_exp(is_only_lval);
            ret->append_children(unary_exp);
        }

        return ret;
    }

    ast::func_rparams* func_rparams() {
        auto ret = new ast::func_rparams();

        while (this->lexer->get_now_token().get_type() != token_type::ROUND_BRACKET_CLOSE) {
            auto exp = this->exp();
            ret->append_children(exp);

            token split = this->lexer->get_now_token();
            if (split.get_type() == token_type::ROUND_BRACKET_CLOSE) {
                break;
            }

            split.assert(token_type::COMMA, L",");
            ret->append_children(new ast::_comma(split));
            this->lexer->next_token_with_skip();
        }

        return ret;
    }

    ast::primary_exp* primary_exp(bool &is_only_lval) {
        auto ret = new ast::primary_exp();

        if (this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN) {
            is_only_lval = false;

            token first = this->lexer->get_now_token();
            ret->append_children(new ast::_round_bracket_open(first));
            this->lexer->next_token_with_skip();

            auto exp = this->exp();
            ret->append_children(exp);

            token third = this->lexer->get_now_token();
            third.assert(token_type::ROUND_BRACKET_CLOSE, L")");
            ret->append_children(new ast::_round_bracket_close(third));
            this->lexer->next_token_with_skip();

        } else if (this->lexer->get_now_token().get_type() == token_type::IDENT) {
            is_only_lval = is_only_lval & true;

            auto lval = this->l_val();
            ret->append_children(lval);
        } else if (this->lexer->get_now_token().get_type() == token_type::INT_CONST) {
            is_only_lval = false;

            auto number = this->number();
            ret->append_children(number);
        }

        return ret;
    }

    ast::primary_exp* primary_exp$2(bool &is_only_lval, const token &now) {
        auto ret = new ast::primary_exp();

        if (now.get_type() == token_type::ROUND_BRACKET_OPEN) {
            is_only_lval = false;

            const token& first = now;
            ret->append_children(new ast::_round_bracket_open(first));

            auto exp = this->exp();
            ret->append_children(exp);

            token third = this->lexer->get_now_token();
            third.assert(token_type::ROUND_BRACKET_CLOSE, L")");
            ret->append_children(new ast::_round_bracket_close(third));
        } else if (now.get_type() == token_type::IDENT) {
            is_only_lval = is_only_lval & true;

            auto lval = this->l_val$2(now);
            ret->append_children(lval);
        } else if (now.get_type() == token_type::INT_CONST) {
            is_only_lval = false;

            auto number = this->number$2(now);
            ret->append_children(number);
        }

        return ret;
    }

    ast::l_val* l_val() {
        token now = this->lexer->get_now_token();
        this->lexer->next_token_with_skip();
        return this->l_val$2(now);
    }

    ast::l_val* l_val$2(const token &ident) {
        auto ret = new ast::l_val();

        ret->append_children(new ast::_ident(ident));

        while (this->lexer->get_now_token().get_type() == token_type::SQUARE_BRACKET_OPEN) {
            token open = this->lexer->get_now_token();
            ret->append_children(new ast::_square_bracket_open(open));
            this->lexer->next_token_with_skip();

            auto exp = this->exp();
            ret->append_children(exp);

            token close = this->lexer->get_now_token();
            close.assert(token_type::SQUARE_BRACKET_CLOSE, L"]");
            ret->append_children(new ast::_square_bracket_close(close));
            this->lexer->next_token_with_skip();
        }

        return ret;
    }

    ast::cond* cond() {
        auto ret = new ast::cond();
        auto l_or_exp = this->l_or_exp();
        ret->append_children(l_or_exp);
        return ret;
    }

    ast::l_or_exp* l_or_exp() {
        auto ret = new ast::l_or_exp();

        token p1 = this->lexer->get_now_token();
        if (
                this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                this->lexer->get_now_token().get_type() == token_type::ADD ||
                this->lexer->get_now_token().get_type() == token_type::SUB ||
                this->lexer->get_now_token().get_type() == token_type::NOT ||
                this->lexer->get_now_token().get_type() == token_type::INT_CONST ||
                this->lexer->get_now_token().get_type() == token_type::IDENT
                ) {
            auto l_and_exp = this->l_and_exp();
            ret->append_children(l_and_exp);
        }
        token p2 = this->lexer->get_now_token();
        if (p1.get_position() == p2.get_position()) {
            throw token_unexpected_exception(p1.get_line(), p1.get_column(), L"(, +, -, !, NUMBER, IDENT", token_type_get_name(p1.get_type()));
        }

        bool expend_flag = false;

        while (true) {
            token op = this->lexer->get_now_token();
            if (op.get_type() != token_type::ADD && op.get_type() != token_type::OR) {
                break;
            }

            if (expend_flag) {
                auto new_node = new ast::l_or_exp();
                new_node->append_children(ret);
                ret = new_node;
            }

            ret->append_children(new ast::_or(op));

            p1 = this->lexer->next_token_with_skip();
            if (
                    this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                    this->lexer->get_now_token().get_type() == token_type::ADD ||
                    this->lexer->get_now_token().get_type() == token_type::SUB ||
                    this->lexer->get_now_token().get_type() == token_type::NOT ||
                    this->lexer->get_now_token().get_type() == token_type::INT_CONST ||
                    this->lexer->get_now_token().get_type() == token_type::IDENT
                    ) {
                auto l_and_exp = this->l_and_exp();
                ret->append_children(l_and_exp);
            }
            p2 = this->lexer->get_now_token();
            if (p1.get_position() == p2.get_position()) {
                throw token_unexpected_exception(p1.get_line(), p1.get_column(), L"(, +, -, !, NUMBER, IDENT", token_type_get_name(p1.get_type()));
            }

            expend_flag = true;
        }

        return ret;
    }

    ast::l_and_exp* l_and_exp() {
        auto ret = new ast::l_and_exp();

        token p1 = this->lexer->get_now_token();
        if (
                this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                this->lexer->get_now_token().get_type() == token_type::ADD ||
                this->lexer->get_now_token().get_type() == token_type::SUB ||
                this->lexer->get_now_token().get_type() == token_type::NOT ||
                this->lexer->get_now_token().get_type() == token_type::INT_CONST ||
                this->lexer->get_now_token().get_type() == token_type::IDENT
                ) {
            auto eq_exp = this->eq_exp();
            ret->append_children(eq_exp);
        }
        token p2 = this->lexer->get_now_token();
        if (p1.get_position() == p2.get_position()) {
            throw token_unexpected_exception(p1.get_line(), p1.get_column(), L"(, +, -, !, NUMBER, IDENT", token_type_get_name(p1.get_type()));
        }

        bool expend_flag = false;

        while (true) {
            token op = this->lexer->get_now_token();
            if (op.get_type() != token_type::ADD && op.get_type() != token_type::AND) {
                break;
            }

            if (expend_flag) {
                auto new_node = new ast::l_and_exp();
                new_node->append_children(ret);
                ret = new_node;
            }

            ret->append_children(new ast::_and(op));

            p1 = this->lexer->next_token_with_skip();
            if (
                    this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                    this->lexer->get_now_token().get_type() == token_type::ADD ||
                    this->lexer->get_now_token().get_type() == token_type::SUB ||
                    this->lexer->get_now_token().get_type() == token_type::NOT ||
                    this->lexer->get_now_token().get_type() == token_type::INT_CONST ||
                    this->lexer->get_now_token().get_type() == token_type::IDENT
                    ) {
                auto eq_exp = this->eq_exp();
                ret->append_children(eq_exp);
            }
            p2 = this->lexer->get_now_token();
            if (p1.get_position() == p2.get_position()) {
                throw token_unexpected_exception(p1.get_line(), p1.get_column(), L"(, +, -, !, NUMBER, IDENT", token_type_get_name(p1.get_type()));
            }

            expend_flag = true;
        }

        return ret;
    }

    ast::eq_exp* eq_exp() {
        auto ret = new ast::eq_exp();

        token p1 = this->lexer->get_now_token();
        if (
                this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                this->lexer->get_now_token().get_type() == token_type::ADD ||
                this->lexer->get_now_token().get_type() == token_type::SUB ||
                this->lexer->get_now_token().get_type() == token_type::NOT ||
                this->lexer->get_now_token().get_type() == token_type::INT_CONST ||
                this->lexer->get_now_token().get_type() == token_type::IDENT
                ) {
            auto rel_exp = this->rel_exp();
            ret->append_children(rel_exp);
        }
        token p2 = this->lexer->get_now_token();
        if (p1.get_position() == p2.get_position()) {
            throw token_unexpected_exception(p1.get_line(), p1.get_column(), L"(, +, -, !, NUMBER, IDENT", token_type_get_name(p1.get_type()));
        }

        bool expend_flag = false;

        while (true) {
            token op = this->lexer->get_now_token();
            if (op.get_type() != token_type::EQUAL && op.get_type() != token_type::NOTEQUAL) {
                break;
            }

            if (expend_flag) {
                auto new_node = new ast::eq_exp();
                new_node->append_children(ret);
                ret = new_node;
            }

            if (op.get_type() == token_type::EQUAL) {
                ret->append_children(new ast::_equal(op));
            } else if (op.get_type() == token_type::NOTEQUAL) {
                ret->append_children(new ast::_notequal(op));
            } else {
                break;
            }

            p1 = this->lexer->next_token_with_skip();
            if (
                    this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                    this->lexer->get_now_token().get_type() == token_type::ADD ||
                    this->lexer->get_now_token().get_type() == token_type::SUB ||
                    this->lexer->get_now_token().get_type() == token_type::NOT ||
                    this->lexer->get_now_token().get_type() == token_type::INT_CONST ||
                    this->lexer->get_now_token().get_type() == token_type::IDENT
                    ) {
                auto rel_exp = this->rel_exp();
                ret->append_children(rel_exp);
            }
            p2 = this->lexer->get_now_token();
            if (p1.get_position() == p2.get_position()) {
                throw token_unexpected_exception(p1.get_line(), p1.get_column(), L"(, +, -, !, NUMBER, IDENT", token_type_get_name(p1.get_type()));
            }

            expend_flag = true;
        }

        return ret;
    }

    ast::node* rel_exp() {
        auto ret = new ast::rel_exp();

        token p1 = this->lexer->get_now_token();
        if (
                this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                this->lexer->get_now_token().get_type() == token_type::ADD ||
                this->lexer->get_now_token().get_type() == token_type::SUB ||
                this->lexer->get_now_token().get_type() == token_type::NOT ||
                this->lexer->get_now_token().get_type() == token_type::INT_CONST ||
                this->lexer->get_now_token().get_type() == token_type::IDENT
                ) {
            auto add_exp = this->add_exp();
            ret->append_children(add_exp);
        }
        token p2 = this->lexer->get_now_token();
        if (p1.get_position() == p2.get_position()) {
            throw token_unexpected_exception(p1.get_line(), p1.get_column(), L"(, +, -, !, NUMBER, IDENT", token_type_get_name(p1.get_type()));
        }

        bool expend_flag = false;

        while (true) {
            token op = this->lexer->get_now_token();
            if (
                    op.get_type() != token_type::ADD && op.get_type() != token_type::LESS &&
                    op.get_type() != token_type::ADD && op.get_type() != token_type::LESS_OR_EQUAL &&
                    op.get_type() != token_type::ADD && op.get_type() != token_type::GREATER_OR_EQUAL &&
                    op.get_type() != token_type::ADD && op.get_type() != token_type::GREATER
                ) {
                break;
            }

            if (expend_flag) {
                auto new_node = new ast::rel_exp();
                new_node->append_children(ret);
                ret = new_node;
            }

            if (op.get_type() == token_type::LESS) {
                ret->append_children(new ast::_less(op));
            } else if (op.get_type() == token_type::LESS_OR_EQUAL) {
                ret->append_children(new ast::_less_or_equal(op));
            } else if (op.get_type() == token_type::GREATER_OR_EQUAL) {
                ret->append_children(new ast::_greater_or_equal(op));
            } else if (op.get_type() == token_type::GREATER) {
                ret->append_children(new ast::_greater(op));
            } else {
                break;
            }

            p1 = this->lexer->next_token_with_skip();
            if (
                    this->lexer->get_now_token().get_type() == token_type::ROUND_BRACKET_OPEN ||
                    this->lexer->get_now_token().get_type() == token_type::ADD ||
                    this->lexer->get_now_token().get_type() == token_type::SUB ||
                    this->lexer->get_now_token().get_type() == token_type::NOT ||
                    this->lexer->get_now_token().get_type() == token_type::INT_CONST ||
                    this->lexer->get_now_token().get_type() == token_type::IDENT
                    ) {
                auto add_exp = this->add_exp();
                ret->append_children(add_exp);
            }
            p2 = this->lexer->get_now_token();
            if (p1.get_position() == p2.get_position()) {
                throw token_unexpected_exception(p1.get_line(), p1.get_column(), L"(, +, -, !, NUMBER, IDENT", token_type_get_name(p1.get_type()));
            }

            expend_flag = true;
        }

        return ret;
    }

    ast::number* number() {
        auto ret = new ast::number();

        token number = this->lexer->get_now_token();
        number.assert(token_type::INT_CONST, L"INT_CONST");
        this->lexer->next_token_with_skip();

        ret->append_children(new ast::_int_const(number));

        return ret;
    }

    ast::number* number$2(const token &number) {
        auto ret = new ast::number();

        number.assert(token_type::INT_CONST, L"INT_CONST");
        this->lexer->next_token_with_skip();

        ret->append_children(new ast::_int_const(number));

        return ret;
    }

private:
    lexer *lexer;
};


#endif //CYAN_PARSER_HPP
