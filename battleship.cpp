#include <windows.h>
#include <stdlib.h>

#define APP_NAME		TEXT("Battleship")
#define WND_TITLE		TEXT("Battleship")
#define WND_WIDTH 800
#define WND_HEIGHT 400
LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI ThreadFunc(LPVOID vdParam) ;
VOID Paint(HDC,HDC);
VOID PaintBackground(HDC);
VOID PaintBattleships(HDC,HDC,int x,int y);
VOID PaintGun(HDC hdc);
VOID PaintShell(HDC);
VOID MoveShell(VOID);

HWND hMainWindow;       		/*アプリケーションウィンドウのハンドル*/

int seaHeight =150;
int waitTime = 30;
BOOL isRun = FALSE;     		/*実行中は TRUE*/

#define Shell_MOVE  5
#define Shell_W    5
#define Shell_H    2
POINT shell[60];  	                        /* 砲弾の位置　*/
float shell_dirx = 1., shell_diry = -1.;

#define GUN_W  50
#define GUN_H  10
POINT gun = { 300 , 400 };



RECT wnd_rect;

static HBRUSH hBrushRed;
static HBRUSH hBrushSky;
static HBRUSH hBrushSea;

#define SHIP_IMAGE TEXT("AOBA.bmp")

//-----------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,PSTR lpCmdLine, int nCmdShow){
	WNDCLASS wc;
	MSG msg;

	wc.style	= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc	= WindowProc;
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	= hInstance;
	wc.hIcon	= LoadIcon(NULL , IDI_APPLICATION);
	wc.hCursor	= LoadCursor(NULL , IDC_ARROW);
	wc.hbrBackground= (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName	= NULL;
	wc.lpszClassName= APP_NAME;

	if (!RegisterClass(&wc)) 	return 0;

	hMainWindow = CreateWindow(
		APP_NAME , WND_TITLE ,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT , CW_USEDEFAULT,
		WND_WIDTH  , WND_HEIGHT,
		NULL , NULL , hInstance , NULL	);
	if (hMainWindow == NULL) return 0;

	while(GetMessage(&msg, NULL, 0, 0) > 0)  DispatchMessage(&msg);

	return msg.wParam;
}

//-----------------------------------------------------------------
LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	HDC hdc;
	PAINTSTRUCT ps;
	static DWORD dwThreadID;

	/*ダブルバッファリング用のビットマップとデバイスコンテキスト*/
	static HBITMAP hWndBuffer;
	static HDC hBufferDC;
	static HBITMAP hBmpShip;
	static BITMAP bmpShip;

	static HDC hMemDC;
	static HINSTANCE hInstance;

	switch(uMsg) {

	case WM_CREATE:

		hMainWindow = hWnd;
		hInstance = (HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE);

		GetClientRect(hWnd, &wnd_rect);

		hdc = GetDC(hWnd);
		hWndBuffer = CreateCompatibleBitmap(hdc, wnd_rect.right, wnd_rect.bottom);
		hBufferDC = CreateCompatibleDC(hdc);
		SelectObject(hBufferDC, hWndBuffer);
		ReleaseDC(hWnd, hdc);

		hBrushRed = CreateSolidBrush(RGB(255,0,0));
		hBrushSky = CreateSolidBrush(RGB(102,160,255));
		hBrushSea = CreateSolidBrush(RGB(60,50,255));

		hMemDC = CreateCompatibleDC(NULL);

		hBmpShip = (HBITMAP)LoadImage(
			(HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),
			SHIP_IMAGE,IMAGE_BITMAP,
			0,0,LR_LOADFROMFILE);

			SelectObject(hMemDC,hBmpShip);
			GetObject(hBmpShip,sizeof(bmpShip),&bmpShip);


		isRun = TRUE;
		CreateThread(NULL, 0, ThreadFunc, (LPVOID)hWnd, 0, &dwThreadID);
		return 0;

	case WM_MOUSEMOVE:
		gun.x  = LOWORD(lParam) - (GUN_W/2);
		if      (gun.x         < 0    ) gun.x = 0;
		else if (gun.x + GUN_W > wnd_rect.right) gun.x = wnd_rect.right - GUN_W;

		gun.y  = HIWORD(lParam) - (GUN_H/2);
		if      (gun.y         < 0    ) gun.y = 0;
		else if (gun.y + GUN_H > wnd_rect.bottom) gun.y = wnd_rect.bottom - GUN_H;

		InvalidateRect(hWnd , NULL , FALSE);
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hWnd , &ps);
		Paint(hBufferDC,hMemDC);
		BitBlt(hdc, 0, 0, wnd_rect.right, wnd_rect.bottom, hBufferDC, 0, 0,SRCCOPY);
		EndPaint(hWnd , &ps);
		return 0;

	case WM_DESTROY:
		DeleteDC(hBufferDC);
		DeleteObject(hWndBuffer);
		DeleteObject(hBrushRed);
		DeleteObject(hBrushSky);
		DeleteObject(hBrushSea);
		DeleteObject(hBmpShip);

			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProc(hWnd , uMsg , wParam , lParam);
}

//-----------------------------------------------------------------
DWORD WINAPI ThreadFunc(LPVOID vdParam){
	srand ((unsigned) time(NULL));
	for(int i =0; i<60; i++){
		shell[i].x = 120;
		shell[i].y = wnd_rect.bottom-seaHeight+15;
	}
	HWND hWnd = (HWND)vdParam;
	while(isRun)
	{
		MoveShell();
		InvalidateRect(hWnd , NULL , FALSE);
		Sleep(waitTime);
	}
	return TRUE;
}

VOID MoveShell(){
	shell[0].x += 3;
}



//-----------------------------------------------------------------
VOID Paint(HDC hdc,HDC hMemDC){

	FillRect(hdc , &wnd_rect , (HBRUSH)GetStockObject(WHITE_BRUSH));

	PaintBackground(hdc);
	PaintBattleships(hdc,hMemDC,50,wnd_rect.bottom-seaHeight-10);
	PaintGun(hdc);
	PaintShell(hdc);
}

VOID PaintBackground(HDC hdc){
	SelectObject(hdc , hBrushSky);
	SelectObject(hdc , GetStockObject(NULL_PEN));
	Rectangle(hdc , 0, 0,wnd_rect.right, wnd_rect.bottom);
	SelectObject(hdc , hBrushSea);
	Rectangle(hdc , 0, wnd_rect.bottom - seaHeight,wnd_rect.right, wnd_rect.bottom);
}

VOID PaintBattleships(HDC hBufferDC,HDC hMemDC,int x, int y){
	//BitBlt(hBufferDC, x, y, wnd_rect.right, wnd_rect.bottom, hMemDC, 0, 0,SRCCOPY);
	TransparentBlt(hBufferDC,x,y,100,50,hMemDC,0,0,100,50,RGB(0,255,0));
}

VOID PaintGun(HDC hdc)
{
	SelectObject(hdc , GetStockObject(BLACK_PEN));
	SelectObject(hdc , GetStockObject(WHITE_BRUSH));
	Rectangle(hdc , gun.x, gun.y, gun.x+GUN_W, gun.y+GUN_H);
}

VOID PaintShell(HDC hdc)
{
	SelectObject(hdc , GetStockObject(NULL_PEN));
	SelectObject(hdc , GetStockObject(BLACK_BRUSH));
	for(int i = 0; i<60; i++){
		Rectangle(hdc , shell[i].x , shell[i].y , shell[i].x + Shell_W, shell[i].y + Shell_H);
	}
}
