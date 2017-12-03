#pragma once

#include "../Parser/Parser.h"
#include "../Scanner/Scanner.h"
#include "../Scanner/Common.h"
#include <string>

struct Type
{
	char name[128];
	size_t size = 4;
};

struct Veriable
{
	char name[128];
	Type* type;
};

struct Label
{
	char name[128];
};

enum OperationCode
{
	OperationCodePlus,
	OperationCodeMinus,
	OperationCodeMult,
	OperationCodeDiv,
	OperationCodeLoadArgument,
	OperationCodeLoadLocal,
	OperationCodeLoadConstant,
	OperationCodeLoadArgumentAddress,
	OperationCodeLoadLocalAddress,
	OperationCodeLoadGlobalAddress,
	OperationCodeLoadGlobal,
	OperationCodeStore,
	OperationCodeAssign,
	OperationCodeCall,
	OperationCodeEnd,
	OperationCodeRet,
	OperationCodeBreakFalse,
	OperationCodeLabel,
	OperationCodeAnd,
	OperationCodeOr,
	OperationCodeCmp,
	OperationCodeRetSkip,
};

struct Function;

struct OperationArgument
{
	OperationArgument() {}
	OperationArgument(Veriable* veriable) : veriable(veriable) {}
	OperationArgument(Function* function) : function(function) {}
	OperationArgument(Label* label) : label(label) {}
	OperationArgument(float constant) : constant(constant) {}
	Veriable* veriable;
	Function* function;
	Label* label;
	float constant;
};

struct Operation
{
	OperationCode code;
	std::vector<OperationArgument> arguments;
};

struct Function
{
	char name[128];
	Type* type;
	std::vector<Veriable> parameters;
	std::vector<Veriable> veriables;
	std::vector<Operation> operations;
	std::vector<Label> labels;
};

struct Code
{
	std::vector<Veriable> veriables;
	std::vector<Function> functions;
	std::vector<Type> types;
	std::vector<Error*> errors;
};

class SemanticAnalyzer
{
public:
	SemanticAnalyzer();
	~SemanticAnalyzer();

	void GenerateCodeFromTree(ParseTree* tree, Code* code);

private:
	bool StrCmp(char* first, char* second) { return strcmp(first, second) == 0; }
	void StrCpy(char* first, char* second) { strcpy(first, second); }
	void StrFmt(char* first, char* format, ...);
	int StrToInt(char* value) { return std::atoi(value); }
	float StrToFloat(char* value) { return std::stof(value); }

	ParseNode* StepInExpression(ParseNode* node, ParseNode* parent);
	ParseNode* StepOutExpression(ParseNode* node, ParseNode* parent);
	ParseNode* StepToLastExpression(ParseNode* node);
	ParseNode* StepInLexema(ParseNode* node, ParseNode* parent);
	Type* TryGetType(char* name);
	Function* TryGetFunction(char* name, std::vector<Type*>& parameters);
	Type* AddOperationFunction(char* name, Type* paramter1, Type* parameter2, bool returnEnabled);
	Type* AddOperationFunction(char* name, std::vector<Type*> parameters, bool returnEnabled);
	Veriable* TryGetVeriable(char* name);
	Veriable* TryGetLocalVeriable(char* name);
	Veriable* TryGetParameter(char* name);
	Veriable* TryGetGlobalVeriable(char* name);
	int TryGetParameterIndex(char* name);
	int TryGetLocalVeriableIndex(char* name);

	void RecursiveAnalyzeNode(ParseNode* node);
	void RecursiveAnalyzeNodeDeclareVeriable(ParseNode* node, bool init);
	void RecursiveAnalyzeNodeDeclareLocalVeriable(ParseNode* node, bool init);
	void RecursiveAnalyzeNodeDeclareFunction(ParseNode* node);
	Type* RecursiveAnalyzeNodeExpression(ParseNode* node, bool addressEnabled = false, bool returnEnabled = false);
	void RecursiveAnalyzeNodeBlock(ParseNode* node);
	void RecursiveAnalyzeNodeReturn(ParseNode* node);
	void RecursiveAnalyzeNodeCondition(ParseNode* node);

	void AddOperation(OperationCode code, OperationArgument argument1, OperationArgument argument2);
	void AddOperation(OperationCode code, OperationArgument argument);
	void AddOperation(OperationCode code);

private:
	std::vector<Veriable>* veriables;
	std::vector<Function>* functions;
	std::vector<Type>* types;
	std::vector<Error*>* errors;
	size_t labelCounter;
};