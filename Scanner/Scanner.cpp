#include "Scanner.h"
#include <queue>

Scanner::Scanner()
{
	stateMachine = new StateMachine();
}

Scanner::~Scanner()
{
	delete stateMachine;
}

Error* Scanner::LoadFromFile(const char* pathToFile)
{
	return stateMachine->LoadFromFile(pathToFile);
}

Error* Scanner::LoadLexemasFromFile(const char* pathToFile, std::vector<Lexema*>& outLexemos)
{
	auto file = fopen(pathToFile, "rb");
	if (file == nullptr)
		return new Error("File in path %s was not found", pathToFile);

	// TODO: Make normal reading
	char begin[5000];
	auto readed = fread(begin, sizeof(char), 5000, file);
	if (readed == 0)
		return new Error("Failed to read file");
	begin[readed] = 0;

	char* current = begin;
	char* end = begin + readed;

	while (true)
	{
		// Check if end of file
		if (current == end)
			break;

		auto last = current;
		auto result = stateMachine->IsInputAcceptable(&current);

		// Make sure valid lexema found
		if (result == kStateMachineExitFlagNone)
			return new Error("Failed to find lexema");

		if (result == kStateMachineExitFlagEnd)
			continue;

		auto lexema = new Lexema(last, current - last, (LexemaType)result);
		outLexemos.push_back(lexema);
	}

	fclose(file);

	return nullptr;
}
