#include "stdafx.h"
#include "Console.h"
#include <Rig3D/Graphics/DirectX11/imgui/imgui_internal.h>

Console::Console()
{
	ClearLog();
	memset(mInputBuf, 0, sizeof(mInputBuf));
	mHistoryPos = -1;
	mCommands.push_back("HELP");
	mCommands.push_back("HISTORY");
	mCommands.push_back("CLEAR");
	mCommands.push_back("CLASSIFY");  // "classify" is here to provide an example of "C"+[tab] completing to "CL" and displaying matches.
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
	if (Stricmp(command_line, "CLEAR") == 0)
	{
		ClearLog();
	}
	else if (Stricmp(command_line, "HELP") == 0)
	{
		AddLog("Commands:");
		for (int i = 0; i < mCommands.Size; i++)
			AddLog("- %s", mCommands[i]);
	}
	else if (Stricmp(command_line, "HISTORY") == 0)
	{
		for (int i = mHistory.Size >= 10 ? mHistory.Size - 10 : 0; i < mHistory.Size; i++)
			AddLog("%3d: %s\n", i, mHistory[i]);
	}
	else
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
			if (Strnicmp(mCommands[i], word_start, (int)(word_end - word_start)) == 0)
				candidates.push_back(mCommands[i]);

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

void Console::DrawConsole()
{
	auto input = Rig3D::Singleton<Rig3D::Engine>::SharedInstance().GetInput();

	if (input->GetKeyDown(Rig3D::KEYCODE_F2)) {
		Toggle();
	}

	if (!mVisible)
		return;

	ImGuiWindowFlags flags = 0
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::Begin("Console", nullptr, ImVec2(1600, 300), 0.80f, flags);
	
	static ImGuiTextFilter filter;
	filter.Draw("Filter (\"incl,-excl\") (\"error\")", 0);

	ImGui::Separator();

	// Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
	// NB- if you have thousands of entries this approach may be too inefficient. You can seek and display only the lines that are visible - CalcListClipping() is a helper to compute this information.
	// If your items are of variable size you may want to implement code similar to what CalcListClipping() does. Or split your data into fixed height items to allow random-seeking into your list.
	ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
	bool copy = false;
	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::Selectable("Clear")) ClearLog();
		copy = ImGui::Selectable("Copy");
		ImGui::EndPopup();
	}

	if (copy) ImGui::LogToClipboard();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
	for (int i = 0; i < mItems.Size; i++)
	{
		const char* item = mItems[i];
		if (!filter.PassFilter(item))
			continue;
		ImVec4 col = ImColor(255, 255, 255); // A better implementation may store a type per-item. For the sample let's just parse the text.
		if (strstr(item, "[Error]"))          col = ImColor(255, 100, 100);
		else if (strstr(item, "[Warning]"))   col = ImColor(255, 255, 100);
		else if (strstr(item, "[Log]"))	      col = ImColor(100, 255, 255);
		else if (strncmp(item, "$ ", 2) == 0) col = ImColor(255, 200, 150);
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		ImGui::TextUnformatted(item);
		ImGui::PopStyleColor();
	}
	if (mScrollToBottom)
		ImGui::SetScrollHere();
	mScrollToBottom = false;
	ImGui::PopStyleVar();
	ImGui::EndChild();
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

	ImGui::End();
	ImGui::PopStyleVar();
}

void Console::ClearLog()
{
	for (int i = 0; i < mItems.Size; i++)
		free(mItems[i]);
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
	mItems.push_back(strdup(buf));
	mScrollToBottom = true;
}

void Console::Clear()
{
	Rig3D::Singleton<Console>::SharedInstance().ClearLog();
}

void Console::Log(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	Rig3D::Singleton<Console>::SharedInstance().AddLog(fmt, args);
	va_end(args);
}

void Console::Draw()
{
	Rig3D::Singleton<Console>::SharedInstance().DrawConsole();
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