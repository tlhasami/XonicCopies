#include <SFML/Graphics.hpp>
#include <iostream>
#include <time.h>
#include <fstream>
using namespace sf;
using namespace std;

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

void reset()
{
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            if (i == 0 || j == 0 || i == M - 1 || j == N - 1)
                grid[i][j] = 1;
            else
                grid[i][j] = 0;
}
string screenMode = "menu";
int selectedItemIndex = 0;

int main()
{
    Font font;
    font.loadFromFile("fonts/arial.ttf"); // Ensure this font file exists

    Text menu[3];
    string items[3] = {"Start", "Scoreboard", "Exit"};

    sf::Text player1Text;
    sf::Text player2Text;

    for (int i = 0; i < 3; i++)
    {
        menu[i].setFont(font);
        menu[i].setString(items[i]);
        menu[i].setCharacterSize(30);
        menu[i].setPosition(300, 150 + i * 50);
    }

    // Set font for the text
    player1Text.setFont(font);
    player2Text.setFont(font);

    srand(time(0));

    RenderWindow window(VideoMode(N * ts, M * ts + 50), "Xonix Game!"); // Increase window height

    window.setFramerateLimit(60);

    Texture t1, t2, t3;
    t1.loadFromFile("images/tiles.png");
    t2.loadFromFile("images/gameover.png");
    t3.loadFromFile("images/enemy.png");

    Sprite sTile(t1), sGameover(t2), sEnemy(t3);
    sGameover.setPosition(100, 100 + 50); // Optional: adjust Game Over screen
    sEnemy.setOrigin(20, 20);

    int enemyCount = 0;
    Enemy a[100];
    int level_enemy = 0;

    int players = 0;

    bool Game = true;
    int x1 = 0, y1 = 0, dx1 = 0, dy1 = 0;
    int x2 = N - 1, y2 = M - 1, dx2 = 0, dy2 = 0;
    float timer = 0, delay = 0.07;

    int p1x = 0, p1y = 0;
    int p2x = 0, p2y = 0;

    int score_1 = 0;
    int score_2 = 0;

    int tiles_player_1 = 0;
    int tiles_player_2 = 0;

    int bonus_1 = 0;
    int bonusApplier_1 = 10;
    int scoreMultiplier_1 = 2;
    int power_1 = 0;
    int previous_powerup_score_1 = 0;

    int bonus_2 = 0;
    int bonusApplier_2 = 10;
    int scoreMultiplier_2 = 2;
    int power_2 = 0;
    int previous_powerup_score_2 = 0;

    // Set text properties
    player1Text.setCharacterSize(17);         // Set the text size
    player1Text.setFillColor(sf::Color::Red); // Set the text color

    player2Text.setCharacterSize(17);            // Set the text size
    player2Text.setFillColor(sf::Color::Yellow); // Set the text color

    // In your game loop, update the text based on `score_1` and `score_2`
    player1Text.setString("Player 1 Tiles: " + std::to_string(score_1));
    player2Text.setString("Player 2 Tiles: " + std::to_string(score_2));

    // Position the text on the screen
    player1Text.setPosition(115.f + 20.f, 10.f); // Position for Player 1
    player2Text.setPosition(350.f + 50.f, 10.f); // Position for Player 2

    sf::Text powerupText1;
    sf::Text powerupText2;

    powerupText1.setFont(font);
    powerupText1.setCharacterSize(15);
    powerupText1.setFillColor(sf::Color::White);
    powerupText1.setPosition(295, 15); // Adjust Y-position based on layout

    powerupText2.setFont(font);
    powerupText2.setCharacterSize(15);
    powerupText2.setFillColor(sf::Color::White);
    powerupText2.setPosition(570, 15); // Adjust for Player 2

    Clock clock;

    sf::Clock gameClock;
    sf::Text timeText;
    sf::Clock enemyClock; // Used to track time for enemy spawn

    timeText.setFont(font); // Make sure `font` is loaded already
    timeText.setCharacterSize(15);
    timeText.setFillColor(sf::Color::Green);
    timeText.setPosition(10, 10); // Top-left corner

    reset();

    int *scores = new int[100]; // assuming max 100 scores
    int count = 0;

    bool enemiesFrozen = false;
    Clock freezeClock;

    while (window.isOpen())
    {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        timer += time;

        Event e;

        if (screenMode == "menu")
        {

            while (window.pollEvent(e))
            {
                if (e.type == Event::Closed)
                    window.close();
                else if (e.type == Event::KeyPressed)
                {
                    if (e.key.code == Keyboard::Up)
                    {
                        selectedItemIndex = (selectedItemIndex + 2) % 3;
                    }
                    if (e.key.code == Keyboard::Down)
                    {
                        selectedItemIndex = (selectedItemIndex + 1) % 3;
                    }
                    if (e.key.code == Keyboard::Enter)
                    {
                        if (selectedItemIndex == 0)
                            screenMode = "level";
                        else if (selectedItemIndex == 1)
                        {
                            screenMode = "scoreboard";

                            // Read from file
                            std::ifstream fin("score.txt");

                            while (fin >> scores[count])
                                count++;
                            fin.close();

                            for (int i = 0; i < count - 1; i++)
                            {
                                for (int j = 0; j < count - i - 1; j++)
                                {
                                    if (scores[j] < scores[j + 1])
                                    {
                                        std::swap(scores[j], scores[j + 1]);
                                    }
                                }
                            }

                            // Create text objects
                            Text scoreTexts[5];
                            for (int i = 0; i < 5 && i < count; i++)
                            {
                                scoreTexts[i].setFont(font);
                                scoreTexts[i].setCharacterSize(24);
                                scoreTexts[i].setFillColor(Color::Cyan);
                                scoreTexts[i].setString("Score: " + std::to_string(scores[i]));
                                scoreTexts[i].setPosition(300, 150 + i * 40);
                            }

                            Text escText;
                            escText.setFont(font);
                            escText.setCharacterSize(20);
                            escText.setFillColor(Color::White);
                            escText.setString("Press ESC to return");
                            escText.setPosition(270, 400);

                            // Scoreboard loop
                            while (window.isOpen() && screenMode == "scoreboard")
                            {
                                Event e;
                                while (window.pollEvent(e))
                                {
                                    if (e.type == Event::Closed)
                                        window.close();
                                    if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape)
                                    {
                                        screenMode = "menu";
                                    }
                                }

                                window.clear();
                                for (int i = 0; i < 5 && i < count; i++)
                                    window.draw(scoreTexts[i]);
                                window.draw(escText);
                                window.display();
                            }
                        }

                        else if (selectedItemIndex == 2)
                            window.close();
                    }
                }
            }
            window.clear();
            for (int i = 0; i < 3; i++)
            {
                if (i == selectedItemIndex)
                    menu[i].setFillColor(Color::Red);
                else
                    menu[i].setFillColor(Color::White);
                window.draw(menu[i]);
            }
            window.display();
        }
        else if (screenMode == "level")
        {
            string levelOptions[4] = {"Easy", "Medium", "Hard", "Continuous"};
            int selectedLevelIndex = 0;
            string selectedLevel = "";
            Text levelTexts[4];
            for (int i = 0; i < 4; ++i)
            {
                levelTexts[i].setFont(font);
                levelTexts[i].setCharacterSize(28);
                levelTexts[i].setPosition(320, 150 + i * 50);
                levelTexts[i].setString(levelOptions[i]);
            }

            Text escText;
            escText.setFont(font);
            escText.setCharacterSize(20);
            escText.setFillColor(Color::White);
            escText.setString("Press ESC to return");
            escText.setPosition(280, 420);

            while (window.isOpen() && screenMode == "level")
            {
                Event e;
                while (window.pollEvent(e))
                {
                    if (e.type == Event::Closed)
                        window.close();

                    if (e.type == Event::KeyPressed)
                    {
                        if (e.key.code == Keyboard::Up)
                        {
                            selectedLevelIndex--;
                            if (selectedLevelIndex < 0)
                                selectedLevelIndex = 3;
                        }
                        else if (e.key.code == Keyboard::Down)
                        {
                            selectedLevelIndex++;
                            if (selectedLevelIndex > 3)
                                selectedLevelIndex = 0;
                        }
                        else if (e.key.code == Keyboard::Enter)
                        {
                            selectedLevel = levelOptions[selectedLevelIndex];
                            screenMode = "player"; // Or however you switch to gameplay
                        }
                        else if (e.key.code == Keyboard::Escape)
                        {
                            screenMode = "menu";
                        }
                    }
                }

                window.clear();

                for (int i = 0; i < 4; ++i)
                {
                    if (i == selectedLevelIndex)
                        levelTexts[i].setFillColor(Color::Yellow);
                    else
                        levelTexts[i].setFillColor(Color::White);
                    window.draw(levelTexts[i]);
                }

                window.draw(escText);
                window.display();
            }
            // reset the enemies position
            for (int i = 0; i < enemyCount; i++)
            {
                a[i].x = a[i].y = 300;
                a[i].dx = 4 - rand() % 8;
                a[i].dy = 4 - rand() % 8;
            }
            enemyCount = 0;
            // Use `selectedLevel` later for difficulty logic
            if (selectedLevelIndex == 0)
            {
                enemyCount = 2;
                level_enemy = 1;
            }
            else if (selectedLevelIndex == 1)
            {
                enemyCount = 4;
                level_enemy = 2;
            }
            else if (selectedLevelIndex == 2)
            {
                enemyCount = 6;
                level_enemy = 3;
            }
            else if (selectedLevelIndex == 3)
            {
                enemyCount = 2;
                level_enemy = 4;
                enemyClock.restart();
            }
        }
        else if (screenMode == "player")
        {

            Text titleText;
            titleText.setFont(font);
            titleText.setCharacterSize(30);
            titleText.setFillColor(Color::White);
            titleText.setString("Select Player Mode");
            titleText.setPosition(250, 150);

            Text optionText;
            optionText.setFont(font);
            optionText.setCharacterSize(24);
            optionText.setFillColor(Color::Cyan);
            optionText.setString("Press T for Two Players\nPress O for One Player");
            optionText.setPosition(240, 230);

            Text escText;
            escText.setFont(font);
            escText.setCharacterSize(20);
            escText.setFillColor(Color::White);
            escText.setString("Press ESC to return");
            escText.setPosition(280, 400);

            while (window.isOpen() && screenMode == "player")
            {

                Event e;
                while (window.pollEvent(e))
                {
                    if (e.type == Event::Closed)
                        window.close();

                    if (e.type == Event::KeyPressed)
                    {
                        if (e.key.code == Keyboard::T)
                        {
                            players = 2;
                            screenMode = "game";
                            reset();
                            x2 = N - 1;
                            y2 = M - 1;
                            Game = true;
                        }
                        else if (e.key.code == Keyboard::O)
                        {
                            players = 1;
                            screenMode = "game";
                            Game = true;
                            reset();
                            x2 = -10;
                            y2 = -10;
                        }
                        else if (e.key.code == Keyboard::Escape)
                        {
                            screenMode = "menu";
                        }
                    }
                }

                window.clear();
                window.draw(titleText);
                window.draw(optionText);
                window.draw(escText);
                window.display();
            }

            tiles_player_1 = 0;
            tiles_player_2 = 0;

            gameClock.restart(); // Restart the game clock when entering the game
        }
        else if (screenMode == "game")
        {
            if (x1 == x2 && y1 == y2 && players == 2)
            {
                Game = false;
            }
            Event e;
            while (window.pollEvent(e))
            {
                if (e.type == Event::Closed)
                    window.close();

                if (e.type == Event::KeyPressed)
                    if (e.key.code == Keyboard::Escape)
                    {
                        reset();
                        screenMode = "menu";
                        x1 = y1 = 0;
                        x2 = N - 1;
                        y2 = M - 1;
                        p1x = p1y = 0;
                        tiles_player_1 = 0;
                        tiles_player_2 = 0;
                        score_1 = score_2 = 0;
                        power_1 = power_2 = 0;
                        enemyCount = 0;
                        bonus_1 = bonus_2 = 0;
                        bonusApplier_1 = bonusApplier_2 = 10;
                        scoreMultiplier_1 = scoreMultiplier_2 = 2;
                        previous_powerup_score_1 = previous_powerup_score_2 = 0;
                        players = 0;
                    }

                // Freeze enemies with Q (Player 1)
                if (Keyboard::isKeyPressed(Keyboard::Q) && power_1 > 0 && !enemiesFrozen)
                {
                    enemiesFrozen = true;
                    freezeClock.restart();
                    power_1--;
                }

                // Freeze enemies with P (Player 2)
                if (players == 2 && Keyboard::isKeyPressed(Keyboard::P) && power_2 > 0 && !enemiesFrozen)
                {
                    enemiesFrozen = true;
                    freezeClock.restart();
                    power_2--;
                }
            }

            if (Keyboard::isKeyPressed(Keyboard::A))
            {
                dx1 = -1;
                dy1 = 0;
            }
            if (Keyboard::isKeyPressed(Keyboard::D))
            {
                dx1 = 1;
                dy1 = 0;
            }
            if (Keyboard::isKeyPressed(Keyboard::W))
            {
                dx1 = 0;
                dy1 = -1;
            }
            if (Keyboard::isKeyPressed(Keyboard::S))
            {
                dx1 = 0;
                dy1 = 1;
            }

            if (players == 2)
            {
                // Player 2 - Arrow Keys
                if (Keyboard::isKeyPressed(Keyboard::Left))
                {
                    dx2 = -1;
                    dy2 = 0;
                }
                if (Keyboard::isKeyPressed(Keyboard::Right))
                {
                    dx2 = 1;
                    dy2 = 0;
                }
                if (Keyboard::isKeyPressed(Keyboard::Up))
                {
                    dx2 = 0;
                    dy2 = -1;
                }
                if (Keyboard::isKeyPressed(Keyboard::Down))
                {
                    dx2 = 0;
                    dy2 = 1;
                }
            }

            if (!Game)
            {
                window.draw(sGameover);
                window.display();
                reset();
                if (players == 2)
                {
                    sleep(milliseconds(1000));
                    screenMode = "gameover_twoplayer";
                }
                else
                {
                    sleep(milliseconds(1000));
                    screenMode = "gameover_oneplayer";
                }
                continue;
            }

            if (timer > delay)
            {

                p1x = x1;
                p1y = y1;
                p2x = x2;
                p2y = y2;

                x1 += dx1;
                y1 += dy1;

                if (x1 < 0)
                    x1 = 0;
                if (x1 > N - 1)
                    x1 = N - 1;
                if (y1 < 0)
                    y1 = 0;
                if (y1 > M - 1)
                    y1 = M - 1;

                if (grid[y1][x1] == 2)
                    Game = false;

                if (grid[y1][x1] == 0)
                    grid[y1][x1] = 2;

                x2 += dx2;
                y2 += dy2;

                if (x2 < 0)
                    x2 = 0;
                if (x2 > N - 1)
                    x2 = N - 1;
                if (y2 < 0)
                    y2 = 0;
                if (y2 > M - 1)
                    y2 = M - 1;

                if (grid[y2][x2] == 3)
                    Game = false;
                if (grid[y2][x2] == 0)
                    grid[y2][x2] = 3;

                timer = 0;
            }

            if (!enemiesFrozen)
            {
                for (int i = 0; i < enemyCount; i++)
                    a[i].move();
            }
            else
            {
                if (freezeClock.getElapsedTime().asSeconds() >= 3)
                {
                    enemiesFrozen = false;
                }
            }

            if (grid[y1][x1] == 1)
            {
                dx1 = dy1 = 0;

                if (grid[p1y][p1x] == 2)
                {
                    for (int i = 0; i < enemyCount; i++)
                        drop(a[i].y / ts, a[i].x / ts);

                    int tiles = 0;
                    for (int i = 0; i < M; i++)
                        for (int j = 0; j < N; j++)
                            if (grid[i][j] == -1)
                                grid[i][j] = 0;
                            else if (grid[i][j] == 2 || grid[i][j] == 0)
                            {
                                tiles++;
                                tiles_player_1++;
                                grid[i][j] = 1;
                            }

                    if (tiles > bonusApplier_1)
                    {
                        score_1 += (tiles * scoreMultiplier_1);
                        bonus_1 += 1;
                    }
                    else
                    {
                        score_1 += tiles;
                    }

                    if (bonus_1 == 3)
                    {
                        bonusApplier_1 = 5;
                    }
                    else if (bonus_1 > 5)
                    {
                        scoreMultiplier_1 = 4;
                    }

                    // First power-up (at 50)
                    if (score_1 >= 50 && previous_powerup_score_1 < 50)
                    {
                        power_1++;
                        previous_powerup_score_1 = 50;
                    }

                    // Subsequent power-ups every 30 points after 70
                    else if (score_1 >= 70 && score_1 - previous_powerup_score_1 >= 30)
                    {
                        power_1++;
                        previous_powerup_score_1 = score_1;
                    }
                }
            }

            if (grid[y2][x2] == 1 && players == 2)
            {
                dx2 = dy2 = 0;

                if (grid[p2y][p2x] == 3)
                {
                    for (int i = 0; i < enemyCount; i++)
                        drop(a[i].y / ts, a[i].x / ts);

                    int tiles = 0;
                    for (int i = 0; i < M; i++)
                        for (int j = 0; j < N; j++)
                            if (grid[i][j] == -1)
                                grid[i][j] = 0;
                            else if (grid[i][j] == 3 || grid[i][j] == 0)
                            {

                                tiles++;
                                grid[i][j] = 1;
                                tiles_player_2++;
                            }

                    if (tiles > bonusApplier_2)
                    {
                        score_2 += (tiles * scoreMultiplier_2);
                        bonus_2 += 1;
                    }
                    else
                    {
                        score_2 += tiles;
                    }

                    if (bonus_2 == 3)
                    {
                        bonusApplier_2 = 5;
                    }
                    else if (bonus_2 > 5)
                    {
                        scoreMultiplier_2 = 4;
                    }

                    // First power-up (at 50)
                    if (score_2 >= 50 && previous_powerup_score_2 < 50)
                    {
                        power_2++;
                        previous_powerup_score_2 = 50;
                    }

                    // Subsequent power-ups every 30 points after 70
                    else if (score_2 >= 70 && score_2 - previous_powerup_score_2 >= 30)
                    {
                        power_2++;
                        previous_powerup_score_2 = score_2;
                    }
                }
            }

            for (int i = 0; i < enemyCount; i++)
                if (grid[a[i].y / ts][a[i].x / ts] == 2 || grid[a[i].y / ts][a[i].x / ts] == 3)
                    Game = false;

            if (enemyClock.getElapsedTime().asSeconds() >= 20 && enemyCount < 100 && level_enemy == 4)
            {
                // Ensure the maximum number of enemies doesn't exceed the limit
                if (enemyCount < 100)
                {
                    // Spawn two new enemies at the previous positions
                    a[enemyCount].x = a[enemyCount - 1].x; // Set to previous enemy's position
                    a[enemyCount].y = a[enemyCount - 1].y;
                    enemyCount++;

                    a[enemyCount].x = a[enemyCount - 1].x; // Set to previous enemy's position
                    a[enemyCount].y = a[enemyCount - 1].y;
                    enemyCount++;

                    enemyClock.restart(); // Restart the clock for next interval
                }
            }

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
                    if (grid[i][j] == 3)                              // For Player 2's trail
                        sTile.setTextureRect(IntRect(ts, 0, ts, ts)); // Adjust texture as needed
                    sTile.setPosition(j * ts, i * ts + 50);           // Add 50 pixel vertical offset
                    window.draw(sTile);
                }

            sTile.setTextureRect(IntRect(36, 0, ts, ts));
            sTile.setPosition(x1 * ts, y1 * ts + 50); // Player position offset
            window.draw(sTile);

            if (players == 2)
            {
                sTile.setTextureRect(IntRect(36 + ts + ts, 0, ts, ts));
                sTile.setPosition(x2 * ts, y2 * ts + 50); // Player 2 position offset
                window.draw(sTile);
            }

            sEnemy.rotate(10);
            for (int i = 0; i < enemyCount; i++)
            {
                sEnemy.setPosition(a[i].x, a[i].y + 50); // Enemy position offset
                window.draw(sEnemy);
            }

            Time elapsed = gameClock.getElapsedTime();
            int seconds = static_cast<int>(elapsed.asSeconds());
            timeText.setString("Time: " + std::to_string(seconds) + " sec");

            // Then draw the text before window.display()
            window.draw(timeText);

            player1Text.setString("Player 1 score: " + std::to_string(score_1));
            window.draw(player1Text);
            // In your game loop, draw the text
            if (players == 2)
            {
                player2Text.setString("Player 2 score: " + std::to_string(score_2));
                window.draw(player2Text);
            }

            powerupText1.setString("P1 Power: " + std::to_string(power_1));
            window.draw(powerupText1);

            if (players == 2)
            {
                powerupText2.setString("P2 Power: " + std::to_string(power_2));
                window.draw(powerupText2);
            }

            window.display();
        }
        else if (screenMode == "gameover_twoplayer")
        {

            sf::Text resultText, menuText;

            resultText.setFont(font);
            resultText.setCharacterSize(30);
            resultText.setFillColor(sf::Color::Magenta);
            resultText.setStyle(sf::Text::Bold);
            resultText.setPosition(100, 100);

            // Compare scores and set result
            if (score_1 > score_2)
                resultText.setString("Player 1 Wins!");
            else if (score_2 > score_1)
                resultText.setString("Player 2 Wins!");
            else
                resultText.setString("It's a Draw!");

            // Menu text
            menuText.setFont(font);
            menuText.setCharacterSize(24);
            menuText.setFillColor(sf::Color::Yellow);
            menuText.setPosition(400, 150); // Right side of screen
            menuText.setString("Game Over Menu:\n\n"
                               "Press R - Restart\n"
                               "Press M - Main Menu\n"
                               "Press E - Exit");

            // Draw results and menu
            window.clear();
            window.draw(resultText);
            window.draw(menuText);
            window.display();

            // Event handling for gameover screen
            Event event;
            while (window.pollEvent(event))
            {
                if (event.type == Event::Closed)
                    window.close();

                if (event.type == Event::KeyPressed)
                {
                    if (event.key.code == Keyboard::R) // Restart
                    {
                        reset(); // your game reset function
                        x1 = y1 = 0;
                        x2 = N - 1;
                        y2 = M - 1;
                        p1x = p1y = 0;
                        tiles_player_1 = 0;
                        tiles_player_2 = 0;
                        score_1 = score_2 = 0;
                        power_1 = power_2 = 0;
                        enemyCount = 0;
                        bonus_1 = bonus_2 = 0;
                        bonusApplier_1 = bonusApplier_2 = 10;
                        scoreMultiplier_1 = scoreMultiplier_2 = 2;
                        previous_powerup_score_1 = previous_powerup_score_2 = 0;
                        Game = true;
                        gameClock.restart(); // Reset timer
                        screenMode = "level";
                    }
                    else if (event.key.code == Keyboard::M) // Main menu
                    {
                        reset();
                        screenMode = "menu";
                        x1 = y1 = 0;
                        x2 = N - 1;
                        y2 = M - 1;
                        p1x = p1y = 0;
                        tiles_player_1 = 0;
                        tiles_player_2 = 0;
                        score_1 = score_2 = 0;
                        power_1 = power_2 = 0;
                        enemyCount = 0;
                        bonus_1 = bonus_2 = 0;
                        bonusApplier_1 = bonusApplier_2 = 10;
                        scoreMultiplier_1 = scoreMultiplier_2 = 2;
                        previous_powerup_score_1 = previous_powerup_score_2 = 0;
                    }
                    else if (event.key.code == Keyboard::E) // Exit
                    {
                        window.close();
                    }
                }
            }
        }
        else if (screenMode == "gameover_oneplayer")
        {
            sf::Text resultText, menuText, scoreText, highScoreText;

            resultText.setFont(font);
            resultText.setCharacterSize(30);
            resultText.setFillColor(sf::Color::Magenta);
            resultText.setStyle(sf::Text::Bold);
            resultText.setPosition(100, 100);
            resultText.setString("Game Over!");

            // Show player's score
            scoreText.setFont(font);
            scoreText.setCharacterSize(22);
            scoreText.setFillColor(sf::Color::White);
            scoreText.setPosition(100, 160);
            scoreText.setString("Your Score: " + std::to_string(score_1));

            // Check high score
            int highScore = 0;
            std::ifstream inFile("score.txt");
            if (inFile.is_open())
            {
                int score;
                while (inFile >> score)
                {
                    if (score > highScore)
                        highScore = score;
                }
                inFile.close();
            }

            // Display high score message if beaten
            if (tiles_player_1 > highScore)
            {
                highScoreText.setFont(font);
                highScoreText.setCharacterSize(24);
                highScoreText.setFillColor(sf::Color::Green);
                highScoreText.setStyle(sf::Text::Bold);
                highScoreText.setPosition(100, 220);
                highScoreText.setString("New High Score!");
            }

            // Append score to file
            std::ofstream outFile("score.txt", std::ios::app);
            if (outFile.is_open())
            {
                outFile << tiles_player_1 << "\n";
                outFile.close();
            }

            // Game over menu
            menuText.setFont(font);
            menuText.setCharacterSize(24);
            menuText.setFillColor(sf::Color::Yellow);
            menuText.setPosition(400, 150);
            menuText.setString("Game Over Menu:\n\n"
                               "Press R - Restart\n"
                               "Press M - Main Menu\n"
                               "Press E - Exit");

            while (window.isOpen() && screenMode == "gameover_oneplayer")
            {

                // Handle input
                Event event;
                while (window.pollEvent(event))
                {
                    if (event.type == Event::Closed)
                        window.close();

                    if (event.type == Event::KeyPressed)
                    {
                        if (event.key.code == Keyboard::R) // Restart
                        {
                            reset();
                            x1 = y1 = 0;
                            x2 = N - 1;
                            y2 = M - 1;
                            p1x = p1y = 0;
                            tiles_player_1 = 0;
                            tiles_player_2 = 0;
                            score_1 = score_2 = 0;
                            power_1 = power_2 = 0;
                            enemyCount = 0;
                            bonus_1 = bonus_2 = 0;
                            bonusApplier_1 = bonusApplier_2 = 10;
                            scoreMultiplier_1 = scoreMultiplier_2 = 2;
                            previous_powerup_score_1 = previous_powerup_score_2 = 0;
                            Game = true;
                            gameClock.restart();
                            screenMode = "level";
                        }
                        else if (event.key.code == Keyboard::M) // Main menu
                        {
                            reset();
                            x1 = y1 = 0;
                            x2 = N - 1;
                            y2 = M - 1;
                            p1x = p1y = 0;
                            tiles_player_1 = 0;
                            tiles_player_2 = 0;
                            score_1 = score_2 = 0;
                            power_1 = power_2 = 0;
                            enemyCount = 0;
                            bonus_1 = bonus_2 = 0;
                            bonusApplier_1 = bonusApplier_2 = 10;
                            scoreMultiplier_1 = scoreMultiplier_2 = 2;
                            previous_powerup_score_1 = previous_powerup_score_2 = 0;
                            screenMode = "menu";
                        }
                        else if (event.key.code == Keyboard::E) // Exit
                        {
                            window.close();
                        }
                    }
                }
                // Draw everything
                window.clear();
                window.draw(resultText);
                window.draw(scoreText);
                if (tiles_player_1 > highScore)
                    window.draw(highScoreText);
                window.draw(menuText);
                window.display();
            }
        }
    }

    delete[] scores;

    return 0;
}