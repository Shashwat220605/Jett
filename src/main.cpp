#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "type_checker.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace
{
    constexpr const char* JETT_VERSION =
        "Jett 1.0.0";

    void printHelp()
    {
        std::cout
            << "Jett - lightweight interpreted programming language\n\n"

            << "Usage:\n"
            << "  jett <file.jett>\n"
            << "  jett --help\n"
            << "  jett --version\n\n"

            << "Options:\n"
            << "  -h, --help       Show this help message\n"
            << "  -v, --version    Show Jett version\n\n"

            << "Examples:\n"
            << "  jett examples/hello.jett\n"
            << "  jett --version\n";
    }

    bool readFile(
        const std::string& path,
        std::string& source
    )
    {
        std::ifstream file(
            path,
            std::ios::in
        );

        if (!file.is_open())
        {
            return false;
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();

        source =
            buffer.str();

        return true;
    }
}

int main(
    int argc,
    char* argv[]
)
{
    // ========================================================
    // No arguments
    // ========================================================

    if (argc == 1)
    {
        printHelp();
        return 0;
    }

    const std::string argument =
        argv[1];

    // ========================================================
    // Help
    // ========================================================

    if (argument == "--help" ||
        argument == "-h")
    {
        printHelp();
        return 0;
    }

    // ========================================================
    // Version
    // ========================================================

    if (argument == "--version" ||
        argument == "-v")
    {
        std::cout
            << JETT_VERSION
            << '\n';

        return 0;
    }

    // ========================================================
    // Too many arguments
    // ========================================================

    if (argc > 2)
    {
        std::cerr
            << "Jett Error: too many arguments.\n\n";

        std::cerr
            << "Usage: jett <file.jett>\n";

        return 1;
    }

    // ========================================================
    // Read source file
    // ========================================================

    const std::string filePath =
        argument;

    std::string source;

    if (!readFile(
            filePath,
            source))
    {
        std::cerr
            << "Jett Error: could not open file '"
            << filePath
            << "'.\n";

        return 1;
    }

    // ========================================================
    // Lexer
    // ========================================================

    Lexer lexer(
        source
    );

    std::vector<Token> tokens =
        lexer.tokenize();

    // ========================================================
    // Parser
    // ========================================================

    Parser parser(
        tokens
    );

    std::unique_ptr<Program> program =
        parser.parse();

    // ========================================================
    // Type checker
    // ========================================================

    TypeChecker checker;

    checker.check(
        *program
    );

    if (checker.hasErrors())
    {
        for (const auto& error :
             checker.getErrors())
        {
            std::cerr
                << error
                << '\n';
        }

        return 1;
    }

    // ========================================================
    // Interpreter
    // ========================================================

    Interpreter interpreter;

    interpreter.execute(
        *program
    );

    return 0;
}