#include "Parser.hpp"

auto parser_error_node = std::make_shared<Node>(NodeType::ERROR);
auto parser_empty_node = std::make_shared<Node>(NodeType::EMPTY);
auto parser_string_node = std::make_shared<Node>(NodeType::STRING);
auto parser_int_node = std::make_shared<Node>(NodeType::INT);
auto parser_float_node = std::make_shared<Node>(NodeType::FLOAT);
auto parser_bool_node = std::make_shared<Node>(NodeType::BOOL);
auto parser_list_node = std::make_shared<Node>(NodeType::LIST);
auto parser_object_node = std::make_shared<Node>(NodeType::OBJECT);
auto parser_range_node = std::make_shared<Node>(NodeType::RANGE);
auto parser_function_node = std::make_shared<Node>(NodeType::FUNCTION);
auto parser_any_node = std::make_shared<Node>(NodeType::ANY);

std::vector<std::string> keywords = {
    "ret",
    "break",
    "continue",
    "if",
    "for",
    "while"
};

std::vector<NodeType> start_types = {
    NodeType::START_OF_FILE,
    NodeType::L_ANGLE,
    NodeType::L_PAREN,
    NodeType::L_BRACE,
    NodeType::L_BRACKET,
};

std::vector<NodeType> end_types = {
    NodeType::R_PAREN,
    NodeType::R_ANGLE,
    NodeType::R_BRACE,
    NodeType::R_BRACKET,
    NodeType::END_OF_FILE
};

void Parser::advance(int n)
{
    if ((index + n) >= nodes.size())
    {
        index = nodes.size() - 1;
        current_node = nodes[index];

        line = current_node->line;
        column = current_node->column;  
         
        return;
    }

    index += n;
    current_node = nodes[index];

    line = current_node->line;
    column = current_node->column; 
}

std::shared_ptr<Node>& Parser::peek(int n)
{
    if ((index + n) >= nodes.size())
    {
        return nodes[nodes.size() - 1];
    }

    return nodes[index + n];
}

void Parser::reset(int n)
{
    index = n;
    current_node = nodes[n];
}

void Parser::parse_paren()
{
    auto previous_node_type = current_node_type;
    current_node_type = NodeType::L_PAREN;

    int start = index;

    advance();

    parse(NodeType::R_PAREN, start + 1);

    int end = index;

    nodes.erase(nodes.begin() + end);
    nodes.erase(nodes.begin() + start);

    index = end - 1;
    current_node = nodes[index];

    advance(-1);

    current_node_type = previous_node_type;
}

void Parser::parse_block()
{
    auto previous_node_type = current_node_type;
    current_node_type = NodeType::BLOCK;
    
    int start = index;

    advance();

    parse(NodeType::R_BRACE, start + 1);

    nodes[start]->type = NodeType::BLOCK;

    int end = index;

    auto _start = nodes.begin() + start + 1;
    auto _end = nodes.begin() + end;

    nodes[start]->BLOCK.nodes = std::vector<std::shared_ptr<Node>>(_start, _end);

    nodes.erase(_start, _end + 1);

    index = start;
    current_node = nodes[index];

    current_node_type = previous_node_type;
}

void Parser::parse_list()
{
    auto previous_node_type = current_node_type;
    current_node_type = NodeType::LIST;

    int start = index;

    advance();

    parse(NodeType::R_BRACKET, start + 1);

    nodes[start]->type = NodeType::LIST;

    int end = index;

    auto _start = nodes.begin() + start + 1;
    auto _end = nodes.begin() + end;

    nodes[start]->LIST.nodes = std::vector<std::shared_ptr<Node>>(_start, _end);

    nodes.erase(_start, _end + 1);

    index = start;
    current_node = nodes[index];

    current_node_type = previous_node_type;
}

void Parser::parse_type()
{
    if (peek(-1)->type == NodeType::ID)
    {
        nodes[index-1]->ID.type = peek();
        nodes[index-1]->ID.type->is_type = true;
        index = index - 1;
        nodes.erase(nodes.begin() + index + 2);
        nodes.erase(nodes.begin() + index + 1);
    }
    else if (peek(-1)->OP.op_type == NodeType::TRIPLE_DOT)
    {
        nodes[index-1]->right->ID.type = peek();
        nodes[index-1]->right->ID.type->is_type = true;
        index = index - 1;
        nodes.erase(nodes.begin() + index + 2);
        nodes.erase(nodes.begin() + index + 1);
    }
    else if (peek(-1)->type == NodeType::LIST)
    {
        current_node->left = peek(-1);
        current_node->right = peek();

        nodes.erase(nodes.begin() + index + 1);
        nodes.erase(nodes.begin() + index - 1);
    }
    else
    {
        error_and_exit("Cannot assign type here");
    }

    current_node = nodes[index];
}

void Parser::parse_function_call()
{
    if (std::find(keywords.begin(), keywords.end(), current_node->ID.value) != keywords.end())
    {
        return;
    }

    auto previous_node_type = current_node_type;
    current_node_type = NodeType::FUNC_CALL;

    auto func_call_node = std::make_shared<Node>(NodeType::FUNC_CALL);
    func_call_node->line = current_node->line;
    func_call_node->column = current_node->column;

    if (current_node->type == NodeType::ID)
    {
        func_call_node->FUNC_CALL.name = current_node->ID.value;
    }
    else if (current_node->type == NodeType::FUNCTION)
    {
        func_call_node->FUNC_CALL.name = current_node->FUNCTION.name;
    }
    else
    {
        func_call_node->FUNC_CALL.name = current_node->FUNC_CALL.name;
    }

    // func_call_node->FUNC_CALL.name = current_node->type == NodeType::ID ? current_node->ID.value : current_node->FUNCTION.name;
    func_call_node->FUNC_CALL.caller = current_node;
    func_call_node->FUNC_CALL.args = peek()->LIST.nodes;

    // for (auto arg : func_call_node->FUNC_CALL.args)
    // {
    //     if (arg->type == NodeType::ID && arg->ID.type != nullptr)
    //     {
    //         error_and_exit("Cannot assign type inside a function call");
    //     }
    // }

    nodes.erase(nodes.begin() + index + 1);

    nodes[index] = func_call_node;

    current_node_type = previous_node_type;
}

void Parser::parse_function_def()
{
    auto previous_node_type = current_node_type;
    current_node_type = NodeType::FUNCTION;

    auto func_node = std::make_shared<Node>(NodeType::FUNCTION);
    func_node->line = current_node->line;
    func_node->column = current_node->column;

    auto params = peek(-1);

    if (!(params->type == NodeType::LIST || params->OP.op_type == NodeType::DOUBLE_COLON || params->type == NodeType::ID))
    {
        error_and_exit("Malformed function definition expression");
    }
    
    if (params->type == NodeType::ID)
    {
        func_node->FUNCTION.params.push_back(params);
    }
    else
    {
        func_node->FUNCTION.params = params->type == NodeType::LIST ? params->LIST.nodes : params->left->LIST.nodes;
    }
    
    // add nullptrs where the args should be, to maintain positions

    for (int i = 0; i < func_node->FUNCTION.params.size(); i++)
    {
        if (func_node->FUNCTION.params[i]->OP.op_type == NodeType::EQUAL)
        {
            func_node->FUNCTION.args.push_back(func_node->FUNCTION.params[i]->right);
            func_node->FUNCTION.default_arg_indices.push_back(i);
        }
        else
        {
            func_node->FUNCTION.args.push_back(nullptr);
        }
    }

    func_node->FUNCTION.return_type = params->type == NodeType::LIST || params->type == NodeType::ID ? nullptr : params->right;
    func_node->FUNCTION.body = peek();

    nodes.erase(nodes.begin() + index + 1);
    nodes.erase(nodes.begin() + index - 1);

    index--;
    current_node = nodes[index];

    nodes[index] = func_node;

    current_node_type = previous_node_type;
}

void Parser::parse_not()
{
    if (current_node->right != nullptr)
    {
        return;
    }

    auto previous_node_type = current_node_type;
    current_node_type = NodeType::OP;

    current_node->OP.op_type = NodeType::NOT;

    current_node->right = nodes[index + 1];
    current_node->OP.is_binary = false;

    nodes.erase(nodes.begin() + index + 1);

    current_node_type = previous_node_type;
}

void Parser::parse_un_op()
{
    if (current_node->right != nullptr)
    {
        return;
    }

    auto previous_node_type = current_node_type;
    current_node_type = NodeType::OP;

    auto& prev = peek(-1);
    auto& next = peek();

    if (std::find(start_types.begin(), start_types.end(), prev->type) != start_types.end()
    || ((prev->type == NodeType::ID && prev->ID.value == "ret")
    || (prev->type == NodeType::OP && prev->left == nullptr && prev->right == nullptr)))
    {
        if (std::find(end_types.begin(), end_types.end(), next->type) != end_types.end())
        {
            return;
        }

        if (current_node->OP.op_type == NodeType::PLUS)
        {
            current_node->OP.op_type = NodeType::POS;
        }
        if (current_node->OP.op_type == NodeType::MINUS)
        {
            current_node->OP.op_type = NodeType::NEG;
        }

        current_node->right = nodes[index + 1];
        current_node->OP.is_binary = false;

        nodes.erase(nodes.begin() + index + 1);

        current_node_type = previous_node_type;

        return;
    }

    current_node_type = previous_node_type;
}

void Parser::parse_bin_op()
{
    if (current_node->left != nullptr || current_node->right != nullptr)
    {
        return;
    }

    auto previous_node_type = current_node_type;
    current_node_type = NodeType::OP;

    auto& prev = peek(-1);

    if (std::find(start_types.begin(), start_types.end(), prev->type) != start_types.end() 
    || prev->ID.value == "_" 
    || (prev->type == NodeType::OP && prev->OP.is_binary && (prev->left == nullptr || prev->right == nullptr)))
    {
        current_node->type = NodeType::PARTIAL_OP;
        current_node->left = std::make_shared<Node>(NodeType::EMPTY);

        if (prev->ID.value == "_")
        {
            nodes.erase(nodes.begin() + index - 1);
            index--;
            current_node = nodes[index];
        }
    }

    auto& next = peek(1);

    if (std::find(end_types.begin(), end_types.end(), next->type) != end_types.end() 
    || next->ID.value == "_" 
    || (next->type == NodeType::OP && next->OP.is_binary && (next->left == nullptr || next->right == nullptr)))
    {
        current_node->type = NodeType::PARTIAL_OP;
        current_node->right = std::make_shared<Node>(NodeType::EMPTY);

        if (next->ID.value == "_")
        {
            nodes.erase(nodes.begin() + index + 1);
        }
    }

    if (current_node->type == NodeType::PARTIAL_OP)
    {
        if (current_node->right == nullptr)
        {
            current_node->right = nodes[index + 1];
            nodes.erase(nodes.begin() + index + 1);
        }

        if (current_node->left == nullptr)
        {
            current_node->left = nodes[index - 1];
            nodes.erase(nodes.begin() + index - 1);
            index = index - 1;
            current_node = nodes[index];
        }
    }
    else
    {
        current_node->left = nodes[index - 1];
        current_node->right = nodes[index + 1];

        nodes.erase(nodes.begin() + index + 1);
        nodes.erase(nodes.begin() + index - 1);

        index = index - 1;
        current_node = nodes[index];
    }

    // check that left hand side is NOT a partial node, unless op is : (for optional properties)

    if (current_node->left->type == NodeType::PARTIAL_OP && current_node->OP.op_type != NodeType::COLON)
    {
        error_and_exit("Left hand operand of '" + current_node->repr() + "' cannot be a partial operator");
    }

    // check if right hand side is partial, op must be either = or >>

    if (current_node->right->type == NodeType::PARTIAL_OP && current_node->type == NodeType::OP)
    {
        if (current_node->OP.op_type != NodeType::EQUAL && current_node->OP.op_type != NodeType::DOUBLE_ARROW)
        {
            error_and_exit("Right hand operand of '" + current_node->repr() + "' cannot be a partial operator");
        }
    }

    // check if =, do some equally stuff

    if (current_node->OP.op_type == NodeType::EQUAL)
    {
        if (current_node->left->type != NodeType::ID 
        && current_node->left->OP.op_type != NodeType::DOT
        && current_node->left->type != NodeType::LIST)
        {
            error_and_exit("Invalid left operand of operator '" + current_node->repr() + "'");
        }

        if (current_node->right->type == NodeType::OBJECT)
        {
            current_node->right->OBJECT.name = current_node->left->type == NodeType::ID ? current_node->left->ID.value : "";
        }

        if (current_node->right->type == NodeType::FUNCTION)
        {
            current_node->right->FUNCTION.name = current_node->left->type == NodeType::ID ? current_node->left->ID.value : "";
        }
    }

    current_node_type = previous_node_type;
}

void Parser::parse(NodeType end_node_type, int start)
{
    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::ID)
        {
            if (current_node->ID.value == "int")
            {
                nodes[index] = std::make_shared<Node>(NodeType::INT);
                nodes[index]->INT.value = NULL;
                nodes[index]->TYPE.is_literal = false;
                nodes[index]->TYPE.allowed_type = nodes[index];
                nodes[index]->line = current_node->line;
                nodes[index]->column = current_node->column;
            }
            else if (current_node->ID.value == "float")
            {
                nodes[index] = std::make_shared<Node>(NodeType::FLOAT);
                nodes[index]->FLOAT.value = NULL;
                nodes[index]->TYPE.is_literal = false;
                nodes[index]->TYPE.allowed_type = nodes[index];
                nodes[index]->line = current_node->line;
                nodes[index]->column = current_node->column;
            }
            else if (current_node->ID.value == "string")
            {
                nodes[index] = std::make_shared<Node>(NodeType::STRING);
                nodes[index]->STRING.value = "___string::default";
                nodes[index]->TYPE.is_literal = false;
                nodes[index]->TYPE.allowed_type = nodes[index];
                nodes[index]->line = current_node->line;
                nodes[index]->column = current_node->column;
            }
            else if (current_node->ID.value == "bool")
            {
                nodes[index] = std::make_shared<Node>(NodeType::BOOL);
                nodes[index]->BOOL.value = NULL;
                nodes[index]->TYPE.is_literal = false;
                nodes[index]->TYPE.allowed_type = nodes[index];
                nodes[index]->line = current_node->line;
                nodes[index]->column = current_node->column;
            }
            else if (current_node->ID.value == "list")
            {
                nodes[index] = std::make_shared<Node>(NodeType::LIST);
                nodes[index]->TYPE.is_literal = false;
                nodes[index]->TYPE.allowed_type = nodes[index];
                nodes[index]->line = current_node->line;
                nodes[index]->column = current_node->column;
            }
            else if (current_node->ID.value == "object")
            {
                nodes[index] = std::make_shared<Node>(NodeType::OBJECT);
                nodes[index]->OBJECT.is_type = true;
                nodes[index]->TYPE.is_literal = false;
                nodes[index]->TYPE.allowed_type = nodes[index];
                nodes[index]->line = current_node->line;
                nodes[index]->column = current_node->column;
            }
            else if (current_node->ID.value == "function")
            {
                nodes[index] = std::make_shared<Node>(NodeType::FUNCTION);
                nodes[index]->TYPE.is_literal = false;
                nodes[index]->TYPE.allowed_type = nodes[index];
                nodes[index]->FUNCTION.return_type = std::make_shared<Node>(NodeType::ANY);
                nodes[index]->line = current_node->line;
                nodes[index]->column = current_node->column;
            }
            else if (current_node->ID.value == "type")
            {
                nodes[index] = std::make_shared<Node>(NodeType::TYPE);
                nodes[index]->TYPE.is_literal = false;
                nodes[index]->TYPE.allowed_type = nodes[index];
                nodes[index]->line = current_node->line;
                nodes[index]->column = current_node->column;
            }
            else if (current_node->ID.value == "any")
            {
                nodes[index] = std::make_shared<Node>(NodeType::ANY);
                nodes[index]->TYPE.is_literal = false;
                nodes[index]->TYPE.allowed_type = nodes[index];
                nodes[index]->line = current_node->line;
                nodes[index]->column = current_node->column;
            }
            else if (current_node->ID.value == "null")
            {
                nodes[index] = std::make_shared<Node>(NodeType::EMPTY);
                nodes[index]->TYPE.is_literal = false;
                nodes[index]->TYPE.allowed_type = nodes[index];
                nodes[index]->line = current_node->line;
                nodes[index]->column = current_node->column;
            }
            else if (current_node->ID.value == "partial")
            {
                nodes[index] = std::make_shared<Node>(NodeType::PARTIAL_OP);
                nodes[index]->TYPE.is_literal = false;
                nodes[index]->TYPE.allowed_type = nodes[index];
                nodes[index]->line = current_node->line;
                nodes[index]->column = current_node->column;
            }
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::L_PAREN)
        {
            parse_paren();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::L_BRACKET)
        {
            parse_list();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::L_BRACE)
        {
            parse_block();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::ID && current_node->ID.value == "if" && peek(1)->type == NodeType::LIST)
        {
            auto conditional = peek(1);
            auto body = peek(2);

            if (conditional->LIST.nodes.size() != 1)
            {
                error_and_exit("If statement expects a boolean conditional");
            }

            if (body->type != NodeType::BLOCK)
            {
                error_and_exit("If statement body must be a block");
            }

            current_node->type = NodeType::IF_STATEMENT;
            current_node->IF_STATEMENT.conditional = conditional->LIST.nodes[0];
            current_node->IF_STATEMENT.body = body;

            nodes.erase(nodes.begin() + index + 2);
            nodes.erase(nodes.begin() + index + 1);
        }

        if (current_node->type == NodeType::ID && current_node->ID.value == "if" && peek(1)->type == NodeType::BLOCK)
        {
            current_node->type = NodeType::IF_BLOCK;
            current_node->IF_BLOCK.cases = peek(1)->BLOCK.nodes;

            nodes.erase(nodes.begin() + index + 1);
        }

        if (current_node->type == NodeType::ID && current_node->ID.value == "match" && peek(1)->type == NodeType::LIST)
        {
            auto value_list = peek(1);
            auto body = peek(2);

            if (value_list->LIST.nodes.size() != 1)
            {
                error_and_exit("Match statement expects a value to match against");
            }

            if (body->type != NodeType::BLOCK)
            {
                error_and_exit("Match statement body must be a block");
            }

            current_node->type = NodeType::MATCH_BLOCK;
            current_node->MATCH_BLOCK.value = value_list->LIST.nodes[0];
            //current_node->MATCH_BLOCK.cases = body->BLOCK.nodes;
            for (auto elem : body->BLOCK.nodes)
            {
                if (elem->OP.op_type != NodeType::SEMICOLON)
                {
                    current_node->MATCH_BLOCK.cases.push_back(elem);
                }
            }

            nodes.erase(nodes.begin() + index + 2);
            nodes.erase(nodes.begin() + index + 1);
        }


        if (current_node->type == NodeType::ID && current_node->ID.value == "for")
        {
            auto for_list = peek();
            auto for_body = peek(2);

            if (for_list->type != NodeType::LIST)
            {
                error_and_exit("Incorrect for loop syntax, missing initializer list");
            }
            if (for_list->LIST.nodes.size() == 0)
            {
                error_and_exit("Incorrect for loop syntax, empty initializer list");
            }
            if (for_list->LIST.nodes.size() > 3)
            {
                error_and_exit("Incorrect for loop syntax, too many arguments in initializer list - maximum arguments count is 3 [iterator index value]");
            }
            if (for_body->type != NodeType::BLOCK)
            {
                error_and_exit("Incorrect for loop syntax, missing loop body");
            }

            current_node->type = NodeType::FOR_LOOP;
            current_node->FOR_LOOP.init_list = for_list;
            current_node->FOR_LOOP.body = for_body;

            nodes.erase(nodes.begin() + index + 2);
            nodes.erase(nodes.begin() + index + 1);
        }

        if (current_node->type == NodeType::ID && current_node->ID.value == "while")
        {
            auto conditional = peek();
            auto body = peek(2);

            if (conditional->type != NodeType::LIST)
            {
                error_and_exit("Incorrect while loop syntax, missing conditional");
            }
            if (conditional->LIST.nodes.size() == 0)
            {
                error_and_exit("Incorrect while loop syntax, empty conditional");
            }
            if (conditional->LIST.nodes.size() != 1)
            {
                error_and_exit("While loop expects a boolean conditional");
            }
            if (body->type != NodeType::BLOCK)
            {
                error_and_exit("Incorrect while loop syntax, missing loop body");
            }

            current_node->type = NodeType::WHILE_LOOP;
            current_node->WHILE_LOOP.conditional = conditional->LIST.nodes[0];
            current_node->WHILE_LOOP.body = body;

            nodes.erase(nodes.begin() + index + 2);
            nodes.erase(nodes.begin() + index + 1);
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::BLOCK)
        {
            if (nodes[index]->BLOCK.nodes.size() > 0 && nodes[index]->BLOCK.nodes[0]->type == NodeType::OP && nodes[index]->BLOCK.nodes[0]->OP.op_type == NodeType::COLON)
            {
                nodes[index]->type = NodeType::OBJECT;

                for (auto& node : nodes[index]->BLOCK.nodes)
                {
                    if (node->type != NodeType::OP && node->OP.op_type != NodeType::COLON)
                    {
                        error_and_exit("Object must contain properties");
                    }

                    auto left = node->left;
                    auto right = node->right;

                    Object_Prop property;

                    if (left->OP.op_type == NodeType::QUESTION)
                    {
                        left = left->left;
                        property.required = false;
                        property.value = right;
                    }
                    else
                    {
                        property.value = right;
                    }

                    if (left->type != NodeType::ID && left->type != NodeType::STRING)
                    {
                        // error_and_exit("Object properties must be either identifiers or strings");
                        return;
                    }

                    std::string prop_name = left->type == NodeType::ID ? left->ID.value : left->STRING.value;

                    if (right->type == NodeType::FUNCTION)
                    {
                        right->FUNCTION.name = left->type == NodeType::ID ? left->ID.value : left->STRING.value;

                        // If function already exists of the same name
                        // We create a pipe list for multiple dispatch

                        if (nodes[index]->OBJECT.properties.find(prop_name) != nodes[index]->OBJECT.properties.end())
                        {
                            if (nodes[index]->OBJECT.properties[prop_name].value->type == NodeType::FUNCTION)
                            {
                                auto func_list = std::make_shared<Node>(NodeType::PIPE_LIST);
                                func_list->PIPE_LIST.nodes.push_back(nodes[index]->OBJECT.properties[prop_name].value);
                                func_list->PIPE_LIST.nodes.push_back(property.value);
                                nodes[index]->OBJECT.properties[prop_name].value = func_list;
                                continue;
                            }
                            else if (nodes[index]->OBJECT.properties[prop_name].value->type == NodeType::PIPE_LIST)
                            {
                                nodes[index]->OBJECT.properties[prop_name].value->PIPE_LIST.nodes.push_back(property.value);
                                continue;
                            }
                        }
                    }

                    nodes[index]->OBJECT.properties[prop_name] = property;
                }

                nodes[index]->BLOCK.nodes.clear();
            }
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if ((current_node->type == NodeType::ID && peek()->type == NodeType::LIST) 
        || (current_node->type == NodeType::FUNCTION && peek()->type == NodeType::LIST)
        || (current_node->OP.op_type == NodeType::DOT && current_node->left != nullptr && current_node->right != nullptr && peek()->type == NodeType::LIST))
        {
            parse_function_call();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::DOUBLE_DOT)
        {
            parse_bin_op();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::DOT)
        {
            parse_bin_op();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->OP.op_type == NodeType::HASH)
        {
            current_node->type = NodeType::COPY;

            if (std::find(end_types.begin(), end_types.end(), peek()->type) != end_types.end())
            {
                error_and_exit("Operator '" + current_node->repr() + "' cannot have an empty right operand");
            }

            if (peek()->type != NodeType::ID)
            {
                error_and_exit("Unexpected right operand for operator '" + current_node->repr() + "'");
            }

            current_node->right = peek();
            nodes.erase(nodes.begin() + index + 1);
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::PLUS || current_node->OP.op_type == NodeType::MINUS)
        {
            parse_un_op();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::PLUS_EQ || current_node->OP.op_type == NodeType::MINUS_EQ)
        {
            parse_bin_op();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::PERCENT)
        {
            parse_bin_op();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::STAR || current_node->OP.op_type == NodeType::SLASH)
        {
            parse_bin_op();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::PLUS || current_node->OP.op_type == NodeType::MINUS)
        {
            parse_bin_op();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && (current_node->OP.op_type == NodeType::L_ANGLE || current_node->OP.op_type == NodeType::R_ANGLE))
        {
            parse_bin_op();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::EQ_EQ 
        || current_node->OP.op_type == NodeType::NOT_EQUAL
        || current_node->OP.op_type == NodeType::GT_EQUAL
        || current_node->OP.op_type == NodeType::LT_EQUAL)
        {
            parse_bin_op();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::AND || current_node->OP.op_type == NodeType::OR)
        {
            parse_bin_op();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::TRIPLE_DOT)
        {
            if (peek()->type != NodeType::ID)
            {
                // error_and_exit("Unexpected operand for operator '" + current_node->repr() + "'");
            }
            else
            {
                nodes[index]->right = peek();
                nodes.erase(nodes.begin() + index + 1);
            }
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::QUESTION)
        {
            parse_bin_op();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::EXCLAMATION)
        {
            parse_not();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::DOUBLE_COLON)
        {
            parse_type();
        }

        advance();
    }

    reset(start);


    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::COMMA)
        {
            parse_bin_op();
        }

        advance();
    }

    // Flatten comma list

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::COMMA)
        {
            flatten_comma_list(current_node);
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::DOUBLE_ARROW)
        {
            parse_bin_op();
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::RIGHT_ARROW_DOUBLE)
        {
            parse_function_def();
        }

        advance();
    }

    reset(start);


    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::PIPE)
        {
            parse_bin_op();
        }

        advance();
    }

    reset(start);

    // flatten pipe list

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::PIPE)
        {
            flatten_pipe_list(current_node);
        }

        advance();
    }

    reset(start);


    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::COLON)
        {
            parse_bin_op();
        }

        advance();
    }

    reset(start);

    // while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    // {
    //     if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::COMMA)
    //     {
    //         parse_bin_op();
    //     }

    //     advance();
    // }

    // // Flatten comma list

    // reset(start);

    // while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    // {
    //     if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::COMMA)
    //     {
    //         flatten_comma_list(current_node);
    //     }

    //     advance();
    // }

    // reset(start);

    // while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    // {
    //     if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::DOUBLE_ARROW)
    //     {
    //         parse_bin_op();
    //     }

    //     advance();
    // }

    // reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::ID && (std::find(keywords.begin(), keywords.end(), current_node->ID.value) != keywords.end()))
        {
            current_node->type = NodeType::KEYWORD;

            if (current_node->ID.value == "ret")
            {
                if (std::find(end_types.begin(), end_types.end(), peek()->type) != end_types.end())
                {
                    current_node->right = std::make_shared<Node>(NodeType::EMPTY);
                }
                else
                {
                    current_node->right = peek();
                    nodes.erase(nodes.begin() + index + 1);
                }
            }
        }

        advance();
    }

    reset(start);

    while (current_node->type != end_node_type && current_node->type != NodeType::END_OF_FILE)
    {
        if (current_node->type == NodeType::OP && current_node->OP.op_type == NodeType::EQUAL)
        {
            parse_bin_op();
        }

        advance();
    }

    // Checking to see if we've hit an end node that isn't part of the current structure

    if (current_node_type == NodeType::LIST && current_node->type != NodeType::R_BRACKET)
    {
        error_and_exit("Missing ']' while parsing list");
    }

    if (current_node_type == NodeType::BLOCK && current_node->type != NodeType::R_BRACE)
    {
        error_and_exit("Missing '}' while parsing block");
    }

    if (current_node_type == NodeType::L_PAREN && current_node->type != NodeType::R_PAREN)
    {
        error_and_exit("Missing ')' while parsing parentheses");
    }

    // Checking to see if we've hit an end node on its own

    reset(start);

    while (current_node->type != end_node_type)
    {
        if (std::find(end_types.begin(), end_types.end(), current_node->type) != end_types.end())
        {
            error_and_exit("Dangling '" + current_node->repr() + "'");
        }

        advance();
    }
}

void Parser::flatten_comma_list(std::shared_ptr<Node>& node)
{
    if (node->OP.op_type == NodeType::COMMA)
    {
        node->type = NodeType::COMMA_LIST;

        if (node->left->OP.op_type == NodeType::COMMA)
        {
            flatten_comma_list(node->left);
        }
        else
        {
            node->COMMA_LIST.nodes.push_back(node->left);
        }

        if (node->left->type == NodeType::COMMA_LIST)
        {
            for (auto elem : node->left->COMMA_LIST.nodes)
            {
                node->COMMA_LIST.nodes.push_back(elem);
            }

            node->left = std::make_shared<Node>(NodeType::EMPTY);
            node->left->COMMA_LIST.nodes.clear();
        }

        if (node->right->OP.op_type == NodeType::COMMA)
        {
            flatten_comma_list(node->right);
        }
        else
        {
            node->COMMA_LIST.nodes.push_back(node->right);
        }

        if (node->right->type == NodeType::COMMA_LIST)
        {
            for (auto elem : node->right->COMMA_LIST.nodes)
            {
                node->COMMA_LIST.nodes.push_back(elem);
            }

            node->right = std::make_shared<Node>(NodeType::EMPTY);
            node->right->COMMA_LIST.nodes.clear();
        }

        node->left = nullptr;
        node->right = nullptr;
    }
}

void Parser::flatten_pipe_list(std::shared_ptr<Node>& node)
{
    if (node->OP.op_type == NodeType::PIPE)
    {
        node->type = NodeType::PIPE_LIST;

        if (node->left->OP.op_type == NodeType::PIPE)
        {
            flatten_pipe_list(node->left);
        }
        else
        {
            node->PIPE_LIST.nodes.push_back(node->left);
        }

        if (node->left->type == NodeType::PIPE_LIST)
        {
            for (auto elem : node->left->PIPE_LIST.nodes)
            {
                node->PIPE_LIST.nodes.push_back(elem);
            }

            node->left = std::make_shared<Node>(NodeType::EMPTY);
            node->left->PIPE_LIST.nodes.clear();
        }

        if (node->right->OP.op_type == NodeType::PIPE)
        {
            flatten_pipe_list(node->right);
        }
        else
        {
            node->PIPE_LIST.nodes.push_back(node->right);
        }

        if (node->right->type == NodeType::PIPE_LIST)
        {
            for (auto elem : node->right->PIPE_LIST.nodes)
            {
                node->PIPE_LIST.nodes.push_back(elem);
            }

            node->right = std::make_shared<Node>(NodeType::EMPTY);
            node->right->PIPE_LIST.nodes.clear();
        }

        node->left = nullptr;
        node->right = nullptr;
    }
}

void Parser::error_and_exit(std::string message)
{
    std::string error_message = "Parsing Error in '" + file_name + "' @ (" + std::to_string(line) + ", " + std::to_string(column) + "): " + message;
	std::cout << error_message << "\n";
    exit(1);
}