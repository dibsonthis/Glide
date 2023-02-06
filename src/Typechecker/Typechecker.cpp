#include "Typechecker.hpp"

auto tc_error_node = std::make_shared<Node>(NodeType::ERROR);

// Node Helpers
std::shared_ptr<Node> Typechecker::make_int(long long value, bool literal)
{
    auto node = std::make_shared<Node>(NodeType::INT);
    node->INT.value = value;
    node->TYPE.is_literal = literal;
    node->TYPE.allowed_type = node;
    node->line = line;
    node->column = column;
    return node;
}
std::shared_ptr<Node> Typechecker::make_float(double value, bool literal)
{
    auto node = std::make_shared<Node>(NodeType::FLOAT);
    node->FLOAT.value = value;
    node->TYPE.is_literal = literal;
    node->TYPE.allowed_type = node;
    node->line = line;
    node->column = column;
    return node;
}
std::shared_ptr<Node> Typechecker::make_string(std::string value, bool literal)
{
    auto node = std::make_shared<Node>(NodeType::STRING);
    node->STRING.value = value;
    node->TYPE.is_literal = literal;
    node->TYPE.allowed_type = node;
    node->line = line;
    node->column = column;
    return node;
}
std::shared_ptr<Node> Typechecker::make_bool(bool value, bool literal)
{
    auto node = std::make_shared<Node>(NodeType::BOOL);
    node->BOOL.value = value;
    node->TYPE.is_literal = literal;
    node->TYPE.allowed_type = node;
    node->line = line;
    node->column = column;
    return node;
}
std::shared_ptr<Node> Typechecker::make_empty(bool literal)
{
    auto node = std::make_shared<Node>(NodeType::EMPTY);
    node->TYPE.is_literal = literal;
    node->TYPE.allowed_type = node;
    node->line = line;
    node->column = column;
    return node;
}
std::shared_ptr<Node> Typechecker::make_any(bool literal)
{
    auto node = std::make_shared<Node>(NodeType::ANY);
    node->TYPE.is_literal = literal;
    node->TYPE.allowed_type = node;
    node->line = line;
    node->column = column;
    return node;
}
std::shared_ptr<Node> Typechecker::make_list(bool literal)
{
    auto node = std::make_shared<Node>(NodeType::LIST);
    node->TYPE.is_literal = literal;
    if (!literal)
    {
        node->LIST.nodes.push_back(make_any());
        node->TYPE.allowed_type = node;
    }
    node->TYPE.allowed_type = node;
    node->line = line;
    node->column = column;
    return node;
}
std::shared_ptr<Node> Typechecker::make_pipe_list(bool literal)
{
    auto node = std::make_shared<Node>(NodeType::PIPE_LIST);
    node->TYPE.is_literal = literal;
    node->line = line;
    node->column = column;
    return node;
}
std::shared_ptr<Node> Typechecker::make_object(bool literal)
{
    auto node = std::make_shared<Node>(NodeType::OBJECT);
    node->TYPE.is_literal = literal;
    node->TYPE.allowed_type = node;
    node->line = line;
    node->column = column;
    return node;
}
std::shared_ptr<Node> Typechecker::make_function(bool literal)
{
    auto node = std::make_shared<Node>(NodeType::FUNCTION);
    node->TYPE.is_literal = literal;
    node->TYPE.allowed_type = node;
    node->line = line;
    node->column = column;
    return node;
}

void Typechecker::sort_and_unique(std::vector<std::shared_ptr<Node>>& list)
{
    std::sort(list.begin(), list.end(), [] (std::shared_ptr<Node>& a, std::shared_ptr<Node>& b) {return int(a->type) < int(b->type);});
    list.erase(std::unique(list.begin(), list.end(), [this] (std::shared_ptr<Node>& a, std::shared_ptr<Node>& b) {return match_types(a, b).result;}), list.end());
}

std::shared_ptr<Node> Typechecker::get_explicit_type(std::shared_ptr<Node>& node)
{
    auto result = make_any();

    if (node->type == NodeType::ID)
    {
        //result = eval_node(node->ID.type);
        result = node->ID.type;
    }
    else if (node->OP.op_type == NodeType::TRIPLE_DOT)
    {
        //result = eval_node(node->right->ID.type);
        result = node->right->ID.type;
        if (result == nullptr)
        {
            auto ls = make_list();
            ls->LIST.nodes.push_back(make_any());
            result = ls;
        }
    }
    else if (node->OP.op_type == NodeType::EQUAL)
    {
        //result = eval_node(node->left->ID.type);
        result = node->left->ID.type;
    }

    if (result == nullptr)
    {
        result = make_any();
    }

    if (result->type != NodeType::FUNCTION)
    {
        result = eval_node(result);
    }

    if (result->type == NodeType::FUNCTION && result->FUNCTION.return_type == nullptr)
    {
        result = eval_node(result);
    }

    if (result->type == NodeType::LIST && result->LIST.nodes.size() == 0)
    {
        result->LIST.nodes.push_back(make_any());
        result->TYPE.allowed_type->LIST.nodes.push_back(make_any());
    }

    return result;
}

// -- Built-in Functions -- //

std::shared_ptr<Node> Typechecker::builtin_exit(std::vector<std::shared_ptr<Node>> args)
{
    if (args.size() > 1)
    {
        error_and_exit("Built-in function 'exit' expects 0 or 1 arguments");
    }

    return make_empty(false);
}

std::shared_ptr<Node> Typechecker::builtin_error(std::vector<std::shared_ptr<Node>> args)
{
    if (args.size() != 2)
    {
        error_and_exit("Built-in function 'error' expects 2 arguments");
    }

    auto type = eval_node(args[0]);
    auto message = eval_node(args[1]);

    if (type->type != NodeType::STRING && message->type != NodeType::STRING)
    {
        error_and_exit("Built-in function 'error' expects 2 string arguments");
    }

    return make_empty(false);
}

std::shared_ptr<Node> Typechecker::builtin_range(std::vector<std::shared_ptr<Node>> args)
{
    if (args.size() != 2)
    {
        error_and_exit("Built-in function 'range' expects 2 arguments");
    }

    auto start_node = eval_node(args[0]);
    auto end_node = eval_node(args[1]);

    if (start_node->type != NodeType::INT && end_node->type != NodeType::INT)
    {
        error_and_exit("Built-in function 'range' expects 2 integer arguments");
    }

    auto res = std::make_shared<Node>(NodeType::RANGE);

    return res;
}

std::shared_ptr<Node> Typechecker::builtin_print(std::vector<std::shared_ptr<Node>> args)
{
    if (args.size() < 1)
    {
        error_and_exit("Built-in function 'print' expects at least one argument");
    }

    return make_empty(false);
}

std::shared_ptr<Node> Typechecker::builtin_delete(std::vector<std::shared_ptr<Node>> args)
{
    for (auto arg : args)
    {
        if (arg->type != NodeType::ID && arg->type != NodeType::STRING)
        {
            error_and_exit("Built-in function 'delete' expects arguments to be either identifiers or strings");
        }

        auto name = arg->type == NodeType::ID ? arg->ID.value : arg->STRING.value;

        symbol_table.erase(name);
    }

    return make_empty();
}

std::shared_ptr<Node> Typechecker::builtin_read(std::vector<std::shared_ptr<Node>> args)
{
    if (args.size() != 1)
    {
       error_and_exit("Built-in function 'read' expects one argument");
    }

    auto file_path_node = eval_node(args[0]);

    if (file_path_node->type != NodeType::STRING)
    {
        error_and_exit("Built-in function 'read' expects one string argument");
    }

    return make_string("", false);
}

std::shared_ptr<Node> Typechecker::builtin_write(std::vector<std::shared_ptr<Node>> args)
{
    if (args.size() != 2)
    {
       error_and_exit("Built-in function 'write' expects 2 arguments");
    }

    auto file_path_node = eval_node(args[0]);
    auto value_node = eval_node(args[1]);

    if (file_path_node->type != NodeType::STRING && value_node->type != NodeType::STRING)
    {
        error_and_exit("Built-in function 'write' expects 2 string arguments");
    }

    return make_empty(false);
}

std::shared_ptr<Node> Typechecker::builtin_append(std::vector<std::shared_ptr<Node>> args)
{

    if (args.size() != 2)
    {
       error_and_exit("Built-in function 'append' expects 2 arguments");
    }

    auto file_path_node = eval_node(args[0]);
    auto value_node = eval_node(args[1]);

    if (file_path_node->type != NodeType::STRING && value_node->type != NodeType::STRING)
    {
        error_and_exit("Built-in function 'append' expects 2 string arguments");
    }

    return make_empty(false);
}

std::shared_ptr<Node> Typechecker::builtin_time(std::vector<std::shared_ptr<Node>> args)
{
    if (args.size() != 0)
    {
        error_and_exit("Built-in function 'time' expects 0 arguments");
    }

    return make_int(0, false);
}

std::shared_ptr<Node> Typechecker::builtin_import(std::vector<std::shared_ptr<Node>> args)
{
    if (args.size() != 1)
    {
        error_and_exit("Built-in function 'import' expects 1 argument");
    }

    auto path_node = eval_node(args[0]);

    if (path_node->type != NodeType::STRING)
    {
        error_and_exit("Built-in function 'import' expects 1 string argument");
    }

    if (!path_node->TYPE.is_literal)
    {
        return make_object(false);
    }

    std::string path = path_node->STRING.value;

    Lexer lexer(path);
    lexer.builtins_path = builtins_path;
    lexer.tokenize();

    Parser parser(lexer.file_name, lexer.nodes);
    parser.parse();

    Typechecker typechecker(parser.file_name, parser.nodes);
    typechecker.runtime = runtime;
    typechecker.builtins_path = builtins_path;
    typechecker.init(typechecker.builtins_path);
    typechecker.evaluate();

    auto import_obj = make_object();

    for (auto symbol : typechecker.symbol_table)
    {
        if (std::find(builtins_names.begin(), builtins_names.end(), symbol.first) != builtins_names.end())
        {
            continue;
        }
        import_obj->OBJECT.properties[symbol.first] = Object_Prop(true, symbol.second);
    }

    import_obj->TYPE.allowed_type = import_obj;

    return import_obj;
}

std::shared_ptr<Node> Typechecker::builtin_to_int(std::vector<std::shared_ptr<Node>> args)
{
    if (args.size() != 1)
    {
       error_and_exit("Built-in function 'to_int' expects one argument");
    }

    return make_int(0, false);
}

std::shared_ptr<Node> Typechecker::builtin_to_float(std::vector<std::shared_ptr<Node>> args)
{
    if (args.size() != 1)
    {
       error_and_exit("Built-in function 'to_float' expects one argument");
    }

    return make_float(0, false);
}

std::shared_ptr<Node> Typechecker::builtin_to_string(std::vector<std::shared_ptr<Node>> args)
{
    if (args.size() != 1)
    {
       error_and_exit("Built-in function 'to_string' expects one argument");
    }

    return make_string("", false);
}

std::shared_ptr<Node> Typechecker::builtin_type(std::vector<std::shared_ptr<Node>> args)
{
    if (args.size() != 1)
    {
       error_and_exit("Built-in function 'type' expects one argument");
    }

   return make_string("", false);
}

std::shared_ptr<Node> Typechecker::builtin_shape(std::vector<std::shared_ptr<Node>> args)
{
    if (args.size() != 1)
    {
       error_and_exit("Built-in function 'shape' expects one argument");
    }

    auto value = eval_node(args[0]);
    return get_type(value);
}

std::unordered_map<std::string, std::shared_ptr<Node> (Typechecker::*)(std::vector<std::shared_ptr<Node>>)> tc_builtin_functions = {
    std::make_pair("__exit__", &Typechecker::builtin_exit),
    std::make_pair("__error__", &Typechecker::builtin_error),
    std::make_pair("__range__", &Typechecker::builtin_range),
    std::make_pair("__print__", &Typechecker::builtin_print),
    std::make_pair("__delete__", &Typechecker::builtin_delete),
    std::make_pair("__time__", &Typechecker::builtin_time),
    std::make_pair("__import__", &Typechecker::builtin_import),
    std::make_pair("__read__", &Typechecker::builtin_read),
    std::make_pair("__write__", &Typechecker::builtin_write),
    std::make_pair("__append__", &Typechecker::builtin_append),
    std::make_pair("__to_int__", &Typechecker::builtin_to_int),
    std::make_pair("__to_float__", &Typechecker::builtin_to_float),
    std::make_pair("__to_string__", &Typechecker::builtin_to_string),
    std::make_pair("__type__", &Typechecker::builtin_type),
    std::make_pair("__shape__", &Typechecker::builtin_shape),
};

void Typechecker::init(std::string builtins_path)
{
    Lexer lexer(builtins_path);
    lexer.tokenize();

    Parser parser(lexer.file_name, lexer.nodes);
    parser.parse();

    Typechecker typechecker(parser.file_name, parser.nodes);
    typechecker.runtime = runtime;
    typechecker.evaluate();

    for (auto symbol : typechecker.symbol_table)
    {
        symbol_table[symbol.first] = symbol.second;
        builtins_names.push_back(symbol.first);
    }
}

// --- HELPERS --- //

bool Typechecker::compare(const std::shared_ptr<Node> &lhs, const std::shared_ptr<Node> &rhs)
{
    auto equality_node = std::make_shared<Node>(NodeType::OP);
    equality_node->OP.op_type = NodeType::L_ANGLE;
    equality_node->left = lhs;
    equality_node->right = rhs;

    return eval_equality(equality_node)->BOOL.value;
}

// ---- //

// --- Type Checking --- //

Typechecker::Match_Result Typechecker::match_values(std::shared_ptr<Node> a, std::shared_ptr<Node> b)
{
    if (a->type == NodeType::INT)
    {
        return {a->INT.value == b->INT.value, ""};
    }
    if (a->type == NodeType::FLOAT)
    {
        return {a->FLOAT.value == b->FLOAT.value, ""};
    }
    if (a->type == NodeType::BOOL)
    {
        return {a->BOOL.value == b->BOOL.value, ""};
    }
    if (a->type == NodeType::STRING)
    {
        return {a->STRING.value == b->STRING.value, ""};
    }

    return {false, ""};
}

Typechecker::Match_Result Typechecker::match_types(std::shared_ptr<Node> a, std::shared_ptr<Node> b, bool match_name)
{
    line = b->line;
    column = b->column;

    if (a->type == NodeType::ANY || b->type == NodeType::ANY)
    {
        return {true, ""};
    }

    if (a->type == NodeType::TYPE)
    {
        return {true, ""};
    }

    if (match_name && a->type == NodeType::OBJECT && b->type == NodeType::OBJECT)
    {
        if (!a->TYPE.is_literal)
        {
            return {true, ""};
        }

        if (b->TYPE.name != "" && a->TYPE.name != b->TYPE.name)
        {
            return {false, ""};
        }
    }

    if (a->type == NodeType::FUNC_LIST)
    {
        auto func = match_function(a, {b});
        return match_types(func, b, match_name);   
    }

    if (a->type == NodeType::FUNCTION && b->type == NodeType::FUNC_LIST)
    {
        if (!a->TYPE.is_literal)
        {
            return {true, ""};
        }

        for (auto& func : b->FUNC_LIST.nodes)
        {
            if (match_types(a, func, match_name).result)
            {
                return {true, ""};
            }
        }

        return {false, "Cannot match against ambiguous function '" + b->repr()};
    }

    if (a->type == NodeType::FUNCTION && a->FUNCTION.is_type)
    {
        std::vector<int> null_arg_indices;

        if (a->FUNCTION.params.size() != 1)
        {
            for (int i = 0; i < a->FUNCTION.args.size(); i++)
            {
                if (a->FUNCTION.args[i] == nullptr)
                {
                    null_arg_indices.push_back(i);
                }
            }

            if (null_arg_indices.size() != 1)
            {
                return {false, "Refinement type function must have one parameter"};
            }
        }
        else
        {
            null_arg_indices.push_back(0);
        }

        int param_index = null_arg_indices[0];

        if (a->FUNCTION.params[param_index]->type != NodeType::ID)
        {
            error_and_exit("Refinement type function must have one basic parameter");
        }

        auto param = a->FUNCTION.params[param_index];

        if (param->ID.type == nullptr)
        {
            param->ID.type = make_any();
        }

        if (!b->TYPE.is_literal)
        {
            auto match = match_types(eval_node(param->ID.type), b, match_name);

            if (match.result)
            {
                return {true, ""};
            }

            return {false, match.message};
        }

        if (!match_types(eval_node(param->ID.type), b, match_name).result)
        {
            return {false, ""};
        }

        auto arg = eval_node(b);

        auto res = eval_function_call(a, {arg});

        std::string refinement_error_message = "Value '" + arg->repr() + "' does not match refinement type '" + a->FUNCTION.name + "'";

        if (res->BOOL.value)
        {
            return {true, ""};
        }

        if (a->FUNCTION.body->BLOCK.nodes.size() > 0 && a->FUNCTION.body->BLOCK.nodes[0]->type == NodeType::BLOCK)
        {
            auto str_block = a->FUNCTION.body->BLOCK.nodes[0];
            if (str_block->BLOCK.nodes.size() == 1)
            {
                auto message = eval_node(str_block->BLOCK.nodes[0]);

                if (message->type == NodeType::STRING)
                {
                    return {false, refinement_error_message + " - " + message->STRING.value};
                }
            }
        }

        return {false, "Value '" + b->repr() + "' does not match refinement type '" + a->FUNCTION.name + "'"};
    }

    if (a->type == NodeType::PIPE_LIST)
    {
        if (b->type == NodeType::PIPE_LIST)
        {
            sort_and_unique(a->PIPE_LIST.nodes);
            sort_and_unique(b->PIPE_LIST.nodes);

            bool match;

            for (auto& b_type : b->PIPE_LIST.nodes)
            {
                match = false;

                for (auto& a_type : a->PIPE_LIST.nodes)
                {
                    if (match_types(a_type, b_type, match_name).result)
                    {
                        if (a_type->OBJECT.is_type)
                        {
                            b_type->TYPE.name = a_type->TYPE.name;
                        }
                        match = true;
                        break;
                    }
                }
            }

            return {match, ""};
        }

        std::string message = "";

        for (auto& t : a->PIPE_LIST.nodes)
        {
            auto match = match_types(t, b, match_name);
            message = match.message;

            if (match.result)
            {
                if (t->type == NodeType::FUNCTION && t->type == NodeType::OBJECT)
                {
                    b->TYPE.name = t->TYPE.name;
                }
                return {true, ""};
            }
        }

        return {false, message};
    }

    if (a->type != b->type)
    {
        return {false, ""};
    }

    if (a->TYPE.is_literal 
    && a->type != NodeType::LIST 
    && a->type != NodeType::OBJECT
    && a->type != NodeType::FUNCTION
    && a->type != NodeType::PARTIAL_OP)
    {
        // match value too

        auto value_match = match_values(a, b);
        if (!value_match.result)
        {
            return {false, "Literal type '" + a->repr() + "' does not match value '" + b->repr() + "'"};
        }
    }

    if (a->type == NodeType::LIST)
    {
        if (!a->TYPE.is_literal && a->LIST.nodes.size() == 0)
        {
            return {true, ""};
        }

        a->TYPE.is_literal = true;

        int type_a_size = a->LIST.nodes.size();

        if (type_a_size > 1)
        {
            error_and_exit("List type cannot contain more than one type - did you mean to separate the types with the pipe operator '|'?");
        }

        if (type_a_size == 0)
        {
            a->LIST.nodes.push_back(make_any());
            return {true, ""};
        }

        auto type_b = b;
        int type_b_size = type_b->LIST.nodes.size();


        if (type_b_size == 0)
        {
            return {true, ""};
        }

        if (type_a_size > 0 && a->LIST.nodes[0]->type == NodeType::ANY)
        {
            return {true, ""};
        }

        if (type_a_size == 1 && a->LIST.nodes[0]->type == NodeType::PIPE_LIST)
        {
            if (type_b->LIST.nodes[0]->type == NodeType::PIPE_LIST)
            {
                for (auto& elem : type_b->LIST.nodes[0]->PIPE_LIST.nodes)
                {
                    auto match = match_types(a->LIST.nodes[0], elem, match_name);
                    if (!match.result)
                    {
                        return {false, match.message};
                    }
                }

                return {true, ""};
            }

            for (auto& elem : type_b->LIST.nodes)
            {
                auto match = match_types(a->LIST.nodes[0], elem, match_name);
                if (!match.result)
                {
                    return {false, match.message};
                }
            }

            return {true, ""};
        }

        if (type_a_size == 1)
        {
            for (auto& t : type_b->LIST.nodes)
            {
                auto match = match_types(a->LIST.nodes[0], t, match_name);
                if (!match.result)
                {
                    return {false, match.message};
                }
            }

            return {true, ""};
        }

        return {false, ""};
    }

    if (a->type == NodeType::OBJECT)
    {
        if (!a->TYPE.is_literal && a->OBJECT.properties.size() == 0)
        {
            return {true, ""};
        }

        a->TYPE.is_literal = true;

        if (a->OBJECT.properties.size() < b->OBJECT.properties.size())
        {
            return {false, ""};
        }

        for (auto& prop : b->OBJECT.properties)
        {
            if (a->OBJECT.properties.find(prop.first) == a->OBJECT.properties.end())
            {
                return {false, ""};
            }
        }
        for (auto& prop : a->OBJECT.properties)
        {
            if (b->OBJECT.properties.find(prop.first) == b->OBJECT.properties.end())
            {
                if (prop.second.required)
                {
                    return {false, ""};
                }

                continue;
            }

            auto a_prop = prop.second;
            auto b_prop = b->OBJECT.properties[prop.first];

            auto match = match_types(a_prop.value, b_prop.value, match_name);
            if (!match.result)
            {
                return {false, match.message};
            }
        }

        return {true, ""};
    }

    if (a->type == NodeType::FUNCTION)
    {
        if (!a->TYPE.is_literal)
        {
            return {true, ""};
        }

        if (a->FUNCTION.params.size() != b->FUNCTION.params.size())
        {
            return {false, ""};
        }

        for (int i = 0; i < a->FUNCTION.params.size(); i++)
        {
            if (a->FUNCTION.params[i]->type != b->FUNCTION.params[i]->type)
            {
                return {false, ""};
            }

            auto a_type = get_explicit_type(a->FUNCTION.params[i]);
            auto b_type = get_explicit_type(b->FUNCTION.params[i]);

            auto match = match_types(a_type, b_type, match_name);
            if (!match.result)
            {
                return {false, match.message};
            }
        }

        auto a_ret = a->FUNCTION.return_type;
        auto b_ret = b->FUNCTION.return_type;

        auto match = match_types(a_ret, b_ret, match_name);
        if (!match.result)
        {
            return {false, match.message};
        }

        return {true, ""};
    }

    return {a->type == b->type, ""};
}

// ---- //

// --- OPERATIONS --- //

std::shared_ptr<Node> Typechecker::eval_add(std::shared_ptr<Node>& node)
{
    auto left = eval_node(node->left);
    auto right = eval_node(node->right);

    auto res = std::make_shared<Node>(*node);

    if (!left->TYPE.is_literal || !right->TYPE.is_literal)
    {
        res->TYPE.is_literal = false;
    }

    if (left->type == NodeType::EMPTY)
    {
        res = right;
        return res;
    }
    if (right->type == NodeType::EMPTY)
    {
        res = left;
        return res;
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY)
    {
        res->type = NodeType::ANY;
        return res;
    }

    if (left->type == NodeType::PIPE_LIST)
    {
        auto res = make_pipe_list();

        for (auto& elem : left->PIPE_LIST.nodes)
        {
            auto op = std::make_shared<Node>(*node);
            op->left = elem;
            op->right = right;
            auto result = eval_node(op);
            if (result->type == NodeType::PIPE_LIST)
            {
                for (auto& e : result->PIPE_LIST.nodes)
                {
                    res->PIPE_LIST.nodes.push_back(e);
                }
            }
            else
            {
                res->PIPE_LIST.nodes.push_back(result);
            }
        }

        sort_and_unique(res->PIPE_LIST.nodes);

        if (res->PIPE_LIST.nodes.size() == 1)
        {
            return res->PIPE_LIST.nodes[0];
        }

        return res;
    }

    if (right->type == NodeType::PIPE_LIST)
    {
        auto res = make_pipe_list();

        for (auto& elem : right->PIPE_LIST.nodes)
        {
            auto op = std::make_shared<Node>(*node);
            op->left = left;
            op->right = elem;
            auto result = eval_node(op);
            if (result->type == NodeType::PIPE_LIST)
            {
                for (auto& e : result->PIPE_LIST.nodes)
                {
                    res->PIPE_LIST.nodes.push_back(e);
                }
            }
            else
            {
                res->PIPE_LIST.nodes.push_back(result);
            }
        }

        sort_and_unique(res->PIPE_LIST.nodes);

        if (res->PIPE_LIST.nodes.size() == 1)
        {
            return res->PIPE_LIST.nodes[0];
        }

        return res;
    }

    // if (left->type == NodeType::PIPE_LIST || right->type == NodeType::PIPE_LIST)
    // {
    //     res->type = NodeType::ANY;
    //     return res;
    // }

    if (left->type == NodeType::INT)
    {
        if (right->type == NodeType::INT)
        {
            res->type = NodeType::INT;
            res->INT.value = left->INT.value + right->INT.value;
            return res;
        }
        if (right->type == NodeType::FLOAT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->INT.value + right->FLOAT.value;
            return res;
        }
        if (right->type == NodeType::BOOL)
        {
            res->type = NodeType::INT;
            res->INT.value = left->INT.value + right->BOOL.value;
            return res;
        }
        if (right->type == NodeType::STRING)
        {
            res->type = NodeType::STRING;
            res->STRING.value = std::to_string(left->INT.value) + right->STRING.value;
            return res;
        }
        if (right->type == NodeType::LIST)
        {
            res = std::make_shared<Node>(*right);

            if (right->LIST.nodes.size() == 0)
            {
                res->LIST.nodes.insert(res->LIST.nodes.begin(), left);
            }
            else
            {
                if (res->LIST.nodes[0]->type == NodeType::PIPE_LIST)
                {
                    res->LIST.nodes[0]->PIPE_LIST.nodes.insert(res->LIST.nodes.begin(), left);
                }
                else
                {
                    res->LIST.nodes[0] = left;
                }
            }

            return res;
        }

        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::FLOAT)
    {
        if (right->type == NodeType::INT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->FLOAT.value + right->INT.value;
            return res;
        }
        if (right->type == NodeType::FLOAT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->FLOAT.value + right->FLOAT.value;
            return res;
        }
        if (right->type == NodeType::BOOL)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->FLOAT.value + right->BOOL.value;
            return res;
        }
        if (right->type == NodeType::STRING)
        {
            res->type = NodeType::STRING;
            res->STRING.value = std::to_string(left->FLOAT.value) + right->STRING.value;
            return res;
        }
        if (right->type == NodeType::LIST)
        {
            res = std::make_shared<Node>(*right);

            if (right->LIST.nodes.size() == 0)
            {
                res->LIST.nodes.insert(res->LIST.nodes.begin(), left);
            }
            else
            {
                if (res->LIST.nodes[0]->type == NodeType::PIPE_LIST)
                {
                    res->LIST.nodes[0]->PIPE_LIST.nodes.insert(res->LIST.nodes.begin(), left);
                }
                else
                {
                    res->LIST.nodes[0] = left;
                }
            }

            return res;
        }
        // if (right->type == NodeType::LIST)
        // {
        //     res->type = NodeType::LIST;
        //     res->LIST.nodes.insert(res->LIST.nodes.begin(), left);
        //     return res;
        // }

        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::BOOL)
    {
        if (right->type == NodeType::INT)
        {
            res->type = NodeType::INT;
            res->INT.value = left->BOOL.value + right->INT.value;
            return res;
        }
        if (right->type == NodeType::FLOAT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->BOOL.value + right->FLOAT.value;
            return res;
        }
        if (right->type == NodeType::BOOL)
        {
            res->type = NodeType::INT;
            res->INT.value = left->BOOL.value + right->BOOL.value;
            return res;
        }
        if (right->type == NodeType::STRING)
        {
            res->type = NodeType::STRING;
            res->STRING.value = std::to_string(left->BOOL.value) + right->STRING.value;
            return res;
        }
        if (right->type == NodeType::LIST)
        {
            res = std::make_shared<Node>(*right);

            if (right->LIST.nodes.size() == 0)
            {
                res->LIST.nodes.insert(res->LIST.nodes.begin(), left);
            }
            else
            {
                if (res->LIST.nodes[0]->type == NodeType::PIPE_LIST)
                {
                    res->LIST.nodes[0]->PIPE_LIST.nodes.insert(res->LIST.nodes.begin(), left);
                }
                else
                {
                    res->LIST.nodes[0] = left;
                }
            }

            return res;
        }
        // if (right->type == NodeType::LIST)
        // {
        //     res->type = NodeType::LIST;
        //     res->LIST.nodes.insert(res->LIST.nodes.begin(), left);
        //     return res;
        // }

        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::STRING)
    {
        if (right->type == NodeType::INT)
        {
            res->type = NodeType::STRING;
            res->STRING.value = left->STRING.value + std::to_string(right->INT.value);
            return res;
        }
        if (right->type == NodeType::FLOAT)
        {
            res->type = NodeType::STRING;
            res->STRING.value = left->STRING.value + std::to_string(right->FLOAT.value);
            return res;
        }
        if (right->type == NodeType::BOOL)
        {
            res->type = NodeType::STRING;
            res->STRING.value = left->STRING.value + std::to_string(right->BOOL.value);
            return res;
        }
        if (right->type == NodeType::STRING)
        {
            res->type = NodeType::STRING;
            res->STRING.value = left->STRING.value + right->STRING.value;
            return res;
        }
        if (right->type == NodeType::LIST)
        {
            res = std::make_shared<Node>(*right);

            if (right->LIST.nodes.size() == 0)
            {
                res->LIST.nodes.insert(res->LIST.nodes.begin(), left);
            }
            else
            {
                if (res->LIST.nodes[0]->type == NodeType::PIPE_LIST)
                {
                    res->LIST.nodes[0]->PIPE_LIST.nodes.insert(res->LIST.nodes.begin(), left);
                }
                else
                {
                    res->LIST.nodes[0] = left;
                }
            }

            return res;
        }
        // if (right->type == NodeType::LIST)
        // {
        //     res->type = NodeType::LIST;
        //     res->LIST.nodes.insert(res->LIST.nodes.begin(), res->left);
        //     return res;
        // }

        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::LIST)
    {
        if (right->type == NodeType::LIST)
        {
            res->type = NodeType::LIST;
            res->LIST.nodes = left->LIST.nodes;

            for (auto _node : right->LIST.nodes)
            {
                res->LIST.nodes.push_back(_node);
            }

            return res;
        }
        else
        {
            res = std::make_shared<Node>(*left);

            if (res->LIST.nodes.size() == 0)
            {
                res->LIST.nodes.insert(res->LIST.nodes.begin(), right);
            }
            else
            {
                if (res->LIST.nodes[0]->type == NodeType::PIPE_LIST)
                {
                    res->LIST.nodes[0]->PIPE_LIST.nodes.insert(res->LIST.nodes.begin(), right);
                }
                else
                {
                    res->LIST.nodes[0] = right;
                }
            }

            return res;
        }

        // res->type = NodeType::LIST;
        // res->LIST.nodes = left->LIST.nodes;
        // res->LIST.nodes.push_back(right);
        // return res;
    }

    if (left->type == NodeType::OBJECT)
    {
        if (right->type == NodeType::OBJECT)
        {
            res->type = NodeType::OBJECT;
            res->OBJECT.properties = left->OBJECT.properties;

            for (auto prop : right->OBJECT.properties)
            {
                res->OBJECT.properties[prop.first] = prop.second;
            }

            return res;
        }

        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
}

std::shared_ptr<Node> Typechecker::eval_sub(std::shared_ptr<Node>& node)
{
    auto left = eval_node(node->left);
    auto right = eval_node(node->right);

    auto res = std::make_shared<Node>(*node);

    if (!left->TYPE.is_literal || !right->TYPE.is_literal)
    {
        res->TYPE.is_literal = false;
    }

    if (left->type == NodeType::EMPTY)
    {
        res = right;
        return res;
    }
    if (right->type == NodeType::EMPTY)
    {
        res = left;
        return res;
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY)
    {
        res->type = NodeType::ANY;
        return res;
    }

    if (left->type == NodeType::PIPE_LIST || right->type == NodeType::PIPE_LIST)
    {
        res->type = NodeType::ANY;
        return res;
    }

    if (left->type == NodeType::INT)
    {
        if (right->type == NodeType::INT)
        {
            res->type = NodeType::INT;
            res->INT.value = left->INT.value - right->INT.value;
            return res;
        }
        if (right->type == NodeType::FLOAT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->INT.value - right->FLOAT.value;
            return res;
        }
        if (right->type == NodeType::BOOL)
        {
            res->type = NodeType::INT;
            res->INT.value = left->INT.value - right->BOOL.value;
            return res;
        }

        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::FLOAT)
    {
        if (right->type == NodeType::INT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->FLOAT.value - right->INT.value;
            return res;
        }
        if (right->type == NodeType::FLOAT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->FLOAT.value - right->FLOAT.value;
            return res;
        }
        if (right->type == NodeType::BOOL)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->FLOAT.value - right->BOOL.value;
            return res;
        }

        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::BOOL)
    {
        if (right->type == NodeType::INT)
        {
            res->type = NodeType::INT;
            res->INT.value = left->BOOL.value - right->INT.value;
            return res;
        }
        if (right->type == NodeType::FLOAT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->BOOL.value - right->FLOAT.value;
            return res;
        }
        if (right->type == NodeType::BOOL)
        {
            res->type = NodeType::INT;
            res->INT.value = left->BOOL.value - right->BOOL.value;
            return res;
        }

        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::STRING)
    {
        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
}

std::shared_ptr<Node> Typechecker::eval_mul(std::shared_ptr<Node>& node)
{
    auto left = eval_node(node->left);
    auto right = eval_node(node->right);

    auto res = std::make_shared<Node>(*node);

    if (!left->TYPE.is_literal || !right->TYPE.is_literal)
    {
        res->TYPE.is_literal = false;
    }

    if (left->type == NodeType::EMPTY)
    {
        res = right;
        return res;
    }
    if (right->type == NodeType::EMPTY)
    {
        res = left;
        return res;
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY)
    {
        res->type = NodeType::ANY;
        return res;
    }

    if (left->type == NodeType::PIPE_LIST || right->type == NodeType::PIPE_LIST)
    {
        res->type = NodeType::ANY;
        return res;
    }

    if (left->type == NodeType::INT)
    {
        if (right->type == NodeType::INT)
        {
            res->type = NodeType::INT;
            res->INT.value = left->INT.value * right->INT.value;
            return res;
        }
        if (right->type == NodeType::FLOAT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->INT.value * right->FLOAT.value;
            return res;
        }
        if (right->type == NodeType::BOOL)
        {
            res->type = NodeType::INT;
            res->INT.value = left->INT.value * right->BOOL.value;
            return res;
        }
        if (right->type == NodeType::STRING)
        {
            res->type = NodeType::STRING;
            for (int i = 0; i < left->INT.value; i++)
            {
                res->STRING.value += right->STRING.value;
            }
            return res;
        }
        if (right->type == NodeType::LIST)
        {
            res->type = NodeType::LIST;
            for (int i = 0; i < left->INT.value; i++)
            {
                res->LIST.nodes.push_back(eval_copy(right));
            }
            return res;
        }

        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::FLOAT)
    {
        if (right->type == NodeType::INT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->FLOAT.value * right->INT.value;
            return res;
        }
        if (right->type == NodeType::FLOAT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->FLOAT.value * right->FLOAT.value;
            return res;
        }
        if (right->type == NodeType::BOOL)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->FLOAT.value * right->BOOL.value;
            return res;
        }

        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::BOOL)
    {
        if (right->type == NodeType::INT)
        {
            res->type = NodeType::INT;
            res->INT.value = left->BOOL.value * right->INT.value;
            return res;
        }
        if (right->type == NodeType::FLOAT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->BOOL.value * right->FLOAT.value;
            return res;
        }
        if (right->type == NodeType::BOOL)
        {
            res->type = NodeType::INT;
            res->INT.value = left->BOOL.value * right->BOOL.value;
            return res;
        }
        if (right->type == NodeType::STRING)
        {
            res->type = NodeType::STRING;
            for (int i = 0; i < left->BOOL.value; i++)
            {
                res->STRING.value += right->STRING.value;
            }
            return res;
        }
        if (right->type == NodeType::LIST)
        {
            res->type = NodeType::LIST;
            for (int i = 0; i < left->BOOL.value; i++)
            {
                res->LIST.nodes.push_back(eval_copy(right));
            }
            return res;
        }

        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::STRING)
    {
        if (right->type == NodeType::INT)
        {
            res->type = NodeType::STRING;
            for (int i = 0; i < right->INT.value; i++)
            {
                res->STRING.value += left->STRING.value;
            }
            return res;
        }
        if (right->type == NodeType::BOOL)
        {
            res->type = NodeType::STRING;
            for (int i = 0; i < right->BOOL.value; i++)
            {
                res->STRING.value += left->STRING.value;
            }
            return res;
        }

        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::LIST)
    {
        if (right->type == NodeType::INT)
        {
            res->type = NodeType::LIST;
            for (int i = 0; i < right->INT.value; i++)
            {
                res->LIST.nodes.push_back(eval_copy(left));
            }
            return res;
        }

        if (right->type == NodeType::BOOL)
        {
            res->type = NodeType::LIST;
            for (int i = 0; i < right->BOOL.value; i++)
            {
                res->LIST.nodes.push_back(eval_copy(left));
            }
            return res;
        }

        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::OBJECT)
    {
        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
}

std::shared_ptr<Node> Typechecker::eval_div(std::shared_ptr<Node>& node)
{
    auto left = eval_node(node->left);
    auto right = eval_node(node->right);

    auto res = std::make_shared<Node>(*node);

    if (!left->TYPE.is_literal || !right->TYPE.is_literal)
    {
        res->TYPE.is_literal = false;
    }

    if (left->type == NodeType::EMPTY)
    {
        res = right;
        return res;
    }
    if (right->type == NodeType::EMPTY)
    {
        res = left;
        return res;
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY)
    {
        res->type = NodeType::ANY;
        return res;
    }

    if (left->type == NodeType::PIPE_LIST || right->type == NodeType::PIPE_LIST)
    {
        res->type = NodeType::ANY;
        return res;
    }

    if (left->type == NodeType::INT)
    {
        if (right->type == NodeType::INT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = (double)left->INT.value / (double)right->INT.value;
            return res;
        }
        if (right->type == NodeType::FLOAT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = (double)left->INT.value / right->FLOAT.value;
            return res;
        }
        if (right->type == NodeType::BOOL)
        {
            res->type = NodeType::INT;
            res->INT.value = left->INT.value / right->BOOL.value;
            return res;
        }

        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::FLOAT)
    {
        if (right->type == NodeType::INT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->FLOAT.value / (double)right->INT.value;
            return res;
        }
        if (right->type == NodeType::FLOAT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->FLOAT.value / right->FLOAT.value;
            return res;
        }
        if (right->type == NodeType::BOOL)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = left->FLOAT.value / (double)right->BOOL.value;
            return res;
        }

        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::BOOL)
    {
        if (right->type == NodeType::INT)
        {
            res->type = NodeType::INT;
            res->INT.value = left->BOOL.value / right->INT.value;
            return res;
        }
        if (right->type == NodeType::FLOAT)
        {
            res->type = NodeType::FLOAT;
            res->FLOAT.value = (double)left->BOOL.value / right->FLOAT.value;
            return res;
        }
        if (right->type == NodeType::BOOL)
        {
            res->type = NodeType::INT;
            res->INT.value = left->BOOL.value / right->BOOL.value;
            return res;
        }

        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::STRING)
    {
        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::LIST)
    {
        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    if (left->type == NodeType::OBJECT)
    {
        error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
    }

    error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
}

std::shared_ptr<Node> Typechecker::eval_mod(std::shared_ptr<Node>& node)
{
    auto left = eval_node(node->left);
    auto right = eval_node(node->right);

    auto res = std::make_shared<Node>(*node);

    if (!left->TYPE.is_literal || !right->TYPE.is_literal)
    {
        res->TYPE.is_literal = false;
    }

    if (left->type == NodeType::EMPTY)
    {
        res = right;
        return res;
    }
    if (right->type == NodeType::EMPTY)
    {
        res = left;
        return res;
    }

    if (left->type == NodeType::ANY || right->type == NodeType::ANY)
    {
        res->type = NodeType::ANY;
        return res;
    }

    if (left->type == NodeType::INT && right->type == NodeType::INT)
    {
        res->type = NodeType::INT;
        res->INT.value = left->INT.value % right->INT.value;
        return res;
    }

    if (left->type == NodeType::INT && right->type == NodeType::FLOAT)
    {
        res->type = NodeType::FLOAT;
        res->FLOAT.value = fmod(left->INT.value, right->FLOAT.value);
        return res;
    }

    if (left->type == NodeType::FLOAT && right->type == NodeType::INT)
    {
        res->type = NodeType::FLOAT;
        res->FLOAT.value = fmod(left->FLOAT.value, right->INT.value);
        return res;
    }

    if (left->type == NodeType::FLOAT && right->type == NodeType::FLOAT)
    {
        res->type = NodeType::FLOAT;
        res->FLOAT.value = fmod(left->FLOAT.value, right->FLOAT.value);
        return res;
    }

    error_and_exit("Cannot perform '" + node->repr() + "' on " + left->repr() + " and " + right->repr());
}

std::shared_ptr<Node> Typechecker::eval_pos_neg(std::shared_ptr<Node>& node)
{
    auto right = eval_node(node->right);

    if (!right->TYPE.is_literal)
    {
        return right;
    }

    auto res = std::make_shared<Node>(*node);

    if (right->type == NodeType::EMPTY)
    {
        res = right;
        return res;
    }

    if (right->type == NodeType::ANY)
    {
        res->type = NodeType::ANY;
        return res;
    }

    if (right->type == NodeType::INT)
    {
        res->type = NodeType::INT;
        if (res->OP.op_type == NodeType::POS)
        {
            res->INT.value = +(right->INT.value);
        }
        else if (res->OP.op_type == NodeType::NEG)
        {
            res->INT.value = -(right->INT.value);
        } 
        return res;
    }

    if (right->type == NodeType::FLOAT)
    {
        res->type = NodeType::FLOAT;
        if (res->OP.op_type == NodeType::POS)
        {
            res->FLOAT.value = +(right->FLOAT.value);
        }
        else if (res->OP.op_type == NodeType::NEG)
        {
            res->FLOAT.value = -(right->FLOAT.value);
        } 
        return res;
    }
    
    if (right->type == NodeType::BOOL)
    {
        res->type = NodeType::BOOL;
        if (res->OP.op_type == NodeType::POS)
        {
            res->BOOL.value = +(right->BOOL.value);
        }
        else if (res->OP.op_type == NodeType::NEG)
        {
            res->BOOL.value = -(right->BOOL.value);
        } 
        return res;
    }

    if (right->type == NodeType::STRING)
    {
        res->type = NodeType::STRING;
        if (res->OP.op_type == NodeType::POS)
        {
            res->STRING.value = right->STRING.value;
        }
        else if (res->OP.op_type == NodeType::NEG)
        {
            std::string copy(right->STRING.value);
            std::reverse(copy.begin(), copy.end());
            res->STRING.value = copy;
        } 
        return res;
    }

    if (right->type == NodeType::LIST)
    {
        res->type = NodeType::LIST;
        if (res->OP.op_type == NodeType::POS)
        {
            res->LIST.nodes = right->LIST.nodes;
        }
        else if (res->OP.op_type == NodeType::NEG)
        {
            std::vector<std::shared_ptr<Node>> copy(right->LIST.nodes);
            std::reverse(copy.begin(), copy.end());
            res->LIST.nodes = copy;
        } 
        return res;
    }

    return right;
}

std::shared_ptr<Node> Typechecker::eval_not(std::shared_ptr<Node>& node)
{
    auto right = eval_node(node->right);

    if (!right->TYPE.is_literal)
    {
        return right;
    }

    auto res = std::make_shared<Node>(*node);

    if (right->type == NodeType::EMPTY)
    {
        res = right;
        return res;
    }

    if (right->type == NodeType::ANY)
    {
        res->type = NodeType::ANY;
        return res;
    }

    if (right->type == NodeType::INT)
    {
        res->type = NodeType::BOOL;
        res->BOOL.value = !(right->INT.value);
        return res;
    }

    if (right->type == NodeType::FLOAT)
    {
        res->type = NodeType::BOOL;
        res->BOOL.value = !(right->FLOAT.value);
        return res;
    }
    
    if (right->type == NodeType::BOOL)
    {
        res->type = NodeType::BOOL;
        res->BOOL.value = !(right->BOOL.value);
        return res;
    }

    res->type = NodeType::BOOL;
    res->BOOL.value = false;
    return res;
}

std::shared_ptr<Node> Typechecker::eval_equality(std::shared_ptr<Node>& node)
{
    auto left = eval_node(node->left);
    auto right = eval_node(node->right);

    if (!left->TYPE.is_literal || !right->TYPE.is_literal)
    {
        return make_bool(true, false);
    }

    auto res = std::make_shared<Node>(*node);
    res->type = NodeType::BOOL;

    if (left->type == NodeType::ANY || right->type == NodeType::ANY)
    {
        return make_bool(true, true);
    }

    if (res->OP.op_type == NodeType::L_ANGLE)
    {
        if (left->type == NodeType::BOOL && right->type == NodeType::BOOL)
        {
            res->BOOL.value = left->BOOL.value < right->BOOL.value;
        }
        else if (left->type == NodeType::INT && right->type == NodeType::INT)
        {
            res->BOOL.value = left->INT.value < right->INT.value;
        }
        else if (left->type == NodeType::INT && right->type == NodeType::FLOAT)
        {
            res->BOOL.value = left->INT.value < right->FLOAT.value;
        }
        else if (left->type == NodeType::FLOAT && right->type == NodeType::INT)
        {
            res->BOOL.value = left->FLOAT.value < right->INT.value;
        }
        else if (left->type == NodeType::FLOAT && right->type == NodeType::FLOAT)
        {
            res->BOOL.value = left->FLOAT.value < right->FLOAT.value;
        }
        else if (left->type == NodeType::STRING && right->type == NodeType::STRING)
        {
            res->BOOL.value = left->STRING.value < right->STRING.value;
        }
        else
        {
            res->BOOL.value = false;
        }

        return res;
    }

    if (res->OP.op_type == NodeType::R_ANGLE)
    {
        if (left->type == NodeType::BOOL && right->type == NodeType::BOOL)
        {
            res->BOOL.value = left->BOOL.value > right->BOOL.value;
        }
        else if (left->type == NodeType::INT && right->type == NodeType::INT)
        {
            res->BOOL.value = left->INT.value > right->INT.value;
        }
        else if (left->type == NodeType::INT && right->type == NodeType::FLOAT)
        {
            res->BOOL.value = left->INT.value > right->FLOAT.value;
        }
        else if (left->type == NodeType::FLOAT && right->type == NodeType::INT)
        {
            res->BOOL.value = left->FLOAT.value > right->INT.value;
        }
        else if (left->type == NodeType::FLOAT && right->type == NodeType::FLOAT)
        {
            res->BOOL.value = left->FLOAT.value > right->FLOAT.value;
        }
        else if (left->type == NodeType::STRING && right->type == NodeType::STRING)
        {
            res->BOOL.value = left->STRING.value > right->STRING.value;
        }
        else
        {
            res->BOOL.value = false;
        }

        return res;
    }

    if (res->OP.op_type == NodeType::LT_EQUAL)
    {
        if (left->type == NodeType::BOOL && right->type == NodeType::BOOL)
        {
            res->BOOL.value = left->BOOL.value <= right->BOOL.value;
        }
        else if (left->type == NodeType::INT && right->type == NodeType::INT)
        {
            res->BOOL.value = left->INT.value <= right->INT.value;
        }
        else if (left->type == NodeType::INT && right->type == NodeType::FLOAT)
        {
            res->BOOL.value = left->INT.value <= right->FLOAT.value;
        }
        else if (left->type == NodeType::FLOAT && right->type == NodeType::INT)
        {
            res->BOOL.value = left->FLOAT.value <= right->INT.value;
        }
        else if (left->type == NodeType::FLOAT && right->type == NodeType::FLOAT)
        {
            res->BOOL.value = left->FLOAT.value <= right->FLOAT.value;
        }
        else if (left->type == NodeType::STRING && right->type == NodeType::STRING)
        {
            res->BOOL.value = left->STRING.value <= right->STRING.value;
        }
        else
        {
            res->BOOL.value = false;
        }

        return res;
    }

    if (res->OP.op_type == NodeType::GT_EQUAL)
    {
        if (left->type == NodeType::BOOL && right->type == NodeType::BOOL)
        {
            res->BOOL.value = left->BOOL.value >= right->BOOL.value;
        }
        else if (left->type == NodeType::INT && right->type == NodeType::INT)
        {
            res->BOOL.value = left->INT.value >= right->INT.value;
        }
        else if (left->type == NodeType::INT && right->type == NodeType::FLOAT)
        {
            res->BOOL.value = left->INT.value >= right->FLOAT.value;
        }
        else if (left->type == NodeType::FLOAT && right->type == NodeType::INT)
        {
            res->BOOL.value = left->FLOAT.value >= right->INT.value;
        }
        else if (left->type == NodeType::FLOAT && right->type == NodeType::FLOAT)
        {
            res->BOOL.value = left->FLOAT.value >= right->FLOAT.value;
        }
        else if (left->type == NodeType::STRING && right->type == NodeType::STRING)
        {
            res->BOOL.value = left->STRING.value >= right->STRING.value;
        }
        else
        {
            res->BOOL.value = false;
        }

        return res;
    }
    if (res->OP.op_type == NodeType::EQ_EQ)
    {
        if (!left->TYPE.is_literal || !right->TYPE.is_literal)
        {
            res->BOOL.value = left->type == right->type;
            return res;
        }
        if (left->type == NodeType::EMPTY && right->type == NodeType::EMPTY)
        {
            res->BOOL.value = true;
        }
        else if (left->type == NodeType::BOOL && right->type == NodeType::BOOL)
        {
            res->BOOL.value = left->BOOL.value == right->BOOL.value;
        }
        else if (left->type == NodeType::INT && right->type == NodeType::INT)
        {
            res->BOOL.value = left->INT.value == right->INT.value;
        }
        else if (left->type == NodeType::INT && right->type == NodeType::FLOAT)
        {
            res->BOOL.value = left->INT.value == right->FLOAT.value;
        }
        else if (left->type == NodeType::FLOAT && right->type == NodeType::INT)
        {
            res->BOOL.value = left->FLOAT.value == right->INT.value;
        }
        else if (left->type == NodeType::FLOAT && right->type == NodeType::FLOAT)
        {
            res->BOOL.value = left->FLOAT.value == right->FLOAT.value;
        }
        else if (left->type == NodeType::STRING && right->type == NodeType::STRING)
        {
            res->BOOL.value = left->STRING.value == right->STRING.value;
        }
        else if (left->type == NodeType::LIST && right->type == NodeType::LIST)
        {
            if (left->LIST.nodes.size() != right->LIST.nodes.size())
            {
                res->BOOL.value = false;
                return res;
            }

            for (int i = 0; i < left->LIST.nodes.size(); i++)
            {
                auto equality_node = std::make_shared<Node>(NodeType::OP);
                equality_node->OP.op_type = NodeType::EQ_EQ;
                equality_node->left = left->LIST.nodes[i];
                equality_node->right = right->LIST.nodes[i];

                auto result = eval_equality(equality_node);
                if (result->BOOL.value == false)
                {
                    res->BOOL.value = false;
                    return res;
                }
            }

            res->BOOL.value = true;
            return res;
        }
        else
        {
            res->BOOL.value = false;
        }

        return res;
    }
    if (res->OP.op_type == NodeType::NOT_EQUAL)
    {
        if (left->type == NodeType::EMPTY && right->type == NodeType::EMPTY)
        {
            res->BOOL.value = false;
        }
        else if (left->type == NodeType::BOOL && right->type == NodeType::BOOL)
        {
            res->BOOL.value = left->BOOL.value != right->BOOL.value;
        }
        else if (left->type == NodeType::INT && right->type == NodeType::INT)
        {
            res->BOOL.value = left->INT.value != right->INT.value;
        }
        else if (left->type == NodeType::INT && right->type == NodeType::FLOAT)
        {
            res->BOOL.value = left->INT.value != right->FLOAT.value;
        }
        else if (left->type == NodeType::FLOAT && right->type == NodeType::INT)
        {
            res->BOOL.value = left->FLOAT.value != right->INT.value;
        }
        else if (left->type == NodeType::FLOAT && right->type == NodeType::FLOAT)
        {
            res->BOOL.value = left->FLOAT.value != right->FLOAT.value;
        }
        else if (left->type == NodeType::STRING && right->type == NodeType::STRING)
        {
            res->BOOL.value = left->STRING.value != right->STRING.value;
        }
        else
        {
            res->BOOL.value = true;
        }

        return res;
    }

    res->BOOL.value = false;
    return res;
}

std::shared_ptr<Node> Typechecker::eval_and_or(std::shared_ptr<Node>& node)
{
    auto left = eval_node(node->left);
    
    if (left->type == NodeType::BOOL && !left->BOOL.value && node->OP.op_type == NodeType::AND)
    {
        auto res = std::make_shared<Node>(*left);
        return res;
    }

    auto right = eval_node(node->right);
    auto res = std::make_shared<Node>(*node);

    if (!left->TYPE.is_literal || !right->TYPE.is_literal)
    {
        res->TYPE.is_literal = false;
        return left;
    }

    if (left->type == NodeType::BOOL && right->type == NodeType::BOOL)
    {
        res->type = NodeType::BOOL;
        if (res->OP.op_type == NodeType::AND)
        {
            res->BOOL.value = left->BOOL.value && right->BOOL.value;
        }
        else if (res->OP.op_type == NodeType::OR)
        {
            res->BOOL.value = left->BOOL.value || right->BOOL.value;
        }
        return res;
    }

    res->type = NodeType::BOOL;
    res->BOOL.value = false;
    return res;
}

std::shared_ptr<Node> Typechecker::eval_copy(std::shared_ptr<Node>& node)
{
    // node is the right operand of #
    // we set it as the node beforehand so we can recurse

    auto value = eval_node(node);
    auto res = std::make_shared<Node>(*value);

    if (!value->TYPE.is_literal)
    {
        return value;
    }

    if (res->type == NodeType::LIST)
    {
        for (int i = 0; i < value->LIST.nodes.size(); i++)
        {
            res->LIST.nodes[i] = eval_copy(value->LIST.nodes[i]);
        }
    }

    if (res->type == NodeType::OBJECT)
    {
        for (auto prop : value->OBJECT.properties)
        {
            res->OBJECT.properties[prop.first] = Object_Prop(prop.second.required, eval_copy(prop.second.value));
        }
    }

    return res;
}

std::shared_ptr<Node> Typechecker::eval_dot(std::shared_ptr<Node>& node)
{
    if (node->left->type == NodeType::FUNC_CALL)
    {
        if (node->right->type == NodeType::LIST)
        {
            auto l = eval_node(node->left);
            return eval_function_call(l, eval_node(node->right)->LIST.nodes);
        }
    }

    auto left = eval_node(node->left);
    auto right = node->right;

    if (left->type == NodeType::ANY)
    {
        return make_any();
    }

    if (left->type == NodeType::PIPE_LIST)
    {
        auto res = make_pipe_list();

        for (auto& elem : left->PIPE_LIST.nodes)
        {
            auto dot = std::make_shared<Node>(NodeType::DOT);
            dot->left = elem;
            dot->right = right;
            auto result = eval_dot(dot);
            if (result->type == NodeType::PIPE_LIST)
            {
                for (auto& e : result->PIPE_LIST.nodes)
                {
                    res->PIPE_LIST.nodes.push_back(e);
                }
            }
            else
            {
                res->PIPE_LIST.nodes.push_back(result);
            }
        }

        sort_and_unique(res->PIPE_LIST.nodes);

        if (res->PIPE_LIST.nodes.size() == 1)
        {
            return res->PIPE_LIST.nodes[0];
        }

        return res;
    }

    if (right->type == NodeType::PIPE_LIST)
    {
        auto res = make_pipe_list();

        for (auto& elem : right->PIPE_LIST.nodes)
        {
            auto dot = std::make_shared<Node>(NodeType::DOT);
            dot->left = left;
            dot->right = elem;
            auto result = eval_dot(dot);
            if (result->type == NodeType::PIPE_LIST)
            {
                for (auto& e : result->PIPE_LIST.nodes)
                {
                    res->PIPE_LIST.nodes.push_back(e);
                }
            }
            else
            {
                res->PIPE_LIST.nodes.push_back(result);
            }
        }

        sort_and_unique(res->PIPE_LIST.nodes);

        if (res->PIPE_LIST.nodes.size() == 1)
        {
            return res->PIPE_LIST.nodes[0];
        }

        return res;
    }

    // List

    if (left->type == NodeType::LIST)
    {
        // element access

        if (right->type == NodeType::LIST)
        {
            if (right->LIST.nodes.size() != 1)
            {
                error_and_exit("List accessor expects one value");
            }
            
            auto accessor = eval_node(right->LIST.nodes[0]);

            if (accessor->type != NodeType::INT)
            {
                error_and_exit("List accessor expects an integer");
            }

            return left->TYPE.allowed_type->LIST.nodes[0];
        }

        // List built-ins

        if (right->type == NodeType::FUNC_CALL)
        {
            if (right->FUNC_CALL.name == "flatten")
            {
                if (right->FUNC_CALL.args.size() != 0 && right->FUNC_CALL.args.size() != 1)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects 0 or 1 arguments");
                }

                bool all = false;

                if (right->FUNC_CALL.args.size() == 1)
                {
                    auto bool_node = eval_node(right->FUNC_CALL.args[0]);
                    if (bool_node->type != NodeType::BOOL)
                    {
                        error_and_exit("List built-in '" + right->FUNC_CALL.name + "' a boolean argument");
                    }

                    all = bool_node->BOOL.value;
                }

                return left;
            }
            if (right->FUNC_CALL.name == "sort")
            {
                if (right->FUNC_CALL.args.size() != 0 && right->FUNC_CALL.args.size() != 1)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects 0 or 1 arguments");
                }

                if (right->FUNC_CALL.args.size() == 0)
                {
                    return left;
                }

                auto func = eval_node(right->FUNC_CALL.args[0]);

                if (func->type != NodeType::FUNCTION)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects argument to be a function");
                }

                return left;
            }
            if (right->FUNC_CALL.name == "map")
            {
                auto res = make_list();

                if (right->FUNC_CALL.args.size() != 1)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects 1 argument");
                }

                auto func = eval_node(right->FUNC_CALL.args[0]);

                if (func->type != NodeType::FUNCTION && func->type != NodeType::PARTIAL_OP)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects argument to be a function or a partial operator");
                }

                if (func->type == NodeType::PARTIAL_OP)
                {
                    auto arrow_node = std::make_shared<Node>(NodeType::OP);
                    arrow_node->OP.op_type = NodeType::DOUBLE_ARROW;
                    arrow_node->left = left->LIST.nodes[0];
                    arrow_node->right = std::make_shared<Node>(*func);
                    res->LIST.nodes.push_back(eval_double_arrow(arrow_node));

                    return res;
                }

                if (func->TYPE.is_literal)
                {
                    std::vector<std::shared_ptr<Node>> args;

                    if (func->FUNCTION.params.size() == 0 || func->FUNCTION.params.size() > 3)
                    {
                        error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects function argument to have 1, 2 or 3 parameters");
                    }

                    auto elem_type = eval_node(left->LIST.nodes[0]);

                    func->FUNCTION.params[0]->ID.type = elem_type;

                    if (func->FUNCTION.params.size() == 1)
                    {
                        args = {elem_type};
                    }
                    else if (func->FUNCTION.params.size() == 2)
                    {
                        auto index_node = std::make_shared<Node>(NodeType::INT);
                        index_node->INT.value = 0;
                        args = {elem_type, index_node};

                        func->FUNCTION.params[1]->ID.type = make_int(0, false);
                    }
                    else if (func->FUNCTION.params.size() == 3)
                    {
                        auto index_node = std::make_shared<Node>(NodeType::INT);
                        index_node->INT.value = 0;
                        args = {elem_type, index_node, left};

                        func->FUNCTION.params[1]->ID.type = make_int(0, false);
                        func->FUNCTION.params[2]->ID.type = left;
                    }

                    type_func(func);

                    res->LIST.nodes.push_back(eval_function_call(func, args));
                }
                else
                {
                    res->LIST.nodes.push_back(make_any());
                }

                res->TYPE.allowed_type = res;

                return res;
            }
            if (right->FUNC_CALL.name == "flatmap")
            {
                if (right->FUNC_CALL.args.size() != 1)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects 1 argument");
                }

                auto func = eval_node(right->FUNC_CALL.args[0]);

                if (func->type != NodeType::FUNCTION && func->type != NodeType::PARTIAL_OP)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects argument to be a function or a partial operator");
                }

                auto res = make_list(left->TYPE.is_literal);

                if (func->type == NodeType::PARTIAL_OP)
                {
                    auto arrow_node = std::make_shared<Node>(NodeType::OP);
                    arrow_node->OP.op_type = NodeType::DOUBLE_ARROW;
                    arrow_node->left = left->TYPE.allowed_type->LIST.nodes[0];
                    arrow_node->right = std::make_shared<Node>(*func);
                    auto result = eval_double_arrow(arrow_node);

                    if (result->type == NodeType::LIST)
                    {
                        res->LIST.nodes.insert(res->LIST.nodes.end(), result->LIST.nodes.begin(), result->LIST.nodes.end());
                    }
                    else
                    {
                        res->LIST.nodes.push_back(result);
                    }

                    res->TYPE.allowed_type = res;

                    return res;
                }

                if (func->TYPE.is_literal)
                {
                    std::vector<std::shared_ptr<Node>> args;

                    if (func->FUNCTION.params.size() == 1)
                    {
                        args = {eval_node(left->LIST.nodes[0])};
                    }
                    else if (func->FUNCTION.params.size() == 2)
                    {
                        auto index_node = std::make_shared<Node>(NodeType::INT);
                        index_node->INT.value = 0;
                        args = {eval_node(left->LIST.nodes[0]), index_node};
                    }
                    else if (func->FUNCTION.params.size() == 3)
                    {
                        auto index_node = std::make_shared<Node>(NodeType::INT);
                        index_node->INT.value = 0;
                        args = {eval_node(left->LIST.nodes[0]), index_node, left};
                    }
                    else
                    {
                        error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects function argument to have 1, 2 or 3 parameters");
                    }

                    auto result = eval_function_call(func, args);

                    if (result->type == NodeType::LIST)
                    {
                        res->LIST.nodes.insert(res->LIST.nodes.end(), result->LIST.nodes.begin(), result->LIST.nodes.end());
                    }
                    else
                    {
                        res->LIST.nodes.push_back(result);
                    }
                }
                else
                {
                    res->LIST.nodes.push_back(make_any());
                }

                res->TYPE.allowed_type = res;

                return res;
            }
            if (right->FUNC_CALL.name == "filter")
            {
                if (right->FUNC_CALL.args.size() != 1)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects 1 argument");
                }

                auto func = eval_node(right->FUNC_CALL.args[0]);

                if (func->type != NodeType::FUNCTION && func->type != NodeType::PARTIAL_OP)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects argument to be a function or a partial operator");
                }

                return left;
            }
            if (right->FUNC_CALL.name == "foreach")
            {
                if (right->FUNC_CALL.args.size() != 1)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects 1 argument");
                }

                auto func = eval_node(right->FUNC_CALL.args[0]);

                if (func->type != NodeType::FUNCTION)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects argument to be a function");
                }

                auto res = make_list(left->TYPE.is_literal);

                if (func->TYPE.is_literal)
                {
                    if (func->FUNCTION.params.size() > 3)
                    {
                        error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects function argument to have 1, 2 or 3 parameters");
                    }
                }

                return left;
            }
            if (right->FUNC_CALL.name == "append")
            {
                if (right->FUNC_CALL.args.size() != 1)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects 1 argument");
                }

                auto value = eval_node(right->FUNC_CALL.args[0]);

                std::shared_ptr<Node> elem_type;

                if (left->TYPE.allowed_type == nullptr)
                {
                    left->TYPE.allowed_type = make_list();
                    left->TYPE.allowed_type->LIST.nodes.push_back(make_any());
                }

                if (left->TYPE.allowed_type->type == NodeType::ANY)
                {
                    elem_type = left->TYPE.allowed_type;
                }
                else
                {
                    elem_type = left->TYPE.allowed_type->LIST.nodes[0];
                }

                auto match = match_types(elem_type, value);

                if (!match.result)
                {
                    error_and_exit("Cannot append element of type '" + value->type_repr() + "' to list of type " + left->TYPE.allowed_type->type_repr());
                }

                return left;
            }
            if (right->FUNC_CALL.name == "prepend")
            {
                if (right->FUNC_CALL.args.size() != 1)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects 1 argument");
                }

                if (typecheck)
                {
                    left->LIST.nodes.clear();
                }

                auto value = eval_node(right->FUNC_CALL.args[0]);

                std::shared_ptr<Node> elem_type;

                if (left->TYPE.allowed_type == nullptr)
                {
                    left->TYPE.allowed_type = make_list();
                    left->TYPE.allowed_type->LIST.nodes.push_back(make_any());
                }

                if (left->TYPE.allowed_type->type == NodeType::ANY)
                {
                    elem_type = left->TYPE.allowed_type;
                }
                else
                {
                    elem_type = left->TYPE.allowed_type->LIST.nodes[0];
                }

                auto match = match_types(elem_type, value);

                if (!match.result)
                {
                    error_and_exit("Cannot prepend element of type '" + value->type_repr() + "' to list of type " + left->TYPE.allowed_type->type_repr());
                }

                return left;
            }
            if (right->FUNC_CALL.name == "insert")
            {
                if (right->FUNC_CALL.args.size() != 2)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects 2 argument");
                }

                if (typecheck)
                {
                    left->LIST.nodes.clear();
                }

                auto position_node = eval_node(right->FUNC_CALL.args[0]);

                if (position_node->type != NodeType::INT)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects first argument to be an integer");
                }
                
                int position = position_node->INT.value;

                if (position == 0)
                {
                    position = 0;
                }
                else if (position < 0)
                {
                    position = 0;
                }
                else if (position >= left->LIST.nodes.size())
                {
                    position = left->LIST.nodes.size()-1;
                }

                auto value = eval_node(right->FUNC_CALL.args[1]);

                std::shared_ptr<Node> elem_type;

                if (left->TYPE.allowed_type == nullptr)
                {
                    left->TYPE.allowed_type = make_list();
                    left->TYPE.allowed_type->LIST.nodes.push_back(make_any());
                }

                if (left->TYPE.allowed_type->type == NodeType::ANY)
                {
                    elem_type = left->TYPE.allowed_type;
                }
                else
                {
                    elem_type = left->TYPE.allowed_type->LIST.nodes[0];
                }

                auto match = match_types(elem_type, value);

                if (!match.result)
                {
                    error_and_exit("Cannot insert element of type '" + value->type_repr() + "' to list of type " + left->TYPE.allowed_type->type_repr());
                }

                return left;
            }
            if (right->FUNC_CALL.name == "remove")
            {
                if (right->FUNC_CALL.args.size() != 1)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects 1 argument");
                }

                auto value = eval_node(right->FUNC_CALL.args[0]);

                if (value->type != NodeType::INT)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects 1 integer argument");
                }

                if (left->LIST.nodes.size() == 0)
                {
                    return left;
                }

                int pos = value->INT.value;

                if (pos < 0)
                {
                    pos = 0;
                }

                if (pos >= left->LIST.nodes.size())
                {
                    pos = left->LIST.nodes.size() - 1;
                }

                return left;
            }
            if (right->FUNC_CALL.name == "clear")
            {
                if (right->FUNC_CALL.args.size() != 0)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects 0 arguments");
                }

                return left;
            }
            if (right->FUNC_CALL.name == "subsection")
            {
                if (left->LIST.nodes.size() == 0)
                {
                    return left;
                }

                if (right->FUNC_CALL.args.size() != 2)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects 2 arguments");
                }

                auto begin = eval_node(right->FUNC_CALL.args[0]);
                auto end = eval_node(right->FUNC_CALL.args[1]);

                if (begin->type != NodeType::INT && end->type != NodeType::INT)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects 2 integer arguments");
                }

                int _begin = begin->INT.value;
                int _end = end->INT.value-1;

                if (_begin < 0)
                {
                    _begin = 0;
                }

                if (_begin >= left->LIST.nodes.size())
                {
                    _begin = left->LIST.nodes.size()-1;
                }

                if (_end >= left->LIST.nodes.size())
                {
                    _end = left->LIST.nodes.size()-1;
                }

                if (_end < 0)
                {
                    _end = 0;
                }

                if (_begin > _end)
                {
                    error_and_exit("List built-in '" + right->FUNC_CALL.name + "' expects first argument to be less than or equal to second argument");
                }

                return left;
            }

            error_and_exit("List does not contain built-in function '" + right->FUNC_CALL.name + "'");
        }

        // List properties

        if (right->type == NodeType::ID)
        {
            if (right->ID.value == "length")
            {
                auto res = make_int(0, false);
                return res;
            }
            if (right->ID.value == "first")
            {
                return left->TYPE.allowed_type->LIST.nodes[0];
            }

            if (right->ID.value == "last")
            {
                return left->TYPE.allowed_type->LIST.nodes[0];
            }
            if (right->ID.value == "empty")
            {
                return make_bool(true, false);
            }
        }

        error_and_exit("List does not contain property '" + right->ID.value + "'");
    }

    // String

    if (left->type == NodeType::STRING)
    {
        // element access

        if (right->type == NodeType::LIST)
        {
            if (right->LIST.nodes.size() != 1)
            {
                error_and_exit("String accessor expects one value");
            }
            
            auto accessor = eval_node(right->LIST.nodes[0]);

            if (accessor->type != NodeType::INT)
            {
                error_and_exit("String accessor expects an integer");
            }

            return make_string("", false);
        }

        // String built-ins

        if (right->type == NodeType::FUNC_CALL)
        {
            if (right->FUNC_CALL.name == "chars")
            {
                if (right->FUNC_CALL.args.size() != 0)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 0 arguments");
                }

                auto res = make_list();
                res->LIST.nodes.push_back(make_string("", false));

                return res;
            }

            if (right->FUNC_CALL.name == "lower")
            {
                if (right->FUNC_CALL.args.size() != 0)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 0 arguments");
                }

                return left;
            }

            if (right->FUNC_CALL.name == "upper")
            {
                if (right->FUNC_CALL.args.size() != 0)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 0 arguments");
                }

                return left;
            }

            if (right->FUNC_CALL.name == "subsection")
            {
                if (right->FUNC_CALL.args.size() != 2)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 2 arguments");
                }

                auto begin = eval_node(right->FUNC_CALL.args[0]);
                auto end = eval_node(right->FUNC_CALL.args[1]);

                if (begin->type != NodeType::INT && end->type != NodeType::INT)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 2 integer arguments");
                }

                int _begin = begin->INT.value;
                int _end = end->INT.value-1;

                if (_begin < 0)
                {
                    _begin = 0;
                }

                if (_begin >= left->STRING.value.length())
                {
                    _begin = left->STRING.value.length()-1;
                }

                if (_end >= left->STRING.value.length())
                {
                    _end = left->STRING.value.length()-1;
                }

                if (_end < 0)
                {
                    _end = 0;
                }

                if (_begin > _end)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects first argument to be less than or equal to second argument");
                }

                auto subsection = std::make_shared<Node>(*left);
                subsection->STRING.value.clear();

                return left;
            }

            if (right->FUNC_CALL.name == "split")
            {
                if (right->FUNC_CALL.args.size() != 0 && right->FUNC_CALL.args.size() != 1)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 0 or 1 arguments");
                }

                std::string delim;

                if (right->FUNC_CALL.args.size() == 0)
                {
                    delim = " ";
                }
                else
                {
                    auto delim_node = eval_node(right->FUNC_CALL.args[0]);

                    if (delim_node->type != NodeType::STRING)
                    {
                        error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 1 string argument");
                    }

                    delim = delim_node->STRING.value;

                    if (delim == "")
                    {
                        delim = " ";
                    }
                }

                size_t pos_start = 0, pos_end, delim_len = delim.length();
                std::string token;

                auto res = make_list();
                res->LIST.nodes.push_back(make_string("", false));

                return res;
            }

            if (right->FUNC_CALL.name == "trim")
            {
                if (right->FUNC_CALL.args.size() != 0)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 0 arguments");
                }

                return left;
            }

            if (right->FUNC_CALL.name == "append")
            {
                if (right->FUNC_CALL.args.size() != 1)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 1 argument");
                }

                auto value = eval_node(right->FUNC_CALL.args[0]);

                if (value->type != NodeType::STRING)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 1 string argument");
                }

                return left;
            }

            if (right->FUNC_CALL.name == "prepend")
            {
                if (right->FUNC_CALL.args.size() != 1)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 1 argument");
                }

                auto value = eval_node(right->FUNC_CALL.args[0]);

                if (value->type != NodeType::STRING)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 1 string argument");
                }

                return left;
            }

            if (right->FUNC_CALL.name == "insert")
            {
                if (right->FUNC_CALL.args.size() != 2)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 2 arguments");
                }

                auto pos_node = eval_node(right->FUNC_CALL.args[0]);
                auto value_node = eval_node(right->FUNC_CALL.args[1]);

                if (pos_node->type != NodeType::INT)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects first argument to be of type int");
                }

                if (value_node->type != NodeType::STRING)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects second argument to be of type string");
                }

                int pos = pos_node->INT.value;

                if (pos < 0)
                {
                    pos = 0;
                }

                if (pos >= left->STRING.value.length())
                {
                    pos = left->STRING.value.length();
                }

                return left;
            }

            if (right->FUNC_CALL.name == "replace_at")
            {
                if (right->FUNC_CALL.args.size() != 2)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 2 arguments");
                }

                auto pos_node = eval_node(right->FUNC_CALL.args[0]);
                auto value_node = eval_node(right->FUNC_CALL.args[1]);

                if (pos_node->type != NodeType::INT)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects first argument to be of type int");
                }

                if (value_node->type != NodeType::STRING)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects second argument to be of type string");
                }

                int pos = pos_node->INT.value;

                if (pos < 0)
                {
                    pos = 0;
                }

                if (pos >= left->STRING.value.length())
                {
                    pos = left->STRING.value.length()-1;
                }

                return left;
            }

            if (right->FUNC_CALL.name == "remove")
            {
                if (right->FUNC_CALL.args.size() != 1)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 1 argument");
                }

                auto pos_node = eval_node(right->FUNC_CALL.args[0]);

                if (pos_node->type != NodeType::INT)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects argument to be of type int");
                }

                if (left->STRING.value.length() == 0)
                {
                    return left;
                }

                int pos = pos_node->INT.value;

                if (pos < 0)
                {
                    pos = 0;
                }

                if (pos >= left->STRING.value.length())
                {
                    pos = left->STRING.value.length()-1;
                }

                return left;
            }

            if (right->FUNC_CALL.name == "replace")
            {
                if (right->FUNC_CALL.args.size() != 2)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 2 argument");
                }

                auto from_node = eval_node(right->FUNC_CALL.args[0]);
                auto to_node = eval_node(right->FUNC_CALL.args[1]);

                if (from_node->type != NodeType::STRING && to_node->type != NodeType::STRING)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 2 string arguments");
                }

                return left;
            }

            if (right->FUNC_CALL.name == "replace_all")
            {
                if (right->FUNC_CALL.args.size() != 2)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 2 arguments");
                }

                auto from_node = eval_node(right->FUNC_CALL.args[0]);
                auto to_node = eval_node(right->FUNC_CALL.args[1]);

                if (from_node->type != NodeType::STRING && to_node->type != NodeType::STRING)
                {
                    error_and_exit("String built-in '" + right->FUNC_CALL.name + "' expects 2 string arguments");
                }

                return left;
            }

            error_and_exit("String does not contain built-in function '" + right->FUNC_CALL.name + "'");
        }

        // String properties

        if (right->type == NodeType::ID)
        {
            auto res = std::make_shared<Node>(*node);

            if (right->ID.value == "length")
            {
                res = make_int(0, false);
                return res;
            }

            if (right->ID.value == "first")
            {
                res = make_string("", false);
                return res;
            }

            if (right->ID.value == "last")
            {
                res = make_string("", false);
                return res;
            }
            if (right->ID.value == "empty")
            {
                res = make_bool(true, false);
                return res;
            }
            if (right->ID.value == "is_alpha")
            {
                res = make_bool(true, false);
                return res;
            }
            if (right->ID.value == "is_digit")
            {
                res = make_bool(true, false);
                return res;
            }

            error_and_exit("String does not contain property '" + right->ID.value + "'");
        }
    }

    // Objects

    if (left->type == NodeType::OBJECT)
    {
        auto res = std::make_shared<Node>(*node);

        // Property access

        if (right->type == NodeType::LIST)
        {
            if (right->LIST.nodes.size() != 1)
            {
                error_and_exit("Object accessor expects one value");
            }
            
            auto accessor = eval_node(right->LIST.nodes[0]);

            if (!left->TYPE.is_literal)
            {
                return make_any();
            }

            if (accessor->type == NodeType::ANY)
            {
                return make_any();
            }

            if (accessor->type != NodeType::STRING)
            {
                error_and_exit("Object accessor expects value of type string");
            }

            auto prop_name = accessor->STRING.value;

            if (left->OBJECT.properties.find(prop_name) == left->OBJECT.properties.end())
            {
                res->type = NodeType::ANY;
                return res;
            }

            auto value = left->TYPE.allowed_type->OBJECT.properties[prop_name].value;

            if (value->type == NodeType::FUNCTION && value->FUNCTION.is_type)
            {
                return get_explicit_type(value->FUNCTION.params[0]);
            }

            return value;
        }

        if (right->type == NodeType::ID)
        {
            auto prop_name = right->ID.value;

            // built-in props

            if (prop_name == "_keys")
            {
                res = make_list();
                res->LIST.nodes.push_back(make_string("", false));
                return res;
            }

            if (prop_name == "_values")
            {
                res = make_list();

                if (!left->TYPE.is_literal)
                {
                    res->LIST.nodes.push_back(make_any());
                    return res;
                }

                auto value = make_pipe_list();

                for (auto prop : left->TYPE.allowed_type->OBJECT.properties)
                {
                    // value->PIPE_LIST.nodes.insert(res->LIST.nodes.begin(), prop.second.value);
                    value->PIPE_LIST.nodes.insert(value->PIPE_LIST.nodes.begin(), prop.second.value);
                }

                sort_and_unique(value->PIPE_LIST.nodes);

                if (value->PIPE_LIST.nodes.size() == 1)
                {
                    value = value->PIPE_LIST.nodes[0]; 
                }

                res->LIST.nodes.push_back(value);

                return res;
            }

            if (prop_name == "_items")
            {
                res = make_list();

                if (!left->TYPE.is_literal)
                {
                    auto elem = make_object();
                    elem->OBJECT.properties["key"] = Object_Prop(true, make_string("", false));
                    elem->OBJECT.properties["value"] = Object_Prop(true, make_any());
                    res->LIST.nodes.push_back(elem);
                    return res;
                }

                auto elem = make_object();
                elem->OBJECT.properties["key"] = Object_Prop(true, make_string("", false));
                auto value = make_pipe_list();

                for (auto prop : left->TYPE.allowed_type->OBJECT.properties)
                {
                    value->PIPE_LIST.nodes.insert(value->PIPE_LIST.nodes.begin(), prop.second.value);
                }

                sort_and_unique(value->PIPE_LIST.nodes);

                if (value->PIPE_LIST.nodes.size() == 1)
                {
                    elem->OBJECT.properties["value"] = Object_Prop(true, value->PIPE_LIST.nodes[0]); 
                }

                elem->OBJECT.properties["value"] = Object_Prop(true, value);

                res->LIST.nodes.push_back(elem);

                return res;
            }

            if (!left->TYPE.is_literal)
            {
                return make_any();
            }

            if (left->OBJECT.properties.find(prop_name) == left->OBJECT.properties.end())
            {
                res->type = NodeType::EMPTY;
                return res;
            }

            auto value = left->OBJECT.properties[prop_name].value;

            if (value->type == NodeType::FUNCTION && value->FUNCTION.is_type)
            {
                return get_explicit_type(value->FUNCTION.params[0]);
            }

            return value;
        }

        if (right->type == NodeType::FUNC_CALL)
        {
            auto name = right->FUNC_CALL.name;

            // built-ins

            if (name == "delete")
            {
                if (right->FUNC_CALL.args.size() != 1)
                {
                    error_and_exit("Object function '" + name + "' expects 1 argument");
                }

                auto prop_name = eval_node(right->FUNC_CALL.args[0]);

                if (prop_name->type != NodeType::STRING)
                {
                    error_and_exit("Object function '" + name + "' expects an argument of type string");
                }

                return left;
            }

            if (name == "clear")
            {
                if (right->FUNC_CALL.args.size() != 0)
                {
                    error_and_exit("Object function '" + name + "' expects 0 arguments");
                }

                return left;
            }
            if (name == "add")
            {
                if (right->FUNC_CALL.args.size() != 2)
                {
                    error_and_exit("Object function '" + name + "' expects 2 arguments");
                }

                auto prop_name = eval_node(right->FUNC_CALL.args[0]);
                auto value = eval_node(right->FUNC_CALL.args[1]);

                if (prop_name->type != NodeType::STRING)
                {
                    error_and_exit("Object function '" + name + "' expects name argument to be of type string");
                }

                return left;
            }

            // function properties

            if (left->TYPE.allowed_type->OBJECT.properties.find(name) == left->TYPE.allowed_type->OBJECT.properties.end())
            {
                error_and_exit("Function '" + name + "' does not exist on object");
            }

            auto function = left->TYPE.allowed_type->OBJECT.properties[name].value;

            // call function

            return eval_function_call(function, right->FUNC_CALL.args);
        }
    }

    if (left->type == NodeType::FUNCTION)
    {
        if (right->type == NodeType::FUNC_CALL)
        {
            if (right->FUNC_CALL.name == "patch")
            {
                if (right->FUNC_CALL.args.size() != 1)
                {
                    error_and_exit("Function function '" + right->FUNC_CALL.name + "' expects 1 argument");
                }

                return left;
            }

            error_and_exit("Function does not contain built-in function '" + right->FUNC_CALL.name + "'");
        }

        if (right->type == NodeType::ID)
        {
            if (right->ID.value == "name")
            {
                return make_string("", false);
            }

            if (right->ID.value == "params")
            {
                auto params = make_list();
                params->LIST.nodes.push_back(make_string("", false));
               
                return params;
            }

            if (right->ID.value == "args")
            {
                auto args = make_list();
                args->LIST.nodes.push_back(make_any());
               
                return args;
            }

            error_and_exit("Function does not contain built-in property '" + right->ID.value + "'");
        }
    }

    error_and_exit("Cannot perform operation '" + node->repr() + "' on '" + left->repr() + "' and '" + right->repr() + "'");
}

std::shared_ptr<Node> Typechecker::eval_equal(std::shared_ptr<Node>& node)
{
    // check if ID first, because evaluating an ID could return an error

    if (node->left->type == NodeType::ID)
    {
        auto& var = eval_look_up(node->left->ID.value);
        auto value = eval_node(node->right);

        if (value->type == NodeType::RETURN)
        {
            value = value->right;
        }

        if (var->type == NodeType::ERROR)
        {
            // If type is not explicit

            if (node->left->ID.type == nullptr)
            {
                auto val = std::make_shared<Node>(*value);

                // EXPERIMENTAL

                if (val->TYPE.allowed_type == nullptr)
                {
                    //val->TYPE.allowed_type = get_type(val);
                    val->TYPE.allowed_type = val;
                }

                if (val->type == NodeType::FUNCTION)
                {
                    val->FUNCTION.name = node->left->ID.value;

                    auto func_list = std::make_shared<Node>(NodeType::FUNC_LIST);
                    func_list->FUNC_LIST.nodes.push_back(val);
                    symbol_table[node->left->ID.value] = func_list;
                    return val;
                }

                symbol_table[node->left->ID.value] = val;

                return val;
            }

            // If type is explicit

            if (value->TYPE.allowed_type == nullptr)
            {
                value->TYPE.allowed_type = value;
            }

            auto explicit_type = eval_node(node->left->ID.type);

            if (explicit_type->type == NodeType::TYPE)
            {
                value->TYPE.name = node->left->ID.value;
                value->OBJECT.is_type = true;
                value->FUNCTION.is_type = true;
            }

            if (explicit_type->type == NodeType::FUNCTION)
            {
                explicit_type->FUNCTION.is_type = true;
            }

            if (value->TYPE.name == "")
            {
                if (explicit_type->type == NodeType::FUNCTION)
                {
                    value->TYPE.name = explicit_type->FUNCTION.name;
                }
                else if (explicit_type->type == NodeType::OBJECT)
                {
                    value->TYPE.name = explicit_type->TYPE.name;
                }
            }
            
            auto match = match_types(explicit_type, value);

            if (!match.result)
            {
                value->TYPE.name.clear();
                error_and_exit("[TypeError] Cannot assign value of type '" + value->repr() + "' to variable of type '" + explicit_type->repr() + "'. " + match.message );
            }

            std::shared_ptr<Node> val;

            if (value->type == NodeType::LIST)
            {
                val = std::make_shared<Node>(*explicit_type);
                val->TYPE.allowed_type = val;
            }
            else
            {
                val = std::make_shared<Node>(*value);
                val->TYPE.allowed_type = explicit_type;
            }

            if (val->type == NodeType::FUNCTION)
            {
                val->FUNCTION.name = node->left->ID.value;
            }

            symbol_table[node->left->ID.value] = val;

            return val;
        }

        // Check that we are not re-assigning the type

        if (node->left->ID.type != nullptr && node->left->ID.type->type != NodeType::ANY)
        {
            error_and_exit("[TypeError] Cannot re-assign type of variable '" + node->left->ID.value + "'");
        }

        if (node->left->ID.type != nullptr && node->left->ID.type->type == NodeType::ANY)
        {
            var->TYPE.allowed_type = node->left->ID.type;
        }

        // Check if we are adding a function to a multiple dispatch list
        if (var->type == NodeType::FUNC_LIST)
        {
            auto val = std::make_shared<Node>(*value);

            if (val->type == NodeType::FUNCTION)
            {
                val->FUNCTION.name = node->left->ID.value;
                var->FUNC_LIST.nodes.push_back(val);
                symbol_table[node->left->ID.value] = var;
                return val;
            }
        }

        // if (var->type == NodeType::PIPE_LIST)
        // {
        //     auto val = std::make_shared<Node>(*value);

        //     if (val->type == NodeType::FUNCTION)
        //     {
        //         val->FUNCTION.name = node->left->ID.value;
        //         var->PIPE_LIST.nodes.push_back(val);
        //         symbol_table[node->left->ID.value] = var;
        //         return val;
        //     }
        // }

        if (var->TYPE.allowed_type == nullptr)
        {
            // var->TYPE.allowed_type = std::make_shared<Node>(NodeType::ANY);
            var->TYPE.allowed_type = var;
        }

        // Check allowed types
        if (var->TYPE.allowed_type->type == NodeType::ANY)
        {
            auto val = std::make_shared<Node>(*value);

            if (val->type == NodeType::FUNCTION)
            {
                val->FUNCTION.name = node->left->ID.value;
            }

            val->TYPE.allowed_type = var->TYPE.allowed_type;

            *var = *val;

            return val;
        }

        auto match = match_types(var->TYPE.allowed_type, value);

        if (!match.result)
        {
            error_and_exit("[TypeError] Cannot assign value of type '" + value->repr() + "' to variable of type '" + var->TYPE.allowed_type->repr() + "'. " + match.message);
        }

        auto val = std::make_shared<Node>(*value);
        val->TYPE.allowed_type = var->TYPE.allowed_type;

        if (val->type == NodeType::FUNCTION)
        {
            val->FUNCTION.name = node->left->ID.value;
        }

        *var = *val;

        return val;
    }

    if (node->left->type == NodeType::LIST)
    {
        if (node->left->LIST.nodes.size() != 1)
        {
            error_and_exit("Dynamic variable creation list expects 1 argument");
        }

        auto name = eval_node(node->left->LIST.nodes[0]);

        if (name->type != NodeType::STRING)
        {
            error_and_exit("Dynamic variable creation list expects 1 string argument");
        }

        auto value = eval_node(node->right);
        value->TYPE.allowed_type = make_any();

        symbol_table[name->STRING.value] = std::make_shared<Node>(*value);
        return value;
    }

    auto left = eval_node(node->left);
    auto right = eval_node(node->right);

    if (node->left->OP.op_type == NodeType::DOT)
    {
        auto obj = eval_node(node->left->left);

        if (obj->type == NodeType::OBJECT)
        {
            std::string prop_name;

            if (node->left->right->type == NodeType::ID)
            {
                prop_name = node->left->right->ID.value;
            }
            else if (node->left->right->type == NodeType::LIST)
            {
                if (node->left->right->LIST.nodes.size() == 0)
                {
                    error_and_exit("Cannot assign an empty property name");
                }

                auto prop_name_node = eval_node(node->left->right->LIST.nodes[0]);

                if (!node->left->left->TYPE.is_literal)
                {
                    return make_any();
                }

                if (prop_name_node->type == NodeType::ANY)
                {
                    return make_any();
                }

                if (prop_name_node->type != NodeType::STRING)
                {
                    error_and_exit("Property name must be of type string");
                }
                else
                {
                    prop_name = prop_name_node->STRING.value;
                }
            }

            if (!obj->TYPE.is_literal)
            {
                return left;
            }

            if (prop_name == "___string::default")
            {
                left->TYPE.is_literal = false;
                return left;
            }

            if (obj->TYPE.allowed_type->OBJECT.properties.find(prop_name) != obj->TYPE.allowed_type->OBJECT.properties.end())
            {
                if (left->type == NodeType::FUNC_LIST && right->type == NodeType::FUNCTION)
                {
                    right->FUNCTION.name = prop_name;
                    left->FUNC_LIST.nodes.push_back(right);
                    obj->OBJECT.properties[prop_name].value = left;
                    obj->TYPE.allowed_type->OBJECT.properties[prop_name].value = left;
                    return left;
                }

                if (left->type == NodeType::FUNCTION && right->type == NodeType::FUNCTION)
                {
                    auto func_list = std::make_shared<Node>(NodeType::FUNC_LIST);
                    func_list->FUNC_LIST.nodes.push_back(left);
                    right->FUNCTION.name = prop_name;
                    func_list->FUNC_LIST.nodes.push_back(right);
                    obj->OBJECT.properties[prop_name].value = func_list;
                    obj->TYPE.allowed_type->OBJECT.properties[prop_name].value = func_list;
                    return left;
                }

                if (obj->TYPE.allowed_type != nullptr)
                {
                    if (!match_types(obj->TYPE.allowed_type->OBJECT.properties[prop_name].value, right).result)
                    {
                        error_and_exit("Cannot assign value of type '" + right->type_repr() + "' to object property of type '" + obj->TYPE.allowed_type->OBJECT.properties[prop_name].value->repr() + "'");
                    }
                }
            }

            if (right->TYPE.allowed_type == nullptr)
            {
                if (obj->TYPE.allowed_type != nullptr)
                {
                    right->TYPE.allowed_type = obj->TYPE.allowed_type->OBJECT.properties[prop_name].value;
                }
                else
                {
                    right->TYPE.allowed_type = right;
                }
            }

            obj->OBJECT.properties[prop_name] = Object_Prop(true, right);
            obj->TYPE.allowed_type->OBJECT.properties[prop_name] = Object_Prop(true, get_type(right));

            return left;
        }
    }

    *left = *right;
    return left;
}

std::shared_ptr<Node> Typechecker::eval_double_dot(std::shared_ptr<Node>& node)
{
    auto left = eval_node(node->left);
    auto right = eval_node(node->right);

    if (left->type != NodeType::INT && right->type != NodeType::INT)
    {
        error_and_exit("Range operator expects both operands to be integer types");
    }

    auto res = make_list();
    res->LIST.nodes.push_back(make_int(0, false));
    return res;
}

std::shared_ptr<Node> Typechecker::eval_double_arrow(std::shared_ptr<Node>& node)
{
    auto left = eval_node(node->left);

    // Built-in function intercept

    if (node->right->type == NodeType::ID)
    {
        if (tc_builtin_functions.find(node->right->ID.value) != tc_builtin_functions.end())
        {
            auto func = tc_builtin_functions[node->right->ID.value];
            std::vector<std::shared_ptr<Node>> args;

            if (left->type == NodeType::COMMA_LIST)
            {
                args = left->COMMA_LIST.nodes;
            }
            else
            {
                args = { left };
            }

            return (this->*func)(args);
        }
    }

    if (node->right->type == NodeType::FUNC_CALL)
    {
        if (tc_builtin_functions.find(node->right->FUNC_CALL.name) != tc_builtin_functions.end())
        {
            auto func = tc_builtin_functions[node->right->FUNC_CALL.name];

            std::vector<std::shared_ptr<Node>> args = node->right->FUNC_CALL.args;

            if (left->type == NodeType::COMMA_LIST)
            {
                for (auto arg : left->COMMA_LIST.nodes)
                {
                    args.push_back(arg);
                }
            }
            else
            {
                args.push_back(left);
            }

            return (this->*func)(args);
        }

        auto func = eval_look_up(node->right->FUNC_CALL.name);

        if (func->type == NodeType::ERROR)
        {
            error_and_exit("Function '" + node->right->FUNC_CALL.name + "' is undefined");
        }

        auto args = node->right->FUNC_CALL.args;

        if (left->type == NodeType::COMMA_LIST)
        {
            for (auto elem : left->COMMA_LIST.nodes)
            {
                args.push_back(eval_node(elem));
            }
        }
        else
        {
            args.push_back(eval_node(left));
        }

        return eval_function_call(func, args);
    }

    auto right = eval_node(node->right);

    auto res = std::make_shared<Node>(*right);

    if (right->type != NodeType::PARTIAL_OP && right->type != NodeType::FUNCTION && right->type != NodeType::FUNC_LIST)
    {
        error_and_exit("Cannot perform operation '" + node->repr() + "' on '" + left->repr() + "' and '" + right->repr() + "'");
    }

    if (right->type == NodeType::PARTIAL_OP)
    {
        if (!right->TYPE.is_literal)
        {
            return make_any();
        }

        int missing_operand_count = 0;

        if (right->left->type == NodeType::EMPTY)
        {
            missing_operand_count++;
        }
        if (right->right->type == NodeType::EMPTY)
        {
            missing_operand_count++;
        }

        if (left->type == NodeType::COMMA_LIST)
        {
            auto elements = left->COMMA_LIST.nodes;

            if (elements.size() != 2)
            {
                error_and_exit("Cannot inject more than 2 values into a partial operator");
            }

            if (missing_operand_count != 2)
            {
                error_and_exit("Cannot inject multiple values into a partial operator with one missing operand");
            }

            res->left = elements[0];
            res->right = elements[1];

            res->type = NodeType::OP;

            return eval_node(res);
        }

        if (right->left->type == NodeType::EMPTY)
        {
            res->left = left;
        }
        else if (right->right->type == NodeType::EMPTY)
        {
            res->right = left;
        }

        if (res->left->type != NodeType::EMPTY && res->right->type != NodeType::EMPTY)
        {
            res->type = NodeType::OP;
        }

        return eval_node(res);
    }

    if (right->type == NodeType::FUNCTION || right->type == NodeType::FUNC_LIST && right->FUNC_LIST.nodes.size() == 1)
    {
        if (right->type == NodeType::FUNC_LIST)
        {
            right = right->FUNC_LIST.nodes[0];
        }

        if (!right->TYPE.is_literal)
        {
            return make_any();
        }

        auto args = right->FUNCTION.args;

        if (args.size() == 0)
        {
            error_and_exit("Function '" + right->FUNCTION.name + "' expects 0 arguments, and cannot be injected");
        }
        
        if (left->type == NodeType::COMMA_LIST)
        {
            for (int i = 0; i < left->COMMA_LIST.nodes.size(); i++)
            {
                int arg_index = 0;

                while (args[arg_index] != nullptr)
                {
                    arg_index++;

                    if (arg_index >= args.size())
                    {
                        error_and_exit("Injecting too many arguments into a function");
                    }
                }

                args[arg_index] = left->COMMA_LIST.nodes[i];
            }
        }
        else
        {

            int arg_index = 0;

            while (args[arg_index] != nullptr)
            {
                arg_index++;

                if (arg_index >= args.size())
                {
                    error_and_exit("Injecting too many arguments into a function");
                }
            }

            args[arg_index] = left;
        }

        std::vector<std::shared_ptr<Node>> flat_args;

        auto func_copy = std::make_shared<Node>(*right);

        for (int i = 0; i < args.size(); i++)
        {
            // Remove the arg value here because
            // it's going to get re-injected in
            // eval_function_call 

            func_copy->FUNCTION.args[i] = nullptr;

            if (args[i] != nullptr)
            {
                flat_args.push_back(eval_node(args[i]));
            }
        }

        return eval_function_call(func_copy, flat_args);
    }

    if (right->type == NodeType::FUNC_LIST)
    {
        std::vector<std::shared_ptr<Node>> args;
        
        if (left->type == NodeType::COMMA_LIST)
        {
            for (int i = 0; i < left->COMMA_LIST.nodes.size(); i++)
            {
                args.push_back(eval_node(left->COMMA_LIST.nodes[i]));
            }
        }
        else
        {
            args.push_back(eval_node(left));
        }

        return eval_function_call(right, args);
    }
}

// --- END OPERATIONS --- //

std::shared_ptr<Node>& Typechecker::eval_look_up(std::string name)
{
    auto typechecker = std::make_shared<Typechecker>(*this);

    if (symbol_table.find(name) == symbol_table.end())
    {
        typechecker = outer_scope;

        while (typechecker != nullptr)
        {
            auto& res = typechecker->eval_look_up(name);

            if (res->type != NodeType::ERROR)
            {
                return res;
            }

            typechecker = typechecker->outer_scope;
        }

        return tc_error_node;
    }

    auto& res = symbol_table[name];

    return res;
}

std::shared_ptr<Node> Typechecker::match_function(std::shared_ptr<Node> function, std::vector<std::shared_ptr<Node>> args)
{
    auto func = std::make_shared<Node>(*function);

    if (func->type == NodeType::FUNCTION)
    {
        return func;
    }

    std::vector<std::shared_ptr<Node>> potential_functions;

    if (function->type == NodeType::FUNC_LIST)
    {
        if (function->FUNC_LIST.nodes.size() == 1)
        {
            func = std::make_shared<Node>(*function->FUNC_LIST.nodes[0]);
            return func;
        }

        Typechecker tc;
        tc.runtime = runtime;
        tc.builtins_path = builtins_path;
        tc.outer_scope = std::make_shared<Typechecker>(*this);
        tc.line = line;
        tc.column = column;
        tc.file_name = file_name;
        tc.builtins_names = builtins_names;

        int func_num = -1;

        for (auto f : function->FUNC_LIST.nodes)
        {
            func_num++;

            if (args.size() > f->FUNCTION.params.size())
            {
                continue;
            }

            int num_null_args = 0;

            // Inject closure into symbol table

            for (auto symbol : f->FUNCTION.closure)
            {
                tc.symbol_table[symbol.first] = symbol.second;
            }

            for (int i = 0; i < args.size(); i++)
            {
                auto& param = f->FUNCTION.params[i];

                auto param_type = tc.get_explicit_type(param);

                if (param_type->type == NodeType::PIPE_LIST && param_type->PIPE_LIST.nodes.size() == 1)
                {
                    param_type = param_type->PIPE_LIST.nodes[0];
                }

                if (param_type->type == NodeType::FUNC_LIST && param_type->FUNC_LIST.nodes.size() == 1)
                {
                    param_type = param_type->FUNC_LIST.nodes[0];
                }

                auto arg = tc.eval_node(args[i]);

                if (arg->type == NodeType::ANY 
                && func_num < function->FUNC_LIST.nodes.size()-1)
                {
                    goto next;
                }

                if (!match_types(param_type, arg).result)
                {
                    goto next;
                }
            }

            for (auto& arg : f->FUNCTION.args)
            {
                if (arg == nullptr)
                {
                    num_null_args++;
                }
            }

            if (args.size() == num_null_args)
            {
                // if we match on something here, we assign and break
                // no need to keep looking

                func = std::make_shared<Node>(*f);
                return func;
            }

            potential_functions.push_back(std::make_shared<Node>(*f));

            next:

                // Remove closure from symbol table

                for (auto symbol : f->FUNCTION.closure)
                {
                    tc.symbol_table.erase(symbol.first);
                }
                continue;
        }
    }
    else
    {
        potential_functions.push_back(func);
    }

    if (potential_functions.size() == 0)
    {
        std::string potentials;

        for (auto& func : function->FUNC_LIST.nodes)
        {
            potentials += func->FUNCTION.name + ": " + func->type_repr() + "\n";
        }

        std::string args_str = "[ ";

        for (auto& arg :args)
        {
            args_str += eval_node(arg)->type_repr() + " ";
        }

        args_str += "]";

        error_and_exit("Cannot find matching function definition for args " + args_str + ". Defined functions are:\n\n" + potentials);
    }

    if (potential_functions.size() == 1)
    {
        func = potential_functions[0];
    }

    if (potential_functions.size() > 1)
    {
        std::string potentials;

        for (auto& func : potential_functions)
        {
            potentials += func->FUNCTION.name + ": " + func->type_repr() + "\n";
        }

        error_and_exit("Ambiguous function call - there are " + std::to_string(potential_functions.size()) + " potential functions this call could refer to:\n\n" + potentials);
    }

    return func;
}

std::shared_ptr<Node> Typechecker::clone_func(std::shared_ptr<Node> func)
{
    if (!func->TYPE.is_literal)
    {
        return func;
    }

    auto function = make_function();
    function->FUNCTION.name = func->FUNCTION.name;
    for (auto param: func->FUNCTION.params)
    {
        if (param->type == NodeType::ID)
        {
            function->FUNCTION.params.push_back(std::make_shared<Node>(*param));
        }
        else if (param->OP.op_type == NodeType::EQUAL)
        {
            auto param_copy = std::make_shared<Node>(*param);
            param_copy->left = std::make_shared<Node>(*param->left);
            param_copy->right = std::make_shared<Node>(*param->left);

            function->FUNCTION.params.push_back(param_copy);
        }
        else if (param->OP.op_type == NodeType::TRIPLE_DOT)
        {
            auto param_copy = std::make_shared<Node>(*param);
            param_copy->right = std::make_shared<Node>(*param->right);

            function->FUNCTION.params.push_back(param_copy);
        }
    }

    for (auto arg: func->FUNCTION.args)
    {
        if (arg == nullptr)
        {
            function->FUNCTION.args.push_back(nullptr);
        }
        else
        {
            function->FUNCTION.args.push_back(std::make_shared<Node>(*arg));
        }
    }
    for (auto elem: func->FUNCTION.closure)
    {
        function->FUNCTION.closure[elem.first] = elem.second;
    }

    if (func->FUNCTION.return_type != nullptr)
    {
        function->FUNCTION.return_type = std::make_shared<Node>(*func->FUNCTION.return_type);
    }

    function->FUNCTION.body = std::make_shared<Node>(*func->FUNCTION.body);

    function->FUNCTION.default_arg_indices = func->FUNCTION.default_arg_indices;
    function->FUNCTION.is_type = func->FUNCTION.is_type;

    return function;
}

std::shared_ptr<Node> Typechecker::eval_function_call(std::shared_ptr<Node> function, std::vector<std::shared_ptr<Node>> args)
{
    if (function->type == NodeType::ANY)
    {
        return make_any();
    }

    if (function->type == NodeType::PIPE_LIST)
    {
        auto returns = make_pipe_list();

        for (auto func : function->PIPE_LIST.nodes)
        {
            auto func_copy = clone_func(func);
            auto res = eval_function_call(func_copy, args);
            returns->PIPE_LIST.nodes.push_back(res);
        }

        sort_and_unique(returns->PIPE_LIST.nodes);

        return returns;
    }

    if (function->type != NodeType::FUNCTION && function->type != NodeType::FUNC_LIST)
    {
        error_and_exit("Malformed function call - variable is not a function");
    }

    auto func = std::make_shared<Node>(*function);

    if (func->type == NodeType::FUNC_LIST)
    {
        func = match_function(function, args);
    }

    func = clone_func(func);

    if (!func->TYPE.is_literal)
    {
        if (func->FUNCTION.return_type == nullptr)
        {
            return make_any();
        }

        return func->FUNCTION.return_type;
    }

    Typechecker typechecker;
    typechecker.runtime = runtime;
    typechecker.builtins_path = builtins_path;
    typechecker.line = line;
    typechecker.column = column;
    typechecker.file_name = file_name + "/" + func->FUNCTION.name;
    typechecker.builtins_names = builtins_names;

    for (auto symbol : symbol_table)
    {
        if (std::find(builtins_names.begin(), builtins_names.end(), symbol.first) != builtins_names.end())
        {
            typechecker.symbol_table[symbol.first] = symbol.second;
        }
    }

    for (auto symbol : func->FUNCTION.closure)
    {
        if (symbol.second->type == NodeType::FUNC_LIST)
        {
            // check to see if it exists already
            // if so, we check to see which list has more functions

            if (typechecker.symbol_table.find(symbol.first) != typechecker.symbol_table.end())
            {
                if (typechecker.symbol_table[symbol.first]->PIPE_LIST.nodes.size() < symbol.second->FUNC_LIST.nodes.size())
                {
                    typechecker.symbol_table[symbol.first] = symbol.second;
                }
            }
            else
            {
                typechecker.symbol_table[symbol.first] = symbol.second;
            }
        }
        else
        {
            typechecker.symbol_table[symbol.first] = symbol.second;
        }
    }

    for (int i = 0; i < args.size(); i++)
    {
        if (i >= func->FUNCTION.args.size())
        {
            error_and_exit("Function '" + func->FUNCTION.name + "' expects " + std::to_string(func->FUNCTION.args.size()) + " arguments, but " + std::to_string(args.size()) + " were provided");
        }

        if (func->FUNCTION.params[i]->OP.op_type == NodeType::TRIPLE_DOT)
        {
            auto variable_arg = std::make_shared<Node>(NodeType::LIST);

            for (int j = i; j < args.size(); j++)
            {
                variable_arg->LIST.nodes.push_back(eval_node(args[j]));
            }

            func->FUNCTION.args[i] = variable_arg;
            // func->FUNCTION.params[i]->right->ID.type = variable_arg;

            break;
        }

        else if (args[i]->OP.op_type == NodeType::COLON)
        {
            auto name = args[i]->left->ID.value;
            auto value = eval_node(args[i]->right);

            int index;
            bool match = false;

            for (int p = 0; p < func->FUNCTION.params.size(); p++)
            {
                auto param = func->FUNCTION.params[p];

                if (param->type == NodeType::ID)
                {
                    if (param->ID.value == name)
                    {
                        match = true;
                        index = p;
                        break;
                    }
                }
                if (param->OP.op_type == NodeType::EQUAL)
                {
                    if (param->left->ID.value == name)
                    {
                        match = true;
                        index = p;
                        break;
                    }
                }
            }

            if (!match)
            {
                error_and_exit("Parameter '" + name + "' does not exist on function '" + func->FUNCTION.name + "'");
            }

            func->FUNCTION.args[index] = value;
            // func->FUNCTION.params[index]->left->ID.type = value;
        }
        else
        {
            int current_index = i;
            
            while (true)
            {
                bool arg_is_default = std::find(func->FUNCTION.default_arg_indices.begin(), func->FUNCTION.default_arg_indices.end(), i) != func->FUNCTION.default_arg_indices.end();

                if (func->FUNCTION.args[i] == nullptr || arg_is_default)
                {
                    auto eval_arg = eval_node(args[current_index]);
                    func->FUNCTION.args[i] = eval_arg;
                    func->FUNCTION.params[i]->ID.type = eval_arg;
                    break;
                }
                else
                {
                    i++;
                }
            }
        }
    }

    for (int i = 0; i < func->FUNCTION.args.size(); i++)
    {
        if (func->FUNCTION.args[i] == nullptr)
        {
            return func;
        }
        else
        {
            auto param = func->FUNCTION.params[i];
            auto param_type = typechecker.get_explicit_type(param);

            if (param->type == NodeType::ID)
            {
                typechecker.symbol_table[param->ID.value] = func->FUNCTION.args[i];
            }
            else if (param->OP.op_type == NodeType::EQUAL)
            {
                typechecker.symbol_table[param->left->ID.value] = func->FUNCTION.args[i];
            }
            else if (param->OP.op_type == NodeType::TRIPLE_DOT)
            {
                typechecker.symbol_table[param->right->ID.value] = func->FUNCTION.args[i];
            }

            // Typecheck arg against param type

            Typechecker::Match_Result match;

            if (func->FUNCTION.args[i]->type == NodeType::FUNCTION && func->FUNCTION.args[i]->FUNCTION.is_type)
            {
                match = match_types(func->FUNCTION.args[i], param_type);
            }
            else
            {
                match = match_types(param_type, func->FUNCTION.args[i]);
            }

            if (!match.result)
            {
                error_and_exit("[ArgTypeError] Function '" + func->FUNCTION.name + "': Cannot assign value of type '" + func->FUNCTION.args[i]->repr() + "' to parameter of type '" + param_type->repr() + "'. " + match.message);
            }
        }
    }

    int arg_index = 0;

    for (auto& param : func->FUNCTION.params)
    {
        auto arg = func->FUNCTION.args[arg_index];

        if (arg->type == NodeType::FUNCTION && arg->FUNCTION.is_type)
        {
            arg = arg->FUNCTION.params[0]->ID.type;
        }

        if (arg->type != NodeType::LIST && arg->type != NodeType::OBJECT && arg->type != NodeType::FUNCTION)
        {
            arg->TYPE.is_literal = false;
        }

        if (param->type == NodeType::ID)
        {
            param->ID.type = arg;
        }
        else if (param->OP.op_type == NodeType::EQUAL)
        {
            param->left->ID.type = arg;
        }
        else if (param->OP.op_type == NodeType::TRIPLE_DOT)
        {
            param->right->ID.type = arg;
        }

        arg_index++;
    }

    // check if we are recursing
    // if so, do not type anymore
    // just return return type

    std::string s = file_name;
    std::string delimiter = "/";

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(pos + 1, s.size());
        if (token == func->FUNCTION.name)
        {
            return func->FUNCTION.return_type;
        }
        s.erase(0, pos + delimiter.length());
    }

    std::shared_ptr<Node> return_val;

    if (func->FUNCTION.body->type == NodeType::BLOCK)
    {
        return_val = typechecker.eval_nodes(func->FUNCTION.body->BLOCK.nodes);
    }
    else
    {
        return_val = typechecker.eval_node(func->FUNCTION.body);
    }

    func->FUNCTION.return_type = return_val;
    func->TYPE.allowed_type = func;

    return func->FUNCTION.return_type;

    // if (typecheck && !func->FUNCTION.is_type)
    // {
    //     return func->FUNCTION.return_type;
    // }

    // if (func->FUNCTION.is_type && !func->FUNCTION.args[0]->TYPE.is_literal)
    // {
    //     return make_bool(true);
    // }

    // if (func->FUNCTION.body->type == NodeType::BLOCK)
    // {
    //     auto eval_expr = std::make_shared<Node>(NodeType::EMPTY);

    //     for (auto expr : func->FUNCTION.body->BLOCK.nodes)
    //     {
    //         eval_expr = typechecker.eval_node(expr);

    //         if (eval_expr->type == NodeType::RETURN)
    //         {
    //             if (func->FUNCTION.return_type->type != NodeType::EMPTY)
    //             {
    //                 eval_expr->right->TYPE.allowed_type = func->FUNCTION.return_type;
    //             }
    //             return eval_expr->right;
    //         }
    //     }

    //     return eval_expr;
    // }

    // auto res = typechecker.eval_node(func->FUNCTION.body);
    // if (func->FUNCTION.return_type->type != NodeType::EMPTY)
    // {
    //     res->TYPE.allowed_type = func->FUNCTION.return_type;
    // }
    // return res;
}

std::shared_ptr<Node> Typechecker::eval_if_statement(std::shared_ptr<Node>& node)
{
    auto res = std::make_shared<Node>(*node);

    auto conditional = eval_node(res->IF_STATEMENT.conditional);

    Typechecker if_eval;
    if_eval.runtime = runtime;
    if_eval.file_name = file_name;
    if_eval.outer_scope = std::make_shared<Typechecker>(*this);

    if (conditional->type == NodeType::ANY)
    {
        conditional = make_bool(true, true);
    }

    if (conditional->type != NodeType::BOOL)
    {
        error_and_exit("If statement expects a boolean conditional");
    }

    if (conditional->BOOL.value)
    {
        return if_eval.eval_nodes(node->IF_STATEMENT.body->BLOCK.nodes);
    }

    return std::make_shared<Node>(NodeType::EMPTY);
}

std::shared_ptr<Node> Typechecker::eval_if_block(std::shared_ptr<Node>& node)
{
    auto res = std::make_shared<Node>(*node);

    Typechecker if_eval;
    if_eval.runtime = runtime;
    if_eval.file_name = file_name;
    if_eval.outer_scope = std::make_shared<Typechecker>(*this);

    for (auto _case : res->IF_BLOCK.cases)
    {
        if (_case->OP.op_type != NodeType::COLON)
        {
            if (_case->type == NodeType::BLOCK)
            {
                return if_eval.eval_nodes(_case->BLOCK.nodes);
            }

            return if_eval.eval_node(_case);
        }

        auto cond = if_eval.eval_node(_case->left);

        if (cond->type != NodeType::BOOL)
        {
            error_and_exit("If block case expects a boolean conditional");
        }

        if (cond->BOOL.value)
        {
            if (_case->right->type == NodeType::BLOCK)
            {
                return if_eval.eval_nodes(_case->right->BLOCK.nodes);
            }

            return if_eval.eval_node(_case->right);
        }

        if_eval.symbol_table.clear();
    }

    return std::make_shared<Node>(NodeType::EMPTY);
}

bool Typechecker::match_pattern(std::shared_ptr<Node> a, std::shared_ptr<Node> b)
{
    if (a->OP.op_type == NodeType::DOUBLE_DOT)
    {
        auto& left = a->left;
        auto& right = a->right;

        if (b->type != NodeType::INT && b->type != NodeType::BOOL)
        {
            return false;
        }

        if (b->type == NodeType::INT && b->INT.value >= left->INT.value && b->INT.value < right->INT.value)
        {
            return true;
        }
        else if (b->type == NodeType::FLOAT && b->FLOAT.value >= left->FLOAT.value && b->FLOAT.value < right->FLOAT.value)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (a->type == NodeType::LIST)
    {
        if (b->type != NodeType::LIST)
        {
            return false;
        }

        auto left = std::make_shared<Node>(*a);

        if (b->LIST.nodes.size() == 0)
        {
            return left->LIST.nodes.size() == 0;
        }

        if (left->LIST.nodes.size() == 0)
        {
            return b->LIST.nodes.size() == 0;
        }

        int a_len = 0;

        for (int i = 0; i < left->LIST.nodes.size(); i++)
        {
            int index = 0;

            if (i >= left->LIST.nodes.size())
            {
                return false;
            }

            auto elem_a = left->LIST.nodes[i];
            if (elem_a->OP.op_type == NodeType::TRIPLE_DOT)
            {
                auto list = make_list();

                int num_elems_right = (a->LIST.nodes.size()-1) - i;
                left->LIST.nodes[i] = make_any();
                list->LIST.nodes.push_back(b->LIST.nodes[index]);
                int num_to_insert = b->LIST.nodes.size() - num_elems_right-i-1;
                for (int j = 0; j < num_to_insert; j++)
                {
                    left->LIST.nodes.insert(left->LIST.nodes.begin() + i, make_any());
                    list->LIST.nodes.push_back(b->LIST.nodes[index]);
                }

                if (elem_a->right != nullptr)
                {
                    symbol_table[elem_a->right->ID.value] = list;
                }

                elem_a = left->LIST.nodes[i];
            }
            if (elem_a->type == NodeType::ID)
            {
                // TODO: figure out better typing here

                // symbol_table[elem_a->ID.value] = make_any();
                // elem_a = make_any();

                if (elem_a->ID.value == "_")
                {
                    elem_a = make_any();
                }
                else
                {
                    symbol_table[elem_a->ID.value] = b->TYPE.allowed_type->LIST.nodes[0];
                    symbol_table[elem_a->ID.value]->TYPE.allowed_type = b->TYPE.allowed_type->LIST.nodes[0];
                    elem_a = b->TYPE.allowed_type->LIST.nodes[0];
                    elem_a->TYPE.allowed_type = b->TYPE.allowed_type->LIST.nodes[0];
                }
            }
            else if (elem_a->type == NodeType::BLOCK)
            {
                elem_a = eval_nodes(elem_a->BLOCK.nodes);
            }

            bool match = match_pattern(b->LIST.nodes[index], elem_a);

            if (!match)
            {
                return false;
            }

            a_len++;
        }

        if (a_len < b->LIST.nodes.size())
        {
            return false;
        }

        return true;
    }
    else if (a->type == NodeType::OBJECT && a->TYPE.name == "")
    {
        if (b->type == NodeType::ANY)
        {
            return true;
        }

        if (b->type != NodeType::OBJECT)
        {
            return false;
        }

        if (!b->TYPE.is_literal)
        {
            return true;
        }

        for (auto prop : a->OBJECT.properties)
        {
            auto value = prop.second.value;
            if (value->type != NodeType::TRIPLE_DOT)
            {
                value = eval_node(value);
            }

            // If prop doesn't exist, fail check

            if (b->OBJECT.properties.find(prop.first) == b->OBJECT.properties.end())
            {
                return false;
            }

            bool match;

            if (b->OBJECT.properties[prop.first].value->type == NodeType::FUNCTION)
            {
                b->OBJECT.properties[prop.first].value->FUNCTION.is_type = true;
                match = match_pattern(b->OBJECT.properties[prop.first].value, value);
            }
            else
            {
                match = match_pattern(value, b->OBJECT.properties[prop.first].value);
            }

            if (!match)
            {
                return false;
            }
        }

        return true;
    }
    else
    {
        return match_types(a, b).result;
    }

    return false;
}

std::shared_ptr<Node> Typechecker::eval_match_block(std::shared_ptr<Node>& node, std::string name)
{
    auto res = std::make_shared<Node>(*node);
    auto value = eval_node(res->MATCH_BLOCK.value);
    auto type = get_type(value);

    std::vector<std::shared_ptr<Node>> returns;

    if (name == "")
    {
        name = res->MATCH_BLOCK.value->ID.value;
    }

    if (type->type == NodeType::PIPE_LIST)
    {
        for (auto& elem : type->PIPE_LIST.nodes)
        {
            elem->is_type = true;
            auto match_block = std::make_shared<Node>(*node);
            match_block->MATCH_BLOCK.value = elem;
            auto match = eval_match_block(match_block, name);
            if (match->type == NodeType::PIPE_LIST)
            {
                for (auto elem : match->PIPE_LIST.nodes)
                {
                    returns.push_back(elem);
                }
            }
            else
            {
                returns.push_back(match);
            }
        }

        sort_and_unique(returns);

        if (returns.size() == 0)
        {
            return make_empty();
        }
        else if (returns.size() == 1)
        {
            return returns[0];
        }
        else
        {
            auto res = make_pipe_list();
            res->PIPE_LIST.nodes = returns;
            return res;
        }
    }
    if (type->type == NodeType::LIST)
    {
        bool has_empty_case = false;
        // Empty check
        for (auto& _case : node->MATCH_BLOCK.cases)
        {
            if (_case->OP.op_type == NodeType::COLON)
            {
                if (_case->left->type == NodeType::LIST && _case->left->LIST.nodes.size() == 0)
                {
                    has_empty_case = true;
                }
            }
            else
            {
                has_empty_case = true;
            }
        }

        if (!has_empty_case)
        {
            line = node->line;
            column = node->column;
            error_and_exit("Missing empty case for type 'list'");
        }
    }

    if (type->type == NodeType::BOOL && !type->TYPE.is_literal)
    {
        auto match_block_true = std::make_shared<Node>(*node);
        match_block_true->MATCH_BLOCK.value = make_bool(true);
        auto match_true = eval_match_block(match_block_true, name);

        auto match_block_false = std::make_shared<Node>(*node);
        match_block_false->MATCH_BLOCK.value = make_bool(true);
        auto match_false = eval_match_block(match_block_false, name);

        return make_bool(true, false);
    }

    for (auto _case : res->MATCH_BLOCK.cases)
    {
        if (_case->OP.op_type != NodeType::COLON)
        {
            auto res = eval_node(_case);
            res->TYPE.is_literal = false;
            returns.push_back(res);
        }
        else
        {
            auto pattern = _case->left;

            if (pattern->type == NodeType::ID)
            {
                pattern = eval_node(pattern);
            }

            bool match = match_pattern(pattern, value);

            if (match)
            {
                std::shared_ptr<Node> res = make_empty();

                symbol_table[name] = value;

                if (_case->right->type == NodeType::BLOCK)
                {
                    res = eval_nodes(_case->right->BLOCK.nodes);
                    res->TYPE.is_literal = false;
                }
                else
                {
                    res = eval_node(_case->right);
                    res->TYPE.is_literal = false;
                }

                // Cleanup, in case of list pattern with bindings

                for (auto elem : pattern->LIST.nodes)
                {
                    if (elem->type == NodeType::ID)
                    {
                        symbol_table.erase(elem->ID.value);
                    }
                }

                returns.push_back(res);
            }
        }
    }

    sort_and_unique(returns);

    if (returns.size() == 0)
    {
        error_and_exit("Missing match case for type '" + value->repr() + "'");
    }
    else if (returns.size() == 1)
    {
        return returns[0];
    }
    else
    {
        auto res = make_pipe_list();
        res->PIPE_LIST.nodes = returns;
        return res;
    }

    line = node->line;
    column = node->column;

    error_and_exit("Missing match case for type '" + value->repr() + "'");
}

std::shared_ptr<Node> Typechecker::eval_while_loop(std::shared_ptr<Node>& node)
{
    auto res = std::make_shared<Node>(*node);

    auto cond_node = node->WHILE_LOOP.conditional;

    auto cond = eval_node(cond_node);

    if (cond->type != NodeType::BOOL)
    {
        error_and_exit("While loop expects a boolean conditional");
    }

    Typechecker while_eval;
    while_eval.runtime = runtime;
    while_eval.file_name = file_name;
    while_eval.outer_scope = std::make_shared<Typechecker>(*this);

    while (while_eval.eval_node(cond_node)->BOOL.value)
    {
        auto expr = while_eval.eval_nodes(node->WHILE_LOOP.body->BLOCK.nodes);

        if (expr->type == NodeType::BREAK)
        {
            break;
        }

        if (expr->type == NodeType::CONTINUE)
        {
            continue;
        }

        if (expr->type == NodeType::RETURN)
        {
            return expr;
        }

        while_eval.symbol_table.clear();
    }

    return std::make_shared<Node>(NodeType::EMPTY);
}

std::shared_ptr<Node> Typechecker::eval_for_loop(std::shared_ptr<Node>& node)
{
    auto res = std::make_shared<Node>(*node);

    auto init_list = res->FOR_LOOP.init_list->LIST.nodes;

    auto iter = eval_node(init_list[0]);

    if (iter->type != NodeType::LIST && iter->type != NodeType::RANGE)
    {
        error_and_exit("For loop initialiser expects iterator of type list or range");
    }

    int begin = 0;
    int end = 0;

    if (iter->type == NodeType::LIST)
    {
        end = iter->LIST.nodes.size();
    }
    else
    {
        begin = iter->RANGE.start;
        end = iter->RANGE.end;
    }

    auto body = res->FOR_LOOP.body->BLOCK.nodes;

    Typechecker for_eval;
    for_eval.runtime = runtime;
    for_eval.file_name = file_name;
    for_eval.outer_scope = std::make_shared<Typechecker>(*this);

    if (init_list.size() == 1)
    {
        for (int i = begin; i < end; i++)
        {
            auto expr = for_eval.eval_nodes(body);

            if (expr->type == NodeType::BREAK)
            {
                break;
            }

            if (expr->type == NodeType::CONTINUE)
            {
                continue;
            }

            if (expr->type == NodeType::RETURN)
            {
                return expr;
            }

            for_eval.symbol_table.clear();
        }

        return std::make_shared<Node>(NodeType::EMPTY);
    }
    if (init_list.size() == 2)
    {
        auto index_var = init_list[1];

        if (index_var->type != NodeType::ID)
        {
            error_and_exit("For loop initialiser expects index argument to be an identifier");
        }

        auto index_var_name = index_var->ID.value;
        auto index_node = make_int(0);

        for (int i = begin; i < end; i++)
        {
            index_node->INT.value = i;
            for_eval.symbol_table[index_var_name] = index_node;

            auto expr = for_eval.eval_nodes(body);

            if (expr->type == NodeType::BREAK)
            {
                break;
            }

            if (expr->type == NodeType::CONTINUE)
            {
                continue;
            }

            if (expr->type == NodeType::RETURN)
            {
                return expr;
            }

            for_eval.symbol_table.clear();
        }

        for_eval.symbol_table.erase(index_var_name);

        return std::make_shared<Node>(NodeType::EMPTY);
    }
    if (init_list.size() == 3)
    {
        auto index_var = init_list[1];
        auto elem_var = init_list[2];

        if (index_var->type != NodeType::ID && elem_var->type != NodeType::ID)
        {
            error_and_exit("For loop initialiser expects index and element arguments to be identifiers");
        }

        auto index_var_name = index_var->ID.value;
        auto elem_var_name = elem_var->ID.value;

        for (int i = begin; i < end; i++)
        {
            auto index_node = std::make_shared<Node>(NodeType::INT);
            index_node->INT.value = i;
            for_eval.symbol_table[index_var_name] = index_node;

            if (iter->type == NodeType::LIST)
            {
                for_eval.symbol_table[elem_var_name] = iter->LIST.nodes[i];
            }
            else if (iter->type == NodeType::RANGE)
            {
                for_eval.symbol_table[elem_var_name] = index_node;
            }

            auto expr = for_eval.eval_nodes(body);

            if (expr->type == NodeType::BREAK)
            {
                break;
            }

            if (expr->type == NodeType::CONTINUE)
            {
                continue;
            }

            if (expr->type == NodeType::RETURN)
            {
                return expr;
            }

            for_eval.symbol_table.clear();
        }

        for_eval.symbol_table.erase(index_var_name);
        for_eval.symbol_table.erase(elem_var_name);

        return std::make_shared<Node>(NodeType::EMPTY);
    }
}

std::shared_ptr<Node> Typechecker::get_type(std::shared_ptr<Node>& node)
{
    if (node->TYPE.allowed_type != nullptr && node->TYPE.allowed_type->type != NodeType::TYPE)
    {
        return node->TYPE.allowed_type;
    }

    if (node->type == NodeType::LIST)
    {
        std::shared_ptr<Node> type = make_pipe_list();
        auto type_list = make_list();

        if (node->LIST.nodes.size() == 0)
        {
            type_list->LIST.nodes.push_back(make_any());
            return type_list;
        }

        for (auto& elem : node->LIST.nodes)
        {
            type->PIPE_LIST.nodes.push_back(get_type(elem));
        }

        sort_and_unique(type->PIPE_LIST.nodes);

        if (type->PIPE_LIST.nodes.size() == 1)
        {
            type = type->PIPE_LIST.nodes[0];
        }

        type_list->LIST.nodes.push_back(type);

        return type_list;
    }
    else if (node->type == NodeType::OBJECT)
    {
        auto type = make_object();
        type->OBJECT.is_type = true;
        type->TYPE.name = node->TYPE.name;

        for (auto& prop : node->OBJECT.properties)
        {
            type->OBJECT.properties[prop.first] = Object_Prop(prop.second.required, get_type(prop.second.value));
        }

        return type;
    }
    else if (node->type == NodeType::FUNCTION)
    {
        auto res = std::make_shared<Node>(*node);
        res->TYPE.is_literal = false;
        res->FUNCTION.is_type = true;
        return res;
    }
    else if (node->is_type)
    {
        return std::make_shared<Node>(*node);
    }
    else
    {
        auto res = std::make_shared<Node>(*node);
        res->TYPE.is_literal = false;
        return res;
    }
}

std::shared_ptr<Node> Typechecker::type_node(std::shared_ptr<Node> node, bool explicit_ret)
{
    auto returns = make_pipe_list();

    Typechecker eval;
    eval.runtime = runtime;
    eval.file_name = file_name;
    eval.outer_scope = std::make_shared<Typechecker>(*this);
    
    if (node->type == NodeType::IF_STATEMENT)
    {
        auto cond = eval.eval_node(node->IF_STATEMENT.conditional);

        if (node->IF_STATEMENT.conditional->type == NodeType::OP && (node->IF_STATEMENT.conditional->left->FUNC_CALL.name == "typeof" || node->IF_STATEMENT.conditional->left->FUNC_CALL.name == "__type__"))
        {
            if (node->IF_STATEMENT.conditional->OP.op_type != NodeType::EQ_EQ)
            {
                goto no_auto_typing;
            }
            if (node->IF_STATEMENT.conditional->left->FUNC_CALL.args[0]->type == NodeType::ID)
            {
                std::string name = node->IF_STATEMENT.conditional->left->FUNC_CALL.args[0]->ID.value;
                std::string type = node->IF_STATEMENT.conditional->right->STRING.value;

                auto type_node = make_empty(false);

                if (type == "int")
                {
                    type_node = make_int(0, false);
                }
                else if (type == "float")
                {
                    type_node = make_float(0, false);
                }
                else if (type == "bool")
                {
                    type_node = make_bool(true, false);
                }
                else if (type == "string")
                {
                    type_node = make_string("", false);
                }
                else if (type == "object")
                {
                    type_node = make_object(false);
                }
                else if (type == "list")
                {
                    type_node = make_list(false);
                }
                else if (type == "null")
                {
                    type_node = make_empty(false);
                }

                // check that the variable can be any of these
                auto var = eval.eval_look_up(name);
                auto match = match_types(var, type_node);

                if (!match.result)
                {
                    error_and_exit("Parameter '" + name + "' can only be of type '" + var->type_repr() + "'");
                }

                eval.symbol_table[name] = type_node;
            }
        }

        no_auto_typing:

        for (auto if_expr : node->IF_STATEMENT.body->BLOCK.nodes)
        {
            auto eval_expr = eval.type_node(if_expr, explicit_ret);
            for (auto expr: eval_expr->PIPE_LIST.nodes)
            {
                if (explicit_ret)
                    {
                        if (expr->type == NodeType::RETURN)
                        {
                            returns->PIPE_LIST.nodes.push_back(expr);
                            break;
                        }
                    }
                    else
                    {
                        if (expr->type == NodeType::RETURN)
                        {
                            returns->PIPE_LIST.nodes.push_back(expr);
                            break;
                        }
                        else
                        {
                            auto ret = std::make_shared<Node>(NodeType::RETURN);
                            ret->right = expr;
                            returns->PIPE_LIST.nodes.push_back(ret);
                            break;
                        }
                    }
            }
        }
    }
    else if (node->type == NodeType::IF_BLOCK)
    {
        for (auto cond : node->IF_BLOCK.cases)
        {
            if (cond->type == NodeType::OP && cond->OP.op_type == NodeType::COLON)
            {
                auto cond_expr = eval.eval_node(cond->left);
                auto eval_expr = eval.type_node(cond->right, explicit_ret);
                for (auto expr: eval_expr->PIPE_LIST.nodes)
                {
                    if (explicit_ret)
                    {
                        if (expr->type == NodeType::RETURN)
                        {
                            returns->PIPE_LIST.nodes.push_back(expr);
                            break;
                        }
                    }
                    else
                    {
                        if (expr->type == NodeType::RETURN)
                        {
                            returns->PIPE_LIST.nodes.push_back(expr);
                            break;
                        }
                        else
                        {
                            auto ret = std::make_shared<Node>(NodeType::RETURN);
                            ret->right = expr;
                            returns->PIPE_LIST.nodes.push_back(ret);
                            break;
                        }
                    }
                }
            }
            else
            {
                auto eval_expr = eval.type_node(cond, explicit_ret);
                for (auto expr: eval_expr->PIPE_LIST.nodes)
                {
                    if (explicit_ret)
                    {
                        if (expr->type == NodeType::RETURN)
                        {
                            returns->PIPE_LIST.nodes.push_back(expr);
                            break;
                        }
                    }
                    else
                    {
                        if (expr->type == NodeType::RETURN)
                        {
                            returns->PIPE_LIST.nodes.push_back(expr);
                            break;
                        }
                        else
                        {
                            auto ret = std::make_shared<Node>(NodeType::RETURN);
                            ret->right = expr;
                            returns->PIPE_LIST.nodes.push_back(ret);
                            break;
                        }
                    }
                }
            }

            eval.symbol_table.clear();
        }
    }
    else if (node->type == NodeType::FOR_LOOP)
    {    
        auto index_node = make_int(0);
        auto init_list = node->FOR_LOOP.init_list->LIST.nodes;
        auto iter = eval.eval_node(init_list[0]);
        auto item_node = make_int(0);
        if (iter->type == NodeType::LIST)
        {
            auto type = get_type(iter);
            if (type->LIST.nodes.size() == 0)
            {
                item_node = make_any();
            }
            else
            {
                item_node = type->LIST.nodes[0];
            }
        }

        if (init_list.size() == 2)
        {
            eval.symbol_table[init_list[1]->ID.value] = index_node;
        }
        if (init_list.size() == 3)
        {
            eval.symbol_table[init_list[1]->ID.value] = index_node;
            eval.symbol_table[init_list[2]->ID.value] = item_node;
        }

        for (auto for_expr : node->FOR_LOOP.body->BLOCK.nodes)
        {
            auto eval_expr = eval.type_node(for_expr, explicit_ret);
            for (auto expr: eval_expr->PIPE_LIST.nodes)
            {
                if (expr->type == NodeType::RETURN)
                {
                    returns->PIPE_LIST.nodes.push_back(expr);
                    break;
                }
            }
        }
    }
    else if (node->type == NodeType::WHILE_LOOP)
    {
        auto iter = eval.eval_node(node->WHILE_LOOP.conditional);

        for (auto while_expr : node->WHILE_LOOP.body->BLOCK.nodes)
        {
            auto eval_expr = type_node(while_expr, explicit_ret);
            for (auto expr: eval_expr->PIPE_LIST.nodes)
            {
                if (expr->type == NodeType::RETURN)
                {
                    returns->PIPE_LIST.nodes.push_back(expr);
                    break;
                }
            }
        }
    }
    else if (node->type == NodeType::FUNCTION)
    {
        return type_func(node);
    }
    else if (node->type == NodeType::MATCH_BLOCK)
    {
        auto res = eval_match_block(node);

        if (res->type == NodeType::PIPE_LIST)
        {
            for (auto expr: res->PIPE_LIST.nodes)
            {
                if (explicit_ret)
                {
                    if (expr->type == NodeType::RETURN)
                    {
                        returns->PIPE_LIST.nodes.push_back(expr);
                    }
                }
                else
                {
                    if (expr->type == NodeType::RETURN)
                    {
                        returns->PIPE_LIST.nodes.push_back(expr);
                    }
                    else
                    {
                        auto ret = std::make_shared<Node>(NodeType::RETURN);
                        ret->right = expr;
                        returns->PIPE_LIST.nodes.push_back(ret);
                    }
                }
            }
        }
        else
        {
            if (explicit_ret)
            {
                if (res->type == NodeType::RETURN)
                {
                    returns->PIPE_LIST.nodes.push_back(res);
                }
            }
            else
            {
                if (res->type == NodeType::RETURN)
                {
                    returns->PIPE_LIST.nodes.push_back(res);
                }
                else
                {
                    auto ret = std::make_shared<Node>(NodeType::RETURN);
                    ret->right = res;
                    returns->PIPE_LIST.nodes.push_back(ret);
                }
            }
        }
    }
    else
    {
        auto eval_expr = eval_node(node);
        returns->PIPE_LIST.nodes.push_back(eval_expr);
    }

    // if (returns->PIPE_LIST.nodes.size() == 0)
    // {
    //     auto ret = std::make_shared<Node>(NodeType::RETURN);
    //     ret->right = make_empty();
    //     returns->PIPE_LIST.nodes.push_back(ret);
    // }

    return returns;
}

std::shared_ptr<Node> Typechecker::type_func(std::shared_ptr<Node> node, bool literal_values)
{
    std::string s = file_name;
    std::string delimiter = "/";

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(pos + 1, s.size());
        if (token == node->FUNCTION.name)
        {
            return node->FUNCTION.return_type;
        }
        s.erase(0, pos + delimiter.length());
    }

    Typechecker func_eval;
    func_eval.runtime = false;
    func_eval.typecheck = true;
    func_eval.builtins_path = builtins_path;
    func_eval.builtins_names = builtins_names;
    func_eval.file_name = file_name + "/" + node->FUNCTION.name;

    for (auto elem : node->FUNCTION.closure)
    {
        func_eval.symbol_table[elem.first] = elem.second;
    }

    for (auto param : node->FUNCTION.params) 
    {
        auto arg = get_explicit_type(param);
        if (arg->type == NodeType::OBJECT && !arg->OBJECT.is_type)
        {
            //error_and_exit("Cannot use literal objects as parameter types.");
            arg = get_type(arg);
        }
        if (arg->type == NodeType::PIPE_LIST)
        {
            for (auto& t : arg->PIPE_LIST.nodes)
            {
                if (t->type == NodeType::OBJECT && !t->OBJECT.is_type)
                {
                    //error_and_exit("Cannot use literal objects as parameter types.");
                    t = get_type(t);
                }
            }
        }
        if (arg->type == NodeType::FUNCTION && arg->FUNCTION.is_type)
        {
            arg = get_explicit_type(arg->FUNCTION.params[0]);
        }

        arg->TYPE.allowed_type = std::make_shared<Node>(*arg);

        if (param->type == NodeType::ID)
        {
            func_eval.symbol_table[param->ID.value] = arg;
        }
        else if (param->OP.op_type == NodeType::EQUAL)
        {
            func_eval.symbol_table[param->left->ID.value] = arg;
        }
        else
        {
            func_eval.symbol_table[param->right->ID.value] = arg;
        }
    }

    std::vector<std::shared_ptr<Node>> returns;

    int index = 0;
    int body_size = node->FUNCTION.body->BLOCK.nodes.size();
    int num_if_exprs = 0;
    std::shared_ptr<Node> last_expr = make_any();

    if (node->FUNCTION.body->type != NodeType::BLOCK)
    {
        body_size = 1;

        if (node->FUNCTION.body->type == NodeType::IF_STATEMENT || node->FUNCTION.body->type == NodeType::IF_BLOCK)
        {
            num_if_exprs = 1;
            last_expr = node->FUNCTION.body;
        }

        auto res = func_eval.type_node(node->FUNCTION.body);
        for (auto n : res->PIPE_LIST.nodes)
        {
            if (n->type == NodeType::RETURN)
            {
                returns.push_back(n);
            }
            else
            {
                auto ret = std::make_shared<Node>(NodeType::RETURN);
                ret->right = n;
                returns.push_back(ret);
            }
        }
    }
    else
    {
        for (auto expr : node->FUNCTION.body->BLOCK.nodes)
        {
            if (expr->type == NodeType::IF_STATEMENT)
            {
                // if (literal_values)
                // {
                //     auto cond_check = func_eval.eval_node(expr->IF_STATEMENT.conditional);

                //     if (!cond_check->BOOL.value)
                //     {
                //         continue;
                //     }
                // }

                num_if_exprs++;
                bool explicit_ret = true;

                if (index == body_size-1)
                {
                    last_expr = expr;
                    explicit_ret = false;
                }

                auto ret = func_eval.type_node(expr, explicit_ret);
                for (auto& n : ret->PIPE_LIST.nodes)
                {
                    returns.push_back(n);
                }
            }
            else if (expr->type == NodeType::IF_BLOCK)
            {
                num_if_exprs++;
                bool explicit_ret = true;

                if (index == body_size-1)
                {
                    last_expr = expr;
                    explicit_ret = false;
                }

                auto ret = func_eval.type_node(expr, explicit_ret);
                for (auto& n : ret->PIPE_LIST.nodes)
                {
                    returns.push_back(n);
                }
            }
            else if (expr->type == NodeType::MATCH_BLOCK)
            {
                bool explicit_ret = true;

                if (index == body_size-1)
                {
                    last_expr = expr;
                    explicit_ret = false;
                }

                auto ret = func_eval.type_node(expr, explicit_ret);
                for (auto& n : ret->PIPE_LIST.nodes)
                {
                    returns.push_back(n);
                }
            }
            else
            {
                auto eval_expr = func_eval.type_node(expr, true);
                for (auto& n : eval_expr->PIPE_LIST.nodes)
                {
                    if (n->type == NodeType::RETURN || index == body_size-1)
                    {
                        if (n->type != NodeType::RETURN)
                        {
                            auto ret = std::make_shared<Node>(NodeType::RETURN);
                            ret->right = n;
                            returns.push_back(ret);
                            break;
                        }

                        returns.push_back(n);
                        break;
                    }
                }
            }

            index++;
        }
    }

    if (num_if_exprs == body_size || last_expr->type == NodeType::IF_STATEMENT || last_expr->type == NodeType::IF_BLOCK)
    {
        auto ret = std::make_shared<Node>(NodeType::RETURN);
        ret->right = make_empty();
        returns.push_back(ret);
    }

    auto pipe_list = make_pipe_list();
    for (auto& ret : returns)
    {
        auto res = ret->right;
        if (res->type != NodeType::LIST && res->type != NodeType::OBJECT && res->type != NodeType::FUNCTION)
        {
            res->TYPE.is_literal = false;
        }
        if (res->type == NodeType::ANY)
        {
            pipe_list->PIPE_LIST.nodes.clear();
            pipe_list->PIPE_LIST.nodes.push_back(res);
            break;
        }
        pipe_list->PIPE_LIST.nodes.push_back(res);
    }

    sort_and_unique(pipe_list->PIPE_LIST.nodes);

    for (auto& ret : pipe_list->PIPE_LIST.nodes)
    {
        if (ret->type == NodeType::OBJECT)
        {
            if (ret->OBJECT.properties.size() == 0)
            {
                ret->TYPE.is_literal = false;
            }
        }
        if (ret->type == NodeType::LIST)
        {
            if (ret->LIST.nodes.size() == 0)
            {
                ret->TYPE.is_literal = false;
            }
        }
    }

    if (pipe_list->PIPE_LIST.nodes.size() == 0)
    {
        pipe_list->PIPE_LIST.nodes.push_back(make_empty(false));
    }

    if (pipe_list->PIPE_LIST.nodes.size() == 1)
    {
        pipe_list = pipe_list->PIPE_LIST.nodes[0];
    }

    if (node->FUNCTION.return_type != nullptr)
    {
        // check that the actual type matches explicit type
        if (pipe_list->type == NodeType::OBJECT && pipe_list->OBJECT.properties.size() == 1)
        {
            int x = 12;
        }
        auto match = match_types(node->FUNCTION.return_type, pipe_list);
        if (!match.result)
        {
            error_and_exit("Function '" + node->FUNCTION.name + "' expects to return a value of type '" + node->FUNCTION.return_type->type_repr() + "' but instead returns value of type '" + pipe_list->type_repr() + "'");
        }

        node->FUNCTION.return_type = pipe_list;       
    }
    else
    {
        node->FUNCTION.return_type = pipe_list;
    }

    auto ret_list = make_pipe_list();
    auto ret = std::make_shared<Node>(NodeType::RETURN);
    ret->right = node;
    ret_list->PIPE_LIST.nodes.push_back(ret);

    node->FUNCTION.typechecked = true;
    node->TYPE.allowed_type = node;

    return ret_list;
}

std::shared_ptr<Node> Typechecker::eval_node(std::shared_ptr<Node>& node)
{
    if (node == nullptr)
    {
        return node;
    }

    line = node->line;
    column = node->column;

    if (node->type == NodeType::RETURN)
    {
        return eval_node(node->right);
    }

    if (node->type == NodeType::INT || node->type == NodeType::FLOAT || node->type == NodeType::BOOL || node->type == NodeType::STRING || node->type == NodeType::EMPTY)
    {
        return std::make_shared<Node>(*node);
    }

    if (node->type == NodeType::OBJECT)
    {
        auto res = std::make_shared<Node>(*node);

        for (auto& prop : res->OBJECT.properties)
        {
            if (prop.second.value->type == NodeType::FUNCTION)
            {
                prop.second.value->FUNCTION.closure["this"] = res;
            }

            prop.second.value = eval_node(prop.second.value);
            if (prop.second.value->type != NodeType::LIST && 
                prop.second.value->type != NodeType::PIPE_LIST &&
                prop.second.value->type != NodeType::FUNCTION &&
                prop.second.value->type != NodeType::OBJECT )
            {
                prop.second.value->TYPE.is_literal = false;
            }
        }

        return res;
    }

    if (node->type == NodeType::BLOCK && node->BLOCK.nodes.size() == 0)
    {
        auto res = std::make_shared<Node>(*node);
        res->type = NodeType::OBJECT;

        return res;
    }

    if (node->type == NodeType::LIST)
    {
        auto res = make_list();

        if (!node->TYPE.is_literal)
        {
            res->LIST.nodes.push_back(make_any());
            res->TYPE.allowed_type = res;
            return res;
        }

        auto type = make_pipe_list();

        for (auto elem : node->LIST.nodes)
        {
            auto eval_elem = eval_node(elem);
            if (eval_elem->type != NodeType::LIST && 
                eval_elem->type != NodeType::PIPE_LIST &&
                eval_elem->type != NodeType::OBJECT )
            {
                eval_elem->TYPE.is_literal = false;
            }
            type->PIPE_LIST.nodes.push_back(eval_elem);
        }

        sort_and_unique(type->PIPE_LIST.nodes);

        if (type->PIPE_LIST.nodes.size() == 0)
        {
            res->LIST.nodes.push_back(make_any());
        }
        else if (type->PIPE_LIST.nodes.size() == 1)
        {
            res->LIST.nodes.push_back(type->PIPE_LIST.nodes[0]);
        }
        else
        {
            res->LIST.nodes.push_back(type);
        }

        res->TYPE.allowed_type = res;

        return res;
    }

    if (node->type == NodeType::COMMA_LIST)
    {
        auto res = std::make_shared<Node>(*node);
        res->COMMA_LIST.nodes.clear();

        for (auto elem : node->COMMA_LIST.nodes)
        {
            res->COMMA_LIST.nodes.push_back(eval_node(elem));
        }

        return res;
    }

    if (node->type == NodeType::PIPE_LIST)
    {
        auto res = std::make_shared<Node>(*node);
        res->PIPE_LIST.nodes.clear();

        for (auto elem : node->PIPE_LIST.nodes)
        {
            res->PIPE_LIST.nodes.push_back(eval_node(elem));
        }

        return res;
    }

    if (node->type == NodeType::ID)
    {
        auto& var = eval_look_up(node->ID.value);

        if (var->type == NodeType::ERROR)
        {
            auto type = get_explicit_type(node);

            if (type->type == NodeType::TYPE)
            {
                auto val = make_object();
                val->TYPE.allowed_type = type;
                val->TYPE.name = node->ID.value;
                val->OBJECT.is_type = true;

                symbol_table[node->ID.value] = val;
                return val;
            }

            error_and_exit("Undefined variable '" + node->ID.value + "'");
        }
        else
        {
            if (node->ID.type != nullptr)
            {
                auto explicit_type = eval_node(node->ID.type);
                auto match = match_types(var->TYPE.allowed_type, explicit_type);

                if (!match.result)
                {
                    error_and_exit("Cannot cast type from '" + var->TYPE.allowed_type->type_repr() + "' to '" + explicit_type->type_repr() + "'");
                }

                var = explicit_type;
                var->TYPE.allowed_type = explicit_type;
            }
        }

        if (var->type == NodeType::PIPE_LIST && var->PIPE_LIST.nodes.size() == 1)
        {
            return var->PIPE_LIST.nodes[0];
        }

        if (var->type == NodeType::FUNC_LIST && var->FUNC_LIST.nodes.size() == 1)
        {
            return var->FUNC_LIST.nodes[0];
        }

        return var;
    }

    if (node->type == NodeType::COPY)
    {
        return eval_copy(node->right);
    }

    if (node->type == NodeType::OP)
    {
        if (node->OP.op_type == NodeType::DOUBLE_ARROW)
        {
            return eval_double_arrow(node);
        }
        if (node->OP.op_type == NodeType::DOUBLE_DOT)
        {
            return eval_double_dot(node);
        }
        if (node->OP.op_type == NodeType::PLUS)
        {
            return eval_add(node);
        }
        if (node->OP.op_type == NodeType::MINUS)
        {
            return eval_sub(node);
        }
        if (node->OP.op_type == NodeType::STAR)
        {
            return eval_mul(node);
        }
        if (node->OP.op_type == NodeType::SLASH)
        {
            return eval_div(node);
        }
        if (node->OP.op_type == NodeType::PERCENT)
        {
            return eval_mod(node);
        }
        if (node->OP.op_type == NodeType::POS || node->OP.op_type == NodeType::NEG)
        {
            return eval_pos_neg(node);
        }
        if (node->OP.op_type == NodeType::NOT)
        {
            return eval_not(node);
        }
        if (node->OP.op_type == NodeType::L_ANGLE
        ||  node->OP.op_type == NodeType::R_ANGLE
        ||  node->OP.op_type == NodeType::LT_EQUAL
        ||  node->OP.op_type == NodeType::GT_EQUAL
        ||  node->OP.op_type == NodeType::EQ_EQ
        ||  node->OP.op_type == NodeType::NOT_EQUAL)
        {
            return eval_equality(node);
        }
        if (node->OP.op_type == NodeType::AND
        ||  node->OP.op_type == NodeType::OR)
        {
            return eval_and_or(node);
        }
        if (node->OP.op_type == NodeType::DOT)
        {
            return eval_dot(node);
        }
        if (node->OP.op_type == NodeType::EQUAL)
        {
            return eval_equal(node);
        }
        if (node->OP.op_type == NodeType::PLUS_EQ || node->OP.op_type == NodeType::MINUS_EQ)
        {
            auto op_node = std::make_shared<Node>(NodeType::OP);
            op_node->left = eval_node(node->left);
            op_node->right = eval_node(node->right);

            auto op_res = node->OP.op_type == NodeType::PLUS_EQ ? eval_add(op_node) : eval_sub(op_node);

            auto equal_node = std::make_shared<Node>(NodeType::OP);
            equal_node->left = node->left;
            equal_node->right = op_res;

            return eval_equal(equal_node);
        }

        return node;
    }

    if (node->type == NodeType::BLOCK)
    {
        Typechecker evaluator;
        evaluator.runtime = runtime;
        evaluator.outer_scope = std::make_shared<Typechecker>(*this);
        evaluator.line = line;
        evaluator.column = column;
        evaluator.file_name = file_name;
        evaluator.nodes = node->BLOCK.nodes;

        evaluator.evaluate();

        if (evaluator.nodes.size() == 0)
        {
            return std::make_shared<Node>(NodeType::EMPTY);
        }

        return evaluator.nodes[evaluator.nodes.size()-1];
    }
    if (node->type == NodeType::FUNCTION)
    {
        for (auto& arg : node->FUNCTION.args)
        {
            if (arg != nullptr)
            {
                arg = eval_node(arg);
            }
        }

        for (auto& param : node->FUNCTION.params)
        {
            if (param->type == NodeType::ID)
            {
                if (param->ID.type == nullptr)
                {
                    param->ID.type = make_any();
                }
                else
                {
                    param->ID.type = eval_node(param->ID.type);
                }

                if (param->ID.value == "__type")
                {
                    node->FUNCTION.is_type = true;
                }
            }
            else if (param->type == NodeType::EQUAL)
            {
                if (param->left->ID.type == nullptr)
                {
                    param->left->ID.type = make_any();
                }
                else
                {
                    param->left->ID.type = eval_node(param->left->ID.type);
                }
            }
            else if (param->type == NodeType::TRIPLE_DOT)
            {
                if (param->right->ID.type == nullptr)
                {
                    param->right->ID.type = make_any();
                }
                else
                {
                    param->right->ID.type = eval_node(param->right->ID.type);
                }
            }
        }

        for (auto symbol : symbol_table)
        {
            node->FUNCTION.closure[symbol.first] = symbol.second;
        }

        if (node->FUNCTION.closure.find(node->FUNCTION.name) != node->FUNCTION.closure.end())
        {
            if (node->FUNCTION.closure[node->FUNCTION.name]->type == NodeType::FUNC_LIST)
            {
                node->FUNCTION.closure[node->FUNCTION.name] = std::make_shared<Node>(*node->FUNCTION.closure[node->FUNCTION.name]);
                node->FUNCTION.closure[node->FUNCTION.name]->FUNC_LIST.nodes.push_back(node);
            }
        }
        else
        {
            node->FUNCTION.closure[node->FUNCTION.name] = node;
        }

        if (!node->TYPE.is_literal)
        {
            return node;
        }

        type_func(node);

        if (node->FUNCTION.return_type->PIPE_LIST.nodes.size() == 1)
        {
            node->FUNCTION.return_type = node->FUNCTION.return_type->PIPE_LIST.nodes[0];
        }

        if (typecheck && node->FUNCTION.is_type)
        {
            return get_explicit_type(node->FUNCTION.params[0]);
        }
    }
    if (node->type == NodeType::FUNC_CALL)
    {
        auto func_name = node->FUNC_CALL.name;

        // Built-in functions

        if (tc_builtin_functions.find(func_name) != tc_builtin_functions.end())
        {
            auto func = tc_builtin_functions[func_name];
            return (this->*func)(node->FUNC_CALL.args);
        }

        auto func = eval_look_up(func_name);

        if (func->type == NodeType::ERROR)
        {
            if (node->FUNC_CALL.caller->type == NodeType::FUNCTION)
            {
                func = eval_node(node->FUNC_CALL.caller);
            }
            else if (node->FUNC_CALL.caller->OP.op_type == NodeType::DOT)
            {
                func = eval_node(node->FUNC_CALL.caller);
            }
            else
            {
                error_and_exit("Function '" + func_name + "' is undefined");
            }
        }

        if (func_name == "import")
        {
            // typecheck only
            eval_function_call(func, node->FUNC_CALL.args);
            return builtin_import({node->FUNC_CALL.args});
        }

        return eval_function_call(func, node->FUNC_CALL.args);
    }
    if (node->type == NodeType::IF_STATEMENT)
    {
        return eval_if_statement(node);
    }
    if (node->type == NodeType::IF_BLOCK)
    {
        return eval_if_block(node);
    }
    if (node->type == NodeType::MATCH_BLOCK)
    {
        return eval_match_block(node);
    }
    if (node->type == NodeType::WHILE_LOOP)
    {
        return eval_while_loop(node);
    }
    if (node->type == NodeType::FOR_LOOP)
    {
        return eval_for_loop(node);
    }

    if (node->type == NodeType::KEYWORD && node->ID.value == "break")
    {
        auto res = std::make_shared<Node>(NodeType::BREAK);
        return res;
    }
    if (node->type == NodeType::KEYWORD && node->ID.value == "continue")
    {
        auto res = std::make_shared<Node>(NodeType::CONTINUE);
        return res;
    }
    if (node->type == NodeType::KEYWORD && node->ID.value == "ret")
    {
        auto res = std::make_shared<Node>(NodeType::RETURN);
        res->right = eval_node(node->right);
        return res;
    }

    return node;
}

std::shared_ptr<Node> Typechecker::eval_nodes(std::vector<std::shared_ptr<Node>> nodes)
{
    auto eval_expr = std::make_shared<Node>(NodeType::EMPTY);

    for (auto expr : nodes)
    {
        eval_expr = eval_node(expr);

        if (eval_expr->type == NodeType::BREAK || eval_expr->type == NodeType::CONTINUE || eval_expr->type == NodeType::RETURN)
        {
            return eval_expr;
        }
    }

    return eval_expr;
}

void Typechecker::evaluate()
{
    std::vector<NodeType> ignore_list = { NodeType::SEMICOLON };

    for (auto& node : nodes)
    {
        if (
            std::find(ignore_list.begin(), ignore_list.end(), node->type) != ignore_list.end() || 
            std::find(ignore_list.begin(), ignore_list.end(), node->OP.op_type) != ignore_list.end()
            )
        {
            continue;
        }

        line = node->line;
        column = node->column;
        auto res = eval_node(node);
    }
}

void Typechecker::error_and_exit(std::string message)
{
    std::string type = "Type";

    std::string error_message = "\n\n" + type + " Error in '" + file_name + "' @ (" + std::to_string(line) + ", " + std::to_string(column) + "): " + message;
	std::cout << error_message << "\n";

    if (outer_scope != nullptr)
    {
        outer_scope->error_and_exit(message);
    }

    exit(1);
}