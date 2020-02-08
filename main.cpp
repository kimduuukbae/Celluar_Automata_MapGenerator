#include <windows.h>
#include <ctime>
#include <iostream>
#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")
HINSTANCE g_hInst;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND handle;
bool** map;
int sizeX = 63;
int sizeY = 65;
int change = 45;
int deathLimit = 4;
int birthLimit = 4;
void makeMapTexture(bool**& v, int w, int h);
int countNeighbours(bool**& tex, int x, int y);
void cellularAutomata(bool**& tex);
int generateCount = 0;

void checkTie(bool**& newmap, bool**& prevmap);
void finishWork(bool**& tex);

bool autoGeneration = false;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	srand(time(NULL));
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;
	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Window Class Name";
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&WndClass);

	hWnd = CreateWindow("Window Class Name", "windows program", (WS_OVERLAPPEDWINDOW), 0, 0, 800, 800, NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HBRUSH h;
	switch (uMsg) {

	case WM_CREATE:
		makeMapTexture(map, sizeX, sizeY);
		generateCount = 1;
		handle = hWnd;
		std::cout << "예약어 : a, r, c : 자동생성, 맵 새로고침, 세대교체" << std::endl;
		break;
	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		h = CreateSolidBrush(RGB(0, 0, 0));
		HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, h);
		for (int i = 0; i < sizeX; ++i) {
			for (int j = 0; j < sizeY; ++j) {
				int x = j * 12;
				int y = i * 12;
				if (map[i][j])
					Rectangle(hDC, x, y, x + 12, y + 12);
			}
		}
		SelectObject(hDC, oldBrush);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_CHAR:
		switch (wParam) {
		case 'a': {
			if (!autoGeneration) {
				generateCount = 1;
				for (int i = 0; i < sizeX; ++i)
					delete[] map[i];
				delete[] map;
				makeMapTexture(map, sizeX, sizeY);
				InvalidateRect(hWnd, NULL, true);
				autoGeneration = true;
				SetTimer(hWnd, 1, 500, NULL);
			}
			break;
		}
		case 'c':
			++generateCount;
			std::cout << "Generate : " << generateCount <<std::endl;
			cellularAutomata(map);
			break;
		case 'r':
			generateCount = 1;
			for (int i = 0; i < sizeX; ++i)
				delete[] map[i];
			delete[] map;
			makeMapTexture(map, sizeX, sizeY);
			std::cout << "Generation" << std::endl;
			break;
		}
		InvalidateRect(hWnd, NULL, true);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		switch (wParam) {
		case 1:
			cellularAutomata(map);
			++generateCount;
			std::cout << "Generate : " << generateCount << std::endl;
			InvalidateRect(hWnd, NULL, true);
			break;
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int countNeighbours(bool**& tex, int x, int y) {
	int count = 0;
	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			int neighbourX = x + i;
			int neighbourY = y + j;
			// i == 0 && j == 0 은 현재 세포의 중앙점에 있다는 뜻, 그땐 넘겨야 함 
			if (i == 0 && j == 0)
				continue;
			
			//In case the index we're looking at it off the edge of the map
			// 지도 가장자리에서 index 보는 경우
			else if (neighbourX < 0 || neighbourY < 0 || neighbourX >= sizeX || neighbourY >= sizeY) 
				count = count + 1;
			
			else if (tex[neighbourX][neighbourY]) 
				count = count + 1;
		}
	}
	return count;
}

void makeMapTexture(bool**& v,int w, int h) {
	v = new bool*[w];
	for (int i = 0; i < w; ++i)
		v[i] = new bool[h];

	for (int x = 0; x < w; ++x) {
		for (int y = 0; y < h; ++y) {
			auto random = rand() % 100;
			if (random < change)
				v[x][y] = true;
			else
				v[x][y] = false;
		}
	}
}

void cellularAutomata(bool**& tex) {
	bool** newMap = nullptr;
	makeMapTexture(newMap, sizeX, sizeY);
	for (int x = 0; x < sizeX; x++) {
		for (int y = 0; y < sizeY; y++) {
			int nbs = countNeighbours(tex, x, y);
			if (tex[x][y]) {
				if (nbs < deathLimit) 
					newMap[x][y] = false;
				else 
					newMap[x][y] = true;
			} 
			// 이웃의 수에 의해 태어나기가 적절한 지 판단
			else {
				if (nbs > birthLimit) 
					newMap[x][y] = true;
				else 
					newMap[x][y] = false;
			}
		}
	}
	checkTie(newMap, tex);

	bool** t = tex;
	tex = newMap;
	for (int i = 0; i < sizeX; ++i)
			delete[] t[i];
	delete[] t;
}

void checkTie(bool**& newmap, bool**& prevmap) {
	for (int x = 0; x < sizeX; ++x) {
		for (int y = 0; y < sizeY; ++y) {
			if (prevmap[x][y] != newmap[x][y]) {
				std::cout << "DisMatch" << std::endl;
				return;
			}
		}
	}
	std::cout << "Match" << std::endl;
	finishWork(newmap);
	if (autoGeneration) {
		KillTimer(handle, 1);
		autoGeneration = false;
		std::cout << "kill" << std::endl;
	}
}

void finishWork(bool**& tex) {
	for (int x = 0; x < sizeY; ++x) {
		tex[0][x] = true;
		tex[sizeX - 1][x] = true;
	}
	for (int y = 0; y < sizeX; ++y) {
		tex[y][0] = true;
		tex[y][sizeY - 1] = true;
	}
}