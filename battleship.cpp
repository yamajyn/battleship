#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define APP_NAME		TEXT("Battleship")
#define WND_TITLE		TEXT("Battleship")
//#define WND_WIDTH 800
//#define WND_HEIGHT 400
LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI ThreadFunc(LPVOID vdParam) ;
VOID Paint(HDC,HDC);
VOID PaintBackground(HDC,HDC);
VOID PaintBattleships(HDC,HDC,int x,int y);
VOID PaintGun(HDC hdc);
VOID PaintShell(HDC);
VOID MoveGun(VOID);
VOID MoveShell(VOID);
VOID MoveCloud(VOID);

HWND hMainWindow;       		/*アプリケーションウィンドウのハンドル*/

int seaHeight =150;
int waitTime = 1;
BOOL isRun = FALSE;     		/*実行中は TRUE*/
int FPS;

#define Shell_MOVE  30.
#define Shell_W    8
#define Shell_H    5
POINT shell[60];  	                        /* 砲弾の位置　*/
float shell_dirx = 1., shell_diry = -1.;
int shellNum = 0;

#define GUN_W  50
#define GUN_H  10
POINT gun = { 210 , 340 };
double gunAngle = 0;

POINT mouse = { 300 , 400 };

int firingTime[60];
double firingAngle[60];


RECT wnd_rect;

static HBRUSH hBrushRed;
static HBRUSH hBrushSky;
static HBRUSH hBrushSea;
static HBITMAP hBmpShip[5];
#define IMAGE_01 TEXT("img/aoba-01.bmp")
#define IMAGE_02 TEXT("img/aoba-02.bmp")
#define IMAGE_03 TEXT("img/aoba-03.bmp")
#define IMAGE_04 TEXT("img/aoba-04.bmp")
#define IMAGE_05 TEXT("img/aoba-05.bmp")
typedef struct {
    HBITMAP image;
    float x;
    float y;
} CLOUD;
CLOUD cloud[10];
static HBITMAP destroyer;
#define IMAGE_06 TEXT("img/cloud.bmp")
#define IMAGE_07 TEXT("img/destroyer.bmp")

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
		CW_USEDEFAULT  , CW_USEDEFAULT,
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


	static HDC hMemDC;
	static HINSTANCE hInstance;

	switch(uMsg) {

	case WM_CREATE:

		hMainWindow = hWnd;
		hInstance = (HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE);

		GetClientRect(hWnd, &wnd_rect);
		gun.y = wnd_rect.bottom-seaHeight-10 +50;

		hdc = GetDC(hWnd);
		hWndBuffer = CreateCompatibleBitmap(hdc, wnd_rect.right, wnd_rect.bottom);
		hBufferDC = CreateCompatibleDC(hdc);
		SelectObject(hBufferDC, hWndBuffer);
		ReleaseDC(hWnd, hdc);

		hBrushRed = CreateSolidBrush(RGB(255,0,0));
		hBrushSky = CreateSolidBrush(RGB(102,160,255));
		hBrushSea = CreateSolidBrush(RGB(60,50,255));

		hMemDC = CreateCompatibleDC(NULL);


		hBmpShip[0] = (HBITMAP)LoadImage(hInstance,IMAGE_01,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
		hBmpShip[1] = (HBITMAP)LoadImage(hInstance,IMAGE_02,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
		hBmpShip[2] = (HBITMAP)LoadImage(hInstance,IMAGE_03,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
		hBmpShip[3] = (HBITMAP)LoadImage(hInstance,IMAGE_04,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
		hBmpShip[4] = (HBITMAP)LoadImage(hInstance,IMAGE_05,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
		for(int i=0; i<10;i++){
			cloud[i].image = (HBITMAP)LoadImage(hInstance,IMAGE_06,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
		}
		destroyer = (HBITMAP)LoadImage(hInstance,IMAGE_07,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);


		isRun = TRUE;
		CreateThread(NULL, 0, ThreadFunc, (LPVOID)hWnd, 0, &dwThreadID);
		return 0;

	case WM_MOUSEMOVE:
		mouse.x = LOWORD(lParam);
		mouse.y = HIWORD(lParam);
		return 0;

	case WM_LBUTTONUP:
		if(shellNum<60){
			firingTime[shellNum] = clock();
			firingAngle[shellNum] = gunAngle;
			shellNum +=1;
		}
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
		for(int i =0; i<5; i++){
			DeleteObject(hBmpShip[i]);
		}
		for(int i= 0; i<10;i++){
		DeleteObject(cloud[i].image);
		}
		DeleteObject(destroyer);

			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProc(hWnd , uMsg , wParam , lParam);
}

//-----------------------------------------------------------------
DWORD WINAPI ThreadFunc(LPVOID vdParam){
	DWORD frames = 0,beforeTime;
	srand ((unsigned) time(NULL));
	for(int i=0; i<10;i++){
		cloud[i].x = rand()%3000;
		cloud[i].y = rand()%(wnd_rect.bottom-seaHeight-50);
	}
	for(int i =0; i<60; i++){
		shell[i].x = gun.x;
		shell[i].y = gun.y;
		firingTime[i] = 0;
	}
	beforeTime = timeGetTime();
	HWND hWnd = (HWND)vdParam;
	while(isRun)
	{
		DWORD nowTime, progress,idealTime;
		nowTime = timeGetTime();
		progress = nowTime - beforeTime;
		idealTime = (DWORD)(frames * (1000.0F / 60));
		MoveGun();
		MoveShell();
		MoveCloud();
		InvalidateRect(hWnd , NULL , FALSE);
		Sleep(waitTime);
		if(progress>= 1000){
			FPS = (int)frames;
			beforeTime =nowTime;
			frames = 0;
		}
		frames++;
	}
	return TRUE;
}

VOID MoveGun(){
	if(mouse.x!=gun.x){
		gunAngle = atan2(-(mouse.y-gun.y),mouse.x-gun.x);
		if(gunAngle>0.785){
			gunAngle = 0.785;
		}else if(gunAngle <0){
			gunAngle = 0;
		}
}
}
VOID MoveShell(){

	double g = 9.8;
	for(int i = 0; i<shellNum; i++){
		double now = clock();
		double t = double(now - firingTime[i])/1000.;
		shell[i].x = gun.x + (Shell_MOVE * cos(firingAngle[i])*t)*14.;
		shell[i].y = gun.y + (-Shell_MOVE*sin(firingAngle[i]) * t + g*t*t/2.)*10.;
	}



}

VOID MoveCloud(){
	for(int i=0;i<10;i++){
		cloud[i].x -=0.1;
		if(cloud[i].x<-100){
			cloud[i].x +=wnd_rect.right+200;
		}
	}
}


//-----------------------------------------------------------------
VOID Paint(HDC hdc,HDC hMemDC){

	FillRect(hdc , &wnd_rect , (HBRUSH)GetStockObject(WHITE_BRUSH));

	PaintBackground(hdc,hMemDC);
	PaintBattleships(hdc,hMemDC,50,wnd_rect.bottom-seaHeight-10);
	PaintGun(hdc);
	PaintShell(hdc);
	char buf[128];
	sprintf(buf," mouse  %d %d gunAngle %0.3f FPS: %04d ",mouse.x,mouse.y,gunAngle,FPS);
	TextOut(hdc,0,0,buf,strlen(buf));
}

VOID PaintBackground(HDC hdc,HDC hMemDC){
	SelectObject(hdc , hBrushSky);
	SelectObject(hdc , GetStockObject(NULL_PEN));
	Rectangle(hdc , 0, 0,wnd_rect.right, wnd_rect.bottom);
	SelectObject(hdc , hBrushSea);
	Rectangle(hdc , 0, wnd_rect.bottom - seaHeight,wnd_rect.right, wnd_rect.bottom);
	for(int i=0; i<10;i++){
		SelectObject(hMemDC,cloud[i].image);
		TransparentBlt(hdc,cloud[i].x,cloud[i].y,100,50,hMemDC,0,0,100,50,RGB(0,255,0));
	}
}

VOID PaintBattleships(HDC hdc,HDC hMemDC,int x, int y){
	float max = 0.785;
	//BitBlt(hBufferDC, x, y, wnd_rect.right, wnd_rect.bottom, hMemDC, 0, 0,SRCCOPY);
	SelectObject(hMemDC,hBmpShip[0]);
	for(int i= 0; i<5; i++){
		if(gunAngle>max*i/5 && gunAngle<=max*(i+1)/5){
			SelectObject(hMemDC,hBmpShip[i]);
		}
	}
	TransparentBlt(hdc,x,y,200,100,hMemDC,0,0,200,100,RGB(0,255,0));
}

VOID PaintGun(HDC hdc)
{
	/*SelectObject(hdc , GetStockObject(BLACK_PEN));
	SelectObject(hdc , GetStockObject(WHITE_BRUSH));
	Rectangle(hdc , gun.x, gun.y, gun.x+GUN_W, gun.y+GUN_H);*/
}

VOID PaintShell(HDC hdc)
{
	SelectObject(hdc , GetStockObject(NULL_PEN));
	SelectObject(hdc , GetStockObject(BLACK_BRUSH));
	for(int i = 0; i<shellNum; i++){
		Rectangle(hdc , shell[i].x , shell[i].y , shell[i].x + Shell_W, shell[i].y + Shell_H);
	}
}
