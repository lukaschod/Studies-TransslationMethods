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
		//__debugbreak();
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
	Range(uint32_t start, uint32_t end) : start(start), end(end) {}

	inline bool Contains(uint32_t value) { return start <= value && value <= end; }

public:
	uint32_t start;
	uint32_t end;
};

class FileToBuffer
{
public:
	FileToBuffer(size_t capacity) :
		capacity(capacity)
	{
		buffer = new char[capacity];
		size = 0;
	}

	~FileToBuffer()
	{
		delete buffer;
	}

	Error* ReadFile(const char* pathToFile)
	{
		auto file = fopen(pathToFile, "rb");
		if (file == nullptr)
			return new Error("File in path %s was not found", pathToFile);

		size = fread(buffer, sizeof(char), capacity, file);
		if (size == 0)
			return new Error("Failed to read file");

		fclose(file);

		return nullptr;
	}

	Error* WriteFile(const char* pathToFile)
	{
		auto file = fopen(pathToFile, "wb");
		if (file == nullptr)
			return new Error("File in path %s was not found", pathToFile);

		size = fwrite(buffer, sizeof(char), size, file);
		if (size == 0)
			return new Error("Failed to read file");

		fclose(file);

		return nullptr;
	}

public:
	char* buffer;
	size_t capacity;
	size_t size;
};

class BufferReader
{
public:
	BufferReader(char* begin, char* end) :
		begin(begin),
		end(end),
		pointer(begin)
	{
	}

	bool MovePointerToFirst(char accurance)
	{
		auto current = pointer;
		while (current != end)
		{
			if (*current == accurance)
			{
				pointer = current + sizeof(char);
				return true;
			}
			current++;
		}
		return false;
	}

	bool MovePointerToFirst(char accurance, char accurance2)
	{
		auto current = pointer;
		while (current != end)
		{
			if (*current == accurance || *current == accurance2)
			{
				pointer = current + sizeof(char);
				return true;
			}
			current++;
		}
		return false;
	}

	bool MovePointerToFirst(const char* accurance)
	{
		auto accuranceSize = strlen(accurance);
		auto current = pointer;
		while (current != end)
		{
			if (memcmp(accurance, (const char*) current, accuranceSize) == 0)
			{
				pointer = current + accuranceSize;
				return true;
			}
			current++;
		}
		return false;
	}

	bool MovePointerToFirstExpression(char* outExpression)
	{
		auto cached = pointer;
		if (!MovePointerToFirst('<', 39))
			return false;

		auto expressionStart = pointer;

		if (!MovePointerToFirst('>', 39))
		{
			cached = pointer;
			return false;
		}

		auto size = pointer - expressionStart - sizeof(char);
		memcpy(outExpression, expressionStart, size);
		outExpression[size] = 0;
		return true;
	}

public:
	char* begin;
	char* pointer;
	char* end;
};

class BufferWriter
{
public:
	BufferWriter(char* begin, char* end) :
		begin(begin),
		end(end),
		pointer(begin)
	{
	}

	void WriteFormated(const char* format, ...)
	{
		va_list ap;
		va_start(ap, format);
		pointer += vsnprintf(pointer, end - begin, format, ap);
	}

public:
	char* begin;
	char* pointer;
	char* end;
};