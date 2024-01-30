#include "iostream"
#include "stdexcept"

#include "parser.hpp"
#include "vm.hpp"

void run_terminal()
{
    std::cout << "Interactive terminal" << std::endl;
    // TODO
}

std::string read_file(std::string file_path)
{
    // TODO
    return "";
}

int main(int argc, char const *argv[])
{
    if (argc == 1) run_terminal();
    else if (argc == 2 && argv[1] == "c") std::cout << "COMPILING...\n"; //pvm::compile(parser::parse(read_file(argv[2])));
    else if (argc == 2 && argv[1] == "r") std::cout << "RUNNING...\n"; //pvm::run(pvm::read_file(argv[2]));
    else
    {
        std::cout << "Wrong arguments\n";
        return 1;
    }
}
