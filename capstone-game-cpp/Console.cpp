#include "stdafx.h"
#include "Console.h"
#include <Rig3D/Graphics/DirectX11/imgui/imgui_internal.h>

Console::Console()
{
	ClearLog();
	memset(mInputBuf, 0, sizeof(mInputBuf));
	mHistoryPos = -1;
	//mCommands.push_back("HELP");
	//mCommands.push_back("HISTORY");
	//mCommands.push_back("CLEAR");
	//mCommands.push_back("CLASSIFY");  // "classify" is here to provide an example of "C"+[tab] completing to "CL" and displaying matches.
}

Console::~Console()
{
	ClearLog();
	for (int i = 0; i < mItems.Size; i++)
		free(mHistory[i]);
}

int Console::Stricmp(const char* str1, const char* str2)
{ int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }

int Console::Strnicmp(const char* str1, const char* str2, int count)
{ int d = 0; while (count > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; count--; } return d; }

void Console::ExecCommand(const char* command_line)
{
	AddLog("$ %s\n", command_line);

	// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
	mHistoryPos = -1;
	for (int i = mHistory.Size - 1; i >= 0; i--)
		if (Stricmp(mHistory[i], command_line) == 0)
		{
			free(mHistory[i]);
			mHistory.erase(mHistory.begin() + i);
			break;
		}
	mHistory.push_back(strdup(command_line));

	// Process command
	bool found = false;
	for (auto command : mCommands)
	{
		if (Stricmp(command_line, command.name) == 0)
		{
			command.callback(0, nullptr);
			found = true;
			break;
		}
	}
	if (!found)
	{
		AddLog("Unknown command: '%s'\n", command_line);
	}
}

int Console::TextEditCallbackStub(ImGuiTextEditCallbackData* data)
{
	Console* console = (Console*)data->UserData;
	return console->TextEditCallback(data);
}

int Console::TextEditCallback(ImGuiTextEditCallbackData* data)
{
	//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
	switch (data->EventFlag)
	{
	case ImGuiInputTextFlags_CallbackCompletion:
	{
		// Example of TEXT COMPLETION

		// Locate beginning of current word
		const char* word_end = data->Buf + data->CursorPos;
		const char* word_start = word_end;
		while (word_start > data->Buf)
		{
			const char c = word_start[-1];
			if (c == ' ' || c == '\t' || c == ',' || c == ';')
				break;
			word_start--;
		}

		// Build a list of candidates
		ImVector<const char*> candidates;
		for (int i = 0; i < mCommands.Size; i++)
			if (Strnicmp(mCommands[i].name, word_start, (int)(word_end - word_start)) == 0)
				candidates.push_back(mCommands[i].name);

		if (candidates.Size == 0)
		{
			// No match
			AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
		}
		else if (candidates.Size == 1)
		{
			// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
			data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
			data->InsertChars(data->CursorPos, candidates[0]);
			data->InsertChars(data->CursorPos, " ");
		}
		else
		{
			// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
			int match_len = (int)(word_end - word_start);
			for (;;)
			{
				int c = 0;
				bool all_candidates_matches = true;
				for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
					if (i == 0)
						c = toupper(candidates[i][match_len]);
					else if (c != toupper(candidates[i][match_len]))
						all_candidates_matches = false;
				if (!all_candidates_matches)
					break;
				match_len++;
			}

			if (match_len > 0)
			{
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
			}

			// List matches
			AddLog("Possible matches:\n");
			for (int i = 0; i < candidates.Size; i++)
				AddLog("- %s\n", candidates[i]);
		}

		break;
	}
	case ImGuiInputTextFlags_CallbackHistory:
	{
		// Example of HISTORY
		const int prev_history_pos = mHistoryPos;
		if (data->EventKey == ImGuiKey_UpArrow)
		{
			if (mHistoryPos == -1)
				mHistoryPos = mHistory.Size - 1;
			else if (mHistoryPos > 0)
				mHistoryPos--;
		}
		else if (data->EventKey == ImGuiKey_DownArrow)
		{
			if (mHistoryPos != -1)
				if (++mHistoryPos >= mHistory.Size)
					mHistoryPos = -1;
		}

		// A better implementation would preserve the data on the current input line along with cursor position.
		if (prev_history_pos != mHistoryPos)
		{
			snprintf(data->Buf, data->BufSize, "%s", (mHistoryPos >= 0) ? mHistory[mHistoryPos] : "");
			data->BufDirty = true;
			data->CursorPos = data->SelectionStart = data->SelectionEnd = (int)strlen(data->Buf);
		}
	}
	}
	return 0;
}

void* Console::DrawConsole()
{
	auto input = Rig3D::Singleton<Rig3D::Engine>::SharedInstance().GetInput();

	if (input->GetKeyDown(Rig3D::KEYCODE_F2)) {
		Toggle();
	}

	ImGuiWindowFlags flags = 0
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::Begin("Console", nullptr, ImVec2(1600, 300), mVisible ? 0.80f : 0.0f, flags);
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	
	static ImGuiTextFilter filter;
	
	if (mVisible)
	{
		filter.Draw("Filter (\"incl,-excl\") (\"error\")", 0);
		ImGui::Separator();
	}

	// Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
	// NB- if you have thousands of entries this approach may be too inefficient. You can seek and display only the lines that are visible - CalcListClipping() is a helper to compute this information.
	// If your items are of variable size you may want to implement code similar to what CalcListClipping() does. Or split your data into fixed height items to allow random-seeking into your list.
	ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
	
	if (mVisible)
	{
		bool copy = false;
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Clear")) ClearLog();
			copy = ImGui::Selectable("Copy");
			ImGui::EndPopup();
		}

		if (copy) ImGui::LogToClipboard();
	}

	float timespan = mVisible ? -1.0f : 5.0f;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
	for (int i = 0; i < mItems.Size; i++)
	{
		const char* item = mItems[i].text;

		if (timespan > 0 && difftime(time(nullptr), mItems[i].timestamp) > timespan)
		{
			continue;
		}

		if (!filter.PassFilter(item))
			continue;
		ImVec4 col = ImColor(255, 255, 255); // A better implementation may store a type per-item. For the sample let's just parse the text.
		if (strstr(item, "[Error]"))          col = ImColor(255, 100, 100);
		else if (strstr(item, "[Warning]"))   col = ImColor(255, 255, 100);
		else if (strstr(item, "[Log]"))	      col = ImColor(100, 255, 255);
		else if (strstr(item, "[Invalid]"))	      col = ImColor(204, 204, 204); // invalid
		else if (strstr(item, "[Running]"))	      col = ImColor(000, 255, 204); // running
		else if (strstr(item, "[Success]"))	      col = ImColor(000, 255, 051); // success
		else if (strstr(item, "[Failure]"))	      col = ImColor(255, 000, 000); // fail
		else if (strstr(item, "[PASS]"))	      col = ImColor(000, 255, 051); // pass test
		else if (strstr(item, "[FAIL]"))	      col = ImColor(255, 000, 000); // fail test
		else if (strncmp(item, "$ ", 2) == 0) col = ImColor(255, 200, 150);
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		ImGui::TextUnformatted(item);
		ImGui::PopStyleColor();
	}
	if (!mVisible || mScrollToBottom)
		ImGui::SetScrollHere();
	mScrollToBottom = false;
	ImGui::PopStyleVar();

	ImGui::EndChild();
	
	
	if (mVisible)
	{
		ImGui::Separator();

		// Command-line
		auto inputFlags = 0
			| ImGuiInputTextFlags_EnterReturnsTrue
			| ImGuiInputTextFlags_CallbackCompletion
			| ImGuiInputTextFlags_CallbackHistory;

		if (ImGui::InputTextEx("", mInputBuf, IM_ARRAYSIZE(mInputBuf), ImVec2(-1, 0), inputFlags, &TextEditCallbackStub, static_cast<void*>(this)))
		{
			char* input_end = mInputBuf + strlen(mInputBuf);
			while (input_end > mInputBuf && input_end[-1] == ' ') input_end--; *input_end = 0;
			if (mInputBuf[0])
				ExecCommand(mInputBuf);
			strcpy(mInputBuf, "");
		}

		// Demonstrate keeping auto focus on the input box
		if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

	}

	ImGui::End();
	ImGui::PopStyleVar();

	return window;
}

void Console::ClearLog()
{
	for (int i = 0; i < mItems.Size; i++)
		free(mItems[i].text);
	mItems.clear();
	mScrollToBottom = true;
}

void Console::AddLog(const char* fmt, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
	buf[IM_ARRAYSIZE(buf) - 1] = 0;
	va_end(args);
	mItems.push_back({ strdup(buf), time(nullptr) });
	mScrollToBottom = true;
}

void Console::DisplayHelpText()
{
	AddLog("Available commands:");
	for (int i = 0; i < mCommands.Size; i++)
		AddLog("- %s", mCommands[i]);
	AddLog("\n");
	
	AddLog("Create new commands using the CONSOLE_COMMAND macro defined in \"Console.h\". e.g.:");
	AddLog("\n");
	AddLog("CONSOLE_COMMAND(my_awesome_command)");
	AddLog("{");
	AddLog("	Engine::PerformApocalypse();");
	AddLog("}");
}

void Console::Clear()
{
	Rig3D::Singleton<Console>::SharedInstance().ClearLog();
}

bool Console::IsVisible()
{
	return Rig3D::Singleton<Console>::SharedInstance().mVisible;
}

void Console::Help()
{
	Rig3D::Singleton<Console>::SharedInstance().DisplayHelpText();
}

void Console::RegisterCommand(Command command)
{
	mCommands.push_back(command);
}

void Console::Log(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	Rig3D::Singleton<Console>::SharedInstance().AddLog(fmt, args);
	va_end(args);
}

void* Console::Draw()
{
	return Rig3D::Singleton<Console>::SharedInstance().DrawConsole();
}

void Console::Show()
{
	Rig3D::Singleton<Console>::SharedInstance().mVisible = true;
}

void Console::Hide()
{
	Rig3D::Singleton<Console>::SharedInstance().mVisible = false;
}

void Console::Toggle()
{
	Console& c = Rig3D::Singleton<Console>::SharedInstance();
	c.mVisible = !c.mVisible;
}

CONSOLE_COMMAND(clear)
{
	Console::Clear();
}

CONSOLE_COMMAND(help)
{
	Console::Help();
}
