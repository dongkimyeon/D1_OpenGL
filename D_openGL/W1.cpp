
#include "CommonInclude.h"

using namespace std;

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> randomNum(1, 9);



int firstMatrix[4][4] = {
    {0, 0, 1, 1},
    {2, 1, 0, 1},
    {1, 0, 0, 1},
    {2, 1, 1, 0}
};


int secondMatrix[4][4] = {
    {2, 1, 0, 0},
    {1, 1, 1, 0},
    {0, 2, 1, 1},
    {1, 0, 0, 0}
};

int tempfirstMatrix[4][4]; //e,f 명령어용 임시 행렬
int tempsecondMatrix[4][4]; //e,f 명령어용 임시 행렬

int tempNum = 0;

bool eToggle = false; //e 명령어 토글
bool fToggle = false; //f 명령어 토글
bool multiplyToggle = false; //곱셈 명령어 토글

int GetMax(int matrix[4][4]) {
    int maxVal = matrix[0][0];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (matrix[i][j] > maxVal) {
                maxVal = matrix[i][j];
            }
        }
    }
    return maxVal;
}
int GetMin(int matrix[4][4]) {
    int minVal = matrix[0][0];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (matrix[i][j] < minVal) {
                minVal = matrix[i][j];
            }
        }
    }
    return minVal;
}

int calculateDeterminant(int matrix[4][4], int size) {
    int determinant = 0;
    int subMatrix[4][4];
    if (size == 2) 
    {
        return ((matrix[0][0] * matrix[1][1]) - (matrix[1][0] * matrix[0][1]));
    }
    else 
    {
        for (int x = 0; x < size; x++) 
        {
            int subRow = 0;
            for (int i = 1; i < size; i++) 
            {
                int subCol = 0;
                for (int j = 0; j < size; j++) 
                {
                    if (j == x) 
                    {
                        continue;
                    }
                    subMatrix[subRow][subCol] = matrix[i][j];
                    subCol++;
                }
                subRow++;
            }
            int sign = 1;
            if (x % 2 != 0) 
            {
                sign = -1;
            }
            determinant += sign * matrix[0][x] * calculateDeterminant(subMatrix, size - 1);
        }
    }
    return determinant;
}

void multiplyMatrices(int result[4][4]) {
    int temp[4][4] = { 0 };
    for (int i = 0; i < 4; i++) 
    {
        for (int j = 0; j < 4; j++) 
        {
            for (int k = 0; k < 4; k++) 
            {
                temp[i][j] += firstMatrix[i][k] * secondMatrix[k][j];
            }
        }
    }
    cout << "곱셈 결과:\n";
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << temp[i][j] << " ";
        }
        cout << endl;
    }
}

void addMatrices() {
    int temp[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp[i][j] = tempfirstMatrix[i][j] + tempsecondMatrix[i][j];
        }
    }
    cout << "덧셈 결과:\n";
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << temp[i][j] << " ";
        }
        cout << endl;
    }
}

void subtractMatrices() {
    int temp[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp[i][j] = tempfirstMatrix[i][j] - tempsecondMatrix[i][j];
        }
    }
    cout << "뺄셈 결과:\n";
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << temp[i][j] << " ";
        }
        cout << endl;
    }
}

void transposeMatrices() {
    int transposedFirst[4][4], transposedSecond[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            transposedFirst[j][i] = firstMatrix[i][j];
            transposedSecond[j][i] = secondMatrix[i][j];
        }
    }
    cout << "전치 행렬 1 (행렬식 결과: " << calculateDeterminant(transposedFirst, 4) << "):\n";
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << transposedFirst[i][j] << " ";
        }
        cout << endl;
    }
    cout << "전치 행렬 2 (행렬식 결과: " << calculateDeterminant(transposedSecond, 4) << "):\n";
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << transposedSecond[i][j] << " ";
        }
        cout << endl;
    }
}

void randomizeMatrices() {
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            firstMatrix[i][j] = randomNum(mt);
            secondMatrix[i][j] = randomNum(mt);
            tempfirstMatrix[i][j] = firstMatrix[i][j];
            tempsecondMatrix[i][j] = secondMatrix[i][j];
        }
    }
}


int main() {
    char command;
     
	randomizeMatrices(); //초기 행렬 랜덤화 
    while (true) {
        cout << "ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ\n";
        for (int i = 0; i < 4; i++) {
           
            if (eToggle == false && fToggle == false && multiplyToggle == false) // eToggle과 fToggle이 모두 꺼져 있을 때
            {
                for (int j = 0; j < 4; j++) {
                    cout << firstMatrix[i][j] << " ";
                }
                cout << "\t";
                for (int j = 0; j < 4; j++) {
                    cout << secondMatrix[i][j] << " ";
                }
                cout << endl;
            }
            else // eToggle 또는 fToggle이 켜져 있을 때
            {
                if(multiplyToggle) //곱셈 명령어 토글이 켜져 있을 때
                {
                    for (int j = 0; j < 4; j++) {
                        if (tempfirstMatrix[i][j] % tempNum == 0)
                            cout << tempfirstMatrix[i][j] << " ";
                        else
                            cout << " ";
                    }
                    cout << "\t";
                    for (int j = 0; j < 4; j++) {
                        if (tempsecondMatrix[i][j] % tempNum == 0)
                            cout << tempsecondMatrix[i][j] << " ";
                        else
                            cout << " ";
                    }
                    cout << endl;
                    continue; //곱셈 명령어 토글이 켜져 있을 때는 아래 코드를 실행하지 않음
				}
                for (int j = 0; j < 4; j++) {
                    cout << tempfirstMatrix[i][j] << " ";
                }
                cout << "\t";
                for (int j = 0; j < 4; j++) {
                    cout << tempsecondMatrix[i][j] << " ";
                }
                cout << endl;
            }

            
        }
        cout << "ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ\n";
        cout << "명령어: ";
        cin >> command;
        if (command == 'm') {
            int result[4][4] = { 0 };
            multiplyMatrices(result);
        }
        else if (command == 'a') {
            addMatrices();
        }
        else if (command == 'd') {
            subtractMatrices();
        }
        else if (command == 'r') {
            cout << "행렬 1의 행렬식: " << calculateDeterminant(firstMatrix, 4) << endl;
            cout << "행렬 2의 행렬식: " << calculateDeterminant(secondMatrix, 4) << endl;
        }
        else if (command == 't') {
            transposeMatrices();
        }
        else if (command == 'e') //최소값 찾아서 모든 행렬에서 뺀다
        {
			eToggle = !eToggle;
            if (eToggle)
            {
                if (GetMin(firstMatrix) < GetMin(secondMatrix))
                {
                    int minVal = GetMin(firstMatrix);
                    for (int i = 0; i < 4; i++) {
                        for (int j = 0; j < 4; j++) {
                            tempfirstMatrix[i][j] -= minVal;
                            tempsecondMatrix[i][j] -= minVal;
                        }
                    }
                    cout << "첫 번째 행렬에서 최소값 " << minVal << "을(를) 뺐습니다.\n";
                }
                else
                {
                    int minVal = GetMin(secondMatrix);
                    for (int i = 0; i < 4; i++) {
                        for (int j = 0; j < 4; j++) {
                            tempfirstMatrix[i][j] -= minVal;
                            tempsecondMatrix[i][j] -= minVal;
                        }
                    }
                    cout << "두 번째 행렬에서 최소값 " << minVal << "을(를) 뺐습니다.\n";
                }
            }
            else //원본 복구 
            {
                for (int i = 0; i < 4; ++i)
                {
                    for (int j = 0; j < 4; ++j)
                    {
                        tempfirstMatrix[i][j] = firstMatrix[i][j];
                        tempsecondMatrix[i][j] = secondMatrix[i][j];
                    }
                }
            }
           
        }
		else if (command == 'f') //최대값 찾아서 모든 행렬에서 더한다
        {
			fToggle = !fToggle;
            if (fToggle)
            {
                if (GetMax(firstMatrix) > GetMax(secondMatrix))
                {
                    int maxVal = GetMax(firstMatrix);
                    for (int i = 0; i < 4; i++) {
                        for (int j = 0; j < 4; j++) {
                            tempfirstMatrix[i][j] += maxVal;
                            tempsecondMatrix[i][j] += maxVal;
                        }
                    }
                    cout << "첫 번째 행렬에서 최대값 " << maxVal << "을(를) 더했습니다.\n";
                }
                else
                {
                    int maxVal = GetMax(secondMatrix);
                    for (int i = 0; i < 4; i++) {
                        for (int j = 0; j < 4; j++) {
                            tempfirstMatrix[i][j] += maxVal;
                            tempsecondMatrix[i][j] += maxVal;
                        }
                    }
                    cout << "두 번째 행렬에서 최대값 " << maxVal << "을(를) 더했습니다.\n";
                }
            }
            else
            {
                for (int i = 0; i < 4; ++i)
                {
                    for (int j = 0; j < 4; ++j)
                    {
                        tempfirstMatrix[i][j] = firstMatrix[i][j];
                        tempsecondMatrix[i][j] = secondMatrix[i][j];
                    }
                }
            }
		}
        else if (command == 's') {
            randomizeMatrices();
        }
        else if ('0' <= command && command <= '9')  //숫자의 배수만 출력 
        {
			multiplyToggle = !multiplyToggle;

			tempNum = command - '0'; 

        }
        else if (command == 'q') {
            break;
        }
    }
    return 0;
}
