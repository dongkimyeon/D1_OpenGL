#include <iostream>

using namespace std;

#define ANSI_COLOR_RED "\033[31m"
#define ANSI_COLOR_RESET "\033[0m"


int BOARD_SIZE = 30; // 보드 크기 정의

class Rect {
public:
	int x1, x2, y1, y2;
};

Rect tempRect1, tempRect2; // 랩어라운드 처리를 위한 임시 사각형
bool tempRectActive1 = false;
bool tempRectActive2 = false;

void DrawBoard(char** board) {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			if (board[i][j] == '#')
				cout << ANSI_COLOR_RED << board[i][j] << ANSI_COLOR_RESET << " ";
			else
				cout << board[i][j] << " ";
		}
		cout << endl;
	}
}

void InitBoard(char** board) {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			board[i][j] = '.';
		}
	}
}

void DrawRect(char** board, Rect* rect, char symbol)
{
	for (int i = rect->y1; i < rect->y2; ++i) {
		for (int j = rect->x1; j < rect->x2; ++j) {
			if (i >= 0 && i < BOARD_SIZE && j >= 0 && j < BOARD_SIZE)
			{

				board[i][j] = symbol;
			}
		}
	}
}

void DrawTempRect(char** board, Rect* tempRect, bool tempRectActive, char symbol) {
	if (tempRectActive)
	{
		for (int i = tempRect->y1; i < tempRect->y2; ++i) {
			for (int j = tempRect->x1; j < tempRect->x2; ++j) {
				if (i >= 0 && i < BOARD_SIZE && j >= 0 && j < BOARD_SIZE) {
					board[i][j] = symbol;
				}
			}
		}
	}
}

void DrawCollisionAreaColorChange(char** board, Rect* rect1, Rect* rect2) {
	int startX = max(rect1->x1, rect2->x1);
	int endX = min(rect1->x2, rect2->x2);

	int startY = max(rect1->y1, rect2->y1);
	int endY = min(rect1->y2, rect2->y2);

	if (startX < endX && startY < endY) {
		for (int i = startY; i < endY; ++i) {
			for (int j = startX; j < endX; ++j) {
				if (i >= 0 && i < BOARD_SIZE && j >= 0 && j < BOARD_SIZE) {
					board[i][j] = '#';
				}
			}
		}
	}
}

int main() {
	Rect userRect1, userRect2;
	char command;
	int manjuk1 = 0, manjuk2 = 0;

	// 첫 번째 사각형 좌표 입력
	cout << "첫 번째 사각형의 좌상단 좌표 (x1 y1)와 우하단 좌표 (x2 y2)를 입력하세요: ";
	cin >> userRect1.x1 >> userRect1.y1 >> userRect1.x2 >> userRect1.y2;
	manjuk1 = (userRect1.x2 - userRect1.x1) * (userRect1.y2 - userRect1.y1);

	// 두 번째 사각형 좌표 입력
	cout << "두 번째 사각형의 좌상단 좌표 (x1 y1)와 우하단 좌표 (x2 y2)를 입력하세요: ";
	cin >> userRect2.x1 >> userRect2.y1 >> userRect2.x2 >> userRect2.y2;
	manjuk2 = (userRect2.x2 - userRect2.x1) * (userRect2.y2 - userRect2.y1);
	cin.ignore();

	while (true) {
		char** board = new char* [BOARD_SIZE];
		for (int i = 0; i < BOARD_SIZE; i++) {
			board[i] = new char[BOARD_SIZE];
		}

		InitBoard(board);
		DrawRect(board, &userRect1, 'O'); // 첫 번째 사각형은 'O'로 표시
		DrawRect(board, &userRect2, 'X'); // 두 번째 사각형은 'X'로 표시
		DrawTempRect(board, &tempRect1, tempRectActive1, 'O');
		DrawTempRect(board, &tempRect2, tempRectActive2, 'X');
		DrawCollisionAreaColorChange(board, &userRect1, &userRect2);
		DrawBoard(board);

		cout << "명령어를 입력하세요 (1/2로 사각형 선택 후 x/X, y/Y, s/S, i/I, j/J, a/A, m, r, c, d, q): ";
		cin >> command;
		cin.ignore();

		Rect* selectedRect = nullptr;
		Rect* tempRect = nullptr;
		bool* tempRectActive = nullptr;
		int* manjuk = nullptr;
		char rectSymbol = ' ';
		int rectNum = 0;

		// 사각형 선택
		if (command == '1') {
			selectedRect = &userRect1;
			tempRect = &tempRect1;
			tempRectActive = &tempRectActive1;
			manjuk = &manjuk1;
			rectSymbol = 'O';
			rectNum = 1;
			cout << "첫 번째 사각형 선택됨. 다음 명령어를 입력하세요: ";
			cin >> command;
			cin.ignore();
		}
		else if (command == '2') {
			selectedRect = &userRect2;
			tempRect = &tempRect2;
			tempRectActive = &tempRectActive2;
			manjuk = &manjuk2;
			rectSymbol = 'X';
			rectNum = 2;
			cout << "두 번째 사각형 선택됨. 다음 명령어를 입력하세요: ";
			cin >> command;
			cin.ignore();
		}
		else {
			cout << "사각형을 먼저 선택하세요 (1 또는 2).\n";
			continue;
		}

		switch (command) {
		case 'x':
			cout << rectNum << "번 사각형: x축 우측으로 한 칸 이동합니다.\n";
			selectedRect->x1++;
			selectedRect->x2++;
			if (selectedRect->x2 > BOARD_SIZE)
			{
				int diff = selectedRect->x2 - BOARD_SIZE;
				if (selectedRect->x1 >= BOARD_SIZE) {
					*tempRectActive = false;
					selectedRect->x1 -= BOARD_SIZE;
					selectedRect->x2 -= BOARD_SIZE;
				}
				else {
					*tempRectActive = true;
					tempRect->x1 = 0;
					tempRect->x2 = diff;
					tempRect->y1 = selectedRect->y1;
					tempRect->y2 = selectedRect->y2;
				}
			}
			else {
				*tempRectActive = false;
			}
			break;
		case 'X':
			cout << rectNum << "번 사각형: x축 좌측으로 한 칸 이동합니다.\n";
			selectedRect->x1--;
			selectedRect->x2--;
			if (selectedRect->x1 < 0) {
				int diff = -selectedRect->x1;
				if (selectedRect->x2 <= 0) {
					*tempRectActive = false;
					selectedRect->x1 += BOARD_SIZE;
					selectedRect->x2 += BOARD_SIZE;
				}
				else {
					*tempRectActive = true;
					tempRect->x1 = BOARD_SIZE - diff;
					tempRect->x2 = BOARD_SIZE;
					tempRect->y1 = selectedRect->y1;
					tempRect->y2 = selectedRect->y2;
				}
			}
			else {
				*tempRectActive = false;
			}
			break;
		case 'y':
			cout << rectNum << "번 사각형: y축 아래쪽으로 한 칸 이동합니다.\n";
			selectedRect->y1++;
			selectedRect->y2++;
			if (selectedRect->y2 > BOARD_SIZE) {
				int diff = selectedRect->y2 - BOARD_SIZE;
				if (selectedRect->y1 >= BOARD_SIZE) {
					*tempRectActive = false;
					selectedRect->y1 -= BOARD_SIZE;
					selectedRect->y2 -= BOARD_SIZE;
				}
				else {
					*tempRectActive = true;
					tempRect->y1 = 0;
					tempRect->y2 = diff;
					tempRect->x1 = selectedRect->x1;
					tempRect->x2 = selectedRect->x2;
				}
			}
			else {
				*tempRectActive = false;
			}
			break;
		case 'Y':
			cout << rectNum << "번 사각형: y축 위쪽으로 한 칸 이동합니다.\n";
			selectedRect->y1--;
			selectedRect->y2--;
			if (selectedRect->y1 < 0) 
			{
				int diff = -selectedRect->y1;
				if (selectedRect->y2 <= 0) {
					*tempRectActive = false;
					selectedRect->y1 += BOARD_SIZE;
					selectedRect->y2 += BOARD_SIZE;
				}
				else {
					*tempRectActive = true;
					tempRect->y1 = BOARD_SIZE - diff;
					tempRect->y2 = BOARD_SIZE;
					tempRect->x1 = selectedRect->x1;
					tempRect->x2 = selectedRect->x2;
				}
			}
			else {
				*tempRectActive = false;
			}
			break;
		case 's':
			cout << rectNum << "번 사각형: x와 y 모두 한 칸 축소합니다.\n";
			if (selectedRect->x2 - selectedRect->x1 > 2 && selectedRect->y2 - selectedRect->y1 > 2 && *tempRectActive == false) {
				selectedRect->y2--;
				selectedRect->x2--;
			}
			break;
		case 'S':
			cout << rectNum << "번 사각형: x와 y 모두 한 칸 확대합니다.\n";
			if (selectedRect->x2 < BOARD_SIZE && selectedRect->y2 < BOARD_SIZE && *tempRectActive == false) {
				selectedRect->y2++;
				selectedRect->x2++;
			}
			break;
		case 'i':
			cout << rectNum << "번 사각형: x축 한 칸 확대합니다.\n";
			if (selectedRect->x2 < BOARD_SIZE && *tempRectActive == false) {
				selectedRect->x2++;
			}
			break;
		case 'I':
			cout << rectNum << "번 사각형: x축 한 칸 축소합니다.\n";
			if (selectedRect->x2 - selectedRect->x1 > 2 && *tempRectActive == false) {
				selectedRect->x2--;
			}
			break;
		case 'j':
			cout << rectNum << "번 사각형: y축 한 칸 확대합니다.\n";
			if (selectedRect->y2 < BOARD_SIZE && *tempRectActive == false) {
				selectedRect->y2++;
			}
			break;
		case 'J':
			cout << rectNum << "번 사각형: y축 한 칸 축소합니다.\n";
			if (selectedRect->y2 - selectedRect->y1 > 2 && *tempRectActive == false) {
				selectedRect->y2--;
			}
			break;
		case 'a':
			cout << rectNum << "번 사각형: x축 한 칸 확대, y축 한 칸 축소합니다.\n";
			if (selectedRect->x2 < BOARD_SIZE && selectedRect->y2 - selectedRect->y1 > 2 && *tempRectActive == false) {
				selectedRect->x2++;
				selectedRect->y2--;
			}
			break;
		case 'A':
			cout << rectNum << "번 사각형: x축 한 칸 축소, y축 한 칸 확대합니다.\n";
			if (selectedRect->x2 - selectedRect->x1 > 2 && selectedRect->y2 < BOARD_SIZE && *tempRectActive == false) {
				selectedRect->x2--;
				selectedRect->y2++;
			}
			break;
		case 'm':
			*manjuk = (selectedRect->x2 - selectedRect->x1) * (selectedRect->y2 - selectedRect->y1);
			cout << rectNum << "번 사각형의 면적: " << *manjuk << endl;
			break;
		case 'r':
			cout << rectNum << "번 사각형을 리셋하고 새 좌표를 입력받습니다.\n";
			cout << rectNum << "번 사각형의 좌상단 좌표 (x1 y1)와 우하단 좌표 (x2 y2)를 입력하세요: ";
			*tempRectActive = false;
			cin >> selectedRect->x1 >> selectedRect->y1 >> selectedRect->x2 >> selectedRect->y2;
			*manjuk = (selectedRect->x2 - selectedRect->x1) * (selectedRect->y2 - selectedRect->y1);
			cin.ignore();
			break;
		case 'c':
			//보드칸 늘리기
			if (BOARD_SIZE >= 40) break;

			BOARD_SIZE++;
			{
				if (selectedRect->x2 > BOARD_SIZE)
				{
					int diff = selectedRect->x2 - BOARD_SIZE;
					if (selectedRect->x1 >= BOARD_SIZE) {
						*tempRectActive = false;
						selectedRect->x1 -= BOARD_SIZE;
						selectedRect->x2 -= BOARD_SIZE;
					}
					else {
						*tempRectActive = true;
						tempRect->x1 = 0;
						tempRect->x2 = diff;
						tempRect->y1 = selectedRect->y1;
						tempRect->y2 = selectedRect->y2;
					}
				}


				if (selectedRect->x1 < 0) {
					int diff = -selectedRect->x1;
					if (selectedRect->x2 <= 0) {
						*tempRectActive = false;
						selectedRect->x1 += BOARD_SIZE;
						selectedRect->x2 += BOARD_SIZE;
					}
					else {
						*tempRectActive = true;
						tempRect->x1 = BOARD_SIZE - diff;
						tempRect->x2 = BOARD_SIZE;
						tempRect->y1 = selectedRect->y1;
						tempRect->y2 = selectedRect->y2;
					}
				}

				if (selectedRect->y2 > BOARD_SIZE) {
					int diff = selectedRect->y2 - BOARD_SIZE;
					if (selectedRect->y1 >= BOARD_SIZE) {
						*tempRectActive = false;
						selectedRect->y1 -= BOARD_SIZE;
						selectedRect->y2 -= BOARD_SIZE;
					}
					else {
						*tempRectActive = true;
						tempRect->y1 = 0;
						tempRect->y2 = diff;
						tempRect->x1 = selectedRect->x1;
						tempRect->x2 = selectedRect->x2;
					}
				}

				if (selectedRect->y1 < 0) {
					int diff = -selectedRect->y1;
					if (selectedRect->y2 <= 0) {
						*tempRectActive = false;
						selectedRect->y1 += BOARD_SIZE;
						selectedRect->y2 += BOARD_SIZE;
					}
					else {
						*tempRectActive = true;
						tempRect->y1 = BOARD_SIZE - diff;
						tempRect->y2 = BOARD_SIZE;
						tempRect->x1 = selectedRect->x1;
						tempRect->x2 = selectedRect->x2;
					}
				}
			}
			break;
		case 'd':
			//보드칸 줄이기
			if (BOARD_SIZE <= 10) break;
			BOARD_SIZE--;
			{
				if (selectedRect->x2 > BOARD_SIZE)
				{
					int diff = selectedRect->x2 - BOARD_SIZE;
					if (selectedRect->x1 >= BOARD_SIZE) {
						*tempRectActive = false;
						selectedRect->x1 -= BOARD_SIZE;
						selectedRect->x2 -= BOARD_SIZE;
					}
					else {
						*tempRectActive = true;
						tempRect->x1 = 0;
						tempRect->x2 = diff;
						tempRect->y1 = selectedRect->y1;
						tempRect->y2 = selectedRect->y2;
					}
				}

				if (selectedRect->x1 < 0) {
					int diff = -selectedRect->x1;
					if (selectedRect->x2 <= 0) {
						*tempRectActive = false;
						selectedRect->x1 += BOARD_SIZE;
						selectedRect->x2 += BOARD_SIZE;
					}
					else {
						*tempRectActive = true;
						tempRect->x1 = BOARD_SIZE - diff;
						tempRect->x2 = BOARD_SIZE;
						tempRect->y1 = selectedRect->y1;
						tempRect->y2 = selectedRect->y2;
					}
				}

				if (selectedRect->y2 > BOARD_SIZE) {
					int diff = selectedRect->y2 - BOARD_SIZE;
					if (selectedRect->y1 >= BOARD_SIZE) {
						*tempRectActive = false;
						selectedRect->y1 -= BOARD_SIZE;
						selectedRect->y2 -= BOARD_SIZE;
					}
					else {
						*tempRectActive = true;
						tempRect->y1 = 0;
						tempRect->y2 = diff;
						tempRect->x1 = selectedRect->x1;
						tempRect->x2 = selectedRect->x2;
					}
				}

				if (selectedRect->y1 < 0) {
					int diff = -selectedRect->y1;
					if (selectedRect->y2 <= 0) {
						*tempRectActive = false;
						selectedRect->y1 += BOARD_SIZE;
						selectedRect->y2 += BOARD_SIZE;
					}
					else {
						*tempRectActive = true;
						tempRect->y1 = BOARD_SIZE - diff;
						tempRect->y2 = BOARD_SIZE;
						tempRect->x1 = selectedRect->x1;
						tempRect->x2 = selectedRect->x2;
					}
				}
			}
			break;
		case 'q':
			cout << "프로그램을 종료합니다.\n";
			for (int i = 0; i < BOARD_SIZE; i++) {
				delete[] board[i];
			}
			delete[] board;
			return 0;
		default:
			cout << "유효하지 않은 명령어입니다. 다시 입력하세요.\n";
			break;
		}

		if (selectedRect->x2 <= BOARD_SIZE)
			*tempRectActive = false;

		

	}

	return 0;
}