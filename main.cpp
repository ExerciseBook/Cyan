#include <iostream>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "visitor/sysy_visitor.hpp"

int main() {

    std::wifstream myfile;
    myfile.open (R"(F:\File\src\AntlrTest\src\main\java\SysY\1.sysy)");

    lexer _lexer = lexer(myfile);
//    token t = _lexer.next_token();
//    while (t.get_type() != token_type::END_OF_FILE) {
//
//        if (t.get_type() != token_type::DEFAULT)
//            std::wcout << t.get_type_name() << " " << t.get_line() << " " << t.get_column() << " " << t.get_text() << std::endl;
//
//        t = _lexer.next_token();
//    }

    sysy_visitor visitor;

    parser _parser = parser(&_lexer);
    _parser.comp_unit();

    myfile.close();
    return 0;
}
