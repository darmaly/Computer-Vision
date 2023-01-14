
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>
#include <process.h>	/* needed for multithreading */
#include "resource.h"
#include "globals.h"

/* Function to handle pixel intensity difference input */
BOOL CALLBACK WndProcPixelIntesity(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemText(hWnd, IDC_PIXEL_INTENSITY, intensity, 256);
			pixelIntensity = atoi(intensity);
			EndDialog(hWnd, wParam);
			break;

		case IDCANCEL:
			EndDialog(hWnd, wParam);
		}
		break;
	default: 
		return FALSE;
	}
	return TRUE;
}

	/* Gets distance from centroid */
BOOL CALLBACK WndProcCentroidDist(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemText(hWnd, IDC_CENTROID_DIST, dist, 256);
			centDist = atoi(dist);
			EndDialog(hWnd, wParam);
			break;

		case IDCANCEL:
			EndDialog(hWnd, wParam);
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}




int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow)

{
	MSG			msg;
	HWND		hWnd;
	WNDCLASS	wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, "ID_PLUS_ICON");
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = "ID_MAIN_MENU";
	wc.lpszClassName = "PLUS";

	if (!RegisterClass(&wc))
		return(FALSE);

	hWnd = CreateWindow("PLUS", "Lab 4",
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
		CW_USEDEFAULT, 0, 400, 400, NULL, NULL, hInstance, NULL);
	if (!hWnd)
		return(FALSE);

	ShowScrollBar(hWnd, SB_BOTH, FALSE);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	MainWnd = hWnd;

	ShowPixelCoords = 0;
	threadCount = 0;
	PlayMode = 0;
	StepMode = 0;
	rg_thread = 0;
	jPress = 0;
	threadStatus = 1;
	pixelIntensity = 0;
	centDist = 0;
	count++;

	strcpy(filename, "");
	OriginalImage = NULL;
	ROWS = COLS = 0;

	InvalidateRect(hWnd, NULL, TRUE);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return(msg.wParam);
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	HMENU				hMenu;
	OPENFILENAME		ofn;
	FILE* fpt;
	HDC					hDC;
	char				header[320], text[320];
	int					BYTES, xPos, yPos;

	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_SHOWPIXELCOORDS:
			ShowPixelCoords = (ShowPixelCoords + 1) % 2;
			PaintImage();
			break;

			/* User selects play or step mode*/
		case ID_MODE_STEPMODE:
			StepMode = (StepMode + 1) % 2;
			PlayMode = 0;
			threadStatus = 1;
			rg_thread = 1;
			break;

		case ID_MODE_PLAYMODE:
			PlayMode = (PlayMode + 1) % 2;
			StepMode = 0;
			threadStatus = 1;
			rg_thread = 1;
			break;

			/* When user selects to input pixel intesity diff or centroid dist */
		case ID_PIX_DIFF:
			DialogBox(NULL, MAKEINTRESOURCE(IDD_PIXEL_INTENSITY), hWnd, WndProcPixelIntesity);
			break;

		case ID_DIST: 
			DialogBox(NULL, MAKEINTRESOURCE(IDD_CENTROID_DIST), hWnd, WndProcCentroidDist);
			break;

			/* Creates dialog box with default color pallet selector and stores it in color struct */
		case ID_COLOR:
			ZeroMemory(&color, sizeof(color));
			color.lStructSize = sizeof(color);
			color.hwndOwner = MainWnd;
			color.lpCustColors = (LPDWORD) custColor;
			color.rgbResult = rgbCurrent;
			color.Flags = CC_FULLOPEN | CC_RGBINIT;

			xPos = LOWORD(lParam);
			yPos = HIWORD(lParam);

			if (ChooseColor(&color) == TRUE) {
				brush = CreateSolidBrush(color.rgbResult);
				rgbCurrent = color.rgbResult;

				Rval = GetRValue(rgbCurrent);
				Gval = GetGValue(rgbCurrent);
				Bval = GetBValue(rgbCurrent);
			}
			break;
		
		case ID_CLEAR:
			threadStatus = 0;
			//int red, green, blue;
			if (threadCount != 0) {
				for (xPos = 0; xPos < ROWS; xPos++) {
					for (yPos = 0; yPos < COLS; yPos++) {
						if (indices[xPos * COLS + yPos] == threadCount) {
							hDC = GetDC(MainWnd);
							/*
							red = GetRValue(OriginalImage[xPos * COLS + yPos]);
							green = GetGValue(OriginalImage[xPos * COLS + yPos]);
							blue = GetBValue(OriginalImage[xPos * COLS + yPos]);
							*/
							SetPixel(hDC, yPos, xPos, RGB(OriginalImage[xPos * COLS + yPos], OriginalImage[xPos * COLS + yPos], OriginalImage[xPos * COLS + yPos]));
							ReleaseDC(MainWnd, hDC);
							indices[xPos * COLS + yPos] = 0;
						}
					}
				}
				threadCount--;
			}
			threadStatus = 1;
			break;


		case ID_FILE_LOAD:
			if (OriginalImage != NULL)
			{
				free(OriginalImage);
				OriginalImage = NULL;
			}
			memset(&(ofn), 0, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.lpstrFile = filename;
			filename[0] = 0;
			ofn.nMaxFile = MAX_FILENAME_CHARS;
			ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
			ofn.lpstrFilter = "PPM files\0*.ppm\0All files\0*.*\0\0";
			if (!(GetOpenFileName(&ofn)) || filename[0] == '\0')
				break;		/* user cancelled load */
			if ((fpt = fopen(filename, "rb")) == NULL)
			{
				MessageBox(NULL, "Unable to open file", filename, MB_OK | MB_APPLMODAL);
				break;
			}
			fscanf(fpt, "%s %d %d %d", header, &COLS, &ROWS, &BYTES);
			if (strcmp(header, "P5") != 0 || BYTES != 255)
			{
				MessageBox(NULL, "Not a PPM (P5 greyscale) image", filename, MB_OK | MB_APPLMODAL);
				fclose(fpt);
				break;
			}
			OriginalImage = (unsigned char*)calloc(ROWS * COLS, 1);
			header[0] = fgetc(fpt);	/* whitespace character after header */
			fread(OriginalImage, 1, ROWS * COLS, fpt);
			fclose(fpt);
			SetWindowText(hWnd, filename);
			PaintImage();
			indices = (int*)calloc(ROWS * COLS, sizeof(int));
			break;

		case ID_FILE_QUIT:
			DestroyWindow(hWnd);
			break;
		}
		break;
	case WM_SIZE:		  /* could be used to detect when window size changes */
		PaintImage();
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	case WM_PAINT:
		PaintImage();
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	case WM_LBUTTONDOWN:case WM_RBUTTONDOWN:
		if (rg_thread == 1) {
			xPos = LOWORD(lParam);
			yPos = HIWORD(lParam);

			rowGlobal = yPos;
			colGlobal = xPos;

			if (xPos >= 0 && xPos < COLS && yPos >= 0 && yPos < ROWS) {
				threadCount++;
				_beginthread(RegionGrow, 0, NULL);
			}
		}
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	case WM_MOUSEMOVE:
		if (ShowPixelCoords == 1)
		{
			xPos = LOWORD(lParam);
			yPos = HIWORD(lParam);
			if (xPos >= 0 && xPos < COLS && yPos >= 0 && yPos < ROWS)
			{
				sprintf(text, "%d,%d=>%d     ", xPos, yPos, OriginalImage[yPos * COLS + xPos]);
				hDC = GetDC(MainWnd);
				TextOut(hDC, 0, 0, text, strlen(text));		/* draw text on the window */
				SetPixel(hDC, xPos, yPos, RGB(255, 0, 0));	/* color the cursor position red */
				ReleaseDC(MainWnd, hDC);
			}
		}
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	case WM_KEYDOWN:
		if (wParam == 's' || wParam == 'S')
			PostMessage(MainWnd, WM_COMMAND, ID_SHOWPIXELCOORDS, 0);	  /* send message to self */

			/* Detect the letter j being pressed for step mode */
		if (wParam == 'j' || wParam == 'J')
			jPress = 1;

		if ((TCHAR)wParam == '1')
		{
			TimerRow = TimerCol = 0;
			SetTimer(MainWnd, TIMER_SECOND, 10, NULL);	/* start up 10 ms timer */
		}
		if ((TCHAR)wParam == '2')
		{
			KillTimer(MainWnd, TIMER_SECOND);			/* halt timer, stopping generation of WM_TIME events */
			PaintImage();								/* redraw original image, erasing animation */
		}
		if ((TCHAR)wParam == '3')
		{
			ThreadRunning = 1;
			_beginthread(AnimationThread, 0, MainWnd);	/* start up a child thread to do other work while this thread continues GUI */
		}
		if ((TCHAR)wParam == '4')
		{
			ThreadRunning = 0;							/* this is used to stop the child thread (see its code below) */
		}
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	case WM_TIMER:	  /* this event gets triggered every time the timer goes off */
		hDC = GetDC(MainWnd);
		SetPixel(hDC, TimerCol, TimerRow, RGB(0, 0, 255));	/* color the animation pixel blue */
		ReleaseDC(MainWnd, hDC);
		TimerRow++;
		TimerCol += 2;
		break;
	case WM_HSCROLL:	  /* this event could be used to change what part of the image to draw */
		PaintImage();	  /* direct PaintImage calls eliminate flicker; the alternative is InvalidateRect(hWnd,NULL,TRUE); UpdateWindow(hWnd); */
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	case WM_VSCROLL:	  /* this event could be used to change what part of the image to draw */
		PaintImage();
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	}

	hMenu = GetMenu(MainWnd);
	if (ShowPixelCoords == 1)
		CheckMenuItem(hMenu, ID_SHOWPIXELCOORDS, MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else
		CheckMenuItem(hMenu, ID_SHOWPIXELCOORDS, MF_UNCHECKED);

	if (StepMode == 1)
		CheckMenuItem(hMenu, ID_MODE_STEPMODE, MF_CHECKED);
	else
		CheckMenuItem(hMenu, ID_MODE_STEPMODE, MF_UNCHECKED);

	if (PlayMode == 1)
		CheckMenuItem(hMenu, ID_MODE_PLAYMODE, MF_CHECKED);
	else
		CheckMenuItem(hMenu, ID_MODE_PLAYMODE, MF_UNCHECKED);

	DrawMenuBar(hWnd);

	return(0L);
}




void PaintImage() {

	PAINTSTRUCT			Painter;
	HDC					hDC;
	BITMAPINFOHEADER	bm_info_header;
	BITMAPINFO* bm_info;
	int					i, r, c, DISPLAY_ROWS, DISPLAY_COLS;
	unsigned char* DisplayImage;

	if (OriginalImage == NULL)
		return;		/* no image to draw */

	/* Windows pads to 4-byte boundaries.  We have to round the size up to 4 in each dimension, filling with black. */
	DISPLAY_ROWS = ROWS;
	DISPLAY_COLS = COLS;
	if (DISPLAY_ROWS % 4 != 0)
		DISPLAY_ROWS = (DISPLAY_ROWS / 4 + 1) * 4;
	if (DISPLAY_COLS % 4 != 0)
		DISPLAY_COLS = (DISPLAY_COLS / 4 + 1) * 4;
	DisplayImage = (unsigned char*)calloc(DISPLAY_ROWS * DISPLAY_COLS, 1);
	for (r = 0; r < ROWS; r++)
		for (c = 0; c < COLS; c++)
			DisplayImage[r * DISPLAY_COLS + c] = OriginalImage[r * COLS + c];

	BeginPaint(MainWnd, &Painter);
	hDC = GetDC(MainWnd);
	bm_info_header.biSize = sizeof(BITMAPINFOHEADER);
	bm_info_header.biWidth = DISPLAY_COLS;
	bm_info_header.biHeight = -DISPLAY_ROWS;
	bm_info_header.biPlanes = 1;
	bm_info_header.biBitCount = 8;
	bm_info_header.biCompression = BI_RGB;
	bm_info_header.biSizeImage = 0;
	bm_info_header.biXPelsPerMeter = 0;
	bm_info_header.biYPelsPerMeter = 0;
	bm_info_header.biClrUsed = 256;
	bm_info_header.biClrImportant = 256;
	bm_info = (BITMAPINFO*)calloc(1, sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD));
	bm_info->bmiHeader = bm_info_header;
	for (i = 0; i < 256; i++)
	{
		bm_info->bmiColors[i].rgbBlue = bm_info->bmiColors[i].rgbGreen = bm_info->bmiColors[i].rgbRed = i;
		bm_info->bmiColors[i].rgbReserved = 0;
	}

	SetDIBitsToDevice(hDC, 0, 0, DISPLAY_COLS, DISPLAY_ROWS, 0, 0,
		0, /* first scan line */
		DISPLAY_ROWS, /* number of scan lines */
		DisplayImage, bm_info, DIB_RGB_COLORS);
	ReleaseDC(MainWnd, hDC);
	EndPaint(MainWnd, &Painter);

	free(DisplayImage);
	free(bm_info);
}




void AnimationThread(HWND AnimationWindowHandle) {

	HDC		hDC;
	char	text[300];

	ThreadRow = ThreadCol = 0;
	while (ThreadRunning == 1)
	{
		hDC = GetDC(MainWnd);
		SetPixel(hDC, ThreadCol, ThreadRow, RGB(0, 255, 0));	/* color the animation pixel green */
		sprintf(text, "%d,%d     ", ThreadRow, ThreadCol);
		TextOut(hDC, 300, 0, text, strlen(text));		/* draw text on the window */
		ReleaseDC(MainWnd, hDC);
		ThreadRow += 3;
		ThreadCol++;
		Sleep(100);		/* pause 100 ms */
	}
}

/* Region grow function */
void RegionGrow(HWND RegionGrowHandle) {
	unsigned char* labels;	/* segmentation labels */
	int r, c;		/* pixel to paint from */
	int i;		/* output:  indices of pixels painted */
	int	r2, c2;
	int	queue[MAX_QUEUE], qh, qt;
	int	average, total;	/* average and total intensity in growing region */
	HDC	hDC;
	int rLocal = rowGlobal;
	int	cLocal = colGlobal;
	int red, green, blue;
	int thread = threadCount;
	int maxDist = 0;
	int intensity = pixelIntensity;
	int distance = centDist;

	int cent_r_avg = rowGlobal;
	int cent_c_avg = colGlobal;

	red = Rval;
	green = Gval;
	blue = Bval;

	count = 0;
	labels = (unsigned char*)calloc(ROWS * COLS, sizeof(unsigned char));
	r = rowGlobal;
	c = colGlobal;


	labels[r * COLS + c] = 1;
	average = total = (int)OriginalImage[r * COLS + c];

	queue[0] = r * COLS + c;
	qh = 1;	/* queue head */
	qt = 0;	/* queue tail */
	count = 1;
	jPress = 0;

	while (qt != qh)
	{
		hDC = GetDC(MainWnd);
		
		average = total / (count);

		if (threadStatus == 0) _endthread();

		for (r2 = -1; r2 <= 1; r2++) 
			for (c2 = -1; c2 <= 1; c2++)
			{
				if (threadStatus == 0 ) _endthread();

				if (r2 == 0 && c2 == 0)
					continue;

				if ((queue[qt] / COLS + r2) < 0 || (queue[qt] / COLS + r2) >= ROWS ||
					(queue[qt] % COLS + c2) < 0 || (queue[qt] % COLS + c2) >= COLS)
					continue;

				if (labels[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2] != 0)
					continue;
				/* test criteria to join region */
				
				/* Pixel Intensity Difference Check */
				if (abs((int)(OriginalImage[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2]) - average) > intensity)
					continue;
				
				r = cent_r_avg / count;
				c = cent_c_avg / count;
				i = (int)sqrt(SQR((queue[qt] / COLS + r2) - r) + SQR((queue[qt] % COLS + c2) - c));
				/* Centroid Distance Check */
				if (distance < i) continue;

				cent_r_avg += queue[qt] / COLS + r2;
				cent_c_avg += queue[qt] % COLS + c2;
				
				/* Paint Image */
				SetPixel(hDC, queue[qt] % COLS + c2, queue[qt] / COLS + r2, RGB(red, green, blue));
				labels[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2] = 1;


				if (indices[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2] == 0)
					indices[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2] = thread;

				total +=(int) OriginalImage[(queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2];
				count++;

				queue[qh] = (queue[qt] / COLS + r2) * COLS + queue[qt] % COLS + c2;
				qh = (qh + 1) % MAX_QUEUE;
				if (qh == qt)
				{
					exit(0);
				}

				if (StepMode == 1) {
					while (jPress == 0 && StepMode == 1) {
						Sleep(1);
						if (threadStatus == 0) _endthread();
					}
					jPress = 0;
				}
				else {
					Sleep(1);
				}
				if (threadStatus == 0) _endthread();
			}
		qt = (qt + 1) % MAX_QUEUE;
		ReleaseDC(MainWnd, hDC);
	}
	_endthread();
	//free(labels);

}
