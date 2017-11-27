#include "Parser.h"
#include <queue>

Parser::Parser()
{
}

Parser::~Parser()
{
}

Error* Parser::AddExpresionsFromMemory(char* data, size_t size)
{
	names.reserve(100);
	BufferReader reader(data, data + size);
	while (true)
	{
		// Means it is finished
		auto lineStart = reader.pointer;
		if (!reader.MovePointerToFirst('\n'))
			break;

		BufferReader lineReader(lineStart, reader.pointer);
		// Check for comments
		{
			auto cached = lineReader.pointer;
			lineReader.pointer = lineStart;

			if (lineReader.MovePointerToFirst("//"))
			{
				lineReader.end = lineReader.pointer;
			}

			if (lineReader.MovePointerToFirst("/*"))
			{
				if (!reader.MovePointerToFirst("*/"))
					return new Error("Expected */");
				continue;
			}

			lineReader.pointer = cached;
		}

		char expressionName[128];
		if (lineReader.MovePointerToFirstExpression(expressionName))
		{
			if (!lineReader.MovePointerToFirst("::="))
				return new Error("Expected ::=");

			auto blockStart = lineReader.pointer;
			while (lineReader.MovePointerToFirst('|') || lineReader.MovePointerToFirst('\n'))
			{
				auto expression = new Expression();
				expression->nameId = NameToId(expressionName);

				BufferReader blockReader(blockStart, lineReader.pointer);

				char termName[128];
				size_t termCount = 0;
				while (blockReader.MovePointerToFirstExpression(termName) && termCount < MAX_TERM_COUNT)
					expression->terms[termCount++] = NameToId(termName);

				blockStart = lineReader.pointer;

				expressions.push_back(expression);
			}
		}
	}

	for (auto name : names)
	{
		name->expression = FindExpression(name);
	}

	return nullptr;
}

enum ExpressionExecutionState
{
	ExpressionExecutionStateNone,
	ExpressionExecutionStateFailed,
	ExpressionExecutionStateTerms,
	ExpressionExecutionStateTermFailed,
};

struct ExpressionExecution
{
	ExpressionName* nameId;
	std::vector<Expression*>::iterator expression;
	uint64_t term;
	uint64_t lexema;
	ExpressionExecutionState state;

	ExpressionExecution()
		: nameId(nullptr)
		, expression()
		, term(0)
		, lexema(0)
		, state(ExpressionExecutionStateNone)
	{
	}
};

struct ParserStep
{
	ExpressionName* nameId;
	Lexema* lexema;
	bool isStart;
	ParserStep(ExpressionName* nameId, Lexema* lexema, bool isStart) :
		nameId(nameId),
		lexema(lexema),
		isStart(isStart)
	{
	}
};

std::vector<ExpressionExecution> executions;
int level = 0;
bool inFunctionDeclare = false;

void Parser::Push()
{
	/*auto& execution = executions.back();
	if (strcmp("expression", IdToName(execution.nameId)) == 0)
		inFunctionDeclare = true;

	if (
		strcmp("lanesDeclare", IdToName(execution.nameId)) == 0
		|| strcmp("program", IdToName(execution.nameId)) == 0
		|| strcmp("laneDeclare", IdToName(execution.nameId)) == 0
		|| strcmp("functionDeclare", IdToName(execution.nameId)) == 0
		|| strcmp("classDeclare", IdToName(execution.nameId)) == 0
		|| strcmp("moduleDeclare", IdToName(execution.nameId)) == 0
		|| strcmp("end", IdToName(execution.nameId)) == 0
		|| inFunctionDeclare
		)
	{
		for (int i = 0; i < level; i++)
			printf("  ");
		printf("%s\n", IdToName(execution.nameId));
	}
	
	level++;*/
}

void Parser::Pop()
{
	/*level--;
	auto& execution = executions.back();
	if (strcmp("expression", IdToName(execution.nameId)) == 0)
		inFunctionDeclare = false;
	if (
		strcmp("lanesDeclare", IdToName(execution.nameId)) == 0
		|| strcmp("program", IdToName(execution.nameId)) == 0
		|| strcmp("laneDeclare", IdToName(execution.nameId)) == 0
		|| strcmp("functionDeclare", IdToName(execution.nameId)) == 0
		|| strcmp("classDeclare", IdToName(execution.nameId)) == 0
		|| strcmp("moduleDeclare", IdToName(execution.nameId)) == 0
		|| strcmp("end", IdToName(execution.nameId)) == 0
		|| inFunctionDeclare
		)
	{
		for (int i = 0; i < level; i++)
			printf("  ");
		printf("%s %d\n", IdToName(execution.nameId), execution.lexema);
	}*/
}

Error* Parser::Parse(char* startExpressionName, std::vector<Lexema*>& lexemos, ParseTree* outTree)
{
	ExpressionExecution startExecution;
	startExecution.nameId = NameToId(startExpressionName);
	executions.push_back(startExecution);

	int logestSpree = 0;
	Error* longestSpreeError = nullptr;

	ParseTreeConstructor treeConstructor(outTree);

	ExpressionExecutionState lastState;
	while (!executions.empty())
	{
		auto& currentExecution = executions.back();

		switch (currentExecution.state)
		{
		case ExpressionExecutionStateNone:
		{
			//if (currentExecution.lexema == 67 && (strcmp(IdToName(currentExecution.nameId), "expressionVeriable") == 0))
			//	__debugbreak();
			//if ((strcmp(IdToName(currentExecution.nameId), "int") == 0))
			//	__debugbreak();

			treeConstructor.MoveForward(currentExecution.nameId->name);

			if (currentExecution.lexema >= lexemos.size())
			{
				currentExecution.state = ExpressionExecutionStateFailed;
				break;
			}

			// Firstly we don't know anything about this term, so lets check if lexema exists for it
			if (strcmp(lexemos[currentExecution.lexema]->type, IdToName(currentExecution.nameId)) == 0)
			{
				auto lexema = currentExecution.lexema;
				Pop();
				executions.pop_back();
				if (!executions.empty())
					executions.back().lexema = lexema + 1;

				treeConstructor.SetLexemaForCurrent(lexemos[currentExecution.lexema]->data);
				treeConstructor.MoveBack();

				break;
			}

			// For optimization
			if ((strcmp(IdToName(currentExecution.nameId), "constantInt") == 0)
				|| (strcmp(IdToName(currentExecution.nameId), "constantFloat") == 0)
				|| (strcmp(IdToName(currentExecution.nameId), "constantBool") == 0)
				|| (strcmp(IdToName(currentExecution.nameId), "name") == 0)
				|| (strcmp(IdToName(currentExecution.nameId), "string") == 0)
				)
			{
				currentExecution.state = ExpressionExecutionStateFailed;
				break;
			}

			// Lets try to find expression for this lexema, if there is no it means its failed
			currentExecution.expression = currentExecution.nameId->expression;
			if (currentExecution.expression == expressions.end())
			{
				currentExecution.state = ExpressionExecutionStateFailed;
				break;
			}

			// If expression is without terms, we should back out as failure
			auto currentExpression = *currentExecution.expression;
			if (currentExpression->terms[0] == nullptr)
			{
				currentExecution.state = ExpressionExecutionStateFailed;
				break;
			}

			// Lets move it to expression state
			currentExecution.state = ExpressionExecutionStateTerms;
			break;
		}
		case ExpressionExecutionStateFailed:
		{
			if (logestSpree < currentExecution.lexema)
			{
				auto execution = executions.back();
				if (executions.size() > 3)
				{
					longestSpreeError = new Error("While executing wiht lexema '%s' (%d). \nExpected in expression %s. \nExpected in expression %s. \nExpected in expression %s.",
						lexemos[execution.lexema], execution.lexema, execution.nameId->name, 
						executions[executions.size() - 3].nameId,
						executions[executions.size() - 2].nameId,
						executions[executions.size() - 1].nameId);
				}
				else
				{
					longestSpreeError = new Error("While executing wiht lexema '%s' (%d).",
						lexemos[execution.lexema], execution.lexema, execution.nameId->name);
				}
				
				logestSpree = currentExecution.lexema;
			}
			Pop();
			executions.pop_back();
			if (!executions.empty())
				executions.back().state = ExpressionExecutionStateTermFailed;

			treeConstructor.Rollback();

			break;
		}
		case ExpressionExecutionStateTerms:
		{
			auto currentExpression = *currentExecution.expression;
			auto currentTerm = currentExpression->terms[currentExecution.term++];

			// Check if current term is valid
			if (currentTerm == nullptr)
			{
				auto lexema = currentExecution.lexema;

				Pop();
				executions.pop_back();

				if (!executions.empty())
					executions.back().lexema = lexema;

				treeConstructor.MoveBack();	
			}
			else
			{
				ExpressionExecution newExecution;
				newExecution.nameId = currentTerm;
				newExecution.lexema = currentExecution.lexema;
				executions.push_back(newExecution);
				Push();
			}

			break;
		}
		
		case ExpressionExecutionStateTermFailed:
		{
			//if (currentExecution.term == 3)
			//	__debugbreak();

			/*if (strcmp("lanesDeclare", IdToName(currentExecution.nameId)) == 0)
			{
				__debugbreak();
			}*/

			// Check if we can try another expression varation
			auto newExpression = FindNextExpression(currentExecution.expression);
			if (newExpression == expressions.end() || (*newExpression)->terms[0] == nullptr)
			{
				currentExecution.state = ExpressionExecutionStateFailed;
				break;
			}

			auto currentExpression = *currentExecution.expression;
			auto currentExecutionLexema = currentExecution.lexema;
			auto currentExecutionTerm = currentExecution.term;

			Pop();
			executions.pop_back();

			ExpressionExecution newExecution;
			newExecution.nameId = currentExecution.nameId;
			newExecution.expression = newExpression;
			newExecution.state = ExpressionExecutionStateTerms;
			newExecution.lexema = executions.empty() ? 1 : executions.back().lexema; // Take parents lexema

			int j = 0;
			for (int i = 0; i < currentExecutionTerm; i++)
			{
				if (currentExpression->terms[i]->name == (*newExpression)->terms[i]->name)
					j = i;
			}
			if (j == currentExecution.term)
			{
				newExecution.lexema = currentExecutionLexema;
				newExecution.term = currentExecutionTerm;
			}

			executions.push_back(newExecution);
			Push();

			break;
		}
		}
		lastState = currentExecution.state;
	}

	if (lastState != ExpressionExecutionStateTerms)
		return longestSpreeError;

	return nullptr;
}

ExpressionName* Parser::NameToId(char* expressionName)
{
	for (size_t i = 0; i < names.size(); i++)
	{
		auto& name = names[i];
		if (strcmp(name->name, expressionName) == 0)
			return name;
	}

	auto name = new ExpressionName(expressionName);
	names.push_back(name);
	return names.back();
}

char* Parser::IdToName(ExpressionName* id)
{
	return id->name;
}

std::vector<Expression*>::iterator Parser::FindExpression(ExpressionName* nameId)
{
	auto itr = expressions.begin();
	for (; itr != expressions.end(); itr++)
	{
		auto& expression = *itr;
		if (expression->nameId->name == nameId->name)
			return itr;
	}
	return itr;
}

std::vector<Expression*>::iterator Parser::FindNextExpression(std::vector<Expression*>::iterator current)
{
	auto itr = current + 1;
	for (; itr != expressions.end(); itr++)
	{
		auto& expression = *itr;
		if (expression->nameId->name == (*current)->nameId->name)
			return itr;
		else
			return expressions.end();
	}
	return itr;
}

bool Parser::CheckLexema(char* name, std::vector<Lexema*>& lexemos)
{
	for (auto lexema : lexemos)
	{
		if (strcmp(lexema->type, name) == 0)
			return true;
	}
	return false;
}
