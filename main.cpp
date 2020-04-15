/***********************************
*文件名称：main.cpp
*文件描述：实现一个横版2D游戏角色移动和背景切换
*最后修改：2018/02/11
************************************/
#include<Windows.h>
#include"tchar.h"
#include<mmsystem.h>
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"Msimg32.lib")


#define WINDOW_CLASS      "KDCSF4545666"
#define WINDOW_TITLE      "DNF"
#define WINDOW_WIDTH   800
#define WINDOW_HEIGHT  600

HDC g_hdc , g_memdc, g_bufdc;
HBITMAP   g_background,g_hHeroBmp[4];  //移动和停止
//1.右跑 2.左跑  3.右站  4.左站

int g_heronum = 0;
int g_heroX=100,g_heroY=400;
int g_dir=0;
int g_bgnum = 0;
int g_ground = 280;
int stdw1 =1050;
int stdw2=430;
int stdw3=900;
int i,j;
HWND hwnd_all = NULL;

int jumpTimes = 0;


BOOL GameInit(HWND hWnd);
VOID GamePaint(HWND hWnd);
BOOL GameClearUp(HWND hWnd);
void Ground();
DWORD WINAPI update(LPVOID lpParameter);//刷新画面
void CreateThread_dirction(int a);//创建调整位置的线程
void SpaceKeydown();//处理同时按下空格
void DirectionKeydown();//处理空格键按下同时按下左右键

LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);

int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nShowCmd)
{

	mciSendString("open .\\res\\4543.mp3 alias bgm", 0, 0, 0);
	mciSendString("play bgm repeat", 0, 0, 0);
	//做一个窗口
	WNDCLASSEX wnd = {0};//对象
	wnd.cbSize = sizeof(WNDCLASSEX);//内存大小
	wnd.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);//背景颜色
	wnd.hIcon =  (HICON)::LoadImage (NULL,"./res/icon.ico",IMAGE_ICON,0,0,
		LR_DEFAULTSIZE| LR_LOADFROMFILE);  //像标
	wnd.cbClsExtra = 0;   //附加内存
	wnd.cbWndExtra = 0;
	wnd.hCursor = NULL;   //鼠标样式
	wnd.hInstance=hInstance;
	wnd.lpszClassName = WINDOW_CLASS;
	wnd.lpszMenuName = NULL;//菜单
	wnd.style = CS_HREDRAW|CS_VREDRAW;//水平和垂直改变窗口大小
	wnd.lpfnWndProc = WndProc;  //窗口的过程函数  函数指针

	if(!RegisterClassEx(&wnd))
		return -1;
	//创建窗口  hWnd 窗口句柄
   HWND hWnd=CreateWindow(WINDOW_CLASS,WINDOW_TITLE,WS_OVERLAPPEDWINDOW,
		300,100,WINDOW_WIDTH,WINDOW_HEIGHT,NULL,NULL,hInstance,NULL);
   

   if(!GameInit(hWnd))
	   return -1;
   
   ShowWindow(hWnd,nShowCmd);  //显示窗口
   UpdateWindow(hWnd);         //更新窗口
   hwnd_all = hWnd;
   CreateThread(NULL, 0, update, NULL, NULL, NULL);

   //闪退 控制台闪退  getch()   getchar()  while()    system("pause")
   //循环。。。等待用户操作
   MSG msg={0};
   BOOL bRet;
   while (bRet=GetMessage(&msg, hWnd, 0, 0))
   {
	   if (bRet == -1)
	   {
		   return -1;
	   }
	   else
	   {
		   TranslateMessage(&msg);
		   DispatchMessage(&msg);
	   }
	   
	  // if (msg.message == WM_QUIT)break;
   }

	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT message,WPARAM wparam,LPARAM lparam)
{
	Ground();
	switch (message)
	{
	case WM_KEYUP:
		break;
	case WM_KEYDOWN:
			//PostQuitMessage(0);   //推送一个退出的消息，结束进程
			switch(wparam)
			{
				SpaceKeydown();
				DirectionKeydown();
			case VK_UP:
				CreateThread_dirction(1);
				break;
			case VK_DOWN:
				CreateThread_dirction(2);
				break;
			case VK_LEFT:
				CreateThread_dirction(3);
				SpaceKeydown();
				break;
			case VK_RIGHT:
				CreateThread_dirction(4);
				SpaceKeydown();
				break;
			case VK_SPACE:
			{
				DirectionKeydown();
				if(jumpTimes<3)
				CreateThread_dirction(5);
				
			}
				
				break;
			}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd,message,wparam,lparam);
}
//窗口创建完毕

//游戏初始化  game Init初始化
BOOL GameInit(HWND hWnd)
{
	//三缓冲体系
	g_hdc=GetDC(hWnd);
	g_memdc = CreateCompatibleDC(g_hdc);
	g_bufdc=CreateCompatibleDC(g_hdc);
	HBITMAP bmp = CreateCompatibleBitmap(g_hdc,WINDOW_WIDTH,WINDOW_HEIGHT);
	SelectObject(g_memdc,bmp);

	//加载图片
	g_background = (HBITMAP)LoadImage(NULL,"./res/bigbg.bmp",IMAGE_BITMAP,2666,
		600,LR_LOADFROMFILE);
	g_hHeroBmp[0]=(HBITMAP)LoadImage(NULL,"./res/right.bmp",IMAGE_BITMAP,524,
		110,LR_LOADFROMFILE);
	g_hHeroBmp[1]=(HBITMAP)LoadImage(NULL,"./res/left.bmp",IMAGE_BITMAP,524,
		110,LR_LOADFROMFILE);
	g_hHeroBmp[2]=(HBITMAP)LoadImage(NULL,"./res/right.bmp",IMAGE_BITMAP,524,
		110,LR_LOADFROMFILE);
	g_hHeroBmp[3]=(HBITMAP)LoadImage(NULL,"./res/left.bmp",IMAGE_BITMAP,524,
		110,LR_LOADFROMFILE);

	return TRUE;
}
VOID GamePaint(HWND hWnd)
{
	SelectObject(g_bufdc,g_background);
	BitBlt(g_memdc,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,g_bufdc,g_bgnum,0,SRCCOPY);

	//贴入图片
	SelectObject(g_bufdc,g_hHeroBmp[g_dir]);// 0 right  1 right
	//BitBlt(g_memdc,200,200,131,110,g_bufdc,0,0,SRCCOPY);
	TransparentBlt(g_memdc,g_heroX,g_heroY,131,110,g_bufdc,g_heronum*131,0,131,110,RGB(0,0,255));//0/绿/蓝
	
	/*if(++g_heronum>=4)
	{

		g_heronum = 0;
	}*/
	//g_heronum = (g_heronum + 1) % 4;
	
	//Sleep(50);
	BitBlt(g_hdc,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,g_memdc,0,0,SRCCOPY);
}
BOOL GameClearUp(HWND hWnd)
{
	return TRUE;
}
void Ground()
{
	if(g_bgnum>=stdw1)
		g_ground=195;
	else if(g_bgnum<=stdw2)
		g_ground=300;
	else if(g_bgnum>=stdw3&&g_bgnum<stdw1)
		g_ground=230;
	else 
		g_ground=260;
}

//刷新窗口
DWORD WINAPI update(LPVOID lpParameter)
{
	while (1)
	{
		GamePaint(hwnd_all);
		Sleep(70);
	}
}

DWORD WINAPI dirction1(LPVOID lpParameter)
{
	if (g_heroY >= g_ground)
		g_heroY -= 8;
	if (g_dir == 1)
		g_heronum = (g_heronum + 3) % 4;
	else
		g_heronum = (g_heronum + 1) % 4;
	return 0;
}
DWORD WINAPI dirction2(LPVOID lpParameter)
{
	if (g_heroY <= 420)
		g_heroY += 8;
	if (g_dir == 1)
		g_heronum = (g_heronum + 3) % 4;
	else
		g_heronum = (g_heronum + 1) % 4;
	return 0;
}
DWORD WINAPI dirction3(LPVOID lpParameter)
{
	g_dir = 1;
	if ((jumpTimes)||!(((g_bgnum <= stdw1) && (g_heroY < 200)) || ((g_bgnum <= stdw2) && (g_heroY < 280)) || ((g_heroY > 190 && g_heroY < 230) && (g_ground >= stdw3 && g_ground < stdw1))))
	{
		g_heroX -= 3;
		g_bgnum -= 12;
	}
	if (g_bgnum <= 0)
	{
		g_bgnum = 0;
		g_heroX = 0;
	}
	g_heronum = (g_heronum + 3) % 4;
	return 0;
}
DWORD WINAPI dirction4(LPVOID lpParameter)
{
	
		g_dir = 0;
		if (g_bgnum <= 2666 - WINDOW_WIDTH)
		{
			g_bgnum += 12;
			g_heroX += 3;
		}
		g_heronum = (g_heronum + 1) % 4;
	
	return 0;
}
DWORD WINAPI dirction5(LPVOID lpParameter)
{
	jumpTimes++;
		//跳跃姿态
		if (g_dir == 1)
			g_heronum = (g_heronum + 3) % 4;
		else
			g_heronum = (g_heronum + 1) % 4;
		/*起跳*/
		int height = 0;
		if (jumpTimes == 1)height = 60;
		if (jumpTimes == 2)height = 80;
		if (jumpTimes == 3)height = 100;
		for (int i = 0; i < height; i++)
		{
			g_heroY -= 1;
			Sleep(i / (height/10));
		}
		
		/*落地*/
		for (int i = 0; i < height; i++)
		{
			g_heroY += 1;
			Sleep(i / (height / 10));
		}
		//恢复姿态
		if (g_dir == 1)
			g_heronum = (g_heronum + 1) % 4;
		else
			g_heronum = (g_heronum + 3) % 4;

		jumpTimes--;
	return 0;
}

void CreateThread_dirction(int a)
{
	switch (a)
	{
	case 1:
		CreateThread(NULL, 0, dirction1, NULL, NULL, NULL);
		break;
	case 2:
		CreateThread(NULL, 0, dirction2, NULL, NULL, NULL);
		break;
	case 3:
		CreateThread(NULL, 0, dirction3, NULL, NULL, NULL);
		break;
	case 4:
		CreateThread(NULL, 0, dirction4, NULL, NULL, NULL);
		break;
	case 5:
		CreateThread(NULL, 0, dirction5, NULL, NULL, NULL);
		break;
	}
	
}

void SpaceKeydown()
{
	SHORT rBet;
	rBet = GetKeyState(VK_SPACE);
	if (rBet < 0 && jumpTimes < 3)
		CreateThread_dirction(5);
}

void DirectionKeydown()
{
	SHORT rBet;
	rBet = GetKeyState(VK_LEFT);
	if (rBet < 0)
		CreateThread_dirction(3);
	rBet = GetKeyState(VK_RIGHT);
	if (rBet < 0)
		CreateThread_dirction(4);
}