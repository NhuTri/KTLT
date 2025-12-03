#include <iostream>   
#include <vector>      
#include <windows.h> 
#include <cstring> 
#include <cmath>
#include <stdio.h>
#include <conio.h>
#include <ctime>
#include <string.h>
#include <fstream>
#include <thread>

using namespace std;

//Constants
#define MAX_SIZE_SNAKE 10
#define MAX_SIZE_FOOD 4
#define MAX_SPEED 3
//Global variables
POINT snake[10]; //snake
POINT food[4]; // food
int CHAR_LOCK;//used to determine the direction my snake cannot move (At a moment, there is one direction my snake cannot move to)
int MOVING;//used to determine the direction my snake moves (At a moment, there are three directions my snake can move)
int SPEED;// Standing for level, the higher the level, the quicker the speed
int HEIGH_CONSOLE, WIDTH_CONSOLE;// Width and height of console-screen
int FOOD_INDEX; // current food-index
int SIZE_SNAKE; // size of snake, initially maybe 6 units and maximum size maybe 10
int STATE; // State of snake: dead or alive
//Global variables for gate
POINT gate[3];// gate
POINT wall[4];// wall    
bool GATE_ACTIVE;// whether the gate is currently active
int LEVEL = 1;

//function to fix the screen with reasonable size
void FixConsoleWindow() {
	HWND consoleWindow = GetConsoleWindow();
	LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
	style = style & ~(WS_MAXIMIZEBOX) & ~(WS_THICKFRAME);
	SetWindowLong(consoleWindow, GWL_STYLE, style);
}

//function to move to all the positions of console window
void GotoXY(int x, int y) {
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

//function to checks if the input-coordinates are the same as anything of snakearray or not
bool IsValid(int x, int y) {
	for (int i = 0; i < SIZE_SNAKE; i++) {
		if (snake[i].x == x && snake[i].y == y)
			return false;
	}
	return true;
}

//function to creates random food-array 
void GenerateFood() {
	int x, y;
	srand(time(NULL));
	for (int i = 0; i < MAX_SIZE_FOOD; i++) {
		do {
			x = rand() % (WIDTH_CONSOLE - 1) + 1;
			y = rand() % (HEIGH_CONSOLE - 1) + 1;
		} while (IsValid(x, y) == false);
		food[i] = { x,y };
	}
}

//function to assign the initial values for the game
void ResetData() {
	//Initialize the global values
	CHAR_LOCK = 'A', MOVING = 'D', SPEED = 1; FOOD_INDEX = 0, WIDTH_CONSOLE = 70, HEIGH_CONSOLE = 20, SIZE_SNAKE = 6;
	// Initialize default values for snake
	snake[0] = { 10, 5 }; snake[1] = { 11, 5 };
	snake[2] = { 12, 5 }; snake[3] = { 13, 5 };
	snake[4] = { 14, 5 }; snake[5] = { 15, 5 };
	GenerateFood();//Create food array
}

//function to draw the surrounded rectangle
void DrawBoard(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0) {
	GotoXY(x, y);cout << 'X';
	for (int i = 1; i < width; i++)cout << 'X';
	cout << 'X';
	GotoXY(x, height + y);cout << 'X';
	for (int i = 1; i < width; i++)cout << 'X';
	cout << 'X';
	for (int i = y + 1; i < height + y; i++) {
		GotoXY(x, i);cout << 'X';
		GotoXY(x + width, i);cout << 'X';
	}
	GotoXY(curPosX, curPosY);
}

//function consists of all tasks needed to do before coming into game
void StartGame() {
	system("cls");
	ResetData(); // Intialize original data
	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE); // Draw game
	STATE = 1;//Start running Thread
}

//function to exit game
void ExitGame(HANDLE t) {
	system("cls");
	TerminateThread(t, 0);
}

//function to pause game
void PauseGame(HANDLE t) {
	SuspendThread(t);
}

//function to activate gate
void ActivateGate() {
	GATE_ACTIVE = true;
	gate[0].x = gate[1].x = gate[2].x = WIDTH_CONSOLE - 1;
	gate[0].y = HEIGH_CONSOLE / 2 - 1;
	gate[1].y = HEIGH_CONSOLE / 2;
	gate[2].y = HEIGH_CONSOLE / 2 + 1;

	wall[0].x = wall[3].x = WIDTH_CONSOLE - 2;
	wall[1].x = wall[2].x = WIDTH_CONSOLE - 1;
	wall[0].y = wall[1].y = HEIGH_CONSOLE / 2 - 2;
	wall[2].y = wall[3].y = HEIGH_CONSOLE / 2 + 2;

}

//function to draw gate and wall of gate
void DrawGate() {
	GotoXY(wall[0].x, wall[0].y);cout << "0";
	GotoXY(wall[1].x, wall[1].y);cout << "0";
	GotoXY(wall[2].x, wall[2].y);cout << "0";
	GotoXY(wall[3].x, wall[3].y);cout << "0";

	GotoXY(gate[0].x, gate[0].y);cout << "0";
	GotoXY(gate[1].x, gate[1].y);cout << "0";
	GotoXY(gate[2].x, gate[2].y);cout << "0";
}

//function to update global data
void Eat() {
	snake[SIZE_SNAKE] = food[FOOD_INDEX];
	SIZE_SNAKE++;

	if (FOOD_INDEX == MAX_SIZE_FOOD - 1) {
		GATE_ACTIVE = true;
		ActivateGate();
		DrawGate();
	}
	else {
		FOOD_INDEX++;
	}

}

//function to process the dead of snake
void ProcessDead() {
	STATE = 0;
	GotoXY(0, HEIGH_CONSOLE + 2);
	printf("Dead, type y to continue or anykey to exit");
}

//function to draw snake and food
void DrawSnakeAndFood(char* str) {
	if (!GATE_ACTIVE && FOOD_INDEX >= 0 && FOOD_INDEX < MAX_SIZE_FOOD) {
		GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
		printf(str);
	}

	for (int i = 0; i < SIZE_SNAKE; i++) {
		GotoXY(snake[i].x, snake[i].y);
		printf(str);
	}
}

//function to draw snake go through gate
void DrawSnakeGoThroughGate() {
	int cnt = SIZE_SNAKE;
	while (cnt > 0) {
		GotoXY(snake[0].x, snake[0].y);
		printf(" ");
		for (int i = 0; i < cnt - 1; i++) {
			snake[i].x = snake[i + 1].x;
			snake[i].y = snake[i + 1].y;
			GotoXY(snake[i].x, snake[i].y); 
			printf("O");
		}
		cnt--;
		Sleep(300 / SPEED);
	}
}

//function solve if you pass 3 levels, you win.
void GameWin() {
	system("cls");
	GotoXY(20, 10);
	printf("YOU WIN!\n");
	exit(0);
}

//function to show next level
void NextLevel() {
	LEVEL++;
	SPEED++;
	GATE_ACTIVE = false;

	if (LEVEL > 3) {
		GameWin();
		return;
	}

	system("cls");
	CHAR_LOCK = 'A', MOVING = 'D'; 
	FOOD_INDEX = 0, WIDTH_CONSOLE = 70, HEIGH_CONSOLE = 20, SIZE_SNAKE = 6;
	snake[0] = { 10, 5 }; snake[1] = { 11, 5 };
	snake[2] = { 12, 5 }; snake[3] = { 13, 5 };
	snake[4] = { 14, 5 }; snake[5] = { 15, 5 };
	GenerateFood();
	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
	STATE = 1;
}

//function to check if the head will collide with the body
bool IsCollisionWithBody(int x, int y) {
	for (int i = 0; i < SIZE_SNAKE - 1; i++) { // exclude head itself
		if (snake[i].x == x && snake[i].y == y)
			return true;
	}
	return false;
}

//function to process the moving direction RIGHT of snake
void MoveRight() {
	if (snake[SIZE_SNAKE - 1].x + 1 == WIDTH_CONSOLE
		|| IsCollisionWithBody(snake[SIZE_SNAKE - 1].x + 1, snake[SIZE_SNAKE - 1].y)) {
		ProcessDead();
	}
	else if (GATE_ACTIVE && ((snake[SIZE_SNAKE - 1].x + 1 == wall[0].x && snake[SIZE_SNAKE - 1].y == wall[0].y)
		|| (snake[SIZE_SNAKE - 1].x + 1 == wall[2].x && snake[SIZE_SNAKE - 1].y == wall[2].y))) {
		ProcessDead();
	}
	else if ((GATE_ACTIVE && snake[SIZE_SNAKE - 1].x + 1 == gate[0].x && snake[SIZE_SNAKE - 1].y == gate[0].y)
		|| (GATE_ACTIVE && snake[SIZE_SNAKE - 1].x + 1 == gate[1].x && snake[SIZE_SNAKE - 1].y == gate[1].y)
		|| (GATE_ACTIVE && snake[SIZE_SNAKE - 1].x + 1 == gate[2].x && snake[SIZE_SNAKE - 1].y == gate[2].y)) {
		DrawSnakeGoThroughGate();
		NextLevel();
	}
	else {
		if (snake[SIZE_SNAKE - 1].x + 1 == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y) {
			Eat();
		}
		for (int i = 0; i < SIZE_SNAKE - 1; i++) {
			snake[i].x = snake[i + 1].x;
			snake[i].y = snake[i + 1].y;
		}
		snake[SIZE_SNAKE - 1].x++;
	}
}

//function to process the moving direction LEFT of snake
void MoveLeft() {
	if (snake[SIZE_SNAKE - 1].x - 1 == 0
		|| IsCollisionWithBody(snake[SIZE_SNAKE - 1].x - 1, snake[SIZE_SNAKE - 1].y)) {
		ProcessDead();
	}
	else {
		if (snake[SIZE_SNAKE - 1].x - 1 == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y) {
			Eat();
		}
		for (int i = 0; i < SIZE_SNAKE - 1; i++) {
			snake[i].x = snake[i + 1].x;
			snake[i].y = snake[i + 1].y;
		}
		snake[SIZE_SNAKE - 1].x--;
	}
}

//function to process the moving direction DOWN of snake
void MoveDown() {
	if (snake[SIZE_SNAKE - 1].y + 1 == HEIGH_CONSOLE
		|| IsCollisionWithBody(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y + 1)) {
		ProcessDead();
	}
	else if (GATE_ACTIVE && ((snake[SIZE_SNAKE - 1].x == wall[0].x && snake[SIZE_SNAKE - 1].y + 1 == wall[0].y)
		|| (snake[SIZE_SNAKE - 1].x == wall[1].x && snake[SIZE_SNAKE - 1].y + 1 == wall[1].y)
		|| (snake[SIZE_SNAKE - 1].x == wall[2].x && snake[SIZE_SNAKE - 1].y + 1 == wall[2].y)
		|| (snake[SIZE_SNAKE - 1].x == wall[3].x && snake[SIZE_SNAKE - 1].y + 1 == wall[3].y))) {
		ProcessDead();
	}
	else {
		if (snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y + 1 == food[FOOD_INDEX].y) {
			Eat();
		}
		for (int i = 0; i < SIZE_SNAKE - 1; i++) {
			snake[i].x = snake[i + 1].x;
			snake[i].y = snake[i + 1].y;
		}
		snake[SIZE_SNAKE - 1].y++;
	}
}

//function to process the moving direction UP of snake
void MoveUp() {
	if (snake[SIZE_SNAKE - 1].y - 1 == 0
		|| IsCollisionWithBody(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y - 1)) {
		ProcessDead();
	}
	else if (GATE_ACTIVE && ((snake[SIZE_SNAKE - 1].x == wall[0].x && snake[SIZE_SNAKE - 1].y - 1 == wall[0].y)
		|| (snake[SIZE_SNAKE - 1].x == wall[1].x && snake[SIZE_SNAKE - 1].y - 1 == wall[1].y)
		|| (snake[SIZE_SNAKE - 1].x == wall[2].x && snake[SIZE_SNAKE - 1].y - 1 == wall[2].y)
		|| (snake[SIZE_SNAKE - 1].x == wall[3].x && snake[SIZE_SNAKE - 1].y - 1 == wall[3].y))) {
		ProcessDead();
	}
	else {
		if (snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y - 1 == food[FOOD_INDEX].y) {
			Eat();
		}
		for (int i = 0; i < SIZE_SNAKE - 1; i++) {
			snake[i].x = snake[i + 1].x;
			snake[i].y = snake[i + 1].y;
		}
		snake[SIZE_SNAKE - 1].y--;
	}
}

//Subfunction for thread
void ThreadFunc() {
	while (1) {
		if (STATE == 1) {//If my snake is alive
			DrawSnakeAndFood((char*)" ");
			switch (MOVING) {
			case 'A':
				MoveLeft();
				break;
			case 'D':
				MoveRight();
				break;
			case 'W':
				MoveUp();
				break;
			case 'S':
				MoveDown();
				break;
			}
			DrawSnakeAndFood((char*)"O");
			Sleep(300 / SPEED);//Sleep function with SPEEED variable
		}
	}
}

// main function
int main() {
	int temp;
	FixConsoleWindow();
	StartGame();
	thread t1(ThreadFunc); //Create thread for snake
	HANDLE handle_t1 = t1.native_handle(); //Take handle of thread
	while (1) {
		temp = toupper(_getch());
		if (STATE == 1) {
			if (temp == 'P') {
				PauseGame(handle_t1);
			}
			else if (temp == 27) {
				ExitGame(handle_t1);
				return 1;
			}
			else {
				ResumeThread(handle_t1);
				if ((temp != CHAR_LOCK) && (temp == 'D' || temp == 'A' || temp == 'W' || temp == 'S'))
				{
					if (temp == 'D') CHAR_LOCK = 'A';
					else if (temp == 'W') CHAR_LOCK = 'S';
					else if (temp == 'S') CHAR_LOCK = 'W';
					else CHAR_LOCK = 'D';
					MOVING = temp;
				}
			}
		}
		else {
			if (temp == 'Y') StartGame();
			else {
				ExitGame(handle_t1);
				return 1;
			}
		}
	}
	return 0;
}