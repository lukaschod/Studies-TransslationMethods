#include "StateMachine.h"
#include <stdio.h>

StateMachine::StateMachine() :
	currentState(nullptr),
	startState(nullptr)
{
	
}

StateMachine::~StateMachine()
{
}

Error* StateMachine::LoadFromFile(const char* pathToFile)
{
	auto file = fopen(pathToFile, "rb");
	if (file == nullptr)
		return new Error("File in path %s was not found", pathToFile);

	while (true)
	{
		char type[128];
		if (fscanf(file, "%s", &type) <= 0)
			break;

		if (strcmp(type, "s") == 0)
		{
			auto state = new State();

			if (fscanf(file, "%s", &state->name) == 0)
				return new Error("Expected state name");

			if (fscanf(file, "%u", &state->flag) == 0)
				return new Error("Expected state flags");
			states.push_back(state);

			// Check if only one starting state is
			if (startState != nullptr && state->flag == kStateMachineExitFlagStart)
				return new Error("Only one starting state is allowed");

			// Mark this state as starting one
			if (startState == nullptr && state->flag == kStateMachineExitFlagStart)
				startState = state;
			continue;
		}

		if (strcmp(type, "c") == 0)
		{
			char firstStateName[128];
			char secondStateName[128];
			Range allowedInputRange;
			if (fscanf(file, "%s %s %d-%d", &firstStateName, &secondStateName,
				&allowedInputRange.start, &allowedInputRange.end) == 0)
				return new Error("Expected connection(%d %d %d-%d)");

			auto stateFirst = TryFindState(firstStateName);
			auto stateSecond = TryFindState(secondStateName);

			if (stateFirst == nullptr)
				return new Error("Unknown state %s", firstStateName);

			if (stateSecond == nullptr)
				return new Error("Unknown state %s", secondStateName);

			stateFirst->connections.push_back(Connection(stateSecond, allowedInputRange));
			continue;
		}
		
		return new Error("Unknown type %c", type);
	}

	/*uint32_t statesCount;
	if (fscanf(file, "%u", &statesCount) == 0)
		return new Error("Expected statesCount(%u)");

	for (uint32_t i = 0; i < statesCount; i++)
	{
		auto state = new State();
		if (fscanf(file, "%u", &state->flag) == 0)
			return new Error("Expected state flags");
		states.push_back(state);

		// Check if only one starting state is
		if (startState != nullptr && state->flag == kStateMachineExitFlagStart)
			return new Error("Only one starting state is allowed");

		// Mark this state as starting one
		if (startState == nullptr && state->flag == kStateMachineExitFlagStart)
			startState = state;
	}

	uint32_t connectionsCount;
	if (fscanf(file, "%u", &connectionsCount) == 0)
		return new Error("Expected connectionsCount(%u)");

	for (uint32_t i = 0; i < connectionsCount; i++)
	{
		uint32_t stateIdFirst;
		uint32_t stateIdSecond;
		Range allowedInputRange;
		if (fscanf(file, "%d %d %d-%d", &stateIdFirst, &stateIdSecond, 
			&allowedInputRange.start, &allowedInputRange.end) == 0)
			return new Error("Expected connection(%d %d %d-%d)");

		if (stateIdFirst >= states.size() || stateIdSecond >= states.size())
			return new Error("Connection referencing unknown states");

		auto stateFirst = states[stateIdFirst];
		auto stateSecond = states[stateIdSecond];
		stateFirst->connections.push_back(Connection(stateSecond, allowedInputRange));
	}*/

	if (startState == nullptr)
		return new Error("One state must contain starting flag");

	fclose(file);

	return nullptr;
}

StateMachineExitFlag StateMachine::IsInputAcceptable(char** input)
{
	currentState = startState;

	while (true)
	{
		// Check if it is the end state
		if (currentState->flag != kStateMachineExitFlagNone && 
			currentState->flag != kStateMachineExitFlagStart)
			return currentState->flag;

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
			return kStateMachineExitFlagNone;
	}
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
