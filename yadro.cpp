#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <queue>
#include <string>
#include <map>

//Функция сохранения прибыли и времени аренды каждого компьютера в клубе
void CountEarnings(std::map<int, std::pair<int, int>>& tablesEarnings, int tableNumber, int  hourPrice, int orderedHourNumber, int hoursDiff, int minDiff) {
	//Проверяем
	if (orderedHourNumber < 1) {
		orderedHourNumber = 1;
	}
	
	tablesEarnings[tableNumber].first += hourPrice * orderedHourNumber;
	if (hoursDiff > 0) {
		tablesEarnings[tableNumber].second += hoursDiff * 60 + minDiff;
	} else {
		tablesEarnings[tableNumber].second += minDiff;
	}
}

int main(int argc, char *argv[]) {
	int numberOfTables = 0;
	int startHour, startMin;
	int endHour, endMin;
	int hourPrice;
	char symbol;
	std::string inputFileName;

	std::ifstream fin(argv[1]);
	std::ofstream fout("output.txt");
	std::queue<std::string> clientsQueue;
	std::map<int, bool> freeTables;
	std::map<std::string, std::pair<int, std::pair<int, int>>> clientsTables; // i - Имя пользователя, map[i].first  - Номер стола, map[i].second.first - Час, когда клиент сел за стол, map[i].second.second - Минута, когда клиент сел за стол
	std::map<int, std::pair<int, int>> tablesEarnings; // i - Номер стола, map[i].first - выручка со стола, map[i].second - время в минутах, которое пользователь провёл за компьютером
	std::string startTime, endTime;
	fin >> numberOfTables;

	fin >> startTime >> endTime;
	fin >> hourPrice;
	startHour = std::stoi(startTime.substr(0, 2));
	startMin = std::stoi(startTime.substr(3, 4));
	endHour = std::stoi(endTime.substr(0, 2));
	endMin = std::stoi(endTime.substr(3, 4));
	fout << startTime << "\n";
	//Инициализируем массив, хранящие данные по занятости столов
	for (int i = 1; i <= numberOfTables; i++) {
		freeTables.insert(std::make_pair(i, true));
		tablesEarnings.insert(std::make_pair(i, std::make_pair(0, 0)));
	}
	//Пока не конец файла считваем данные событий
	while (!fin.eof()) {
		std::string curTime;
		int curHour, curMin;
		int indexOfEvent;
		int tableNumber;
		std::string clientName;
		fin >> curTime >> indexOfEvent >> clientName;
		//Проверка на последнюю строку
		if (!curTime.empty()) {
			curHour = std::stoi(curTime.substr(0, 2));
			curMin = std::stoi(curTime.substr(3, 4));
			//Если событие - это посадка за компьютер, то дополнительно считываем номер стола
			//Также делаем запись события в файл
			if (indexOfEvent == 2) {
				fin >> tableNumber;
				fout << curTime << " " << indexOfEvent << " " << clientName << " " << tableNumber << "\n";
			} else {
				fout << curTime << " " << indexOfEvent << " " << clientName << "\n";
			}
			//Проверка на попадания события по времени в промежуток работы комп. клуба
			if ((curHour > startHour && curHour < endHour) || ((curHour == startHour && curMin >= startMin) || (curHour == endHour && curMin <= endMin))) {
				bool isAnyFreeTable = false;
				switch (indexOfEvent) {
				case(1):
					//Проверяем есть ли клиент в комп. клубе 
					if (clientsTables.find(clientName) != clientsTables.end()) {
						fout << curTime << " 13 YouShallNotPass" << "\n";
					} else {
						clientsTables.insert(std::make_pair(clientName, std::make_pair(0, std::make_pair(0, 0))));
					}
					break;
				case(2):
					//Проверяем зашёл ли клиент в наш клуб
					if (clientsTables.find(clientName) == clientsTables.end()) {
						fout << curTime << " 13 ClientUnknown" << "\n";
					} else if (freeTables[tableNumber]) {
					//Если стол свободен, то нужно проверить сидит ли студент уже за столом и посчитать его
						freeTables[tableNumber] = false;
						if (clientsTables.find(clientName) != clientsTables.end() && clientsTables[clientName].first != 0) {
							freeTables[clientsTables[clientName].first] = true;
							int bonusHour = ((curMin - clientsTables[clientName].second.second) > 0) ? 1 : 0;
							int curTableOrderedHours = curHour - clientsTables[clientName].second.first + bonusHour;
							int curHoursDiff = curHour - clientsTables[clientName].second.first;
							int curMinutesDiff = curMin - clientsTables[clientName].second.second;
							CountEarnings(tablesEarnings, tableNumber, hourPrice, curTableOrderedHours, curHoursDiff, curMinutesDiff);
						}
						clientsTables[clientName].first = tableNumber;
						clientsTables[clientName].second.first = curHour;
						clientsTables[clientName].second.second = curMin;
					} else {
						fout << curTime << " 13 PlaceIsBusy" << "\n";
					}
					break;
				case(3):
					//Проверяем есть ли свободные столы
					for (int i = 1; i <= numberOfTables; i++) {
						if (freeTables[i]) {
							fout << curTime << " 13 ICanWaitNoLonger!" << "\n";
							//clientsTables.erase(clientName);
							isAnyFreeTable = true;
							break;
						}
					}
					//Если свободных столов нет, то проверяем есть ли свободное место в очереди, если нет, то клиент уходит из клуба
					if (!isAnyFreeTable) {
						if (clientsQueue.size() >= numberOfTables) {
							fout << curTime << " 11 " + clientName << "\n";
						} else {
							clientsQueue.push(clientName);
						}
					}
					break;
				case(4):
					//Проверяем есть ли клиент в клубе
					if (clientsTables.find(clientName) == clientsTables.end()) {
						fout << curTime << " 13 ClientUnknown" << "\n";
					} else if (clientsQueue.size() > 0) {
					//Проверяем есть ли клиенты в очереди, тогда садим первого клиента из очереди за освободившийся компьютер
						fout << curTime << " 12 " << clientsQueue.front() << " " << clientsTables[clientName].first << "\n";
						clientsTables[clientsQueue.front()].first = clientsTables[clientName].first;
						clientsTables[clientsQueue.front()].second.first = curHour;
						clientsTables[clientsQueue.front()].second.second = curMin;
						clientsQueue.pop();
					} else {
						freeTables[clientsTables[clientName].first] = true;
					}
					//Считаем прибыль от арендованного компьютера и удаляем запись о клиенте
					if (clientsTables[clientName].first != 0) {
						int bonusHour = ((curMin - clientsTables[clientName].second.second) > 0) ? 1 : 0;
						int curTableOrderedHours = curHour - clientsTables[clientName].second.first + bonusHour;
						int curHoursDiff = curHour - clientsTables[clientName].second.first;
						int curMinutesDiff = curMin - clientsTables[clientName].second.second;
						
						CountEarnings(tablesEarnings, clientsTables[clientName].first, hourPrice, curTableOrderedHours, curHoursDiff, curMinutesDiff);
					}
					clientsTables.erase(clientName);
					break;
				}
			} else {
				fout << curTime << " 13 NotOpenYet" << "\n";
			}
		}
	}

	for (const auto& element : clientsTables) {
		if (element.second.first != 0) {
			fout << endTime << " 11 " << element.first << "\n";
			int bonusHour = ((endMin - element.second.second.second) > 0) ? 1 : 0;
			int curTableOrderedHours = endHour - element.second.second.first + bonusHour;
			int curHoursDiff = endHour - element.second.second.first;
			int curMinutesDiff = endMin - element.second.second.second;
			CountEarnings(tablesEarnings, element.second.first, hourPrice, curTableOrderedHours, curHoursDiff, curMinutesDiff);
		}
	}
	//Выводим время конца смены и прибыль со столов
	fout << endTime << "\n";
	//Выводим прибыль по столам
	for (const auto& element : tablesEarnings) {
		int orderedTime = element.second.second;
		int tableHours = orderedTime / 60, tableMinutes = orderedTime % 60;
		if (tableHours > 9 && tableMinutes> 9) {
			fout << element.first << " " << element.second.first << " " << tableHours << ":" << tableMinutes<< "\n";
		} else if (tableHours > 9 && tableMinutes< 9) {
			fout << element.first << " " << element.second.first << " " << tableHours << ":0" << tableMinutes<< "\n";
		} else if (tableHours < 9 && tableMinutes< 9) {
			fout << element.first << " " << element.second.first << " 0" << tableHours << ":0" << tableMinutes<< "\n";
		} else if (tableHours < 9 && tableMinutes> 9) {
			fout << element.first << " " << element.second.first << " 0" << tableHours << ":" << tableMinutes<< "\n";
		}
	}
	return 0;
}