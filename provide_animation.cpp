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

#pragma comment(lib, "winmm.lib")

using namespace std;

typedef struct POINT_POINT {
	int x;
	int y;
}Point;

//constant 
#define MAX_SIZE_SNAKE 10
#define MAX_SIZE_FOOD 4
#define MAX_SPEED 3

//Global variables
POINT snake[10]; //snake
POINT food[4]; // food
int CHAR_LOCK; // used to determine the direction my snake cannot move (At one point, there was a direction my snake could not move to)
int MOVING;// used to determine the direction of movement of the snake (At any given time, there are 3 directions that my snake can move)
int SPEED;// Abbreviation of level, the higher the level the faster the speed  
int HEIGH_CONSOLE, WIDTH_CONSOLE;// Width and height of the monitor screen
int FOOD_INDEX; //Current food index  
int SIZE_SNAKE; // The size of the snake, initially can be 6 units and the maximum size can be 10
int STATE; // The state of the snake: dead or alive

enum Sound { on, off }; // manage game audio status

void gotoXY(int x, int y) // move the console cursor to the coordinate position (x, y)
{
	HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD cursor_an_Pos = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
	SetConsoleCursorPosition(hConsoleOutput, cursor_an_Pos);
}

void textColor(int backgound_color, int text_color) // change text and background color in Windows console
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

	int color_code = backgound_color * 16 + text_color;
	SetConsoleTextAttribute(hStdout, color_code);
}

// sound 
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

// draw 
// draw a wall or rectangle on the console screen
void draw_wall(Point start, Point end, int type_wall, int backgound_color, int text_color, int background_wall_color = 11, int text_wall_color = 9) { 
	textColor(background_wall_color, text_wall_color);
	for (int y = start.y; y <= end.y; y++) {
		gotoXY(start.x - 1, y);
		cout << char(type_wall) << char(type_wall);
	}
	for (int x = start.x; x <= end.x; x++) {
		gotoXY(x, end.y);
		cout << char(type_wall);
	}
	for (int y = end.y; y >= start.y; y--) {
		gotoXY(end.x, y);
		cout << char(type_wall) << char(type_wall);
	}
	for (int x = end.x; x >= start.x; x--) {
		gotoXY(x, start.y);
		cout << char(type_wall);
	}
	for (int x = start.x + 1; x <= end.x - 1; x++) {
		for (int y = start.y + 1; y <= end.y - 1; y++) {
			textColor(backgound_color, text_color);
			gotoXY(x, y);
			cout << ' ';
		}
	}
}

void draw_gameover(Point point_start_box, Point point_end_box, int answer) { // Draw a Game Over screen on the console, with a menu to select “Yes” or “No”
	char play_again_1[] = { char(219),char(223),char(223),char(219),char(32),char(219),char(32),char(32),char(219),char(223),char(223),char(220),char(32),char(219),char(32),char(32),char(219),char(32),char(32),char(32),char(219),char(223),char(223),char(220),char(32),char(219),char(223),char(223),char(223),char(32),char(219),char(223),char(223),char(220),char(32),char(32),char(223),char(32),char(32),char(219),char(223),char(223),char(220),'\0' };
	char play_again_2[] = { char(219),char(220),char(220),char(219),char(32),char(219),char(32),char(32),char(219),char(220),char(220),char(219),char(32),char(219),char(220),char(220),char(219),char(32),char(32),char(32),char(219),char(220),char(220),char(219),char(32),char(219),char(32),char(223),char(220),char(32),char(219),char(220),char(220),char(219),char(32),char(32),char(219),char(223),char(32),char(219),char(32),char(32),char(219),'\0' };
	char play_again_3[] = { char(219),char(32),char(32),char(32),char(32),char(223),char(223),char(32),char(223),char(32),char(32),char(223),char(32),char(220),char(220),char(220),char(223),char(32),char(32),char(32),char(223),char(32),char(32),char(223),char(32),char(223),char(223),char(223),char(223),char(32),char(223),char(32),char(32),char(223),char(32),char(223),char(223),char(223),char(32),char(223),char(32),char(32),char(223),'\0' };
	textColor(10, 0);
	gotoXY(point_start_box.x + 2, point_start_box.y + 2); cout << play_again_1;
	gotoXY(point_start_box.x + 2, point_start_box.y + 3); cout << play_again_2;
	gotoXY(point_start_box.x + 2, point_start_box.y + 4); cout << play_again_3;
	if (answer == 1) textColor(4, 14);
	else textColor(7, 3);
	char yes_1[] = { char(32),char(32),char(223),char(220),char(223),char(32),char(219),char(219),char(223),char(32),char(219),char(223),char(223),char(32),char(32),'\0' };
	char yes_2[] = { char(32),char(32),char(32),char(219),char(32),char(32),char(219),char(220),char(220),char(32),char(220),char(219),char(219),char(32),char(32),'\0' };
	gotoXY(point_start_box.x + 3, point_start_box.y + 9); cout << yes_1;
	gotoXY(point_start_box.x + 3, point_start_box.y + 10); cout << yes_2;
	if (answer == 2) textColor(4, 14);
	else textColor(7, 3);
	char no_1[] = { char(32),char(32),char(32),char(219),char(220),char(32),char(219),char(32),char(220),char(223),char(223),char(220),char(32),char(32),char(32),'\0' };
	char no_2[] = { char(32),char(32),char(32),char(219),char(32),char(223),char(219),char(32),char(223),char(220),char(220),char(223),char(32),char(32),char(32),'\0' };
	gotoXY(point_start_box.x + 23 + 6, point_start_box.y + 9); cout << no_1;
	gotoXY(point_start_box.x + 23 + 6, point_start_box.y + 10); cout << no_2;
}

void draw_snake(char* snake_content, Point* snake_point, int move) { // draw a snake on the console screen
	int num_snake = *(int*)(snake_content - sizeof(int));
	if (move == 3) {
		for (int i = 0; i < num_snake; i++) {
			gotoXY(snake_point[i].x, snake_point[i].y);
			if (i == 0) {
				textColor(14, 4);
			}
			else {
				textColor(14, 5);
			}
			cout << snake_content[i];
		}
	}
	else {
		for (int i = num_snake - 1; i >= 0; i--) {
			gotoXY(snake_point[i].x, snake_point[i].y);
			if (i == 0) {
				textColor(14, 4);
			}
			else {
				textColor(14, 5);
			}
			cout << snake_content[i];
		}
	}
}

// play 
bool gate_passed(Point* snake_point, Point& point_gate) { // check if the snake head goes through the gate
	if (point_gate.x == snake_point[0].x && point_gate.y == snake_point[0].y) return true;
	return false;
}

bool IsValid(int x, int y) { // check if a valid (x,y) position to place food or move does not coincide with the snake's body
	for (int i = 0; i < SIZE_SNAKE; i++) {
		if (snake[i].x == x && snake[i].y == y)
			return false;
	}
	return true;
}

void GenerateFood() { // randomly generate food positions on the screen, making sure the food does not overlap with the snake's body
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

void Eat() { // how to handle snake eating food
	if (SIZE_SNAKE < MAX_SIZE_SNAKE) {
		snake[SIZE_SNAKE] = food[FOOD_INDEX];
		SIZE_SNAKE++;
	}

	if (FOOD_INDEX == MAX_SIZE_FOOD - 1) {
		FOOD_INDEX = 0;
		if (SPEED == MAX_SPEED) {
			SPEED = 1;
			SIZE_SNAKE = 6;
		}
		else {
			SPEED++;
		}
		GenerateFood();
	}
	else {
		FOOD_INDEX++;
	}
}

void move_snake(char key, int& move) { // determine the direction of the snake's movement
	if (key == 'w' && move != 2) move = 1;
	else if (key == 's' && move != 1) move = 2;
	else if (key == 'a' && move != 4) move = 3;
	else if (key == 'd' && move != 3) move = 4;
}

void animation_pass_gate(Point*& snake_point, char* snake_content, int move) { // create animation when snake goes through gate
	int num_snake = *(int*)(snake_content - sizeof(int));
	textColor(14, 5); gotoXY(snake_point[num_snake].x, snake_point[num_snake].y); cout << char(32);
	while (true) {
		snake_point[0].x = snake_point[1].x;
		snake_point[0].y = snake_point[1].y;
		for (int i = num_snake; i > 0; i--) {
			snake_point[i].x = snake_point[i - 1].x;
			snake_point[i].y = snake_point[i - 1].y;
		}
		textColor(14, 5); gotoXY(snake_point[num_snake].x, snake_point[num_snake].y); cout << char(32);
		for (int i = 0; i < num_snake; i++) {
			gotoXY(snake_point[i].x, snake_point[i].y);
			if (i == 0) {
				textColor(14, 4);
			}
			else {
				textColor(14, 5);
			}
			cout << snake_content[i];
		}
		Sleep(250);
		if (snake_point[0].x == snake_point[num_snake - 1].x && snake_point[0].y == snake_point[num_snake - 1].y) {
			break;
		}
	}
}

void animation_die(Point*& snake_point, char* snake_content, int move) { // create an animation when the snake dies
	int num_snake = *(int*)(snake_content - sizeof(int));
	textColor(14, 5); gotoXY(snake_point[num_snake].x, snake_point[num_snake].y); cout << char(32);
	for (int i = 0; i < 5; i++) {;
		for (int j = num_snake - 1; j >= 0; j--) {
			textColor(14, 4); gotoXY(snake_point[j].x, snake_point[j].y);
			if (j == 0) cout << 'X';
			else cout << ' ';
		}
		Sleep(250);
		draw_snake(snake_content, snake_point, move);
		Sleep(250);
	}
}

// game over 
bool touch_wall(Point* snake_point, Point point_start, Point point_end) { // check to see if the snake's head is touching the wall
	if (snake_point[0].x == point_start.x || snake_point[0].x == point_end.x) return true;
	if (snake_point[0].y == point_start.y || snake_point[0].y == point_end.y) return true;
	return false;
}

bool touch_body(Point* snake_point, char* snake_content) { // check to see if the snake's head is touching the snake's body
	int num_snake = *(int*)(snake_content - sizeof(int));
	if (num_snake < 5) return false;
	for (int i = 4; i < num_snake; i++) {
		if (snake_point[0].x == snake_point[i].x && snake_point[0].y == snake_point[i].y) return true;
	}
	return false;
}

bool touch_obstacle(Point* snake_point, vector<Point>& obstacles) { // check to see if the snake's head is touching an obstacle
	for (auto& ob : obstacles) {
		if (snake_point[0].x == ob.x && snake_point[0].y == ob.y)
			return true;
	}
	return false;
}

bool touch_wallgate(Point* snake_point, vector<Point>& gatewall) { // check if the snake head is touching the gate cells
	for (auto& ob : gatewall) {
		if (snake_point[0].x == ob.x && snake_point[0].y == ob.y)
			return true;
	}
	return false;
}

void game_over(Point point_start, Point point_end, bool& end_while, Sound sound) { // handles the Game Over screen and the “Play Again / Exit” menu options
	bool click = false;
	int answer = 1;
	Point point_start_box = { point_start.x + 16, point_start.y + 7 };
	Point point_end_box = { point_end.x - 15, point_end.y - 7 };
	draw_wall(point_start_box, { point_end_box.x, point_end_box.y - 5 }, 254, 10, 10);
	draw_wall({ point_start_box.x + 2, point_start_box.y + 8 }, { point_end_box.x - 28, point_end_box.y }, 175, 4, 4);
	draw_wall({ point_start_box.x + 28, point_start_box.y + 8 }, { point_end_box.x - 2, point_end_box.y }, 174, 4, 4);
	textColor(11, 9);
	gotoXY(point_start_box.x + 9, point_start_box.y + 7); cout << char(179) << char(179);
	gotoXY(point_end_box.x - 10, point_start_box.y + 7); cout << char(179) << char(179);
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



