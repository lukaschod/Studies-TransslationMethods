#pragma once

#include "StateMachine.h"
#include "Common.h"

class Lexema
{
public:
	Lexema(char* data, size_t size, const char* type)
	{
		memcpy(this->data, data, size);
		strcpy(this->type, type);
		this->data[size] = 0;
	}

public:
	char data[128];
	char type[128];
};

class Scanner
{
public:
	Scanner();
	~Scanner();

	Error* LoadFromMemory(char* data, size_t size);
	Error* LoadLexemasFromMemory(char* data, size_t size, std::vector<Lexema*>& outLexemos);

private:
	StateMachine* stateMachine;
};