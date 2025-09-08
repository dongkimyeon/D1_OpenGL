#include "CommonInclude.h"

using namespace std;

struct Vector3D {
    int coordX, coordY, coordZ;
    double calculateDistance() const {
        return sqrt(coordX * coordX + coordY * coordY + coordZ * coordZ);
    }

    bool isEmpty = true;
};

void CopyArray(Vector3D* source, Vector3D* destination, int size) {
    for (int i = 0; i < size; i++) {
        destination[i] = source[i];
    }
}

Vector3D pointArray[11];
Vector3D sortedArray[11];

int pointCount = 0;

int main() {
    char userCommand = 0;
    bool isSortedView = false;

    int rearIndex = 0;
    int frontIndex = 0;

    while (true) {
        if (isSortedView) {
            cout << "--------------------------" << endl;
            for (int i = 9; i >= 0; i--) {
                if (sortedArray[i].isEmpty)
                    cout << "List [" << i << "]: " << endl;
                else
                    cout << "List [" << i << "]: (" << sortedArray[i].coordX << ", "
                    << sortedArray[i].coordY << ", " << sortedArray[i].coordZ << ")" << endl;
            }
            cout << "--------------------------" << endl;
        }
        else {
            cout << "--------------------------" << endl;
            for (int i = 9; i >= 0; i--) {
                if (pointArray[i].isEmpty)
                    cout << "List [" << i << "]: " << endl;
                else
                    cout << "List [" << i << "]: (" << pointArray[i].coordX << ", "
                    << pointArray[i].coordY << ", " << pointArray[i].coordZ << ")" << endl;
            }
            cout << "--------------------------" << endl;
        }

        cout << "Command : ";
        cin >> userCommand;

        if (userCommand == '+') {
            if (frontIndex - rearIndex == 9) {
                cout << "List full" << endl;
                continue;
            }

            int inputX, inputY, inputZ;
            cin >> inputX >> inputY >> inputZ;

            if (frontIndex == 0) {
                for (int i = 9; i > frontIndex; i--) {
                    pointArray[i] = pointArray[i - 1];
                }
                pointArray[frontIndex].coordX = inputX;
                pointArray[frontIndex].coordY = inputY;
                pointArray[frontIndex].coordZ = inputZ;
                pointArray[frontIndex].isEmpty = false;
                rearIndex++;
                pointCount++;
            }
            else {
                frontIndex--;
                pointArray[frontIndex].coordX = inputX;
                pointArray[frontIndex].coordY = inputY;
                pointArray[frontIndex].coordZ = inputZ;
                pointArray[frontIndex].isEmpty = false;
                pointCount++;
            }

            CopyArray(pointArray, sortedArray, 10);
            if (isSortedView)
            {
                for (int i = 0; i < 9; i++)
                {
                    for (int j = i + 1; j < 10; j++)
                    {
                        if (sortedArray[i].isEmpty || sortedArray[j].isEmpty)
                            continue;
                        if (sortedArray[i].calculateDistance() > sortedArray[j].calculateDistance())
                        {
                            Vector3D temp = sortedArray[i];
                            sortedArray[i] = sortedArray[j];
                            sortedArray[j] = temp;
                        }
                    }
                }
            }
        }
        else if (userCommand == '-') {
            if (rearIndex == frontIndex) {
                cout << "List empty" << endl;
                continue;
            }

            pointArray[frontIndex].isEmpty = true;
            frontIndex++;
            pointCount--;
            CopyArray(pointArray, sortedArray, 10);
            if (isSortedView)
            {
                for (int i = 0; i < 9; i++)
                {
                    for (int j = i + 1; j < 10; j++)
                    {
                        if (sortedArray[i].isEmpty || sortedArray[j].isEmpty)
                            continue;
                        if (sortedArray[i].calculateDistance() > sortedArray[j].calculateDistance())
                        {
                            Vector3D temp = sortedArray[i];
                            sortedArray[i] = sortedArray[j];
                            sortedArray[j] = temp;
                        }
                    }
                }
            }
        }
        else if (userCommand == 'e') 
        {
            if (frontIndex - rearIndex == 9) {
                cout << "List full" << endl;
                continue;
            }

            int inputX, inputY, inputZ;
            cin >> inputX >> inputY >> inputZ;
            pointArray[rearIndex].coordX = inputX;
            pointArray[rearIndex].coordY = inputY;
            pointArray[rearIndex].coordZ = inputZ;
            pointArray[rearIndex].isEmpty = false;
            rearIndex++;
            pointCount++;
            CopyArray(pointArray, sortedArray, 10);
            if (isSortedView)
            {
                for (int i = 0; i < 9; i++)
                {
                    for (int j = i + 1; j < 10; j++)
                    {
                        if (sortedArray[i].isEmpty || sortedArray[j].isEmpty)
                            continue;
                        if (sortedArray[i].calculateDistance() > sortedArray[j].calculateDistance())
                        {
                            Vector3D temp = sortedArray[i];
                            sortedArray[i] = sortedArray[j];
                            sortedArray[j] = temp;
                        }
                    }
                }
            }
        }
        else if (userCommand == 'd') {
            if (rearIndex == frontIndex) {
                cout << "List empty" << endl;
                continue;
            }

            pointArray[rearIndex - 1].isEmpty = true;
            rearIndex--;
            pointCount--;
            CopyArray(pointArray, sortedArray, 10);
            if (isSortedView)
            {
                for (int i = 0; i < 9; i++)
                {
                    for (int j = i + 1; j < 10; j++)
                    {
                        if (sortedArray[i].isEmpty || sortedArray[j].isEmpty)
                            continue;
                        if (sortedArray[i].calculateDistance() > sortedArray[j].calculateDistance())
                        {
                            Vector3D temp = sortedArray[i];
                            sortedArray[i] = sortedArray[j];
                            sortedArray[j] = temp;
                        }
                    }
                }
            }
        }
        else if (userCommand == 'c') {
            for (int i = 0; i < 10; i++) {
                pointArray[i].isEmpty = true;
            }
            frontIndex = 0;
            rearIndex = 0;
            pointCount = 0;
            CopyArray(pointArray, sortedArray, 10);
            if (isSortedView)
            {
                for (int i = 0; i < 9; i++)
                {
                    for (int j = i + 1; j < 10; j++)
                    {
                        if (sortedArray[i].isEmpty || sortedArray[j].isEmpty)
                            continue;
                        if (sortedArray[i].calculateDistance() > sortedArray[j].calculateDistance())
                        {
                            Vector3D temp = sortedArray[i];
                            sortedArray[i] = sortedArray[j];
                            sortedArray[j] = temp;
                        }
                    }
                }
            }
        }
        else if (userCommand == 'a') {
            cout << "Count: " << pointCount << endl;
        }
        else if (userCommand == 'b') 
        {
            
            if (pointCount == 0) {
                cout << "List empty" << endl;
                continue;
            }
            // Save the element at index 0
            Vector3D temp = pointArray[0];
            // Shift elements left (1->0, 2->1, ..., 9->8)
            for (int i = 0; i < 9; i++) {
                pointArray[i] = pointArray[i + 1];
            }
            // Place element from index 0 at index 9
            pointArray[9] = temp;
            // Adjust head and tail to follow the rotation
            frontIndex = (frontIndex == 0) ? 9 : frontIndex - 1;
            rearIndex = (rearIndex == 0) ? 9 : rearIndex - 1;

            //CopyArray(pointArray, sortedArray, 10);
        }
        else if (userCommand == 'f') 
        {
            isSortedView = !isSortedView;
            if (frontIndex == rearIndex) {
                cout << "List empty" << endl;
                continue;
            }

			CopyArray(pointArray, sortedArray, 10);

            for (int i = 0; i < 9; i++) 
            {
                for (int j = i + 1; j < 10; j++) 
                {
                    if (sortedArray[i].isEmpty || sortedArray[j].isEmpty)
                        continue;
                    if (sortedArray[i].calculateDistance() > sortedArray[j].calculateDistance()) 
                    {
                        Vector3D temp = sortedArray[i];
                        sortedArray[i] = sortedArray[j];
                        sortedArray[j] = temp;
                    }
                }
            }
			
        }
        else if (userCommand == 'q') {
            break;
        }
        else {
            cout << "그런 커맨드는 없어요잉~~!!" << endl;
        }

        if (pointCount == 0) {
            frontIndex = 0;
            rearIndex = 0;
        }
    }
    return 0;
}