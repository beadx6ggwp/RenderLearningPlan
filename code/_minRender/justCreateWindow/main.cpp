#include <tchar.h>

#include <iostream>

#define UNICODE
#include <Windows.h>

// WindowProc
LRESULT CALLBACK myEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_KEYDOWN: {
            std::cout << "WM_KEYDOWN:\t" << wParam << "\n";
            break;
        }
        case WM_CHAR: {
            std::cout << "WM_CHAR:\t" << wParam << "\n";
            break;
        }
        case WM_KEYUP: {
            std::cout << "WM_KEYUP:\t" << wParam << "\n";
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            RECT rect;
            HDC hdc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &rect);
            DrawText(hdc, TEXT("Hello, Windows!"), (int)-1, &rect,
                     DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main() {
    // https://stackoverflow.com/questions/13977388/error-cannot-convert-const-wchar-t-13-to-lpcstr-aka-const-char-in-assi
    // https://stackoverflow.com/questions/2230758/what-does-lpcwstr-stand-for-and-how-should-it-be-handled-with
    // LPCSTR cls_Name = L"TestWindow"; // unicode問題，透過上面兩篇文章解決

    std::cout << "WindowsApp\n";

    // 綁定此window的資訊，並向系統註冊此程式
    LPCWSTR cls_Name = L"TestWindow";
    WNDCLASS wc = {};
    wc.lpfnWndProc = myEvent;
    wc.hInstance = GetModuleHandle(NULL);  // get this app hInstance
    wc.lpszClassName = cls_Name;
    if (!RegisterClass(&wc)) return -1;

    // 建立window執行的物件
    HWND hwnd = CreateWindow(cls_Name, L"HelloWorld", WS_OVERLAPPEDWINDOW, 0, 0,
                             400, 400, NULL, NULL, wc.hInstance, NULL);

    if (hwnd == NULL) return 0;  //判斷是否成功Create

    // 顯示此window物件
    ShowWindow(hwnd, SW_SHOW);

    // 訊息循環，持續監聽
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        // do something
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}