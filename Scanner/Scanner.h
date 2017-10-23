#pragma once

#include "StateMachine.h"

enum LexemaType
{
	kLexemaTypeNone,
	kLexemaTypeStart,
	kLexemaTypeEnd,
	kLexemaTypeLetters,
	kLexemaTypeInt,
	kLexemaTypeFloat,
};

class Lexema
{
public:
	Lexema(char* data, size_t size, LexemaType type) :
		type(type)
	{
		memcpy(this->data, data, size);
		this->data[size] = 0;
	}

public:
	LexemaType type;
	char data[128];
};

class Scanner
{
public:
	Scanner();
	~Scanner();

	Error* LoadFromFile(const char* pathToFile);
	Error* LoadLexemasFromFile(const char* pathToFile, std::vector<Lexema*>& outLexemos);

private:
	StateMachine* stateMachine;
};