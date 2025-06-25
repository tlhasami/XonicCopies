#include <SFML/Graphics.hpp>
#include <time.h>
#include <iostream>
using namespace sf;
using namespace std;

const int M = 25;
const int N = 40;

int grid[M][N] = {0};
int ts = 18; // tile size

struct Enemy
{
    int x, y, dx, dy;
    float speed;

    Enemy()
    {
        x = y = 300;
        dx = 4 - rand() % 8;
        dy = 4 - rand() % 8;
        speed = 1.0f;
    }

    void move()
    {
        x += dx * speed;
        int gx = x / ts;
        int gy = y / ts;
        if (gx < 0 || gx >= N || gy < 0 || gy >= M || grid[gy][gx] == 1)
        {
            dx = -dx;
            x += dx * speed;
        }

        y += dy * speed;
        gx = x / ts;
        gy = y / ts;
        if (gx < 0 || gx >= N || gy < 0 || gy >= M || grid[gy][gx] == 1)
        {
            dy = -dy;
            y += dy * speed;
        }
    }
};

void drop(int y, int x)
{
    if (y < 0 || y >= M || x < 0 || x >= N || grid[y][x] != 0)
        return;
    grid[y][x] = -1;
    drop(y - 1, x);
    drop(y + 1, x);
    drop(y, x - 1);
    drop(y, x + 1);
}

int StartMenu()
{
    int Choice = 0;
    cout << "Welcome to single Player mode: " << endl;
    cout << "Choose difficulty: " << endl;
    cout << "Enter 1 for Easy Mode: " << endl;
    cout << "Enter 2 for Medium Mode: " << endl;
    cout << "Enter 3 for Hard Mode: " << endl;
    cout << "Enter 4 for Continuous Mode: " << endl;
    cin >> Choice;
    switch (Choice)
    {
    case 1:
        cout << "Difficulty level Easy chosen: " << endl;
        return 1;
    case 2:
        cout << "Difficulty level Medium chosen: " << endl;
        return 2;
    case 3:
        cout << "Difficulty level Hard chosen: " << endl;
        return 3;
    case 4:
        cout << "Difficulty level Continuous chosen: " << endl;
        return 4;
    default:
        return 0;
    }
}

void PlayerChoice()
{
    int choice = 0;
    cout << "Enter 1 for single player: " << endl;
    cout << "Enter 2 for two players: " << endl;
    cin >> choice;
    switch (choice)
    {
    case 1:
        cout << "welcome to single player mode: " << endl;
        break;
    case 2:
        cout << "welcome to multiplayer mode: " << endl;
        break;
    default:
        break;
    }
}

int main()
{
    int Moves = 0;
    bool isBuilding = false;
    int c = StartMenu();
    PlayerChoice();
    srand(time(0));

    float enemyAddTimer = 0;
    float TotalTime = 0;

    RenderWindow window(VideoMode(N * ts, M * ts), "Xonix Game!");
    window.setFramerateLimit(60);

    Texture t1, t2, t3;
    t1.loadFromFile("images/tiles.png");
    t2.loadFromFile("images/gameover.png");
    t3.loadFromFile("images/enemy.png");

    Sprite sTile(t1), sGameover(t2), sEnemy(t3);
    sGameover.setPosition(100, 100);
    sEnemy.setOrigin(20, 20);

    bool Game = true;
    float time;
    Enemy a[10];

    int enemyCount;
    if (c == 1)
        enemyCount = 2;
    else if (c == 2)
        enemyCount = 4;
    else if (c == 3)
        enemyCount = 6;
    else if (c == 4)
        enemyCount = 2;

    int x = 0, y = 0, dx = 0, dy = 0;
    float timer = 0, delay = 0.07;
    Clock clock;

    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            if (i == 0 || j == 0 || i == M - 1 || j == N - 1)
                grid[i][j] = 1;

    while (window.isOpen())
    {
        
        time = clock.getElapsedTime().asSeconds();
        clock.restart();
        timer += time;
        TotalTime += time;
        if (c == 1)
        { ////////////////////////// if timechecked equals 20 seconds, we increase the speed
            enemyAddTimer += time;
            if (enemyAddTimer >= 20.0f && enemyCount == 2)
            {
                for (int i = 0; i < enemyCount; i++)
                    a[i].speed += 0.3f;
                enemyAddTimer = 0;
            }
            if (enemyAddTimer >= 30.0f && enemyCount == 4)
            {
                //////////
            }
        }

        if (c == 2)
        { ////////////////////////// if timechecked equals 20 seconds, we increase the speed
            enemyAddTimer += time;
            if (enemyAddTimer >= 20.0f && enemyCount == 4)
            {
                for (int i = 0; i < enemyCount; i++)
                    a[i].speed += 0.3f;
                enemyAddTimer = 0;
            }
            if (enemyAddTimer >= 30.0f && enemyCount == 4)
            {
                //////////
            }
        }

        if (c == 3)
        { ////////////////////////// if timechecked equals 20 seconds, we increase the speed
            enemyAddTimer += time;
            if (enemyAddTimer >= 20.0f && enemyCount == 6)
            {
                for (int i = 0; i < enemyCount; i++)
                    a[i].speed += 0.3f;
                enemyAddTimer = 0;
            }
            if (enemyAddTimer >= 30.0f && enemyCount == 4)
            {
                //////////
            }
        }

        if (c == 4)
        { ////////////////////////// if timechecked equals 20 seconds, we increase the speed
            enemyAddTimer += time;
            if (enemyAddTimer >= 4.0f && enemyCount + 2 <= 10)
            {
                enemyCount += 2;
                for (int i = 0; i < enemyCount; i++)
                    a[i].speed += 0.3f;
                enemyAddTimer = 0;
                cout << "Enemies increased to: " << enemyCount << endl;
            }
            if (enemyAddTimer >= 20.0f && enemyCount == 4)
            {
                //////////
            }
        }

        Event e;
        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();

            if (e.type == Event::KeyPressed)
                if (e.key.code == Keyboard::Escape)
                {
                    for (int i = 1; i < M - 1; i++)
                        for (int j = 1; j < N - 1; j++)
                            grid[i][j] = 0;
                    x = 10;
                    y = 0;
                    Game = true;
                }
        }

        if (Keyboard::isKeyPressed(Keyboard::Left))
        {
            dx = -1;
            dy = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Right))
        {
            dx = 1;
            dy = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Up))
        {
            dx = 0;
            dy = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::Down))
        {
            dx = 0;
            dy = 1;
        }

        if (!Game)
            continue;

        if (timer > delay)
        {
            x += dx;
            y += dy;

            if (x < 0)
                x = 0;
            if (x >= N)
                x = N - 1;
            if (y < 0)
                y = 0;
            if (y >= M)
                y = M - 1;

            if (grid[y][x] == 0 && !isBuilding)
            {
                Moves++;
                isBuilding = true;
            }
            if (grid[y][x] != 0 && isBuilding)
            {
                isBuilding = false;
            }

            if (grid[y][x] == 2)
                Game = false;
            if (grid[y][x] == 0)
                grid[y][x] = 2;
                
            timer = 0;
        }

        for (int i = 0; i < enemyCount; i++)
            a[i].move();

        if (grid[y][x] == 1)
        {
            dx = dy = 0;
            for (int i = 0; i < enemyCount; i++)
                drop(a[i].y / ts, a[i].x / ts);

            for (int i = 0; i < M; i++)
                for (int j = 0; j < N; j++)
                    if (grid[i][j] == -1)
                        grid[i][j] = 0;
                    else
                        grid[i][j] = 1;
        }

        for (int i = 0; i < enemyCount; i++)
        {
            int gx = a[i].x / ts;
            int gy = a[i].y / ts;
            if (gx >= 0 && gx < N && gy >= 0 && gy < M && grid[gy][gx] == 2)
                Game = false;
        }

        window.clear();

        for (int i = 0; i < M; i++)
            for (int j = 0; j < N; j++)
            {
                if (grid[i][j] == 0)
                    continue;
                if (grid[i][j] == 1)
                    sTile.setTextureRect(IntRect(0, 0, ts, ts));
                if (grid[i][j] == 2)
                    sTile.setTextureRect(IntRect(54, 0, ts, ts));
                sTile.setPosition(j * ts, i * ts);
                window.draw(sTile);
            }

        sTile.setTextureRect(IntRect(36, 0, ts, ts));
        sTile.setPosition(x * ts, y * ts);
        window.draw(sTile);

        sEnemy.rotate(10);
        for (int i = 0; i < enemyCount; i++)
        {
            sEnemy.setPosition(a[i].x, a[i].y);
            window.draw(sEnemy);
        }

        if (!Game)
            window.draw(sGameover);
        window.display();
    }

    if (!Game)
    {
        cout << "The total moves are: " << Moves << endl;
        cout << "The total time the game played is :" << TotalTime << endl;
    }

    return 0;
}