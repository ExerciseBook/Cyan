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
        node() {

        }

        node(std::wstring name) : name(name) {

        }

        virtual std::wstring text() {
            return this->name;
        }

        virtual void print_tree(int padding = 0) {
        }

        std::wstring get_name() {
            return this->name;
        }

        virtual std::wstring to_json() {
            return L"";
        }

    protected:
        std::wstring name{};

        std::wstring get_padding(int depth) {
            std::wostringstream stringStream;
            for (int i = 0; i < depth; i++) {
                stringStream << "    ";
            }
            return stringStream.str();
        }
    };

    class non_terminal : public node {
    public:
        non_terminal(): node() {

        }

        non_terminal(std::wstring name) : node(name) {

        }

        std::vector<node*> children{};

        void append_children(node* t) {
            children.push_back(t);
        }

        void print_tree(int padding = 0) override {
            std::wcout << get_padding(padding) << get_name() << std::endl;

            for (auto i : children) {
                i->print_tree(padding + 1);
            }
        };

        std::wstring to_json() override {
            std::wostringstream ret;

            ret << L"{ \"" << get_name() << "\" : [";

            auto size = children.size();
            for (int i = 0; i < size - 1; i++) {
                ret << children[i]->to_json();
                ret << ",";
            }

            ret << children[size - 1]->to_json();

            ret << L"] }";

            return ret.str();
        };

    };

    class terminal : public node {
    public:
        terminal(const token& _token): node(token_type_get_name(_token.get_type())) {
            this->_token = _token;
        }

        std::wstring text() override {
            return this->_token.get_text();
        }

        void print_tree(int padding = 0) override {
            std::wcout << get_padding(padding) << text() << std::endl;
        };

        std::wstring to_json() override {
            std::wostringstream ret;

            ret << L"{ \"" << get_name() << "\" : {";

            ret << "\"text\" : \"" << json_text() << "\", ";
            ret << "\"line\" : " << _token.get_line() << ",";
            ret << "\"column\" : " << _token.get_column() << ",";
            ret << "\"position\" : " << _token.get_position();

            ret << L"} }";

            return ret.str();
        }
    private:
        std::wstring json_text() {
            std::wostringstream ret;
            for (auto i : text()) {
                if (i == L'"' ||
                    i == L'\\' ||
                    i == L'/' ||
                    i == L'\b' ||
                    i == L'\f' ||
                    i == L'\n' ||
                    i == L'\r' ||
                    i == L'\t'
                ) {
                    ret << "\\";
                }

                ret << i;
            }
            return ret.str();
        }

    protected:
        token _token;

    };

    class comp_unit : public non_terminal {
    public:
        comp_unit() : non_terminal(L"comp_unit") {

        }
    };

    class decl : public non_terminal {
    public:
        decl() : non_terminal(L"decl") {}

    };

    class const_decl : public non_terminal {
    public:
        const_decl() : non_terminal(L"const_decl") {}

    };

    class b_type : public non_terminal {
    public:
        b_type() : non_terminal(L"b_type") {}

    };

    class const_def : public non_terminal {
    public:
        const_def() : non_terminal(L"const_def") {}

    };

    class const_init_val : public non_terminal {
    public:
        const_init_val() : non_terminal(L"const_init_val") {}

    };

    class var_decl : public non_terminal {
    public:
        var_decl() : non_terminal(L"var_decl") {}

    };

    class var_def : public non_terminal {
    public:
        var_def() : non_terminal(L"var_def") {}

    };

    class init_val : public non_terminal {
    public:
        init_val() : non_terminal(L"init_val") {}

    };

    class func_def : public non_terminal {
    public:
        func_def() : non_terminal(L"func_def") {}

    };

    class func_type : public non_terminal {
    public:
        func_type() : non_terminal(L"func_type") {}

    };

    class func_fparams : public non_terminal {
    public:
        func_fparams() : non_terminal(L"func_fparams") {}

    };

    class func_fparam : public non_terminal {
    public:
        func_fparam() : non_terminal(L"func_fparam") {}

    };

    class block : public non_terminal {
    public:
        block() : non_terminal(L"block") {}

    };

    class block_item : public non_terminal {
    public:
        block_item() : non_terminal(L"block_item") {}

    };

    class stmt : public non_terminal {
    public:
        stmt() : non_terminal(L"stmt") {}

    };

    class exp : public non_terminal {
    public:
        exp() : non_terminal(L"exp") {}

    };

    class cond : public non_terminal {
    public:
        cond() : non_terminal(L"cond") {}

    };

    class l_val : public non_terminal {
    public:
        l_val() : non_terminal(L"l_val") {}

    };

    class primary_exp : public non_terminal {
    public:
        primary_exp() : non_terminal(L"primary_exp") {}

    };

    class number : public non_terminal {
    public:
        number() : non_terminal(L"number") {}

    };

    class unary_exp : public non_terminal {
    public:
        unary_exp() : non_terminal(L"unary_exp") {}

    };

    class func_rparams : public non_terminal {
    public:
        func_rparams() : non_terminal(L"func_rparams") {}

    };

    class mul_exp : public non_terminal {
    public:
        mul_exp() : non_terminal(L"mul_exp") {}

    };

    class add_exp : public non_terminal {
    public:
        add_exp() : non_terminal(L"add_exp") {}

    };

    class rel_exp : public non_terminal {
    public:
        rel_exp() : non_terminal(L"rel_exp") {}

    };

    class eq_exp : public non_terminal {
    public:
        eq_exp() : non_terminal(L"eq_exp") {}

    };

    class l_and_exp : public non_terminal {
    public:
        l_and_exp() : non_terminal(L"l_and_exp") {}

    };

    class l_or_exp : public non_terminal {
    public:
        l_or_exp() : non_terminal(L"l_or_exp") {}

    };

    class const_exp : public non_terminal {
    public:
        const_exp() : non_terminal(L"const_exp") {}

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
