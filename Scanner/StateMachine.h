#pragma once

#pragma warning(disable:4996)

#include <vector>
#include <cstdarg>

class Error
{
public:
	Error(const char* format, ...) : id(0) 
	{
		va_list ap;
		va_start(ap, format);
		vsprintf_s(message, 256, format, ap);
	}

	~Error()
	{
	}

public:
	uint32_t id;
	char message[256];
};

class Range
{
public:
	Range() {}
	Range(uint32_t start, size_t end) : start(start), end(end) {}

	inline bool Contains(uint32_t value) { return start <= value && value <= end; }

public:
	uint32_t start;
	uint32_t end;
};

enum StateMachineExitFlag
{
	kStateMachineExitFlagNone,
	kStateMachineExitFlagStart,
	kStateMachineExitFlagEnd,
	kStateMachineExitFlagUnknown0,
	kStateMachineExitFlagUnknown1,
	kStateMachineExitFlagUnknown2,
	kStateMachineExitFlagUnknown3,
	kStateMachineExitFlagUnknown4,
	kStateMachineExitFlagUnknown5,
	kStateMachineExitFlagUnknown6,
	kStateMachineExitFlagUnknown7,
	kStateMachineExitFlagUnknown8,
	kStateMachineExitFlagUnknown9,
};

class StateMachine
{
private:
	struct State;
	struct Connection
	{
		State* target;
		Range allowedInputRange;
		Connection() {}
		Connection(State* target, Range allowedInputRange) : target(target), allowedInputRange(allowedInputRange) {}
	};

	struct State
	{
		StateMachineExitFlag flag;
		std::vector<Connection> connections;
		char name[128];
	};

public:
	StateMachine();
	~StateMachine();

	Error* LoadFromFile(const char* pathToFile);
	StateMachineExitFlag IsInputAcceptable(char** input);

private:
	State* TryFindState(char* name);

private:
	std::vector<State*> states;
	State* startState;
	State* currentState;
};