#include "window.h"

// =====================================================
#pragma region BASE_WINDOW

int window::get_x_quarter(int quarter)
{
	if (quarter < 1 || quarter > 4)
		return 0;

	RECT rc;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
	return (rc.left + rc.right) * quarter / 4;
}

int window::get_y_quarter(int quarter)
{
	if (quarter < 1 || quarter > 4)
		return 0;

	RECT rc;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
	return (rc.top + rc.bottom) * quarter / 4;
}

const HPEN   window::gdi::get_pen{ CreatePen(PS_INSIDEFRAME, 2, RGB(222, 225, 233)) };
const HPEN   window::gdi::get_empty_pen{ CreatePen(PS_NULL, 0, 0) };
const HBRUSH window::gdi::get_bckgrd_brush{ CreateSolidBrush(RGB(255, 255, 255)) };
const HBRUSH window::gdi::get_default_brush{ CreateSolidBrush(RGB(251, 252, 255)) };
const HBRUSH window::gdi::get_gray_brush{ CreateSolidBrush(RGB(164, 174, 196)) };
const HBRUSH window::gdi::get_yellow_brush{ CreateSolidBrush(RGB(243, 194, 55)) };
const HBRUSH window::gdi::get_green_brush{ CreateSolidBrush(RGB(121, 184, 81)) };
const HBRUSH window::gdi::get_win_brush{ CreateSolidBrush(RGB(0, 255, 0)) };
const HBRUSH window::gdi::get_lose_brush{ CreateSolidBrush(RGB(255, 0, 0)) };
struct window::gdi window::_gdi;

window::window() : m_hWnd{ nullptr }, s_class_name{ nullptr }, title{ L"" }
{
	
}

bool window::is_class_registered(HINSTANCE hInst, LPCWSTR cName)
{
	WNDCLASSEXW wcx;
	return GetClassInfoExW(hInst, cName, &wcx);
}

void window::register_class(HINSTANCE hInst, LPCWSTR cName, LPCWSTR menuName)
{
	WNDCLASSEXW wcx{};

	wcx.cbSize = sizeof(wcx);

	wcx.style = CS_VREDRAW | CS_HREDRAW;
	wcx.lpfnWndProc = window_proc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hInst;
	wcx.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LAB2));
	wcx.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wcx.hbrBackground = nullptr;
	wcx.lpszMenuName = menuName;
	wcx.lpszClassName = cName;
	wcx.hIconSm = LoadIcon(wcx.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcx);
}

LRESULT window::window_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	window* w = nullptr;
	if (msg == WM_NCCREATE) {
		auto pcs = reinterpret_cast<LPCREATESTRUCTW>(lParam);
		w = reinterpret_cast<window*>(pcs->lpCreateParams);
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(w));
		w->m_hWnd = hWnd;

		HDC hdc = GetDC(hWnd);
		_gdi.get_font = CreateFontW(
			-MulDiv(16, GetDeviceCaps(hdc, LOGPIXELSY), 72),
			0, 0, 0, FW_BOLD, FALSE, FALSE, 0, EASTEUROPE_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_SWISS, L"Arial");
		ReleaseDC(hWnd, hdc);

	}
	else w = reinterpret_cast<window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
	if (w) {
		auto r = w->window_proc(msg, wParam, lParam);
		if (msg == WM_NCDESTROY) {
			w->m_hWnd = nullptr;
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
		}
		return r;
	}
	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

LRESULT window::window_proc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(m_hWnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(EXIT_SUCCESS);
		return 0;
	}
	return DefWindowProcW(m_hWnd, msg, wParam, lParam);
}

//HFONT window::gdi::get_font(HDC hdc)
//{
//	return CreateFontW(
//		-MulDiv(16, GetDeviceCaps(hdc, LOGPIXELSY), 72),
//		0, 0, 0, FW_BOLD, FALSE, FALSE, 0, EASTEUROPE_CHARSET,
//		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
//		DEFAULT_PITCH | FF_SWISS, L"Arial");
//}

window::~window()
{
	DeleteObject(_gdi.get_font);
	if (m_hWnd)
		DestroyWindow(m_hWnd);
}

#pragma endregion


// =====================================================
#pragma region KEYBOARD_WINDOW

LPCWSTR const keyboard_window::s_class_name{ L"keyboard_window" };
struct keyboard_window::properties keyboard_window::info;

void keyboard_window::register_class(HINSTANCE hInst)
{
	fill_properties();
	window::register_class(hInst, s_class_name, MAKEINTRESOURCE(IDC_LAB2));
}

void keyboard_window::fill_properties()
{
	RECT rc;
	rc.left = rc.top = 0;
	rc.right = info.width;
	rc.bottom = info.height;
	AdjustWindowRect(&rc, style, true);

	info.width = rc.right - rc.left;
	info.height = rc.bottom - rc.top;

	for (int i = 0; i < 2; i++)
		info.row_offset[i] =
		((key_row_lengths[0] - key_row_lengths[i + 1])
			* (info.letter_size + info.margin)
			+ info.margin) / 2;

	info.x = get_x_center() - info.width / 2;
	info.y = get_y_center() + 200;
}

void keyboard_window::update_difficulty(UINT wmId)
{
	CheckMenuItem(GetMenu(m_hWnd), difficulty, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(GetMenu(m_hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
	difficulty = wmId;

	switch (wmId)
	{
	case ID_DIFFICULTY_EASY:
		running_puzzles_count = 1;
		word_limit = puzzle_window::info.word_count_easy;
		ShowWindow(*m_puzzles[1], SW_HIDE);
		ShowWindow(*m_puzzles[2], SW_HIDE);
		ShowWindow(*m_puzzles[3], SW_HIDE);

		MoveWindow(*m_puzzles[0],
			puzzle_window::info.x_center, puzzle_window::info.y_center,
			puzzle_window::info.width, puzzle_window::info.height_easy, true);

		break;

	case ID_DIFFICULTY_MEDIUM:
		running_puzzles_count = 2;
		word_limit = puzzle_window::info.word_count_medium;
		ShowWindow(*m_puzzles[1], SW_SHOWNA);
		ShowWindow(*m_puzzles[2], SW_HIDE);
		ShowWindow(*m_puzzles[3], SW_HIDE);

		MoveWindow(*m_puzzles[0],
			puzzle_window::info.x_left, puzzle_window::info.y_center,
			puzzle_window::info.width, puzzle_window::info.height_medium, true);
		MoveWindow(*m_puzzles[1],
			puzzle_window::info.x_right, puzzle_window::info.y_center,
			puzzle_window::info.width, puzzle_window::info.height_medium, true);
		break;

	case ID_DIFFICULTY_HARD:
		running_puzzles_count = 4;
		word_limit = puzzle_window::info.word_count_hard;
		for (int i = 0; i < 4; i++)
			ShowWindow(*m_puzzles[i], SW_SHOWNA);

		MoveWindow(*m_puzzles[0],
			puzzle_window::info.x_left, puzzle_window::info.y_top,
			puzzle_window::info.width, puzzle_window::info.height_hard, true);
		MoveWindow(*m_puzzles[1],
			puzzle_window::info.x_left, puzzle_window::info.y_bottom,
			puzzle_window::info.width, puzzle_window::info.height_hard, true);
		MoveWindow(*m_puzzles[2],
			puzzle_window::info.x_right, puzzle_window::info.y_top,
			puzzle_window::info.width, puzzle_window::info.height_hard, true);
		MoveWindow(*m_puzzles[3],
			puzzle_window::info.x_right, puzzle_window::info.y_bottom,
			puzzle_window::info.width, puzzle_window::info.height_hard, true);
		break;
	}

	for (int i = 0; i < 4; i++)
		m_puzzles[i]->reset();
	reset();
}

void keyboard_window::draw_keys()
{
	HPEN pen = gdi::get_pen;
	HPEN empty_pen = gdi::get_empty_pen;
	HPEN old_pen = (HPEN)SelectObject(off_dc, pen);

	HBRUSH bckgrd_brush = gdi::get_bckgrd_brush;
	HBRUSH default_brush = gdi::get_default_brush;
	HBRUSH gray_brush = gdi::get_gray_brush;
	HBRUSH yellow_brush = gdi::get_yellow_brush;
	HBRUSH green_brush = gdi::get_green_brush;
	HBRUSH old_brush = (HBRUSH)SelectObject(off_dc, default_brush);

	HFONT font = _gdi.get_font;
	HFONT old_font = (HFONT)SelectObject(off_dc, font);

	RECT rc;
	GetClientRect(m_hWnd, &rc);
	Rectangle(off_dc, rc.left - 1, rc.top - 1, rc.right + 1, rc.bottom + 1);

	rc.left = info.margin;
	rc.top = info.margin;
	rc.right = info.margin + info.letter_size;
	rc.bottom = info.margin + info.letter_size;

	int current_key = 0;
	SelectObject(off_dc, default_brush);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < key_row_lengths[i]; j++)
		{
			draw_one_key(rc, current_key, pen, empty_pen,
				default_brush, gray_brush, yellow_brush, green_brush);

			current_key++;
			rc.left += info.letter_size + info.margin;
			rc.right += info.letter_size + info.margin;
		}

		rc.left = info.row_offset[i];
		rc.right = rc.left + info.letter_size;

		rc.top += info.letter_size + info.margin;
		rc.bottom += info.letter_size + info.margin;
	}

	SelectObject(off_dc, old_pen);
	SelectObject(off_dc, old_brush);
	SelectObject(off_dc, old_font);
}

void keyboard_window::draw_one_key(RECT rc, int current_key, HPEN pen, HPEN empty_pen,
	HBRUSH default_brush, HBRUSH gray_brush, HBRUSH yellow_brush, HBRUSH green_brush)
{
	RoundRect(off_dc, rc.left, rc.top, rc.right, rc.bottom, 3, 3);

	int tiles = running_puzzles_count;

	RECT old_rc = rc;
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	if (tiles >= 2)
	{
		rc.right -= width / 2;
		if (tiles == 4)
			rc.bottom -= height / 2;
	}

	int color;
	for (int i = 0; i < tiles; i++)
	{
		color = kbrd(i, current_key);
		SelectObject(off_dc, empty_pen);
		switch (color)
		{
		case 1:
			SelectObject(off_dc, gray_brush);
			break;
		case 2:
			SelectObject(off_dc, yellow_brush);
			break;
		case 3:
			SelectObject(off_dc, green_brush);
			break;
		}

		if (color != 0)
			RoundRect(off_dc, rc.left, rc.top, rc.right, rc.bottom, ROUND);

		if (tiles == 2)
		{
			rc.left = rc.right - 1;
			rc.right += width / 2;
		}
		else if (tiles == 4)
		{
			if (i % 2 == 0)
			{
				rc.top = rc.bottom - 1;
				rc.bottom += height / 2;
				continue;
			}
			rc.bottom = rc.top + 1;
			rc.top -= height / 2;
			rc.left = rc.right - 1;
			rc.right += width / 2;
		}

	}

	SetBkMode(off_dc, TRANSPARENT);
	DrawText(off_dc, &keys[current_key], 1, &old_rc,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	SelectObject(off_dc, pen);
	SelectObject(off_dc, default_brush);
}

int& keyboard_window::keyboard::operator()(const int x, char c)
{
	int y = 0;
	while (y <= key_count && keys[y] != c)
		y++;
	return key_colors[x][y];
}

LRESULT keyboard_window::window_proc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		HDC hdc = GetDC(m_hWnd);
		off_dc = CreateCompatibleDC(hdc);
		ReleaseDC(m_hWnd, hdc);
	}
	break;
	case WM_SIZE:
	{
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);

		HDC hdc = GetDC(m_hWnd);

		if (off_oldbitmap != NULL)
			SelectObject(off_dc, off_oldbitmap);
		if (off_bitmap != NULL)
			DeleteObject(off_bitmap);

		off_bitmap = CreateCompatibleBitmap(hdc, width, height);
		off_oldbitmap = (HBITMAP)SelectObject(off_dc, off_bitmap);
		ReleaseDC(m_hWnd, hdc);
	}
	break;
	
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);

		switch (wmId)
		{
		case ID_DIFFICULTY_EASY:
		case ID_DIFFICULTY_MEDIUM:
		case ID_DIFFICULTY_HARD:
			update_difficulty(wmId);
			break;

		case IDM_EXIT:
			DestroyWindow(m_hWnd);
			break;

		default:
			return window::window_proc(msg, wParam, lParam);
		}
	}
	break;
	case WM_CHAR:
	{
		WCHAR c = (WCHAR)wParam;
		if (iswalpha(c) && isascii(c) || c == VK_BACK || c == VK_RETURN)
		{
			c = toupper(c);
			for (int i = 0; i < running_puzzles_count; i++)
				if (m_puzzles[i]->win == false)
					m_puzzles[i]->update_board(c);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(*this, &ps);
		draw_keys();

		RECT rc;
		GetClientRect(m_hWnd, &rc);
		BitBlt(hdc, 0, 0, rc.right, rc.bottom, off_dc, 0, 0, SRCCOPY);

		EndPaint(*this, &ps);
	}
	break;
	case WM_ERASEBKGND:
		return 1;
	case WM_CLOSE:
	case WM_DESTROY:
	{
		if (off_oldbitmap != NULL)
			SelectObject(off_dc, off_oldbitmap);
		if (off_dc != NULL)
			DeleteDC(off_dc);
		if (off_bitmap != NULL)
			DeleteObject(off_bitmap);
		PostQuitMessage(0);

		const CHAR* str;
		switch (difficulty)
		{
		case ID_DIFFICULTY_MEDIUM:
			str = "medium\0";
			break;
		case ID_DIFFICULTY_HARD:
			str = "hard\0";
			break;
		case ID_DIFFICULTY_EASY:
		default:
			str = "easy\0";
			break;
		}
		WritePrivateProfileStringA("WORDLE", "DIFFICULTY", str, "Resources\\Wordle.ini");
	}
	break;
	default:
		return window::window_proc(msg, wParam, lParam);
	}
	return 0;
}

keyboard_window::keyboard_window(HINSTANCE hInst)
	: window(s_class_name), m_puzzles{ nullptr }
{
	if (!is_class_registered(hInst, s_class_name))
		register_class(hInst);

	CHAR difficulty_setting[MAX_LOADSTRING];
	GetPrivateProfileStringA("WORDLE", "DIFFICULTY",
		"easy\0", (LPSTR)difficulty_setting, MAX_LOADSTRING, "Resources\\Wordle.ini");

	if (strcmp(difficulty_setting, "easy\0") == 0)
		difficulty = ID_DIFFICULTY_EASY;
	if (strcmp(difficulty_setting, "medium\0") == 0)
		difficulty = ID_DIFFICULTY_MEDIUM;
	if (strcmp(difficulty_setting, "hard\0") == 0)
		difficulty = ID_DIFFICULTY_HARD;

	LoadStringW(hInst, IDS_KEYBOARD_TITLE, title, MAX_LOADSTRING);
	m_hWnd = CreateWindowW(s_class_name, title,
		style, info.x, info.y, info.width, info.height,
		nullptr, nullptr, hInst, reinterpret_cast<LPVOID>(this));

	SetWindowLong(m_hWnd, GWL_EXSTYLE,
		GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(m_hWnd, 0, 255 * info.opacity / 100, LWA_ALPHA);
}

keyboard_window::~keyboard_window()
{
	delete m_puzzles;
}

void keyboard_window::load_children(puzzle_window* puzzles[])
{
	m_puzzles = new puzzle_window * [PUZZLEWINDOW_COUNT];
	std::memcpy(m_puzzles, puzzles, PUZZLEWINDOW_COUNT * sizeof(puzzle_window*));
	update_difficulty(difficulty);
}

void keyboard_window::reset()
{
	for (auto& row : kbrd.key_colors)
		row.fill(0);
	InvalidateRect(m_hWnd, NULL, FALSE);
	return;
}

#pragma endregion


// =====================================================
#pragma region PUZZLE_WINDOW 

LPCWSTR const puzzle_window::s_class_name{ L"puzzle_window" };
struct puzzle_window::properties puzzle_window::info;

int puzzle_window::properties::adjust_rect(int height)
{
	RECT rc;
	rc.left = rc.top = 0;
	rc.right = info.width;
	rc.bottom = height;
	AdjustWindowRect(&rc, style, false);
	if (height == 0)
		return rc.right - rc.left;
	return rc.bottom - rc.top;
}

void puzzle_window::fill_properties()
{
	info.height_easy = info.adjust_rect(info.height_easy);
	info.height_medium = info.adjust_rect(info.height_medium);
	info.height_hard = info.adjust_rect(info.height_hard);
	info.width = info.adjust_rect(0);

	info.x_center = get_x_center() - info.width / 2;
	info.y_center = get_y_center() - info.height_medium / 2;

	info.x_left = get_x_quarter(1) - info.width / 2;
	info.y_top = get_y_quarter(1) - info.height_hard / 2;
	info.x_right = get_x_quarter(3) - info.width / 2;
	info.y_bottom = get_y_quarter(3) - info.height_hard / 2;
}

void puzzle_window::register_class(HINSTANCE hInst)
{
	fill_properties();
	window::register_class(hInst, s_class_name, nullptr);
}

void puzzle_window::draw_tiles()
{
	HPEN pen = gdi::get_pen;
	HPEN old_pen = (HPEN)SelectObject(off_dc, pen);

	HBRUSH bckgrd_brush = gdi::get_bckgrd_brush;
	HBRUSH default_brush = gdi::get_default_brush;
	HBRUSH gray_brush = gdi::get_gray_brush;
	HBRUSH yellow_brush = gdi::get_yellow_brush;
	HBRUSH green_brush = gdi::get_green_brush;
	HBRUSH old_brush = (HBRUSH)SelectObject(off_dc, bckgrd_brush);

	HFONT font = _gdi.get_font;
	HFONT old_font = (HFONT)SelectObject(off_dc, font);

	RECT rc;
	GetClientRect(m_hWnd, &rc);
	Rectangle(off_dc, rc.left, rc.top, rc.right, rc.bottom);

	rc.left = info.margin;
	rc.top = info.margin;
	rc.right = info.margin + info.letter_size;
	rc.bottom = info.margin + info.letter_size;

	WCHAR c;
	SelectObject(off_dc, default_brush);
	for (int i = 0; i < info.word_count_hard; i++)
	{
		for (int j = 0; j < info.word_length; j++)
		{
			c = brd(i, j);

			switch (brd.colors[i][j])
			{
			case 1:
				SelectObject(off_dc, gray_brush);
				break;
			case 2:
				SelectObject(off_dc, yellow_brush);
				break;
			case 3:
				SelectObject(off_dc, green_brush);
				break;
			case 0:
			default:
				SelectObject(off_dc, default_brush);
			}

			RoundRect(off_dc, rc.left, rc.top, rc.right, rc.bottom, ROUND);

			SetBkMode(off_dc, TRANSPARENT);
			DrawText(off_dc, &c, 1, &rc,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			rc.left += info.letter_size + info.margin;
			rc.right += info.letter_size + info.margin;
		}
		rc.left = info.margin;
		rc.right = info.margin + info.letter_size;

		rc.top += info.letter_size + info.margin;
		rc.bottom += info.letter_size + info.margin;
	}

	SelectObject(off_dc, old_pen);
	SelectObject(off_dc, old_brush);
	SelectObject(off_dc, old_font);
}

// source: https://learn.microsoft.com/en-us/windows/win32/gdi/alpha-blending-a-bitmap
void puzzle_window::draw_overlay()
{
	if (!win && !lose)
		return;
	if (animating)
	{
		pending_overlay = true;
		return;
	}

	pending_overlay = false;

	RECT rc;
	GetClientRect(m_hWnd, &rc);

	HDC tmp_dc = CreateCompatibleDC(off_dc);
	HBITMAP tmp_bitmap = CreateCompatibleBitmap(off_dc,
		rc.right - rc.left, rc.bottom - rc.top);
	SelectObject(tmp_dc, tmp_bitmap);

	HBRUSH brush = win ? gdi::get_win_brush : gdi::get_lose_brush;
	HBRUSH old_brush = (HBRUSH)SelectObject(tmp_dc, brush);

	Rectangle(tmp_dc, rc.left - 1, rc.top - 1, rc.right + 1, rc.bottom + 1);

	BLENDFUNCTION bf = {
		.BlendOp = AC_SRC_OVER,
		.BlendFlags = 0,
		.SourceConstantAlpha = 0x7f,
		.AlphaFormat = 0
	};

	AlphaBlend(off_dc, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
		tmp_dc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, bf);
	SelectObject(tmp_dc, old_brush);
	DeleteObject(tmp_bitmap);
	DeleteDC(tmp_dc);

	if (!win)
	{
		HFONT font = _gdi.get_font;
		HFONT old_font = (HFONT)SelectObject(off_dc, font);
		COLORREF old_color = SetTextColor(off_dc, RGB(255, 255, 255));

		std::wstring text(answer.begin(), answer.end());
		DrawText(off_dc, text.c_str(), 5, &rc,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		SetTextColor(off_dc, old_color);
		SelectObject(off_dc, old_font);
	}
	HDC hdc = GetDC(*this);
	BitBlt(hdc, 0, 0, rc.right, rc.bottom, off_dc, 0, 0, SRCCOPY);
	ReleaseDC(*this, hdc);
}

void puzzle_window::animate_tile(int timer_id)
{
	animating = true;
	int i = timer_id / info.word_length;
	int j = timer_id % info.word_length;

	if (brd.animation_time[i][j] > info.animation_events)
	{
		KillTimer(*this, timer_id);
		if ((lose || win) && i == brd.word_count - 1 && j == info.word_length - 1)
			animating = false;
		if (pending_overlay)
			draw_overlay();
		return;
	}

	double progress = 1.0 * brd.animation_time[i][j] / info.animation_events;
	if (j < 4 && abs(progress - 0.5) < 0.01)
		SetTimer(*this, get_timer_id(i, j + 1, id), info.animation_tick, NULL);

	brd.animation_time[i][j]++;

	RECT rc = {
		.left = (info.margin + info.letter_size) * j + info.margin,
		.top = (info.margin + info.letter_size) * i + info.margin,
		.right = rc.left + info.letter_size,
		.bottom = rc.top + info.letter_size
	};
	RECT old_rc = rc;

	HPEN frame_pen = gdi::get_pen;
	HPEN empty_pen = gdi::get_empty_pen;
	HBRUSH brush = gdi::get_bckgrd_brush;
	HBRUSH default_brush = gdi::get_default_brush;
	HBRUSH color_brush;
	HFONT font = _gdi.get_font;

	HPEN old_pen = (HPEN)SelectObject(off_dc, empty_pen);
	HBRUSH old_brush = (HBRUSH)SelectObject(off_dc, brush);
	HFONT old_font = (HFONT)SelectObject(off_dc, font);

	switch (brd.colors[i][j])
	{
	case 3:
		color_brush = gdi::get_green_brush;
		break;
	case 2:
		color_brush = gdi::get_yellow_brush;
		break;
	case 1:
		color_brush = gdi::get_gray_brush;
		break;
	case 0:
	default:
		color_brush = brush;
	}

	RoundRect(off_dc, rc.left - 3, rc.top - 3, rc.right + 3, rc.bottom + 3, ROUND);

	SelectObject(off_dc, frame_pen);
	if (progress < 0.5)
		SelectObject(off_dc, default_brush);
	else
		SelectObject(off_dc, color_brush);

	int height = rc.bottom - rc.top;
	rc.top = rc.top + height * progress;
	rc.bottom = rc.bottom - height * progress;
	RoundRect(off_dc, rc.left, rc.top, rc.right, rc.bottom, ROUND);

	DrawText(off_dc, &brd(i, j), 1, &rc,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	GetClientRect(m_hWnd, &rc);
	HDC hdc = GetDC(m_hWnd);
	BitBlt(hdc, 0, 0, rc.right, rc.bottom, off_dc, 0, 0, SRCCOPY);

	SelectObject(off_dc, old_brush);
	SelectObject(off_dc, old_pen);
	DeleteDC(hdc);

	SetTimer(m_hWnd, 0, info.animation_tick, NULL);
}

LRESULT puzzle_window::window_proc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		HDC hdc = GetDC(m_hWnd);
		off_dc = CreateCompatibleDC(hdc);
		ReleaseDC(m_hWnd, hdc);
	}
	break;
	case WM_SIZE:
	{
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);

		HDC hdc = GetDC(m_hWnd);

		if (off_oldbitmap != NULL)
			SelectObject(off_dc, off_oldbitmap);
		if (off_bitmap != NULL)
			DeleteObject(off_bitmap);

		off_bitmap = CreateCompatibleBitmap(hdc, width, height);
		off_oldbitmap = (HBITMAP)SelectObject(off_dc, off_bitmap);
		ReleaseDC(m_hWnd, hdc);
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(m_hWnd, &ps);
		draw_tiles();
		if (win || lose)
			draw_overlay();

		RECT rc;
		GetClientRect(m_hWnd, &rc);
		BitBlt(hdc, 0, 0, rc.right, rc.bottom, off_dc, 0, 0, SRCCOPY);

		EndPaint(m_hWnd, &ps);
	}
	break;
	case WM_TIMER:
		animate_tile(wParam);
	break;
	case WM_NCHITTEST:
		return HTCAPTION;
	case WM_DESTROY:
		if (off_oldbitmap != NULL)
			SelectObject(off_dc, off_oldbitmap);
		if (off_dc != NULL)
			DeleteDC(off_dc);
		if (off_bitmap != NULL)
			DeleteObject(off_bitmap);
		PostQuitMessage(0);
		break;
	default:
		return window::window_proc(msg, wParam, lParam);
	}
	return 0;
}

puzzle_window::puzzle_window(HINSTANCE hInst, keyboard_window* parent, int id, std::string answer)
	: m_instance{ hInst }, window(s_class_name), id{ id }, answer{ answer }
{
	if (!is_class_registered(hInst, s_class_name))
		register_class(hInst);

	m_parent = parent;

	LoadStringW(hInst, IDS_PUZZLE_TITLE, title, MAX_LOADSTRING);
	m_hWnd = CreateWindowExW(0, s_class_name, title, (id > 0 ? 0 : WS_VISIBLE) | style,
		info.x_center, info.y_center, info.width, info.height_easy, *parent, nullptr, hInst,
		reinterpret_cast<LPVOID>(this));
}

void puzzle_window::update_board(WCHAR c)
{
	int limit = m_parent->word_limit;

	if (brd.word_count >= limit)
		return;

	if (isalpha(c))
	{
		if (brd.letter_count < info.word_length)
		{
			brd(brd.word_count, brd.letter_count) = c;
			brd.letter_count++;
		}
		else
			brd.letter_count = info.word_length;
		InvalidateRect(*this, NULL, FALSE);
	}
	else if (c == VK_BACK)
	{
		brd.letter_count > 0 ? brd.letter_count-- : 0;
		brd(brd.word_count, brd.letter_count) = '\0';
		InvalidateRect(*this, NULL, FALSE);
	}
	if (c == VK_RETURN && brd.letter_count == info.word_length)
	{
		check_guess();
	}
}

void puzzle_window::check_guess()
{
	int limit = m_parent->word_limit;

	std::string guess = "";
	for (int i = 0; i < info.word_length; i++)
		guess += brd(brd.word_count, i);

	brd.letter_count = 0;

	if (!wordle::dictionary.contains(guess))
	{
		for (int i = 0; i < info.word_length; i++)
			brd(brd.word_count, i) = '\0';
		InvalidateRect(*this, NULL, FALSE);
		return;
	}

	int color = 0;
	win = true;

	for (int i = 0; i < info.word_length; i++)
	{
		if (answer.find(guess[i]) == std::string::npos)
			color = 1;
		else if (answer[i] == guess[i])
			color = 3;
		else
			color = 2;

		if (color != 3)
			win = false;

		bool better_color = true;
		for (int j = 0; j < i; j++)
			if (brd.colors[brd.word_count][i] > color)
				better_color = false;

		if (better_color)
		{
			brd.colors[brd.word_count][i] = color;
			if (m_parent->kbrd(id, guess[i]) < color)
				m_parent->kbrd(id, guess[i]) = color;
		}

	}

	animating = true;
	SetTimer(*this, get_timer_id(brd.word_count, 0, id),
		info.animation_tick, NULL);

	if (win)
		draw_overlay();

	if (brd.word_count < limit)
		brd.word_count++;
	if (brd.word_count == limit)
	{
		lose = true;
		draw_overlay();
	}

	InvalidateRect(*m_parent, NULL, FALSE);
}

void puzzle_window::reset()
{
	win = lose = animating = pending_overlay = false;
	answer = wordle::get_random_entry();
	for (auto& row : brd.letters)
		row.fill('\0');
	for (auto& row : brd.colors)
		row.fill(0);
	for (auto& row : brd.animation_time)
		row.fill(0);
	brd.letter_count = brd.word_count = 0;
	InvalidateRect(m_hWnd, NULL, FALSE);
}

int puzzle_window::get_timer_id(int i, int j, int wnd_id)
{
	return i * info.word_length + j;
}

#pragma endregion