#include <windows.h>
#include <ctime>

#include <gdiplus.h>
#pragma comment(lib, "GdiPlus.lib") 
using namespace Gdiplus; 

static const GUID png =
{ 0x557cf406, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e } };

int main()
{
	HWND hWnd = GetConsoleWindow();
	ShowWindow(hWnd, SW_HIDE);	//hide console window

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);	//initialize Windows GDI+

	HDC screenDC = GetDC(NULL);	
	HDC virtualDC = CreateCompatibleDC(screenDC);

	int height = GetSystemMetrics(SM_CYSCREEN); //height of monitor
	int width = GetSystemMetrics(SM_CXSCREEN); //width of monitor

	HBITMAP bmp = CreateCompatibleBitmap(screenDC, width, height);
	HGDIOBJ oldBmp = SelectObject(virtualDC, bmp);

	BitBlt(virtualDC, 0, 0, width, height, screenDC, 0, 0, SRCCOPY);	

	Gdiplus::Bitmap *bitmap = new Gdiplus::Bitmap(bmp, NULL);
	bitmap->Save(L"screenshot.png", &png);

	SelectObject(virtualDC, oldBmp);
	DeleteObject(screenDC);
	DeleteObject(virtualDC);

	delete bitmap;
	DeleteObject(bmp);
	DeleteObject(oldBmp);
	
	GdiplusShutdown(gdiplusToken);
	CloseWindow(hWnd);

	return 0;
}