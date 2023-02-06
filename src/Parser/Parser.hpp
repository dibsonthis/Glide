#pragma once
#include "../Lexer/Lexer.hpp"

class Parser
{
    int index = 0;
    std::shared_ptr<Node> current_node;
    int line, column = 1;
    NodeType current_node_type = NodeType::START_OF_FILE;

public:

    std::string file_name;
    std::vector<std::shared_ptr<Node>> nodes;
    std::vector<std::string> errors;

    Parser(std::string& file_name, std::vector<std::shared_ptr<Node>>& nodes) : file_name(file_name), nodes(nodes) 
    {
        if (nodes.size() == 0)
        {
            current_node = nullptr;
            return;
        }

        current_node = nodes[0];
    }

    void advance(int n = 1);
    std::shared_ptr<Node>& peek(int n = 1);
    void reset(int n = 0);

    void parse(NodeType end_node_type = NodeType::END_OF_FILE, int start = 0);
    void parse_paren();
    void parse_block();
    void parse_list();
    void parse_type();
    void parse_function_call();
    void parse_function_def();
    void parse_arrow();
    void parse_not();
    void parse_un_op();
    void parse_bin_op();

    void flatten_comma_list(std::shared_ptr<Node>& node);
    void flatten_pipe_list(std::shared_ptr<Node>& node);

    void error_and_exit(std::string message);
};