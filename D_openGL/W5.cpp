#include "CommonInclude.h"

using namespace std;

const int BOARD_SIZE = 30; // 보드 크기 정의


class Rect
{
public:
    int x1, x2, y1, y2;
};

Rect tempRect; //랩어라운드 처리를 위한 임시 사각형
bool tempRectActive = false;
void DrawBoard(char board[BOARD_SIZE][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            cout << board[i][j] << " ";
        }
        cout << endl;
    }
}

void InitBoard(char board[BOARD_SIZE][BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = '.';
        }
    }
}

void DrawRect(char board[BOARD_SIZE][BOARD_SIZE], Rect* rect) {
    for (int i = rect->y1; i < rect->y2; ++i) 
    {
        for (int j = rect->x1; j < rect->x2; ++j) {
            if (i >= 0 && i < BOARD_SIZE && j >= 0 && j < BOARD_SIZE) { // 범위 체크
                board[i][j] = 'O';
            }
        }
    }
    if (tempRectActive)
    {
        for (int i = tempRect.y1; i < tempRect.y2; ++i)
        {
            for (int j = tempRect.x1; j < tempRect.x2; ++j)
            {
                if (i >= 0 && i < BOARD_SIZE && j >= 0 && j < BOARD_SIZE) // 범위 체크
                {
                    board[i][j] = 'O';
                }
            }
        }
    }
	
}

//명령어별 조건 불확실함
int main() 
{
    char board[BOARD_SIZE][BOARD_SIZE];
    InitBoard(board);
   
    Rect userRect;
    char command;
	int manjuk = 0; 

    // 초기 사각형 좌표 입력 (여기서는 간단히 처리)
    cout << "사각형의 좌상단 좌표 (x1 y1)와 우하단 좌표 (x2 y2)를 입력하세요: ";
    cin >> userRect.x1 >> userRect.y1 >> userRect.x2 >> userRect.y2;
    manjuk = (userRect.x2 - userRect.x1) * (userRect.y2 - userRect.y1);
    cin.ignore();

    while (true) {
        system("cls");
        InitBoard(board); 
        DrawRect(board, &userRect); 
        DrawBoard(board);

        cout << "명령어를 입력하세요 (x/X, y/Y, s/S, i/I, j/J, a/A, m, n, r, q): ";
        cin >> command;
        cin.ignore();
     
        switch (command) {
        case 'x':
            // x축 우측으로 한 칸 이동 (랩어라운드 적용)
            cout << "x축 우측으로 한 칸 이동합니다.\n";
            userRect.x1++;
            userRect.x2++;
            if (userRect.x2 > BOARD_SIZE) {
                int diff = userRect.x2 - BOARD_SIZE;
                if (userRect.x1 >= BOARD_SIZE) {
                    // Entire rectangle is beyond right edge, move it fully to left
                    tempRectActive = false;
                    userRect.x1 -= BOARD_SIZE;
                    userRect.x2 -= BOARD_SIZE;
                }
                else {
                    // Part of rectangle wraps around to left
                    tempRectActive = true;
                    tempRect.x1 = 0;
                    tempRect.x2 = diff;
                    tempRect.y1 = userRect.y1;
                    tempRect.y2 = userRect.y2;
                }
            }
            else {
                tempRectActive = false;
            }
            break;
        case 'X':
            // x축 좌측으로 한 칸 이동 (랩어라운드 적용)
            cout << "x축 좌측으로 한 칸 이동합니다.\n";
            userRect.x1--;
            userRect.x2--;
            if (userRect.x1 < 0) {
                int diff = -userRect.x1;
                if (userRect.x2 <= 0) {
                    // Entire rectangle is beyond left edge, move it fully to right
                    tempRectActive = false;
                    userRect.x1 += BOARD_SIZE;
                    userRect.x2 += BOARD_SIZE;
                }
                else {
                    // Part of rectangle wraps around to right
                    tempRectActive = true;
                    tempRect.x1 = BOARD_SIZE - diff;
                    tempRect.x2 = BOARD_SIZE;
                    tempRect.y1 = userRect.y1;
                    tempRect.y2 = userRect.y2;
                }
            }
            else {
                tempRectActive = false;
            }
            break;
        case 'y':
            // y축 아래쪽으로 한 칸 이동 (랩어라운드 적용)
            cout << "y축 아래쪽으로 한 칸 이동합니다.\n";
            userRect.y1++;
            userRect.y2++;
            if (userRect.y2 > BOARD_SIZE) {
                int diff = userRect.y2 - BOARD_SIZE;
                if (userRect.y1 >= BOARD_SIZE) {
                    // Entire rectangle is beyond bottom edge, move it fully to top
                    tempRectActive = false;
                    userRect.y1 -= BOARD_SIZE;
                    userRect.y2 -= BOARD_SIZE;
                }
                else {
                    // Part of rectangle wraps around to top
                    tempRectActive = true;
                    tempRect.y1 = 0;
                    tempRect.y2 = diff;
                    tempRect.x1 = userRect.x1;
                    tempRect.x2 = userRect.x2;
                }
            }
            else {
                tempRectActive = false;
            }
            break;
        case 'Y':
            // y축 위쪽으로 한 칸 이동 (랩어라운드 적용)
            cout << "y축 위쪽으로 한 칸 이동합니다.\n";
            userRect.y1--;
            userRect.y2--;
            if (userRect.y1 < 0) {
                int diff = -userRect.y1;
                if (userRect.y2 <= 0) {
                    // Entire rectangle is beyond top edge, move it fully to bottom
                    tempRectActive = false;
                    userRect.y1 += BOARD_SIZE;
                    userRect.y2 += BOARD_SIZE;
                }
                else {
                    // Part of rectangle wraps around to bottom
                    tempRectActive = true;
                    tempRect.y1 = BOARD_SIZE - diff;
                    tempRect.y2 = BOARD_SIZE;
                    tempRect.x1 = userRect.x1;
                    tempRect.x2 = userRect.x2;
                }
            }
            else {
                tempRectActive = false;
            }
            break;
        case 's':
            // x와 y 모두 한 칸 축소 (범위 초과 불가)
            cout << "x와 y 모두 한 칸 축소합니다.\n"; 
            if (userRect.x2 - userRect.x1 > 2 && userRect.y2 - userRect.y1 > 2)
            {
                userRect.y2--;
                userRect.x2--;
            }
           
            break;
        case 'S':
            // x와 y 모두 한 칸 확대 (범위 초과 불가)
            cout << "x와 y 모두 한 칸 확대합니다.\n";
            if (userRect.x2 < 30 || userRect.y2 < 30)
            {
                userRect.y2++;
                userRect.x2++;
            }
            

            break;
        case 'i':
            // x축 한 칸 확대
            cout << "x축 한 칸 확대합니다.\n";
            if (userRect.x2 < 30)
            {
                userRect.x2++;
            }
            break;
        case 'I':
            // x축 한 칸 축소
            cout << "x축 한 칸 축소합니다.\n";
            if (userRect.x2 - userRect.x1 > 2)
            {
                userRect.x2--;
            }
            break;
        case 'j':
            // y축 한 칸 확대
            cout << "y축 한 칸 확대합니다.\n";
            if (userRect.y2 < 30)
            {
                userRect.y2++;
            }
            break;
        case 'J':
            // y축 한 칸 축소
            cout << "y축 한 칸 축소합니다.\n";
            if (userRect.y2 - userRect.y1 > 2)
            {
                userRect.y2--;
            }
            break;
        case 'a':
            // x축 한 칸 확대, y축 한 칸 축소
            cout << "x축 한 칸 확대, y축 한 칸 축소합니다.\n";
            if (userRect.x2 < 30 && userRect.y2 - userRect.y1 > 2)
            {
                userRect.x2++;
                userRect.y2--;
            }
            break;
        case 'A':
            // x축 한 칸 축소, y축 한 칸 확대
            cout << "x축 한 칸 축소, y축 한 칸 확대합니다.\n";
            if (userRect.x2 - userRect.x1 > 2 && userRect.y2 < 30)
            {
                userRect.x2--;
                userRect.y2++;
            }
            break;
        case 'm':
        {
            // 사각형의 면적 출력
            manjuk = (userRect.x2 - userRect.x1) * (userRect.y2 - userRect.y1);
            cout << "사각형의 면적 : " << manjuk << endl;
           
          
            break;
        }
        case 'n':
        {
            // 전체 면적에 대한 사각형 면적 비율 출력
			manjuk = (userRect.x2 - userRect.x1) * (userRect.y2 - userRect.y1);
        
			float biryul = manjuk / 900.0f;
			cout << "전체 면적에 대한 사각형 면적 비율 : " << biryul<< endl;
            
            break;
        }
        case 'r':
            // 모든 것을 리셋하고 새 좌표 입력
            cout << "사각형을 리셋하고 새 좌표를 입력받습니다.\n";
            cout << "사각형의 좌상단 좌표 (x1 y1)와 우하단 좌표 (x2 y2)를 입력하세요: ";
            cin >> userRect.x1 >> userRect.y1 >> userRect.x2 >> userRect.y2;
            manjuk = (userRect.x2 - userRect.x1) * (userRect.y2 - userRect.y1);
            cin.ignore();
            break;
        case 'q':
            // 프로그램 종료
            cout << "프로그램을 종료합니다.\n";
            return 0;
        default:
            // 유효하지 않은 명령어
            cout << "유효하지 않은 명령어입니다. 다시 입력하세요.\n";
            break;
        }

        
    }
  
    return 0;

  
}