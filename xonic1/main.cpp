#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>

using namespace sf;
using namespace std;

const int row = 25;
const int cols = 40;

int grid[row][cols] = {0};

int tileSize = 18;

int level = 1;

int game_mode_select = 1;

int scores[100] = {0};
int scoreCount = 0;

void readScores()
{
    ifstream file("scores.txt");
    scoreCount = 0;

    while (file >> scores[scoreCount] && scoreCount < 100)
    {
        scoreCount++;
    }

    file.close();
}

int getHighestScore()
{
    if (scoreCount == 0)
        return 0;

    int maxScore = scores[0];
    for (int i = 1; i < scoreCount; i++)
    {
        if (scores[i] > maxScore)
            maxScore = scores[i];
    }

    return maxScore;
}

void writeScores()
{
    ofstream file("scores.txt");

    for (int i = 0; i < scoreCount; i++)
    {
        file << scores[i] << endl;
    }

    file.close();
}

/*
1 - easy
2 - medium
3 - hard
4 - continous
 */

struct Enemy
{
    int x, y;
    int dx, dy;

    Enemy()
    {
        x = y = 300;
        do{
            dx = 4 - rand() % 8;
            dy = 4 - rand() % 8;
        } while (dx==0 || dy==0);
        
    }

    void move()
    {
        x += dx;
        if (grid[y / tileSize][x / tileSize] == 1)
        {
            dx = -dx;
            x += dx;
        }
        y += dy;
        if (grid[y / tileSize][x / tileSize] == 1)
        {
            dy = -dy;
            y += dy;
        }
    }
};

void drop(int r, int c)
{
    if (grid[r][c] == 0)
    {
        grid[r][c] = -1;
        // tiles collected
    }
    if (grid[r - 1][c] == 0)
        drop(r - 1, c);
    if (grid[r + 1][c] == 0)
        drop(r + 1, c);
    if (grid[r][c - 1] == 0)
        drop(r, c - 1);
    if (grid[r][c + 1] == 0)
        drop(r, c + 1);
}

void loadTextures(Texture &t1, Texture &t2, Texture &t3)
{
    if (!t1.loadFromFile("images/tiles.png"))
        cout << "Error loading tiles texture" << endl;
    if (!t2.loadFromFile("images/gameover.png"))
        cout << "Error loading gameover texture" << endl;
    if (!t3.loadFromFile("images/enemy.png"))
        cout << "Error loading enemy texture" << endl;
}

void setupGrid()
{
    for (int i = 0; i < row; i++)
        for (int j = 0; j < cols; j++)
            if (i == 0 || j == 0 || i == row - 1 || j == cols - 1)
                grid[i][j] = 1;
            else
                grid[i][j] = 0;
}

bool menu_selection = false;
bool game_type_selection = false;
bool level_selection = false;
bool game_started = false;
bool showScoreBoard = false;

void drawText(RenderWindow &window, const std::string &textStr, float scaleX, float scaleY, float posX, float posY, Color color)
{
    static Font f;
    static bool fontLoaded = false;

    if (!fontLoaded)
    {
        if (!f.loadFromFile("fonts/superComic.ttf")) // Change path if needed
        {
            cout << "Failed to load font!" << endl;
            return;
        }
        fontLoaded = true;
    }

    Text t;
    t.setFont(f);
    t.setString(textStr);
    t.setCharacterSize(30); // You can make this parameter too if needed
    t.setFillColor(color);
    t.setScale(scaleX, scaleY);
    t.setPosition(posX, posY);

    window.draw(t);
}

void setMenuSpritePositions(Sprite &sStart, Sprite &sLevel, Sprite &sScore, Sprite &sExit)
{
    float startX = ((row * tileSize) / 2.0f - sStart.getGlobalBounds().width / 2.0f) + 150.0f;
    float startY = 100.0f;
    float spacing = 70.0f;

    sStart.setPosition(startX, startY);
    sLevel.setPosition(startX, startY + spacing);
    sScore.setPosition(startX, startY + 2 * spacing);
    sExit.setPosition(startX + 70, startY + 3 * spacing);
}

void setMenuSpriteForSelection(Sprite &one, Sprite &two, Sprite &Back)
{
    float startX = ((row * tileSize) / 2.0f - one.getGlobalBounds().width / 2.0f) + 150.0f;
    float startY = 100.0f;
    float spacing = 70.0f;

    one.setPosition(startX + 10, startY);
    two.setPosition(startX, startY + spacing);
    Back.setPosition(startX + 65, startY + 2 * spacing);
}

void setGameOver(Sprite &m, Sprite &r, Sprite &e)
{
    float startX = ((row * tileSize) / 2.0f - m.getGlobalBounds().width / 2.0f) + 150.0f;
    float startY = 100.0f;
    float spacing = 70.0f;

    m.setPosition(startX + 175, startY);
    r.setPosition(startX + 200, startY + spacing);
    e.setPosition(startX + 225, startY + 2 * spacing);
}

void setMenuSpriteForLevelSelection(Sprite &easy, Sprite &medium, Sprite &hard, Sprite &continueBtn, Sprite &back)
{
    float startX = ((row * tileSize) / 2.0f - easy.getGlobalBounds().width / 2.0f) + 150.0f;
    float startY = 50.0f;
    float spacing = 70.0f;

    easy.setPosition(startX - 10, startY);
    medium.setPosition(startX - 20, startY + spacing);
    hard.setPosition(startX - 10, startY + 2 * spacing);
    continueBtn.setPosition(startX - 130, startY + 3 * spacing);
    back.setPosition(startX, startY + 4 * spacing);
}

// Function to check if a button is clicked
void checkButtonClicked(Vector2i mousePos, Sprite &b)
{
    if (b.getGlobalBounds().contains(static_cast<Vector2f>(mousePos)))
    {
        cout << "Back clicked!" << endl;
        showScoreBoard = false;
        menu_selection = true;
    }
}

// Function to check if a button is clicked
void checkButtonClicked(Vector2i mousePos, Sprite &sStart, Sprite &sLevel, Sprite &sScore, Sprite &sExit)
{
    if (sStart.getGlobalBounds().contains(static_cast<Vector2f>(mousePos)))
    {
        menu_selection = false;
        game_type_selection = true;
        cout << "Start Game clicked!" << endl;
    }
    else if (sLevel.getGlobalBounds().contains(static_cast<Vector2f>(mousePos)))
    {
        level_selection = true;
        menu_selection = false;
        cout << "Select Level clicked!" << endl;
    }
    else if (sScore.getGlobalBounds().contains(static_cast<Vector2f>(mousePos)))
    {
        menu_selection = false;
        showScoreBoard = true;
        cout << "Scoreboard clicked!" << endl;
    }
    else if (sExit.getGlobalBounds().contains(static_cast<Vector2f>(mousePos)))
    {
        cout << "Exit clicked!" << endl;
        exit(0);
    }
}

void checkButtonClicked(Vector2i mousePos, Sprite &easy, Sprite &med, Sprite &hard, Sprite &cont, Sprite &back)
{
    if (easy.getGlobalBounds().contains(static_cast<Vector2f>(mousePos)))
    {
        menu_selection = true;
        level = 1;
        level_selection = false;
        cout << "Easy Level clicked!" << endl;
    }
    else if (med.getGlobalBounds().contains(static_cast<Vector2f>(mousePos)))
    {
        menu_selection = true;
        level = 2;
        level_selection = false;
        cout << "Medium Level clicked!" << endl;
    }
    else if (hard.getGlobalBounds().contains(static_cast<Vector2f>(mousePos)))
    {
        menu_selection = true;
        level = 3;
        level_selection = false;
        cout << "Hard Level clicked!" << endl;
    }
    else if (cont.getGlobalBounds().contains(static_cast<Vector2f>(mousePos)))
    {
        menu_selection = true;
        level = 4;
        level_selection = false;
        cout << "Continue clicked!" << endl;
    }
    else if (back.getGlobalBounds().contains(static_cast<Vector2f>(mousePos)))
    {
        cout << "Back clicked!" << endl;
        menu_selection = true;
        game_type_selection = false;
        level_selection = false;
    }
}

void checkButtonClicked(Vector2i mousePos, Sprite &o, Sprite &t, Sprite &b)
{
    if (o.getGlobalBounds().contains(static_cast<Vector2f>(mousePos)))
    {
        game_mode_select = 1;
        cout << "One Player clicked!" << endl;
        game_started = true;
        game_type_selection = false;
    }
    else if (t.getGlobalBounds().contains(static_cast<Vector2f>(mousePos)))
    {
        game_mode_select = 2;
        game_started = true;
        game_type_selection = false;
        cout << "Two Player clicked!" << endl;
    }
    else if (b.getGlobalBounds().contains(static_cast<Vector2f>(mousePos)))
    {
        cout << "Back clicked!" << endl;
        menu_selection = true;
        game_type_selection = false;
    }
}

void checkButtonOver(Vector2i mousePos, Sprite &m, Sprite &r, Sprite &e)
{
    if (m.getGlobalBounds().contains(static_cast<Vector2f>(mousePos)))
    {
        cout << "Menu clicked!" << endl;
        menu_selection = true;
    }
    else if (r.getGlobalBounds().contains(static_cast<Vector2f>(mousePos)))
    {

        cout << "Restart clicked!" << endl;
    }
    else if (e.getGlobalBounds().contains(static_cast<Vector2f>(mousePos)))
    {
        cout << "Exit clicked!" << endl;
        exit(0);
    }
}

void gameMode(RenderWindow &window)
{

    Texture bg1;
    bg1.loadFromFile("images/bg1.png");

    Sprite sBg1(bg1);
    sBg1.setPosition(0, 0);
    sBg1.setScale(static_cast<float>(window.getSize().x) / bg1.getSize().x,
                  static_cast<float>(window.getSize().y) / bg1.getSize().y);

    Texture b1, b2, b3;

    b1.loadFromFile("images/one.png");
    b2.loadFromFile("images/two.png");
    b3.loadFromFile("images/back.png");

    Sprite sOnePlayer(b1), sTwoPlayer(b2), sBack(b3);

    setMenuSpriteForSelection(sOnePlayer, sTwoPlayer, sBack);

    while (window.isOpen())
    {
        Event e;

        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();

            if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
            {
                Vector2i mousePos = Mouse::getPosition(window);
                checkButtonClicked(mousePos, sOnePlayer, sTwoPlayer, sBack);
                return;
            }
        }

        window.clear();

        // Draw background and buttons
        window.draw(sBg1);
        window.draw(sOnePlayer);
        window.draw(sTwoPlayer);
        window.draw(sBack);

        window.display();
    }
}

void menu(RenderWindow &window)
{
    Texture bg;
    bg.loadFromFile("images/bg2.png");

    Sprite sBg(bg);
    sBg.setPosition(0, 0);
    sBg.setScale(static_cast<float>(window.getSize().x) / bg.getSize().x,
                 static_cast<float>(window.getSize().y) / bg.getSize().y);

    Texture b1, b2, b3, b4;
    b1.loadFromFile("images/start.png");
    b2.loadFromFile("images/level.png");
    b3.loadFromFile("images/score.png");
    b4.loadFromFile("images/exit.png");

    Sprite sStart(b1), sLevel(b2), sScore(b3), sExit(b4);

    setMenuSpritePositions(sStart, sLevel, sScore, sExit);

    while (window.isOpen())
    {
        Event e;

        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();

            if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
            {
                Vector2i mousePos = Mouse::getPosition(window);
                checkButtonClicked(mousePos, sStart, sLevel, sScore, sExit);
                return;
            }
        }

        window.clear();

        // Draw background and buttons
        window.draw(sBg);
        window.draw(sStart);
        window.draw(sLevel);
        window.draw(sScore);
        window.draw(sExit);

        window.display();
    }
}

void levelSelection(RenderWindow &window)
{
    Texture bg;
    bg.loadFromFile("images/bg3.png");

    Sprite sBg(bg);
    sBg.setPosition(0, 0);
    sBg.setScale(static_cast<float>(window.getSize().x) / bg.getSize().x,
                 static_cast<float>(window.getSize().y) / bg.getSize().y);

    Texture b1, b2, b3, b4, b5;
    b1.loadFromFile("images/easy.png");
    b2.loadFromFile("images/medium.png");
    b3.loadFromFile("images/hard.png");
    b4.loadFromFile("images/continue.png");
    b5.loadFromFile("images/back2.png");

    Sprite sEasy(b1), sMedium(b2), sHard(b3), sContinue(b4), sBack(b5);
    setMenuSpriteForLevelSelection(sEasy, sMedium, sHard, sContinue, sBack);

    while (window.isOpen())
    {
        Event e;

        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();

            if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
            {
                Vector2i mousePos = Mouse::getPosition(window);
                checkButtonClicked(mousePos, sEasy, sMedium, sHard, sContinue, sBack);
                return;
            }
        }

        window.clear();

        // Draw background and buttons
        window.draw(sBg);
        window.draw(sEasy);
        window.draw(sMedium);
        window.draw(sHard);
        window.draw(sContinue);
        window.draw(sBack);

        window.display();
    }
}

void GameOverScreen(RenderWindow &window, Clock frameClock, int score, int tiles, int moves, int highScore)
{
    Texture bg;
    bg.loadFromFile("images/bg3.png");

    Sprite sBg(bg);
    sBg.setPosition(0, 0);
    sBg.setScale(static_cast<float>(window.getSize().x) / bg.getSize().x,
                 static_cast<float>(window.getSize().y) / bg.getSize().y);

    Texture b1, b2, b3;

    b1.loadFromFile("images/menu.png");
    b2.loadFromFile("images/restart.png");
    b3.loadFromFile("images/exit.png");

    Sprite sMenu(b1), sRestart(b2), sExit(b3);

    setupGrid();
    setGameOver(sMenu, sRestart, sExit);

    float timeElapsed = frameClock.getElapsedTime().asSeconds();

    string timeStr = " Time: " + to_string(static_cast<int>(timeElapsed)) + " sec";
    string scoreStr = " Score: " + to_string(score);
    string movesStr = " Moves: " + to_string(moves);
    bool isHighScore = score >= highScore;

    while (window.isOpen())
    {
        Event e;

        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();

            if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
            {
                Vector2i mousePos = Mouse::getPosition(window);
                checkButtonOver(mousePos, sMenu, sRestart, sExit);
                return;
            }
        }

        window.clear();

        window.draw(sBg);

        drawText(window, " -- Results -- ", 1.0f, 1.0f, 70, 70, Color::Black);
        drawText(window, timeStr, 1.f, 1.f, 100, 120, Color::Blue);
        drawText(window, scoreStr, 1.f, 1.f, 100, 170, Color::Yellow);
        drawText(window, movesStr, 1.f, 1.f, 100, 220, Color::Magenta);

        if (isHighScore)
        {
            drawText(window, " Got High Score! ", 1.f, 1.f, 55, 270, Color::Red);
        }

        window.draw(sMenu);
        window.draw(sRestart);
        window.draw(sExit);

        window.display();
    }

    
}

void GameOverScreen(RenderWindow &window, Clock frameClock, int p1, int p2)
{
    Texture bg;
    bg.loadFromFile("images/bg1.png");

    Sprite sBg(bg);
    sBg.setPosition(0, 0);
    sBg.setScale(static_cast<float>(window.getSize().x) / bg.getSize().x,
                 static_cast<float>(window.getSize().y) / bg.getSize().y);

    Texture b1, b2, b3;

    b1.loadFromFile("images/menu.png");
    b2.loadFromFile("images/restart.png");
    b3.loadFromFile("images/exit.png");

    Sprite sMenu(b1), sRestart(b2), sExit(b3);

    setupGrid();
    setGameOver(sMenu, sRestart, sExit);

    float timeElapsed = frameClock.getElapsedTime().asSeconds();

    string timeStr = " Time: " + to_string(static_cast<int>(timeElapsed)) + " sec";

    while (window.isOpen())
    {
        Event e;

        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();

            if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
            {
                Vector2i mousePos = Mouse::getPosition(window);
                checkButtonOver(mousePos, sMenu, sRestart, sExit);
                return;
            }
        }

        window.clear();

        window.draw(sBg);

        drawText(window, " -- Results -- ", 1.0f, 1.0f, 70, 70, Color::Black);
        drawText(window, timeStr, 1.f, 1.f, 100, 120, Color::Magenta);
        drawText(window, "RED Score : " + to_string(p1), 1.f, 1.f, 100, 170, Color::Red);
        drawText(window, "YELLOW Score : " + to_string(p2), 1.f, 1.f, 70, 220, Color::Yellow);

        if (p1 > p2)
        {
            drawText(window, "Red Wins!", 1.0f, 1.0f, 100, 270, Color::Green);
        }
        else if (p2 > p1)
        {
            drawText(window, "Yellow Wins!", 1.0f, 1.0f, 100, 270, Color::Green);
        }
        else
        {
            drawText(window, "It's a Tie!", 1.0f, 1.0f, 100, 270, Color::Green);
        }

        window.draw(sMenu);
        window.draw(sRestart);
        window.draw(sExit);

        window.display();
    }
}

void l1(RenderWindow &window)
{
    Texture tileTexture, gameOverTexture, enemyTexture;
    loadTextures(tileTexture, gameOverTexture, enemyTexture);

    Sprite tileSprite(tileTexture), gameOverSprite(gameOverTexture), enemySprite(enemyTexture);
    gameOverSprite.setPosition(100, 100);
    enemySprite.setOrigin(20, 20);

    const int maxEnemies = 2;
    Enemy enemies[maxEnemies];

    bool isGameRunning = true;
    int playerX = 0, playerY = 0, dirX = 0, dirY = 0;
    float animationTimer = 0, animationDelay = 0.07;

    Clock frameClock;
    Clock gameOverClock;
    Clock TimePassed;
    bool gameOverClockStarted = false;

    bool safe_zone = false;

    int moves = 0;
    int tilesCollected = 0;
    int currentScore = 0;

    int prev_high_score = 0;

    int bonus = 0;

    int bonusApplier = 10;

    int scoreMultiplier = 2;

    int powerUps = 0;

    int lastPowerUpScore = 0;

    bool isPowerUpActive = false;

    Clock powerUpClock;

    setupGrid(); // Setup initial game gridTiles

    while (window.isOpen())
    {
        float deltaTime = frameClock.getElapsedTime().asSeconds();
        frameClock.restart();
        animationTimer += deltaTime;

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed && event.key.code == Keyboard::P)
            {
                if (!isPowerUpActive && powerUps > 0)
                {
                    isPowerUpActive = true;
                    powerUps--;
                    powerUpClock.restart();
                }
            }
        }

        if (!isGameRunning)
        {
            if (!gameOverClockStarted)
            {
                gameOverClock.restart();
                gameOverClockStarted = true;
            }
            else if (gameOverClock.getElapsedTime().asSeconds() >= 2.5)
            {
                scores[scoreCount++] = currentScore;
                writeScores();
                cout << "score is written " << endl;
                GameOverScreen(window, TimePassed, currentScore, tilesCollected, ++moves, getHighestScore());

                return;
            }
        }

        // Handle player movement

        if (Keyboard::isKeyPressed(Keyboard::Left))
        {
            dirX = -1;
            dirY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Right))
        {
            dirX = 1;
            dirY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Up))
        {
            dirX = 0;
            dirY = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::Down))
        {
            dirX = 0;
            dirY = 1;
        }

        if (!isGameRunning)
            continue;

        // Move player every frame if enough time has passed
        if (animationTimer > animationDelay)
        {
            playerX += dirX;
            playerY += dirY;

            // Keep player inside grid bounds
            if (playerX < 0)
                playerX = 0;
            if (playerX > cols - 1)
                playerX = cols - 1;
            if (playerY < 0)
                playerY = 0;
            if (playerY > row - 1)
                playerY = row - 1;

            // Check for collision with self
            if (grid[playerY][playerX] == 2)
                isGameRunning = false;

            // Mark visited cell
            if (grid[playerY][playerX] == 0)
            {
                safe_zone = true;
                grid[playerY][playerX] = 2;
            }

            animationTimer = 0;
        }

        // Move all enemies
        if (!isPowerUpActive || powerUpClock.getElapsedTime().asSeconds() >= 3)
        {
            isPowerUpActive = false;

            for (int i = 0; i < maxEnemies; i++)
                enemies[i].move();
        }

        // If player hits a tile (special zone)
        if (grid[playerY][playerX] == 1)
        {
            dirX = dirY = 0;
            if (safe_zone)
            {
                moves += 1;
            }

            safe_zone = false;

            // Drop traps under enemy positions
            for (int i = 0; i < maxEnemies; i++)
                drop(enemies[i].y / tileSize, enemies[i].x / tileSize);

            // Reset grid

            int tileNow = 0;

            for (int i = 0; i < row; i++)
                for (int j = 0; j < cols; j++)
                    if (grid[i][j] == -1)
                        grid[i][j] = 0;
                    else if (grid[i][j] == 0 || grid[i][j] == 2)
                    {

                        tileNow += 1;
                        
                        grid[i][j] = 1;
                    }

            tilesCollected += tileNow;

            if (tileNow > bonusApplier)
            {
                currentScore += (tileNow * scoreMultiplier);
                bonus += 1;
            }
            else
            {
                currentScore += tileNow;
            }

            if (bonus == 3)
            {
                bonusApplier = 5;
            }
            else if (bonus > 5)
            {
                scoreMultiplier = 4;
            }

            // First power-up (at 50)
            if (currentScore >= 50 && lastPowerUpScore < 50)
            {
                powerUps++;
                lastPowerUpScore = 50;
            }

            // Subsequent power-ups every 30 points after 70
            else if (currentScore >= 70 && currentScore - lastPowerUpScore >= 30)
            {
                powerUps++;
                lastPowerUpScore = currentScore;
            }
        }

        // End game if enemy touches player trail
        for (int i = 0; i < maxEnemies; i++)
            if (grid[enemies[i].y / tileSize][enemies[i].x / tileSize] == 2)
                isGameRunning = false;

        ///////// DRAW SECTION /////////
        // Check if power-up is active and applied
        if (isPowerUpActive)
        {
            // Change background color to white
            window.clear(Color(33, 33, 36));
        }
        else
        {
            // Default background color when power-up is not active
            window.clear(Color::Black);
        }

        // Rest of the game logic...

        // Draw grid tiles
        for (int i = 0; i < row; i++)
            for (int j = 0; j < cols; j++)
            {
                if (grid[i][j] == 0)
                    continue;

                if (grid[i][j] == 1)
                    tileSprite.setTextureRect(IntRect(0, 0, tileSize, tileSize));
                else if (grid[i][j] == 2)
                    tileSprite.setTextureRect(IntRect(54, 0, tileSize, tileSize));

                tileSprite.setPosition(j * tileSize, i * tileSize);
                window.draw(tileSprite);
            }

        // Draw player
        tileSprite.setTextureRect(IntRect(36, 0, tileSize, tileSize));
        tileSprite.setPosition(playerX * tileSize, playerY * tileSize);
        window.draw(tileSprite);

        // Draw rotating enemies
        enemySprite.rotate(10);
        for (int i = 0; i < maxEnemies; i++)
        {
            enemySprite.setPosition(enemies[i].x, enemies[i].y);
            window.draw(enemySprite);
        }

        // Show game over screen if not running
        if (!isGameRunning)
            window.draw(gameOverSprite);

        // Time display
        int timeElapsed = static_cast<int>(TimePassed.getElapsedTime().asSeconds());
        drawText(window, "Time: " + to_string(timeElapsed) + " sec", 0.5f, 0.5f, 25.0f, 470.0f, Color::Yellow);

        // Score display
        drawText(window, "Score: " + to_string(currentScore), 0.5f, 0.5f, 180.0f + 30.f, 470.0f, Color::Cyan);

        // Tiles collected display
        drawText(window, "Tiles: " + to_string(tilesCollected), 0.5f, 0.5f, 350.0f + 30.f, 470.0f, Color::Green);

        // Power-up display
        drawText(window, "Power-Ups: " + to_string(powerUps), 0.5f, 0.5f, 540.0f, 470.0f, Color::Magenta);

        if (isPowerUpActive)
        {
            // Display power-up message
            float remainingTime = 3.0f - powerUpClock.getElapsedTime().asSeconds();

            // Display the remaining time
            if (remainingTime > 0.0f)
            {
                drawText(window, "Power Activated! Time Left: " + to_string(static_cast<int>(remainingTime)) + "s",
                         0.5f, 0.5f, 50.0f, 50.0f, Color::Red);
            }
        }

        window.display();
    }
}

void l2(RenderWindow &window)
{
    Texture tileTexture, gameOverTexture, enemyTexture;
    loadTextures(tileTexture, gameOverTexture, enemyTexture);

    Sprite tileSprite(tileTexture), gameOverSprite(gameOverTexture), enemySprite(enemyTexture);
    gameOverSprite.setPosition(100, 100);
    enemySprite.setOrigin(20, 20);

    const int maxEnemies = 4;
    Enemy enemies[maxEnemies];

    bool isGameRunning = true;
    int playerX = 0, playerY = 0, dirX = 0, dirY = 0;
    float animationTimer = 0, animationDelay = 0.07;
    Clock frameClock;
    Clock gameOverClock;
    Clock TimePassed;
    bool gameOverClockStarted = false;

    bool safe_zone = false;

    int moves = 0;
    int tilesCollected = 0;
    int currentScore = 0;

    int prev_high_score = 0;

    int bonus = 0;

    int bonusApplier = 10;

    int scoreMultiplier = 2;

    int powerUps = 0;

    int lastPowerUpScore = 0;

    bool isPowerUpActive = false;
    Clock powerUpClock;

    setupGrid(); // Setup initial game grid

    while (window.isOpen())
    {
        float deltaTime = frameClock.getElapsedTime().asSeconds();
        frameClock.restart();
        animationTimer += deltaTime;

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed && event.key.code == Keyboard::P)
            {
                if (!isPowerUpActive && powerUps > 0)
                {
                    isPowerUpActive = true;
                    powerUps--;
                    powerUpClock.restart();
                }
            }
        }

        if (!isGameRunning)
        {
            if (!gameOverClockStarted)
            {
                gameOverClock.restart();
                gameOverClockStarted = true;
            }
            else if (gameOverClock.getElapsedTime().asSeconds() >= 2.5)
            {
                scores[scoreCount++] = currentScore;
                writeScores();
                cout << "score is written " << endl;
                GameOverScreen(window, TimePassed, currentScore, tilesCollected, ++moves, getHighestScore());
                return;
            }
        }

        // Handle player movement
        if (Keyboard::isKeyPressed(Keyboard::Left))
        {
            dirX = -1;
            dirY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Right))
        {
            dirX = 1;
            dirY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Up))
        {
            dirX = 0;
            dirY = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::Down))
        {
            dirX = 0;
            dirY = 1;
        }

        if (!isGameRunning)
            continue;

        // Move player every frame if enough time has passed
        if (animationTimer > animationDelay)
        {
            playerX += dirX;
            playerY += dirY;

            // Keep player inside grid bounds
            if (playerX < 0)
                playerX = 0;
            if (playerX > cols - 1)
                playerX = cols - 1;
            if (playerY < 0)
                playerY = 0;
            if (playerY > row - 1)
                playerY = row - 1;

            // Check for collision with self
            if (grid[playerY][playerX] == 2)
                isGameRunning = false;

            // Mark visited cell
            if (grid[playerY][playerX] == 0)
            {
                safe_zone = true;
                grid[playerY][playerX] = 2;
            }

            animationTimer = 0;
        }

        // Move all enemies
        if (!isPowerUpActive || powerUpClock.getElapsedTime().asSeconds() >= 3)
        {
            isPowerUpActive = false;

            for (int i = 0; i < maxEnemies; i++)
                enemies[i].move();
        }

        // If player hits a tile (special zone)
        if (grid[playerY][playerX] == 1)
        {
            dirX = dirY = 0;
            if (safe_zone)
            {
                moves += 1;
            }

            safe_zone = false;

            // Drop traps under enemy positions
            for (int i = 0; i < maxEnemies; i++)
                drop(enemies[i].y / tileSize, enemies[i].x / tileSize);

            // Reset grid

            int tileNow = 0;
            for (int i = 0; i < row; i++)
                for (int j = 0; j < cols; j++)
                    if (grid[i][j] == -1)
                        grid[i][j] = 0;
                    else if (grid[i][j] == 0 || grid[i][j] == 2)
                    {

                        tileNow += 1;

                        grid[i][j] = 1;
                    }
                    else
                        grid[i][j] = 1;

            tilesCollected += tileNow;

            if (tileNow > bonusApplier)
            {
                currentScore += (tileNow * scoreMultiplier);
                bonus += 1;
            }
            else
            {
                currentScore += tileNow;
            }

            if (bonus == 3)
            {
                bonusApplier = 5;
            }
            else if (bonus > 5)
            {
                scoreMultiplier = 4;
            }

            // First power-up (at 50)
            if (currentScore >= 50 && lastPowerUpScore < 50)
            {
                powerUps++;
                lastPowerUpScore = 50;
            }

            // Subsequent power-ups every 30 points after 70
            else if (currentScore >= 70 && currentScore - lastPowerUpScore >= 30)
            {
                powerUps++;
                lastPowerUpScore = currentScore;
            }
        }

        // End game if enemy touches player trail
        for (int i = 0; i < maxEnemies; i++)
            if (grid[enemies[i].y / tileSize][enemies[i].x / tileSize] == 2)
                isGameRunning = false;

        ///////// DRAW SECTION /////////
        // Check if power-up is active and applied
        if (isPowerUpActive)
        {
            // Change background color to white
            window.clear(Color(33, 33, 36));
        }
        else
        {
            // Default background color when power-up is not active
            window.clear(Color::Black);
        }

        // Rest of the game logic...

        // Draw grid tiles
        for (int i = 0; i < row; i++)
            for (int j = 0; j < cols; j++)
            {
                if (grid[i][j] == 0)
                    continue;

                if (grid[i][j] == 1)
                    tileSprite.setTextureRect(IntRect(0, 0, tileSize, tileSize));
                else if (grid[i][j] == 2)
                    tileSprite.setTextureRect(IntRect(54, 0, tileSize, tileSize));

                tileSprite.setPosition(j * tileSize, i * tileSize);
                window.draw(tileSprite);
            }

        // Draw player
        tileSprite.setTextureRect(IntRect(36, 0, tileSize, tileSize));
        tileSprite.setPosition(playerX * tileSize, playerY * tileSize);
        window.draw(tileSprite);

        // Draw rotating enemies
        enemySprite.rotate(10);
        for (int i = 0; i < maxEnemies; i++)
        {
            enemySprite.setPosition(enemies[i].x, enemies[i].y);
            window.draw(enemySprite);
        }

        // Show game over screen if not running
        if (!isGameRunning)
            window.draw(gameOverSprite);

        // Time display
        int timeElapsed = static_cast<int>(TimePassed.getElapsedTime().asSeconds());
        drawText(window, "Time: " + to_string(timeElapsed) + " sec", 0.5f, 0.5f, 25.0f, 470.0f, Color::Yellow);

        // Score display
        drawText(window, "Score: " + to_string(currentScore), 0.5f, 0.5f, 180.0f + 30.f, 470.0f, Color::Cyan);

        // Tiles collected display
        drawText(window, "Tiles: " + to_string(tilesCollected), 0.5f, 0.5f, 350.0f + 30.f, 470.0f, Color::Green);

        // Power-up display
        drawText(window, "Power-Ups: " + to_string(powerUps), 0.5f, 0.5f, 540.0f, 470.0f, Color::Magenta);

        if (isPowerUpActive)
        {
            // Display power-up message
            float remainingTime = 3.0f - powerUpClock.getElapsedTime().asSeconds();

            // Display the remaining time
            if (remainingTime > 0.0f)
            {
                drawText(window, "Power Activated! Time Left: " + to_string(static_cast<int>(remainingTime)) + "s",
                         0.5f, 0.5f, 50.0f, 50.0f, Color::Red);
            }
        }

        window.display();
    }
}

void l3(RenderWindow &window)
{
    Texture tileTexture, gameOverTexture, enemyTexture;
    loadTextures(tileTexture, gameOverTexture, enemyTexture);

    Sprite tileSprite(tileTexture), gameOverSprite(gameOverTexture), enemySprite(enemyTexture);
    gameOverSprite.setPosition(100, 100);
    enemySprite.setOrigin(20, 20);

    const int maxEnemies = 6;
    Enemy enemies[maxEnemies];

    bool isGameRunning = true;
    int playerX = 0, playerY = 0, dirX = 0, dirY = 0;
    float animationTimer = 0, animationDelay = 0.07;
    Clock frameClock;
    Clock gameOverClock;
    Clock TimePassed;
    bool gameOverClockStarted = false;

    bool safe_zone = false;

    int moves = 0;
    int tilesCollected = 0;
    int currentScore = 0;

    int prev_high_score = 0;

    int bonus = 0;

    int bonusApplier = 10;

    int scoreMultiplier = 2;

    int powerUps = 0;

    int lastPowerUpScore = 0;

    bool isPowerUpActive = false;
    Clock powerUpClock;

    setupGrid(); // Setup initial game grid

    while (window.isOpen())
    {
        float deltaTime = frameClock.getElapsedTime().asSeconds();
        frameClock.restart();
        animationTimer += deltaTime;

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed && event.key.code == Keyboard::P)
            {
                if (!isPowerUpActive && powerUps > 0)
                {
                    isPowerUpActive = true;
                    powerUps--;
                    powerUpClock.restart();
                }
            }
        }

        if (!isGameRunning)
        {
            if (!gameOverClockStarted)
            {
                gameOverClock.restart();
                gameOverClockStarted = true;
            }
            else if (gameOverClock.getElapsedTime().asSeconds() >= 2.5)
            {
                scores[scoreCount++] = currentScore;
                writeScores();
                cout << "Score" << endl;
                GameOverScreen(window, TimePassed, currentScore, tilesCollected, ++moves, getHighestScore());

                return;
            }
        }

        // Handle player movement
        if (Keyboard::isKeyPressed(Keyboard::Left))
        {
            dirX = -1;
            dirY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Right))
        {
            dirX = 1;
            dirY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Up))
        {
            dirX = 0;
            dirY = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::Down))
        {
            dirX = 0;
            dirY = 1;
        }

        if (!isGameRunning)
            continue;

        // Move player every frame if enough time has passed
        if (animationTimer > animationDelay)
        {
            playerX += dirX;
            playerY += dirY;

            // Keep player inside grid bounds
            if (playerX < 0)
                playerX = 0;
            if (playerX > cols - 1)
                playerX = cols - 1;
            if (playerY < 0)
                playerY = 0;
            if (playerY > row - 1)
                playerY = row - 1;

            // Check for collision with self
            if (grid[playerY][playerX] == 2)
                isGameRunning = false;

            // Mark visited cell
            if (grid[playerY][playerX] == 0)
            {
                safe_zone = true;
                grid[playerY][playerX] = 2;
            }

            animationTimer = 0;
        }

        // Move all enemies
        if (!isPowerUpActive || powerUpClock.getElapsedTime().asSeconds() >= 3)
        {
            isPowerUpActive = false;

            for (int i = 0; i < maxEnemies; i++)
                enemies[i].move();
        }

        // If player hits a tile (special zone)
        if (grid[playerY][playerX] == 1)
        {
            dirX = dirY = 0;
            if (safe_zone)
            {
                moves += 1;
            }

            safe_zone = false;

            // Drop traps under enemy positions
            for (int i = 0; i < maxEnemies; i++)
                drop(enemies[i].y / tileSize, enemies[i].x / tileSize);

            // Reset grid

            int tileNow = 0;
            for (int i = 0; i < row; i++)
                for (int j = 0; j < cols; j++)
                    if (grid[i][j] == -1)
                        grid[i][j] = 0;
                    else if (grid[i][j] == 0 || grid[i][j] == 2)
                    {

                        tileNow += 1;

                        grid[i][j] = 1;
                    }
                    else
                        grid[i][j] = 1;

            tilesCollected += tileNow;

            if (tileNow > bonusApplier)
            {
                currentScore += (tileNow * scoreMultiplier);
                bonus += 1;
            }
            else
            {
                currentScore += tileNow;
            }

            if (bonus == 3)
            {
                bonusApplier = 5;
            }
            else if (bonus > 5)
            {
                scoreMultiplier = 4;
            }

            // First power-up (at 50)
            if (currentScore >= 50 && lastPowerUpScore < 50)
            {
                powerUps++;
                lastPowerUpScore = 50;
            }

            // Subsequent power-ups every 30 points after 70
            else if (currentScore >= 70 && currentScore - lastPowerUpScore >= 30)
            {
                powerUps++;
                lastPowerUpScore = currentScore;
            }
        }

        // End game if enemy touches player trail
        for (int i = 0; i < maxEnemies; i++)
            if (grid[enemies[i].y / tileSize][enemies[i].x / tileSize] == 2)
                isGameRunning = false;

        ///////// DRAW SECTION /////////
        // Check if power-up is active and applied
        if (isPowerUpActive)
        {
            // Change background color to white
            window.clear(Color(33, 33, 36));
        }
        else
        {
            // Default background color when power-up is not active
            window.clear(Color::Black);
        }

        // Rest of the game logic...

        // Draw grid tiles
        for (int i = 0; i < row; i++)
            for (int j = 0; j < cols; j++)
            {
                if (grid[i][j] == 0)
                    continue;

                if (grid[i][j] == 1)
                    tileSprite.setTextureRect(IntRect(0, 0, tileSize, tileSize));
                else if (grid[i][j] == 2)
                    tileSprite.setTextureRect(IntRect(54, 0, tileSize, tileSize));

                tileSprite.setPosition(j * tileSize, i * tileSize);
                window.draw(tileSprite);
            }

        // Draw player
        tileSprite.setTextureRect(IntRect(36, 0, tileSize, tileSize));
        tileSprite.setPosition(playerX * tileSize, playerY * tileSize);
        window.draw(tileSprite);

        // Draw rotating enemies
        enemySprite.rotate(10);
        for (int i = 0; i < maxEnemies; i++)
        {
            enemySprite.setPosition(enemies[i].x, enemies[i].y);
            window.draw(enemySprite);
        }

        // Show game over screen if not running
        if (!isGameRunning)
            window.draw(gameOverSprite);

        // Time display
        int timeElapsed = static_cast<int>(TimePassed.getElapsedTime().asSeconds());
        drawText(window, "Time: " + to_string(timeElapsed) + " sec", 0.5f, 0.5f, 25.0f, 470.0f, Color::Yellow);

        // Score display
        drawText(window, "Score: " + to_string(currentScore), 0.5f, 0.5f, 180.0f + 30.f, 470.0f, Color::Cyan);

        // Tiles collected display
        drawText(window, "Tiles: " + to_string(tilesCollected), 0.5f, 0.5f, 350.0f + 30.f, 470.0f, Color::Green);

        // Power-up display
        drawText(window, "Power-Ups: " + to_string(powerUps), 0.5f, 0.5f, 540.0f, 470.0f, Color::Magenta);

        if (isPowerUpActive)
        {
            // Display power-up message
            float remainingTime = 3.0f - powerUpClock.getElapsedTime().asSeconds();

            // Display the remaining time
            if (remainingTime > 0.0f)
            {
                drawText(window, "Power Activated! Time Left: " + to_string(static_cast<int>(remainingTime)) + "s",
                         0.5f, 0.5f, 50.0f, 50.0f, Color::Red);
            }
        }

        window.display();
    }
}

void cont(RenderWindow &window)
{
    Texture tileTexture, gameOverTexture, enemyTexture;
    loadTextures(tileTexture, gameOverTexture, enemyTexture);

    Sprite tileSprite(tileTexture), gameOverSprite(gameOverTexture), enemySprite(enemyTexture);
    gameOverSprite.setPosition(100, 100);
    enemySprite.setOrigin(20, 20);

    Enemy enemies[1000];       // large array for many enemies
    int currentEnemyCount = 2; // start with 2 enemies

    float newEnemyTimeLimit = 10;

    bool isGameRunning = true;
    int playerX = 0, playerY = 0, dirX = 0, dirY = 0;
    float animationTimer = 0, animationDelay = 0.07;
    float enemySpawnTimer = 0; // to keep track of 20-second intervals

    Clock frameClock;
    Clock gameOverClock;
    Clock TimePassed;

    bool gameOverClockStarted = false;

    bool safe_zone = false;

    int moves = 0;
    int tilesCollected = 0;
    int currentScore = 0;

    int prev_high_score = 0;

    int bonus = 0;

    int bonusApplier = 10;

    int scoreMultiplier = 2;

    int powerUps = 0;

    int lastPowerUpScore = 0;

    bool isPowerUpActive = false;
    Clock powerUpClock;

    setupGrid(); // set up grid structure

    while (window.isOpen())
    {
        float deltaTime = frameClock.getElapsedTime().asSeconds();
        frameClock.restart();
        animationTimer += deltaTime;
        enemySpawnTimer += deltaTime;

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed && event.key.code == Keyboard::P)
            {
                if (!isPowerUpActive && powerUps > 0)
                {
                    isPowerUpActive = true;
                    powerUps--;
                    powerUpClock.restart();
                }
            }
        }

        if (!isGameRunning)
        {
            if (!gameOverClockStarted)
            {
                gameOverClock.restart();
                gameOverClockStarted = true;
            }
            else if (gameOverClock.getElapsedTime().asSeconds() >= 2.5)
            {
                scores[scoreCount++] = currentScore;
                writeScores();
                cout << "score" << endl;
                GameOverScreen(window, TimePassed, currentScore, tilesCollected, ++moves, getHighestScore());

                return;
            }
        }

        // movement input
        if (Keyboard::isKeyPressed(Keyboard::Left))
        {
            dirX = -1;
            dirY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Right))
        {
            dirX = 1;
            dirY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Up))
        {
            dirX = 0;
            dirY = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::Down))
        {
            dirX = 0;
            dirY = 1;
        }

        if (!isGameRunning)
            continue;

        // add 2 enemies after time
        if (enemySpawnTimer >= newEnemyTimeLimit && currentEnemyCount + 2 <= 1000)
        {
            // Copy coordinates of the last enemy to the new enemy
            for (int i = 0; i < 2; i++)
            {
                // Only add a new enemy if we already have some enemies spawned
                if (currentEnemyCount > 0)
                {
                    // Copy position of the last enemy
                    enemies[currentEnemyCount + i].x = enemies[currentEnemyCount - 1].x;
                    enemies[currentEnemyCount + i].y = enemies[currentEnemyCount - 1].y;
                }
                else
                {
                    // If no enemies yet, spawn at a default location (example: 0,0)
                    enemies[currentEnemyCount + i].x = 0;
                    enemies[currentEnemyCount + i].y = 0;
                }
            }

            currentEnemyCount += 2;
            enemySpawnTimer = 0;
        }

        // player movement update
        if (animationTimer > animationDelay)
        {
            playerX += dirX;
            playerY += dirY;

            // bounds check
            if (playerX < 0)
                playerX = 0;
            if (playerX > cols - 1)
                playerX = cols - 1;
            if (playerY < 0)
                playerY = 0;
            if (playerY > row - 1)
                playerY = row - 1;

            // collision with trail
            if (grid[playerY][playerX] == 2)
                isGameRunning = false;

            // Mark visited cell
            if (grid[playerY][playerX] == 0)
            {
                safe_zone = true;
                grid[playerY][playerX] = 2;
            }

            animationTimer = 0;
        }

        // Move all enemies
        if (!isPowerUpActive || powerUpClock.getElapsedTime().asSeconds() >= 3)
        {
            isPowerUpActive = false;

            for (int i = 0; i < currentEnemyCount; i++)
                enemies[i].move();
        }

        // drop enemies if player on special tile
        if (grid[playerY][playerX] == 1)
        {
            dirX = dirY = 0;

            if (safe_zone)
            {
                moves += 1;
            }

            safe_zone = false;

            for (int i = 0; i < currentEnemyCount; i++)
                drop(enemies[i].y / tileSize, enemies[i].x / tileSize);

            // Reset grid

            int tileNow = 0;
            for (int i = 0; i < row; i++)
                for (int j = 0; j < cols; j++)
                    if (grid[i][j] == -1)
                        grid[i][j] = 0;
                    else if (grid[i][j] == 0 || grid[i][j] == 2)
                    {

                        tileNow += 1;

                        grid[i][j] = 1;
                    }
                    else
                        grid[i][j] = 1;

            tilesCollected += tileNow;

            if (tileNow > bonusApplier)
            {
                currentScore += (tileNow * scoreMultiplier);
                bonus += 1;
            }
            else
            {
                currentScore += tileNow;
            }

            if (bonus == 3)
            {
                bonusApplier = 5;
            }
            else if (bonus > 5)
            {
                scoreMultiplier = 4;
            }

            // First power-up (at 50)
            if (currentScore >= 50 && lastPowerUpScore < 50)
            {
                powerUps++;
                lastPowerUpScore = 50;
            }

            // Subsequent power-ups every 30 points after 70
            else if (currentScore >= 70 && currentScore - lastPowerUpScore >= 30)
            {
                powerUps++;
                lastPowerUpScore = currentScore;
            }
        }

        // collision check: player with enemy
        for (int i = 0; i < currentEnemyCount; i++)
            if (grid[enemies[i].y / tileSize][enemies[i].x / tileSize] == 2)
                isGameRunning = false;

        ///////// DRAW SECTION /////////
        // Check if power-up is active and applied
        if (isPowerUpActive)
        {
            // Change background color to white
            window.clear(Color(33, 33, 36));
        }
        else
        {
            // Default background color when power-up is not active
            window.clear(Color::Black);
        }

        // draw grid
        for (int i = 0; i < row; i++)
            for (int j = 0; j < cols; j++)
            {
                if (grid[i][j] == 0)
                    continue;

                if (grid[i][j] == 1)
                    tileSprite.setTextureRect(IntRect(0, 0, tileSize, tileSize));
                else if (grid[i][j] == 2)
                    tileSprite.setTextureRect(IntRect(54, 0, tileSize, tileSize));

                tileSprite.setPosition(j * tileSize, i * tileSize);
                window.draw(tileSprite);
            }

        // draw player
        tileSprite.setTextureRect(IntRect(36, 0, tileSize, tileSize));
        tileSprite.setPosition(playerX * tileSize, playerY * tileSize);
        window.draw(tileSprite);

        // draw enemies
        enemySprite.rotate(10);
        for (int i = 0; i < currentEnemyCount; i++)
        {
            enemySprite.setPosition(enemies[i].x, enemies[i].y);
            window.draw(enemySprite);
        }

        // draw game over if game has ended
        if (!isGameRunning)
            window.draw(gameOverSprite);

        // Time display
        int timeElapsed = static_cast<int>(TimePassed.getElapsedTime().asSeconds());
        drawText(window, "Time: " + to_string(timeElapsed) + " sec", 0.5f, 0.5f, 25.0f, 470.0f, Color::Yellow);

        // Score display
        drawText(window, "Score: " + to_string(currentScore), 0.5f, 0.5f, 180.0f + 30.f, 470.0f, Color::Cyan);

        // Tiles collected display
        drawText(window, "Tiles: " + to_string(tilesCollected), 0.5f, 0.5f, 350.0f + 30.f, 470.0f, Color::Green);

        // Power-up display
        drawText(window, "Power-Ups: " + to_string(powerUps), 0.5f, 0.5f, 540.0f, 470.0f, Color::Magenta);

        if (isPowerUpActive)
        {
            // Display power-up message
            float remainingTime = 3.0f - powerUpClock.getElapsedTime().asSeconds();

            // Display the remaining time
            if (remainingTime > 0.0f)
            {
                drawText(window, "Power Activated! Time Left: " + to_string(static_cast<int>(remainingTime)) + "s",
                         0.5f, 0.5f, 50.0f, 50.0f, Color::Red);
            }
        }

        window.display();
    }
}

void l1_2(RenderWindow &window)
{
    Texture tileTexture, gameOverTexture, enemyTexture;
    loadTextures(tileTexture, gameOverTexture, enemyTexture);

    Sprite tileSprite(tileTexture), gameOverSprite(gameOverTexture), enemySprite(enemyTexture);
    gameOverSprite.setPosition(100, 100);
    enemySprite.setOrigin(20, 20);

    const int maxEnemies = 2;
    Enemy enemies[maxEnemies];

    bool isGameRunning = true;
    int p1X = 0, p1Y = 0, p1DX = 0, p1DY = 0;
    int p2X = cols - 1, p2Y = row - 1, p2DX = 0, p2DY = 0;
    float animationTimer = 0, animationDelay = 0.07;

    Clock frameClock;
    Clock gameOverClock;
    Clock TimePassed;
    bool gameOverClockStarted = false;

    bool safe_zone = false;

    int tiles_player_1 = 0;
    int tiles_player_2 = 0;

    Clock powerUpClock;

    setupGrid(); // Setup initial game grid

    int prex1 = 0, prey1 = 0;
    int prex2 = 0, prey2 = 0;

    while (window.isOpen())
    {
        if (p1X == p2X && p1Y == p2Y)
        {
            isGameRunning = false;
        }
        float deltaTime = frameClock.getElapsedTime().asSeconds();
        frameClock.restart();
        animationTimer += deltaTime;

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }

        if (!isGameRunning)
        {
            if (!gameOverClockStarted)
            {
                gameOverClock.restart();
                gameOverClockStarted = true;
            }
            else if (gameOverClock.getElapsedTime().asSeconds() >= 2.5)
            {
                GameOverScreen(window, TimePassed, tiles_player_1, tiles_player_2);
                return;
            }
        }

        // Handle Player 1 movement (WASD)
        if (Keyboard::isKeyPressed(Keyboard::A)) // Left
        {
            p1DX = -1;
            p1DY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::D)) // Right
        {
            p1DX = 1;
            p1DY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::W)) // Up
        {
            p1DX = 0;
            p1DY = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::S)) // Down
        {
            p1DX = 0;
            p1DY = 1;
        }

        // Handle Player 2 movement (Arrow Keys)
        if (Keyboard::isKeyPressed(Keyboard::Left)) // Left
        {
            p2DX = -1;
            p2DY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Right)) // Right
        {
            p2DX = 1;
            p2DY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Up)) // Up
        {
            p2DX = 0;
            p2DY = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::Down)) // Down
        {
            p2DX = 0;
            p2DY = 1;
        }

        if (!isGameRunning)
            continue;

        // Move players every frame if enough time has passed
        if (animationTimer > animationDelay)
        {

            prex1 = p1X;
            prey1 = p1Y;

            prex2 = p2X;
            prey2 = p2Y;

            // Move Player 1
            p1X += p1DX;
            p1Y += p1DY;

            // Move Player 2
            p2X += p2DX;
            p2Y += p2DY;

            // Keep Player 1 inside grid bounds
            if (p1X < 0)
                p1X = 0;
            if (p1X > cols - 1)
                p1X = cols - 1;
            if (p1Y < 0)
                p1Y = 0;
            if (p1Y > row - 1)
                p1Y = row - 1;

            // Keep Player 2 inside grid bounds
            if (p2X < 0)
                p2X = 0;
            if (p2X > cols - 1)
                p2X = cols - 1;
            if (p2Y < 0)
                p2Y = 0;
            if (p2Y > row - 1)
                p2Y = row - 1;

            // Check for collision with self for Player 1
            if (grid[p1Y][p1X] == 2)
                isGameRunning = false;

            // Check for collision with self for Player 2
            if (grid[p2Y][p2X] == 3)
                isGameRunning = false;

            // Mark visited cells for Player 1
            if (grid[p1Y][p1X] == 0)
            {
                safe_zone = true;
                grid[p1Y][p1X] = 2;
            }

            // Mark visited cells for Player 2
            if (grid[p2Y][p2X] == 0)
            {

                grid[p2Y][p2X] = 3;
            }

            animationTimer = 0;
        }

        // Move all enemies
        for (int i = 0; i < maxEnemies; i++)
            enemies[i].move();
        // If player hits a tile (special zone)
        if (grid[p1Y][p1X] == 1)
        {

            p1DX = p1DY = 0;
            // Drop traps under enemy positions
            if (grid[prey1][prex1] == 2)
            {
               
                for (int i = 0; i < maxEnemies; i++)
                    drop(enemies[i].y / tileSize, enemies[i].x / tileSize);

                // Now, we'll update the grid according to some game logic if needed (e.g., new tiles)
                int tileNow = 0;
                for (int i = 0; i < row; i++)
                {
                    for (int j = 0; j < cols; j++)
                    {
                        // Handle tiles for player-specific logic
                        if (grid[i][j] == -1)
                            grid[i][j] = 0;
                        else if (grid[i][j] == 2 || grid[i][j] == 0)
                        {
                            // Debugging for player 1 and 2

                            tiles_player_1 += 1;
                            grid[i][j] = 1;
                        }
                    }
                }
            }
        }

        if (grid[p2Y][p2X] == 1)
        {

            p2DX = p2DY = 0;

            if (grid[prey2][prex2] == 3)
            {
                // Drop traps under enemy positions
                for (int i = 0; i < maxEnemies; i++)
                    drop(enemies[i].y / tileSize, enemies[i].x / tileSize);

                // Now, we'll update the grid according to some game logic if needed (e.g., new tiles)
                int tileNow = 0;
                for (int i = 0; i < row; i++)
                {
                    for (int j = 0; j < cols; j++)
                    {
                        // Handle tiles for player-specific logic
                        if (grid[i][j] == -1)
                            grid[i][j] = 0;
                        else if (grid[i][j] == 3 || grid[i][j] == 0)
                        {

                            tiles_player_2 += 1;

                            tileNow += 1;
                            grid[i][j] = 1;
                        }
                    }
                }
            }
        }

        // End game if enemy touches player trail
        for (int i = 0; i < maxEnemies; i++)
            if (grid[enemies[i].y / tileSize][enemies[i].x / tileSize] == 2 || grid[enemies[i].y / tileSize][enemies[i].x / tileSize] == 3)
                isGameRunning = false;

        ///////// DRAW SECTION /////////

        window.clear(Color::Black);

        // Draw grid tiles
        for (int i = 0; i < row; i++)
            for (int j = 0; j < cols; j++)
            {
                if (grid[i][j] == 0)
                    continue;

                if (grid[i][j] == 1)
                    tileSprite.setTextureRect(IntRect(0, 0, tileSize, tileSize));
                else if (grid[i][j] == 2)
                    tileSprite.setTextureRect(IntRect(54, 0, tileSize, tileSize));
                else if (grid[i][j] == 3)
                    tileSprite.setTextureRect(IntRect(108, 0, tileSize, tileSize));

                tileSprite.setPosition(j * tileSize, i * tileSize);
                window.draw(tileSprite);
            }

        // Draw Player 1
        tileSprite.setTextureRect(IntRect(36, 0, tileSize, tileSize));
        tileSprite.setPosition(p1X * tileSize, p1Y * tileSize);
        window.draw(tileSprite);

        // Draw Player 2
        tileSprite.setTextureRect(IntRect(72, 0, tileSize, tileSize));
        tileSprite.setPosition(p2X * tileSize, p2Y * tileSize);
        window.draw(tileSprite);

        // Draw rotating enemies
        enemySprite.rotate(10);
        for (int i = 0; i < maxEnemies; i++)
        {
            enemySprite.setPosition(enemies[i].x, enemies[i].y);
            window.draw(enemySprite);
        }

        // Show game over screen if not running
        if (!isGameRunning)
            window.draw(gameOverSprite);

        drawText(window, "Player 1 Tiles : " + to_string(tiles_player_1), 0.5f, 0.5f, 60.0f, 470.0f, Color::Yellow);
        drawText(window, "Player 2 Tiles : " + to_string(tiles_player_2), 0.5f, 0.5f, 400.0f + 30.f, 470.0f, Color::Green);

        window.display();
    }
}

void l2_2(RenderWindow &window)
{
    Texture tileTexture, gameOverTexture, enemyTexture;
    loadTextures(tileTexture, gameOverTexture, enemyTexture);

    Sprite tileSprite(tileTexture), gameOverSprite(gameOverTexture), enemySprite(enemyTexture);
    gameOverSprite.setPosition(100, 100);
    enemySprite.setOrigin(20, 20);

    const int maxEnemies = 4;
    Enemy enemies[maxEnemies];

    bool isGameRunning = true;
    int p1X = 0, p1Y = 0, p1DX = 0, p1DY = 0;
    int p2X = cols - 1, p2Y = row - 1, p2DX = 0, p2DY = 0;
    float animationTimer = 0, animationDelay = 0.07;

    Clock frameClock;
    Clock gameOverClock;
    Clock TimePassed;
    bool gameOverClockStarted = false;

    bool safe_zone = false;

    int tiles_player_1 = 0;
    int tiles_player_2 = 0;

    Clock powerUpClock;

    setupGrid(); // Setup initial game grid

    int prex1 = 0, prey1 = 0;
    int prex2 = 0, prey2 = 0;

    while (window.isOpen())
    {
        if (p1X == p2X && p1Y == p2Y)
        {
            isGameRunning = false;
        }
        float deltaTime = frameClock.getElapsedTime().asSeconds();
        frameClock.restart();
        animationTimer += deltaTime;

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }

        if (!isGameRunning)
        {
            if (!gameOverClockStarted)
            {
                gameOverClock.restart();
                gameOverClockStarted = true;
            }
            else if (gameOverClock.getElapsedTime().asSeconds() >= 2.5)
            {
                GameOverScreen(window, TimePassed, tiles_player_1, tiles_player_2);
                return;
            }
        }

        // Handle Player 1 movement (WASD)
        if (Keyboard::isKeyPressed(Keyboard::A)) // Left
        {
            p1DX = -1;
            p1DY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::D)) // Right
        {
            p1DX = 1;
            p1DY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::W)) // Up
        {
            p1DX = 0;
            p1DY = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::S)) // Down
        {
            p1DX = 0;
            p1DY = 1;
        }

        // Handle Player 2 movement (Arrow Keys)
        if (Keyboard::isKeyPressed(Keyboard::Left)) // Left
        {
            p2DX = -1;
            p2DY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Right)) // Right
        {
            p2DX = 1;
            p2DY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Up)) // Up
        {
            p2DX = 0;
            p2DY = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::Down)) // Down
        {
            p2DX = 0;
            p2DY = 1;
        }

        if (!isGameRunning)
            continue;

        // Move players every frame if enough time has passed
        if (animationTimer > animationDelay)
        {

            prex1 = p1X;
            prey1 = p1Y;

            prex2 = p2X;
            prey2 = p2Y;

            // Move Player 1
            p1X += p1DX;
            p1Y += p1DY;

            // Move Player 2
            p2X += p2DX;
            p2Y += p2DY;

            // Keep Player 1 inside grid bounds
            if (p1X < 0)
                p1X = 0;
            if (p1X > cols - 1)
                p1X = cols - 1;
            if (p1Y < 0)
                p1Y = 0;
            if (p1Y > row - 1)
                p1Y = row - 1;

            // Keep Player 2 inside grid bounds
            if (p2X < 0)
                p2X = 0;
            if (p2X > cols - 1)
                p2X = cols - 1;
            if (p2Y < 0)
                p2Y = 0;
            if (p2Y > row - 1)
                p2Y = row - 1;

            // Check for collision with self for Player 1
            if (grid[p1Y][p1X] == 2)
                isGameRunning = false;

            // Check for collision with self for Player 2
            if (grid[p2Y][p2X] == 3)
                isGameRunning = false;

            // Mark visited cells for Player 1
            if (grid[p1Y][p1X] == 0)
            {
                safe_zone = true;
                grid[p1Y][p1X] = 2;
            }

            // Mark visited cells for Player 2
            if (grid[p2Y][p2X] == 0)
            {

                grid[p2Y][p2X] = 3;
            }

            animationTimer = 0;
        }

        // Move all enemies
        for (int i = 0; i < maxEnemies; i++)
            enemies[i].move();
        // If player hits a tile (special zone)
        // If player hits a tile (special zone)
        if (grid[p1Y][p1X] == 1)
        {

            p1DX = p1DY = 0;
            // Drop traps under enemy positions
            if (grid[prey1][prex1] == 2)
            {
                for (int i = 0; i < maxEnemies; i++)
                    drop(enemies[i].y / tileSize, enemies[i].x / tileSize);

                // Now, we'll update the grid according to some game logic if needed (e.g., new tiles)
                int tileNow = 0;
                for (int i = 0; i < row; i++)
                {
                    for (int j = 0; j < cols; j++)
                    {
                        // Handle tiles for player-specific logic
                        if (grid[i][j] == -1)
                            grid[i][j] = 0;
                        else if (grid[i][j] == 2 || grid[i][j] == 0)
                        {
                            // Debugging for player 1 and 2

                            tiles_player_1 += 1;
                            grid[i][j] = 1;
                        }
                    }
                }
            }
        }

        if (grid[p2Y][p2X] == 1)
        {

            p2DX = p2DY = 0;

            if (grid[prey2][prex2] == 3)
            {
                // Drop traps under enemy positions
                for (int i = 0; i < maxEnemies; i++)
                    drop(enemies[i].y / tileSize, enemies[i].x / tileSize);

                // Now, we'll update the grid according to some game logic if needed (e.g., new tiles)
                int tileNow = 0;
                for (int i = 0; i < row; i++)
                {
                    for (int j = 0; j < cols; j++)
                    {
                        // Handle tiles for player-specific logic
                        if (grid[i][j] == -1)
                            grid[i][j] = 0;
                        else if (grid[i][j] == 3 || grid[i][j] == 0)
                        {

                            tiles_player_2 += 1;

                            tileNow += 1;
                            grid[i][j] = 1;
                        }
                    }
                }
            }
        }

        // End game if enemy touches player trail
        for (int i = 0; i < maxEnemies; i++)
            if (grid[enemies[i].y / tileSize][enemies[i].x / tileSize] == 2 || grid[enemies[i].y / tileSize][enemies[i].x / tileSize] == 3)
                isGameRunning = false;

        ///////// DRAW SECTION /////////

        window.clear(Color::Black);

        // Draw grid tiles
        for (int i = 0; i < row; i++)
            for (int j = 0; j < cols; j++)
            {
                if (grid[i][j] == 0)
                    continue;

                if (grid[i][j] == 1)
                    tileSprite.setTextureRect(IntRect(0, 0, tileSize, tileSize));
                else if (grid[i][j] == 2)
                    tileSprite.setTextureRect(IntRect(54, 0, tileSize, tileSize));
                else if (grid[i][j] == 3)
                    tileSprite.setTextureRect(IntRect(108, 0, tileSize, tileSize));

                tileSprite.setPosition(j * tileSize, i * tileSize);
                window.draw(tileSprite);
            }

        // Draw Player 1
        tileSprite.setTextureRect(IntRect(36, 0, tileSize, tileSize));
        tileSprite.setPosition(p1X * tileSize, p1Y * tileSize);
        window.draw(tileSprite);

        // Draw Player 2
        tileSprite.setTextureRect(IntRect(72, 0, tileSize, tileSize));
        tileSprite.setPosition(p2X * tileSize, p2Y * tileSize);
        window.draw(tileSprite);

        // Draw rotating enemies
        enemySprite.rotate(10);
        for (int i = 0; i < maxEnemies; i++)
        {
            enemySprite.setPosition(enemies[i].x, enemies[i].y);
            window.draw(enemySprite);
        }

        // Show game over screen if not running
        if (!isGameRunning)
            window.draw(gameOverSprite);

        drawText(window, "Player 1 Tiles : " + to_string(tiles_player_1), 0.5f, 0.5f, 60.0f, 470.0f, Color::Yellow);
        drawText(window, "Player 2 Tiles : " + to_string(tiles_player_2), 0.5f, 0.5f, 400.0f + 30.f, 470.0f, Color::Green);

        window.display();
    }
}

void l3_2(RenderWindow &window)
{
    Texture tileTexture, gameOverTexture, enemyTexture;
    loadTextures(tileTexture, gameOverTexture, enemyTexture);

    Sprite tileSprite(tileTexture), gameOverSprite(gameOverTexture), enemySprite(enemyTexture);
    gameOverSprite.setPosition(100, 100);
    enemySprite.setOrigin(20, 20);

    const int maxEnemies = 6;
    Enemy enemies[maxEnemies];

    bool isGameRunning = true;
    int p1X = 0, p1Y = 0, p1DX = 0, p1DY = 0;
    int p2X = cols - 1, p2Y = row - 1, p2DX = 0, p2DY = 0;
    float animationTimer = 0, animationDelay = 0.07;

    Clock frameClock;
    Clock gameOverClock;
    Clock TimePassed;
    bool gameOverClockStarted = false;

    bool safe_zone = false;

    int tiles_player_1 = 0;
    int tiles_player_2 = 0;

    Clock powerUpClock;

    setupGrid(); // Setup initial game grid

    int prex1 = 0, prey1 = 0;
    int prex2 = 0, prey2 = 0;

    while (window.isOpen())
    {
        if (p1X == p2X && p1Y == p2Y)
        {
            isGameRunning = false;
        }
        float deltaTime = frameClock.getElapsedTime().asSeconds();
        frameClock.restart();
        animationTimer += deltaTime;

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }

        if (!isGameRunning)
        {
            if (!gameOverClockStarted)
            {
                gameOverClock.restart();
                gameOverClockStarted = true;
            }
            else if (gameOverClock.getElapsedTime().asSeconds() >= 2.5)
            {
                GameOverScreen(window, TimePassed, tiles_player_1, tiles_player_2);
                return;
            }
        }

        // Handle Player 1 movement (WASD)
        if (Keyboard::isKeyPressed(Keyboard::A)) // Left
        {
            p1DX = -1;
            p1DY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::D)) // Right
        {
            p1DX = 1;
            p1DY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::W)) // Up
        {
            p1DX = 0;
            p1DY = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::S)) // Down
        {
            p1DX = 0;
            p1DY = 1;
        }

        // Handle Player 2 movement (Arrow Keys)
        if (Keyboard::isKeyPressed(Keyboard::Left)) // Left
        {
            p2DX = -1;
            p2DY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Right)) // Right
        {
            p2DX = 1;
            p2DY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Up)) // Up
        {
            p2DX = 0;
            p2DY = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::Down)) // Down
        {
            p2DX = 0;
            p2DY = 1;
        }

        if (!isGameRunning)
            continue;

        // Move players every frame if enough time has passed
        if (animationTimer > animationDelay)
        {
            prex1 = p1X;
            prey1 = p1Y;

            prex2 = p2X;
            prey2 = p2Y;

            // Move Player 1
            p1X += p1DX;
            p1Y += p1DY;

            // Move Player 2
            p2X += p2DX;
            p2Y += p2DY;

            // Keep Player 1 inside grid bounds
            if (p1X < 0)
                p1X = 0;
            if (p1X > cols - 1)
                p1X = cols - 1;
            if (p1Y < 0)
                p1Y = 0;
            if (p1Y > row - 1)
                p1Y = row - 1;

            // Keep Player 2 inside grid bounds
            if (p2X < 0)
                p2X = 0;
            if (p2X > cols - 1)
                p2X = cols - 1;
            if (p2Y < 0)
                p2Y = 0;
            if (p2Y > row - 1)
                p2Y = row - 1;

            // Check for collision with self for Player 1
            if (grid[p1Y][p1X] == 2)
                isGameRunning = false;

            // Check for collision with self for Player 2
            if (grid[p2Y][p2X] == 3)
                isGameRunning = false;

            // Mark visited cells for Player 1
            if (grid[p1Y][p1X] == 0)
            {
                safe_zone = true;
                grid[p1Y][p1X] = 2;
            }

            // Mark visited cells for Player 2
            if (grid[p2Y][p2X] == 0)
            {

                grid[p2Y][p2X] = 3;
            }

            animationTimer = 0;
        }

        // Move all enemies
        for (int i = 0; i < maxEnemies; i++)
            enemies[i].move();
        // If player hits a tile (special zone)
        // If player hits a tile (special zone)
        if (grid[p1Y][p1X] == 1)
        {

            p1DX = p1DY = 0;
            // Drop traps under enemy positions
            if (grid[prey1][prex1] == 2)
            {
                for (int i = 0; i < maxEnemies; i++)
                    drop(enemies[i].y / tileSize, enemies[i].x / tileSize);

                // Now, we'll update the grid according to some game logic if needed (e.g., new tiles)
                int tileNow = 0;
                for (int i = 0; i < row; i++)
                {
                    for (int j = 0; j < cols; j++)
                    {
                        // Handle tiles for player-specific logic
                        if (grid[i][j] == -1)
                            grid[i][j] = 0;
                        else if (grid[i][j] == 2 || grid[i][j] == 0)
                        {
                            // Debugging for player 1 and 2

                            tiles_player_1 += 1;
                            grid[i][j] = 1;
                        }
                    }
                }
            }
        }

        if (grid[p2Y][p2X] == 1)
        {

            p2DX = p2DY = 0;

            if (grid[prey2][prex2] == 3)
            {
                // Drop traps under enemy positions
                for (int i = 0; i < maxEnemies; i++)
                    drop(enemies[i].y / tileSize, enemies[i].x / tileSize);

                // Now, we'll update the grid according to some game logic if needed (e.g., new tiles)
                int tileNow = 0;
                for (int i = 0; i < row; i++)
                {
                    for (int j = 0; j < cols; j++)
                    {
                        // Handle tiles for player-specific logic
                        if (grid[i][j] == -1)
                            grid[i][j] = 0;
                        else if (grid[i][j] == 3 || grid[i][j] == 0)
                        {

                            tiles_player_2 += 1;

                            tileNow += 1;
                            grid[i][j] = 1;
                        }
                    }
                }
            }
        }
        // End game if enemy touches player trail
        for (int i = 0; i < maxEnemies; i++)
            if (grid[enemies[i].y / tileSize][enemies[i].x / tileSize] == 2 || grid[enemies[i].y / tileSize][enemies[i].x / tileSize] == 3)
                isGameRunning = false;

        ///////// DRAW SECTION /////////

        window.clear(Color::Black);

        // Draw grid tiles
        for (int i = 0; i < row; i++)
            for (int j = 0; j < cols; j++)
            {
                if (grid[i][j] == 0)
                    continue;

                if (grid[i][j] == 1)
                    tileSprite.setTextureRect(IntRect(0, 0, tileSize, tileSize));
                else if (grid[i][j] == 2)
                    tileSprite.setTextureRect(IntRect(54, 0, tileSize, tileSize));
                else if (grid[i][j] == 3)
                    tileSprite.setTextureRect(IntRect(108, 0, tileSize, tileSize));

                tileSprite.setPosition(j * tileSize, i * tileSize);
                window.draw(tileSprite);
            }

        // Draw Player 1
        tileSprite.setTextureRect(IntRect(36, 0, tileSize, tileSize));
        tileSprite.setPosition(p1X * tileSize, p1Y * tileSize);
        window.draw(tileSprite);

        // Draw Player 2
        tileSprite.setTextureRect(IntRect(72, 0, tileSize, tileSize));
        tileSprite.setPosition(p2X * tileSize, p2Y * tileSize);
        window.draw(tileSprite);

        // Draw rotating enemies
        enemySprite.rotate(10);
        for (int i = 0; i < maxEnemies; i++)
        {
            enemySprite.setPosition(enemies[i].x, enemies[i].y);
            window.draw(enemySprite);
        }

        // Show game over screen if not running
        if (!isGameRunning)
            window.draw(gameOverSprite);

        drawText(window, "Player 1 Tiles : " + to_string(tiles_player_1), 0.5f, 0.5f, 60.0f, 470.0f, Color::Yellow);
        drawText(window, "Player 2 Tiles : " + to_string(tiles_player_2), 0.5f, 0.5f, 400.0f + 30.f, 470.0f, Color::Green);

        window.display();
    }
}

void cont_2(RenderWindow &window)
{
    Texture tileTexture, gameOverTexture, enemyTexture;
    loadTextures(tileTexture, gameOverTexture, enemyTexture);

    Sprite tileSprite(tileTexture), gameOverSprite(gameOverTexture), enemySprite(enemyTexture);
    gameOverSprite.setPosition(100, 100);
    enemySprite.setOrigin(20, 20);

    Enemy enemies[1000];       // large array for many enemies
    int currentEnemyCount = 2; // start with 2 enemies

    float newEnemyTimeLimit = 10;

    bool isGameRunning = true;
    int p1X = 0, p1Y = 0, p1DX = 0, p1DY = 0;
    int p2X = cols - 1, p2Y = row - 1, p2DX = 0, p2DY = 0;
    float animationTimer = 0, animationDelay = 0.07;
    float enemySpawnTimer = 0; // to keep track of 20-second intervals

    Clock frameClock;
    Clock gameOverClock;
    Clock TimePassed;

    bool gameOverClockStarted = false;

    int tiles_player_1 = 0;
    int tiles_player_2 = 0;

    setupGrid(); // set up grid structure

    int prex1 = 0, prey1 = 0;
    int prex2 = 0, prey2 = 0;

    while (window.isOpen())
    {
        if (p1X == p2X && p1Y == p2Y)
        {
            isGameRunning = false;
        }
        float deltaTime = frameClock.getElapsedTime().asSeconds();
        frameClock.restart();
        animationTimer += deltaTime;
        enemySpawnTimer += deltaTime;

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }

        if (!isGameRunning)
        {
            if (!gameOverClockStarted)
            {
                gameOverClock.restart();
                gameOverClockStarted = true;
            }
            else if (gameOverClock.getElapsedTime().asSeconds() >= 2.5)
            {
                GameOverScreen(window, TimePassed, tiles_player_1, tiles_player_2);

                return;
            }
        }

        // Handle Player 1 movement (WASD)
        if (Keyboard::isKeyPressed(Keyboard::A)) // Left
        {
            p1DX = -1;
            p1DY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::D)) // Right
        {
            p1DX = 1;
            p1DY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::W)) // Up
        {
            p1DX = 0;
            p1DY = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::S)) // Down
        {
            p1DX = 0;
            p1DY = 1;
        }

        // Handle Player 2 movement (Arrow Keys)
        if (Keyboard::isKeyPressed(Keyboard::Left)) // Left
        {
            p2DX = -1;
            p2DY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Right)) // Right
        {
            p2DX = 1;
            p2DY = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Up)) // Up
        {
            p2DX = 0;
            p2DY = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::Down)) // Down
        {
            p2DX = 0;
            p2DY = 1;
        }

        if (!isGameRunning)
            continue;

        // add 2 enemies after time
        if (enemySpawnTimer >= newEnemyTimeLimit && currentEnemyCount + 2 <= 1000)
        {
            // Copy coordinates of the last enemy to the new enemy
            for (int i = 0; i < 2; i++)
            {
                // Only add a new enemy if we already have some enemies spawned
                if (currentEnemyCount > 0)
                {
                    // Copy position of the last enemy
                    enemies[currentEnemyCount + i].x = enemies[currentEnemyCount - 1].x;
                    enemies[currentEnemyCount + i].y = enemies[currentEnemyCount - 1].y;
                }
                else
                {
                    // If no enemies yet, spawn at a default location (example: 0,0)
                    enemies[currentEnemyCount + i].x = 0;
                    enemies[currentEnemyCount + i].y = 0;
                }
            }

            currentEnemyCount += 2;
            enemySpawnTimer = 0;
        }

        // player movement update
        // Move players every frame if enough time has passed
        if (animationTimer > animationDelay)
        {
            prex1 = p1X;
            prey1 = p1Y;

            prex2 = p2X;
            prey2 = p2Y;

            // Move Player 1
            p1X += p1DX;
            p1Y += p1DY;

            // Move Player 2
            p2X += p2DX;
            p2Y += p2DY;

            // Keep Player 1 inside grid bounds
            if (p1X < 0)
                p1X = 0;
            if (p1X > cols - 1)
                p1X = cols - 1;
            if (p1Y < 0)
                p1Y = 0;
            if (p1Y > row - 1)
                p1Y = row - 1;

            // Keep Player 2 inside grid bounds
            if (p2X < 0)
                p2X = 0;
            if (p2X > cols - 1)
                p2X = cols - 1;
            if (p2Y < 0)
                p2Y = 0;
            if (p2Y > row - 1)
                p2Y = row - 1;

            // Check for collision with self for Player 1
            if (grid[p1Y][p1X] == 2)
                isGameRunning = false;

            // Check for collision with self for Player 2
            if (grid[p2Y][p2X] == 3)
                isGameRunning = false;

            // Mark visited cells for Player 1
            if (grid[p1Y][p1X] == 0)
            {

                grid[p1Y][p1X] = 2;
            }

            // Mark visited cells for Player 2
            if (grid[p2Y][p2X] == 0)
            {

                grid[p2Y][p2X] = 3;
            }

            animationTimer = 0;
        }

        // Move all enemies

        for (int i = 0; i < currentEnemyCount; i++)
            enemies[i].move();

        if (grid[p1Y][p1X] == 1)
        {

            p1DX = p1DY = 0;
            // Drop traps under enemy positions
            if (grid[prey1][prex1] == 2)
            {
                for (int i = 0; i < currentEnemyCount; i++)
                    drop(enemies[i].y / tileSize, enemies[i].x / tileSize);

                // Now, we'll update the grid according to some game logic if needed (e.g., new tiles)
                int tileNow = 0;
                for (int i = 0; i < row; i++)
                {
                    for (int j = 0; j < cols; j++)
                    {
                        // Handle tiles for player-specific logic
                        if (grid[i][j] == -1)
                            grid[i][j] = 0;
                        else if (grid[i][j] == 2 || grid[i][j] == 0)
                        {
                            // Debugging for player 1 and 2

                            tiles_player_1 += 1;
                            grid[i][j] = 1;
                        }
                    }
                }
            }
        }

        if (grid[p2Y][p2X] == 1)
        {

            p2DX = p2DY = 0;

            if (grid[prey2][prex2] == 3)
            {
                // Drop traps under enemy positions
                for (int i = 0; i < currentEnemyCount; i++)
                    drop(enemies[i].y / tileSize, enemies[i].x / tileSize);

                // Now, we'll update the grid according to some game logic if needed (e.g., new tiles)
                int tileNow = 0;
                for (int i = 0; i < row; i++)
                {
                    for (int j = 0; j < cols; j++)
                    {
                        // Handle tiles for player-specific logic
                        if (grid[i][j] == -1)
                            grid[i][j] = 0;
                        else if (grid[i][j] == 3 || grid[i][j] == 0)
                        {

                            tiles_player_2 += 1;

                            tileNow += 1;
                            grid[i][j] = 1;
                        }
                    }
                }
            }
        }

        // collision check: player with enemy
        // End game if enemy touches player trail
        for (int i = 0; i < currentEnemyCount; i++)
            if (grid[enemies[i].y / tileSize][enemies[i].x / tileSize] == 2 || grid[enemies[i].y / tileSize][enemies[i].x / tileSize] == 3)
                isGameRunning = false;

        ///////// DRAW SECTION /////////

        // Default background color when power-up is not active
        window.clear(Color::Black);

        // Draw grid tiles
        for (int i = 0; i < row; i++)
            for (int j = 0; j < cols; j++)
            {
                if (grid[i][j] == 0)
                    continue;

                if (grid[i][j] == 1)
                    tileSprite.setTextureRect(IntRect(0, 0, tileSize, tileSize));
                else if (grid[i][j] == 2)
                    tileSprite.setTextureRect(IntRect(54, 0, tileSize, tileSize));
                else if (grid[i][j] == 3)
                    tileSprite.setTextureRect(IntRect(108, 0, tileSize, tileSize));

                tileSprite.setPosition(j * tileSize, i * tileSize);
                window.draw(tileSprite);
            }
        // Draw Player 1
        tileSprite.setTextureRect(IntRect(36, 0, tileSize, tileSize));
        tileSprite.setPosition(p1X * tileSize, p1Y * tileSize);
        window.draw(tileSprite);

        // Draw Player 2
        tileSprite.setTextureRect(IntRect(72, 0, tileSize, tileSize));
        tileSprite.setPosition(p2X * tileSize, p2Y * tileSize);
        window.draw(tileSprite);
        // draw enemies
        enemySprite.rotate(10);
        for (int i = 0; i < currentEnemyCount; i++)
        {
            enemySprite.setPosition(enemies[i].x, enemies[i].y);
            window.draw(enemySprite);
        }

        // draw game over if game has ended
        if (!isGameRunning)
            window.draw(gameOverSprite);

        // Time display
        // Draw rotating enemies
        enemySprite.rotate(10);
        for (int i = 0; i < currentEnemyCount; i++)
        {
            enemySprite.setPosition(enemies[i].x, enemies[i].y);
            window.draw(enemySprite);
        }

        // Show game over screen if not running
        if (!isGameRunning)
            window.draw(gameOverSprite);

        drawText(window, "Player 1 Tiles : " + to_string(tiles_player_1), 0.5f, 0.5f, 60.0f, 470.0f, Color::Yellow);
        drawText(window, "Player 2 Tiles : " + to_string(tiles_player_2), 0.5f, 0.5f, 400.0f + 30.f, 470.0f, Color::Green);

        window.display();
    }
}

void game(RenderWindow &window)
{
    if (level == 1)
    {

        if (game_mode_select == 1)
            l1(window);
        else
            l1_2(window);
    }
    else if (level == 2)
    {
        if (game_mode_select == 1)
            l2(window);
        else
            l2_2(window);
    }
    else if (level == 3)
    {
        if (game_mode_select == 1)
            l3(window);
        else
            l3_2(window);
    }
    else if (level == 4)
    {
        if (game_mode_select == 1)
            cont(window);
        else
        {
            cont_2(window);
        }
    }
}

void showTopScores(RenderWindow &window)
{
    // Bubble Sort (descending order)
    for (int i = 0; i < scoreCount - 1; i++)
    {
        for (int j = 0; j < scoreCount - i - 1; j++)
        {
            if (scores[j] < scores[j + 1])
            {
                int temp = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = temp;
            }
        }
    }
    cout << "High Score: " << getHighestScore() << endl;
    cout << "All Scores: ";
    for (int i = 0; i < scoreCount; i++)
    {
        cout << scores[i] << " ";
    }

    cout << endl;
    Texture bg;
    if (!bg.loadFromFile("images/bg3.png"))
    {
        cout << "Error loading background image" << endl;
        return;
    }

    Sprite sBg(bg);
    sBg.setPosition(0, 0);
    sBg.setScale(static_cast<float>(window.getSize().x) / bg.getSize().x,
                 static_cast<float>(window.getSize().y) / bg.getSize().y);

    Texture backTexture;
    if (!backTexture.loadFromFile("images/back2.png"))
    {
        cout << "Error loading back button texture" << endl;
        return;
    }

    Sprite sBack(backTexture);
    sBack.setPosition(300, 400); // Adjust position as needed

    while (window.isOpen())
    {
        Event e;
        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();

            if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
            {
                Vector2i mousePos = Mouse::getPosition(window);
                checkButtonClicked(mousePos, sBack);
                return;
            }
        }

        window.clear();
        window.draw(sBg);

        drawText(window, "Top 5 Scores", 1.0f, 1.0f, 150, 50, Color::Red);

        for (int i = 0; i < min(5, scoreCount); i++)
        {
            drawText(window, to_string(i + 1) + ". " + to_string(scores[i]), 1.0f, 1.0f, 150, 100 + i * 50, Color::Yellow);
        }

        window.draw(sBack);
        window.display();
    }
}

int main()
{
    srand(time(0));

    RenderWindow window(VideoMode(cols * tileSize, row * tileSize + 50), "Xonix Game!");

    window.setFramerateLimit(60);

    Music bgMusic;
    if (!bgMusic.openFromFile("audio/magical.ogg"))
    {
        cout << "Error loading background music" << endl;
    }
    bgMusic.setLoop(true);
    bgMusic.play();
    bgMusic.setVolume(100); // Set volume to 50 (adjust as needed)

    menu_selection = true;

    readScores();

    cout << "High Score: " << getHighestScore() << endl;
    cout << "All Scores: ";
    for (int i = 0; i < scoreCount; i++)
    {
        cout << scores[i] << " ";
    }
    cout << endl;

    while (window.isOpen())
    {

        if (menu_selection)
        {
            menu(window);
        }
        else if (level_selection)
        {
            levelSelection(window);
        }
        else if (showScoreBoard)
        {
            showTopScores(window);
        }
        else if (game_type_selection)
        {
            gameMode(window);
        }
        else if (game_started)
        {
            game(window);
        }
    }



    cout << "Program Finished" << endl;
    return 0;
}