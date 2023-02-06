#include <iostream>
#include <fstream>
#include "src/libs/JSON/json.hpp"
#include "src/Lexer/Lexer.hpp"
#include "src/Parser/Parser.hpp"
#include "src/Evaluator/Evaluator.hpp"
#include "src/Typechecker/Typechecker.hpp"

enum Type
{
    DEV,
    INTERP,
};

Type type = Type::DEV;

int main(int argc, char** argv)
{
    if (type == Type::DEV)
    {
        std::string buitins_path;

        std::ifstream fJson("../../../glide.json");
        std::stringstream buffer;
        buffer << fJson.rdbuf();
        auto size = buffer.str().size();

        if (size == 0)
        {
            buitins_path = "../../../src/builtins/builtins.gl";
        }
        else
        {
            auto json = nlohmann::json::parse(buffer.str());
            buitins_path = json["settings"]["builtins_path"];
        }

        Lexer lexer("../../../source.gl");
        lexer.builtins_path = buitins_path;
        lexer.tokenize();

        Parser parser(lexer.file_name, lexer.nodes);
        parser.parse();

        Typechecker tc(parser.file_name, parser.nodes);
        tc.builtins_path = lexer.builtins_path;
        tc.init(buitins_path);
        tc.evaluate();

        Evaluator evaluator(tc.file_name, tc.nodes);
        evaluator.builtins_path = lexer.builtins_path;
        evaluator.init(buitins_path);
        evaluator.evaluate();

        std::cin.get();
        exit(0);
    }

    if (type == Type::INTERP)
    {
        if (argc == 1)
        {
            std::cout << "You must enter a source path e.g: glide \"main.gl\"\n";
            return 1;
        }

        if (argc > 2)
        {
            std::cout << "Compiler only accepts 1 argument: source path";
            return 1;
        }

        std::string buitins_path;

        std::ifstream fJson("glide.json");
        std::stringstream buffer;
        buffer << fJson.rdbuf();
        auto size = buffer.str().size();

        if (size == 0)
        {
            buitins_path = "builtins/builtins.gl";
        }
        else
        {
            auto json = nlohmann::json::parse(buffer.str());
            buitins_path = json["settings"]["builtins_path"];
        }

        std::string path = argv[1];

        Lexer lexer(path);
        lexer.builtins_path = buitins_path;
        lexer.tokenize();

        Parser parser(lexer.file_name, lexer.nodes);
        parser.parse();

        Typechecker tc(parser.file_name, parser.nodes);
        tc.builtins_path = lexer.builtins_path;
        tc.init(buitins_path);
        tc.evaluate();

        Evaluator evaluator(tc.file_name, tc.nodes);
        evaluator.builtins_path = lexer.builtins_path;
        evaluator.init(buitins_path);
        evaluator.evaluate();

        exit(0);
    }
}