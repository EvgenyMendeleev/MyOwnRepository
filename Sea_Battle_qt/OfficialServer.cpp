#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <iostream>
#include <clocale>
#include <fcntl.h>
using namespace std;

enum states {WaitingOfReadyPlayer, WaitingOfConnection, Win, Lose, PlacingShips, Ready, WaitingOfTurn, Turn};
enum Msg_type {result_of_shot, enemy_shot, state_for_client, error};
enum ResultOfShot {not_hit, hit, kill};

struct Shot
{
	int16_t PosX;
	int16_t PosY;
};

struct Message
{
	int16_t type;
	int16_t PosX;
	int16_t PosY;
	int16_t Result;
};

struct StateForClient
{
	int16_t type = static_cast<state_for_client>;
	int16_t state;
};

int FirstPlayer, SecondPlayer;
int16_t FieldOfSP[100], FieldOfFP[100];
states stateOfFirstPlayer, stateOfSecondPlayer;

void showFP()
{
	for(int i = 0; i < 10; i++)
	{
		for(int j = 0; j < 10; j++)
		{
			cout << FieldOfFP[10 * i + j] << " ";
		}

		cout << endl;
	}
}

void showSP()
{
	for(int i = 0; i < 10; i++)
	{
		for(int j = 0; j < 10; j++)
		{
			cout << FieldOfSP[10 * i + j] << " ";
		}

		cout << endl;
	}
}

bool DeadShips(int* LifeOfShip)
{
	int count = 0;

	for(int i = 0; i < 10; i++)
	{
		if(LifeOfShip[i] == 0)
		{
			++count;
		}
	}

	if(count == 10)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void BurnAroundShip(int16_t* Field, int i0, int j0, int k, int l)
{
	for(int i = i0; i <= k; i++)
	{
		for(int j = j0; j <= l; j++)
		{
			Field[10 * i + j] = -11;
		}
	}
}

void FindBurnedShip(int16_t* Field, int16_t shipID)
{
	int typeOfShip;

	if(shipID >= 1 && shipID <= 4)
	{
		typeOfShip = 1;
	}
	else if(shipID >= 5 && shipID <= 7)
	{
		typeOfShip = 2;
	}
	else if(shipID >= 8 && shipID <= 9)
	{
		typeOfShip = 3;
	}
	else if(shipID == 10)
	{
		typeOfShip = 4;
	}

	int minusShipID = -shipID;

	int ShipI = -1;
        int ShipJ = -1;
	
	bool isHorisontal;

	for(int i = 0; i < 10; i++)
	{
		for(int j = 0; j < 10; j++)
		{
			if(Field[10 * i + j] == minusShipID)
			{
				ShipI = i;
				ShipJ = j;
				break;
			}
		}

		if(ShipI != -1 && ShipJ != -1)
		{
			cout << "I'm in if()" << endl;
			break;
		}
	}

	if(typeOfShip != 1)
	{
		if(ShipI + 1 > 9)
		{
			isHorisontal = true;
		}
		else if(ShipJ + 1 > 9)
		{
			isHorisontal = false;
		}
		else
		{
			if(Field[10 * (ShipI + 1) + ShipJ] == minusShipID)
			{
				isHorisontal = false;
			}
			else if(Field[10 * ShipI + ShipJ + 1] == minusShipID)
			{
				isHorisontal = true;
			}
		}
	}
	else
	{
		isHorisontal = true;
	}

	if(isHorisontal)
	{
		if(ShipI - 1 < 0)	//Верх
		{
			if(ShipJ - 1 < 0)		//Верхний левый угол
			{
				BurnAroundShip(Field, ShipI, ShipJ, ShipI + 1, ShipJ + typeOfShip);
			}
			else if(ShipJ + typeOfShip > 9)	//Верхний правый угол
			{
				BurnAroundShip(Field, ShipI, ShipJ - 1, ShipI + 1, ShipJ + typeOfShip - 1);
			}
			else		//Просто верх
			{
				BurnAroundShip(Field, ShipI, ShipJ - 1, ShipI + 1, ShipJ + typeOfShip);
			}
		}
		else if(ShipI + 1 > 9)	//Низ
		{
			if(ShipJ - 1 < 0)	//Нижний левый угол
			{	
				BurnAroundShip(Field, ShipI - 1, ShipJ, ShipI, ShipJ + typeOfShip);
			}
			else if(ShipJ + typeOfShip > 9)	//Нижний правый угол
			{
				BurnAroundShip(Field, ShipI - 1, ShipJ - 1, ShipI, ShipJ + typeOfShip - 1);
			}
			else		//Просто низ
			{
				BurnAroundShip(Field, ShipI - 1, ShipJ - 1, ShipI, ShipJ + typeOfShip);
			}
		}
		else if(ShipJ - 1 < 0)	//Левый бок
		{
			BurnAroundShip(Field, ShipI - 1, ShipJ, ShipI + 1, ShipJ + typeOfShip);
		}
		else if(ShipJ + typeOfShip > 9)	//Правый бок
		{
			BurnAroundShip(Field, ShipI - 1, ShipJ - 1, ShipI + 1, ShipJ + typeOfShip - 1);
		}
		else	//Не по краям
		{
			BurnAroundShip(Field, ShipI - 1, ShipJ - 1, ShipI + 1, ShipJ + typeOfShip);
		}
	}
	else
	{
		if(ShipI - 1 < 0)	//Верх
		{
			if(ShipJ - 1 < 0)	//Верхний левый
			{
				BurnAroundShip(Field, ShipI, ShipJ, ShipI + typeOfShip, ShipJ + 1);
			}
			else if(ShipJ + 1 > 9)	//Верхний правый
			{
				BurnAroundShip(Field, ShipI, ShipJ - 1, ShipI + typeOfShip, ShipJ);
			}
			else		//Просто верх
			{
				BurnAroundShip(Field, ShipI, ShipJ - 1, ShipI + typeOfShip, ShipJ + 1);
			}
		}
		else if(ShipI + typeOfShip > 9)	//Низ
		{
                        if(ShipJ - 1 < 0)    //Нижний левый
                        {
                                BurnAroundShip(Field, ShipI - 1, ShipJ, ShipI + typeOfShip - 1, ShipJ + 1);
                        }
                        else if(ShipJ + 1 > 9)       //Нижний правый
                        {
                                BurnAroundShip(Field, ShipI - 1, ShipJ - 1, ShipI + typeOfShip - 1, ShipJ);
                        }
                        else			//Просто низ
                        {
                                BurnAroundShip(Field, ShipI - 1, ShipJ - 1, ShipI + typeOfShip - 1, ShipJ + 1);
                        }

		}
		else if(ShipJ - 1 < 0)	//Левый бок
		{
			BurnAroundShip(Field, ShipI - 1, ShipJ, ShipI + typeOfShip, ShipJ + 1);
		}
		else if(ShipJ + 1 > 9)	//Правый бок
		{
			BurnAroundShip(Field, ShipI - 1, ShipJ - 1, ShipI + typeOfShip, ShipJ);
		}
		else	//Не по краям
		{
			BurnAroundShip(Field, ShipI - 1, ShipJ - 1, ShipI + typeOfShip, ShipJ + 1);
		}
	}
}

void* DataFromFirstClient(void* NullData)
{
	stateOfFirstPlayer = PlacingShips;
	recv(FirstPlayer, &FieldOfFP, 100 * sizeof(int16_t), MSG_NOSIGNAL);
	stateOfFirstPlayer = Ready;

	showFP();

	cout << "В первом потоке: " << stateOfFirstPlayer << " " << stateOfSecondPlayer << "\n";

	if(stateOfSecondPlayer == PlacingShips)
	{
		stateOfFirstPlayer = WaitingOfReadyPlayer;
		//send(*firstPlayer, &stateOfFirstPlayer, sizeof(stateOfFirstPlayer), MSG_NOSIGNAL);
		while(stateOfSecondPlayer != Ready);
	}

	stateOfFirstPlayer = Turn;
	//send(*firstPlayer, &stateOfFirstPlayer, sizeof(stateOfFirstPlayer), MSG_NOSIGNAL);

    	/*
         * Информация о кораблях противника.
         * -------------------------------------------------------
         * i = 0,9 - индентификаторы кораблей,
         * которые получаются как i = FieldOf**[10 * x + y] - 1.
         * -------------------------------------------------------
        */

        int LifeOfShip[10];

        for(int i = 0; i < 10; i++)
        {
                if(i >= 0 && i <= 3)
                {
                        LifeOfShip[i] = 1;
                }
                else if(i >= 4 && i <= 6)
                {
                        LifeOfShip[i] = 2;
                }
                else if(i >= 7 && i <= 8)
                {
                        LifeOfShip[i] = 3;
                }
                else if(i == 9)
                {
                        LifeOfShip[i] = 4;
                }
        }

	fcntl(FirstPlayer, F_SETFL, O_NONBLOCK);

	bool DeadAllShips = false;

	while(!DeadAllShips && (stateOfSecondPlayer != Win))
	{
		Shot* InfoFromClient = new Shot;

		if(recv(FirstPlayer, InfoFromClient, sizeof(Shot), MSG_NOSIGNAL) > 0)
		{

		if(stateOfFirstPlayer != Turn)
		{
			cout << "Первый игрок, не наглей, а! Сиди и жди!" << endl;
		}
		else
		{
			if(FieldOfSP[10 * InfoFromClient->PosY + InfoFromClient->PosX] > 0)
			{
				Message MsgForFP, MsgForSP;

				MsgForFP.type = static_cast<int16_t>(result_of_shot);
				MsgForFP.PosX = InfoFromClient->PosX;
				MsgForFP.PosY = InfoFromClient->PosY;

				MsgForSP.type = static_cast<int16_t>(enemy_shot);
				MsgForSP.PosX = InfoFromClient->PosX;
				MsgForSP.PosY = InfoFromClient->PosY;

				if(FieldOfSP[10 * InfoFromClient->PosY + InfoFromClient->PosX] != 0)
				{
					int16_t shipID = FieldOfSP[10 * InfoFromClient->PosY + InfoFromClient->PosX];
				
					--LifeOfShip[shipID - 1];
					FieldOfSP[10 * InfoFromClient->PosY + InfoFromClient->PosX] = -shipID;

					if(LifeOfShip[shipID - 1] == 0)
					{
						FindBurnedShip(FieldOfSP, shipID);

						MsgForFP.Result = static_cast<int16_t>(kill);
						MsgForSP.Result = static_cast<int16_t>(kill);
					}
					else
					{
						MsgForFP.Result = static_cast<int16_t>(hit);
						MsgForSP.Result = static_cast<int16_t>(hit);
					}
				}
				else
				{
					FieldOfSP[10 * InfoFromClient->PosY + InfoFromClient->PosX] = -11;

					cout << "Первый игрок не попал!" << endl;

					MsgForFP.Result = static_cast<int16_t>(not_hit);
					MsgForSP.Result = static_cast<int16_t>(not_hit);

					stateOfFirstPlayer = WaitingOfTurn;
					stateOfSecondPlayer = Turn;
				}

				showSP();

				send(FirstPlayer, &MsgForFP, sizeof(MsgForFP), MSG_NOSIGNAL);
				send(SecondPlayer, &MsgForSP, sizeof(MsgForSP), MSG_NOSIGNAL);

				DeadAllShips = DeadShips(LifeOfShip);

				if(DeadAllShips)
				{
					cout << "====================" << endl;
					cout << "Первый игрок выиграл" << endl;
					cout << "====================" << endl;
					stateOfFirstPlayer = Win;
				}
			}
			else
			{
				cout << "На поле второго игрока в координатах (" << InfoFromClient->PosY << ";" << InfoFromClient->PosX << ") подбито!" << endl;
			}
		}
		}
		delete InfoFromClient;
	}

	cout << "++++++++++++++++++++++++++++" << endl;
	cout << "Первый поток завершил работу" << endl;
	cout << "++++++++++++++++++++++++++++" << endl;

	pthread_exit(0);
}

void* DataFromSecondClient(void* NullData)
{
	stateOfSecondPlayer = PlacingShips;
	recv(SecondPlayer, &FieldOfSP, 100 * sizeof(int16_t), MSG_NOSIGNAL);
	stateOfSecondPlayer = Ready;

	showSP();

	cout << "В втором потоке: " << stateOfFirstPlayer << " " << stateOfSecondPlayer << "\n";

	if(stateOfFirstPlayer == PlacingShips)
	{
		stateOfSecondPlayer = WaitingOfReadyPlayer;
		//send(*secondPlayer, &stateOfSecondPlayer, sizeof(stateOfSecondPlayer), MSG_NOSIGNAL);

		while(stateOfFirstPlayer != Ready);
	}
	
	stateOfSecondPlayer = WaitingOfTurn;
	//send(*secondPlayer, &stateOfSecondPlayer, sizeof(stateOfSecondPlayer), MSG_NOSIGNAL);
	
	/*
	 * Информация о кораблях противника.
	 * -------------------------------------------------------
	 * i = 0,9 - индентификаторы кораблей,
	 * которые получаются как i = FieldOf**[10 * x + y] - 1.
	 * -------------------------------------------------------
	*/

	int LifeOfShip[10];

	for(int i = 0; i < 10; i++)
	{
		if(i >= 0 && i <= 3)
		{
			LifeOfShip[i] = 1;
		}
		else if(i >= 4 && i <= 6)
		{
			LifeOfShip[i] = 2;
		}
		else if(i >= 7 && i <= 8)
		{
			LifeOfShip[i] = 3;
		}
		else if(i == 9)
		{
			LifeOfShip[i] = 4;
		}
	}

	fcntl(SecondPlayer, F_SETFL, O_NONBLOCK);

	bool DeadAllShips = false;

	while(!DeadAllShips && (stateOfFirstPlayer != Win))
	{
		Shot* InfoFromClient = new Shot;
		if(recv(SecondPlayer, InfoFromClient, sizeof(Shot), MSG_NOSIGNAL) > 0)
		{
		if(stateOfSecondPlayer != Turn)
		{
			cout << "Второй игрок, не наглей, а! Сиди и жди!" << endl;
		}
		else
		{
			if(FieldOfFP[10 * InfoFromClient->PosY + InfoFromClient->PosX] > 0)
			{
				Message MsgForFP, MsgForSP;

				MsgForFP.type = static_cast<int16_t>(enemy_shot);
				MsgForFP.PosX = InfoFromClient->PosX;
				MsgForFP.PosY = InfoFromClient->PosY;

				MsgForSP.type = static_cast<int16_t>(result_of_shot);
				MsgForSP.PosX = InfoFromClient->PosX;
				MsgForSP.PosY = InfoFromClient->PosY;

				if(FieldOfFP[10 * InfoFromClient->PosY + InfoFromClient->PosX] != 0)
				{
					int16_t shipID = FieldOfFP[10 * InfoFromClient->PosY + InfoFromClient->PosX];

					--LifeOfShip[shipID - 1];
					FieldOfFP[10 * InfoFromClient->PosY + InfoFromClient->PosY] = -shipID;

					if(LifeOfShip[shipID - 1] == 0)
					{
						FindBurnedShip(FieldOfFP, shipID);

						MsgForFP.Result = static_cast<int16_t>(kill);
						MsgForSP.Result = static_cast<int16_t>(kill);
					}
					else
					{
						MsgForFP.Result = static_cast<int16_t>(hit);
						MsgForSP.Result = static_cast<int16_t>(hit);
					}
				}
				else
				{
					FieldOfFP[10 * InfoFromClient->PosY + InfoFromClient->PosX] = -11;

					cout << "Второй игрок не попал!" << endl;
					
					MsgForFP.Result = static_cast<int16_t>(not_hit);
					MsgForSP.Result = static_cast<int16_t>(not_hit);

					stateOfFirstPlayer = Turn;
					stateOfSecondPlayer = WaitingOfTurn;
				}

				showFP();

				send(FirstPlayer, &MsgForFP, sizeof(MsgForFP), MSG_NOSIGNAL);
				send(SecondPlayer, &MsgForSP, sizeof(MsgForSP), MSG_NOSIGNAL);

				DeadAllShips = DeadShips(LifeOfShip);

				if(DeadAllShips)
				{
					cout << "=======================" << endl;
					cout << "Второй игрок выиграл!!!" << endl;
					cout << "=======================" << endl;
					stateOfSecondPlayer = Win;
				}
			}
			else
			{
				cout << "На поле первого игрока (" << InfoFromClient->PosY << ";" << InfoFromClient->PosX << ") подбито" << endl;
			}
		}
		}
		delete InfoFromClient;
	}

	cout << "============================" << endl;
	cout << "Второй поток завершил работу" << endl;
	cout << "============================" << endl;

	pthread_exit(0);
}

int main()
{
	setlocale(LC_ALL, "Russian");

	//---------------Создание слушающего сокета---------------------
	int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in MasterAddr;
	MasterAddr.sin_family = AF_INET;
	MasterAddr.sin_port = htons(25567);
	MasterAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(MasterSocket, (struct sockaddr*) (&MasterAddr), sizeof(MasterAddr));

	listen(MasterSocket, 1);
	//-------------------------------------------------------------
	
	//----------------Соединение клиентов---------------------
	
	FirstPlayer = accept(MasterSocket, 0, 0);

	if(FirstPlayer > 0)
	{
		printf("Первый игрок подключился!\n");
		stateOfFirstPlayer = WaitingOfConnection;
		//send(FirstPlayer, &stateOfFirstPlayer, sizeof(stateOfFirstPlayer), MSG_NOSIGNAL);
	}
	else
	{
		printf("Соединение провалено!\n");
		return 0;
	}

	cout << "После подключения первого игрока: " << stateOfFirstPlayer << " " << stateOfSecondPlayer << "\n";

	SecondPlayer = accept(MasterSocket, 0, 0);

	if(SecondPlayer > 0)
	{
		printf("Второй игрок подключился!\n");
		
		stateOfFirstPlayer = stateOfSecondPlayer = PlacingShips;
		
		//send(FirstPlayer, &stateOfFirstPlayer, sizeof(stateOfFirstPlayer), MSG_NOSIGNAL);
		//send(SecondPlayer, &stateOfSecondPlayer, sizeof(stateOfSecondPlayer), MSG_NOSIGNAL);
	}
	else
	{
		printf("Соединение провалено!\n");
		return 0;
	}
	//------------------------------------------------------
	
	cout << "После подключения всех игроков: " << stateOfFirstPlayer << " " << stateOfSecondPlayer << "\n";

	//----------------------Процесс игры----------------------
	void* Null = NULL;
	
	pthread_t FirstThread, SecondThread;

	pthread_create(&FirstThread, 0, DataFromFirstClient, Null);
	pthread_create(&SecondThread, 0, DataFromSecondClient, Null);

	pthread_join(FirstThread, 0);
	pthread_join(SecondThread, 0);
	//--------------------------------------------------------

	return 0;
}
