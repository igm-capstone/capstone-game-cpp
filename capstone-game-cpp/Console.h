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

	struct Command
	{
		const char* name;
		void(*callback)(int argc, char* argv[]);
	};

public:
	class AutoCommandRegister
	{
	public:
		AutoCommandRegister(const char* name, void(*callback)(int argc, char* argv[]))
		{
			Rig3D::Singleton<Console>::SharedInstance().RegisterCommand({ name, callback });
		}
	};

private:
	char                  mInputBuf[256];
	ImVector<ConsoleItem> mItems;
	bool                  mScrollToBottom;
	ImVector<char*>       mHistory;
	int                   mHistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
	ImVector<Command>     mCommands;
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

	void* DrawConsole();
	void AddLog(const char* fmt, ...) IM_PRINTFARGS(2);
	void ClearLog();
	void DisplayHelpText();

public:
	static void Show();
	static void Hide();
	static void Toggle();
	static void* Draw();
	static void Log(const char* fmt, ...) IM_PRINTFARGS(2);
	static void Clear();
	static bool IsVisible();
	static void Help();
	
	void RegisterCommand(Command command);
};

#define CONSOLE_COMMAND(command_name)                             \
    void __console_command__##command_name(int argc, char* argv[]);                            \
    static Console::AutoCommandRegister __auto_command_register_##command_name(#command_name, __console_command__##command_name); \
    void __console_command__##command_name(int argc, char* argv[])

