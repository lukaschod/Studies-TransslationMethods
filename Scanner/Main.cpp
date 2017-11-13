#include "Scanner.h"

void PrintError(Error* error)
{
	if (error == nullptr)
		return;

	printf("ERROR: %d %s\n", error->id, error->message);
	delete error;
}

void PrintLexemes(std::vector<Lexema*>& lexemas)
{
	printf("Lexemes Table\n");
	for (auto lexeme : lexemas)
	{
		printf("%s %s\n", lexeme->type, lexeme->data);
	}
}

int main(int argc, const char * argv[])
{
	Error* error;

	if (argc != 3)
	{
		PrintError(new Error("Program requires to pass [pathToStates] [pathToPorgram]"));
		return -1;
	}

	FileToBuffer fileToBuffer(2 ^ 16);

	auto scanner = new Scanner();

	fileToBuffer.ReadFile(argv[1]);
	error = scanner->LoadFromMemory(fileToBuffer.buffer, fileToBuffer.size);
	if (error != nullptr)
	{
		PrintError(error);
		delete scanner;
		return -1;
	}

	std::vector<Lexema*> lexemas;
	fileToBuffer.ReadFile(argv[2]);
	error = scanner->LoadLexemasFromMemory(fileToBuffer.buffer, fileToBuffer.size, lexemas);
	if (error != nullptr)
	{
		PrintError(error);
		delete scanner;
		return -1;
	}

	PrintLexemes(lexemas);

	delete scanner;
	return 0;
}