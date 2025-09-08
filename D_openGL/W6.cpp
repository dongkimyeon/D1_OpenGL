#include "CommonInclude.h"

#define BOARD_SIZE 30 // 보드 크기 정의

#define ANSI_COLOR_RED "\033[31m"
#define ANSI_COLOR_GREEN "\033[32m"
#define ANSI_COLOR_YELLOW "\033[33m"
#define ANSI_COLOR_RESET "\033[0m"

using namespace std;

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> coord_dist(0, BOARD_SIZE - 1);



enum KanState
{
	WALL, PATH, BACKGROUND
};

struct Player
{
	int x, y;
};
struct Kan
{
	int moveCnt = 0;
	int color = 0; // 0: 없음, 1: 빨강, 2: 초록, 3: 노랑
	KanState state = BACKGROUND;
};

Kan board[BOARD_SIZE][BOARD_SIZE];
Player player;

void CreateWall()
{
	const int numWalls = BOARD_SIZE * BOARD_SIZE * 0.01;
	int placedWalls = 0;

	while (placedWalls < numWalls)
	{
		int x = coord_dist(mt); // 무작위 x 좌표
		int y = coord_dist(mt); // 무작위 y 좌표

		// (0,0)과 (29,29)을 제외
		if ((x == 0 && y == 0) || (x == BOARD_SIZE - 1 && y == BOARD_SIZE - 1))
		{
			continue;
		}

		// 이미 장애물이 있으면 다시 시도
		if (board[y][x].state == WALL)
		{
			continue;
		}

		// 장애물 배치
		board[y][x].state = WALL;
		board[y][x].color = 1; // 빨간색 (장애물 색상)
		placedWalls++;
	}
}

void InitGame()
{
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			board[i][j].moveCnt = 0;
			board[i][j].color = 0;
			board[i][j].state = BACKGROUND;
		}
	}

	CreateWall();
	player.x = 0;
	player.y = 0;

}

void DrawBoard(Player player)
{
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (i == player.y && j == player.x)
			{
				cout << ANSI_COLOR_YELLOW << "P " << ANSI_COLOR_RESET;
			}
			else if (board[i][j].state == WALL)
			{
				cout << ANSI_COLOR_RED << "X " << ANSI_COLOR_RESET;
			}
			else if (board[i][j].state == PATH)
			{
				cout << ANSI_COLOR_GREEN << board[i][j].moveCnt << " " << ANSI_COLOR_RESET;
			}
			else
			{
				cout << "O ";
			}
		}
		cout << endl;
	}
}

void MoveCount(Kan& kan)
{
	kan.moveCnt++;
	kan.state = PATH;
}



int main()
{
	
	InitGame();
	while (true)
	{
		DrawBoard(player);

		cout << "명령어를 입력하세요 (w/a/s/d: 이동, r: 리셋, q: 종료): ";
		char command;
		cin >> command;
		cin.ignore(); // 버퍼 비우기
		
		if (command == 'w')
		{
			if(board[player.y - 1][player.x].state == WALL)
			{
				cout << "장애물이 있는 칸입니다. 이동할 수 없습니다.\n";
				continue;
			}
			MoveCount(board[player.y][player.x]);
			player.y--;
		}
		else if (command == 'a')
		{
			if(board[player.y][player.x - 1].state == WALL)
			{
				cout << "장애물이 있는 칸입니다. 이동할 수 없습니다.\n";
				continue;
			}
			MoveCount(board[player.y][player.x]);
			player.x--;
		}
		else if (command == 's')
		{
			if(board[player.y + 1][player.x].state == WALL)
			{
				cout << "장애물이 있는 칸입니다. 이동할 수 없습니다.\n";
				continue;
			}
			MoveCount(board[player.y][player.x]);
			player.y++;
		}
		else if (command == 'd')
		{
			if(board[player.y][player.x + 1].state == WALL)
			{
				cout << "장애물이 있는 칸입니다. 이동할 수 없습니다.\n";
				continue;
			}
			MoveCount(board[player.y][player.x]);
			player.x++;
		}
		else if (command == 'r')
		{
			InitGame();
			continue;
		}
		else if (command == 'q')
		{
			cout << "프로그램을 종료합니다.\n";
			break;
		}
		else
		{
			cout << "잘못된 명령어입니다. 다시 입력하세요.\n";
			continue;
		}
	}
	return 0;
}
