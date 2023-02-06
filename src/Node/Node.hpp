#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <vector>
#include <set>

enum class NodeType {
	INT,
	FLOAT,
	STRING,
	BOOL,
	ID,

	OP,
	PARTIAL_OP,

	EQ_EQ,
	NOT_EQUAL,
	LT_EQUAL,
	GT_EQUAL,
	PLUS_EQ,
	MINUS_EQ,
	NOT,

	RIGHT_ARROW_SINGLE,
	RIGHT_ARROW_DOUBLE,
	DOUBLE_ARROW,

	DOUBLE_COLON,
	COLON,

	AND,
	OR,

	EQUAL,

	L_PAREN,
	R_PAREN,
	L_BRACE,
	R_BRACE,
	L_BRACKET,
	R_BRACKET,
	L_ANGLE,
	R_ANGLE,

	DOT,
	APOSTROPHE,
	EXCLAMATION,

	BACKSLASH,

	AT,
	HASH,
	DOLLAR,
	CARET,
	QUESTION,
	PERCENT,
	COMMA,
	PIPE,
	SEMICOLON,

	DOUBLE_QUOTE,

	MINUS,
	PLUS,
	SLASH,
	STAR,

	POS,
	NEG,

	END_OF_FILE,
	START_OF_FILE,

	EMPTY,
	ANY,

	BLOCK,
	LIST,
	OBJECT,
	FUNCTION,
	COMMA_LIST,
	PIPE_LIST,
	FUNC_LIST,
	SWITCH,
	FOR_LOOP,
	WHILE_LOOP,
	RANGE,

	IF_STATEMENT,
	IF_BLOCK,
	MATCH_BLOCK,

	KEYWORD,

	FUNC_CALL,

	DOUBLE_DOT,
	TRIPLE_DOT,

	RETURN,
	BREAK,
	CONTINUE,

	ITERATOR,

	ERROR,
	COPY,
	TYPE
};

struct Node;

struct Int_Node
{
	long long value = 0;
	Int_Node() = default;
	Int_Node(long value) : value(value) {}
};

struct Float_Node
{
	double value = 0;
	Float_Node() = default;
	Float_Node(double value) : value(value) {}
};

struct Bool_Node
{
	bool value = 0;
	Bool_Node() = default;
	Bool_Node(bool value) : value(value) {}
};

struct ID_Node
{
	std::string value = "";
	std::shared_ptr<Node> type = nullptr;
	ID_Node() = default;
	ID_Node(std::string value) : value(value) {}
};

struct String_Node
{
	std::string value = "";
	String_Node() = default;
	String_Node(std::string value) : value(value) {}
	bool is_type = false;
};

struct Op_Node
{
	std::string value = "";
	NodeType op_type = NodeType::EMPTY;
	bool is_binary = true;
	Op_Node() = default;
	Op_Node(std::string value) : value(value) {}
};

struct Block_Node
{
	std::unordered_map<std::string, std::shared_ptr<Node>> symbol_table;
	std::vector<std::shared_ptr<Node>> nodes;
	Block_Node() = default;
};

struct List_Node
{
	std::vector<std::shared_ptr<Node>> nodes;
	List_Node() = default;
};

struct Object_Prop
{
	bool required = true;
	std::shared_ptr<Node> value;

	Object_Prop() {}
	Object_Prop(bool required, std::shared_ptr<Node> value) : required(required), value(value) {}
};

struct Object_Node
{
	std::string name;
	std::unordered_map<std::string, Object_Prop> properties;
	bool is_type = false;
};

struct Func_Call_Node
{
	std::string name;
	std::shared_ptr<Node> caller;
	std::vector<std::shared_ptr<Node>> args;
};

struct Function_Node
{
	std::string name = "lambda";
	std::shared_ptr<Node> return_type;
	std::vector<std::shared_ptr<Node>> params;
	std::vector<std::shared_ptr<Node>> args;
	std::shared_ptr<Node> body;
	std::unordered_map<std::string, std::shared_ptr<Node>> closure;
	std::vector<int> default_arg_indices;
	bool is_type = false;
	bool typechecked = false;
};

struct If_Statement_Node
{
	std::shared_ptr<Node> conditional;
	std::shared_ptr<Node> body;
};

struct If_Block_Node
{
	std::vector<std::shared_ptr<Node>> cases;
};

struct Match_Block_Node
{
	std::shared_ptr<Node> value;
	std::vector<std::shared_ptr<Node>> cases;
};

struct For_Loop_Node
{
	std::shared_ptr<Node> init_list;
	std::shared_ptr<Node> body;
};

struct While_Loop_Node
{
	std::shared_ptr<Node> conditional;
	std::shared_ptr<Node> body;
};

struct Range_Node
{
	int start, end = 0;
};

struct Type_Node
{
	bool is_literal = true;
	std::shared_ptr<Node> allowed_type;
	std::string name;
};

struct Error_Node
{
	std::vector<std::string> errors;
};

struct Node
{
	NodeType type;

	int column = 1;
	int line = 1;

	std::shared_ptr<Node> left = nullptr;
	std::shared_ptr<Node> right = nullptr;

	Node() = default;
	Node(NodeType type) : type(type) {}
	Node(int line, int column) : line(line), column(column) {}
	Node(NodeType type, int line, int column) : type(type), line(line), column(column) {}


	Int_Node				INT;
	Float_Node				FLOAT;
	Bool_Node				BOOL;
	ID_Node					ID;
	String_Node				STRING;
	Op_Node					OP;
	Block_Node				BLOCK;
	Object_Node				OBJECT;
	List_Node				LIST;
	Func_Call_Node			FUNC_CALL;
	Error_Node				ERROR;
	List_Node				COMMA_LIST;
	List_Node				PIPE_LIST;
	List_Node				FUNC_LIST;
	Function_Node			FUNCTION;
	If_Statement_Node		IF_STATEMENT;
	If_Block_Node			IF_BLOCK;
	Match_Block_Node		MATCH_BLOCK;
	For_Loop_Node			FOR_LOOP;
	While_Loop_Node			WHILE_LOOP;
	Range_Node				RANGE;
	Type_Node				TYPE;
	bool					is_type = false;

	std::string repr();
	std::string type_repr();
};