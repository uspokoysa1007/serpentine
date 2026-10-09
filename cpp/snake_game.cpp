#include <windows.h>
#include <conio.h>
#include <deque>
#include <cstdlib>
#include <ctime>
#include <cstdio>

static const int W = 30, H = 20;
static HANDLE g_out;

static void gotoxy(int x, int y){
    COORD c{ (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(g_out, c);
}

static void hide_cursor(){
    CONSOLE_CURSOR_INFO ci{ 1, FALSE };
    SetConsoleCursorInfo(g_out, &ci);
}

int run_snake(){
    g_out = GetStdHandle(STD_OUTPUT_HANDLE);
    hide_cursor();
    system("title Snake");

    std::srand((unsigned)time(nullptr));
    std::deque<COORD> body = { { (SHORT)(W/2), (SHORT)(H/2) } };
    COORD food{ (SHORT)(rand() % W), (SHORT)(rand() % H) };
    int dx = 1, dy = 0, score = 0;

    while(true){
        for(int i = 0; i < W + 2; i++){
            gotoxy(i, 0);     putchar('#');
            gotoxy(i, H + 1); putchar('#');
        }
        for(int i = 0; i < H + 2; i++){
            gotoxy(0, i);     putchar('#');
            gotoxy(W + 1, i); putchar('#');
        }

        gotoxy(food.X + 1, food.Y + 1);
        putchar('*');

        for(size_t i = 0; i < body.size(); ++i){
            gotoxy(body[i].X + 1, body[i].Y + 1);
            putchar(i == 0 ? 'O' : 'o');
        }

        if(_kbhit()){
            int k = _getch();
            if(k == 224 || k == 0){
                k = _getch();
                if(k == 72 && dy == 0){ dx = 0; dy = -1; }
                if(k == 80 && dy == 0){ dx = 0; dy =  1; }
                if(k == 75 && dx == 0){ dx = -1; dy = 0; }
                if(k == 77 && dx == 0){ dx =  1; dy = 0; }
            } else if(k == 'q' || k == 27) return 0;
        }

        COORD head{ (SHORT)(body.front().X + dx),
                    (SHORT)(body.front().Y + dy) };
        if(head.X < 0 || head.X >= W || head.Y < 0 || head.Y >= H) break;
        for(auto& s : body)
            if(s.X == head.X && s.Y == head.Y) return 0;

        body.push_front(head);
        if(head.X == food.X && head.Y == food.Y){
            score++;
            food = { (SHORT)(rand() % W), (SHORT)(rand() % H) };
        } else {
            auto tail = body.back();
            body.pop_back();
            gotoxy(tail.X + 1, tail.Y + 1);
            putchar(' ');
        }

        gotoxy(0, H + 3);
        printf("score: %d   (arrows to move, q to quit)   ", score);
        Sleep(80);
    }

    gotoxy(0, H + 3);
    printf("GAME OVER. score: %d. press any key...", score);
    _getch();
    return 0;
}