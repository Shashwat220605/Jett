#include "ast_printer.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "type_checker.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    constexpr const char* JETT_VERSION = "Jett 1.1.0";

    void printHelp()
    {
        std::cout
            << "Jett - lightweight interpreted programming language\n\n"
            << "Usage:\n"
            << "  jett <file.jett>\n"
            << "  jett --check <file.jett>\n"
            << "  jett --tokens <file.jett>\n"
            << "  jett --ast <file.jett>\n"
            << "  jett --help\n"
            << "  jett --version\n\n"
            << "Options:\n"
            << "  -h, --help       Show this help message\n"
            << "  -v, --version    Show Jett version\n"
            << "  --check          Validate a source file without executing it\n"
            << "  --tokens         Print lexer tokens for a source file\n"
            << "  --ast            Print the parsed AST for a source file\n\n"
            << "Examples:\n"
            << "  jett examples/hello.jett\n"
            << "  jett --check examples/functions.jett\n"
            << "  jett --tokens examples/hello.jett\n"
            << "  jett --ast examples/functions.jett\n"
            << "  jett --version\n";
    }

    bool readFile(const std::string& path, std::string& source)
    {
        std::ifstream file(path, std::ios::in);
        if (!file.is_open())
        {
            return false;
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();
        source = buffer.str();
        return true;
    }

    bool loadSource(const std::string& path, std::string& source)
    {
        if (readFile(path, source))
        {
            return true;
        }

        std::cerr
            << "Jett Error: could not open file '"
            << path
            << "'.\n";
        return false;
    }

    int checkSource(const std::string& source)
    {
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens);
        std::unique_ptr<Program> program = parser.parse();

        TypeChecker checker;
        checker.check(*program);

        if (checker.hasErrors())
        {
            for (const auto& error : checker.getErrors())
            {
                std::cerr << error << '\n';
            }
            return 1;
        }

        return 0;
    }

    int printTokens(const std::string& source)
    {
        Lexer lexer(source);
        const std::vector<Token> tokens = lexer.tokenize();

        for (const auto& token : tokens)
        {
            std::cout
                << token.line << ':'
                << token.column << "  "
                << tokenTypeToString(token.type)
                << "  '" << token.lexeme << "'\n";
        }

        return 0;
    }

    int printAst(const std::string& source)
    {
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens);
        std::unique_ptr<Program> program = parser.parse();

        ASTPrinter printer;
        printer.print(*program);
        return 0;
    }
}

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        printHelp();
        return 0;
    }

    const std::string argument = argv[1];

    if (argument == "--help" || argument == "-h")
    {
        if (argc != 2)
        {
            std::cerr << "Jett Error: --help does not accept a file argument.\n";
            return 2;
        }

        printHelp();
        return 0;
    }

    if (argument == "--version" || argument == "-v")
    {
        if (argc != 2)
        {
            std::cerr << "Jett Error: --version does not accept a file argument.\n";
            return 2;
        }

        std::cout << JETT_VERSION << '\n';
        return 0;
    }

    const bool isCheck = argument == "--check";
    const bool isTokens = argument == "--tokens";
    const bool isAst = argument == "--ast";

    std::string filePath;

    if (isCheck || isTokens || isAst)
    {
        if (argc != 3)
        {
            std::cerr
                << "Jett Error: " << argument
                << " requires exactly one .jett file.\n\n"
                << "Usage: jett " << argument << " <file.jett>\n";
            return 2;
        }

        filePath = argv[2];
    }
    else
    {
        if (!argument.empty() && argument[0] == '-')
        {
            std::cerr
                << "Jett Error: unknown option '"
                << argument
                << "'.\n\n"
                << "Run 'jett --help' to see available options.\n";
            return 2;
        }

        if (argc != 2)
        {
            std::cerr
                << "Jett Error: too many arguments.\n\n"
                << "Usage: jett <file.jett>\n";
            return 2;
        }

        filePath = argument;
    }

    std::string source;
    if (!loadSource(filePath, source))
    {
        return 1;
    }

    if (isTokens)
    {
        return printTokens(source);
    }

    if (isAst)
    {
        return printAst(source);
    }

    if (isCheck)
    {
        const int result = checkSource(source);
        if (result == 0)
        {
            std::cout
                << "Jett: no errors found in '"
                << filePath
                << "'.\n";
        }
        return result;
    }

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    std::unique_ptr<Program> program = parser.parse();

    TypeChecker checker;
    checker.check(*program);

    if (checker.hasErrors())
    {
        for (const auto& error : checker.getErrors())
        {
            std::cerr << error << '\n';
        }
        return 1;
    }

    Interpreter interpreter;
    interpreter.execute(*program);
    return 0;
}
