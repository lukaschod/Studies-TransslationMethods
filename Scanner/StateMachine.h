#pragma once

#include <vector>
#include "Common.h"

class StateMachine
{
private:
	struct State;
	struct Connection
	{
		State* target;
		Range allowedInputRange;
		bool assert;
		Connection() {}
		Connection(State* target, Range allowedInputRange, bool assert) : 
			target(target), 
			allowedInputRange(allowedInputRange),
			assert(assert)
			{}
	};

	struct State
	{
		std::vector<Connection> connections;
		char name[128];
		char type[128];
	};

	struct IsInputAcceptableContext
	{
		State* state;
		char* input;

		IsInputAcceptableContext(State* state, char* input) :
			state(state),
			input(input)
		{
		}
	};

public:
	StateMachine();
	~StateMachine();

	Error* LoadFromMemory(char* data, size_t size);
	const char* IsInputAcceptable(char** input);
	const char* IsInputAcceptableND(char** input);

private:
	State* TryFindState(char* name);

private:
	std::vector<State*> states;
	State* startState;
	State* currentState;
};