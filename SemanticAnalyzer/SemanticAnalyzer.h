#pragma once

#include "../Parser/Parser.h"
#include "../Scanner/Scanner.h"
#include "../Scanner/Common.h"
#include <string>

class SemanticAnalyzer
{
public:
	SemanticAnalyzer();
	~SemanticAnalyzer();

	Error* GenerateCodeFromTree(ParseTree* tree);

private:
	struct Type
	{
		char name[128];
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
		OperationCodeLoadAddress,
		OperationCodeLoadConstant,
		OperationCodeStore,
		OperationCodeCall,
		OperationCodeEnd,
		OperationCodeRet,
		OperationCodeBreakFalse,
		OperationCodeLabel,
		OperationCodeAnd,
		OperationCodeOr,
		OperationCodeCmp,
		OperationCodeStoreArgumentless,
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

private:
	bool StrCmp(char* first, char* second) { return strcmp(first, second) == 0; }
	void StrCpy(char* first, char* second) { strcpy(first, second); }
	void StrFmt(char* first, char* format, ...);
	int StrToInt(char* value) { return std::atoi(value); }

	ParseNode* SetpInExpression(ParseNode* node, ParseNode* parent);
	ParseNode* StepToLastExpression(ParseNode* node);
	ParseNode* SetpInLexema(ParseNode* node, ParseNode* parent);
	Type* TryGetType(char* name);
	Function* TryGetFunction(char* name);
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
	void RecursiveAnalyzeNodeExpression(ParseNode* node);
	void RecursiveAnalyzeNodeBlock(ParseNode* node);
	void RecursiveAnalyzeNodeReturn(ParseNode* node);
	void RecursiveAnalyzeNodeCondition(ParseNode* node);

	void AddOperation(OperationCode code, OperationArgument argument);
	void AddOperation(OperationCode code);

private:
	std::vector<Veriable> veriables;
	std::vector<Function> functions;
	std::vector<Type> types;
	std::vector<Error*> errors;
	size_t labelCounter;
};