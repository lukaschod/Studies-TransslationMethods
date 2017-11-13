#pragma once

#include <Common.h>
#include <Scanner.h>

#define MAX_TERM_COUNT 10

struct ParseNode
{
	std::vector<ParseNode*> childs;
	ParseNode* parent;
	char expression[128];
	char lexema[128];
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
		, totalNodeCount(0)
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

		totalNodeCount++;
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

		totalNodeCount -= node->childs.size();
		node->childs.clear();
	}

public:
	ParseTree* tree;
	ParseNode* current;
	size_t totalNodeCount;
};

struct ExpressionName
{
	char name[128];

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