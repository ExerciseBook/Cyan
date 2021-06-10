//
// Created by Eric on 2021-06-10 0010.
//

#ifndef CYAN_NODE_HPP
#define CYAN_NODE_HPP

#include <utility>
#include <vector>
#include "../lexer/token.hpp"

namespace ast {
    class node {
    public:
        virtual std::wstring text() {
            return L"";
        }
    };

    class non_terminal : public node {
    public:
        std::vector<node*> children{};

        void append_children(node* t) {
            children.push_back(t);
        }
    };

    class terminal : public node {
    private:
        token _token;
    public:
        terminal(const token& _token): node() {
            this->_token = _token;
        }

        std::wstring text() override {
            return this->_token.get_text();
        }
    };

    class comp_unit : public non_terminal {

    };

    class const_decl : public non_terminal {

    };

    class b_type : public non_terminal {

    };

    class const_def : public non_terminal {

    };

    class const_init_val : public non_terminal {

    };

    class var_decl : public non_terminal {

    };

    class var_def : public non_terminal {

    };

    class init_val : public non_terminal {

    };

    class func_def : public non_terminal {

    };

    class func_type : public non_terminal {

    };

    class func_fparams : public non_terminal {

    };

    class func_fparam : public non_terminal {

    };

    class block : public non_terminal {

    };

    class block_item : public non_terminal {

    };

    class stmt : public non_terminal {

    };

    class exp : public non_terminal {

    };

    class cond : public non_terminal {

    };

    class l_val : public non_terminal {

    };

    class primary_exp : public non_terminal {

    };

    class number : public non_terminal {

    };

    class unary_exp : public non_terminal {

    };

    class func_rparams : public non_terminal {

    };

    class mul_exp : public non_terminal {

    };

    class add_exp : public non_terminal {

    };

    class add_exp_p : public non_terminal {

    };

    class rel_exp : public non_terminal {

    };

    class eq_exp : public non_terminal {

    };

    class l_and_exp : public non_terminal {

    };

    class l_or_exp : public non_terminal {

    };

    class const_exp : public non_terminal {

    };

    class _const : public terminal {
    public:
        explicit _const(const token& token) : terminal(token) {}
    };

    class _comma : public terminal {
    public:
        explicit _comma(const token& token) : terminal(token) {}
    };

    class _semicolon : public terminal {
    public:
        explicit _semicolon(const token& token) : terminal(token) {}
    };

    class _square_bracket_open : public terminal {
    public:
        explicit _square_bracket_open(const token& token) : terminal(token) {}
    };

    class _square_bracket_close : public terminal {
    public:
        explicit _square_bracket_close(const token& token) : terminal(token) {}
    };

    class _assignment_symbol : public terminal {
    public:
        explicit _assignment_symbol(const token& token) : terminal(token) {}
    };

    class _parentheses_open : public terminal {
    public:
        explicit _parentheses_open(const token& token) : terminal(token) {}
    };

    class _parentheses_close : public terminal {
    public:
        explicit _parentheses_close(const token& token) : terminal(token) {}
    };

    class _round_bracket_open : public terminal {
    public:
        explicit _round_bracket_open(const token& token) : terminal(token) {}
    };

    class _round_bracket_close : public terminal {
    public:
        explicit _round_bracket_close(const token& token) : terminal(token) {}
    };

    class _if : public terminal {
    public:
        explicit _if(const token& token) : terminal(token) {}
    };

    class _else : public terminal {
    public:
        explicit _else(const token& token) : terminal(token) {}
    };

    class _while : public terminal {
    public:
        explicit _while(const token& token) : terminal(token) {}
    };

    class _break : public terminal {
    public:
        explicit _break(const token& token) : terminal(token) {}
    };

    class _continue : public terminal {
    public:
        explicit _continue(const token& token) : terminal(token) {}
    };

    class _return : public terminal {
    public:
        explicit _return(const token& token) : terminal(token) {}
    };

    class _int : public terminal {
    public:
        explicit _int(const token& token) : terminal(token) {}
    };

    class _void : public terminal {
    public:
        explicit _void(const token& token) : terminal(token) {}
    };

    class _mul : public terminal {
    public:
        explicit _mul(const token& token) : terminal(token) {}
    };

    class _div : public terminal {
    public:
        explicit _div(const token& token) : terminal(token) {}
    };

    class _add : public terminal {
    public:
        explicit _add(const token& token) : terminal(token) {}
    };

    class _sub : public terminal {
    public:
        explicit _sub(const token& token) : terminal(token) {}
    };

    class _mod : public terminal {
    public:
        explicit _mod(const token& token) : terminal(token) {}
    };

    class _less : public terminal {
    public:
        explicit _less(const token& token) : terminal(token) {}
    };

    class _less_or_equal : public terminal {
    public:
        explicit _less_or_equal(const token& token) : terminal(token) {}
    };

    class _equal : public terminal {
    public:
        explicit _equal(const token& token) : terminal(token) {}
    };

    class _greater_or_equal : public terminal {
    public:
        explicit _greater_or_equal(const token& token) : terminal(token) {}
    };

    class _greater : public terminal {
    public:
        explicit _greater(const token& token) : terminal(token) {}
    };

    class _notequal : public terminal {
    public:
        explicit _notequal(const token& token) : terminal(token) {}
    };

    class _not : public terminal {
    public:
        explicit _not(const token& token) : terminal(token) {}
    };

    class _and : public terminal {
    public:
        explicit _and(const token& token) : terminal(token) {}
    };

    class _or : public terminal {
    public:
        explicit _or(const token& token) : terminal(token) {}
    };

    class _int_const : public terminal {
    public:
        explicit _int_const(const token& token) : terminal(token) {}
    };

    class _ident : public terminal {
    public:
        explicit _ident(const token& token) : terminal(token) {}
    };

}

#endif //CYAN_NODE_HPP
