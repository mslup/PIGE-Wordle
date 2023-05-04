#pragma once

#include "framework.h"
#define ROUND info.roundness, info.roundness

class window;
class keyboard_window;
class puzzle_window;
class wordle;

class window
{
protected:
	static bool is_class_registered(HINSTANCE, LPCWSTR);
	static void register_class(HINSTANCE, LPCWSTR, LPCWSTR);
	HWND m_hWnd;
	WCHAR title[MAX_LOADSTRING];
	LPCWSTR s_class_name;

	HDC off_dc;
	HBITMAP off_oldbitmap, off_bitmap;

public:
	static int get_x_quarter(int quarter);
	static int get_y_quarter(int quarter);
	static int get_x_center() { return get_x_quarter(2); }
	static int get_y_center() { return get_y_quarter(2); }

	static struct gdi {
		static const HPEN   get_pen;
		static const HPEN   get_empty_pen;
		static const HBRUSH get_bckgrd_brush;
		static const HBRUSH get_default_brush;
		static const HBRUSH get_gray_brush;
		static const HBRUSH get_yellow_brush;
		static const HBRUSH get_green_brush;
		static const HBRUSH get_win_brush;
		static const HBRUSH get_lose_brush;
		 HFONT get_font;
	} _gdi;

	//static struct gdi {
	//	static HPEN get_pen() { return CreatePen(PS_INSIDEFRAME, 2, RGB(222, 225, 233)); }
	//	static HPEN get_empty_pen() { return CreatePen(PS_NULL, 0, 0); }
	//	static HBRUSH get_bckgrd_brush() { return CreatePen(PS_NULL, 0, 0); }
	//	static HBRUSH get_default_brush() { return CreateSolidBrush(RGB(251, 252, 255)); }
	//	static HBRUSH get_gray_brush() { return	CreateSolidBrush(RGB(164, 174, 196)); };
	//	static HBRUSH get_yellow_brush() { return CreateSolidBrush(RGB(243, 194, 55)); }
	//	static HBRUSH get_green_brush() { return CreateSolidBrush(RGB(121, 184, 81)); }
	//	static HBRUSH get_win_brush() { return CreateSolidBrush(RGB(0, 255, 0)); }
	//	static HBRUSH get_lose_brush() { return CreateSolidBrush(RGB(255, 0, 0)); }
	//	HFONT get_font(HDC hdc);
	//} _gdi;

	static LRESULT window_proc(HWND, UINT, WPARAM, LPARAM);
	virtual LRESULT window_proc(UINT, WPARAM, LPARAM);
	window();
	window(LPCWSTR class_name) : m_hWnd{ nullptr }, s_class_name{ class_name } { }
	window(const window&) = delete;
	window(window&& other) : m_hWnd{ nullptr } { *this = std::move(other); }
	window(HINSTANCE, const std::wstring&);
	window& operator=(const window&) = delete;
	window& operator=(window&& other)
	{
		std::swap(m_hWnd, other.m_hWnd);
		return *this;
	}
	operator HWND() const { return m_hWnd; }
	virtual ~window();
};

class keyboard_window : public window
{
	static void register_class(HINSTANCE);
	static LPCWSTR const s_class_name;
	static const DWORD style = WS_CAPTION | WS_SYSMENU;
	static void fill_properties();
	puzzle_window** m_puzzles;
	int running_puzzles_count;
	int word_limit;
	void update_difficulty(UINT);
	void draw_keys();
	void draw_one_key(RECT, int, HPEN, HPEN, HBRUSH, HBRUSH, HBRUSH, HBRUSH);
	void reset();

	static struct properties {
		static constexpr int
			opacity = 80,
			letter_size = 55,
			margin = 6,
			roundness = 5;

		int width = (margin + letter_size) * 10 + margin,
			height = (margin + letter_size) * 3 + margin;

		int row_offset[2];

		int x, y;
	} info;

	static constexpr WCHAR keys[] = {
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
		'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
		'Z', 'X', 'C', 'V', 'B', 'N', 'M' };
	static constexpr int key_row_lengths[] = { 10, 9, 7 };
	static constexpr int key_count = 10 + 9 + 7;

	class keyboard {
		/// 0 - no color
		/// 1 - gray (letter not present)
		/// 2 - yellow (letter at the wrong place)
		/// 3 - green (letter at the correct place)
		std::array<std::array<int, 26>, 4> key_colors;

	public:
		keyboard() : key_colors{} {}
		int& operator()(const int x, char c);
		int& operator()(const int id, const int key) { return key_colors[id][key]; }

		friend keyboard_window;
		friend puzzle_window;
	};
	keyboard kbrd;

public:
	LRESULT window_proc(UINT, WPARAM, LPARAM) override;
	UINT difficulty = ID_DIFFICULTY_EASY;
	keyboard_window(HINSTANCE);
	~keyboard_window();
	void load_children(puzzle_window* []);

	friend puzzle_window;
};

class puzzle_window : public window
{
	static void register_class(HINSTANCE);
	static const LPCWSTR s_class_name;
	static const DWORD style = WS_CAPTION;
	HINSTANCE m_instance;
	const int id;

	static void fill_properties();
	static struct properties {
		static constexpr int
			letter_size = 55,
			margin = 6,
			word_length = 5,
			word_count_easy = 6,
			word_count_medium = 8,
			word_count_hard = 10,
			roundness = 5,
			animation_events = 18,
			animation_tick = 10;

		int width = (margin + letter_size) * word_length + margin,
			height_easy = (margin + letter_size) * word_count_easy + margin,
			height_medium = (margin + letter_size) * word_count_medium + margin,
			height_hard = (margin + letter_size) * word_count_hard + margin;

		int x_center,
			y_center,
			x_left,
			y_top,
			x_right,
			y_bottom;

		static int adjust_rect(int);
	} info;
	void draw_tiles();
	void draw_overlay();
	void update_board(WCHAR);
	void animate_tile(int);
	void check_guess();
	void reset();
	int get_timer_id(int, int, int);

	keyboard_window* m_parent;

	std::string answer;
	bool win = false;
	bool lose = false;
	bool animating = false;
	bool pending_overlay = false;

	class board
	{
		std::array<std::array<WCHAR, info.word_length>, info.word_count_hard> letters;
		std::array<std::array<int, info.word_length>, info.word_count_hard> animation_time;

		/// 0 - no color
		/// 1 - gray (letter not present)
		/// 2 - yellow (letter at the wrong place)
		/// 3 - green (letter at the correct place)
		std::array<std::array<int, info.word_length>, info.word_count_hard> colors;

		int word_count = 0,
			letter_count = 0;

	public:
		board() : letters{}, colors{}, animation_time{} {};
		WCHAR& operator()(const int x, const int y) { return letters[x][y]; }

		friend puzzle_window;
	};
	board brd;

public:
	LRESULT window_proc(UINT, WPARAM, LPARAM) override;
	puzzle_window(HINSTANCE, keyboard_window*, int, std::string);

	friend keyboard_window;
};



