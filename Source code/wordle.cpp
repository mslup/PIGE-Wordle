#include "wordle.h"

wordle::wordle(HINSTANCE hInst) : m_instance{ hInst }
{
	m_keyboard = new keyboard_window(hInst);
	for (int i = 0; i < PUZZLEWINDOW_COUNT; i++)
		m_puzzles[i] = new puzzle_window(hInst, m_keyboard, i, get_random_entry());

	m_keyboard->load_children(m_puzzles);
}

wordle::~wordle()
{
	for (int i = 0; i < PUZZLEWINDOW_COUNT; i++)
		delete m_puzzles[i];
}

int wordle::run(int show_command)
{
	ShowWindow(*m_keyboard, show_command);
	
	MSG msg{};
	BOOL result = TRUE;

	while ((result = GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (result == -1)
			return EXIT_FAILURE;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	DeleteObject(window::gdi::get_pen);
	DeleteObject(window::gdi::get_empty_pen);
	DeleteObject(window::gdi::get_bckgrd_brush);
	DeleteObject(window::gdi::get_default_brush);
	DeleteObject(window::gdi::get_gray_brush);
	DeleteObject(window::gdi::get_yellow_brush);
	DeleteObject(window::gdi::get_green_brush);
	DeleteObject(window::gdi::get_win_brush);
	DeleteObject(window::gdi::get_lose_brush);

	return EXIT_SUCCESS;
}

const std::string wordle::srcfile_path = R"(Resources\Wordle.txt)";
const std::set<std::string> wordle::dictionary = wordle::load_dictionary();

std::set<std::string> wordle::load_dictionary()
{
	std::ifstream input(srcfile_path);
	std::set<std::string> ret_set;
	std::string buf;

	while (getline(input, buf))
	{
		for (char& c : buf) c = toupper(c);
		ret_set.insert(buf);
	}

	input.close();
	return ret_set;
}

std::string wordle::get_random_entry()
{
	int index = rand() % dictionary.size();
	auto it = dictionary.cbegin();
	std::advance(it, index);
	return *it;
}