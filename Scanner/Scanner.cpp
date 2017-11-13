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

Error* Scanner::LoadFromMemory(char* data, size_t size)
{
	return stateMachine->LoadFromMemory(data, size);
}

Error* Scanner::LoadLexemasFromMemory(char* data, size_t size, std::vector<Lexema*>& outLexemos)
{
	char* current = (char*)data;
	char* end = (char*) data + size;

	while (true)
	{
		// Check if end of file
		if (current == end)
			break;

		auto last = current;
		auto result = stateMachine->IsInputAcceptableND(&current);

		// Make sure valid lexema found
		if (strcmp(result, "invalid") == 0)
		{
			stateMachine->IsInputAcceptableND(&current); // For debugging
			return new Error("Failed to find lexema");
		}

		if (strcmp(result, "skip") == 0)
			continue;

		auto lexema = new Lexema(last, current - last, result);
		outLexemos.push_back(lexema);
	}
	return nullptr;
}
