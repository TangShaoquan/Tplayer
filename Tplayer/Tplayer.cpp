// VideoProcDemo_OpenCV.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "Tplayer.h"
#include "shobjidl_core.h"
#include <windowsx.h>


#include "opencv2/opencv.hpp" 

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

//自定义全局变量
cv::Mat img;                        //视频帧——>图像
cv::Mat pip_img;                    //画中画视频帧
WCHAR FileNameOfVideo_1[1024];      //视频1的文件路径
WCHAR FileNameOfVideo_2[1024];      //视频2的文件路径用作画中画
cv::VideoCapture VidCap_1;          //视频1读取器1
cv::VideoCapture VidCap_2;          //视频2读取器2


//动画相关
DWORD preTime = GetTickCount();//未使用
DWORD deltaTime; //未使用
float animSpeed = 0.001;
//初始大小
int x = 510;
int y = 0;
int wid = 500;
int hei = 400;

//用来控制循环播放
int totalFrames_1;
int totalFrames_2;
int currentFrames_1 = 0;
int currentFrames_2 = 0;


bool flag = true;//用以标志画中画是被关闭过还是未被关闭过


//播放状态枚举
enum PlayState {
    playing, paused, stopped   //停止
};
PlayState playState_1 = PlayState::stopped;
PlayState playState_2 = PlayState::stopped;

//是否开启画中画
enum PiPState {
    nul , pip
};
PiPState pipState = PiPState::nul;
//播放特效
enum VideoEffect {
    no,edge
};

VideoEffect vidEffect_1 = VideoEffect::no; //视频1效果 ， 默认关闭
VideoEffect vidEffect_2 = VideoEffect::no; //视频1效果 ， 默认关闭

//自定义函数前置声明
bool OpenVideoFile(HWND hWnd, LPWSTR* fn);
std::string WCHAR2String(LPCWSTR pwszSrc);

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TPLAYER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TPLAYER));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}




//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TPLAYER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TPLAYER);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中

    //宽1024 高450
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 1020, 450, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    SetTimer(hWnd, 1, 40, NULL);

    return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //打开视频1
    WCHAR* fn = (WCHAR*)FileNameOfVideo_1;
    bool result;

    //打开视频2
    WCHAR* fn_2 = (WCHAR*)FileNameOfVideo_2;
    bool result_2;

    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 分析菜单选择:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case IDM_OpenVideo1: //打开视频一
            result = OpenVideoFile(hWnd, &fn);
            
            if (result)
            {
                bool opened = VidCap_1.open(WCHAR2String(fn));
                //总帧数
                totalFrames_1 = VidCap_1.get(cv::CAP_PROP_FRAME_COUNT);
                if (opened)
                {
                    VidCap_1 >> img; //获取第一帧图像并显示
                    //激发WM_PAINT时间，让窗口重绘
                    InvalidateRect(hWnd, NULL, false);
                }
                else
                {
                    MessageBox(
                        hWnd,
                        L"视频未能打开",
                        L"错误提示",
                        MB_OK
                    );
                }
            }
            break;
        case IDM_OpenVideo2: //打开视频二
            result_2 = OpenVideoFile(hWnd, &fn_2);
            if (result_2) {
                bool opened = VidCap_2.open(WCHAR2String(fn_2));
                //总帧数
                totalFrames_2 = VidCap_2.get(cv::CAP_PROP_FRAME_COUNT);
                if (opened)
                {
                    VidCap_2 >> pip_img; //获取第一帧图像并显示
                    //激发WM_PAINT时间，让窗口重绘
                    InvalidateRect(hWnd, NULL, false);
                }
                else
                {
                    MessageBox(
                        hWnd,
                        L"视频未能打开",
                        L"错误提示",
                        MB_OK
                    );
                }
            }
        case IDM_Play://视频一 播放
            playState_1 = PlayState::playing;
            break;
        case IDM_Pause://视频一 暂停
            playState_1 = PlayState::paused;
            break;
        case IDM_Stop://视频一 关闭
            playState_1 = PlayState::stopped;
            VidCap_1.set(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES, 0);
            break;
        case IDM_PinPOpen: //开启画中画
            pipState = PiPState::pip;
            flag = true;
            break;
        case IDM_PinPClose: //关闭画中画
            pipState = PiPState::nul;
            flag = false;
            break;
        case IDM_EDGE_Open: //视频一开启边缘图
            vidEffect_1 = VideoEffect::edge;
            break;
        case IDM_EDGE_Close: //视频一关闭边缘图
            vidEffect_1 = VideoEffect::no;
            break;
        case IDM_PinP_Play: //视频二 ： 画中画开始播放
            playState_2 = PlayState::playing;
            break;
        case IDM_PinP_Pause: //视频二：画中画暂停
            playState_2 = PlayState::paused;
            break;
        case IDM_PinP_Stop: //视频二：画中画停止播放
            playState_2 = PlayState::stopped;
            VidCap_2.set(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES, 0);
            break;
        case IDM_PinP_EDGE_Open: //视频二：画中画开启边缘图
            vidEffect_2 = VideoEffect::edge;
            break;
        case IDM_PinP_EDGE_Close: //视频二：画中画关闭边缘图
            vidEffect_2 = VideoEffect::no;
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_TIMER:
        //如果视频1打开 ， 且当前状态为播放
        if (VidCap_1.isOpened() && playState_1 == PlayState::playing)
        {
            //输出
            VidCap_1 >> img;
            //循环播放
            //必须在播放前就让currentFrames+1 ，这样才能保证播到最后一帧的时候才能大于或等于总数。
            if (++currentFrames_1 >= totalFrames_1 - 1) {
                currentFrames_1 = 0;
                VidCap_1.set(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES, currentFrames_1);
            }
            //如果当前帧不空
            if (img.empty() == false)
            {
                //且边缘图打开 ， 则提取边缘进行显示
                if (vidEffect_1 == VideoEffect::edge)
                {
                    //cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
                    cv::Mat edgeY, edgeX;
                    cv::Sobel(img, edgeY, CV_8U, 1, 0);
                    cv::Sobel(img, edgeX, CV_8U, 0, 1);
                    img = edgeX + edgeY;
                }
               
                InvalidateRect(hWnd, NULL, false);
            }
        }

        //视频二 并且 还没有开启画中画的时候 对 播放、暂停、停止 的控制
        if (VidCap_2.isOpened() && playState_2 == PlayState::playing && pipState == PiPState::nul && flag == true) {
            //激发WM_PAINT时间，让窗口重绘
            InvalidateRect(hWnd, NULL, false);
            //输出
            VidCap_2 >> pip_img;
            //循环播放
            if (++currentFrames_2 >= totalFrames_2 - 1) {
                currentFrames_2 = 0;
                VidCap_2.set(cv::CAP_PROP_POS_FRAMES, currentFrames_2);
            }
            //如果当前帧不空
            if (pip_img.empty() == false)
            {
                //且边缘图打开 ， 则提取边缘进行显示
                if (vidEffect_2 == VideoEffect::edge)
                {
                    //cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
                    cv::Mat edgeY, edgeX;
                    cv::Sobel(pip_img, edgeY, CV_8U, 1, 0);
                    cv::Sobel(pip_img, edgeX, CV_8U, 0, 1);
                    pip_img = edgeX + edgeY;
                }
                InvalidateRect(hWnd, NULL, false);
            }
        }

        //视频二 开启画中画时有一个动画过渡到最终的画中画
        if (VidCap_2.isOpened() && pipState == PiPState::pip && playState_2 == PlayState::playing && flag == true) {
            
            //宽大于250时才有动画
            if (wid >= 250) {
                wid -= animSpeed * wid;
            }
            //高大于200时才有动画
            if (hei >= 200) {
                hei -= animSpeed * hei;
            }
            //x的位置不超过500 - 250 = 250
            if (x >= 250) {
                x -= animSpeed * x;
            }
            //y的位置不变

            //输出
            VidCap_2 >> pip_img;
            //循环播放
            if (++currentFrames_2 >= totalFrames_2 - 1) {
                currentFrames_2 = 0;
                VidCap_2.set(cv::CAP_PROP_POS_FRAMES, currentFrames_2);
            }

            //如果当前帧不空
             //激发WM_PAINT时间，让窗口重绘
            InvalidateRect(hWnd, NULL, false);
            if (pip_img.empty() == false)
            {
                //InvalidateRect(hWnd, NULL, false);
                //且边缘图打开 ， 则提取边缘进行显示
                if (vidEffect_2 == VideoEffect::edge)
                {
                    //cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
                    cv::Mat edgeY, edgeX;
                    cv::Sobel(pip_img, edgeY, CV_8U, 1, 0);
                    cv::Sobel(pip_img, edgeX, CV_8U, 0, 1);
                    pip_img = edgeX + edgeY;
                }
                InvalidateRect(hWnd, NULL, false);
            }
        }
        if (VidCap_2.isOpened() && playState_2 == PlayState::playing && pipState == PiPState::nul && flag == false) {
            
            //输出
            VidCap_2 >> pip_img;
            //循环播放
            if (++currentFrames_2 >= totalFrames_2 - 1) {
                currentFrames_2 = 0;
                VidCap_2.set(cv::CAP_PROP_POS_FRAMES, currentFrames_2);
            }

            wid = 500;
            hei = 400;
            x = 510;
            ////宽小于500时才有动画
            //if (wid < 500) {
            //    wid += animSpeed * wid;
            //}
            ////高小于400时才有动画
            //if (hei < 400) {
            //    hei += animSpeed * hei;
            //}
            ////x的位置不超过510
            //if (x < 510) {
            //    x += animSpeed * x;
            //}
            //y的位置不变

            

            //如果当前帧不空
             //激发WM_PAINT时间，让窗口重绘
            InvalidateRect(hWnd, NULL, false);
            if (pip_img.empty() == false)
            {
                //InvalidateRect(hWnd, NULL, false);
                //且边缘图打开 ， 则提取边缘进行显示
                if (vidEffect_2 == VideoEffect::edge)
                {
                    //cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
                    cv::Mat edgeY, edgeX;
                    cv::Sobel(pip_img, edgeY, CV_8U, 1, 0);
                    cv::Sobel(pip_img, edgeX, CV_8U, 0, 1);
                    pip_img = edgeX + edgeY;
                }
                //InvalidateRect(hWnd, NULL, false);
            }
        }
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: 在此处添加使用 hdc 的任何绘图代码...
        // 转一下格式 ,这段可以放外面,
        if (img.rows > 0)
        {
            switch (img.channels())
            {
            case 1:
                cv::cvtColor(img, img, cv::COLOR_GRAY2BGR); // GRAY单通道
                break;
            case 3:
                cv::cvtColor(img, img, cv::COLOR_BGR2BGRA);  // BGR三通道
                break;
            default:
                break;
            }

            int pixelBytes = img.channels() * (img.depth() + 1); // 计算一个像素多少个字节

                                                                 // 制作bitmapinfo(数据头)
            BITMAPINFO bitInfo;
            bitInfo.bmiHeader.biBitCount = 8 * pixelBytes;
            bitInfo.bmiHeader.biWidth = img.cols;
            bitInfo.bmiHeader.biHeight = -img.rows;
            bitInfo.bmiHeader.biPlanes = 1;
            bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bitInfo.bmiHeader.biCompression = BI_RGB;
            bitInfo.bmiHeader.biClrImportant = 0;
            bitInfo.bmiHeader.biClrUsed = 0;
            bitInfo.bmiHeader.biSizeImage = 0;
            bitInfo.bmiHeader.biXPelsPerMeter = 0;
            bitInfo.bmiHeader.biYPelsPerMeter = 0;
            // Mat.data + bitmap数据头 -> MFC

            StretchDIBits(
                hdc,
                0, 0, 500, 400,
                0, 0, img.cols, img.rows,
                img.data,
                &bitInfo,
                DIB_RGB_COLORS,
                SRCCOPY
            );
        }


        if (pip_img.rows > 0)
        {
            switch (pip_img.channels())
            {
            case 1:
                cv::cvtColor(pip_img, pip_img, cv::COLOR_GRAY2BGR); // GRAY单通道
                break;
            case 3:
                cv::cvtColor(pip_img, pip_img, cv::COLOR_BGR2BGRA);  // BGR三通道
                break;
            default:
                break;
            }

            int pixelBytes = pip_img.channels() * (pip_img.depth() + 1); // 计算一个像素多少个字节

                                                                 // 制作bitmapinfo(数据头)
            BITMAPINFO bitInfo;
            bitInfo.bmiHeader.biBitCount = 8 * pixelBytes;
            bitInfo.bmiHeader.biWidth = pip_img.cols;
            bitInfo.bmiHeader.biHeight = -pip_img.rows;
            bitInfo.bmiHeader.biPlanes = 1;
            bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bitInfo.bmiHeader.biCompression = BI_RGB;
            bitInfo.bmiHeader.biClrImportant = 0;
            bitInfo.bmiHeader.biClrUsed = 0;
            bitInfo.bmiHeader.biSizeImage = 0;
            bitInfo.bmiHeader.biXPelsPerMeter = 0;
            bitInfo.bmiHeader.biYPelsPerMeter = 0;
            // Mat.data + bitmap数据头 -> MFC

            StretchDIBits(
                hdc,
                x, y, wid, hei,
                0, 0, pip_img.cols, pip_img.rows,
                pip_img.data,
                &bitInfo,
                DIB_RGB_COLORS,
                SRCCOPY
            );
        }


        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


//打开视频
bool OpenVideoFile(HWND hWnd, LPWSTR* fn)
{
    IFileDialog* pfd = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pfd));

    DWORD dwFlags;
    hr = pfd->GetOptions(&dwFlags);
    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);


    //校验文件名
    COMDLG_FILTERSPEC rgSpec[] =
    {
        { L"MP4", L"*.mp4" },
        { L"AVI", L"*.avi" },
        { L"ALL", L"*.*" },
    };

    HRESULT SetFileTypes(UINT cFileTypes, const COMDLG_FILTERSPEC * rgFilterSpec);
    hr = pfd->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);
    hr = pfd->SetFileTypeIndex(1);

    hr = pfd->Show(hWnd);///显示打开文件对话框

    IShellItem* pShellItem = NULL;
    if (SUCCEEDED(hr))
    {
        hr = pfd->GetResult(&pShellItem);
        hr = pShellItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, fn);//获取文件的完整路径

        return true;
    }

    return false;

}

std::string WCHAR2String(LPCWSTR pwszSrc)
{
    int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);
    if (nLen <= 0)
        return std::string("");

    char* pszDst = new char[nLen];
    if (NULL == pszDst)
        return std::string("");

    WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
    pszDst[nLen - 1] = 0;

    std::string strTmp(pszDst);
    delete[] pszDst;

    return strTmp;
}
//————————————————
//版权声明：本文为CSDN博主「kingkee」的原创文章，遵循CC 4.0 BY - SA版权协议，转载请附上原文出处链接及本声明。
//原文链接：https ://blog.csdn.net/kingkee/java/article/details/98115024
