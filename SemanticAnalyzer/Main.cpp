#include "SemanticAnalyzer.h"

void PrintError(Error* error)
{
	if (error == nullptr)
		return;

	printf("ERROR: %d %s\n", error->id, error->message);
	delete error;
	__debugbreak();
}

void PrintLexemes(BufferWriter& writer, std::vector<Lexema*>& lexemas)
{
	int number = 0;
	for (auto lexeme : lexemas)
	{
		writer.WriteFormated("%d %s %s\n", number++, lexeme->type, lexeme->data);
	}
}

void PrintSpacing(BufferWriter& writer, int level)
{
	if (level == 0)
		return;

	char buffer[128];
	memset(buffer, 32, 128 * sizeof(char));
	buffer[127] = 0;

	for (int i = 0; i < level && i < 128 / 2; i++)
		buffer[level * 2] = 0;

	writer.WriteFormated("%s", buffer);
}

void PrintParseTreeNode(BufferWriter& writer, ParseNode* node, int level)
{
	if (node->lexema[0] != 0)
	{
		PrintSpacing(writer, level);
		writer.WriteFormated("<_%s 'lexema'='%s'/>\n", node->expression, node->lexema);
	}
	else
	{
		PrintSpacing(writer, level);
		writer.WriteFormated("<_%s>\n", node->expression);

		for (auto child : node->childs)
		{
			PrintParseTreeNode(writer, child, level + 1);
		}

		PrintSpacing(writer, level);
		writer.WriteFormated("</_%s>\n", node->expression);
	}

	
}

void PrintParseTree(BufferWriter& writer, ParseTree* tree)
{
	PrintParseTreeNode(writer, tree->root, 0);
}

void PrintCode(BufferWriter& writer, Code* code)
{
	// Add main function
	writer.WriteFormated("call Main\nhalt\n\n");

	// Add global veriables
	for (auto& veriable : code->veriables)
	{
		writer.WriteFormated(".data %s \"", veriable.name);
		
		// TODO: Make unitialized allocation
		for (int i = 0; i < veriable.type->size; i++)
		{
			writer.WriteFormated("0", veriable.name);
		}
		writer.WriteFormated("\"\n");
	}

	// Add functions
	for (auto& function : code->functions)
	{
		writer.WriteFormated("func %s %d %d\n", function.name, function.parameters.size(), function.veriables.size());

		// Add operations
		int level = 1;
		for (auto& operation : function.operations)
		{
			PrintSpacing(writer, level);
			switch (operation.code)
			{
			case OperationCodeCmp:
				writer.WriteFormated("ceq\n");
				break;
			case OperationCodeAnd:
				writer.WriteFormated("and\n");
				break;
			case OperationCodeOr:
				writer.WriteFormated("or\n");
				break;

			case OperationCodePlus:
				writer.WriteFormated("add\n");
				break;
			case OperationCodeMinus:
				writer.WriteFormated("sub\n");
				break;
			case OperationCodeMult:
				writer.WriteFormated("mul\n");
				break;
			case OperationCodeDiv:
				writer.WriteFormated("div\n");
				break;
			
			case OperationCodeCall:
				writer.WriteFormated("call %s\n", operation.arguments[0].function->name);
				break;
			case OperationCodeRet:
				writer.WriteFormated("ret\n");
				break;
			case OperationCodeEnd:
				writer.WriteFormated("end\n");
				break;
			case OperationCodeBreakFalse:
				writer.WriteFormated("brfalse %s\n", operation.arguments[0].label->name);
				break;

			case OperationCodeLoadArgument:
				writer.WriteFormated("ldarg.%d\n", (int)operation.arguments[0].constant);
				break;
			case OperationCodeLoadArgumentAddress:
				writer.WriteFormated("ldarga.%d\n", (int) operation.arguments[0].constant);
				break;
			case OperationCodeLoadLocal:
				writer.WriteFormated("ldloc.%d\n", (int) operation.arguments[0].constant);
				break;
			case OperationCodeLoadLocalAddress:
				writer.WriteFormated("ldloca.%d\n", (int) operation.arguments[0].constant);
				break;
			case OperationCodeLoadGlobal:
				writer.WriteFormated("ldc.%d %s\n", 4, operation.arguments[0].veriable->name);
				break;
			case OperationCodeLoadGlobalAddress:
				writer.WriteFormated("ldc.%d %s\n", 4, operation.arguments[0].veriable->name);
				writer.WriteFormated("lda.%d\n", 4);
				break;
			case OperationCodeLoadConstant:
				writer.WriteFormated("ldc.%d %d\n", (int) operation.arguments[0].constant, (int)operation.arguments[1].constant);
				break;

			case OperationCodeStore:
				writer.WriteFormated("sta.%d\n", 4);
				break;
			case OperationCodeAssign:
				writer.WriteFormated("asi.%d\n", 4);
				break;

			case OperationCodeRetSkip:
				writer.WriteFormated("retskip\n");
				break;
			case OperationCodeLabel:
				writer.WriteFormated(".label %s\n", operation.arguments[0].label->name);
				break;
			}
		}

		// Seperate by new line
		writer.WriteFormated("\n");
	}
}

int main(int argc, const char * argv[])
{
	Error* error;

	if (argc != 7)
	{
		PrintError(new Error("Program requires to pass [pathToStates] [pathToPorgram] [pathToExpressions]"));
		return -1;
	}

	FileToBuffer fileToBuffer(65000);

	Scanner scanner;

	error = fileToBuffer.ReadFile(argv[1]);
	if (error != nullptr)
	{
		PrintError(error);
		return -1;
	}

	error = scanner.LoadFromMemory(fileToBuffer.buffer, fileToBuffer.size);
	if (error != nullptr)
	{
		PrintError(error);
		return -1;
	}

	std::vector<Lexema*> lexemas;
	error = fileToBuffer.ReadFile(argv[2]);
	if (error != nullptr)
	{
		PrintError(error);
		return -1;
	}

	error = scanner.LoadLexemasFromMemory(fileToBuffer.buffer, fileToBuffer.size, lexemas);
	if (error != nullptr)
	{
		PrintError(error);
		return -1;
	}

	BufferWriter lexemosWriter(fileToBuffer.buffer, fileToBuffer.buffer + fileToBuffer.capacity);
	PrintLexemes(lexemosWriter, lexemas);
	fileToBuffer.size = lexemosWriter.pointer - lexemosWriter.begin;
	error = fileToBuffer.WriteFile(argv[4]);
	if (error != nullptr)
	{
		PrintError(error);
		return -1;
	}

	Parser parser;

	error = fileToBuffer.ReadFile(argv[3]);
	if (error != nullptr)
	{
		PrintError(error);
		return -1;
	}

	error = parser.AddExpresionsFromMemory(fileToBuffer.buffer, fileToBuffer.size);
	if (error != nullptr)
	{
		PrintError(error);
		return -1;
	}

	ParseTree tree;
	error = parser.Parse("program", lexemas, &tree);
	if (error != nullptr)
	{
		PrintError(error);
		return -1;
	}

	BufferWriter treeWriter(fileToBuffer.buffer, fileToBuffer.buffer + fileToBuffer.capacity);
	PrintParseTree(treeWriter, &tree);
	fileToBuffer.size = treeWriter.pointer - treeWriter.begin;
	error = fileToBuffer.WriteFile(argv[5]);
	if (error != nullptr)
	{
		PrintError(error);
		return -1;
	}

	SemanticAnalyzer analyzer;
	Code code;
	analyzer.GenerateCodeFromTree(&tree, &code);
	if (code.errors.size() != 0)
	{
		for (auto error : code.errors)
			PrintError(error);
		return -1;
	}

	BufferWriter codeWriter(fileToBuffer.buffer, fileToBuffer.buffer + fileToBuffer.capacity);
	PrintCode(codeWriter, &code);
	fileToBuffer.size = codeWriter.pointer - codeWriter.begin;
	error = fileToBuffer.WriteFile(argv[6]);
	if (error != nullptr)
	{
		PrintError(error);
		return -1;
	}

	return 0;
}