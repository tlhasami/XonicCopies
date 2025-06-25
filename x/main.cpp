#include <SFML/Graphics.hpp>
#include <SFML/System.hpp> // Required for sf::sleep

#include <time.h>
using namespace sf;

const int M = 25;
const int N = 40;

int grid[M][N] = {0};
int ts = 18; // tile size

struct Enemy
{
    int x, y, dx, dy;

    Enemy()
    {
        x = y = 300;
        dx = 4 - rand() % 8;
        dy = 4 - rand() % 8;
    }

    void move()
    {
        x += dx;
        if (grid[y / ts][x / ts] == 1)
        {
            dx = -dx;
            x += dx;
        }
        y += dy;
        if (grid[y / ts][x / ts] == 1)
        {
            dy = -dy;
            y += dy;
        }
    }
};

void drop(int y, int x)
{
    if (grid[y][x] == 0)
        grid[y][x] = -1;
    if (grid[y - 1][x] == 0)
        drop(y - 1, x);
    if (grid[y + 1][x] == 0)
        drop(y + 1, x);
    if (grid[y][x - 1] == 0)
        drop(y, x - 1);
    if (grid[y][x + 1] == 0)
        drop(y, x + 1);
}

int main()
{
    srand(time(0));

    RenderWindow window(VideoMode(N * ts, M * ts), "Xonix Game!");
    window.setFramerateLimit(60);

    Texture t1, t2, t3;
    t1.loadFromFile("images/tiles.png");
    t2.loadFromFile("images/gameover.png");
    t3.loadFromFile("images/enemy.png");

    Sprite sTile(t1), sGameover(t2), sEnemy(t3);
    sGameover.setPosition(100, 100);
    sEnemy.setOrigin(20, 20);

    int enemyCount = 4;
    Enemy a[10];

    bool Game = true;
    int x = 0, y = 0, dx = 0, dy = 0;
    float timer = 0, delay = 0.07;
    Clock clock;

    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            if (i == 0 || j == 0 || i == M - 1 || j == N - 1)
                grid[i][j] = 1;

    bool menuMode = true;

    Font font;
    if (!font.loadFromFile("ARIAL.TTF"))
    {
        return -1; // handle error
    }

    Text title("Xonix Game", font, 40);
    title.setPosition(250, 80);
    title.setFillColor(Color::Yellow);

    Text option1("1. Start Game", font, 30);
    option1.setPosition(250, 160);
    option1.setFillColor(Color::Green);

    Text option2("2. Scoreboard", font, 30);
    option2.setPosition(250, 210);
    option2.setFillColor(Color::Cyan);

    Text option3("3. Exit", font, 30);
    option3.setPosition(250, 260);
    option3.setFillColor(Color::Red);

    bool levelSelection = false;

    Text option5("4. EASY", font, 30);
    Text option6("5. MEDIUM", font, 30);
    Text option7("6. HARD", font, 30);
    Text option8("7. CONTINOUS", font, 30);

    option5.setPosition(250, 160);
    option5.setFillColor(Color::Green);

    option6.setPosition(250, 210);
    option6.setFillColor(Color::Cyan);

    option7.setPosition(250, 260);
    option7.setFillColor(Color::Red);

    option8.setPosition(250, 310);
    option8.setFillColor(Color::Blue);

    bool end = false;
    Text option9(" 8. Menu", font, 30);
    Text option10(" 9. Restart", font, 30);
    Text option11(" E. Exit", font, 30);

    option9.setPosition(250, 160);
    option9.setFillColor(Color::Green);

    option10.setPosition(250, 210);
    option10.setFillColor(Color::Cyan);

    option11.setPosition(250, 260);
    option11.setFillColor(Color::Red);

    bool continuousMode = false;
    float enemyTimer = 0;

    while (window.isOpen())
    {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        timer += time;

        Event e;

        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();

            if (menuMode && e.type == Event::KeyPressed)
            {
                if (e.key.code == Keyboard::Num1)
                { // Start Game
                    menuMode = false;
                    levelSelection = true;
                }
                else if (e.key.code == Keyboard::Num2)
                {
                    // show scoreboard (not implemented yet)
                    // For now, just stay on menu
                }
                else if (e.key.code == Keyboard::Num3)
                {
                    window.close(); // Exit
                }
            }

            else if (levelSelection && e.type == Event::KeyPressed)
            {
                if (e.key.code == Keyboard::Num4)
                { // Start Game
                    enemyCount = 2;
                    levelSelection = false;
                    Game = true;
                }
                else if (e.key.code == Keyboard::Num5)
                {
                    enemyCount = 4;
                    levelSelection = false;
                    Game = true;
                }
                else if (e.key.code == Keyboard::Num6)
                {
                    enemyCount = 6;
                    levelSelection = false;
                    Game = true;
                }
                else if (e.key.code == Keyboard::Num7) // Continuous Mode
                {
                    enemyCount = 2;
                    levelSelection = false;
                    Game = true;
                    continuousMode = true;
                    enemyTimer = 0;
                }
            }

            else if (end && e.type == Event::KeyPressed)
            {
                if (e.key.code == Keyboard::Num8)
                { // Start Game
                    menuMode = true;
                    end = false;
                }
                else if (e.key.code == Keyboard::Num9)
                {

                    for (int i = 1; i < M - 1; i++)
                        for (int j = 1; j < N - 1; j++)
                            grid[i][j] = 0;

                    x = 0;
                    y = 0;
                    Game = false;
                    levelSelection = true;
                    end = false;
                }
                else if (e.key.code == Keyboard::E)
                {
                    window.close();
                }
            }

            else if (!menuMode && e.type == Event::KeyPressed)
            {
                if (e.key.code == Keyboard::Escape)
                {
                    for (int i = 1; i < M - 1; i++)
                        for (int j = 1; j < N - 1; j++)
                            grid[i][j] = 0;

                    x = 0;
                    y = 0;
                    Game = false;

                    menuMode = true;
                    continuousMode = false;
                    enemyTimer = 0;
                }
            }
        }

        if (menuMode)
        {
            window.clear();
            window.draw(title);
            window.draw(option1);
            window.draw(option2);
            window.draw(option3);
            window.display();
            continue; // skip rest of game loop if in menu
        }
        else if (levelSelection)
        {
            window.clear();
            window.draw(title);
            window.draw(option5);
            window.draw(option6);
            window.draw(option7);
            window.draw(option8);
            window.display();
            continue; // skip rest of game loop if in menu
        }
        else if (end)
        {
            window.clear();
            window.draw(title);
            window.draw(option9);
            window.draw(option10);
            window.draw(option11);
            window.display();
            continue; // skip rest of game loop if in menu
        }

        // ... the rest of. your game logic and drawing below

        if (Keyboard::isKeyPressed(Keyboard::Left))
        {
            dx = -1;
            dy = 0;
        };
        if (Keyboard::isKeyPressed(Keyboard::Right))
        {
            dx = 1;
            dy = 0;
        };
        if (Keyboard::isKeyPressed(Keyboard::Up))
        {
            dx = 0;
            dy = -1;
        };
        if (Keyboard::isKeyPressed(Keyboard::Down))
        {
            dx = 0;
            dy = 1;
        };

        if (!Game)
        {
            window.draw(sGameover);
            window.display();
            sleep(seconds(2)); // pauses for 2 seconds
            continuousMode = false;
            enemyTimer = 0;

            end = true;
        }

        if (continuousMode && Game)
        {
            enemyTimer += time;
            if (enemyTimer >= 20.0f) // increase decrese timer here
            {
                if (enemyCount + 2 <= 10) // Max 10 enemies
                {
                    enemyCount += 2;

                    // Set manual positions for newly activated enemies
                    a[enemyCount - 2].x = 100;
                    a[enemyCount - 2].y = 100;

                    a[enemyCount - 1].x = 200;
                    a[enemyCount - 1].y = 200;
                }
                enemyTimer = 0;
            }
        }

        if (timer > delay)
        {
            x += dx;
            y += dy;

            if (x < 0)
                x = 0;
            if (x > N - 1)
                x = N - 1;
            if (y < 0)
                y = 0;
            if (y > M - 1)
                y = M - 1;

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
            if (grid[a[i].y / ts][a[i].x / ts] == 2)
                Game = false;

        /////////draw//////////
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

        window.display();
    }
    return 0;
}