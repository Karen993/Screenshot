#include <windows.h>

BOOL SaveBitmapToFile(HBITMAP hBitmap, LPCTSTR szfilename)
{
	HDC hDC;
	//Number of bytes per pixel at current resolution          
	int  iBits;
	//Number of bytes occupied by each pixel in the bitmap          
	WORD  wBitCount;
	//Define the size of the palette, the size of the pixels in the bitmap, the size of the bitmap file, the number of bytes written to the file              
	DWORD  dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
	//Bitmap attribute structure              
	BITMAP Bitmap;
	//Bitmap file header structure          
	BITMAPFILEHEADER bmfHdr;
	//Bitmap information header structure              
	BITMAPINFOHEADER bi;
	//Point to the bitmap information header structure                  
	LPBITMAPINFOHEADER lpbi;
	//Define file, allocate memory handle, palette handle              
	HANDLE fh, hDib, hPal, hOldPal = NULL;

	//Calculate the number of bytes occupied by each pixel of the bitmap file              
	hDC = CreateDC(L"DISPLAY", NULL, NULL, NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	if (iBits <= 1)
		wBitCount = 1;
	else  if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else
		wBitCount = 24;

	GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	bi.biClrUsed = 0;

	dwBmBitsSize = ((Bitmap.bmWidth *wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

	//Allocate memory for bitmap content              
	hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;

	// handle palette                  
	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}

	// Get the new pixel value under this palette              
	GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight,
		(LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize,
		(BITMAPINFO *)lpbi, DIB_RGB_COLORS);

	//Restore the color palette                  
	if (hOldPal)
	{
		::SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}

	//Create bitmap file                  
	fh = CreateFile(szfilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (fh == INVALID_HANDLE_VALUE)         return     FALSE;

	// Set the bitmap file header              
	bmfHdr.bfType = 0x4D42;     //     "BM"              
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
	// write bitmap file header              
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	// Write the rest of the bitmap file              
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
	//Clear                  
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);

	return TRUE;
}

HBITMAP getScreenBmp() {
	//get monitor parameters
	int xLeftTop = GetSystemMetrics(SM_XVIRTUALSCREEN); // x coordinate for left upper corner of the monitor
	int yLeftTop = GetSystemMetrics(SM_XVIRTUALSCREEN); // y coordinate for left upper corner of the monitor
	int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);	//width of monitor
	int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);	//height of monitor

	HDC screenDC = GetDC(NULL); //get DC for the entire screen
	HDC virtualDC = CreateCompatibleDC(screenDC);
	HBITMAP hBitmap = CreateCompatibleBitmap(screenDC, width, height);

	HGDIOBJ oldBitmap = SelectObject(virtualDC, hBitmap);

	if (!BitBlt(virtualDC, 0, 0, width, height, screenDC, xLeftTop, yLeftTop, SRCCOPY)) {
		MessageBox(NULL, L"An error has occurred", L"Error", MB_OK);

		SelectObject(virtualDC, oldBitmap);
		return NULL;
	}

	SelectObject(virtualDC, oldBitmap);
	ReleaseDC(NULL, screenDC);
	DeleteDC(virtualDC);

	return hBitmap;
}



int main() {
	HWND hWnd = GetConsoleWindow();

	if (hWnd == NULL) {
		MessageBox(NULL, L"The program can`t be opened.", L"Error", MB_OK);
		return 0;
	}

	ShowWindow(hWnd, SW_HIDE);	//hide console window

	HBITMAP bmp = getScreenBmp();

	if (bmp) {
		SaveBitmapToFile(bmp, L"screenshot.bmp");
	}

	DeleteObject(bmp);
	CloseWindow(hWnd);

	return 0;
}
