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
VOID PaintFE(HDC hdc,HDC hMemDC);
VOID PaintScore(HDC hdc,HDC hMemDC);
VOID posInit();
VOID Collision();
VOID MoveGun(VOID);
VOID MoveShell(VOID);
VOID MoveDD(VOID);
VOID MoveAircraft(VOID);
VOID MoveCloud(VOID);

HWND hMainWindow;       		/*??��?��A??��?��v??��?��??��?��??��?��P??��?��[??��?��V??��?��??��?��??��?��??��?��??��?��E??��?��B??��?��??��?��??��?��h??��?��E??��?��̃n??��?��??��?��??��?��h??��?��??��?��*/

int seaHeight =150;
BOOL isRun = FALSE;     		/*??��?��??��?��??��?��s??��?��??��?��??��?��??��?�� TRUE*/
float FPS;
int score = 0;
int playerLife = 3;

#define Shell_MOVE  40.
#define Shell_W    7
#define Shell_H    4

#define Shell2_MOVE  80.
#define Shell2_W    5
#define Shell2_H    3

float shell_dirx = 1., shell_diry = -1.;
typedef struct {
    int x;
    int y;
    int firingTime;
    double firingAngle;
} Shell;
vector<Shell> shell;

#define BOMB_MOVE  5.
typedef struct {
    int x;
    int y;
    int sx;
    int sy;
    int firingTime;
    double firingAngle;
} Bomb;
vector<Bomb> bomb;
typedef struct {
    int x;
    int y;
    int firingTime;
    double firingAngle;
    int expX;
} Shell2;
vector<Shell2> shell2;

#define GUN_W  50
#define GUN_H  10
POINT gun = { 190 , 400 };
double gunAngle = 0;

POINT mouse = { 300 , 370 };

RECT wnd_rect;

static HBRUSH hBrushRed;
static HBRUSH hBrushSky;
static HBRUSH hBrushSea;
static HBRUSH hBrushShell2;
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
    float speed;
    int life;
} Node;


Node cloud[10];
#define IMAGE_06 TEXT("img/cloud.bmp")

vector<Node> dd;
HBITMAP hBmpDD;
#define IMAGE_07 TEXT("img/destroyer.bmp")
int interval = 2000;

typedef struct {
    HBITMAP image;
    float x;
    float y;
    float speed;
    int life;
    bool bomb;
} Plane;

vector<Plane> aircraft;
HBITMAP hBmpAircraft;
int interval2 = 1300;

typedef struct {
    HBITMAP image;
    float x;
    float y;
    float moveX;
    float moveY;
    float speed;
    int mode;
} Biplane;

HBITMAP hBmpAircraft2,hBmpAircraft3;
Biplane aircraft2;

typedef struct {
    HBITMAP image;
    float x;
    float y;
    int width;
    int height;
    int life;
} Effect;

//water
vector<Effect> wE;
HBITMAP hBmpWE[7];
#define IMAGE_08 TEXT("img/wE01.bmp")
#define IMAGE_09 TEXT("img/wE02.bmp")
#define IMAGE_10 TEXT("img/wE03.bmp")
#define IMAGE_11 TEXT("img/wE04.bmp")
#define IMAGE_12 TEXT("img/wE05.bmp")
#define IMAGE_13 TEXT("img/wE06.bmp")
#define IMAGE_14 TEXT("img/wE07.bmp")

//fire
vector<Effect> fE;
HBITMAP hBmpFE[6];
#define IMAGE_15 TEXT("img/fE01.bmp")
#define IMAGE_16 TEXT("img/fE02.bmp")
#define IMAGE_17 TEXT("img/fE03.bmp")
#define IMAGE_18 TEXT("img/fE04.bmp")
#define IMAGE_19 TEXT("img/fE05.bmp")
#define IMAGE_20 TEXT("img/fE06.bmp")

//heart
HBITMAP hbmpHP;
HBITMAP hbmpGameover;

HBITMAP font[10];
//-----------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,PSTR lpCmdLine, int nCmdShow){
    WNDCLASS wc;
    MSG msg;

    wc.style	= CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc	= WindowProc;
    wc.cbClsExtra	= 0;
    wc.cbWndExtra	= 0;
    wc.hInstance	= hInstance;
    wc.hIcon	= LoadIcon(hInstance , TEXT("ICON1"));
    wc.hCursor	= LoadCursor(hInstance , TEXT("CURSOR1"));
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
    static HBITMAP hWndBuffer;
    static HDC hBufferDC;


    static HDC hMemDC;
    static HINSTANCE hInstance;
    Shell a = {0,0,clock(),gunAngle};
    Shell2 b = {0,0,clock(),gunAngle,0};

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
            hBrushShell2  =CreateSolidBrush(RGB(255,239,159));

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
            if(playerLife>0){
              shell.push_back(a);
            }

            return 0;
        case WM_RBUTTONUP:
            if(playerLife>0){
              b.expX = mouse.x;
              shell2.push_back(b);
            }

            return 0;
        case WM_KEYDOWN:
            if (GetKeyState(VK_ESCAPE) < 0) {
              DestroyWindow(hWnd);
            }else if(GetKeyState(VK_SPACE) < 0){
              if(aircraft2.mode == 0){
                aircraft2.mode = 1;
              }else if(aircraft2.mode == 3){
                Bomb bom = {aircraft2.x,aircraft2.y+5,aircraft2.x,aircraft2.y+5,clock(),0};
                bomb.push_back(bom);
                aircraft2.mode = 4;
              }
            }else if(GetKeyState('W') <0){
              if(aircraft2.mode == 3 ){
                aircraft2.moveY = -aircraft2.speed;
              }
            }else if(GetKeyState('S') <0){
              if(aircraft2.mode == 3 ){
                aircraft2.moveY = aircraft2.speed;
              }
            }

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

    hBmpDD = (HBITMAP)LoadImage(hInstance,IMAGE_07,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);

    hBmpAircraft = (HBITMAP)LoadImage(hInstance,TEXT("img/aircraft.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpAircraft2 = (HBITMAP)LoadImage(hInstance,TEXT("img/aircraft2.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpAircraft3 = (HBITMAP)LoadImage(hInstance,TEXT("img/aircraft3.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);


    hBmpWE[0] = (HBITMAP)LoadImage(hInstance,IMAGE_08,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpWE[1] = (HBITMAP)LoadImage(hInstance,IMAGE_09,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpWE[2] = (HBITMAP)LoadImage(hInstance,IMAGE_10,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpWE[3] = (HBITMAP)LoadImage(hInstance,IMAGE_11,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpWE[4] = (HBITMAP)LoadImage(hInstance,IMAGE_12,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpWE[5] = (HBITMAP)LoadImage(hInstance,IMAGE_13,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpWE[6] = (HBITMAP)LoadImage(hInstance,IMAGE_14,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);

    hBmpFE[0] = (HBITMAP)LoadImage(hInstance,IMAGE_15,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpFE[1] = (HBITMAP)LoadImage(hInstance,IMAGE_16,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpFE[2] = (HBITMAP)LoadImage(hInstance,IMAGE_17,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpFE[3] = (HBITMAP)LoadImage(hInstance,IMAGE_18,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpFE[4] = (HBITMAP)LoadImage(hInstance,IMAGE_19,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hBmpFE[5] = (HBITMAP)LoadImage(hInstance,IMAGE_20,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);

    font[0] = (HBITMAP)LoadImage(hInstance,TEXT("img/font/f0.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    font[1] = (HBITMAP)LoadImage(hInstance,TEXT("img/font/f1.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    font[2] = (HBITMAP)LoadImage(hInstance,TEXT("img/font/f2.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    font[3] = (HBITMAP)LoadImage(hInstance,TEXT("img/font/f3.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    font[4] = (HBITMAP)LoadImage(hInstance,TEXT("img/font/f4.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    font[5] = (HBITMAP)LoadImage(hInstance,TEXT("img/font/f5.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    font[6] = (HBITMAP)LoadImage(hInstance,TEXT("img/font/f6.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    font[7] = (HBITMAP)LoadImage(hInstance,TEXT("img/font/f7.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    font[8] = (HBITMAP)LoadImage(hInstance,TEXT("img/font/f8.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    font[9] = (HBITMAP)LoadImage(hInstance,TEXT("img/font/f9.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);

    hbmpGameover = (HBITMAP)LoadImage(hInstance,TEXT("img/font/gameover.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    hbmpHP = (HBITMAP)LoadImage(hInstance,TEXT("img/heart.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
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
        Collision();
        MoveGun();
        MoveShell();
        MoveDD();
        MoveAircraft();
        MoveCloud();
        InvalidateRect(hWnd , NULL , FALSE);
        if(idealTime > progress)Sleep(idealTime - progress);
        if(progress>= 1000){
            FPS = frames;
            beforeTime =nowTime;
            frames = 0;
        }
        frames++;
        interval--;
        interval2--;
    }
    return TRUE;
}
VOID posInit(){
  gun.y = wnd_rect.bottom-seaHeight-10 +80;

  Node node ={hBmpDD,wnd_rect.right+100,wnd_rect.bottom-130,0.5,5};
  dd.push_back(node);

  Plane ac1 ={hBmpAircraft,wnd_rect.right+600,150,1,1,true};
  Plane ac2 ={hBmpAircraft,wnd_rect.right+615,160,1,1,true};
  Plane ac3 ={hBmpAircraft,wnd_rect.right+630,170,1,1,true};
  Plane ac4 ={hBmpAircraft,wnd_rect.right+645,180,1,1,true};
  Plane ac5 ={hBmpAircraft,wnd_rect.right+660,190,1,1,true};
  aircraft.push_back(ac1);
  aircraft.push_back(ac2);
  aircraft.push_back(ac3);
  aircraft.push_back(ac4);
  aircraft.push_back(ac5);

  Biplane playerAc = {hBmpAircraft2,75,267,0,0,1,0};
  aircraft2 = playerAc;

  srand ((unsigned) time(NULL));
  for(int i=0; i<10;i++){
      cloud[i].x = rand()%wnd_rect.right;
      cloud[i].y = rand()%(wnd_rect.bottom-seaHeight-10-180)+180;
  }
}

VOID Collision(){
  //playerShell
  for(unsigned int i=0; i<shell.size(); i++){
    if(shell[i].y>wnd_rect.bottom-60){
        Effect a = {hBmpWE[0],shell[i].x,wnd_rect.bottom-110,30,50,70};
        wE.push_back(a);
        for(unsigned int j=0;j<dd.size(); j++){
          if(dd[j].x-10<shell[i].x && shell[i].x<dd[j].x+100){
            Effect fx = {hBmpFE[0],shell[i].x-40,shell[i].y-30,50,50,60};
            dd[j].life--;
            score+=10;
            fE.push_back(fx);
            wE.pop_back();
          }
        }
        shell.erase(shell.begin()+i);
      }
  }
  //bomb
  for(unsigned int i=0; i<bomb.size(); i++){
    if(bomb[i].y>290){
      if(bomb[i].x<250){
        Effect fx = {hBmpFE[0],bomb[i].x-20,bomb[i].y-30,80*(4-playerLife)/4,80*(4-playerLife)/4,60};
        fE.push_back(fx);
        bomb.erase(bomb.begin()+i);
        playerLife--;
      }
      else if(bomb[i].x>=300){
        for(unsigned int j=0; j<dd.size(); j++){
          if(bomb[i].x>dd[j].x && bomb[i].x<dd[j].x+70){
            Effect fx = {hBmpFE[0],bomb[i].x-20,bomb[i].y-30,50,50,60};
            fE.push_back(fx);
            bomb.erase(bomb.begin()+i);
            dd[i].life-=5;
            break;
          }else{
            Effect a = {hBmpWE[0],bomb[i].x,wnd_rect.bottom-110,30,50,70};
            wE.push_back(a);
            bomb.erase(bomb.begin()+i);
            break;
          }
        }
      }
    }
  }
  //shell2
  for(unsigned int i=0; i<shell2.size(); i++){
    if(shell2[i].x>shell2[i].expX){
      Effect fx = {hBmpFE[0],shell2[i].x-30,shell2[i].y-30,25,25,60};
      fE.push_back(fx);
      for(unsigned int j=0; j<aircraft.size(); j++){
        if(shell2[i].x>aircraft[j].x-20 && shell2[i].x<aircraft[j].x+20){
          if(shell2[i].y>aircraft[j].y-20 && shell2[i].y<aircraft[j].y+20){
            Effect fx2 = {hBmpFE[0],shell2[i].x-30,shell2[i].y-30,40,40,60};
            fE.push_back(fx2);
            if(playerLife>0){
              aircraft[j].life--;
            }
          }
        }
      }
      shell2.erase(shell2.begin()+i);
    }
  }

  //plane&Plane
  for(unsigned int i=0; i<aircraft.size(); i++){
    if(abs(aircraft[i].y-aircraft2.y)<8 && abs(aircraft[i].x-aircraft2.x)<8){
      Effect fx = {hBmpFE[0],aircraft[i].x-30,aircraft[i].y-10,25,25,60};
      fE.push_back(fx);
      aircraft2.mode = 10;
      aircraft[i].life--;
    }
  }
}

VOID MoveGun(){
    if(mouse.x!=gun.x){
        gunAngle = atan2(-(mouse.y-gun.y),mouse.x-gun.x);
        if(gunAngle>0.785){
            gunAngle = 0.785;
        }else if(gunAngle <-0.17){
            gunAngle = -0.17;
        }
    }
}
VOID MoveShell(){
    double g = 9.8;
    double now = clock();
    for(unsigned int i = 0; i<shell.size(); i++){
        double t = double(now - shell[i].firingTime)/1000.;
        shell[i].x = gun.x + (Shell_MOVE * cos(shell[i].firingAngle)*t)*14.;
        shell[i].y = gun.y + (-Shell_MOVE*sin(shell[i].firingAngle) * t + g*t*t/2.)*10.;
    }
    for(unsigned int i = 0; i<bomb.size(); i++){
        double t = double(now - bomb[i].firingTime)/1000.;
        bomb[i].x = bomb[i].sx + (BOMB_MOVE * cos(bomb[i].firingAngle)*t)*14.;
        bomb[i].y = bomb[i].sy + (g*t*t/2.)*10.;
    }

    for(unsigned int i = 0; i<shell2.size(); i++){
        double t = double(now - shell2[i].firingTime)/1000.;
        shell2[i].x = gun.x-6. + (Shell2_MOVE * cos(shell2[i].firingAngle)*t)*14.;
        shell2[i].y = gun.y-7. + (-Shell2_MOVE*sin(shell2[i].firingAngle) * t )*12.;
    }
}
VOID MoveDD(){

  //dd
  for(unsigned int i=0; i<dd.size(); i++){
    dd[i].x -= dd[i].speed;
    if(dd[i].x<265){
      playerLife=0;

    }
    if(dd[i].life<=0){
      Effect fx = {hBmpFE[0],dd[i].x-20,dd[i].y+40,60,60,60};
      Effect fx2 = {hBmpFE[0],dd[i].x-10,dd[i].y+15,100,100,60};
      Effect fx3 = {hBmpFE[0],dd[i].x+40,dd[i].y+40,70,70,60};
      fE.push_back(fx);
      fE.push_back(fx2);
      fE.push_back(fx3);
      dd.erase(dd.begin()+i);
      score+=100;
    }
  }
  if(interval<=0){
    Node node ={hBmpDD,wnd_rect.right+100,wnd_rect.bottom-130,0.5+score/10000,5};
    dd.push_back(node);
    interval+=2000-score/20;
  }
}

VOID MoveAircraft(){
  for(unsigned int i=0; i<aircraft.size(); i++){
    aircraft[i].x -= aircraft[i].speed;
    if(aircraft[i].life<=0){
      Effect fx = {hBmpFE[0],aircraft[i].x-15,aircraft[i].y,30,30,60};
      fE.push_back(fx);
      aircraft.erase(aircraft.begin()+i);
      score+=30;
      break;
    }else if(aircraft[i].x<=260&&aircraft[i].bomb){
      Bomb bom = {aircraft[i].x,aircraft[i].y+5,aircraft[i].x,aircraft[i].y+5,clock(),3.14};
      bomb.push_back(bom);
      aircraft[i].bomb = false;
    }
    if(aircraft[i].x<-100){
      aircraft.erase(aircraft.begin()+i);
    }
    if(score>1000){
      srand ((unsigned) time(NULL));
      aircraft[i].y+=(rand()*i%3/4.-1./3)*score/1000.;
      aircraft[i].x+=(rand()*i*27%3/4.-1./3)*score/1000.;
    }
    if(aircraft[i].y<60){
      aircraft[i].y=60;
    }else if(aircraft[i].y>230){
      aircraft[i].y=230;
    }
  }
  if(interval2<=0){
    srand ((unsigned) time(NULL));
    int height = rand()%150+60;
    int width = rand()%10+15;
    Plane ac1 ={hBmpAircraft,wnd_rect.right+600,height,1+score/5000.,1,true};
    Plane ac2 ={hBmpAircraft,wnd_rect.right+600+width,height+10,1+score/5000.,1,true};
    Plane ac3 ={hBmpAircraft,wnd_rect.right+600+width*2,height+20,1+score/5000.,1,true};
    Plane ac4 ={hBmpAircraft,wnd_rect.right+600+width*3,height+30,1+score/5000.,1,true};
    Plane ac5 ={hBmpAircraft,wnd_rect.right+600+width*4,height+40,1+score/5000.,1,true};
    aircraft.push_back(ac1);
    aircraft.push_back(ac2);
    aircraft.push_back(ac3);
    aircraft.push_back(ac4);
    aircraft.push_back(ac5);
    interval2+=1300-score/10;
  }
  switch (aircraft2.mode) {
    case 0: aircraft2.moveX=0;
            aircraft2.moveY=0;
            aircraft2.x = 75;
            aircraft2.y = 267;
            break;
    case 1: aircraft2.moveX=-aircraft2.speed;
            aircraft2.moveY=-aircraft2.speed/4;
            if(aircraft2.x<-100){
              aircraft2.mode=2;
            }
            break;
    case 2: aircraft2.image = hBmpAircraft3;
            aircraft2.moveX = aircraft2.speed;
            aircraft2.moveY = -aircraft2.speed/4;
            if(aircraft2.x>250){
              aircraft2.mode = 3;
            }
            break;
    case 3: aircraft2.moveX = aircraft2.speed;
            if(aircraft2.x>wnd_rect.right-80){
              Bomb bom = {aircraft2.x,aircraft2.y+5,aircraft2.x,aircraft2.y+5,clock(),0};
              bomb.push_back(bom);
              aircraft2.mode = 4;
            }
            break;
    case 4: aircraft2.image = hBmpAircraft2;
            aircraft2.moveX = -aircraft2.speed;
            if(aircraft2.y>267){
              aircraft2.moveY = -aircraft2.speed/5;
            }else if(aircraft2.y<267){
              aircraft2.moveY = aircraft2.speed;
            }else{
              aircraft2.moveY = 0;
            }
            if(aircraft2.x<=75){
              aircraft2.mode = 0;
            }
            break;
    case 10:  aircraft2.x=-100;
              aircraft2.moveY=0;
              aircraft2.moveX=0;
              break;
    case 11:  aircraft2.moveY = 0;
              aircraft2.moveX = 0;
              break;

  }
  aircraft2.x += aircraft2.moveX;
  aircraft2.y += aircraft2.moveY;

  if(aircraft2.y<10){
    aircraft2.y = 10;
  }else if(aircraft2.y>wnd_rect.bottom-60){
    Effect a = {hBmpWE[0],aircraft2.x-5,wnd_rect.bottom-110,30,50,70};
    wE.push_back(a);
    aircraft2.mode = 10;
  }

  aircraft2.moveX=0;
  aircraft2.moveY=0;


}

VOID MoveCloud(){
    for(int i=0;i<10;i++){
        cloud[i].x -=0.1+(cloud[i].y-170)/(wnd_rect.bottom-seaHeight-10-120);
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
    PaintFE(hdc,hMemDC);

    PaintScore(hdc,hMemDC);
    //char buf[128];
    //sprintf(buf," mouse  %d %d gunAngle %0.3f Score: %d ",mouse.x,mouse.y,gunAngle,score);
    //TextOut(hdc,200,0,buf,strlen(buf));
}

VOID PaintBackground(HDC hdc,HDC hMemDC){
    SelectObject(hdc , hBrushSky);
    SelectObject(hdc , GetStockObject(NULL_PEN));
    Rectangle(hdc , 0, 0,wnd_rect.right, wnd_rect.bottom);
    SelectObject(hdc , hBrushSea);
    Rectangle(hdc , 0, wnd_rect.bottom - seaHeight,wnd_rect.right, wnd_rect.bottom);
    for(int i=0; i<10;i++){
        SelectObject(hMemDC,cloud[i].image);
        TransparentBlt(hdc,cloud[i].x,cloud[i].y,
          100*(cloud[i].y-150)/(wnd_rect.bottom-seaHeight-10-150),
          50*(cloud[i].y-150)/(wnd_rect.bottom-seaHeight-10-150),
          hMemDC,0,0,100,50,RGB(0,255,0));
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

    SelectObject(hMemDC,aircraft2.image);
    TransparentBlt(hdc,aircraft2.x,aircraft2.y,20,10,hMemDC,0,0,10,5,RGB(0,255,0));

    //enemy

    for(unsigned int i=0; i<dd.size(); i++){
      SelectObject(hMemDC,dd[i].image);
      TransparentBlt(hdc,dd[i].x,dd[i].y,100,100,hMemDC,0,0,100,50,RGB(0,255,0));
    }

    for(unsigned int i=0; i<aircraft.size(); i++){
      SelectObject(hMemDC,aircraft[i].image);
      TransparentBlt(hdc,aircraft[i].x,aircraft[i].y,16,6,hMemDC,0,0,16,6,RGB(0,255,0));
    }


}


VOID PaintShell(HDC hdc)
{
    SelectObject(hdc , GetStockObject(NULL_PEN));
    SelectObject(hdc , GetStockObject(BLACK_BRUSH));
    for(unsigned int i = 0; i<shell.size(); i++){
        Rectangle(hdc , shell[i].x , shell[i].y , shell[i].x + Shell_W, shell[i].y + Shell_H);
    }
    //bomb
    for(unsigned int i = 0; i<bomb.size(); i++){
        Rectangle(hdc , bomb[i].x , bomb[i].y , bomb[i].x + 5, bomb[i].y + 3);
    }
    SelectObject(hdc , hBrushShell2);
    for(unsigned int i = 0; i<shell2.size(); i++){
        Rectangle(hdc , shell2[i].x , shell2[i].y , shell2[i].x + Shell2_W, shell2[i].y + Shell2_H);
    }
}
VOID PaintWE(HDC hdc,HDC hMemDC){
  for(unsigned int i=0; i<wE.size(); i++){
    for(int j=0; j<7;j++){
      if(wE[i].life>=(6-j)*10 && wE[i].life<=(7-j)*10){
        SelectObject(hMemDC,wE[i].image);
        wE[i].image=hBmpWE[j];
      }
    }
    TransparentBlt(hdc,wE[i].x,wE[i].y,wE[i].width,wE[i].height,hMemDC,0,0,30,50,RGB(0,255,0));
    wE[i].life-=1;
    if(wE[i].life<0){
      wE.erase(wE.begin()+i);
    }
  }
}
VOID PaintFE(HDC hdc,HDC hMemDC){
  for(unsigned int i=0; i<fE.size(); i++){
    for(int j=0; j<6;j++){
      if(fE[i].life>=(5-j)*10 && fE[i].life<=(6-j)*10){
        SelectObject(hMemDC,fE[i].image);
        fE[i].image=hBmpFE[j];
      }
    }
    TransparentBlt(hdc,fE[i].x,fE[i].y,fE[i].width,fE[i].height,hMemDC,0,0,50,50,RGB(0,255,0));
    fE[i].life-=1;
    if(fE[i].life<0){
      fE.erase(fE.begin()+i);
    }
  }
}

VOID PaintScore(HDC hdc,HDC hMemDC){
  int n=0;
  for(int i=0; i<6; i++){
    switch (i) {
      case 0: n=score/100000;
              break;
      case 1: n=score%100000/10000;
              break;
      case 2: n=score%10000/1000;
              break;
      case 3: n=score%1000/100;
              break;
      case 4: n=score%100/10;
              break;
      case 5: n=score%10;
              break;
    }
    SelectObject(hMemDC,font[n]);
    TransparentBlt(hdc,30+i*16,10,15,15,hMemDC,0,0,7,7,RGB(0,0,255));
  }
  for(int i=0; i<playerLife; i++){
    SelectObject(hMemDC,hbmpHP);
    TransparentBlt(hdc,wnd_rect.right-60+i*17,10,15,15,hMemDC,0,0,7,7,RGB(0,255,0));
  }
  if(playerLife<=0){
    SelectObject(hMemDC,hbmpGameover);
    TransparentBlt(hdc,wnd_rect.right/2-128,wnd_rect.bottom/2-14,256,28,hMemDC,0,0,64,7,RGB(0,255,0));
    Effect fx = {hBmpFE[0],160-50,280-50,100,100,60};
    fE.push_back(fx);
    aircraft2.mode = 11;
  }
}
