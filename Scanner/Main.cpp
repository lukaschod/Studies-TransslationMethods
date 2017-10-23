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
		printf("%d %s\n", lexeme->type, lexeme->data);
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

	auto scanner = new Scanner();

	error = scanner->LoadFromFile(argv[1]);
	if (error != nullptr)
	{
		PrintError(error);
		delete scanner;
		return -1;
	}

	std::vector<Lexema*> lexemas;
	error = scanner->LoadLexemasFromFile(argv[2], lexemas);
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