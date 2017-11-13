#include "Parser.h"

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
		writer.WriteFormated("<%s 'lexema'='%s'/>\n", node->expression, node->lexema);
	}
	else
	{
		PrintSpacing(writer, level);
		writer.WriteFormated("<%s>\n", node->expression);

		for (auto child : node->childs)
		{
			PrintParseTreeNode(writer, child, level + 1);
		}

		PrintSpacing(writer, level);
		writer.WriteFormated("</%s>\n", node->expression);
	}

	
}

void PrintParseTree(BufferWriter& writer, ParseTree* tree)
{
	PrintParseTreeNode(writer, tree->root, 0);
}

int main(int argc, const char * argv[])
{
	Error* error;

	if (argc != 6)
	{
		PrintError(new Error("Program requires to pass [pathToStates] [pathToPorgram] [pathToExpressions]"));
		return -1;
	}

	FileToBuffer fileToBuffer(65000);

	Scanner scanner;

	fileToBuffer.ReadFile(argv[1]);
	error = scanner.LoadFromMemory(fileToBuffer.buffer, fileToBuffer.size);
	if (error != nullptr)
	{
		PrintError(error);
		return -1;
	}

	std::vector<Lexema*> lexemas;
	fileToBuffer.ReadFile(argv[2]);
	error = scanner.LoadLexemasFromMemory(fileToBuffer.buffer, fileToBuffer.size, lexemas);
	if (error != nullptr)
	{
		PrintError(error);
		return -1;
	}

	BufferWriter lexemosWriter(fileToBuffer.buffer, fileToBuffer.buffer + fileToBuffer.capacity);
	PrintLexemes(lexemosWriter, lexemas);
	fileToBuffer.size = lexemosWriter.pointer - lexemosWriter.begin;
	fileToBuffer.WriteFile(argv[4]);
	

	Parser parser;

	fileToBuffer.ReadFile(argv[3]);
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
	fileToBuffer.WriteFile(argv[5]);

	__debugbreak();

	return 0;
}