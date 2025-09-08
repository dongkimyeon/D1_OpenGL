#include "CommonInclude.h"

using namespace std;

#define ANSI_COLOR_RED "\033[31m"

#define ANSI_COLOR_YELLOW "\033[33m"
#define ANSI_COLOR_RESET "\033[0m"

void setConsoleColor(bool isRed) {
#ifdef _WIN32
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (isRed) {
        SetConsoleTextAttribute(consoleHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
    }
    else {
        SetConsoleTextAttribute(consoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
#else
    if (isRed) {
        cout << ANSI_COLOR_RED;
    }
    else {
        cout << ANSI_COLOR_RESET;
    }
#endif
}

struct BoardTile {
    char value;
    bool isHidden = false;
};

BoardTile gameBoard[5][5];
int remainingMoves = 3;

void initializeBoard();

int main() {
    char firstCol = 0;
    bool isHintEnabled = false;

    initializeBoard();

    while (true) {
        cout << "天天天天天天天天天天天\n";
        int remainingTiles = 0;
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 5; col++) {
                if (!gameBoard[row][col].isHidden) {
                    remainingTiles++;
                }
            }
        }

        cout << "Score: " << remainingTiles << "  Coin: " << remainingMoves << "廓\n";
        if(remainingMoves == 0) {
            cout << "今丑 六六" << endl;
            break;
		}
        cout << "  a b c d e\n";
        if (isHintEnabled) {
            for (int row = 0; row < 5; row++) {
                cout << row + 1 << " ";
                for (int col = 0; col < 5; col++) {
                    if (!gameBoard[row][col].isHidden) {
                        cout << ANSI_COLOR_RED << gameBoard[row][col].value << ANSI_COLOR_RESET << " ";
                    }
                    else {
                        cout << gameBoard[row][col].value << " ";
                    }
                }
                cout << endl;
            }
        }
        else {
            for (int row = 0; row < 5; row++) {
                cout << row + 1 << " ";
                for (int col = 0; col < 5; col++) {
                    if (!gameBoard[row][col].isHidden) {
                        cout << ANSI_COLOR_RED << gameBoard[row][col].value << ANSI_COLOR_RESET << " ";
                    }
                    else {
                        cout << "* ";
                    }
                }
                cout << endl;
            }
        }
        cout << "天天天天天天天天天天天\n";

        cout << "暀硉 : ";
        cin >> firstCol;

        if (firstCol == 'q') {
            break;
        }
        else if (firstCol == 'r') {
            initializeBoard();
        }
        else if (firstCol == 'h') {
            isHintEnabled = !isHintEnabled;
        }
        else {
            char secondCol = ' ';
            int firstRow = 0, secondRow = 0;

            cin >> firstRow >> secondCol >> secondRow;
            remainingMoves--;

            if (firstCol < 'a' || firstCol > 'e' || secondCol < 'a' || secondCol > 'e' ||
                firstRow < 1 || firstRow > 5 || secondRow < 1 || secondRow > 5) {
                cout << "Out of range!" << endl;
                Sleep(800);
                system("cls");
                continue;
            }

            if (gameBoard[firstRow - 1][firstCol - 'a'].isHidden &&
                gameBoard[secondRow - 1][secondCol - 'a'].isHidden) {
                if ((gameBoard[firstRow - 1][firstCol - 'a'].value == gameBoard[secondRow - 1][secondCol - 'a'].value &&
                    (firstCol != secondCol || firstRow != secondRow)) ||
                    (gameBoard[firstRow - 1][firstCol - 'a'].value == '@' ||
                        gameBoard[secondRow - 1][secondCol - 'a'].value == '@')) {
                    gameBoard[firstRow - 1][firstCol - 'a'].isHidden = false;
                    if (gameBoard[firstRow - 1][firstCol - 'a'].value != '@') {
                        gameBoard[firstRow - 1][firstCol - 'a'].value = toupper(gameBoard[firstRow - 1][firstCol - 'a'].value);
                    }

                    gameBoard[secondRow - 1][secondCol - 'a'].isHidden = false;
                    if (gameBoard[secondRow - 1][secondCol - 'a'].value != '@') {
                        gameBoard[secondRow - 1][secondCol - 'a'].value = toupper(gameBoard[secondRow - 1][secondCol - 'a'].value);
                    }

                    if (gameBoard[firstRow - 1][firstCol - 'a'].value == '@' ||
                        gameBoard[secondRow - 1][secondCol - 'a'].value == '@') {
                        char targetChar = (gameBoard[firstRow - 1][firstCol - 'a'].value == '@') ?
                            gameBoard[secondRow - 1][secondCol - 'a'].value :
                            gameBoard[firstRow - 1][firstCol - 'a'].value;

                        for (int row = 0; row < 5; row++) {
                            for (int col = 0; col < 5; col++) {
                                if (gameBoard[row][col].isHidden &&
                                    tolower(gameBoard[row][col].value) == tolower(targetChar)) {
                                    gameBoard[row][col].value = toupper(gameBoard[row][col].value);
                                    gameBoard[row][col].isHidden = false;
                                }
                            }
                        }
                    }
                    system("cls");
                }
                else {
                    system("cls");
                    cout << "天天天天天天天天天天天\n";
                    cout << "  a b c d e\n";
                    for (int row = 0; row < 5; row++) {
                        cout << row + 1 << " ";
                        for (int col = 0; col < 5; col++) {
                            if (!gameBoard[row][col].isHidden ||
                                (row == firstRow - 1 && col == firstCol - 'a') ||
                                (row == secondRow - 1 && col == secondCol - 'a')) {
                                cout << ANSI_COLOR_YELLOW << gameBoard[row][col].value << ANSI_COLOR_RESET << " ";
                            }
                            else {
                                cout << "* ";
                            }
                        }
                        cout << endl;
                    }
                    cout << "天天天天天天天天天天天\n";
                    cout << "ぎ溜雖煬~~!!!" << endl;
                    Sleep(800);
                    system("cls");
                }
            }
        }
    }
    return 0;
}

void initializeBoard() {
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            gameBoard[row][col].isHidden = false;
        }
    }

    for (int i = 0; i < 12; i++) {
        while (true) {
            int col = rand() % 5;
            int row = rand() % 5;

            if (!gameBoard[row][col].isHidden) {
                gameBoard[row][col].value = 'a' + i;
                gameBoard[row][col].isHidden = true;
                break;
            }
        }
    }

    for (int i = 0; i < 12; i++) {
        while (true) {
            int col = rand() % 5;
            int row = rand() % 5;

            if (!gameBoard[row][col].isHidden) {
                gameBoard[row][col].value = 'a' + i;
                gameBoard[row][col].isHidden = true;
                break;
            }
        }
    }

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            if (!gameBoard[row][col].isHidden) {
                gameBoard[row][col].value = '@';
                gameBoard[row][col].isHidden = true;
            }
        }
    }
}