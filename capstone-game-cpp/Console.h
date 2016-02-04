#pragma once
#include <Rig3D/Graphics/DirectX11/imgui/imgui.h>
#include <time.h>

#ifdef _MSC_VER
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#define snprintf _snprintf
#endif

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

// For the console example, here we are using a more C++ like approach of declaring a class to hold the data and the functions.
class Console
{
	struct ConsoleItem
	{
		char* text;
		time_t timestamp;
	};

	char                  mInputBuf[256];
	ImVector<ConsoleItem> mItems;
	bool                  mScrollToBottom;
	ImVector<char*>       mHistory;
	int                   mHistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
	ImVector<const char*> mCommands;
	bool                  mVisible;

protected:
	Console();
	~Console();

	static int Stricmp(const char* str1, const char* str2);
	static int Strnicmp(const char* str1, const char* str2, int count);
	void ExecCommand(const char* command_line);
	
	// In C++11 you are better off using lambdas for this sort of forwarding callbacks
	static int TextEditCallbackStub(ImGuiTextEditCallbackData* data);
	int TextEditCallback(ImGuiTextEditCallbackData* data);

	void DrawConsole();
	void DrawConsoleItems(ImGuiTextFilter& filter, double timespan);
	void AddLog(const char* fmt, ...) IM_PRINTFARGS(2);
	void ClearLog();

public:
	static void Show();
	static void Hide();
	static void Toggle();
	static void Draw();
	static void Log(const char* fmt, ...) IM_PRINTFARGS(2);
	static void Clear();
};
