#include<filesystem>
#include <iostream>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "visitor/sysy_visitor.hpp"

void run_test(const std::filesystem::path& case_path) {
    std::wcout << "\u001b[0m" << L"Now running case " << case_path.native() << std::endl;

    std::wifstream myfile;
    myfile.open(case_path);

    lexer _lexer = lexer(myfile);
    sysy_visitor visitor;
    try {
        auto _parser = parser(&_lexer);
        _parser.comp_unit();
    }
    catch (token_unexpected_exception &e) {
        std::wcout << "\u001b[31m" << e.get_message() << std::endl;
    }
    myfile.close();
}

int run_test_suite(const std::filesystem::path& test_cases_path) {
    if (!std::filesystem::exists(test_cases_path)) {
        return -1;
    }

    std::filesystem::directory_iterator entry(test_cases_path);
    for (auto &i : entry) {
        if (i.is_regular_file()) {
            run_test(i);
        }
    }

}

int main(int argc, char *argv[]) {

    run_test_suite("../cases/section1/functional_test");
    run_test_suite("../cases/section1/performance_test");
    run_test_suite("../cases/section2/performance_test");

    return 0;
}
