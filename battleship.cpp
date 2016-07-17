#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <vector>
using namespace std;

#define APP_NAME		TEXT("Battleship")
#define WND_TITLE		TEXT("Battleship")
//#define WND_WIDTH 800
//#define WND_HEIGHT 400
LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI ThreadFunc(LPVOID vdParam) ;
VOID loadImages(HINSTANCE hInstance);
VOID Paint(HDC,HDC);
VOID PaintBackground(HDC,HDC);
VOID PaintBattleships(HDC,HDC,int x,int y);
VOID PaintShell(HDC);
VOID PaintWE(HDC hdc,HDC hMemDC);
VOID posInit();
VOID MoveGun(VOID);
VOID MoveShell(VOID);
VOID MoveDD(VOID);
VOID MoveCloud(VOID);

HWND hMainWindow;       		/*?ｿｽA?ｿｽv?ｿｽ?ｿｽ?ｿｽP?ｿｽ[?ｿｽV?ｿｽ?ｿｽ?ｿｽ?ｿｽ?ｿｽE?ｿｽB?ｿｽ?ｿｽ?ｿｽh?ｿｽE?ｿｽﾌハ?ｿｽ?ｿｽ?ｿｽh?ｿｽ?ｿｽ*/

int seaHeight =150;
BOOL isRun = FALSE;     		/*?ｿｽ?ｿｽ?ｿｽs?ｿｽ?ｿｽ?ｿｽ?ｿｽ TRUE*/
float FPS;

#define Shell_MOVE  40.
#define Shell_W    8
#define Shell_H    5
/* ?ｿｽC?ｿｽe?ｿｽﾌ位置?ｿｽ@*/

float shell_dirx = 1., shell_diry = -1.;
typedef struct {
    int x;
    int y;
    int firingTime;
    double firingAngle;
} Shell;
vector<Shell> shell;

#define GUN_W  50
#define GUN_H  10
POINT gun = { 190 , 400 };
double gunAngle = 0;

POINT mouse = { 300 , 370 };

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
} Node;
Node cloud[10];
#define IMAGE_06 TEXT("img/cloud.bmp")

Node dd;
#define IMAGE_07 TEXT("img/destroyer.bmp")

typedef struct {
    HBITMAP image;
    float x;
    float y;
    int lifeTime;
} Effect;
vector<Effect> wE;
HBITMAP hBmpWE[7];
#define IMAGE_08 TEXT("img/wE01.bmp")
#define IMAGE_09 TEXT("img/wE02.bmp")
#define IMAGE_10 TEXT("img/wE03.bmp")
#define IMAGE_11 TEXT("img/wE04.bmp")
#define IMAGE_12 TEXT("img/wE05.bmp")
#define IMAGE_13 TEXT("img/wE06.bmp")
#define IMAGE_14 TEXT("img/wE07.bmp")
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
                               0 , 0,
                               GetSystemMetrics(SM_CXSCREEN)  , 400,
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

    /*?ｿｽ_?ｿｽu?ｿｽ?ｿｽ?ｿｽo?ｿｽb?ｿｽt?ｿｽ@?ｿｽ?ｿｽ?ｿｽ?ｿｽ?ｿｽO?ｿｽp?ｿｽﾌビ?ｿｽb?ｿｽg?ｿｽ}?ｿｽb?ｿｽv?ｿｽﾆデ?ｿｽo?ｿｽC?ｿｽX?ｿｽR?ｿｽ?ｿｽ?ｿｽe?ｿｽL?ｿｽX?ｿｽg*/
    static HBITMAP hWndBuffer;
    static HDC hBufferDC;


    static HDC hMemDC;
    static HINSTANCE hInstance;
    Shell a = {gun.x,gun.y,clock(),gunAngle};

    switch(uMsg) {

        case WM_CREATE:

            hMainWindow = hWnd;
            hInstance = (HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE);

            GetClientRect(hWnd, &wnd_rect);
            gun.y = wnd_rect.bottom-seaHeight-10 +80;

            hdc = GetDC(hWnd);
            hWndBuffer = CreateCompatibleBitmap(hdc, wnd_rect.right, wnd_rect.bottom);
            hBufferDC = CreateCompatibleDC(hdc);
            SelectObject(hBufferDC, hWndBuffer);
            ReleaseDC(hWnd, hdc);

            hBrushRed = CreateSolidBrush(RGB(255,0,0));
            hBrushSky = CreateSolidBrush(RGB(102,160,255));
            hBrushSea = CreateSolidBrush(RGB(60,50,255));

            hMemDC = CreateCompatibleDC(NULL);

            loadImages(hInstance);

            isRun = TRUE;
            CreateThread(NULL, 0, ThreadFunc, (LPVOID)hWnd, 0, &dwThreadID);
            return 0;

        case WM_MOUSEMOVE:
            mouse.x = LOWORD(lParam);
            mouse.y = HIWORD(lParam);
            return 0;

        case WM_LBUTTONUP:

            shell.push_back(a);
            //firingTime[shellNum] = clock();
            //firingAngle[shellNum] = gunAngle;
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
            DeleteObject(dd.image);

            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd , uMsg , wParam , lParam);
}


VOID loadImages(HINSTANCE hInstance){
    hBmpShip[0] = (HBITMAP)LoadImage(hInstance,IMAGE_01,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpShip[1] = (HBITMAP)LoadImage(hInstance,IMAGE_02,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpShip[2] = (HBITMAP)LoadImage(hInstance,IMAGE_03,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpShip[3] = (HBITMAP)LoadImage(hInstance,IMAGE_04,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpShip[4] = (HBITMAP)LoadImage(hInstance,IMAGE_05,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    for(int i=0; i<10;i++){
        cloud[i].image = (HBITMAP)LoadImage(hInstance,IMAGE_06,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    }
    dd.image = (HBITMAP)LoadImage(hInstance,IMAGE_07,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpWE[0] = (HBITMAP)LoadImage(hInstance,IMAGE_08,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpWE[1] = (HBITMAP)LoadImage(hInstance,IMAGE_09,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpWE[2] = (HBITMAP)LoadImage(hInstance,IMAGE_10,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpWE[3] = (HBITMAP)LoadImage(hInstance,IMAGE_11,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpWE[4] = (HBITMAP)LoadImage(hInstance,IMAGE_12,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpWE[5] = (HBITMAP)LoadImage(hInstance,IMAGE_13,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpWE[6] = (HBITMAP)LoadImage(hInstance,IMAGE_14,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
}

//-----------------------------------------------------------------
DWORD WINAPI ThreadFunc(LPVOID vdParam){
    DWORD frames = 0,beforeTime;

    posInit();

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
        MoveDD();
        MoveCloud();
        InvalidateRect(hWnd , NULL , FALSE);
        if(idealTime > progress)Sleep(idealTime - progress);
        if(progress>= 1000){
            FPS = frames;
            beforeTime =nowTime;
            frames = 0;
        }
        frames++;
    }
    return TRUE;
}
VOID posInit(){
    srand ((unsigned) time(NULL));
    for(int i=0; i<10;i++){
        cloud[i].x = rand()%wnd_rect.right;
        cloud[i].y = rand()%(wnd_rect.bottom-seaHeight-10-150)+150;
    }
    dd.x = wnd_rect.right+300;
    dd.y = wnd_rect.bottom-100;
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
    for(int i = 0; i<shell.size(); i++){
        double now = clock();
        double t = double(now - shell[i].firingTime)/1000.;
        shell[i].x = gun.x + (Shell_MOVE * cos(shell[i].firingAngle)*t)*14.;
        shell[i].y = gun.y + (-Shell_MOVE*sin(shell[i].firingAngle) * t + g*t*t/2.)*10.;
    }
}
VOID MoveDD(){
    dd.x -= 0.3;
}

VOID MoveCloud(){
    for(int i=0;i<10;i++){
        cloud[i].x -=0.1+cloud[i].y/(wnd_rect.bottom-seaHeight-10)/2;
        if(cloud[i].x<-100){
            cloud[i].x +=wnd_rect.right+200;
        }
    }
}


//-----------------------------------------------------------------
VOID Paint(HDC hdc,HDC hMemDC){

    FillRect(hdc , &wnd_rect , (HBRUSH)GetStockObject(WHITE_BRUSH));

    PaintBackground(hdc,hMemDC);
    PaintBattleships(hdc,hMemDC,30,wnd_rect.bottom-seaHeight+20);
    PaintShell(hdc);
    PaintWE(hdc,hMemDC);
    for(int i=0; i<shell.size(); i++){
        if(shell[i].y>wnd_rect.bottom-70){
            Effect a = {hBmpWE[0],shell[i].x,wnd_rect.bottom-120,70};
            shell.erase(shell.begin()+i);
            wE.push_back(a);
        }
    }
    char buf[128];
    sprintf(buf," mouse  %d %d gunAngle %0.3f FPS: %0.1f ",mouse.x,mouse.y,gunAngle,FPS);
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
        TransparentBlt(hdc,cloud[i].x,cloud[i].y,100*(cloud[i].y-40)/(wnd_rect.bottom-seaHeight-10),50*(cloud[i].y-40)/(wnd_rect.bottom-seaHeight-10),hMemDC,0,0,100,50,RGB(0,255,0));
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
    TransparentBlt(hdc,x,y,200,90,hMemDC,0,0,200,100,RGB(0,255,0));

    //enemy
    SelectObject(hMemDC,dd.image);
    TransparentBlt(hdc,dd.x,dd.y,100,100,hMemDC,0,0,100,50,RGB(0,255,0));
}


VOID PaintShell(HDC hdc)
{
    SelectObject(hdc , GetStockObject(NULL_PEN));
    SelectObject(hdc , GetStockObject(BLACK_BRUSH));
    for(int i = 0; i<shell.size(); i++){
        Rectangle(hdc , shell[i].x , shell[i].y , shell[i].x + Shell_W, shell[i].y + Shell_H);
    }
}
VOID PaintWE(HDC hdc,HDC hMemDC){
  for(int i=0; i<wE.size(); i++){
    for(int j=0; j<7;j++){
      if(wE[i].lifeTime>=(6-j)*10 && wE[i].lifeTime<=(7-j)*10){
        SelectObject(hMemDC,wE[i].image);
        wE[i].image=hBmpWE[j];
      }
    }
    TransparentBlt(hdc,wE[i].x,wE[i].y,30,50,hMemDC,0,0,30,50,RGB(0,255,0));
    wE[i].lifeTime-=1;
    if(wE[i].lifeTime<0){
      wE.erase(wE.begin()+i);
    }
  }
}
