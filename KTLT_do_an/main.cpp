#include <iostream>
#include <vector>
#include <windows.h>
#include <conio.h>
#include <ctime>
#include <fstream>
#include <thread>

#pragma comment(lib, "winmm.lib")

using namespace std;

// --- CONSTANTS ---
// Maximum size of the snake array
#define MAX_SIZE_SNAKE 100
// Maximum number of food items
#define MAX_SIZE_FOOD 4
// Maximum speed level
#define MAX_SPEED 3
// Default console width
#define DEFAULT_WIDTH 40
// Default console height
#define DEFAULT_HEIGHT 20

#define STATS_WIDTH 35

// Define Colors
// Background color definition (White = Red + Green + Blue)
const int BG_COLOR = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
// Normal text color (Black)
const int COLOR_NORMAL = 0;
// Alert text color (Red)
const int COLOR_ALERT = FOREGROUND_RED | FOREGROUND_INTENSITY;


// Requirement 4.6: Student IDs used for snake body
const string TEAM_IDS = "221203792212039123120234";

enum GateDirection {
    GATE_LEFT,
    GATE_RIGHT,
    GATE_TOP,
    GATE_BOTTOM
};


GateDirection GATE_DIR;

// --- GLOBAL VARIABLES ---
POINT snake[MAX_SIZE_SNAKE];// Snake body array

POINT food[MAX_SIZE_FOOD];// Food array

POINT gate[3];// Gate coordinates

POINT wall[4];// Wall coordinates around the gate

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
// Status of the gate (Active or Inactive)
bool GATE_ACTIVE;
// Current Game Level
int LEVEL = 1;

volatile bool IS_RUNNING = true;

volatile int STATE; // Nên thêm cả cho STATE cho chắc chắn

POINT GetGateExitPoint() {
    switch (GATE_DIR) {
    case GATE_LEFT:   return { WIDTH_CONSOLE - 1, gate[1].y };
    case GATE_RIGHT:  return { 1, gate[1].y };
    case GATE_TOP:    return { gate[1].x, HEIGH_CONSOLE - 1 };
    case GATE_BOTTOM: return { gate[1].x, 1 };
    }
    return gate[1];
}

bool GOD_MODE = false; // Mặc định là tắt
// --- THÊM BIẾN NÀY ---
bool SOUND_ON = true; // Mặc định là Bật âm thanh

// Hàm này sẽ kiểm tra xem SOUND_ON có bật không rồi mới phát
void PlayAudio(const char* soundFile, bool loop = false) {
    if (!SOUND_ON) {
        // Nếu đang tắt tiếng thì dừng mọi âm thanh đang phát
        PlaySound(NULL, 0, 0);
        return;
    }

    DWORD flags = SND_FILENAME | SND_ASYNC;
    if (loop) flags |= SND_LOOP; // Nếu cần lặp (cho nhạc nền)

    PlaySoundA(soundFile, NULL, flags);
}

// --- CONSOLE HELPER FUNCTIONS ---

// Function to disable resizing and maximizing the console window
void FixConsoleWindow() {
    HWND consoleWindow = GetConsoleWindow();
    LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
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

void ShowCur(bool CursorVisibility) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursor = { 1, CursorVisibility };
    SetConsoleCursorInfo(handle, &cursor);
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
    int x = 0;
    int y = 0;
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

    // --- THÊM 2 DÒNG NÀY ---
    LEVEL = 1;          // Reset Level về 1
    GATE_ACTIVE = false;// Đảm bảo cổng đóng khi bắt đầu game mới
    // -----------------------

    // Initialize snake position
    for (int i = 0; i < SIZE_SNAKE; i++) {
        snake[i].x = 10 + i;
        snake[i].y = HEIGH_CONSOLE / 2;
    }

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
    for (int i = 0; i < SIZE_SNAKE - 4; i++) { //đầu rắn không thể chạm vào 4 phần tử cuối cùng
        if (snake[i].x == x && snake[i].y == y)
            return true;
    }

    return false;
}

// --- DRAWING FUNCTIONS ---

// Function to draw the game borders
// Hàm vẽ khung giao diện chia đôi màn hình
void DrawBoard(int x, int y, int width, int height) {
    int totalWidth = width + STATS_WIDTH;

    // 1. Vẽ cạnh TRÊN (dùng ký tự 205 '═')
    GotoXY(x, y);
    cout << (char)201; // Góc trên trái
    for (int i = 1; i < totalWidth; i++) {
        if (i == width) cout << (char)203; // Ngã 3 chia khung (╦)
        else cout << (char)205;            // Đường ngang
    }
    cout << (char)187; // Góc trên phải

    // 2. Vẽ cạnh DƯỚI (dùng ký tự 205 '═')
    GotoXY(x, height + y);
    cout << (char)200; // Góc dưới trái
    for (int i = 1; i < totalWidth; i++) {
        if (i == width) cout << (char)202; // Ngã 3 chia khung (╩)
        else cout << (char)205;            // Đường ngang
    }
    cout << (char)188; // Góc dưới phải

    // 3. Vẽ 3 đường dọc: Trái, Giữa, Phải (dùng ký tự 186 '║')
    for (int i = y + 1; i < height + y; i++) {
        GotoXY(x, i); cout << (char)186;             // Tường trái
        GotoXY(x + width, i); cout << (char)186;     // Tường GIỮA (Ngăn cách)
        GotoXY(x + totalWidth, i); cout << (char)186;// Tường phải
    }

}

// Function to draw the gate structure
void DrawGate() {
    char wallChar;

    // Chọn ký tự tường theo hướng
    if (GATE_DIR == GATE_LEFT || GATE_DIR == GATE_RIGHT)
        wallChar = (char)205;   // ║ (tường dọc)
    else
        wallChar = (char)186;   // ═ (tường ngang)

    // Vẽ tường bảo vệ cổng
    for (int i = 0; i < 4; i++) {
        GotoXY(wall[i].x, wall[i].y);
        cout << wallChar;
    }

    // Vẽ lối cổng
    for (int i = 0; i < 3; i++) {
        GotoXY(gate[i].x, gate[i].y);
        cout << " ";
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
    STATE = 3; // khóa game
    int cnt = SIZE_SNAKE;

    PlayAudio("stage_clear.wav");

    while (cnt > 0) {
        // 1. Xóa đuôi
        GotoXY(snake[0].x, snake[0].y);
        cout << " ";

        // 2. Dồn thân rắn
        for (int i = 0; i < cnt - 1; i++) {
            snake[i] = snake[i + 1];

            GotoXY(snake[i].x, snake[i].y);
            cout << TEAM_IDS[i % TEAM_IDS.length()];
        }


        cnt--;
        Sleep(max(50, 200 - (SPEED * 20)));
    }
}

// Hàm vẽ bảng thông tin bên phải màn hình
// Hàm vẽ bảng thông tin (Giao diện Clean & Modern)
// Hàm vẽ bảng thông tin (Tối ưu cho NỀN TRẮNG & Màn hình thấp)
void DrawInfoBoard() {
    int x = WIDTH_CONSOLE + 1;
    int y = 2;

    // 1. Xóa sạch vùng info cũ
    for (int i = 2; i < HEIGH_CONSOLE; i++) {
        GotoXY(x, i);
        for (int j = 0; j < STATS_WIDTH - 1; j++) cout << " ";
    }

    // --- TIÊU ĐỀ: GAME STATS ---
    SetColor(1); // Xanh dương đậm (Blue)
    GotoXY(x + 12, y++); cout << "GAME STATS";
    SetColor(0); // Màu Đen (Black) cho đường kẻ
    GotoXY(x + 5, y++); for (int i = 0; i < 25; i++) cout << (char)196;

    // Căn lề
    int labelX = x + 6;
    int colonX = x + 12;
    int valueX = x + 14;

    // --- CÁC CHỈ SỐ (Màu đậm) ---
    // Level
    SetColor(0);  GotoXY(labelX, y); cout << "Level"; // Đen
    SetColor(0);  GotoXY(colonX, y); cout << ":";
    SetColor(4);  GotoXY(valueX, y++); cout << LEVEL; // Đỏ đậm (Red)

    // Score
    SetColor(0);  GotoXY(labelX, y); cout << "Score";
    SetColor(0);  GotoXY(colonX, y); cout << ":";
    SetColor(2);  GotoXY(valueX, y++); cout << SIZE_SNAKE; // Xanh lá đậm (Green)

    // Food
    SetColor(0);  GotoXY(labelX, y); cout << "Food";
    SetColor(0);  GotoXY(colonX, y); cout << ":";
    SetColor(5);  GotoXY(valueX, y++); cout << FOOD_INDEX << " / " << MAX_SIZE_FOOD; // Tím (Purple)

    // Speed
    SetColor(0);  GotoXY(labelX, y); cout << "Speed";
    SetColor(0);  GotoXY(colonX, y); cout << ":";
    SetColor(1);  GotoXY(valueX, y++); cout << SPEED; // Xanh dương

    y++; // Dòng trống

    // --- CONTROLS (Gom nhóm để tiết kiệm dòng) ---
    SetColor(1); // Xanh dương đậm
    GotoXY(x + 13, y++); cout << "CONTROLS";
    SetColor(0); // Đen
    GotoXY(x + 5, y++); for (int i = 0; i < 25; i++) cout << (char)196;

    // Move
    SetColor(0); GotoXY(labelX, y); cout << "Move";
    SetColor(0); GotoXY(colonX, y); cout << ":";
    SetColor(8); GotoXY(valueX, y++); cout << "W A S D"; // Xám đậm

    // Menu Keys (Gộp Pause, Save, Load vào 1 dòng)
    SetColor(0); GotoXY(labelX, y); cout << "Menu";
    SetColor(0); GotoXY(colonX, y); cout << ":";
    SetColor(8); GotoXY(valueX, y++); cout << "P, L, T, ESC";

    y++; // Dòng trống

    // --- CHEATS (Gom nhóm) ---
    SetColor(4); // Đỏ đậm (Red) - Cảnh báo Cheat
    GotoXY(x + 14, y++); cout << "CHEATS";
    SetColor(0);
    GotoXY(x + 5, y++); for (int i = 0; i < 25; i++) cout << (char)196;

    // Gộp God, Grow, Next vào 2 dòng
    labelX = x + 5;

    // Dòng 1: God & Grow
    SetColor(0); GotoXY(labelX, y); cout << "[G]od Mode   [+] Grow";
    y++;

    // Dòng 2: Next Level
    SetColor(0); GotoXY(labelX, y); cout << "[N]ext Level";
    y++;

    // --- TRẠNG THÁI GOD MODE ---
    if (GOD_MODE) {
        y++; // Xuống 1 dòng cho thoáng
        SetColor(4); // Đỏ đậm
        GotoXY(x + 8, y++); cout << "<< GOD MODE ON >>";
    }

    SetColor(COLOR_NORMAL); // Reset về mặc định
}

// Hàm vẽ chiếc cúp bằng ký tự ASCII
// Hàm vẽ Cúp (Tối ưu cho nền trắng)
void DrawTrophy(int x, int y) {
    SetColor(6); // Màu Vàng đất (Dark Yellow/Gold)
    GotoXY(x, y);     cout << "  ___________  ";
    GotoXY(x, y + 1); cout << " '._==_==_=_.' ";
    GotoXY(x, y + 2); cout << " .-\\:      /-. ";

    SetColor(0); // Màu Đen để tạo độ tương phản cho thân cúp
    GotoXY(x, y + 3); cout << "| (|:.     |) |";

    SetColor(6); // Vàng đất
    GotoXY(x, y + 4); cout << " '-|:.     |-' ";
    GotoXY(x, y + 5); cout << "   \\::.    /   ";
    GotoXY(x, y + 6); cout << "    '::. .'    ";
    GotoXY(x, y + 7); cout << "      ) (      ";
    GotoXY(x, y + 8); cout << "    _.' '._    ";
    GotoXY(x, y + 9); cout << "   `\"\"\"\"\"\"\"`   ";
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
    if (STATE == 3) return;

    STATE = 0;

    PlayAudio(NULL);
    PlayAudio("die.wav");

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
    int type;
    bool overlap;

    int wc = WIDTH_CONSOLE;
    int hc = HEIGH_CONSOLE;
    int midX = wc / 2;
    int midY = hc / 2;

    do {
        srand((unsigned int)time(NULL));
        overlap = false;
        type = rand() % 4; // Random 1 trong 4 hướng

        // --- TÍNH TOÁN TỌA ĐỘ DỰA VÀO HƯỚNG ---
        switch (type) {
        case 0: // RIGHT (Cổng Phải)
            GATE_DIR = GATE_RIGHT;
            gate[0] = { wc, midY - 1 };
            gate[1] = { wc, midY };
            gate[2] = { wc, midY + 1 };
            wall[0] = { wc - 1, midY - 2 }; wall[1] = { wc , midY - 2 };
            wall[2] = { wc - 1, midY + 2 }; wall[3] = { wc , midY + 2 };
            break;
        case 1: // LEFT (Cổng Trái)
            GATE_DIR = GATE_LEFT;
            gate[0] = { 0, midY - 1 };
            gate[1] = { 0, midY };
            gate[2] = { 0, midY + 1 };
            wall[0] = { 1, midY - 2 }; wall[1] = { 0, midY - 2 };
            wall[2] = { 1, midY + 2 }; wall[3] = { 0, midY + 2 };
            break;
        case 2: // TOP (Cổng Trên)
            GATE_DIR = GATE_TOP;
            gate[0] = { midX - 1, 0 };
            gate[1] = { midX, 0 };
            gate[2] = { midX + 1, 0 };
            wall[0] = { midX - 2, 1 }; wall[1] = { midX - 2, 0 };
            wall[2] = { midX + 2, 1 }; wall[3] = { midX + 2, 0 };
            break;
        case 3: // BOTTOM (Cổng Dưới)
            GATE_DIR = GATE_BOTTOM;
            gate[0] = { midX - 1, hc };
            gate[1] = { midX, hc };
            gate[2] = { midX + 1, hc };
            wall[0] = { midX - 2, hc - 1 }; wall[1] = { midX - 2, hc };
            wall[2] = { midX + 2, hc - 1 }; wall[3] = { midX + 2, hc };
            break;
        }

        // --- SỬA LỖI LOGIC TẠI ĐÂY ---
        // Chỉ khi IsValid trả về false (đụng rắn) thì mới lặp lại
        for (int i = 0; i < 3; i++) {
            if (IsValid(gate[i].x, gate[i].y) == false) overlap = true;
        }
        for (int i = 0; i < 4; i++) {
            if (IsValid(wall[i].x, wall[i].y) == false) overlap = true;
        }

    } while (overlap);
}
// Function to handle eating food logic
// Hàm Eat đã sửa lỗi hiển thị và logic đếm
void Eat() {
    snake[SIZE_SNAKE] = food[FOOD_INDEX];
    SIZE_SNAKE++;
    PlayAudio("eat.wav");

    // 1. Luôn tăng chỉ số và cập nhật bảng điểm TRƯỚC
    FOOD_INDEX++;
    DrawInfoBoard();

    // 2. Sau đó mới kiểm tra xem đã đủ thức ăn để mở cổng chưa
    if (FOOD_INDEX >= MAX_SIZE_FOOD) {
        GATE_ACTIVE = true;
        ActivateGate();
        DrawGate();
    }
}

void SettingsMenu() {
    PlayAudio("bgm.wav", true);
    char choice;
    while (true) {
        ClearScreen();
        SetColor(11); // Màu xanh dương nhạt
        cout << "==========================\n";
        cout << "        SETTINGS\n";
        cout << "==========================\n";

        SetColor(COLOR_NORMAL);
        // Hiển thị trạng thái ON/OFF
        cout << "1. Sound: ";
        if (SOUND_ON) {
            SetColor(10); // Màu xanh lá
            cout << "ON  [Enabled]";
        }
        else {
            SetColor(12); // Màu đỏ
            cout << "OFF [Muted]  ";
        }

        SetColor(COLOR_NORMAL);
        cout << "\n2. Back to Menu\n";
        cout << "--------------------------\n";
        cout << "Press key to select: ";

        choice = _getch();

        if (choice == '1') {
            SOUND_ON = !SOUND_ON; // Đảo trạng thái

            if (SOUND_ON) {
                PlayAudio("choose.wav");
                // Mẹo: Sau tiếng choose, nhạc nền sẽ tắt. 
                // Nếu muốn nhạc nền chạy lại ngay thì cần gọi lại:
                // PlayAudio("bgm.wav", true); 
            }
            else {
                PlayAudio(NULL); // Tắt hẳn nếu chọn OFF
            }
        }
        else if (choice == '2' || choice == 27) {
            PlayAudio("choose.wav");
            return; // Quay lại MainMenu
        }
    }
}

// Function to start the game
void StartGame() {
    PlayAudio(NULL); // <--- Tắt mọi âm thanh (BGM) để vào game yên tĩnh
    ClearScreen();
    ResetData();
    DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
    DrawInfoBoard();
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

void MainMenu();
// Function to handle Game Win scenario
void GameWin() {
    STATE = 0;
    PlayAudio(NULL);
    PlayAudio("world_clear.wav");

    ClearScreen();

    // Tính toán tâm màn hình (bao gồm cả phần game và stats)
    int totalWidth = WIDTH_CONSOLE + STATS_WIDTH;
    int centerX = totalWidth / 2;
    int startY = 2; // Bắt đầu vẽ từ dòng 2

    // --- VẼ KHUNG VIỀN (BORDER) ---
    // Khung rộng 40, cao 16 nằm giữa màn hình
    int boxWidth = 40;
    int boxHeight = 17;
    int boxX = centerX - (boxWidth / 2);
    int boxY = 1;

    SetColor(1); // Xanh dương đậm (Dark Blue)
    // Vẽ cạnh trên
    GotoXY(boxX, boxY); cout << (char)218;
    for (int i = 1; i < boxWidth; i++) cout << (char)196;
    cout << (char)191;

    // Vẽ 2 bên
    for (int i = 1; i < boxHeight; i++) {
        GotoXY(boxX, boxY + i); cout << (char)179;
        GotoXY(boxX + boxWidth, boxY + i); cout << (char)179;
    }

    // Vẽ cạnh dưới
    GotoXY(boxX, boxY + boxHeight); cout << (char)192;
    for (int i = 1; i < boxWidth; i++) cout << (char)196;
    cout << (char)217;

    // --- VẼ CÚP ---
    // Căn giữa cúp trong khung
    DrawTrophy(centerX - 7, boxY + 1);

    // --- HIỂN THỊ THÔNG TIN (MÀU ĐẬM) ---
    int textY = boxY + 11;

    // Tiêu đề VICTORY
    SetColor(4); // Đỏ đậm (Dark Red)
    GotoXY(centerX - 7, textY++); cout << "=== VICTORY ===";

    textY++; // Cách 1 dòng

    // Thông số
    SetColor(0); // Đen
    GotoXY(centerX - 12, textY);   cout << "Final Length : ";
    SetColor(2); // Xanh lá đậm
    cout << SIZE_SNAKE;

    SetColor(0); // Đen
    GotoXY(centerX + 2, textY++);  cout << "Speed : ";
    SetColor(1); // Xanh dương đậm
    cout << SPEED;

    textY++;

    // Credit
    SetColor(8); // Xám đậm
    GotoXY(centerX - 14, textY++); cout << "Dev Team: " << TEAM_IDS;

    // Hướng dẫn thoát
    SetColor(5); // Tím đậm (Dark Magenta)
    GotoXY(centerX - 12, boxY + boxHeight - 1);
    cout << "Press ANY KEY to Return";

    _getch();
    PlayAudio(NULL);
    MainMenu();
}

// Function to proceed to the next level
// Requirement 4.3: Keep length unchanged
void NextLevel() {
    LEVEL++;
    SPEED++;
    GATE_ACTIVE = false;

    if (LEVEL > MAX_SPEED) {
        STATE = 2;
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
        snake[i].y = HEIGH_CONSOLE / 2;
    }

    GenerateFood();
    DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
    DrawInfoBoard();
    STATE = 1;
}

// --- MOVEMENT LOGIC ---

// Function to check if snake enters the gate
bool CheckGateEntry(int nextX, int nextY) {
    if (!GATE_ACTIVE) return false;
    for (int i = 0; i < 3; i++) {
        // Chỉ cần chạm vào bất kỳ điểm nào của cổng là qua màn
        if (nextX == gate[i].x && nextY == gate[i].y) return true;
    }
    return false;
}
// Function to check collision with walls (including gate walls)
bool CheckWallCollision(int nextX, int nextY) {
    // 1. ƯU TIÊN CAO NHẤT: Nếu chạm vào Cổng khi cổng đang mở -> AN TOÀN
    if (GATE_ACTIVE) {
        for (int i = 0; i < 3; i++) {
            // Nếu tọa độ trùng với bất kỳ điểm nào của cổng -> Không chết
            if (nextX == gate[i].x && nextY == gate[i].y) return false;
        }
    }

    // 2. Kiểm tra va chạm với tường bao quanh sân
    // Lưu ý: Tường bên trái là 0, bên phải là WIDTH_CONSOLE
    // Bên trên là 0, bên dưới là HEIGH_CONSOLE
    if (nextX >= WIDTH_CONSOLE || nextX <= 0 || nextY >= HEIGH_CONSOLE || nextY <= 0)
        return true;

    // 3. Kiểm tra va chạm với tường bảo vệ cổng (4 cục gạch xung quanh cổng)
    if (GATE_ACTIVE) {
        for (int i = 0; i < 4; i++) {
            if (nextX == wall[i].x && nextY == wall[i].y) return true;
        }
    }

    return false;
}

void HandleMove(int nextX, int nextY, char dir) {
    if (STATE != 1) return; // ⛔ chỉ chơi khi Playing

    if (CheckGateEntry(nextX, nextY)) {
        DrawSnakeGoThroughGate();
        NextLevel();
        return;
    }


    if (CheckWallCollision(nextX, nextY) || IsCollisionWithBody(nextX, nextY)) {
        if (!GOD_MODE) ProcessDead();
        return;
    }

    if (nextX == food[FOOD_INDEX].x && nextY == food[FOOD_INDEX].y) {
        Eat();
        return;
    }

    UpdateSnakeBody();
    snake[SIZE_SNAKE - 1].x = nextX;
    snake[SIZE_SNAKE - 1].y = nextY;
}

// Function to move Snake to the Right
void MoveRight() {
    int nextX = snake[SIZE_SNAKE - 1].x + 1;
    int nextY = snake[SIZE_SNAKE - 1].y;
    HandleMove(nextX, nextY, 'D');
}


// Function to move Snake to the Left
void MoveLeft() {
    int nextX = snake[SIZE_SNAKE - 1].x - 1;
    int nextY = snake[SIZE_SNAKE - 1].y;
    HandleMove(nextX, nextY, 'A');
}


// Function to move Snake Down
void MoveDown() {
    int nextX = snake[SIZE_SNAKE - 1].x;
    int nextY = snake[SIZE_SNAKE - 1].y + 1;
    HandleMove(nextX, nextY, 'A');
}

// Function to move Snake Up
void MoveUp() {
    int nextX = snake[SIZE_SNAKE - 1].x;
    int nextY = snake[SIZE_SNAKE - 1].y - 1;
    HandleMove(nextX, nextY, 'A');
}

// Function for the separate thread handling snake movement
// Function for the separate thread handling snake movement
void ThreadFunc() {
    while (IS_RUNNING) {
        // --- THÊM ĐOẠN CODE NÀY ---
        if (STATE == 2) { 
            // Nếu phát hiện trạng thái đã Win/Kết thúc thì thoát luồng ngay
            return; 
        }
        // --------------------------

        if (STATE != 1) {
            Sleep(50);   
            continue;
        }

        DrawSnakeAndFood(" ");
        switch (MOVING) {
        case 'A': MoveLeft(); break;
        case 'D': MoveRight(); break;
        case 'W': MoveUp(); break;
        case 'S': MoveDown(); break;
        }
        DrawSnakeAndFood("O");

        Sleep(max(50, 200 - (SPEED * 20)));
    }
}

// Trong ExitGame
// Hàm này không cần tham số HANDLE nữa
void ExitGame() {
    STATE = 0;
    IS_RUNNING = false; // <--- Ra lệnh cho ThreadFunc dừng lại
    // Luồng sẽ tự kết thúc sau tối đa 50ms hoặc 300/SPEED ms
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

// Hàm PlayGame đã tối ưu độ mượt
void PlayGame() {
    IS_RUNNING = true;
    thread t1(ThreadFunc);
    HANDLE handle_t1 = t1.native_handle();
    int temp;

    while (1) {
        // 1. Kiểm tra thắng game
        if (STATE == 2) {
            ExitGame();
            if (t1.joinable()) t1.join();
            GameWin();
            return;
        }

        // 2. Kiểm tra phím
        if (_kbhit()) {
            temp = toupper(_getch());

            if (STATE == 1) {
                if (temp == 'P') {
                    PlayAudio(NULL);
                    PauseGame(handle_t1);
                }
                else if (temp == 27) { // ESC
                    ExitGame();
                    if (t1.joinable()) t1.join();
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
                        DrawInfoBoard();
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
                else if (temp == 'G') {
                    GOD_MODE = !GOD_MODE;
                    DrawInfoBoard();
                    GotoXY(WIDTH_CONSOLE + 2, 2);
                    SetColor(GOD_MODE ? 14 : 7);
                    SetColor(COLOR_NORMAL);
                }
                else if (temp == '+') {
                    if (SIZE_SNAKE < MAX_SIZE_SNAKE) {
                        snake[SIZE_SNAKE] = snake[SIZE_SNAKE - 1];
                        SIZE_SNAKE++;
                        DrawSnakeAndFood("O");
                        PlayAudio("eat.wav");
                    }
                }
                else if (temp == 'N') {
                    NextLevel();
                }
                else {
                    ResumeThread(handle_t1);
                    if ((temp == 'D' || temp == 'A' || temp == 'W' || temp == 'S') && temp != CHAR_LOCK) {
                        int oldMoving = MOVING;
                        if (temp == 'D') CHAR_LOCK = 'A';
                        else if (temp == 'W') CHAR_LOCK = 'S';
                        else if (temp == 'S') CHAR_LOCK = 'W';
                        else if (temp == 'A') CHAR_LOCK = 'D';

                        MOVING = temp;
                        if (MOVING != oldMoving) PlayAudio("move.wav");
                    }
                }
            }
            else if (STATE == 0) {
                if (temp == 'Y') {
                    ExitGame();
                    if (t1.joinable()) t1.join();
                    StartGame();
                    PlayGame();
                    return;
                }
                else {
                    ExitGame();
                    if (t1.joinable()) t1.join();
                    return;
                }
            }
        }
        else {
            // --- QUAN TRỌNG: THÊM DÒNG NÀY ĐỂ FIX LAG ---
            // Nếu không có phím nào được bấm, cho CPU nghỉ 50ms
            // Điều này trả lại tài nguyên cho luồng vẽ rắn hoạt động mượt mà
            Sleep(50);
        }
    }
}


// Function to display the Main Menu
void MainMenu() {
    // 1. Bật nhạc nền ngay khi vào chương trình
    PlayAudio("bgm.wav", true);

    char choice;
    while (true) {
        ClearScreen();
        SetColor(COLOR_NORMAL);
        cout << "==========================\n";
        cout << "        SNAKE GAME\n";
        cout << "==========================\n";
        cout << "1. New Game\n";
        cout << "2. Load Game\n";
        cout << "3. Settings\n";
        cout << "4. Exit\n";

        choice = _getch();

        switch (choice) {
        case '1':
            // Khi bấm chọn, tiếng "choose" vang lên sẽ TỰ ĐỘNG ngắt nhạc nền -> Đúng ý đồ
            PlayAudio("choose.wav");
            StartGame();
            PlayGame();

            // SAU KHI CHƠI XONG (Game Over hoặc Win) và quay lại đây:
            PlayAudio("bgm.wav", true); // <--- Bật lại nhạc nền Menu
            break;

        case '2':
        {
            PlayAudio("choose.wav");
            string filename;
            cout << "\nFilename: ";
            cin >> filename;
            if (LoadGame(filename)) {
                ClearScreen();
                DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
                DrawInfoBoard();
                DrawSnakeAndFood("O");
                STATE = 1;
                PlayGame();

                // SAU KHI CHƠI XONG (Game Over hoặc Win) và quay lại đây:
                PlayAudio("bgm.wav", true); // <--- Bật lại nhạc nền Menu
            }
            else {
                cout << "\nLoad failed! Press any key.";
                _getch();
            }
            break;
        }
        case '3':
            PlayAudio("choose.wav");
            SettingsMenu();
            // Sau khi thoát Settings, cần bật lại nhạc (vì trong settings có thể đã tắt)
            if (SOUND_ON) PlayAudio("bgm.wav", true);
            break;

        case '4':
            PlayAudio("choose.wav");
            // Sleep một chút để kịp nghe tiếng bye bye trước khi tắt
            Sleep(500);
            std::exit(0);
        }
    }
}
// Main function - Entry point of the program
int main() {
    ShowCur(0);
    // --- THÊM ĐOẠN NÀY ĐỂ MỞ RỘNG CỬA SỔ ---
    system("mode con: cols=120 lines=30");
    // cols=120: Đủ rộng cho (70 game + 35 stats + lề)
    // ----------------------------------------

    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h, BG_COLOR);
    FixConsoleWindow();
    MainMenu();
    return 0;
}
