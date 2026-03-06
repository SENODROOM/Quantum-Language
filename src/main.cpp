#include "../include/Lexer.h"
#include "../include/Parser.h"
#include "../include/Interpreter.h"
#include "../include/Error.h"
#include "../include/Value.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif

static void printBanner()
{
    std::cout << Colors::CYAN << Colors::BOLD
              << "\n"
              << "  ██████╗ ██╗   ██╗ █████╗ ███╗   ██╗████████╗██╗   ██╗███╗   ███╗\n"
              << " ██╔═══██╗██║   ██║██╔══██╗████╗  ██║╚══██╔══╝██║   ██║████╗ ████║\n"
              << " ██║   ██║██║   ██║███████║██╔██╗ ██║   ██║   ██║   ██║██╔████╔██║\n"
              << " ██║▄▄ ██║██║   ██║██╔══██║██║╚██╗██║   ██║   ██║   ██║██║╚██╔╝██║\n"
              << " ╚██████╔╝╚██████╔╝██║  ██║██║ ╚████║   ██║   ╚██████╔╝██║ ╚═╝ ██║\n"
              << "  ╚══▀▀═╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝   ╚═╝    ╚═════╝ ╚═╝     ╚═╝\n"
              << Colors::RESET
              << Colors::YELLOW << "  Quantum Language v1.0.0 | The Cybersecurity-Ready Scripting Language\n"
              << Colors::RESET << "\n";
}

static void printAura()
{
    std::cout << Colors::CYAN << Colors::BOLD
              << "\n╔══════════════════════════════════════════════════════════════════╗\n"
              << "║" << Colors::YELLOW << "                🌟 QUANTUM LANGUAGE ACHIEVEMENTS 🌟" << Colors::CYAN << "               ║\n"
              << "╠══════════════════════════════════════════════════════════════════╣\n"
              << "║" << Colors::GREEN << "  ✅ Complete C++17 Compiler Implementation" << Colors::CYAN << "                       ║\n"
              << "║" << Colors::GREEN << "  ✅ Tree-Walk Interpreter Engine" << Colors::CYAN << "                                 ║\n"
              << "║" << Colors::GREEN << "  ✅ Lexical Analysis & Tokenization" << Colors::CYAN << "                              ║\n"
              << "║" << Colors::GREEN << "  ✅ Recursive Descent Parser" << Colors::CYAN << "                                     ║\n"
              << "║" << Colors::GREEN << "  ✅ Abstract Syntax Tree (AST)" << Colors::CYAN << "                                   ║\n"
              << "║" << Colors::GREEN << "  ✅ Dynamic Type System" << Colors::CYAN << "                                          ║\n"
              << "║" << Colors::GREEN << "  ✅ REPL Interactive Mode" << Colors::CYAN << "                                        ║\n"
              << "║" << Colors::GREEN << "  ✅ Cross-Platform Build System" << Colors::CYAN << "                                  ║\n"
              << "║" << Colors::GREEN << "  ✅ VS Code Language Support" << Colors::CYAN << "                                     ║\n"
              << "║" << Colors::GREEN << "  ✅ GitHub CI/CD Pipeline" << Colors::CYAN << "                                        ║\n"
              << "║" << Colors::GREEN << "  ✅ Comprehensive Documentation" << Colors::CYAN << "                                  ║\n"
              << "╠══════════════════════════════════════════════════════════════════╣\n"
              << "║" << Colors::MAGENTA << "                    📊 PROJECT STATISTICS 📊" << Colors::CYAN << "                      ║\n"
              << "╠══════════════════════════════════════════════════════════════════╣\n"
              << "║" << Colors::WHITE << "  📁 Source Files: " << Colors::YELLOW << "6 core modules" << Colors::CYAN << "                                 ║\n"
              << "║" << Colors::WHITE << "  📝 Language Version: " << Colors::YELLOW << "v1.0.0" << Colors::CYAN << "                                     ║\n"
              << "║" << Colors::WHITE << "  🔧 Build System: " << Colors::YELLOW << "CMake + MSVC" << Colors::CYAN << "                                   ║\n"
              << "║" << Colors::WHITE << "  🎯 Language Standard: " << Colors::YELLOW << "C++17" << Colors::CYAN << "                                     ║\n"
              << "║" << Colors::WHITE << "  🚀 Performance: " << Colors::YELLOW << "Optimized Release Build" << Colors::CYAN << "                         ║\n"
              << "╠══════════════════════════════════════════════════════════════════╣\n"
              << "║" << Colors::BLUE << "                    🛡️  CYBERSECURITY FEATURES 🛡️" << Colors::CYAN << "                   ║\n"
              << "╠══════════════════════════════════════════════════════════════════╣\n"
              << "║" << Colors::BLUE << "  🔍 scan()      - Network scanning capabilities" << Colors::CYAN << "                  ║\n"
              << "║" << Colors::BLUE << "  💣 payload()   - Exploit payload creation" << Colors::CYAN << "                       ║\n"
              << "║" << Colors::BLUE << "  🔐 encrypt()   - Cryptographic operations" << Colors::CYAN << "                       ║\n"
              << "║" << Colors::BLUE << "  🔓 decrypt()   - Decryption functions" << Colors::CYAN << "                           ║\n"
              << "║" << Colors::BLUE << "  🗝️  hash()     - Hashing algorithms" << Colors::CYAN << "                              ║\n"
              << "╚══════════════════════════════════════════════════════════════════╝\n"
              << Colors::RESET;
}

static void runREPL()
{
    printBanner();
    std::cout << Colors::GREEN << "  REPL Mode — type 'exit' or Ctrl+D to quit\n"
              << Colors::RESET << "\n";

    Interpreter interp;
    std::string line;
    int lineNum = 1;

    while (true)
    {
        std::cout << Colors::CYAN << "quantum[" << lineNum++ << "]> " << Colors::RESET;
        if (!std::getline(std::cin, line))
            break;
        if (line == "exit" || line == "quit")
            break;
        if (line.empty())
            continue;

        try
        {
            Lexer lexer(line);
            auto tokens = lexer.tokenize();
            Parser parser(std::move(tokens));
            auto ast = parser.parse();
            interp.execute(*ast);
        }
        catch (const ParseError &e)
        {
            std::cerr << Colors::RED << "[ParseError] " << Colors::RESET << e.what() << " (line " << e.line << ")\n";
        }
        catch (const QuantumError &e)
        {
            std::cerr << Colors::RED << "[" << e.kind << "] " << Colors::RESET << e.what();
            if (e.line > 0)
                std::cerr << " (line " << e.line << ")";
            std::cerr << "\n";
        }
        catch (const std::exception &e)
        {
            std::cerr << Colors::RED << "[Error] " << Colors::RESET << e.what() << "\n";
        }
    }

    std::cout << Colors::YELLOW << "\n  Goodbye! 👋\n"
              << Colors::RESET;
}

static void runFile(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << Colors::RED << "[Error] " << Colors::RESET
                  << "Cannot open file: " << path << "\n";
        std::exit(1);
    }

    // Check extension
    if (path.size() < 3 || path.substr(path.size() - 3) != ".sa")
    {
        std::cerr << Colors::YELLOW << "[Warning] " << Colors::RESET
                  << "File does not have .sa extension\n";
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string source = ss.str();

    try
    {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(std::move(tokens));
        auto ast = parser.parse();

        Interpreter interp;
        // Execute in globals scope so top-level functions/classes are accessible
        if (ast->is<BlockStmt>())
        {
            interp.execBlock(ast->as<BlockStmt>(), interp.globals);
        }
        else
        {
            interp.execute(*ast);
        }

        // C/C++ style: if a top-level "main" function was defined, call it automatically
        // (only if it wasn't already called during execution)
        try
        {
            auto mainFn = interp.globals->get("main");
            if (mainFn.isFunction() &&
                std::holds_alternative<std::shared_ptr<QuantumFunction>>(mainFn.data))
            {
                // Build a call expression node and evaluate it
                CallExpr ce;
                ce.callee = std::make_unique<ASTNode>(Identifier{"main"}, 0);
                ASTNode callNode(std::move(ce), 0);
                interp.evaluate(callNode);
            }
        }
        catch (const ReturnSignal &)
        {
        } // main returned normally
        catch (const NameError &)
        {
        } // main not defined — normal for Quantum scripts
        catch (const QuantumError &e)
        {
            // Report errors that happen INSIDE main()
            if (std::string(e.what()).find("main") == std::string::npos)
            {
                std::cerr << Colors::RED << Colors::BOLD
                          << "\n  \u2717 " << e.kind << Colors::RESET;
                if (e.line > 0)
                    std::cerr << " at line " << e.line;
                std::cerr << "\n    " << e.what() << "\n\n";
                std::exit(1);
            }
        }
        catch (...)
        {
        } // main not defined or other non-critical issue
    }
    catch (const ParseError &e)
    {
        std::cerr << Colors::RED << Colors::BOLD
                  << "\n  ✗ ParseError" << Colors::RESET
                  << " in " << path << " at line " << e.line << ":" << e.col << "\n"
                  << "    " << e.what() << "\n\n";
        std::exit(1);
    }
    catch (const QuantumError &e)
    {
        std::cerr << Colors::RED << Colors::BOLD
                  << "\n  ✗ " << e.kind << Colors::RESET;
        if (e.line > 0)
            std::cerr << " at line " << e.line;
        std::cerr << "\n    " << e.what() << "\n\n";
        std::exit(1);
    }
    catch (const std::exception &e)
    {
        std::cerr << Colors::RED << "[Fatal] " << Colors::RESET << e.what() << "\n";
        std::exit(1);
    }
}

static int checkFile(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << path << ":1:1: error: Cannot open file\n";
        return 1;
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    std::string source = ss.str();
    try
    {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        auto ast = parser.parse();
        (void)ast;
        return 0;
    }
    catch (const ParseError &e)
    {
        std::cerr << path << ":" << e.line << ":" << e.col << ": error: " << e.what() << "\n";
        return 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << path << ":1:1: error: " << e.what() << "\n";
        return 1;
    }
}

static void printHelp(const char *prog)
{
    std::cout << Colors::BOLD << "Usage:\n"
              << Colors::RESET
              << "  " << prog << " <file.sa>          Run a Quantum script\n"
              << "  " << prog << "                     Start interactive REPL\n"
              << "  " << prog << " --help              Show this help\n"
              << "  " << prog << " --version           Show version info\n\n"
              << Colors::BOLD << "File extension:\n"
              << Colors::RESET
              << "  Quantum scripts use the .sa extension\n\n"
              << Colors::BOLD << "Examples:\n"
              << Colors::RESET
              << "  quantum hello.sa\n"
              << "  quantum script.sa\n";
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    if (argc == 1)
    {
        runREPL();
        return 0;
    }

    std::string arg = argv[1];

    if (arg == "--help" || arg == "-h")
    {
        printBanner();
        printHelp(argv[0]);
        return 0;
    }

    if (arg == "--aura")
    {
        printBanner();
        printAura();
        return 0;
    }

    if (arg == "--version" || arg == "-v")
    {
        std::cout << "Quantum Language v1.0.0\n"
                  << "Runtime: Tree-walk interpreter\n"
                  << "Built By Muhammad Saad Amin\n";
        return 0;
    }

    if (arg == "--check" && argc >= 3)
    {
        return checkFile(argv[2]);
    }
    runFile(arg);
    return 0;
}