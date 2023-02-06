#pragma once
#include <sstream>
#include <fstream>
#include <cmath>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <numeric>
#include "../Parser/Parser.hpp"
#include "../Typechecker/Typechecker.hpp"

class Evaluator
{

public:

    std::string file_name;
    std::unordered_map<std::string, std::shared_ptr<Node>> symbol_table;
    std::shared_ptr<Evaluator> outer_scope = nullptr;
    int line, column;

    Evaluator() {}
    Evaluator(std::string file_name, std::vector<std::shared_ptr<Node>> nodes) : file_name(file_name), nodes(nodes) {}

    std::vector<std::shared_ptr<Node>> nodes;

    std::string builtins_path;

    bool runtime = true;
    bool typecheck = false;

    std::shared_ptr<Node> eval_node(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_nodes(std::vector<std::shared_ptr<Node>> nodes);

    std::shared_ptr<Node> eval_add(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_sub(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_mul(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_div(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_mod(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_dot(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_double_dot(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_pos_neg(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_not(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_and_or(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_equality(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_equal(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_double_arrow(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_if_statement(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_if_block(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_match_block(std::shared_ptr<Node>& node, std::string name="");
    std::shared_ptr<Node> eval_while_loop(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_for_loop(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> eval_copy(std::shared_ptr<Node>& node);

    std::shared_ptr<Node> eval_function_call(std::shared_ptr<Node> function, std::vector<std::shared_ptr<Node>> args);

    std::shared_ptr<Node> builtin_exit(std::vector<std::shared_ptr<Node>> args);
    std::shared_ptr<Node> builtin_error(std::vector<std::shared_ptr<Node>> args);
    std::shared_ptr<Node> builtin_range(std::vector<std::shared_ptr<Node>> args);
    std::shared_ptr<Node> builtin_print(std::vector<std::shared_ptr<Node>> args);
    std::shared_ptr<Node> builtin_delete(std::vector<std::shared_ptr<Node>> args);
    std::shared_ptr<Node> builtin_time(std::vector<std::shared_ptr<Node>> args);
    std::shared_ptr<Node> builtin_import(std::vector<std::shared_ptr<Node>> args);
    std::shared_ptr<Node> builtin_read(std::vector<std::shared_ptr<Node>> args);
    std::shared_ptr<Node> builtin_write(std::vector<std::shared_ptr<Node>> args);
    std::shared_ptr<Node> builtin_append(std::vector<std::shared_ptr<Node>> args);
    std::shared_ptr<Node> builtin_to_int(std::vector<std::shared_ptr<Node>> args);
    std::shared_ptr<Node> builtin_to_float(std::vector<std::shared_ptr<Node>> args);
    std::shared_ptr<Node> builtin_to_string(std::vector<std::shared_ptr<Node>> args);
    std::shared_ptr<Node> builtin_type(std::vector<std::shared_ptr<Node>> args);
    std::shared_ptr<Node> builtin_shape(std::vector<std::shared_ptr<Node>> args);

    void sort_and_unique(std::vector<std::shared_ptr<Node>>& list);
    std::shared_ptr<Node> get_explicit_type(std::shared_ptr<Node>& node);

    bool compare(const std::shared_ptr<Node> &lhs, const std::shared_ptr<Node> &rhs);
    std::shared_ptr<Node> match_function(std::shared_ptr<Node> function, std::vector<std::shared_ptr<Node>> args);
    std::shared_ptr<Node> get_type(std::shared_ptr<Node>& node);

    std::shared_ptr<Node> type_func(std::shared_ptr<Node> node);
    std::shared_ptr<Node> type_node(std::shared_ptr<Node> node);

    void init(std::string builtins_path);
    std::vector<std::string> builtins_names;
    void evaluate();

    std::shared_ptr<Node>& eval_look_up(std::string name);

    // Dynamic Typing

    struct Match_Result 
    {
        bool result;
        std::string message;
    };

    Match_Result match_types(std::shared_ptr<Node> a, std::shared_ptr<Node> b, bool match_name = true);
    Match_Result match_values(std::shared_ptr<Node> a, std::shared_ptr<Node> b);
    bool match_pattern(std::shared_ptr<Node> a, std::shared_ptr<Node> b);

    // Node Helpers
    std::shared_ptr<Node> make_int(long long value, bool literal = true);
    std::shared_ptr<Node> make_float(double value, bool literal = true);
    std::shared_ptr<Node> make_string(std::string value, bool literal = true);
    std::shared_ptr<Node> make_bool(bool value, bool literal = true);
    std::shared_ptr<Node> make_empty(bool literal = true);
    std::shared_ptr<Node> make_any(bool literal = true);
    std::shared_ptr<Node> make_list(bool literal = true);
    std::shared_ptr<Node> make_pipe_list(bool literal = true);
    std::shared_ptr<Node> make_object(bool literal = true);
    std::shared_ptr<Node> make_function(bool literal = true);

    void error_and_exit(std::string message);
};