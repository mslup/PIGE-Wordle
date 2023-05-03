#pragma once

#include "framework.h"

class window;
class keyboard_window;
class puzzle_window;

class wordle
{
private:
	HINSTANCE m_instance;
	keyboard_window *m_keyboard;
	puzzle_window *m_puzzles[PUZZLEWINDOW_COUNT];
	static const std::string srcfile_path;
	static std::set<std::string> load_dictionary();

public:
	static const std::set<std::string> dictionary;
	static std::string get_random_entry();
	wordle(HINSTANCE instance);
	~wordle();

	int run(int show_command);
};