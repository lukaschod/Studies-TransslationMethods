#pragma once

#include "../Scanner/Scanner.h"
#include "../Scanner/Common.h"

#define MAX_TERM_COUNT 10

/*struct ParseNode
{
	char expression[128];
	char lexema[128];
	size_t lineNumber;
	size_t level;
};

struct ParseTree
{
	std::vector<ParseNode> nodes;
};

class ParseTreeConstructor
{
public:
	ParseTreeConstructor(ParseTree* tree)
		: tree(tree)
		, currentLevel(0)
	{
	}

	void MoveForward(char* name)
	{
		tree->nodes.push_back(ParseNode());
		auto& node = tree->nodes.back();

		auto node = new ParseNode();
		strcpy(node.expression, name);
		memset(node.lexema, 0, sizeof(char) * 128);
		node.level = currentLevel;
	}

	void MoveBack()
	{
		currentLevel--;
	}

	void Rollback()
	{
		MoveBack();
		while (true)
		{
			auto& node = tree->nodes.back();
			if (node.level == currentLevel)
				break;
			tree->nodes.pop_back();
		}
	}

	void SetLexemaForCurrent(char* name)
	{
		auto& node = tree->nodes.back();
		strcpy(node.lexema, name);
	}

public:
	ParseTree* tree;
	size_t currentLevel;
};*/

struct ParseNode
{
std::vector<ParseNode*> childs;
ParseNode* parent;
char expression[128];
char lexema[128];
size_t lineNumber;
};

struct ParseTree
{
ParseNode* root;
};


class ParseTreeConstructor
{
public:
	ParseTreeConstructor(ParseTree* tree)
		: current(nullptr)
		, tree(tree)
	{
	}

	void MoveForward(char* name)
	{
		auto node = new ParseNode();
		strcpy(node->expression, name);
		memset(node->lexema, 0, sizeof(char) * 128);

		if (current == nullptr)
		{
			current = node;
			tree->root = node;
		}
		else
		{
			current->childs.push_back(node);
			node->parent = current;
			current = node;
		}
	}

	void MoveBack()
	{
		current = current->parent;
	}

	void Rollback()
	{
		MoveBack();

		if (current == nullptr)
			return;

		DeleteChildsRecursive(current);
	}

	void SetLexemaForCurrent(char* name)
	{
		strcpy(current->lexema, name);
	}

private:
	void DeleteChildsRecursive(ParseNode* node)
	{
		for (auto child : node->childs)
		{
			DeleteChildsRecursive(child);
			delete child;
		}

		node->childs.clear();
	}

public:
	ParseTree* tree;
	ParseNode* current;
};

struct Expression;

struct ExpressionName
{
	char name[128];
	std::vector<Expression*>::iterator expression;

	ExpressionName(char* name) { strcpy(this->name, name); }
};

struct Expression
{
	ExpressionName* nameId;
	ExpressionName* terms[MAX_TERM_COUNT];

	Expression()
	{
		nameId = nullptr; 
		ClearTerms();
	}

	void ClearTerms()
	{
		memset(terms, 0, sizeof(uint64_t) * MAX_TERM_COUNT);
	}
};

class Parser
{
public:
	Parser();
	~Parser();

	Error* AddExpresionsFromMemory(char* data, size_t size);
	Error* Parse(char* startExpressionName, std::vector<Lexema*>& lexemos, ParseTree* outTree);

private:
	ExpressionName* NameToId(char* expressionName);
	char* IdToName(ExpressionName* id);
	std::vector<Expression*>::iterator FindExpression(ExpressionName* nameId);
	std::vector<Expression*>::iterator FindNextExpression(std::vector<Expression*>::iterator current);
	bool CheckLexema(char* name, std::vector<Lexema*>& lexemos);

	void Push();
	void Pop();

private:
	std::vector<Expression*> expressions;
	std::vector<ExpressionName*> names;
};