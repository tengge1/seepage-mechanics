#include<Windows.h>
#include <CommCtrl.h>
#include <math.h>
#include "resource.h"

#define WM_CALCULATE WM_USER+9
#define WM_RESET WM_USER+10
#define WM_HELPS WM_USER+11
#define WM_ABOUT WM_USER+12
#define WM_GETDATA WM_USER+13
#define WM_SETDATA WM_USER+14

typedef struct _INPUTINFO//输入数据
{
	float fLength;
	float fRadius;
	float fFirstLength;
	float fRowLength;
	float fInjectPress;
	float fProductPress;
	int nWellNum;
	float fThick;
	float fPermeablity;
	float fViscosity;
	float fFactor;
	float fDensity;
}INPUTINFO,*LPINPUTINFO;

typedef struct _OUTPUTINFO//输出结果
{
	float fFirstTotalPro;
	float fSecondTotalPro;
	float fThirdTotalPro;
	float fFirstOnePro;
	float fSecondOnePro;
	float fThirdOnePro;
}OUTPUTINFO,*LPOUTPUTINFO;

BOOL CALLBACK DialogProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL Calculate(LPINPUTINFO lpInput,LPOUTPUTINFO lpOutput);
BOOL FloatToString(float f,char c[10]);

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrecInstance,LPSTR lpCmdLine,int nShowCmd)//主函数
{
	HWND hwnd;
	MSG msg;
	int screen_width;
	int screen_height;
	RECT rect;

	//创建主窗口并修改图标光标
	hwnd=CreateDialog(hInstance,MAKEINTRESOURCE(IDD_DIALOG),NULL,DialogProc);
	SetClassLong(hwnd,GCL_HICON,(LONG)LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON)));
	SetClassLong(hwnd,GCL_HICONSM,(LONG)LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON)));
	SetClassLong(hwnd,GCL_HCURSOR,(LONG)LoadCursor(hInstance,MAKEINTRESOURCE(IDC_CURSOR)));

	//把对话框移动到屏幕中央
	screen_width=GetSystemMetrics(SM_CXSCREEN);
	screen_height=GetSystemMetrics(SM_CYSCREEN);
	GetWindowRect(hwnd,&rect);
	MoveWindow(hwnd,(screen_width-(rect.right-rect.left))/2,(screen_height-(rect.bottom-rect.top))/2,
		(rect.right-rect.left),(rect.bottom-rect.top),FALSE);

	//显示窗口
	ShowWindow(hwnd,SW_SHOW);

	while(GetMessage(&msg,NULL,NULL,NULL))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

BOOL CALLBACK DialogProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)//对话框消息处理函数
{
	LV_COLUMN lvColumn;
	LV_ITEM lvItem;
	LPINPUTINFO input;
	LPOUTPUTINFO output;
	CHAR lpString[100];
	static HANDLE hEvent;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		//初始化列表控件
		lvColumn.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT;
		lvColumn.fmt=LVCFMT_LEFT;
		lvColumn.cx=80;
		lvColumn.pszText="井排";
		ListView_SetExtendedListViewStyle(GetDlgItem(hWnd,IDC_LIST_RESULT),
			ListView_GetExtendedListViewStyle(GetDlgItem(hWnd,IDC_LIST_RESULT))|LVS_EX_FULLROWSELECT);
		ListView_InsertColumn(GetDlgItem(hWnd,IDC_LIST_RESULT),0,&lvColumn);
		lvColumn.pszText="井排总产量";
		ListView_InsertColumn(GetDlgItem(hWnd,IDC_LIST_RESULT),1,&lvColumn);
		lvColumn.pszText="单井产量";
		ListView_InsertColumn(GetDlgItem(hWnd,IDC_LIST_RESULT),1,&lvColumn);
		lvItem.mask=LVIF_TEXT;
		lvItem.iItem=0;
		lvItem.iSubItem=0;
		lvItem.pszText="0";
		ListView_InsertItem(GetDlgItem(hWnd,IDC_LIST_RESULT),&lvItem);
		lvItem.iItem=1;
		lvItem.pszText="1";
		ListView_InsertItem(GetDlgItem(hWnd,IDC_LIST_RESULT),&lvItem);
		lvItem.iItem=2;
		lvItem.pszText="2";
		ListView_InsertItem(GetDlgItem(hWnd,IDC_LIST_RESULT),&lvItem);
		//在编辑框内填入预置数据
		SendMessage(hWnd,WM_RESET,NULL,NULL);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_CALCULATE://计算按钮
			SendMessage(hWnd,WM_CALCULATE,NULL,NULL);
			break;
		case IDC_BUTTON_RESET://重置按钮
			SendMessage(hWnd,WM_RESET,NULL,NULL);
			break;
		case IDC_BUTTON_HELP://帮助按钮
			SendMessage(hWnd,WM_HELPS,NULL,NULL);
			break;
		case IDC_BUTTON_ABOUT://关于按钮
			SendMessage(hWnd,WM_ABOUT,NULL,NULL);
			break;
		default:
			break;
		}
		break;
	case WM_CALCULATE://计算消息
		input=(LPINPUTINFO)malloc(sizeof(INPUTINFO));
		output=(LPOUTPUTINFO)malloc(sizeof(OUTPUTINFO));
		hEvent=CreateEvent(NULL,FALSE,FALSE,TEXT("EventDone"));
		ResetEvent(hEvent);
		SendMessage(hWnd,WM_GETDATA,NULL,(LPARAM)input);
		WaitForSingleObject(hEvent,-1);
		Calculate(input,output);
		ResetEvent(hEvent);
		SendMessage(hWnd,WM_SETDATA,NULL,(LPARAM)output);
		WaitForSingleObject(hEvent,-1);
		free(input);
		free(output);
		CloseHandle(hEvent);
		return TRUE;
	case WM_RESET://重置消息
		SetDlgItemText(hWnd,IDC_EDIT_LENGTH,TEXT("420"));
		SetDlgItemText(hWnd,IDC_EDIT_RADIUS,TEXT("10"));
		SetDlgItemText(hWnd,IDC_EDIT_FIRSTLENGTH,TEXT("1100"));
		SetDlgItemText(hWnd,IDC_EDIT_ROWLENGTH,TEXT("600"));
		SetDlgItemText(hWnd,IDC_EDIT_INJECTPRESS,TEXT("19.5"));
		SetDlgItemText(hWnd,IDC_EDIT_PRODUCTPRESS,TEXT("7.5"));
		SetDlgItemText(hWnd,IDC_EDIT_WELLNUM,TEXT("16"));
		SetDlgItemText(hWnd,IDC_EDIT_THICK,TEXT("16"));
		SetDlgItemText(hWnd,IDC_EDIT_PERMEABILITY,TEXT("0.5"));
		SetDlgItemText(hWnd,IDC_EDIT_VISCOSITY,TEXT("9"));
		SetDlgItemText(hWnd,IDC_EDIT_FACTOR,TEXT("1.12"));
		SetDlgItemText(hWnd,IDC_EDIT_DENSITY,TEXT("0.85"));
		SendMessage(hWnd,WM_CALCULATE,NULL,NULL);
		return TRUE;
	case WM_HELPS://帮助消息
		MessageBox(NULL,TEXT("输入数据后点击“计算”开始计算"),TEXT("软件关于"),MB_OK|MB_ICONINFORMATION);
		return TRUE;
	case WM_ABOUT://关于消息
		MessageBox(NULL,TEXT("石工09-7班\n李腾"),TEXT("软件关于"),MB_OK|MB_ICONINFORMATION);
		return TRUE;
	case WM_GETDATA://获取数据
		input=(LPINPUTINFO)lParam;
		memset(lpString,0,100);
		GetDlgItemText(hWnd,IDC_EDIT_LENGTH,lpString,100);
		input->fLength=(float)atof(lpString);
		memset(lpString,0,100);
		GetDlgItemText(hWnd,IDC_EDIT_RADIUS,lpString,100);
		input->fRadius=(float)atof(lpString);
		memset(lpString,0,100);
		GetDlgItemText(hWnd,IDC_EDIT_FIRSTLENGTH,lpString,100);
		input->fFirstLength=(float)atof(lpString);
		memset(lpString,0,100);
		GetDlgItemText(hWnd,IDC_EDIT_ROWLENGTH,lpString,100);
		input->fRowLength=(float)atof(lpString);
		memset(lpString,0,100);
		GetDlgItemText(hWnd,IDC_EDIT_INJECTPRESS,lpString,100);
		input->fInjectPress=(float)atof(lpString);
		memset(lpString,0,100);
		GetDlgItemText(hWnd,IDC_EDIT_PRODUCTPRESS,lpString,100);
		input->fProductPress=(float)atof(lpString);
		memset(lpString,0,100);
		GetDlgItemText(hWnd,IDC_EDIT_WELLNUM,lpString,100);
		input->nWellNum=atoi(lpString);
		memset(lpString,0,100);
		GetDlgItemText(hWnd,IDC_EDIT_THICK,lpString,100);
		input->fThick=(float)atof(lpString);
		memset(lpString,0,100);
		GetDlgItemText(hWnd,IDC_EDIT_PERMEABILITY,lpString,100);
		input->fPermeablity=(float)atof(lpString);
		memset(lpString,0,100);
		GetDlgItemText(hWnd,IDC_EDIT_VISCOSITY,lpString,100);
		input->fViscosity=(float)atof(lpString);
		memset(lpString,0,100);
		GetDlgItemText(hWnd,IDC_EDIT_FACTOR,lpString,100);
		input->fFactor=(float)atof(lpString);
		memset(lpString,0,100);
		GetDlgItemText(hWnd,IDC_EDIT_DENSITY,lpString,100);
		input->fDensity=(float)atof(lpString);
		SetEvent(hEvent);
		return TRUE;
	case WM_SETDATA://输出数据
		output=(LPOUTPUTINFO)lParam;
		ListView_SetItemText(GetDlgItem(hWnd,IDC_LIST_RESULT),0,0,TEXT("第一排井(t/d)"));
		FloatToString(output->fFirstTotalPro,lpString);
		ListView_SetItemText(GetDlgItem(hWnd,IDC_LIST_RESULT),0,1,lpString);
		FloatToString(output->fFirstOnePro,lpString);
		ListView_SetItemText(GetDlgItem(hWnd,IDC_LIST_RESULT),0,2,lpString);
		ListView_SetItemText(GetDlgItem(hWnd,IDC_LIST_RESULT),1,0,TEXT("第二排井(t/d)"));
		FloatToString(output->fSecondTotalPro,lpString);
		ListView_SetItemText(GetDlgItem(hWnd,IDC_LIST_RESULT),1,1,lpString);
		FloatToString(output->fSecondOnePro,lpString);
		ListView_SetItemText(GetDlgItem(hWnd,IDC_LIST_RESULT),1,2,lpString);
		ListView_SetItemText(GetDlgItem(hWnd,IDC_LIST_RESULT),2,0,TEXT("第三排井(t/d)"));
		FloatToString(output->fThirdTotalPro,lpString);
		ListView_SetItemText(GetDlgItem(hWnd,IDC_LIST_RESULT),2,1,lpString);
		FloatToString(output->fThirdOnePro,lpString);
		ListView_SetItemText(GetDlgItem(hWnd,IDC_LIST_RESULT),2,2,lpString);
		SetEvent(hEvent);
		return TRUE;
	case WM_CLOSE://关闭窗口
		DestroyWindow(hWnd);
		return TRUE;
	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

BOOL FloatToString(float f,char c[10])//浮点数转字符串
{
	int dec;
	int sign;
	char *temp;
	memset(c,0,10);
	temp=fcvt(f,7,&dec,&sign);
	if(sign!=0) strcat(c,"-");
	if(dec==0) strcat(c,"0");
	strncat(c,temp,dec);
	strcat(c,".");
	temp+=dec;
	strncat(c,temp,8-dec);
	return TRUE;
}

BOOL Calculate(LPINPUTINFO lpInput,LPOUTPUTINFO lpOutput)//计算函数
{
	if(lpOutput==NULL) return FALSE;
	float mu=lpInput->fViscosity;
	float B=lpInput->fLength*100*lpInput->nWellNum;
	float k=lpInput->fPermeablity;
	float h=lpInput->fThick*100;
	float L1=lpInput->fFirstLength*100;
	float L2=lpInput->fRowLength*100;
	float L3=L2;
	int n=lpInput->nWellNum;
	float a=lpInput->fLength/2*100;
	float Rw=lpInput->fRadius;
	float Pe=lpInput->fInjectPress*10;
	float Pw=lpInput->fProductPress*10;
	float f[3][4];

	float Rou1=mu*L1/B/k/h;
	float Rou2=mu*L2/B/k/h;
	float Rou3=mu*L3/B/k/h;

	float Rin1=mu/n/2/3.1415926/k/h*log(a/3.1415926/Rw);
	float Rin2=mu/n/2/3.1415926/k/h*log(a/3.1415926/Rw);
	float Rin3=mu/n/2/3.1415926/k/h*log(a/3.1415926/Rw);

	f[0][0]=Rou1+Rin1;
	f[0][1]=Rou1;
	f[0][2]=Rou1;
	f[0][3]=Pe-Pw;
	f[1][0]=-Rin1;
	f[1][1]=Rou2+Rin2;
	f[1][2]=Rou2;
	f[1][3]=0;
	f[2][0]=0;
	f[2][1]=-Rin2;
	f[2][2]=Rou3+Rin3;
	f[2][3]=0;

	for (int i=3;i>=0;i--)
	{
		f[0][i]=f[0][i]/f[0][0];
	}

	for (int i=3;i>=0;i--)
	{
		f[1][i]=f[0][i]*(-f[1][0])+f[1][i];
		f[2][i]=f[0][i]*(-f[2][0])+f[2][i];
	}

	for (int i=2;i>=0;i--)
	{
		f[1][i+1]=f[1][i+1]/f[1][1];
	}

	for (int i=2;i>=0;i--)
	{
		f[2][i+1]=f[1][i+1]*(-f[2][1])+f[2][i+1];
	}
	f[2][3]=f[2][3]/f[2][2];
	f[2][2]=1;
	f[1][3]=f[2][3]*(-f[1][2])+f[1][3];
	f[1][2]=f[2][2]*(-f[1][2])+f[1][2];
	f[0][3]=f[2][3]*(-f[0][2])+f[0][3];
	f[0][2]=f[2][2]*(-f[0][2])+f[0][2];

	f[0][3]=f[1][3]*(-f[0][1])+f[0][3];
	f[0][1]=f[1][1]*(-f[0][1])+f[0][1];


	lpOutput->fFirstTotalPro=f[0][3]*lpInput->fDensity*3600*24/lpInput->fFactor/1000/1000;
	lpOutput->fFirstOnePro=lpOutput->fFirstTotalPro/lpInput->nWellNum;

	lpOutput->fSecondTotalPro=f[1][3]*lpInput->fDensity*3600*24/lpInput->fFactor/1000/1000;
	lpOutput->fSecondOnePro=lpOutput->fSecondTotalPro/lpInput->nWellNum;

	lpOutput->fThirdTotalPro=f[2][3]*lpInput->fDensity*3600*24/lpInput->fFactor/1000/1000;
		lpOutput->fThirdOnePro=lpOutput->fThirdTotalPro/16;
	return TRUE;
}
