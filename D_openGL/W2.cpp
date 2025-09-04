#include "CommonInclude.h"

using namespace std;

#define ANSI_COLOR_RED "\033[31m"
#define ANSI_COLOR_RESET "\033[0m"

void setConsoleColor(bool isColored) {
#ifdef _WIN32
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (isColored) {
        SetConsoleTextAttribute(consoleHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
    }
    else {
        SetConsoleTextAttribute(consoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
#else
    if (isColored) {
        cout << ANSI_COLOR_RED;
    }
    else {
        cout << ANSI_COLOR_RESET;
    }
#endif
}

// 단어 수 계산 함수
int countWords(const string& inputString, bool useSlashAsSeparator = false) {
    stringstream stream(inputString);
    string word;
    int wordCount = 0;
    if (useSlashAsSeparator) {
        string temp = inputString;
        replace(temp.begin(), temp.end(), '/', ' ');
        stringstream tempStream(temp);
        while (tempStream >> word) {
            wordCount++;
        }
    }
    else {
        while (stream >> word) {
            wordCount++;
        }
    }
    return wordCount;
}



int main() {
    string fileName;
    string lines[10];
    char command = 0;
    bool isCapitalColored = false;
    bool isLineReversed = false;
    bool isInsertMarker = false;
    bool isWordReversed = false;
    bool isCharReplaced = false;
    bool isAllWordUppercase = false; // Renamed for clarity
    bool isSplitAtNumber = false;    // New flag for command 'h'
    char replaceFrom = 0, replaceTo = 0;
    int sortMode = 0; // 0: 원래 순서, 1: 오름차순, 2: 내림차순

    cout << "파일 이름 : ";
    cin >> fileName;
    cin.ignore();
    ifstream inputFile(fileName);
    if (!inputFile.is_open()) {
        cerr << "파일이 없습니다: " << fileName << endl;
        return 1;
    }
    int lineCount = 0;
    for (int i = 0; i < 10; i++) {
        if (!getline(inputFile, lines[i])) break;
        lineCount++;
    }
    inputFile.close();

    string originalLines[10];
    for (int i = 0; i < lineCount; i++) {
        originalLines[i] = lines[i];
    }

    while (true) {
        cout << "ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ" << endl;
        // 정렬 모드 처리
        if (sortMode != 0) {
            vector<pair<int, string>> sortedLines;
            for (int i = 0; i < lineCount; i++) {
                sortedLines.emplace_back(countWords(lines[i], isInsertMarker), lines[i]);
            }
            if (sortMode == 1) {
                sort(sortedLines.begin(), sortedLines.end());
            }
            else {
                sort(sortedLines.rbegin(), sortedLines.rend());
            }
            for (const auto& line : sortedLines) {
                cout << line.second << endl;
            }
        }
        else {
            for (int i = 0; i < lineCount; i++) {
                string currentLine = originalLines[i];
                vector<string> outputLines = { currentLine };

               

                for (string& line : outputLines) 
                {
                    // 문장 역순
                    if (isLineReversed) {
                        reverse(line.begin(), line.end());
                    }
                    // 단어 역순
                    if (isWordReversed) 
                    {
                        stringstream stream(line);
                        string word;
                        vector<string> words;
                        if (isInsertMarker) {
                            string temp = line;
                            replace(temp.begin(), temp.end(), '/', ' ');
                            stringstream tempStream(temp);
                            while (tempStream >> word) {
                                reverse(word.begin(), word.end());
                                words.push_back(word);
                            }
                        }
                        else {
                            while (stream >> word) 
                            {
                                reverse(word.begin(), word.end());
                                words.push_back(word);
                            }
                        }
                        
                        line.clear();
                        for (size_t j = 0; j < words.size(); j++) {
                            line += words[j];
                            if (j < words.size() - 1) {
                                line += (isInsertMarker ? "/" : " ");
                            }
                        }
                    }
                    // 숫자 뒤에 문장 아래로 내리기
                    if (isSplitAtNumber) {
                        string result;
                        for (char c : line) {
                            result += c;
                            if (isdigit(c)) {
                                result += "\n";
                            }
                        }
                        line = result;
                    }
                    // 모든 단어 대문자
                    if (isAllWordUppercase) {
                        string result;
                        stringstream stream(line);
                        string word;
                        bool first = true;
                        while (stream >> word) {
                            if (!first) result += (isInsertMarker ? "/" : " ");
                            transform(word.begin(), word.end(), word.begin(), ::toupper);
                            result += word;
                            first = false;
                        }
                        line = result;
                    }
                    // 문자 교체
                    if (isCharReplaced && replaceFrom != 0) {
                        for (char& c : line) {
                            if (c == replaceFrom) {
                                c = replaceTo;
                            }
                        }
                    }
                    // 공백에 '/' 삽입
                    if (isInsertMarker && !isWordReversed) {
                        string result;
                        stringstream stream(line);
                        string word;
                        bool first = true;
                        while (stream >> word) {
                            if (!first) result += "/";
                            result += word;
                            first = false;
                        }
                        line = result;
                    }
                    // 대문자 시작 단어 색상 변경
                    if (isCapitalColored) {
                        stringstream stream(line);
                        string word;
                        int capitalCount = 0;
                        vector<string> words;
                        char delimiter = isInsertMarker ? '/' : ' ';
                        string temp = line;
                        replace(temp.begin(), temp.end(), '/', ' ');
                        stringstream tempStream(temp);
                        while (tempStream >> word) {
                            if (!word.empty() && isupper(word[0])) {
                                words.push_back(ANSI_COLOR_RED + word + ANSI_COLOR_RESET);
                                capitalCount++;
                            }
                            else {
                                words.push_back(word);
                            }
                        }
                        line.clear();
                        for (size_t j = 0; j < words.size(); j++) {
                            line += words[j];
                            if (j < words.size() - 1) {
                                line += delimiter;
                            }
                        }
                        cout << line << endl;
                        if (capitalCount > 0) {
                            cout << "대문자로 시작하는 단어 수: " << capitalCount << endl;
                        }
                    }
                    else {
                        cout << line << endl;
                    }
                }
            }
        }
        cout << "ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ" << endl;
        cout << "명령어를 입력하세요: ";
        cin >> command;
        cin.ignore();
        if (command == 'q') {
            break;
        }
        else if (command == 'a') {
            isAllWordUppercase = !isAllWordUppercase;
        }
        else if (command == 'c') {
            isCapitalColored = !isCapitalColored;
        }
        else if (command == 'd') {
            isLineReversed = !isLineReversed;
        }
        else if (command == 'e') {
            isInsertMarker = !isInsertMarker;
        }
        else if (command == 'f') {
            isWordReversed = !isWordReversed;
        }
        else if (command == 'g') {
            isCharReplaced = !isCharReplaced;
            if (isCharReplaced) {
                cout << "바꿀 문자와 새 문자를 입력하세요 (예: a b): ";
                cin >> replaceFrom >> replaceTo;
                cin.ignore();
            }
        }
        else if (command == 'b') {
            for (int i = 0; i < lineCount; i++) {
                cout << (i + 1) << "번째 줄 : " << countWords(originalLines[i], isInsertMarker) << "개" << endl;
            }
        }
        else if (command == 'i') {
            sortMode = (sortMode + 1) % 3;
            if (sortMode == 0) {
                cout << "원래 순서로 출력" << endl;
            }
            else if (sortMode == 1) {
                cout << "단어 수 오름차순으로 정렬" << endl;
            }
            else {
                cout << "단어 수 내림차순으로 정렬" << endl;
            }
        }
        else if (command == 'h') {
            isSplitAtNumber = !isSplitAtNumber;
        }
        else if (command == 'j') {
            string searchWord;
            cout << "검색할 단어를 입력하세요: ";
            getline(cin, searchWord);
            int wordCount = 0;
            for (int i = 0; i < lineCount; i++) {
                string currentLine = originalLines[i];
                stringstream stream(currentLine);
                string word;
                vector<string> words;
                char delimiter = isInsertMarker ? '/' : ' ';
                string temp = currentLine;
                replace(temp.begin(), temp.end(), '/', ' ');
                stringstream tempStream(temp);
                while (tempStream >> word) {
                    string wordLower = word;
                    string searchLower = searchWord;
                    transform(wordLower.begin(), wordLower.end(), wordLower.begin(), ::tolower);
                    transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
                    if (wordLower == searchLower) {
                        words.push_back(ANSI_COLOR_RED + word + ANSI_COLOR_RESET);
                        wordCount++;
                    }
                    else {
                        words.push_back(word);
                    }
                }
                currentLine.clear();
                for (size_t j = 0; j < words.size(); j++) {
                    currentLine += words[j];
                    if (j < words.size() - 1) {
                        currentLine += delimiter;
                    }
                }
                cout << currentLine << endl;
            }
            cout << "검색한 단어 : '" << searchWord << "'  " << wordCount << "개" << endl;
        }
    }
    return 0;
}