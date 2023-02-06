#include "Node.hpp"

std::string repr_obj(std::shared_ptr<Node> obj, int depth = 0) {
                
    if (obj->type != NodeType::OBJECT)
    {
        return obj->repr();
    }

    std::string repr = "{\n";
    for (auto elem : obj->OBJECT.properties)
    {
        if (elem.second.value->type == NodeType::OBJECT)
        {
            depth++;
        }

        for (int i = 0; i < depth; i++)
        {
            repr += " ";
        }

        repr += ("  " + elem.first + ": " + repr_obj(elem.second.value, depth) + "\n");
    }
    repr += "}";
    return repr;
};

std::string Node::repr()
{
    switch(type)
    {
        case NodeType::ID:
        {
            return ID.value;
        }
        case NodeType::TYPE:
        {
            return TYPE.name;
        }
        case NodeType::BOOL:
        {
            if (!TYPE.is_literal)
            {
                return "bool";
            }

            return (BOOL.value ? "true" : "false");
        }
        case NodeType::INT:
        {
            if (!TYPE.is_literal)
            {
                return "int";
            }

            return std::to_string(INT.value);
        }
        case NodeType::FLOAT:
        {
            if (!TYPE.is_literal)
            {
                return "float";
            }

            return std::to_string(FLOAT.value);
        }
        case NodeType::STRING:
        {
            if (!TYPE.is_literal)
            {
                return "string";
            }

            return STRING.value;
        }
        case NodeType::OP:
        {
            return OP.value;
        }
        case NodeType::PARTIAL_OP:
        {
            return "..." + OP.value;
        }
        case NodeType::EMPTY:
        {
            return "null";
        }
        case NodeType::ANY:
        {
            return "any";
        }
        case NodeType::LIST:
        {
            if (!TYPE.is_literal)
            {
                return "list";
            }

            std::string repr = "[ ";
            for (auto elem : LIST.nodes)
            {
                std::string elem_repr = "";
                if (elem->type == NodeType::STRING && elem->TYPE.is_literal)
                {
                    elem_repr = "\"" + elem->repr() + "\"";
                }
                else
                {
                    elem_repr = elem->repr();
                }
                
                repr += (elem_repr + " ");
            }
            repr += "]";
            return repr;
        }
        case NodeType::COMMA_LIST:
        {
            std::string repr = "( ";
            for (auto elem : COMMA_LIST.nodes)
            {
                repr += (elem->repr() + " ");
            }
            repr += ")";
            return repr;
        }
        case NodeType::PIPE_LIST:
        {
            std::string repr = "";
            for (int i = 0; i < PIPE_LIST.nodes.size(); i++)
            {
                auto elem = PIPE_LIST.nodes[i];
                repr += elem->repr();
                if (i < PIPE_LIST.nodes.size()-1)
                {
                    repr += " | ";
                }
            }
            return repr;
        }
        case NodeType::OBJECT:
        {
            if (!TYPE.is_literal)
            {
                return "object";
            }

            if (TYPE.name != "")
            {
                return TYPE.name;
            }

            // return repr_obj(std::make_shared<Node>(*this));

            std::string repr = "{\n";
            for (auto elem : OBJECT.properties)
            {
                repr += ("  " + elem.first + ": " + elem.second.value->repr() + "\n");
            }
            repr += "}";
            return repr;
        }
        case NodeType::FUNCTION:
        {
            if (!TYPE.is_literal)
            {
                return "function";
            }

            if (FUNCTION.is_type && FUNCTION.name != "lambda")
            {
                return FUNCTION.name;
            }

            std::string repr = "[ ";
            for (auto param : FUNCTION.params)
            {
                std::string name;

                if (param->type == NodeType::ID)
                {
                    name = param->ID.value;
                    if (param->ID.type != nullptr)
                    {
                        name += "::" + param->ID.type->repr();
                    }
                    else
                    {
                        name += "::any";
                    }
                }
                else if (param->OP.op_type == NodeType::TRIPLE_DOT)
                {
                    name = "..." + param->right->ID.value;
                    if (param->right->ID.type != nullptr)
                    {
                        name += "::" + param->right->ID.type->repr();
                    }
                    else
                    {
                        name += "::any";
                    }
                }
                else if (param->OP.op_type == NodeType::EQUAL)
                {
                    name = param->left->ID.value;
                    if (param->left->ID.type != nullptr)
                    {
                        name += "::" + param->left->ID.type->repr();
                    }
                    else
                    {
                        name += "::any";
                    }
                }
                repr += (name + " ");
                
            }

            auto ret_type = FUNCTION.return_type;
            repr += "] => " + (ret_type == nullptr ? "any" : ret_type->type_repr()) ;

            return repr;
        }
        case NodeType::RANGE:
        {
            std::string repr = "(" + std::to_string(RANGE.start) + ", " + std::to_string(RANGE.end) + ")";
            return repr;
        }
        case NodeType::BLOCK:
        {
            return "{ block }";
        }
        default:
        {
            return "<no repr>";
            break;
        }
    }
}

std::string Node::type_repr()
{
    switch(type)
    {
        case NodeType::ID:
        {
            return ID.value;
        }
        case NodeType::BOOL:
        {
            return "bool";
        }
        case NodeType::INT:
        {
            return "int";
        }
        case NodeType::FLOAT:
        {
            return "float";
        }
        case NodeType::STRING:
        {
            return "string";
        }
        case NodeType::OP:
        {
            return OP.value;
        }
        case NodeType::PARTIAL_OP:
        {
            return "_" + OP.value + "_";
        }
        case NodeType::EMPTY:
        {
            return "null";
        }
        case NodeType::ANY:
        {
            return "any";
        }
        case NodeType::LIST:
        {
            std::set<std::string> types;
            if (TYPE.allowed_type == nullptr)
            {
                for (auto elem : LIST.nodes)
                {
                    types.insert(elem->type_repr());
                }
            }
            else
            {
                for (auto elem : TYPE.allowed_type->LIST.nodes)
                {
                    types.insert(elem->type_repr());
                }
            }
            std::string repr = "[ ";
            for (int i = 0; i < types.size(); i++)
            {
                repr += (*std::next(types.begin(), i) + " ");
                if (i != types.size()-1)
                {
                    repr += "| ";
                }
            }
            repr += "]";
            return repr;
        }
        case NodeType::COMMA_LIST:
        {
            return "comma_list";
        }
        case NodeType::PIPE_LIST:
        {
            std::string repr = "";
            for (int i = 0; i < PIPE_LIST.nodes.size(); i++)
            {
                auto elem = PIPE_LIST.nodes[i];
                repr += elem->repr();
                if (i < PIPE_LIST.nodes.size()-1)
                {
                    repr += " | ";
                }
            }
            return repr;
        }
        case NodeType::OBJECT:
        {
            if (TYPE.name != "")
            {
                return TYPE.name;
            }

            std::string repr = "{\n";
            for (auto elem : OBJECT.properties)
            {
                repr += ("  " + elem.first + ": " + elem.second.value->repr() + "\n");
            }
            repr += "}";
            return repr;
        }
        case NodeType::FUNCTION:
        {
            if (FUNCTION.is_type && FUNCTION.name != "lambda")
            {
                return FUNCTION.name;
            }
            
            std::string repr = "[ ";
            for (auto param : FUNCTION.params)
            {
                std::string name;

                if (param->type == NodeType::ID)
                {
                    name = param->ID.value;
                    if (param->ID.type != nullptr)
                    {
                        name += "::" + param->ID.type->repr();
                    }
                    else
                    {
                        name += "::any";
                    }
                }
                else if (param->OP.op_type == NodeType::TRIPLE_DOT)
                {
                    name = "..." + param->right->ID.value;
                    if (param->right->ID.type != nullptr)
                    {
                        name += "::" + param->right->ID.type->repr();
                    }
                    else
                    {
                        name += "::any";
                    }
                }
                else if (param->OP.op_type == NodeType::EQUAL)
                {
                    name = param->left->ID.value;
                    if (param->left->ID.type != nullptr)
                    {
                        name += "::" + param->left->ID.type->repr();
                    }
                    else
                    {
                        name += "::any";
                    }
                }
                repr += (name + " ");
                
            }

            auto ret_type = FUNCTION.return_type;
            if (ret_type == nullptr)
            {
                ret_type = std::make_shared<Node>(NodeType::ANY);
            }

            ret_type->TYPE.is_literal = false;

            if (ret_type->type == NodeType::PIPE_LIST)
            {
                for (auto& t : ret_type->PIPE_LIST.nodes)
                {
                    t->TYPE.is_literal = false;
                }
            }
            
            repr += "] => " + ret_type->repr();

            return repr;
        }
        case NodeType::RANGE:
        {
            std::string repr = "(" + std::to_string(RANGE.start) + ", " + std::to_string(RANGE.end) + ")";
            return repr;
        }
        case NodeType::BLOCK:
        {
            return "{ block }";
        }
        default:
        {
            return "<no repr>";
            break;
        }
    }
}
