#include "SemanticAnalyzer.h"

#define REPORT_ERROR(...) { errors->push_back(new Error(__VA_ARGS__)); return; }
#define REPORT_ERROR_NULL(...) { errors->push_back(new Error(__VA_ARGS__)); return nullptr; }

SemanticAnalyzer::SemanticAnalyzer()
{
}

SemanticAnalyzer::~SemanticAnalyzer()
{
}

void SemanticAnalyzer::GenerateCodeFromTree(ParseTree* tree, Code* code)
{
	types = &code->types;
	veriables = &code->veriables;
	functions = &code->functions;
	errors = &code->errors;

	types->push_back(Type());
	auto type = &types->back();
	StrCpy(type->name, "void");

	types->push_back(Type());
	type = &types->back();
	StrCpy(type->name, "int");

	types->push_back(Type());
	type = &types->back();
	StrCpy(type->name, "float");

	types->push_back(Type());
	type = &types->back();
	StrCpy(type->name, "label");

	labelCounter = 0;

	RecursiveAnalyzeNode(tree->root);

	std::vector<Type*> paramters;
	auto mainFunction = TryGetFunction("Main", paramters);
	if (mainFunction == nullptr)
		REPORT_ERROR("Main function is not declared");
}

void SemanticAnalyzer::RecursiveAnalyzeNode(ParseNode * node)
{
	for (auto child : node->childs)
	{
		if (StrCmp(child->expression, "veriableDeclareInit"))
			RecursiveAnalyzeNodeDeclareLocalVeriable(child, true);
		else if (StrCmp(child->expression, "veriableDeclare"))
			RecursiveAnalyzeNodeDeclareVeriable(child, false);
		else if (StrCmp(child->expression, "functionDeclare"))
			RecursiveAnalyzeNodeDeclareFunction(child);
		else
			RecursiveAnalyzeNode(child);
	}
}

void SemanticAnalyzer::RecursiveAnalyzeNodeDeclareVeriable(ParseNode* node, bool init)
{
	veriables->push_back(Veriable());
	auto& veriable = veriables->back();

	auto nodeTarget = StepInLexema(node, node);
	if (nodeTarget == nullptr)
		REPORT_ERROR("Expected function type");

	veriable.type = TryGetType(nodeTarget->lexema);
	if (veriable.type == nullptr)
		REPORT_ERROR("Unknown type %s", nodeTarget->lexema);

	nodeTarget = StepInLexema(nodeTarget, node);
	if (nodeTarget == nullptr || !StrCmp(nodeTarget->expression, "name"))
		REPORT_ERROR("Expected function type");
	StrCpy(veriable.name, nodeTarget->lexema);

	if (init)
	{
		// TODO:
		__debugbreak();
	}
}

void SemanticAnalyzer::RecursiveAnalyzeNodeDeclareLocalVeriable(ParseNode * node, bool init)
{
	auto& function = functions->back();

	function.veriables.push_back(Veriable());
	auto& veriable = function.veriables.back();

	auto nodeTarget = StepInLexema(node, node);
	if (nodeTarget == nullptr)
		REPORT_ERROR("Expected function type");

	veriable.type = TryGetType(nodeTarget->lexema);
	if (veriable.type == nullptr)
		REPORT_ERROR("Unknown type %s", nodeTarget->lexema);

	nodeTarget = StepInLexema(nodeTarget, node);
	if (nodeTarget == nullptr || !StrCmp(nodeTarget->expression, "name"))
		REPORT_ERROR("Expected function type");
	StrCpy(veriable.name, nodeTarget->lexema);

	if (init)
	{
		nodeTarget = StepInLexema(nodeTarget, node);
		if (!StrCmp(nodeTarget->expression, "="))
			REPORT_ERROR("Expected =");

		nodeTarget = StepInExpression(nodeTarget, node);
		AddOperation(OperationCodeLoadLocalAddress, function.veriables.size() - 1);
		auto parameter = RecursiveAnalyzeNodeExpression(nodeTarget, false, true);

		AddOperationFunction("operator=", veriable.type, parameter, false);
	}
}

void SemanticAnalyzer::RecursiveAnalyzeNodeDeclareFunction(ParseNode * node)
{
	functions->push_back(Function());
	auto& function = functions->back();

	auto nodeTarget = StepInLexema(node, node);
	if (nodeTarget == nullptr)
		REPORT_ERROR("Expected function type");

	function.type = TryGetType(nodeTarget->lexema);
	if (function.type == nullptr)
		REPORT_ERROR("Unknown type %s", nodeTarget->lexema);

	nodeTarget = StepInLexema(nodeTarget, node);
	if (nodeTarget == nullptr || !StrCmp(nodeTarget->expression, "name"))
		REPORT_ERROR("Expected function name");

	auto functionNode = nodeTarget;

	nodeTarget = StepInLexema(nodeTarget, node);
	if (!StrCmp(nodeTarget->expression, "("))
		REPORT_ERROR("Expected (");

	std::vector<Type*> parameters;

	while (true)
	{
		nodeTarget = StepInLexema(nodeTarget, node);
		if (nodeTarget == nullptr)
			REPORT_ERROR("Expected parameter");

		if (StrCmp(nodeTarget->expression, ")"))
			break;

		function.parameters.push_back(Veriable());
		auto& paramter = function.parameters.back();

		paramter.type = TryGetType(nodeTarget->lexema);
		if (paramter.type == nullptr)
			REPORT_ERROR("Unknown type %s", nodeTarget->lexema);
		parameters.push_back(paramter.type);

		nodeTarget = StepInLexema(nodeTarget, node);
		StrCpy(paramter.name, nodeTarget->lexema);
	}

	auto functionDuplicate = TryGetFunction(functionNode->lexema, parameters);
	if (functionDuplicate != nullptr)
		REPORT_ERROR("Duplicated function");
	StrCpy(function.name, functionNode->lexema);

	RecursiveAnalyzeNodeBlock(node);

	if (StrCmp(function.type->name, "void"))
	{
		AddOperation(OperationCodeEnd);
	}
}

Type* SemanticAnalyzer::RecursiveAnalyzeNodeExpression(ParseNode * node, bool addressEnabled, bool returnEnabled)
{
	if (StrCmp(node->expression, "expression") && node->childs.size() == 3)
	{
		auto parameter1 = RecursiveAnalyzeNodeExpression(node->childs[0], true, true);
		auto parameter2 = RecursiveAnalyzeNodeExpression(node->childs[2], false, true);

		auto targetNode = StepInLexema(node->childs[1], node);
		if (!StrCmp(targetNode->lexema, "="))
			REPORT_ERROR_NULL("Expected =");

		return AddOperationFunction("operator=", parameter1, parameter2, returnEnabled);
	}

	if (StrCmp(node->expression, "expressionAnd") && node->childs.size() == 3)
	{
		auto parameter1 = RecursiveAnalyzeNodeExpression(node->childs[0], false, true);
		auto parameter2 = RecursiveAnalyzeNodeExpression(node->childs[2], false, true);

		auto targetNode = StepInLexema(node->childs[1], node);
		if (!StrCmp(targetNode->lexema, "&"))
			REPORT_ERROR_NULL("Expected &");

		targetNode = StepInLexema(targetNode, node);
		if (!StrCmp(targetNode->lexema, "&"))
			REPORT_ERROR_NULL("Expected &");

		return AddOperationFunction("operator&&", parameter1, parameter2, returnEnabled);
	}

	if (StrCmp(node->expression, "expressionOr") && node->childs.size() == 3)
	{
		auto parameter1 = RecursiveAnalyzeNodeExpression(node->childs[0], false, true);
		auto parameter2 = RecursiveAnalyzeNodeExpression(node->childs[2], false, true);

		auto targetNode = StepInLexema(node->childs[1], node);
		if (!StrCmp(targetNode->lexema, "|"))
			REPORT_ERROR_NULL("Expected |");

		targetNode = StepInLexema(targetNode, node);
		if (!StrCmp(targetNode->lexema, "|"))
			REPORT_ERROR_NULL("Expected |");

		return AddOperationFunction("operator||", parameter1, parameter2, returnEnabled);
	}

	if (StrCmp(node->expression, "expressionCmp") && node->childs.size() == 3)
	{
		auto parameter1 = RecursiveAnalyzeNodeExpression(node->childs[0], false, true);
		auto parameter2 = RecursiveAnalyzeNodeExpression(node->childs[2], false, true);

		auto targetNode = StepInLexema(node->childs[1], node);
		if (StrCmp(targetNode->lexema, "="))
		{
			targetNode = StepInLexema(targetNode, node);
			if (StrCmp(targetNode->lexema, "="))
			{
				return AddOperationFunction("operator==", parameter1, parameter2, returnEnabled);
			}
		}

		REPORT_ERROR_NULL("Unexpected operator");
	}

	if (StrCmp(node->expression, "expressionAdd") && node->childs.size() == 3)
	{
		auto parameter1 = RecursiveAnalyzeNodeExpression(node->childs[0], false, true);
		auto parameter2 = RecursiveAnalyzeNodeExpression(node->childs[2], false, true);

		auto targetNode = StepInLexema(node->childs[1], node);
		if (targetNode == nullptr)
			REPORT_ERROR_NULL("Expected operator");

		if (StrCmp(targetNode->lexema, "+"))
		{
			return AddOperationFunction("operator+", parameter1, parameter2, returnEnabled);
		}
		else if (StrCmp(targetNode->lexema, "-"))
		{
			return AddOperationFunction("operator-", parameter1, parameter2, returnEnabled);
		}

		REPORT_ERROR_NULL("Unexpected operator");
	}

	if (StrCmp(node->expression, "expressionMult") && node->childs.size() == 3)
	{
		auto parameter1 = RecursiveAnalyzeNodeExpression(node->childs[0], false, true);
		auto parameter2 = RecursiveAnalyzeNodeExpression(node->childs[2], false, true);

		auto targetNode = StepInLexema(node->childs[1], node);
		if (targetNode == nullptr)
			REPORT_ERROR_NULL("Expected operator");

		if (StrCmp(targetNode->lexema, "*"))
		{
			return AddOperationFunction("operator*", parameter1, parameter2, returnEnabled);
		}
		else if (StrCmp(targetNode->lexema, "/"))
		{
			return AddOperationFunction("operator/", parameter1, parameter2, returnEnabled);
		}

		REPORT_ERROR_NULL("Unexpected operator");
	}

	if (StrCmp(node->expression, "veriableName"))
	{
		auto targetNode = StepInLexema(node, node);
		if (targetNode == nullptr)
			REPORT_ERROR_NULL("Expected veriable name");

		auto parameterIndex = TryGetParameterIndex(targetNode->lexema);
		if (parameterIndex != -1)
		{
			AddOperation(addressEnabled ? OperationCodeLoadArgumentAddress : OperationCodeLoadArgument, OperationArgument(parameterIndex));
			return functions->back().parameters[parameterIndex].type;
		}

		auto localVeriableIndex = TryGetLocalVeriableIndex(targetNode->lexema);
		if (localVeriableIndex != -1)
		{
			AddOperation(addressEnabled ? OperationCodeLoadLocalAddress : OperationCodeLoadLocal, OperationArgument(localVeriableIndex));
			return functions->back().veriables[localVeriableIndex].type;
		}

		auto veriable = TryGetGlobalVeriable(targetNode->lexema);
		if (veriable != nullptr)
		{
			AddOperation(addressEnabled ? OperationCodeLoadGlobalAddress : OperationCodeLoadGlobal, OperationArgument(veriable));
			return veriable->type;
		}

		REPORT_ERROR_NULL("Used of undeclared veriable %s", targetNode->lexema);
	}

	if (StrCmp(node->expression, "functionCall"))
	{
		auto targetNode = StepInLexema(node, node);
		if (targetNode == nullptr)
			REPORT_ERROR_NULL("Expected function name");

		auto functionNode = targetNode;
		
		targetNode = StepInLexema(targetNode, node);
		if (!StrCmp(targetNode->expression, "("))
			REPORT_ERROR_NULL("Expected (");

		std::vector<Type*> parameters;

		targetNode = StepInExpression(targetNode, node);
		if (StrCmp(targetNode->expression, "parametersCall"))
		{
			while (true)
			{
				auto parameter = RecursiveAnalyzeNodeExpression(targetNode, false, true);
				parameters.push_back(parameter);
				targetNode = StepInExpression(targetNode, node); // Move to expression
				targetNode = StepToLastExpression(targetNode); // Skip expression

				targetNode = StepInExpression(targetNode, node); // Skip expression
				if (StrCmp(targetNode->expression, ")"))
					break;

				targetNode = StepOutExpression(targetNode, node);
			}
		}

		return AddOperationFunction(functionNode->lexema, parameters, returnEnabled);
	}

	if (StrCmp(node->expression, "constant"))
	{
		auto targetNode = StepInLexema(node, node);
		if (targetNode == nullptr)
			REPORT_ERROR_NULL("Expected veriable name");

		if (StrCmp(targetNode->expression, "constantInt"))
		{
			AddOperation(OperationCodeLoadConstant, OperationArgument(4), OperationArgument(StrToInt(targetNode->lexema)));
			return TryGetType("int");
		}

		if (StrCmp(targetNode->expression, "constantFloat"))
		{
			AddOperation(OperationCodeLoadConstant, OperationArgument(4), OperationArgument(StrToFloat(targetNode->lexema)));
			return TryGetType("float");
		}

		REPORT_ERROR_NULL("Unexpected constant type");
	}

	for (auto child : node->childs)
	{
		auto t = RecursiveAnalyzeNodeExpression(child, addressEnabled, returnEnabled);
		if (t != nullptr)
			return t;
	}
	return nullptr;
}

void SemanticAnalyzer::RecursiveAnalyzeNodeBlock(ParseNode * node)
{
	for (auto child : node->childs)
	{
		if (StrCmp(child->expression, "expression"))
			RecursiveAnalyzeNodeExpression(child);
		else if (StrCmp(child->expression, "veriableDeclareInit"))
			RecursiveAnalyzeNodeDeclareLocalVeriable(child, true);
		else if (StrCmp(child->expression, "veriableDeclare"))
			RecursiveAnalyzeNodeDeclareLocalVeriable(child, false);
		else if (StrCmp(child->expression, "statementReturn"))
			RecursiveAnalyzeNodeReturn(child);
		else if (StrCmp(child->expression, "statementCondition"))
			RecursiveAnalyzeNodeCondition(child);
		else
			RecursiveAnalyzeNodeBlock(child);
	}
}

void SemanticAnalyzer::RecursiveAnalyzeNodeReturn(ParseNode * node)
{
	auto nodeTarget = StepInLexema(node, node);
	if (!StrCmp(nodeTarget->expression, "return"))
		REPORT_ERROR("Expected return");

	nodeTarget = StepInExpression(nodeTarget, node);
	RecursiveAnalyzeNodeExpression(nodeTarget, false, true);

	AddOperation(OperationCodeRet);
}

void SemanticAnalyzer::RecursiveAnalyzeNodeCondition(ParseNode * node)
{
	auto nodeTarget = StepInLexema(node, node);
	if (!StrCmp(nodeTarget->expression, "if"))
		REPORT_ERROR("Expected if");

	nodeTarget = StepInLexema(nodeTarget, node);
	if (!StrCmp(nodeTarget->expression, "("))
		REPORT_ERROR("Expected (");

	nodeTarget = StepInExpression(nodeTarget, node);
	RecursiveAnalyzeNodeExpression(nodeTarget, false, true);
	nodeTarget = StepToLastExpression(nodeTarget);

	auto& function = functions->back();
	function.labels.push_back(Label());
	auto& label = function.labels.back();
	StrFmt(label.name, "LABEL_%d", labelCounter++);

	nodeTarget = StepInLexema(nodeTarget, node);
	if (!StrCmp(nodeTarget->expression, ")"))
		REPORT_ERROR("Expected )");

	AddOperation(OperationCodeBreakFalse, OperationArgument(&label));

	nodeTarget = StepInExpression(nodeTarget, node);
	RecursiveAnalyzeNodeBlock(nodeTarget);

	AddOperation(OperationCodeLabel, OperationArgument(&label));
}

void SemanticAnalyzer::AddOperation(OperationCode code, OperationArgument argument1, OperationArgument argument2)
{
	auto& function = functions->back();
	function.operations.push_back(Operation());
	auto& operation = function.operations.back();
	operation.code = code;
	operation.arguments.push_back(argument1);
	operation.arguments.push_back(argument2);
}

void SemanticAnalyzer::AddOperation(OperationCode code, OperationArgument argument)
{
	auto& function = functions->back();
	function.operations.push_back(Operation());
	auto& operation = function.operations.back();
	operation.code = code;
	operation.arguments.push_back(argument);
}

void SemanticAnalyzer::AddOperation(OperationCode code)
{
	auto& function = functions->back();
	function.operations.push_back(Operation());
	auto& operation = function.operations.back();
	operation.code = code;
}

void SemanticAnalyzer::StrFmt(char * first, char * format, ...)
{
	va_list ap;
	va_start(ap, format);
	vsprintf_s(first, 256, format, ap);
}

ParseNode* SemanticAnalyzer::StepInExpression(ParseNode* node, ParseNode* parent)
{
	if (!node->childs.empty())
		return node->childs.front();

	while (true)
	{
		if (node != parent)
		{
			auto& childs = node->parent->childs;
			auto itr = childs.begin();
			for (; itr != childs.end(); itr++)
			{
				if (*itr == node)
				{
					itr++;
					break;
				}
			}

			if (itr != childs.end())
			{
				return *itr;
			}
			else
			{
				node = node->parent;
			}
			continue;
		}

		return nullptr;
	}
}

ParseNode * SemanticAnalyzer::StepOutExpression(ParseNode * node, ParseNode * parent)
{
	if (node == parent)
		return nullptr;
	node = node->parent;
	return StepInExpression(node, parent);
}

ParseNode * SemanticAnalyzer::StepToLastExpression(ParseNode* node)
{
	while (true)
	{
		if (node->childs.empty())
			return node;
		node = node->childs.back();
	}
}

ParseNode * SemanticAnalyzer::StepInLexema(ParseNode* node, ParseNode* parent)
{
	while (true)
	{
		node = StepInExpression(node, parent);
		if (node->childs.empty())
			return node;
	}
}

Type * SemanticAnalyzer::TryGetType(char * name)
{
	for (auto& type : *types)
		if (StrCmp(type.name, name))
			return &type;
	return nullptr;
}

Function * SemanticAnalyzer::TryGetFunction(char * name, std::vector<Type*>& parameters)
{
	for (auto& function : *functions)
		if (StrCmp(function.name, name))
		{
			if (function.parameters.size() != parameters.size())
				continue;
			for (int i = 0; i < function.parameters.size(); i++)
			{
				if (function.parameters[i].type != parameters[i])
					continue;
			}
			return &function;
		}
	return nullptr;
}

Type * SemanticAnalyzer::AddOperationFunction(char * name, Type * parameter1, Type * parameter2, bool returnEnabled)
{
#define AUTO_OPERATION(Name, ArgumenType, ReturnType, OperationName) \
if (StrCmp(name, Name) && StrCmp(parameter1->name, ArgumenType) && StrCmp(parameter2->name, ArgumenType)) \
{ \
	AddOperation(OperationName); \
	if (!returnEnabled) \
		AddOperation(OperationCodeRetSkip); \
	return TryGetType(ReturnType); \
}
	if (StrCmp(name, "operator=") && StrCmp(parameter1->name, "int") && StrCmp(parameter2->name, "int"))
	{
		AddOperation(returnEnabled ? OperationCodeAssign : OperationCodeStore);
		return TryGetType("int");
	}

	//AUTO_OPERATION("operator=", "int", "int", OperationCodeStore);
	AUTO_OPERATION("operator+", "int", "int", OperationCodePlus);
	AUTO_OPERATION("operator-", "int", "int", OperationCodeMinus);
	AUTO_OPERATION("operator*", "int", "int", OperationCodeMult);
	AUTO_OPERATION("operator/", "int", "int", OperationCodeDiv);
	AUTO_OPERATION("operator==", "int", "bool", OperationCodeCmp);

	AUTO_OPERATION("operator==", "bool", "bool", OperationCodeCmp);
	AUTO_OPERATION("operator||", "bool", "bool", OperationCodeOr);
	AUTO_OPERATION("operator&&", "bool", "bool", OperationCodeAnd);

#undef AUTO_OPERATION

	std::vector<Type*> parameters;
	parameters.push_back(parameter1);
	parameters.push_back(parameter2);
	auto function = TryGetFunction(name, parameters);

	if (function == nullptr)
		REPORT_ERROR_NULL("Function %s(count=%d) is not declared", name, parameters.size());

	AddOperation(OperationCodeCall, OperationArgument(function));
	if (!returnEnabled)
		AddOperation(OperationCodeRetSkip);
	return function->type;
}

Type * SemanticAnalyzer::AddOperationFunction(char * name, std::vector<Type*> parameters, bool returnEnabled)
{
	auto function = TryGetFunction(name, parameters);

	if (function == nullptr)
		REPORT_ERROR_NULL("Function %s(count=%d) is not declared", name, parameters.size());

	AddOperation(OperationCodeCall, OperationArgument(function));
	if (!returnEnabled)
		AddOperation(OperationCodeRetSkip);
	return function->type;
}

Veriable * SemanticAnalyzer::TryGetVeriable(char * name)
{
	auto veriable = TryGetParameter(name);
	if (veriable != nullptr)
		return veriable;

	veriable = TryGetLocalVeriable(name);
	if (veriable != nullptr)
		return veriable;

	veriable = TryGetGlobalVeriable(name);
	if (veriable != nullptr)
		return veriable;

	return nullptr;
}

Veriable * SemanticAnalyzer::TryGetLocalVeriable(char * name)
{
	for (auto& veriable : *veriables)
		if (StrCmp(veriable.name, name))
			return &veriable;
	return nullptr;
}

Veriable * SemanticAnalyzer::TryGetParameter(char * name)
{
	auto& function = functions->back();
	for (auto& veriable : function.parameters)
		if (StrCmp(veriable.name, name))
			return &veriable;
	return nullptr;
}

Veriable * SemanticAnalyzer::TryGetGlobalVeriable(char * name)
{
	auto& function = functions->back();
	for (auto& veriable : function.veriables)
		if (StrCmp(veriable.name, name))
			return &veriable;
	return nullptr;
}

int SemanticAnalyzer::TryGetParameterIndex(char * name)
{
	auto& function = functions->back();
	for (int i = 0; i < function.parameters.size(); i++)
		if (StrCmp(function.parameters[i].name, name))
			return i;
	return -1;
}

int SemanticAnalyzer::TryGetLocalVeriableIndex(char * name)
{
	auto& function = functions->back();
	for (int i = 0; i < function.veriables.size(); i++)
		if (StrCmp(function.veriables[i].name, name))
			return i;
	return -1;
}
