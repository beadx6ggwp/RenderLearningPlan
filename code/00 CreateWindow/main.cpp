#include <Windows.h>

//LRESULT CALLBACK WindowProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//MSDN

// entry point
//_In_opt_ SAL annotations(向編譯器詳細規範參數型態)
int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    WCHAR cls_Name[] = L"Win32 Class";
    /*
    執行時此處要注意
    RegisterClass():
    MSDN:You must fill the structure with the appropriate class attributes before passing it to the function.

    表示WNDCLASS需要完全初始化才行，如果沒有全部初始化，會導致變數無法正確的分配空間，導致註冊失敗
    要不就是所有變數手動指定預設資料，但有點麻煩，所以需要 WNDCLASS wc = { sizeof(WNDCLASS) };
    再簡化就是WNDCLASS wc = { };

    補充:
    typedef struct rectStruct
    {
        int x;
        int y;
        int width;
        int height;
    } RECT
    初始化所有資料: RECT rect = { 0, 0, 20, 30 };
    初始化所有為預設值(此為int所以為0) RECT rect = { };
    */
    WNDCLASS wc = { };

    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = cls_Name;
    wc.hInstance = hInstance;

    // 向上層註冊
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        cls_Name,               //類別名稱，要和剛剛一致
        L"Win32 test",          //標題文字
        WS_OVERLAPPEDWINDOW,    //視窗外觀樣式
        100,                    //相對於parent的x座標
        200,                    //相對於parent的y座標
        800,                    //視窗寬度
        600,                    //視窗高度
        NULL,                   //parent window，沒的話NULL即可
        NULL,                   //menu
        hInstance,              //當前程式的追蹤
        NULL);                  //附加資料

    if (hwnd == NULL) //判斷是否成功Create
        return 0;

    // 顯示視窗
    ShowWindow(hwnd, SW_SHOW);

    // 更新視窗，通常可有可無，因為只要視窗開著，WM_PAINT就會持續調用並更新視窗
    //UpdateWindow(hwnd);

    // 事件訊息loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

// callback事件處裡 function pointer
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}