#include "CommonInclude.h"

using namespace std;

struct Point {
	int x, y, z;
	double distance() const {
		return sqrt(x * x + y * y + z * z);
	}

	bool _off = true;
};

Point _pointList[11];
Point _tempList[11];

int _count = 0;


int main() {

	char _command = 0;
	bool toggleSort = false;

	int tail = 0;
	int head = 0;




	while (true)
	{
		if (toggleSort) {
			cout << "--------------------------" << endl;
			for (int i = 9; i >= 0; i--)
			{
				if (_tempList[i]._off)
					cout << "List [" << i << "]: " << endl;
				else
					cout << "List [" << i << "]: (" << _tempList[i].x << ", "
					<< _tempList[i].y << ", " << _tempList[i].z << ")" << endl;
			}
			cout << "--------------------------" << endl;
		}
		else {
			cout << "--------------------------" << endl;
			for (int i = 9; i >= 0; i--)
			{
				if (_pointList[i]._off)
					cout << "List [" << i << "]: " << endl;
				else
					cout << "List [" << i << "]: (" << _pointList[i].x << ", "
					<< _pointList[i].y << ", " << _pointList[i].z << ")" << endl;
			}
			cout << "--------------------------" << endl;
		}



		cout << "Command : ";
		cin >> _command;


		if (_command == '+')
		{
			if (head - tail == 9)
			{
				cout << "List full" << endl;
				continue;
			}

			int _x, _y, _z;
			cin >> _x >> _y >> _z;

			if (head == 0)
			{
				for (int i = 9; i > head; i--)
				{
					_pointList[i] = _pointList[i - 1];
				}
				_pointList[head].x = _x;
				_pointList[head].y = _y;
				_pointList[head].z = _z;
				_pointList[head]._off = false;
				tail++;
				_count++;
			}
			else
			{
				head--;
				_pointList[head].x = _x;
				_pointList[head].y = _y;
				_pointList[head].z = _z;
				_pointList[head]._off = false;
				_count++;
			}

		}


		else if (_command == '-')
		{
			if (tail == head)
			{
				cout << "List empty" << endl;
				continue;
			}


			_pointList[head]._off = true;
			head++;
			_count--;
		}


		else if (_command == 'e')
		{
			if (head - tail == 9)
			{
				cout << "List full" << endl;
				continue;
			}

			int _x, _y, _z;
			cin >> _x >> _y >> _z;
			_pointList[tail].x = _x;
			_pointList[tail].y = _y;
			_pointList[tail].z = _z;
			_pointList[tail]._off = false;
			tail++;
			_count++;
		}


		else if (_command == 'd')
		{
			if (tail == head)
			{
				cout << "List empty" << endl;
				continue;
			}

			_pointList[tail - 1]._off = true;
			tail--;
			_count--;
		}


		else if (_command == 'c')
		{
			for (int i = 0; i < 10; i++)
			{
				_pointList[i]._off = true;
			}
			head = 0;
			tail = 0;
			_count = 0;
		}


		else if (_command == 'a')
		{
			cout << "Count: " << _count << endl;
		}


		else if (_command == 'b')
		{
			_pointList[10] = _pointList[0];

			for (int i = 1; i < 11; i++)
			{
				_pointList[i - 1] = _pointList[i];
			}

			for (int i = 0; i < 10; i++)
			{
				if (_pointList[i]._off)
				{
					head = i;
					break;
				}
			}

			for (int i = 9; i >= 0; i--)
			{
				if (!_pointList[i]._off)
				{
					tail = i + 1;
					break;
				}
			}
		}


		else if (_command == 'f')
		{
			if (head == tail)
			{
				cout << "List empty" << endl;
				continue;
			}

			for (int i = 0; i < 10; i++)
			{
				_tempList[i] = _pointList[i];
			}

			for (int i = 0; i < 9; i++)
			{
				for (int j = i + 1; j < 10; j++)
				{
					if (_tempList[i]._off || _tempList[j]._off)
						continue;
					if (_tempList[i].distance() > _tempList[j].distance())
					{
						Point _temp = _tempList[i];
						_tempList[i] = _tempList[j];
						_tempList[j] = _temp;
					}
				}
			}
			toggleSort = true;
		}


		else if (_command == 'q')
		{
			break;
		}
		else
		{
			cout << "wrong command" << endl;
		}



		if (_count == 0)
		{
			head = 0;
			tail = 0;
		}
	}
	return 0;
}