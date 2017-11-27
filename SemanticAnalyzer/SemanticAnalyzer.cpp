#include "SemanticAnalyzer.h"

SemanticAnalyzer::SemanticAnalyzer()
{
}

SemanticAnalyzer::~SemanticAnalyzer()
{
}

Error* SemanticAnalyzer::GenerateCodeFromTree(ParseTree* tree)
{
	types.push_back(Type());
	auto type = &types.back();
	StrCpy(type->name, "void");

	types.push_back(Type());
	type = &types.back();
	StrCpy(type->name, "int");

	types.push_back(Type());
	type = &types.back();
	StrCpy(type->name, "float");

	types.push_back(Type());
	type = &types.back();
	StrCpy(type->name, "label");

	labelCounter = 0;

	RecursiveAnalyzeNode(tree->root);
	return nullptr;
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

#define REPORT_ERROR(...) { errors.push_back(new Error(__VA_ARGS__)); return; }

void SemanticAnalyzer::RecursiveAnalyzeNodeDeclareVeriable(ParseNode* node, bool init)
{
	veriables.push_back(Veriable());
	auto& veriable = veriables.back();

	auto nodeTarget = SetpInLexema(node, node);
	if (nodeTarget == nullptr)
		REPORT_ERROR("Expected function type");

	veriable.type = TryGetType(nodeTarget->lexema);
	if (veriable.type == nullptr)
		REPORT_ERROR("Unknown type %s", nodeTarget->lexema);

	nodeTarget = SetpInLexema(nodeTarget, node);
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
	auto& function = functions.back();

	function.veriables.push_back(Veriable());
	auto& veriable = function.veriables.back();

	auto nodeTarget = SetpInLexema(node, node);
	if (nodeTarget == nullptr)
		REPORT_ERROR("Expected function type");

	veriable.type = TryGetType(nodeTarget->lexema);
	if (veriable.type == nullptr)
		REPORT_ERROR("Unknown type %s", nodeTarget->lexema);

	nodeTarget = SetpInLexema(nodeTarget, node);
	if (nodeTarget == nullptr || !StrCmp(nodeTarget->expression, "name"))
		REPORT_ERROR("Expected function type");
	StrCpy(veriable.name, nodeTarget->lexema);

	if (init)
	{
		nodeTarget = SetpInLexema(nodeTarget, node);
		if (!StrCmp(nodeTarget->expression, "="))
			REPORT_ERROR("Expected =");

		nodeTarget = SetpInExpression(nodeTarget, node);
		RecursiveAnalyzeNodeExpression(nodeTarget);
	}

	AddOperation(OperationCodeStore, &veriable);
}

void SemanticAnalyzer::RecursiveAnalyzeNodeDeclareFunction(ParseNode * node)
{
	functions.push_back(Function());
	auto& function = functions.back();

	auto nodeTarget = SetpInLexema(node, node);
	if (nodeTarget == nullptr)
		REPORT_ERROR("Expected function type");

	function.type = TryGetType(nodeTarget->lexema);
	if (function.type == nullptr)
		REPORT_ERROR("Unknown type %s", nodeTarget->lexema);

	nodeTarget = SetpInLexema(nodeTarget, node);
	if (nodeTarget == nullptr || !StrCmp(nodeTarget->expression, "name"))
		REPORT_ERROR("Expected function name");

	auto functionDuplicate = TryGetFunction(nodeTarget->lexema);
	if (functionDuplicate != nullptr)
		REPORT_ERROR("Duplicated function");
	StrCpy(function.name, nodeTarget->lexema);

	nodeTarget = SetpInLexema(nodeTarget, node);
	if (!StrCmp(nodeTarget->expression, "("))
		REPORT_ERROR("Expected (");

	while (true)
	{
		nodeTarget = SetpInLexema(nodeTarget, node);
		if (nodeTarget == nullptr)
			REPORT_ERROR("Expected parameter");

		if (StrCmp(nodeTarget->expression, ")"))
			break;

		function.parameters.push_back(Veriable());
		auto& paramter = function.parameters.back();

		paramter.type = TryGetType(nodeTarget->lexema);
		if (paramter.type == nullptr)
			REPORT_ERROR("Unknown type %s", nodeTarget->lexema);

		nodeTarget = SetpInLexema(nodeTarget, node);
		StrCpy(paramter.name, nodeTarget->lexema);
	}

	RecursiveAnalyzeNodeBlock(node);

	if (StrCmp(function.type->name, "void"))
	{
		AddOperation(OperationCodeEnd);
	}
}

void SemanticAnalyzer::RecursiveAnalyzeNodeExpression(ParseNode * node)
{
	if (StrCmp(node->expression, "expression") && node->childs.size() == 3)
	{
		RecursiveAnalyzeNodeExpression(node->childs[0]);
		RecursiveAnalyzeNodeExpression(node->childs[2]);

		auto targetNode = SetpInLexema(node->childs[1], node);
		if (!StrCmp(targetNode->lexema, "="))
			REPORT_ERROR("Expected =");

		AddOperation(OperationCodeStoreArgumentless);
		return;
	}

	if (StrCmp(node->expression, "expressionAnd") && node->childs.size() == 3)
	{
		RecursiveAnalyzeNodeExpression(node->childs[0]);
		RecursiveAnalyzeNodeExpression(node->childs[2]);

		auto targetNode = SetpInLexema(node->childs[1], node);
		if (!StrCmp(targetNode->lexema, "&"))
			REPORT_ERROR("Expected &");

		targetNode = SetpInLexema(targetNode, node);
		if (!StrCmp(targetNode->lexema, "&"))
			REPORT_ERROR("Expected &");

		AddOperation(OperationCodeAnd);
		return;
	}

	if (StrCmp(node->expression, "expressionOr") && node->childs.size() == 3)
	{
		RecursiveAnalyzeNodeExpression(node->childs[0]);
		RecursiveAnalyzeNodeExpression(node->childs[2]);

		auto targetNode = SetpInLexema(node->childs[1], node);
		if (!StrCmp(targetNode->lexema, "|"))
			REPORT_ERROR("Expected |");

		targetNode = SetpInLexema(targetNode, node);
		if (!StrCmp(targetNode->lexema, "|"))
			REPORT_ERROR("Expected |");

		AddOperation(OperationCodeOr);
		return;
	}

	if (StrCmp(node->expression, "expressionCmp") && node->childs.size() == 3)
	{
		RecursiveAnalyzeNodeExpression(node->childs[0]);
		RecursiveAnalyzeNodeExpression(node->childs[2]);

		auto targetNode = SetpInLexema(node->childs[1], node);
		if (!StrCmp(targetNode->lexema, "="))
			REPORT_ERROR("Expected =");

		targetNode = SetpInLexema(targetNode, node);
		if (!StrCmp(targetNode->lexema, "="))
			REPORT_ERROR("Expected =");

		AddOperation(OperationCodeCmp);
		return;
	}

	if (StrCmp(node->expression, "expressionAdd") && node->childs.size() == 3)
	{
		RecursiveAnalyzeNodeExpression(node->childs[0]);
		RecursiveAnalyzeNodeExpression(node->childs[2]);

		auto targetNode = SetpInLexema(node->childs[1], node);
		if (targetNode == nullptr)
			REPORT_ERROR("Expected operator");

		if (StrCmp(targetNode->lexema, "+"))
			AddOperation(OperationCodePlus);
		else if (StrCmp(targetNode->lexema, "-"))
			AddOperation(OperationCodeMinus);
		return;
	}

	if (StrCmp(node->expression, "expressionMult") && node->childs.size() == 3)
	{
		RecursiveAnalyzeNodeExpression(node->childs[0]);
		RecursiveAnalyzeNodeExpression(node->childs[2]);

		auto targetNode = SetpInLexema(node->childs[1], node);
		if (targetNode == nullptr)
			REPORT_ERROR("Expected operator");

		if (StrCmp(targetNode->lexema, "*"))
			AddOperation(OperationCodeMult);
		else if (StrCmp(targetNode->lexema, "/"))
			AddOperation(OperationCodeDiv);
		return;
	}

	if (StrCmp(node->expression, "veriableName"))
	{
		auto targetNode = SetpInLexema(node, node);
		if (targetNode == nullptr)
			REPORT_ERROR("Expected veriable name");

		auto parameterIndex = TryGetParameterIndex(targetNode->lexema);
		if (parameterIndex != -1)
		{
			AddOperation(OperationCodeLoadArgument, OperationArgument(parameterIndex));
			return;
		}

		auto localVeriableIndex = TryGetLocalVeriableIndex(targetNode->lexema);
		if (localVeriableIndex != -1)
		{
			AddOperation(OperationCodeLoadLocal, OperationArgument(localVeriableIndex));
			return;
		}

		auto veriable = TryGetGlobalVeriable(targetNode->lexema);
		if (veriable != nullptr)
		{
			AddOperation(OperationCodeLoadAddress, OperationArgument(veriable));
			return;
		}

		REPORT_ERROR("Used of undeclared veriable %s", targetNode->lexema);

		return;
	}

	if (StrCmp(node->expression, "functionCall"))
	{
		auto targetNode = SetpInLexema(node, node);
		if (targetNode == nullptr)
			REPORT_ERROR("Expected function name");

		auto function = TryGetFunction(targetNode->lexema);
		if (function == nullptr)
			REPORT_ERROR("Used of undeclared function %s", targetNode->lexema);
		
		targetNode = SetpInLexema(targetNode, node);
		if (!StrCmp(targetNode->expression, "("))
			REPORT_ERROR("Expected (");

		for (auto paramter : function->parameters)
		{
			RecursiveAnalyzeNodeExpression(targetNode);

			// Skip , or )
			targetNode = SetpInLexema(targetNode, node);
		}

		AddOperation(OperationCodeCall, function);

		return;
	}

	if (StrCmp(node->expression, "constant"))
	{
		auto targetNode = SetpInLexema(node, node);
		if (targetNode == nullptr)
			REPORT_ERROR("Expected veriable name");

		if (!StrCmp(targetNode->expression, "constantInt"))
			REPORT_ERROR("Used unsupported type %s", targetNode->expression);

		AddOperation(OperationCodeLoadConstant, OperationArgument(StrToInt(targetNode->lexema)));

		return;
	}

	for (auto child : node->childs)
	{
		RecursiveAnalyzeNodeExpression(child);
	}
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
	auto nodeTarget = SetpInLexema(node, node);
	if (!StrCmp(nodeTarget->expression, "return"))
		REPORT_ERROR("Expected return");

	nodeTarget = SetpInExpression(nodeTarget, node);
	RecursiveAnalyzeNodeExpression(nodeTarget);

	AddOperation(OperationCodeRet);
}

void SemanticAnalyzer::RecursiveAnalyzeNodeCondition(ParseNode * node)
{
	auto nodeTarget = SetpInLexema(node, node);
	if (!StrCmp(nodeTarget->expression, "if"))
		REPORT_ERROR("Expected if");

	nodeTarget = SetpInLexema(nodeTarget, node);
	if (!StrCmp(nodeTarget->expression, "("))
		REPORT_ERROR("Expected (");

	nodeTarget = SetpInExpression(nodeTarget, node);
	RecursiveAnalyzeNodeExpression(nodeTarget);
	nodeTarget = StepToLastExpression(nodeTarget);

	auto& function = functions.back();
	function.labels.push_back(Label());
	auto& label = function.labels.back();
	StrFmt(label.name, "LABEL_%d", labelCounter++);

	nodeTarget = SetpInLexema(nodeTarget, node);
	if (!StrCmp(nodeTarget->expression, ")"))
		REPORT_ERROR("Expected )");

	AddOperation(OperationCodeBreakFalse, OperationArgument(&label));

	nodeTarget = SetpInExpression(nodeTarget, node);
	RecursiveAnalyzeNodeBlock(nodeTarget);

	AddOperation(OperationCodeLabel, OperationArgument(&label));
}

void SemanticAnalyzer::AddOperation(OperationCode code, OperationArgument argument)
{
	auto& function = functions.back();
	function.operations.push_back(Operation());
	auto& operation = function.operations.back();
	operation.code = code;
	operation.arguments.push_back(argument);
}

void SemanticAnalyzer::AddOperation(OperationCode code)
{
	auto& function = functions.back();
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

ParseNode* SemanticAnalyzer::SetpInExpression(ParseNode* node, ParseNode* parent)
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

ParseNode * SemanticAnalyzer::StepToLastExpression(ParseNode* node)
{
	while (true)
	{
		if (node->childs.empty())
			return node;
		node = node->childs.back();
	}
}

ParseNode * SemanticAnalyzer::SetpInLexema(ParseNode* node, ParseNode* parent)
{
	while (true)
	{
		node = SetpInExpression(node, parent);
		if (node->childs.empty())
			return node;
	}
}

SemanticAnalyzer::Type * SemanticAnalyzer::TryGetType(char * name)
{
	for (auto& type : types)
		if (StrCmp(type.name, name))
			return &type;
	return nullptr;
}

SemanticAnalyzer::Function * SemanticAnalyzer::TryGetFunction(char * name)
{
	for (auto& function : functions)
		if (StrCmp(function.name, name))
			return &function;
	return nullptr;
}

SemanticAnalyzer::Veriable * SemanticAnalyzer::TryGetVeriable(char * name)
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

SemanticAnalyzer::Veriable * SemanticAnalyzer::TryGetLocalVeriable(char * name)
{
	for (auto& veriable : veriables)
		if (StrCmp(veriable.name, name))
			return &veriable;
	return nullptr;
}

SemanticAnalyzer::Veriable * SemanticAnalyzer::TryGetParameter(char * name)
{
	auto& function = functions.back();
	for (auto& veriable : function.parameters)
		if (StrCmp(veriable.name, name))
			return &veriable;
	return nullptr;
}

SemanticAnalyzer::Veriable * SemanticAnalyzer::TryGetGlobalVeriable(char * name)
{
	auto& function = functions.back();
	for (auto& veriable : function.veriables)
		if (StrCmp(veriable.name, name))
			return &veriable;
	return nullptr;
}

int SemanticAnalyzer::TryGetParameterIndex(char * name)
{
	auto& function = functions.back();
	for (int i = 0; i < function.parameters.size; i++)
		if (StrCmp(function.parameters[i].name, name))
			return i;
	return -1;
}

int SemanticAnalyzer::TryGetLocalVeriableIndex(char * name)
{
	auto& function = functions.back();
	for (int i = 0; i < function.veriables.size; i++)
		if (StrCmp(function.veriables[i].name, name))
			return i;
	return -1;
}
