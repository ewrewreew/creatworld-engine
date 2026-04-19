// Snake.cpp - 贪吃蛇游戏（最终修正版）
#include "Creatworld.hpp"

using namespace creatworld;

// ==================== 蛇身节 ====================
class SnakePart : public game_object {
public:
    SnakePart(int x, int y, bool isHead = false) {
        width = 20;
        height = 20;
        position = vec2((float)x, (float)y);
        
        if (isHead) {
            object_color = color(0, 255, 0);
        } else {
            object_color = color(0, 180, 0);
        }
        tag = isHead ? "snake_head" : "snake_body";
    }
};

// ==================== 食物 ====================
class Food : public game_object {
public:
    Food(int x, int y) {
        width = 16;
        height = 16;
        position = vec2((float)x + 2, (float)y + 2);
        object_color = color(255, 50, 50);
        tag = "food";
    }
    
    void draw(HDC hdc) override {
        HBRUSH brush = CreateSolidBrush(object_color.to_rgb());
        SelectObject(hdc, brush);
        Ellipse(hdc, (int)position.x, (int)position.y,
                (int)position.x + width, (int)position.y + height);
        DeleteObject(brush);
    }
};

// ==================== 背景网格 ====================
class GameBackground : public game_object {
private:
    int startX, startY, gridW, gridH, cellSize;
public:
    GameBackground(int x, int y, int w, int h, int size) 
        : startX(x), startY(y), gridW(w), gridH(h), cellSize(size) {}
    
    void draw(HDC hdc) override {
        // 背景色
        RECT bg = {startX - 3, startY - 3, startX + gridW * cellSize + 3, startY + gridH * cellSize + 3};
        HBRUSH gray = CreateSolidBrush(RGB(20, 20, 30));
        FillRect(hdc, &bg, gray);
        DeleteObject(gray);
        
        // 边框
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(100, 100, 150));
        SelectObject(hdc, pen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, startX - 3, startY - 3, startX + gridW * cellSize + 3, startY + gridH * cellSize + 3);
        DeleteObject(pen);
        
        // 网格线
        pen = CreatePen(PS_SOLID, 1, RGB(50, 50, 60));
        SelectObject(hdc, pen);
        for (int i = 0; i <= gridW; i++) {
            MoveToEx(hdc, startX + i * cellSize, startY, NULL);
            LineTo(hdc, startX + i * cellSize, startY + gridH * cellSize);
        }
        for (int i = 0; i <= gridH; i++) {
            MoveToEx(hdc, startX, startY + i * cellSize, NULL);
            LineTo(hdc, startX + gridW * cellSize, startY + i * cellSize);
        }
        DeleteObject(pen);
    }
};

// ==================== 游戏场景 ====================
class GameScene : public scene {
private:
    std::vector<SnakePart*> snake;
    int direction = 0;
    int nextDir = 0;
    float moveTimer = 0;
    float moveDelay = 0.15f;
    int score = 0;
    bool gameOver = false;
    
    const int startX = 40;
    const int startY = 60;
    const int cellSize = 20;
    const int gridW = 36;
    const int gridH = 26;
    
public:
    GameScene() : scene("GameScene") {}
    
    int toGridX(float px) { return ((int)px - startX) / cellSize; }
    int toGridY(float py) { return ((int)py - startY) / cellSize; }
    int toScreenX(int gx) { return startX + gx * cellSize; }
    int toScreenY(int gy) { return startY + gy * cellSize; }
    
    void on_enter() override {
        // 添加背景（最先添加，在最底层）
        add_object(new GameBackground(startX, startY, gridW, gridH, cellSize));
        
        initSnake();
        spawnFood();
    }
    
    void initSnake() {
        for (auto* p : snake) remove_object(p);
        snake.clear();
        
        // 蛇头在中心
        int centerGx = gridW / 2;
        int centerGy = gridH / 2;
        
        SnakePart* head = new SnakePart(toScreenX(centerGx), toScreenY(centerGy), true);
        add_object(head);
        snake.push_back(head);
        
        // 身体向左延伸
        for (int i = 1; i <= 2; i++) {
            SnakePart* body = new SnakePart(toScreenX(centerGx - i), toScreenY(centerGy), false);
            add_object(body);
            snake.push_back(body);
        }
        
        direction = 0;
        nextDir = 0;
        moveTimer = 0;
        gameOver = false;
        score = 0;
    }
    
    void spawnFood() {
        // 收集被蛇占用的格子
        std::vector<std::pair<int, int>> occupied;
        for (auto* seg : snake) {
            occupied.push_back({toGridX(seg->position.x), toGridY(seg->position.y)});
        }
        
        // 找空闲格子
        std::vector<std::pair<int, int>> freeCells;
        for (int gy = 0; gy < gridH; gy++) {
            for (int gx = 0; gx < gridW; gx++) {
                bool taken = false;
                for (auto& occ : occupied) {
                    if (occ.first == gx && occ.second == gy) {
                        taken = true;
                        break;
                    }
                }
                if (!taken) freeCells.push_back({gx, gy});
            }
        }
        
        if (freeCells.empty()) {
            gameOver = true;
            return;
        }
        
        int idx = rand() % freeCells.size();
        int fx = freeCells[idx].first;
        int fy = freeCells[idx].second;
        
        add_object(new Food(toScreenX(fx), toScreenY(fy)));
    }
    
    void moveSnake() {
        direction = nextDir;
        
        int headGx = toGridX(snake[0]->position.x);
        int headGy = toGridY(snake[0]->position.y);
        
        int newGx = headGx, newGy = headGy;
        switch (direction) {
            case 0: newGx++; break;
            case 1: newGy++; break;
            case 2: newGx--; break;
            case 3: newGy--; break;
        }
        
        // 检查是否吃到食物
        bool ate = false;
        auto foods = g_engine->get_objects_with_tag("food");
        for (auto* f : foods) {
            int fgx = toGridX(f->position.x - 2);
            int fgy = toGridY(f->position.y - 2);
            if (fgx == newGx && fgy == newGy) {
                ate = true;
                remove_object(f);
                score++;
                if (score % 5 == 0 && moveDelay > 0.08f) {
                    moveDelay -= 0.008f;
                }
                break;
            }
        }
        
        // 添加新头
        SnakePart* newHead = new SnakePart(toScreenX(newGx), toScreenY(newGy), true);
        add_object(newHead);
        snake.insert(snake.begin(), newHead);
        
        if (!ate) {
            remove_object(snake.back());
            snake.pop_back();
        } else {
            spawnFood();
        }
        
        // 更新颜色
        snake[0]->object_color = color(0, 255, 0);
        for (size_t i = 1; i < snake.size(); i++) {
            snake[i]->object_color = color(0, 180, 0);
        }
        
        // 碰撞检测
        if (newGx < 0 || newGx >= gridW || newGy < 0 || newGy >= gridH) {
            gameOver = true;
        }
        
        for (size_t i = 1; i < snake.size(); i++) {
            int bodyGx = toGridX(snake[i]->position.x);
            int bodyGy = toGridY(snake[i]->position.y);
            if (bodyGx == newGx && bodyGy == newGy) {
                gameOver = true;
                break;
            }
        }
        
        // 更新标题
        char title[256];
        sprintf(title, "贪吃蛇 - 分数: %d", score);
        SetWindowTextA(g_engine->get_window_handle(), title);
    }
    
    void on_update(float dt) override {
        if (gameOver) {
            static float timer = 0;
            timer += dt;
            if (timer > 2.0f) {
                timer = 0;
                // 重置
                auto foods = g_engine->get_objects_with_tag("food");
                for (auto* f : foods) remove_object(f);
                initSnake();
                spawnFood();
            }
            return;
        }
        
        if (g_engine->is_key_just_pressed(VK_RIGHT) && direction != 2) nextDir = 0;
        if (g_engine->is_key_just_pressed(VK_DOWN) && direction != 3) nextDir = 1;
        if (g_engine->is_key_just_pressed(VK_LEFT) && direction != 0) nextDir = 2;
        if (g_engine->is_key_just_pressed(VK_UP) && direction != 1) nextDir = 3;
        
        moveTimer += dt;
        if (moveTimer >= moveDelay) {
            moveTimer = 0;
            moveSnake();
        }
    }
    
    void on_draw(HDC hdc) override {
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 0));
        
        char buf[64];
        sprintf(buf, "SCORE: %d", score);
        TextOutA(hdc, startX, startY - 25, buf, strlen(buf));
        
        if (gameOver) {
            SetTextColor(hdc, RGB(255, 0, 0));
            TextOutA(hdc, startX + gridW * cellSize / 2 - 40, startY + gridH * cellSize / 2 - 10, "GAME OVER", 9);
            SetTextColor(hdc, RGB(150, 150, 150));
            TextOutA(hdc, startX + gridW * cellSize / 2 - 55, startY + gridH * cellSize / 2 + 15, "PRESS R TO RESTART", 18);
        }
        
        SetTextColor(hdc, RGB(100, 100, 100));
        TextOutA(hdc, startX, startY + gridH * cellSize + 5, "Arrow Keys: Move", 16);
    }
};

// ==================== 主游戏类 ====================
class SnakeGame : public engine {
public:
    void on_start() override {
        srand(GetTickCount());
        
        GameScene* scene = new GameScene();
        add_scene(scene);
        switch_scene("GameScene");
    }
};

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    gdiplus_init gdiplus;
    
    SnakeGame game;
    if (game.start(800, 600, "贪吃蛇 - Creatworld Engine")) {
        game.run();
        game.shutdown();
    }
    return 0;
}

