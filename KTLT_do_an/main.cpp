#include <iostream>
#include <vector>
#include <windows.h>
#include <string>
#include <conio.h>
#include <ctime>
#include <fstream>
#include <thread>

using namespace std;

// --- CONSTANTS ---
// Maximum size of the snake array
#define MAX_SIZE_SNAKE 20
// Maximum number of food items
#define MAX_SIZE_FOOD 4
// Maximum speed level
#define MAX_SPEED 3
// Default console width
#define DEFAULT_WIDTH 70
// Default console height
#define DEFAULT_HEIGHT 20

// Define Colors
// Background color definition (White = Red + Green + Blue)
const int BG_COLOR = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
// Normal text color (Black)
const int COLOR_NORMAL = 0;
// Alert text color (Red)
const int COLOR_ALERT = FOREGROUND_RED | FOREGROUND_INTENSITY;

// Requirement 4.6: Student IDs used for snake body
const string TEAM_IDS = "221203792212039123120234";

// --- GLOBAL VARIABLES ---
struct Point { int x, y; };

// Snake body array
POINT snake[MAX_SIZE_SNAKE];
// Food array
POINT food[MAX_SIZE_FOOD];
// Gate coordinates
POINT gate[3];
// Wall coordinates around the gate
POINT wall[4];

// Direction locked (cannot move back to this direction)
int CHAR_LOCK;
// Current moving direction
int MOVING;
// Current level/speed
int SPEED;
// Width and Height of the console
int HEIGH_CONSOLE, WIDTH_CONSOLE;
// Index of the current food being targeted
int FOOD_INDEX;
// Current length of the snake
int SIZE_SNAKE;
// Game state: 0 = Dead, 1 = Alive
int STATE;
// Status of the gate (Active or Inactive)
bool GATE_ACTIVE;
// Current Game Level
int LEVEL = 1;

// --- CONSOLE HELPER FUNCTIONS ---

// Function to disable resizing and maximizing the console window
void FixConsoleWindow() {
    HWND consoleWindow = GetConsoleWindow();
    LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
    // Remove maximize box and thick frame (resizing)
    style = style & ~(WS_MAXIMIZEBOX) & ~(WS_THICKFRAME);
    SetWindowLong(consoleWindow, GWL_STYLE, style);
}

// Function to move the cursor to a specific (x, y) coordinate
void GotoXY(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Function to set text color while maintaining the white background
void SetColor(int textColor) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, textColor | BG_COLOR);
}

// Function to clear the console screen
void ClearScreen() {
    ::system("cls"); // Use global scope system to avoid ambiguity
}

// --- GAME LOGIC HELPERS ---

// Function to check if a coordinate (x, y) overlaps with the snake's body
bool IsValid(int x, int y) {
    for (int i = 0; i < SIZE_SNAKE; i++) {
        if (snake[i].x == x && snake[i].y == y)
            return false;
    }
    return true;
}

// Function to generate random food positions ensuring no overlap with snake
void GenerateFood() {
    int x, y;
    srand((unsigned int)time(NULL));
    for (int i = 0; i < MAX_SIZE_FOOD; i++) {
        do {
            x = rand() % (WIDTH_CONSOLE - 1) + 1;
            y = rand() % (HEIGH_CONSOLE - 1) + 1;
        } while (IsValid(x, y) == false);
        food[i] = { x, y };
    }
}

// Function to reset game data to initial state
void ResetData() {
    CHAR_LOCK = 'A';
    MOVING = 'D';
    SPEED = 1;
    FOOD_INDEX = 0;
    WIDTH_CONSOLE = DEFAULT_WIDTH;
    HEIGH_CONSOLE = DEFAULT_HEIGHT;
    SIZE_SNAKE = 6;

    // Initialize snake position
    snake[0] = { 10, 5 }; snake[1] = { 11, 5 };
    snake[2] = { 12, 5 }; snake[3] = { 13, 5 };
    snake[4] = { 14, 5 }; snake[5] = { 15, 5 };

    GenerateFood();
}

// Function to update the snake's body segments (shift coordinates)
void UpdateSnakeBody() {
    for (int i = 0; i < SIZE_SNAKE - 1; i++) {
        snake[i].x = snake[i + 1].x;
        snake[i].y = snake[i + 1].y;
    }
}

// Function to check if the head collides with the body
bool IsCollisionWithBody(int x, int y) {
    for (int i = 0; i < SIZE_SNAKE - 1; i++) {
        if (snake[i].x == x && snake[i].y == y)
            return true;
    }
    return false;
}

// --- DRAWING FUNCTIONS ---

// Function to draw the game borders
void DrawBoard(int x, int y, int width, int height) {
    GotoXY(x, y); cout << 'X';
    for (int i = 1; i < width; i++) cout << 'X';
    cout << 'X';

    GotoXY(x, height + y); cout << 'X';
    for (int i = 1; i < width; i++) cout << 'X';
    cout << 'X';

    for (int i = y + 1; i < height + y; i++) {
        GotoXY(x, i); cout << 'X';
        GotoXY(x + width, i); cout << 'X';
    }
}

// Function to draw the gate structure
void DrawGate() {
    // Draw walls of the gate
    for (int i = 0; i < 4; i++) {
        GotoXY(wall[i].x, wall[i].y); cout << "0";
    }
    // Draw the gate passage
    for (int i = 0; i < 3; i++) {
        GotoXY(gate[i].x, gate[i].y); cout << "0";
    }
}

// Function to draw the snake and food on the screen
void DrawSnakeAndFood(const string& str) {
    // Draw Food
    if (!GATE_ACTIVE && FOOD_INDEX >= 0 && FOOD_INDEX < MAX_SIZE_FOOD) {
        GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
        cout << str;
    }

    // Draw Snake
    for (int i = 0; i < SIZE_SNAKE; i++) {
        GotoXY(snake[i].x, snake[i].y);
        if (str == " ") {
            cout << " ";
        }
        else {
            // Requirement 4.6: Print Student IDs on snake body
            cout << TEAM_IDS[i % TEAM_IDS.length()];
        }
    }
}

// Function to handle the animation of snake going through the gate
void DrawSnakeGoThroughGate() {
    int cnt = SIZE_SNAKE;
    while (cnt > 0) {
        GotoXY(snake[0].x, snake[0].y);
        cout << " "; // Clear tail

        // Shift snake body forward for animation
        for (int i = 0; i < cnt - 1; i++) {
            snake[i].x = snake[i + 1].x;
            snake[i].y = snake[i + 1].y;
            GotoXY(snake[i].x, snake[i].y);
            cout << TEAM_IDS[i % TEAM_IDS.length()];
        }
        cnt--;
        Sleep(300 / SPEED);
    }
}

// Function to play visual effect when snake dies (Blinking Red)
void PlayDeadEffect() {
    for (int k = 0; k < 3; k++) {
        // Set color to RED (Alert)
        SetColor(COLOR_ALERT);
        for (int i = 0; i < SIZE_SNAKE; i++) {
            GotoXY(snake[i].x, snake[i].y);
            cout << TEAM_IDS[i % TEAM_IDS.length()];
        }
        Sleep(200);

        // Set color back to NORMAL (Black)
        SetColor(COLOR_NORMAL);
        for (int i = 0; i < SIZE_SNAKE; i++) {
            GotoXY(snake[i].x, snake[i].y);
            cout << TEAM_IDS[i % TEAM_IDS.length()];
        }
        Sleep(200);
    }
    SetColor(COLOR_NORMAL);
}

// Function to process game over state
void ProcessDead() {
    STATE = 0;
    PlayDeadEffect();
    GotoXY(0, HEIGH_CONSOLE + 2);
    SetColor(COLOR_ALERT);
    cout << "GAME OVER! Press 'y' to continue or any key to exit.";
    SetColor(COLOR_NORMAL);
}

// --- GAME FLOW CONTROL ---

// Function to activate the gate coordinates
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

// Function to handle eating food logic
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

// Function to start the game
void StartGame() {
    ClearScreen();
    ResetData();
    DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
    STATE = 1;
}

// Function to exit the game and terminate thread
void ExitGame(HANDLE t) {
    ClearScreen();
    TerminateThread(t, 0);
}

// Function to pause the game
void PauseGame(HANDLE t) {
    SuspendThread(t);
}

// Function to handle Game Win scenario
void GameWin() {
    ClearScreen();
    GotoXY(20, 10);
    cout << "YOU WIN!" << endl;
    exit(0);
}

// Function to proceed to the next level
// Requirement 4.3: Keep length unchanged
void NextLevel() {
    LEVEL++;
    SPEED++;
    GATE_ACTIVE = false;

    if (LEVEL > MAX_SPEED) {
        GameWin();
        return;
    }

    ClearScreen();
    CHAR_LOCK = 'A'; MOVING = 'D';
    FOOD_INDEX = 0;
    WIDTH_CONSOLE = DEFAULT_WIDTH;
    HEIGH_CONSOLE = DEFAULT_HEIGHT;

    // Reposition snake at starting point (keeping current length)
    for (int i = 0; i < SIZE_SNAKE; i++) {
        snake[i].x = 10 + i;
        snake[i].y = 5;
    }

    GenerateFood();
    DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
    STATE = 1;
}

// --- MOVEMENT LOGIC ---

// Function to check if snake enters the gate
bool CheckGateEntry(int nextX, int nextY) {
    if (!GATE_ACTIVE) return false;
    for (int i = 0; i < 3; i++) {
        if (nextX == gate[i].x && nextY == gate[i].y) return true;
    }
    return false;
}

// Function to check collision with walls (including gate walls)
bool CheckWallCollision(int nextX, int nextY) {
    // Normal board walls
    if (nextX == WIDTH_CONSOLE || nextX == 0 || nextY == HEIGH_CONSOLE || nextY == 0) return true;

    // Gate walls (only if gate is active)
    if (GATE_ACTIVE) {
        for (int i = 0; i < 4; i++) {
            if (nextX == wall[i].x && nextY == wall[i].y) return true;
        }
    }
    return false;
}

// Function to move Snake to the Right
void MoveRight() {
    int nextX = snake[SIZE_SNAKE - 1].x + 1;
    int nextY = snake[SIZE_SNAKE - 1].y;

    if (CheckWallCollision(nextX, nextY) || IsCollisionWithBody(nextX, nextY)) {
        ProcessDead();
    }
    else if (CheckGateEntry(nextX, nextY)) {
        DrawSnakeGoThroughGate();
        NextLevel();
    }
    else {
        if (nextX == food[FOOD_INDEX].x && nextY == food[FOOD_INDEX].y) {
            Eat();
        }
        else {
            UpdateSnakeBody();
            snake[SIZE_SNAKE - 1].x++;
        }
    }
}

// Function to move Snake to the Left
void MoveLeft() {
    int nextX = snake[SIZE_SNAKE - 1].x - 1;
    int nextY = snake[SIZE_SNAKE - 1].y;

    if (CheckWallCollision(nextX, nextY) || IsCollisionWithBody(nextX, nextY)) {
        ProcessDead();
    }
    else {
        if (nextX == food[FOOD_INDEX].x && nextY == food[FOOD_INDEX].y) {
            Eat();
        }
        else {
            UpdateSnakeBody();
            snake[SIZE_SNAKE - 1].x--;
        }
    }
}

// Function to move Snake Down
void MoveDown() {
    int nextX = snake[SIZE_SNAKE - 1].x;
    int nextY = snake[SIZE_SNAKE - 1].y + 1;

    if (CheckWallCollision(nextX, nextY) || IsCollisionWithBody(nextX, nextY)) {
        ProcessDead();
    }
    else {
        if (nextX == food[FOOD_INDEX].x && nextY == food[FOOD_INDEX].y) {
            Eat();
        }
        else {
            UpdateSnakeBody();
            snake[SIZE_SNAKE - 1].y++;
        }
    }
}

// Function to move Snake Up
void MoveUp() {
    int nextX = snake[SIZE_SNAKE - 1].x;
    int nextY = snake[SIZE_SNAKE - 1].y - 1;

    if (CheckWallCollision(nextX, nextY) || IsCollisionWithBody(nextX, nextY)) {
        ProcessDead();
    }
    else {
        if (nextX == food[FOOD_INDEX].x && nextY == food[FOOD_INDEX].y) {
            Eat();
        }
        else {
            UpdateSnakeBody();
            snake[SIZE_SNAKE - 1].y--;
        }
    }
}

// Function for the separate thread handling snake movement
void ThreadFunc() {
    while (1) {
        if (STATE == 1) {
            DrawSnakeAndFood(" ");
            switch (MOVING) {
            case 'A': MoveLeft(); break;
            case 'D': MoveRight(); break;
            case 'W': MoveUp(); break;
            case 'S': MoveDown(); break;
            }
            DrawSnakeAndFood("O");
            Sleep(300 / SPEED);
        }
    }
}

// --- SAVE / LOAD ---

// Function to save game state to a binary file
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
    file.write((char*)&LEVEL, sizeof(int));
    file.write((char*)snake, sizeof(POINT) * MAX_SIZE_SNAKE);
    file.write((char*)food, sizeof(POINT) * MAX_SIZE_FOOD);
    file.close();

    GotoXY(0, HEIGH_CONSOLE + 3);
    cout << "Game saved to " << filename << " successfully! Press any key to continue.";
    _getch();
}

// Function to load game state from a binary file
bool LoadGame(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) return false;

    file.read((char*)&SIZE_SNAKE, sizeof(int));
    file.read((char*)&SPEED, sizeof(int));
    file.read((char*)&FOOD_INDEX, sizeof(int));
    file.read((char*)&CHAR_LOCK, sizeof(int));
    file.read((char*)&MOVING, sizeof(int));
    file.read((char*)&WIDTH_CONSOLE, sizeof(int));
    file.read((char*)&HEIGH_CONSOLE, sizeof(int));
    file.read((char*)&LEVEL, sizeof(int));
    file.read((char*)snake, sizeof(POINT) * MAX_SIZE_SNAKE);
    file.read((char*)food, sizeof(POINT) * MAX_SIZE_FOOD);
    file.close();
    return true;
}

// --- MAIN CONTROLLER ---

// Function to handle keyboard input and main game loop
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
            else if (temp == 27) { // ESC Key
                ExitGame(handle_t1);
                t1.detach();
                return;
            }
            else if (temp == 'L') {
                PauseGame(handle_t1);
                GotoXY(0, HEIGH_CONSOLE + 2);
                string filename;
                cout << "Save game, enter filename: ";
                cin >> filename;
                SaveGame(filename);
                ClearScreen();
                DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
                ResumeThread(handle_t1);
            }
            else if (temp == 'T') {
                PauseGame(handle_t1);
                GotoXY(0, HEIGH_CONSOLE + 2);
                string filename;
                cout << "Load game, enter filename: ";
                cin >> filename;
                if (LoadGame(filename)) {
                    ClearScreen();
                    DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
                    DrawSnakeAndFood("O");
                    ResumeThread(handle_t1);
                }
                else {
                    cout << "\nLoad failed! Press any key.";
                    _getch();
                    ClearScreen();
                    DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
                    ResumeThread(handle_t1);
                }
            }
            else {
                ResumeThread(handle_t1);
                if ((temp != CHAR_LOCK) && (temp == 'D' || temp == 'A' || temp == 'W' || temp == 'S')) {
                    if (temp == 'D') CHAR_LOCK = 'A';
                    else if (temp == 'W') CHAR_LOCK = 'S';
                    else if (temp == 'S') CHAR_LOCK = 'W';
                    else CHAR_LOCK = 'D';
                    MOVING = temp;
                }
            }
        }
        else { // Game Over State handling
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

// Function to display the Main Menu
void MainMenu() {
    int choice;
    while (true) {
        ClearScreen();
        // Set white background for menu
        SetColor(COLOR_NORMAL);
        cout << "==========================\n";
        cout << "       SNAKE GAME\n";
        cout << "==========================\n";
        cout << "1. New Game\n2. Load Game\n3. Exit\n";
        cout << "Your choice: ";
        cin >> choice;

        switch (choice) {
        case 1:
            StartGame();
            PlayGame();
            break;
        case 2: {
            string filename;
            cout << "Filename: ";
            cin >> filename;
            if (LoadGame(filename)) {
                ClearScreen();
                DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
                STATE = 1;
                PlayGame();
            }
            else {
                cout << "\nLoad failed! Press any key.";
                _getch();
            }
            break;
        }
        case 3: exit(0);
        }
    }
}

// Main function - Entry point of the program
int main() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h, BG_COLOR); // Initialize White Background
    FixConsoleWindow();
    MainMenu();
    return 0;
}
