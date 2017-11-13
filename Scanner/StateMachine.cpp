#include "StateMachine.h"
#include <stdio.h>
#include <queue>

StateMachine::StateMachine() :
	currentState(nullptr),
	startState(nullptr)
{
	
}

StateMachine::~StateMachine()
{
}

Error* StateMachine::LoadFromMemory(char* data, size_t size)
{
	BufferReader reader(data, data + size);
	while (true)
	{
		auto lineStart = reader.pointer;
		if (!reader.MovePointerToFirst('\n'))
			break;

		char type[128];
		if (sscanf(lineStart, "%s", &type) <= 0)
			break;
		lineStart += strlen(type);

		if (strcmp(type, "//") == 0)
			continue;

		if (strcmp(type, "s") == 0)
		{
			auto state = new State();

			if (sscanf(lineStart, "%s %s", &state->name, &state->type) == 0)
				return new Error("Expected state name");

			states.push_back(state);
			continue;
		}

		if (strcmp(type, "c") == 0)
		{
			char firstStateName[128];
			char secondStateName[128];
			Range allowedInputRange;

			if (sscanf(lineStart, "%s %s %d-%d", &firstStateName, &secondStateName,
				&allowedInputRange.start, &allowedInputRange.end) == 0)
				return new Error("Expected connection(%d %d %d-%d)");

			auto stateFirst = TryFindState(firstStateName);
			auto stateSecond = TryFindState(secondStateName);

			if (stateFirst == nullptr)
				return new Error("Unknown state %s", firstStateName);

			if (stateSecond == nullptr)
				return new Error("Unknown state %s", secondStateName);

			stateFirst->connections.push_back(Connection(stateSecond, allowedInputRange, false));
			continue;
		}

		if (strcmp(type, "t") == 0)
		{
			char firstStateName[128];
			char secondStateName[128];
			Range allowedInputRange;

			if (sscanf(lineStart, "%s %s %d-%d", &firstStateName, &secondStateName,
				&allowedInputRange.start, &allowedInputRange.end) == 0)
				return new Error("Expected connection(%d %d %d-%d)");

			auto stateFirst = TryFindState(firstStateName);
			auto stateSecond = TryFindState(secondStateName);

			if (stateFirst == nullptr)
				return new Error("Unknown state %s", firstStateName);

			if (stateSecond == nullptr)
				return new Error("Unknown state %s", secondStateName);

			stateFirst->connections.push_back(Connection(stateSecond, allowedInputRange, true));
			continue;
		}

		if (strcmp(type, "a") == 0)
		{
			char stateName[128];

			char phrase[128];
			if (sscanf(lineStart, "%s %s", &stateName, &phrase) == 0)
				return new Error("Expected connection(%d)");

			auto phraseSize = strlen(phrase);
			State* lastState = TryFindState(stateName);
			for (int i = 0; i < phraseSize; i++)
			{
				auto state = new State();
				sprintf(state->name, "%s%d", phrase, i);
				sprintf(state->type, "default");
				states.push_back(state);

				lastState->connections.push_back(Connection(state, Range(phrase[i], phrase[i]), false));
				lastState = state;
			}

			sprintf(states.back()->type, "%s", phrase);
			continue;
		}
		
		return new Error("Unknown type %c", type);
	}

	startState = TryFindState("start");
	if (startState == nullptr)
		startState = states[0];

	return nullptr;
}

const char* StateMachine::IsInputAcceptable(char** input)
{
	currentState = startState;

	while (true)
	{
		// Check if it is the end state
		if (strcmp(currentState->type, "default") != 0)
			return currentState->type;

		// Iterate through all next connections
		bool isConnectionFound = false;
		for (auto& connection : currentState->connections)
		{
			isConnectionFound = false;
			if (connection.allowedInputRange.Contains(**input))
			{
				currentState = connection.target;
				if (connection.allowedInputRange.end != 256)
					(*input)++;
				isConnectionFound = true;
				break;
			}
		}

		if (!isConnectionFound)
			return "invalid";
	}
}

const char * StateMachine::IsInputAcceptableND(char ** input)
{
	std::queue<IsInputAcceptableContext> contexts;
	contexts.push(IsInputAcceptableContext(startState, *input));

	// Do undertemenistic state machine with BNF
	while (!contexts.empty())
	{
		auto context = contexts.front();
		auto state = context.state;
		contexts.pop();

		// Check if it is the end state
		if (strcmp(state->type, "default") != 0)
		{
			(*input) = context.input;
			return state->type;
		}

		// Try the child states
		for (auto& connection : state->connections)
		{
			if (connection.allowedInputRange.Contains(*context.input))
			{
				contexts.push(IsInputAcceptableContext(connection.target, 
					connection.assert ? context.input :context.input + 1));
			}
		}
	}

	return "invalid";
}

StateMachine::State* StateMachine::TryFindState(char * name)
{
	for (auto state : states)
	{
		if (strcmp(state->name, name) == 0)
			return state;
	}
	return nullptr;
}