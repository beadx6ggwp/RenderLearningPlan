#include <Windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//MSDN
bool running = true;

void* buffer_memory;
int buffer_width, buffer_height;
BITMAPINFO buffer_bitmap_info;


//-----------------------------------------------

// Entry point
int CALLBACK WinMain(_In_ HINSTANCE hInstance,
					 _In_opt_ HINSTANCE hPrevInstance,
					 _In_ LPSTR lpCmdLine,
					 _In_ int nCmdShow) {
	WCHAR cls_Name[] = L"Win32 Class";
	WNDCLASS wc = { };

	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = cls_Name;
	wc.hInstance = hInstance;

	RegisterClass(&wc);

	// create
	HWND hwnd = CreateWindow(
		cls_Name,			//���O�W�١A�M���@�P
		L"Win32 test",		//���D��r
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, //�����~�[�˦�
		100,				//�۹��parent��x�y��
		200,				//�۹��parent��y�y��
		800,				//�����e��
		600,				//��������
		NULL,				//parent window�A�S����NULL�Y�i
		NULL,				//menu
		hInstance,			//��e�{�����l��
		NULL);				//���[���
	HDC hdc = GetDC(hwnd);

	if (hwnd == NULL) //�P�_�O�_���\Create
		return 0;

	// ��ܵ��� dxstyle =| WS_VISIBLE �N���γo��
	//ShowWindow(hwnd, SW_SHOW);

	// �ƥ�T��loop
	while (running) {
		// Input
		MSG msg;
		while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Update
		unsigned int* pixel = (unsigned int*)buffer_memory;
		for (size_t y = 0; y < buffer_height; y++)
		{
			for (size_t x = 0; x < buffer_width; x++)
			{
				*pixel++ = 0x000000;
			}
		}
		pixel = (unsigned int*)buffer_memory;
		int y = 10;
		for (size_t x = 0; x < 100; x++)
		{
			*(pixel + buffer_width * (buffer_height - y) + x) = 0xff0000;
		}

		// Render
		StretchDIBits(
			hdc,
			0, 0,
			buffer_width, buffer_height,
			0, 0,
			buffer_width, buffer_height,
			buffer_memory,
			&buffer_bitmap_info, DIB_RGB_COLORS, SRCCOPY);
	}
	return 0;
}

// callback�ƥ�B�� function pointer
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CLOSE:
		case WM_DESTROY: {
			running = false;
			break;
		}
					   // when window change
		case WM_SIZE: {
			RECT rect;
			GetClientRect(hwnd, &rect);// ���]�t���D�C���ϰ�
			buffer_width = rect.right - rect.left;
			buffer_height = rect.bottom - rect.top;

			int buffer_size = buffer_width * buffer_height * sizeof(unsigned int);

			if (buffer_memory) VirtualFree(buffer_memory, 0, MEM_RELEASE);
			buffer_memory = VirtualAlloc(0, buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			buffer_bitmap_info.bmiHeader.biSize = sizeof(buffer_bitmap_info.bmiHeader);
			buffer_bitmap_info.bmiHeader.biWidth = buffer_width;
			buffer_bitmap_info.bmiHeader.biHeight = buffer_height;
			buffer_bitmap_info.bmiHeader.biPlanes = 1;
			buffer_bitmap_info.bmiHeader.biBitCount = 32;
			buffer_bitmap_info.bmiHeader.biCompression = BI_RGB;
			break;
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}