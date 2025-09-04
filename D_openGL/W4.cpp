#include "CommonInclude.h"

using namespace std;


struct board {
	char val;
	bool condition = false;
};

board _board[5][5];

void initialize();

int main()
{
	char x1 = 0;
	bool toggleHint = false;

	initialize();


	while (true)
	{
		cout << "================\n";
		cout << "  a b c d e\n";
		if (toggleHint)
		{
			for (int i = 0; i < 5; i++)
			{
				cout << i + 1 << " ";
				for (int j = 0; j < 5; j++)
				{
					cout << _board[i][j].val << " ";
				}
				cout << endl;
			}
		}
		else
		{
			for (int i = 0; i < 5; i++)
			{
				cout << i + 1 << " ";
				for (int j = 0; j < 5; j++)
				{
					if (!_board[i][j].condition)
						cout << _board[i][j].val << " ";
					else
						cout << "* ";
				}
				cout << endl;
			}
		}
		cout << "================\n";

		cout << "Command : ";
		cin >> x1;

		if (x1 == 'q')
			break;
		else if (x1 == 'r')
		{
			initialize();
		}
		else if (x1 == 'h')
		{
			toggleHint = !toggleHint;
		}
		else {
			char x2 = ' ';
			int  y1 = 0, y2 = 0;

			cin >> y1 >> x2 >> y2;

			if (x1 < 'a' || x1 > 'e' || x2 < 'a' || x2 > 'e' || y1 < 1 || y1 > 5 || y2 < 1 || y2 > 5)
			{
				cout << "Out of range!" << endl;
				Sleep(800);
				system("cls");
				continue;
			}

			if (_board[y1 - 1][x1 - 'a'].condition == true && _board[y2 - 1][x2 - 'a'].condition == true)

			{

				if ((_board[y1 - 1][x1 - 'a'].val == _board[y2 - 1][x2 - 'a'].val && (x1 != x2 || y1 != y2))
					|| ((_board[y1 - 1][x1 - 'a'].val == '@') || (_board[y2 - 1][x2 - 'a'].val == '@')))
				{
					_board[y1 - 1][x1 - 'a'].condition = false;
					if (_board[y1 - 1][x1 - 'a'].val != '@')
						_board[y1 - 1][x1 - 'a'].val -= 32;
					_board[y2 - 1][x2 - 'a'].condition = false;
					if (_board[y2 - 1][x2 - 'a'].val != '@')
						_board[y2 - 1][x2 - 'a'].val -= 32;
				}
				else
				{
					system("cls");
					cout << "================\n";
					cout << "  a b c d e\n";
					for (int i = 0; i < 5; i++)
					{
						cout << i + 1 << " ";
						for (int j = 0; j < 5; j++)
						{
							if (!_board[i][j].condition)
								cout << _board[i][j].val << " ";
							else if ((i == y1 - 1 && j == x1 - 'a') || (i == y2 - 1 && j == x2 - 'a'))
								cout << _board[i][j].val << " ";
							else
								cout << "* ";
						}
						cout << endl;
					}
					cout << "================\n";
					cout << "Not Matched!" << endl;
					Sleep(800);
					system("cls");
				}
			}
		}
	}
}

void initialize()
{
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			_board[i][j].condition = false;
		}
	}


	for (int i = 0; i < 12; i++)
	{
		while (true)
		{
			int x = rand() % 5;
			int y = rand() % 5;

			if (_board[y][x].condition == false)
			{
				_board[y][x].val = 'a' + i;;
				_board[y][x].condition = true;
				break;
			}
		}
	}

	for (int i = 0; i < 12; i++)
	{
		while (true)
		{
			int x = rand() % 5;
			int y = rand() % 5;

			if (_board[y][x].condition == false)
			{
				_board[y][x].val = 'a' + i;;
				_board[y][x].condition = true;
				break;
			}
		}
	}

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			if (!_board[i][j].condition)
			{
				_board[i][j].val = '@';
				_board[i][j].condition = true;
			}
		}
	}
}