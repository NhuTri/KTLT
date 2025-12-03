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
const string TEAM_IDS = "221203792212039123120234"; //Requirement 4.6: Student IDs

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
	GotoXY(x, y); cout << 'X';
	for (int i = 1; i < width; i++)cout << 'X';
	cout << 'X';
	GotoXY(x, height + y); cout << 'X';
	for (int i = 1; i < width; i++)cout << 'X';
	cout << 'X';
	for (int i = y + 1; i < height + y; i++) {
		GotoXY(x, i); cout << 'X';
		GotoXY(x + width, i); cout << 'X';
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
	GotoXY(wall[0].x, wall[0].y); cout << "0";
	GotoXY(wall[1].x, wall[1].y); cout << "0";
	GotoXY(wall[2].x, wall[2].y); cout << "0";
	GotoXY(wall[3].x, wall[3].y); cout << "0";

	GotoXY(gate[0].x, gate[0].y); cout << "0";
	GotoXY(gate[1].x, gate[1].y); cout << "0";
	GotoXY(gate[2].x, gate[2].y); cout << "0";
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
//Requiment 4.6: UPDATE the function DrawSnakeAndFood() to draw the snake's body including all the team's student IDs
void DrawSnakeAndFood(char* str) {
	if (!GATE_ACTIVE && FOOD_INDEX >= 0 && FOOD_INDEX < MAX_SIZE_FOOD) {
		GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
		printf(str);
	}

	for (int i = 0; i < SIZE_SNAKE; i++) {
		GotoXY(snake[i].x, snake[i].y);
		if (strcmp(str, " ") == 0) printf(" ");
		else printf("%c", TEAM_IDS[i % TEAM_IDS.length()]);
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

// --- REQUIREMENT 4.2 Save/Load ---
//function to save game
void SaveGame(const string& filename) {
	ofstream file(filename, ios::binary);
	if (!file) {
		GotoXY(0, HEIGH_CONSOLE + 3);
		cout << "Error opening file for save!" << endl;
		return;
	}
	file.write((char*)&SIZE_SNAKE, sizeof(int));
	file.write((char*)&SPEED, sizeof(int));
	file.write((char*)&FOOD_INDEX, sizeof(int));
	file.write((char*)&CHAR_LOCK, sizeof(int));
	file.write((char*)&MOVING, sizeof(int));
	file.write((char*)&WIDTH_CONSOLE, sizeof(int));
	file.write((char*)&HEIGH_CONSOLE, sizeof(int));
	file.write((char*)snake, sizeof(POINT) * MAX_SIZE_SNAKE);
	file.write((char*)food, sizeof(POINT) * MAX_SIZE_FOOD);
	file.close();

	GotoXY(0, HEIGH_CONSOLE + 3);
	cout << "Game saved to " << filename << " successfully! Press any key to continue.";
	_getch();
}

//function to load game
bool LoadGame(const string& filename) {
	ifstream file(filename, ios::binary);
	if (!file) {
		return false;
	}
	file.read((char*)&SIZE_SNAKE, sizeof(int));
	file.read((char*)&SPEED, sizeof(int));
	file.read((char*)&FOOD_INDEX, sizeof(int));
	file.read((char*)&CHAR_LOCK, sizeof(int));
	file.read((char*)&MOVING, sizeof(int));
	file.read((char*)&WIDTH_CONSOLE, sizeof(int));
	file.read((char*)&HEIGH_CONSOLE, sizeof(int));
	file.read((char*)snake, sizeof(POINT) * MAX_SIZE_SNAKE);
	file.read((char*)food, sizeof(POINT) * MAX_SIZE_FOOD);
	file.close();
	return true;
}

// --- REQUIMENT 4.6 Main Menu ---
//function to process keystrokes (W, A, S, D, L, T...) entered by the user
//separated from main function
void PlayGame() {
	thread t1(ThreadFunc);
	HANDLE handle_t1 = t1.native_handle();
	int temp;

	while (1) {
		temp = toupper(_getch());
		if (STATE == 1) {
			if (temp == 'P') {
				PauseGame(handle_t1);
			}
			else if (temp == 27) {
				ExitGame(handle_t1);
				t1.detach();
				return;
			}
			else if (temp == 'L') {
				PauseGame(handle_t1);
				GotoXY(0, HEIGH_CONSOLE + 2);
				string filename;
				cout << "Save game, enter filename to save: ";
				cin >> filename;
				SaveGame(filename);
				system("cls");
				DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
				ResumeThread(handle_t1);
			}
			else if (temp == 'T') {
				PauseGame(handle_t1);
				GotoXY(0, HEIGH_CONSOLE + 2);
				string filename;
				cout << "Load game, enter filename to load: ";
				cin >> filename;
				if (LoadGame(filename)) {
					system("cls");
					DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
					DrawSnakeAndFood((char*)"O");
					ResumeThread(handle_t1);
				}
				else {
					cout << "\nLoad failed! Press any key to return.";
					_getch();
					system("cls");
					DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
					ResumeThread(handle_t1);
				}
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
			if (temp == 'Y') {
				ExitGame(handle_t1);
				t1.detach();
				StartGame();
				PlayGame();
				return;
			}
			else {
				ExitGame(handle_t1);
				t1.detach();
				return;
			}
		}
	}
}

//function to display main menu
//Displays a simple menu interface for users to select (1. New Game, 2. Load Game, 3. Exit).
void MainMenu() {
	int choice;
	while (true) {
		system("cls");
		cout << "1. New Game\n2. Load Game\n3. Exit\n";
		cout << "Your choice: "; cin >> choice;
		switch (choice) {
		case 1: StartGame(); PlayGame(); break;
		case 2: {
			string name; cout << "Filename: "; cin >> name;
			LoadGame(name); PlayGame(); break;
		}
		case 3: exit(0);
		}
	}
}

//Use a loop to keep the user in the program.
//After the game is finished (or Game Over), the program will return to the Main Menu instead of closing.
int main() {
	FixConsoleWindow();
	MainMenu();
	return 0;
}
