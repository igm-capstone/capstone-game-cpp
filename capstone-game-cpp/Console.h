#pragma once
#include <Rig3D/Graphics/DirectX11/imgui/imgui.h>

#ifdef _MSC_VER
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#define snprintf _snprintf
#endif

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

// For the console example, here we are using a more C++ like approach of declaring a class to hold the data and the functions.
class Console
{
	char                  mInputBuf[256];
	ImVector<char*>       mItems;
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

public:
	void Show();
	void Hide();
	void Toggle();
	void AddLog(const char* fmt, ...) IM_PRINTFARGS(2);
	void ClearLog();
	void Draw();
};
