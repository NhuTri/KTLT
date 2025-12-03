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

#pragma comment(lib, "winmm.lib")

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
int HEIGHT_CONSOLE, WIDTH_CONSOLE;// Width and height of console-screen
int FOOD_INDEX; // current food-index
int SIZE_SNAKE; // size of snake, initially maybe 6 units and maximum size maybe 10
int STATE; // State of snake: dead or alive

void textColor(int background_color, int text_color) // change text and background color in Windows console
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

	int color_code = background_color * 16 + text_color;
	SetConsoleTextAttribute(hStdout, color_code);
}

// sound 
enum Sound { on, off }; // manage game audio status

void sound_intro() { // play intro music 
	PlaySound(TEXT("intro.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
}

void sound_eatFood() { // sound when snake eats food 
	PlaySound(TEXT("eatFood.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void sound_gameOver() { // play the sound when the game ends
	PlaySound(TEXT("gameOver.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void sound_choose() { // play a sound when the player selects an item in the menu
	PlaySound(TEXT("choose.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void off_sound() { // mute all sounds playing using PlaySound
	PlaySound(NULL, 0, 0);
}

void sound_beforePlay() { // play sound before game starts
	PlaySound(TEXT("beforePlay.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void sound_passGate() { // plays sound when player passes through gate
	PlaySound(TEXT("passGate.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void sound_exit() { // play background music when player exits or at game end/exit screen
	PlaySound(TEXT("exit.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
}

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
bool gate_passed(POINT* snake_point, POINT& point_gate) { // check if the snake head goes through the gate
	if (point_gate.x == snake_point[0].x && point_gate.y == snake_point[0].y) return true;
	return false;
}

bool touch_wall(POINT* snake_point, POINT point_start, POINT point_end) { // check to see if the snake's head is touching the wall
	if (snake_point[0].x == point_start.x || snake_point[0].x == point_end.x) return true;
	if (snake_point[0].y == point_start.y || snake_point[0].y == point_end.y) return true;
	return false;
}

bool touch_body(POINT* snake_point, char* snake_content) { // check to see if the snake's head is touching the snake's body
	// int SIZE_SNAKE = *(int*)(snake_content - sizeof(int));
	// if (SIZE_SNAKE < 5) return false;
	for (int i = 4; i < SIZE_SNAKE; i++) {
		if (snake_point[0].x == snake_point[i].x && snake_point[0].y == snake_point[i].y) return true;
	}
	return false;
}

bool touch_obstacle(POINT* snake_point, vector<POINT>& obstacles) { // check to see if the snake's head is touching an obstacle
	for (auto& ob : obstacles) {
		if (snake_point[0].x == ob.x && snake_point[0].y == ob.y)
			return true;
	}
	return false;
}

bool touch_wallgate(POINT* snake_point, vector<POINT>& gatewall) { // check if the snake head is touching the gate cells
	for (auto& ob : gatewall) {
		if (snake_point[0].x == ob.x && snake_point[0].y == ob.y)
			return true;
	}
	return false;
}
//function to creates random food-array 
void GenerateFood() {
	int x, y;
	srand(time(NULL));
	for (int i = 0; i < MAX_SIZE_FOOD; i++) {
		do {
			x = rand() % (WIDTH_CONSOLE - 1) + 1;
			y = rand() % (HEIGHT_CONSOLE - 1) + 1;
		} while (IsValid(x, y) == false);
		food[i] = { x,y };
	}
}

//function to assign the initial values for the game
void ResetData() {
	//Initialize the global values
	CHAR_LOCK = 'A', MOVING = 'D', SPEED = 1; FOOD_INDEX = 0, WIDTH_CONSOLE = 70, HEIGHT_CONSOLE = 20, SIZE_SNAKE = 6;
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

// draw a wall or rectangle on the console screen
void draw_wall(POINT start, POINT end, int type_wall, int backgound_color, int text_color, int background_wall_color = 11, int text_wall_color = 9) {
	textColor(background_wall_color, text_wall_color);
	for (int y = start.y; y <= end.y; y++) {
		GotoXY(start.x - 1, y);
		cout << char(type_wall) << char(type_wall);
	}
	for (int x = start.x; x <= end.x; x++) {
		GotoXY(x, end.y);
		cout << char(type_wall);
	}
	for (int y = end.y; y >= start.y; y--) {
		GotoXY(end.x, y);
		cout << char(type_wall) << char(type_wall);
	}
	for (int x = end.x; x >= start.x; x--) {
		GotoXY(x, start.y);
		cout << char(type_wall);
	}
	for (int x = start.x + 1; x <= end.x - 1; x++) {
		for (int y = start.y + 1; y <= end.y - 1; y++) {
			textColor(backgound_color, text_color);
			GotoXY(x, y);
			cout << ' ';
		}
	}
}

void draw_gameover(POINT point_start_box, POINT point_end_box, int answer) { // Draw a Game Over screen on the console, with a menu to select “Yes” or “No”
	char play_again_1[] = { char(219),char(223),char(223),char(219),char(32),char(219),char(32),char(32),char(219),char(223),char(223),char(220),char(32),char(219),char(32),char(32),char(219),char(32),char(32),char(32),char(219),char(223),char(223),char(220),char(32),char(219),char(223),char(223),char(223),char(32),char(219),char(223),char(223),char(220),char(32),char(32),char(223),char(32),char(32),char(219),char(223),char(223),char(220),'\0' };
	char play_again_2[] = { char(219),char(220),char(220),char(219),char(32),char(219),char(32),char(32),char(219),char(220),char(220),char(219),char(32),char(219),char(220),char(220),char(219),char(32),char(32),char(32),char(219),char(220),char(220),char(219),char(32),char(219),char(32),char(223),char(220),char(32),char(219),char(220),char(220),char(219),char(32),char(32),char(219),char(223),char(32),char(219),char(32),char(32),char(219),'\0' };
	char play_again_3[] = { char(219),char(32),char(32),char(32),char(32),char(223),char(223),char(32),char(223),char(32),char(32),char(223),char(32),char(220),char(220),char(220),char(223),char(32),char(32),char(32),char(223),char(32),char(32),char(223),char(32),char(223),char(223),char(223),char(223),char(32),char(223),char(32),char(32),char(223),char(32),char(223),char(223),char(223),char(32),char(223),char(32),char(32),char(223),'\0' };
	textColor(10, 0);
	GotoXY(point_start_box.x + 2, point_start_box.y + 2); cout << play_again_1;
	GotoXY(point_start_box.x + 2, point_start_box.y + 3); cout << play_again_2;
	GotoXY(point_start_box.x + 2, point_start_box.y + 4); cout << play_again_3;
	if (answer == 1) textColor(4, 14);
	else textColor(7, 3);
	char yes_1[] = { char(32),char(32),char(223),char(220),char(223),char(32),char(219),char(219),char(223),char(32),char(219),char(223),char(223),char(32),char(32),'\0' };
	char yes_2[] = { char(32),char(32),char(32),char(219),char(32),char(32),char(219),char(220),char(220),char(32),char(220),char(219),char(219),char(32),char(32),'\0' };
	GotoXY(point_start_box.x + 3, point_start_box.y + 9); cout << yes_1;
	GotoXY(point_start_box.x + 3, point_start_box.y + 10); cout << yes_2;
	if (answer == 2) textColor(4, 14);
	else textColor(7, 3);
	char no_1[] = { char(32),char(32),char(32),char(219),char(220),char(32),char(219),char(32),char(220),char(223),char(223),char(220),char(32),char(32),char(32),'\0' };
	char no_2[] = { char(32),char(32),char(32),char(219),char(32),char(223),char(219),char(32),char(223),char(220),char(220),char(223),char(32),char(32),char(32),'\0' };
	GotoXY(point_start_box.x + 23 + 6, point_start_box.y + 9); cout << no_1;
	GotoXY(point_start_box.x + 23 + 6, point_start_box.y + 10); cout << no_2;
}

void draw_snake(char* snake_content, POINT* snake_point, int move) {
	int num_snake = SIZE_SNAKE;

	for (int i = num_snake - 1; i >= 0; i--) {
		GotoXY(snake_point[i].x, snake_point[i].y);
		if (i == 0) textColor(14, 4);
		else textColor(14, 5);

		cout << snake_content[i];
	}
}


//function consists of all tasks needed to do before coming into game
void StartGame() {
	system("cls");
	ResetData(); // Intialize original data
	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGHT_CONSOLE); // Draw game
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

//function to update global data
void Eat() {
	if (SIZE_SNAKE < MAX_SIZE_SNAKE) snake[SIZE_SNAKE] = food[FOOD_INDEX];
	if (FOOD_INDEX == MAX_SIZE_FOOD - 1)
	{
		FOOD_INDEX = 0;
		SIZE_SNAKE = 6;
		if (SPEED == MAX_SPEED) SPEED = 1;
		else SPEED++;
		GenerateFood();
	}
	else {
		FOOD_INDEX++;
		SIZE_SNAKE++;
	}
}

//function to process the dead of snake
void ProcessDead() {
	STATE = 0;
	GotoXY(0, HEIGHT_CONSOLE + 2);
	printf("Dead, type y to continue or anykey to exit");
}

//function to draw
void DrawSnakeAndFood(char* str) {
	GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
	printf(str);
	for (int i = 0; i < SIZE_SNAKE; i++) {
		GotoXY(snake[i].x, snake[i].y);
		printf(str);
	}
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
	if (snake[SIZE_SNAKE - 1].y + 1 == HEIGHT_CONSOLE
		|| IsCollisionWithBody(snake[SIZE_SNAKE - 1].x, snake[SIZE_SNAKE - 1].y + 1)) {
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

void move_snake(char key) { // determine the direction of the snake's movement
	if (key == 'w' && MOVING != 2) MOVING = 1;
	else if (key == 's' && MOVING != 1) MOVING = 2;
	else if (key == 'a' && MOVING != 4) MOVING = 3;
	else if (key == 'd' && MOVING != 3) MOVING = 4;
}

// Animation
void animation_pass_gate(POINT* snake_point, char* snake_content, int move) {
	int n = SIZE_SNAKE;

	textColor(14, 5);
	GotoXY(snake_point[n - 1].x, snake_point[n - 1].y);
	cout << ' ';

	while (true) {
		snake_point[0] = snake_point[1];

		for (int i = n - 1; i > 0; i--)
			snake_point[i] = snake_point[i - 1];

		GotoXY(snake_point[n - 1].x, snake_point[n - 1].y);
		cout << ' ';

		draw_snake(snake_content, snake_point, move);
		Sleep(250);

		if (snake_point[0].x == snake_point[n - 2].x &&
			snake_point[0].y == snake_point[n - 2].y)
			break;
	}
}

void animation_die(POINT* snake_point, char* snake_content, int move) {
	int n = SIZE_SNAKE;

	for (int k = 0; k < 5; k++) {
		for (int i = n - 1; i >= 0; i--) {
			textColor(14, 4);
			GotoXY(snake_point[i].x, snake_point[i].y);
			cout << (i == 0 ? 'X' : ' ');
		}
		Sleep(250);

		draw_snake(snake_content, snake_point, move);
		Sleep(250);
	}
}

void game_over(POINT point_start, POINT point_end, bool& end_while, Sound sound) { // handles the Game Over screen and the “Play Again / Exit” menu options
	bool click = false;
	int answer = 1;
	POINT point_start_box = { point_start.x + 16, point_start.y + 7 };
	POINT point_end_box = { point_end.x - 15, point_end.y - 7 };
	draw_wall(point_start_box, { point_end_box.x, point_end_box.y - 5 }, 254, 10, 10);
	draw_wall({ point_start_box.x + 2, point_start_box.y + 8 }, { point_end_box.x - 28, point_end_box.y }, 175, 4, 4);
	draw_wall({ point_start_box.x + 28, point_start_box.y + 8 }, { point_end_box.x - 2, point_end_box.y }, 174, 4, 4);
	textColor(11, 9);
	GotoXY(point_start_box.x + 9, point_start_box.y + 7); cout << char(179) << char(179);
	GotoXY(point_end_box.x - 10, point_start_box.y + 7); cout << char(179) << char(179);
	while (true) {
		draw_gameover(point_start_box, point_end_box, answer);
		if (_kbhit()) {
			char key = _getch();
			if (key == 'a') {
				if (answer == 2) answer = 1;
			}
			else if (key == 'd') {
				if (answer == 1) answer = 2;
			}
			else if (key == '\r') {
				click = true;
			}
		}
		if (click) {
			if (sound == on) sound_choose();
			Sleep(500);
			if (answer == 1) {
				break;
			}
			else if (answer == 2) {
				end_while = false;
				break;
			}
		}
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
			Sleep(1000 / SPEED);//Sleep function with SPEEED variable
		}
	}
}

// main function
int main() {
	int temp;
	bool end_while = true;
	FixConsoleWindow();
	sound_intro();
	system("cls");
	StartGame();
	off_sound();
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
		if (touch_wall(snake, { 0,0 }, { WIDTH_CONSOLE, HEIGHT_CONSOLE }) || touch_body(snake, (char*)&SIZE_SNAKE)) {
			STATE = 0;
			animation_die(snake, (char*)&SIZE_SNAKE, MOVING);
			sound_gameOver();
			game_over({ 0,0 }, { WIDTH_CONSOLE, HEIGHT_CONSOLE }, end_while, on);

			if (end_while) {
				StartGame();
				off_sound();
			}
			else {
				ExitGame(handle_t1);
				break;
			}
		}
		if (snake[SIZE_SNAKE - 1].x == food[FOOD_INDEX].x && snake[SIZE_SNAKE - 1].y == food[FOOD_INDEX].y) {
			Eat();
			sound_eatFood();
		}
		Sleep(50);
	}
	return 0;
}
