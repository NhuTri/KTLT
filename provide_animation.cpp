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

//Hang so 
#define MAX_SIZE_SNAKE 10
#define MAX_SIZE_FOOD 4
#define MAX_SPEED 3

//Bien toan cuc 
POINT snake[10]; //ran 
POINT food[4]; // thuc an
int CHAR_LOCK; // duoc su dung de xac dinh huong ma con ran cua toi khong the di chuyen (Tai mot thoi diem, co mot huong ma con ran cua toi khong the di chuyen toi) 
int MOVING;// duoc su dung de xac dinh huong di chuyen cua con ran (Tai mot thoi diem, co 3 huong ma con ran cua toi co the di chuyen)
int SPEED;// Viet tat cua level, level cang cao thi toc do cang nhanh  
int HEIGH_CONSOLE, WIDTH_CONSOLE;// Chieu rong va chieu cao cua man hinh dieu khien 
int FOOD_INDEX; // Chi muc thuc pham hien tai  
int SIZE_SNAKE; // Kich thuoc cua con ran, ban dau co the la 6 don vi va kich thuoc toi da co the la 10 
int STATE; // Trang thai cua con ran: chet hoac song 

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
