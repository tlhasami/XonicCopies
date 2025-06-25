#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <time.h>
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace sf;
using namespace std;

const int M = 25;
const int N = 40;
const int MAX_ENEMIES = 10;
const int ts = 18; // tile size
const float PI = 3.14159f;
const int MOVEMENT_LINEAR = 0;
const int MOVEMENT_ZIGZAG = 1;
const int MOVEMENT_SPIRAL = 2;

int grid[M][N] = {0};

struct GameState
{
    bool inMenu = true;
    bool inGame = false;
    bool gameOver = false;
    bool selectingDifficulty = false;
    int difficulty = 1;
    int addenemies = 0;
    Clock enemyspawnclock;
    Clock gameTimer;
    Clock speedIncreaseTimer;
    float speedMultiplier = 1.0f;
    int moveCount = 0;
    bool isMoving = false;
    bool patternsActivated = false;
};

struct Enemy
{
    int x, y;
    float dx, dy;
    float speedMultiplier;
    int pattern;     // Movement pattern type
    float angle;     // For circular movement
    float amplitude; // For zigzag movement
    float frequency; // For zigzag movement
    float time;      // For pattern timing
    float spiralRadius;
    float spiralSpeed;
    float spiralAngle;  // Separate angle for spiral motion
    float forwardSpeed; // Speed for forward movement

    Enemy(float initialSpeedMultiplier = 1.0f) : speedMultiplier(initialSpeedMultiplier)
    {
        x = (rand() % (N - 2) + 1) * ts;
        y = (rand() % (M - 2) + 1) * ts;
        pattern = MOVEMENT_LINEAR;
        angle = 0;
        amplitude = 30.0f;
        frequency = 0.05f;
        time = 0;
        spiralRadius = 20.0f;
        spiralSpeed = 0.05f;
        spiralAngle = 0;
        forwardSpeed = 4.0f * speedMultiplier; // Match the base speed of other enemies
        setRandomDirection();
    }

    void setRandomDirection()
    {
        float angle = (rand() % 360) * PI / 180.0f;
        float speed = 4.0f * speedMultiplier;
        dx = cos(angle) * speed;
        dy = sin(angle) * speed;
    }

    // Linear movement pattern
    void moveLinear()
    {
        int prevX = x;
        int prevY = y;

        x += dx;
        y += dy;

        int gridX = x / ts;
        int gridY = y / ts;

        if (gridX < 0 || gridX >= N || gridY < 0 || gridY >= M || grid[gridY][gridX] == 1)
        {
            x = prevX;
            y = prevY;
            setRandomDirection();
        }
    }

    // Zigzag movement pattern
    void moveZigzag()
    {
        int prevX = x;
        int prevY = y;

        time += 0.05f; // Slower time increment for smoother motion
        float offset = sin(time * frequency) * amplitude;

        // Calculate new position with continuous movement
        float newX = x + dx;
        float newY = y + dy + offset;

        // Check boundaries and adjust if needed
        int gridX = static_cast<int>(newX) / ts;
        int gridY = static_cast<int>(newY) / ts;

        if (gridX < 1 || gridX >= N - 1 || gridY < 1 || gridY >= M - 1 || grid[gridY][gridX] == 1)
        {
            // If hitting boundary, reverse direction and adjust angle
            dx = -dx;
            dy = -dy;
            time += PI; // Phase shift to maintain smooth motion
        }
        else
        {
            x = newX;
            y = newY;
        }
    }

    // Updated spiral movement pattern
    void moveSpiral()
    {
        int prevX = x;
        int prevY = y;

        // Update spiral parameters with reduced speed
        spiralAngle += 0.05f;                                    // Reduced from 0.1f
        spiralRadius = 20.0f + sin(spiralAngle * 0.25f) * 10.0f; // Reduced oscillation speed

        // Calculate spiral offset
        float spiralOffsetX = cos(spiralAngle) * spiralRadius;
        float spiralOffsetY = sin(spiralAngle) * spiralRadius;

        // Calculate forward movement with matching speed
        float forwardX = x + dx * forwardSpeed;
        float forwardY = y + dy * forwardSpeed;

        // Combine forward movement with spiral motion
        float newX = forwardX + spiralOffsetX;
        float newY = forwardY + spiralOffsetY;

        // Check boundaries
        int gridX = static_cast<int>(newX) / ts;
        int gridY = static_cast<int>(newY) / ts;

        if (gridX < 1 || gridX >= N - 1 || gridY < 1 || gridY >= M - 1 || grid[gridY][gridX] == 1)
        {
            // If hitting boundary, reverse direction and adjust spiral
            dx = -dx;
            dy = -dy;
            spiralAngle += PI; // Phase shift to maintain smooth spiral
        }
        else
        {
            x = newX;
            y = newY;
        }
    }

    void move()
    {
        switch (pattern)
        {
        case MOVEMENT_LINEAR:
            moveLinear();
            break;
        case MOVEMENT_ZIGZAG:
            moveZigzag();
            break;
        case MOVEMENT_SPIRAL:
            moveSpiral();
            break;
        }
    }

    void updateSpeed(float newMultiplier)
    {
        speedMultiplier = newMultiplier;
        float currentSpeed = sqrt(dx * dx + dy * dy);
        if (currentSpeed > 0)
        {
            dx = (dx / currentSpeed) * 4.0f * newMultiplier;
            dy = (dy / currentSpeed) * 4.0f * newMultiplier;
        }
        else
        {
            setRandomDirection();
        }
    }

    void setPattern(int newPattern)
    {
        pattern = newPattern;
        if (pattern == MOVEMENT_SPIRAL)
        {
            spiralAngle = 0;
            spiralRadius = 20.0f;
            // Set initial direction for spiral movement
            float angle = (rand() % 360) * PI / 180.0f;
            dx = cos(angle);
            dy = sin(angle);
        }
        else if (pattern == MOVEMENT_ZIGZAG)
        {
            time = 0;
        }
    }
};

void drop(int y, int x)
{
    if (y < 0 || y >= M || x < 0 || x >= N)
        return;
    if (grid[y][x] == 0)
        grid[y][x] = -1;
    if (y - 1 >= 0 && grid[y - 1][x] == 0)
        drop(y - 1, x);
    if (y + 1 < M && grid[y + 1][x] == 0)
        drop(y + 1, x);
    if (x - 1 >= 0 && grid[y][x - 1] == 0)
        drop(y, x - 1);
    if (x + 1 < N && grid[y][x + 1] == 0)
        drop(y, x + 1);
}

void resetGame()
{
    // Clear the entire grid first
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            grid[i][j] = 0;

    // Set only the boundary walls (hollow square)
    for (int i = 0; i < M; i++)
    {
        grid[i][0] = 1;     // Left wall
        grid[i][N - 1] = 1; // Right wall
    }
    for (int j = 0; j < N; j++)
    {
        grid[0][j] = 1;     // Top wall
        grid[M - 1][j] = 1; // Bottom wall
    }
}

void updateMoveCount(GameState &gameState, int oldX, int oldY, int newX, int newY, const int grid[M][N])
{
    static bool isBuilding = false; // Track if we're currently building

    // Check if we're starting to build (moving from filled to empty)
    if (grid[oldY][oldX] == 2 && grid[newY][newX] == 0 && !isBuilding)
    {
        isBuilding = true; // Mark that we've started building
    }

    // Check if we're finishing building (hitting a wall)
    if (grid[newY][newX] == 1 && isBuilding)
    {
        gameState.moveCount++; // Only increment when we finish building
        isBuilding = false;    // Reset building state
    }
}

bool isPositionEmpty(int x, int y)
{
    return grid[y / ts][x / ts] == 0; // check if the grid position is empty
}

void spawnEnemyAtRandomPosition(Enemy enemies[], int &enemyCount, float speedMultiplier)
{
    if (enemyCount >= MAX_ENEMIES)
        return;

    int maxAttempts = 100; // Prevent infinite loop
    int attempts = 0;
    bool positionFound = false;

    while (!positionFound && attempts < maxAttempts)
    {
        // Generate random position
        int newX = (rand() % (N - 2) + 1) * ts;
        int newY = (rand() % (M - 2) + 1) * ts;

        // Check if position is empty
        if (isPositionEmpty(newX, newY))
        {
            // Create new enemy at empty position
            enemies[enemyCount] = Enemy(speedMultiplier);
            enemies[enemyCount].x = newX;
            enemies[enemyCount].y = newY;
            enemyCount++;
            positionFound = true;
        }
        attempts++;
    }
}

void twoPlayer(RenderWindow &window, GameState &gameState, Sprite &sTile)
{
    Texture tileTexture, gameOverTexture, enemyTexture;

    if (!tileTexture.loadFromFile("images/tiles.png"))
        cout << "Error loading tiles texture" << endl;
    if (!gameOverTexture.loadFromFile("images/gameover.png"))
        cout << "Error loading gameover texture" << endl;
    if (!enemyTexture.loadFromFile("images/enemy.png"))
        cout << "Error loading enemy texture" << endl;

    Sprite tileSprite(tileTexture), gameOverSprite(gameOverTexture), enemySprite(enemyTexture);

    gameOverSprite.setPosition(100, 100);
    enemySprite.setOrigin(20, 20);

    int maxEnemies = (gameState.difficulty == 0) ? 2 : (gameState.difficulty == 1) ? 4
                                                   : (gameState.difficulty == 2)   ? 6
                                                   : (gameState.difficulty == 3)   ? 2
                                                                                   : 2; // Default for other cases
            
    Enemy enemies[MAX_ENEMIES];
    int enemyCount = maxEnemies;

    int player_1_x = 0, player_1_y = 0;
    int player_1_direction_x = 0, player_1_direction_y = 0;
    int player_2_x = N - 1, player_2_y = M - 1;
    int player_2_direction_x = 0, player_2_direction_y = 0;
    int player_1_previous_position_x = 0, player_1_previous_position_y = 0;
    int player_2_previous_position_x = 0, player_2_previous_position_y = 0;

    int tiles_player_1 = 0, tiles_player_2 = 0;

    bool isGameRunning = true, gameOverClockStarted = false;

    Clock frameClock, gameOverClock, enemySpawnClock;

    float animationTimer = 0, animationDelay = 0.1f;

    // Text objects for Player 1 Tiles, Player 2 Tiles, and Time
    Text player1TilesText, player2TilesText, gameTimeText;

    Font font;
    // Load font for the text objects
    if (!font.loadFromFile("fonts/ARIAL.TTF"))
    {
        cerr << "Failed to load font!" << endl;
        return;
    }

    // Set font and properties for Player 1 Tiles text
    player1TilesText.setFont(font);
    player1TilesText.setCharacterSize(20);
    player1TilesText.setFillColor(Color::Magenta);
    player1TilesText.setPosition(10, 40);

    // Set font and properties for Player 2 Tiles text
    player2TilesText.setFont(font);
    player2TilesText.setCharacterSize(20);
    player2TilesText.setFillColor(Color::Yellow);
    player2TilesText.setPosition(window.getSize().x - player2TilesText.getLocalBounds().width - 200, 40);

    // Set font and properties for Game Time text
    gameTimeText.setFont(font);
    gameTimeText.setCharacterSize(20);
    gameTimeText.setFillColor(Color::White);
    gameTimeText.setPosition((window.getSize().x - gameTimeText.getLocalBounds().width) / 2, 10);

    // Bold all strings and set default values
    player1TilesText.setStyle(Text::Bold);
    player1TilesText.setString("Player 1 Tiles : 0");

    player2TilesText.setStyle(Text::Bold);
    player2TilesText.setString("Player 2 Tiles : 0");

    gameTimeText.setStyle(Text::Bold);
    gameTimeText.setString("Time: 00:00");

    while (window.isOpen())
    {
        if (player_1_x == player_2_x && player_1_y == player_2_y)
        {
            isGameRunning = false;
        }

        float deltaTime = frameClock.getElapsedTime().asSeconds();

        frameClock.restart();

        animationTimer += deltaTime;

        // Update game time text
        int totalSeconds = static_cast<int>(gameState.gameTimer.getElapsedTime().asSeconds());

        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;

        stringstream gameTimeStream;
        gameTimeStream << "Time: " << setw(2) << setfill('0') << minutes << ":"
                       << setw(2) << setfill('0') << seconds;

        gameTimeText.setString(gameTimeStream.str());

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
                // Display game over screen and exit
                cout << "Game Over! Player 1 Tiles: " << tiles_player_1
                     << ", Player 2 Tiles: " << tiles_player_2 << endl;
                return;
            }
        }

        // Handle Player 1 movement (WASD)
        if (Keyboard::isKeyPressed(Keyboard::A)) // Left
        {
            player_1_direction_x = -1;
            player_1_direction_y = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::D)) // Right
        {
            player_1_direction_x = 1;
            player_1_direction_y = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::W)) // Up
        {
            player_1_direction_x = 0;
            player_1_direction_y = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::S)) // Down
        {
            player_1_direction_x = 0;
            player_1_direction_y = 1;
        }

        // Handle Player 2 movement (Arrow Keys)
        if (Keyboard::isKeyPressed(Keyboard::Left)) // Left
        {
            player_2_direction_x = -1;
            player_2_direction_y = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Right)) // Right
        {
            player_2_direction_x = 1;
            player_2_direction_y = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Up)) // Up
        {
            player_2_direction_x = 0;
            player_2_direction_y = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::Down)) // Down
        {
            player_2_direction_x = 0;
            player_2_direction_y = 1;
        }

        if (!isGameRunning)
            continue;

        // Move players every frame if enough time has passed
        if (animationTimer > animationDelay)
        {
            player_1_previous_position_x = player_1_x;
            player_1_previous_position_y = player_1_y;

            player_2_previous_position_x = player_2_x;
            player_2_previous_position_y = player_2_y;

            // Move Player 1
            player_1_x += player_1_direction_x;
            player_1_y += player_1_direction_y;

            // Move Player 2
            player_2_x += player_2_direction_x;
            player_2_y += player_2_direction_y;

            // Keep Player 1 inside grid bounds
            if (player_1_x < 0)
                player_1_x = 0;
            if (player_1_x > N - 1)
                player_1_x = N - 1;
            if (player_1_y < 0)
                player_1_y = 0;
            if (player_1_y > M - 1)
                player_1_y = M - 1;

            // Keep Player 2 inside grid bounds
            if (player_2_x < 0)
                player_2_x = 0;
            if (player_2_x > N - 1)
                player_2_x = N - 1;
            if (player_2_y < 0)
                player_2_y = 0;
            if (player_2_y > M - 1)
                player_2_y = M - 1;

            // Check for collision with self for Player 1 and 2
            if (grid[player_1_y][player_1_x] == 2 || grid[player_2_y][player_2_x] == 3)
                isGameRunning = false;

            // Mark visited cells for Player 1
            if (grid[player_1_y][player_1_x] == 0)
            {
                grid[player_1_y][player_1_x] = 2;
            }

            // Mark visited cells for Player 2
            if (grid[player_2_y][player_2_x] == 0)
            {
                grid[player_2_y][player_2_x] = 3;
            }

            animationTimer = 0;
        }

        // Move all enemies
        for (int i = 0; i < enemyCount; i++)
            enemies[i].move();

        // Handle Player 1 reaching a wall
        if (grid[player_1_y][player_1_x] == 1)
        {
            player_1_direction_x = player_1_direction_y = 0;
            if (grid[player_1_previous_position_y][player_1_previous_position_x] == 2)
            {
                cout << "Inside" << endl;
                for (int i = 0; i < enemyCount; i++)
                    drop(enemies[i].y / ts, enemies[i].x / ts);

                for (int i = 0; i < M; i++)
                {
                    for (int j = 0; j < N; j++)
                    {
                        if (grid[i][j] == -1)
                            grid[i][j] = 0;
                        else if (grid[i][j] == 2 || grid[i][j] == 0)
                        {
                            tiles_player_1++;
                            grid[i][j] = 1;
                        }
                    }
                }
                player1TilesText.setString("Player 1 Tiles : " + to_string(tiles_player_1));
            }
        }

        // Handle Player 2 reaching a wall
        if (grid[player_2_y][player_2_x] == 1)
        {
            player_2_direction_x = player_2_direction_y = 0;
            if (grid[player_2_previous_position_y][player_2_previous_position_x] == 3)
            {
                for (int i = 0; i < enemyCount; i++)
                    drop(enemies[i].y / ts, enemies[i].x / ts);

                for (int i = 0; i < M; i++)
                {
                    for (int j = 0; j < N; j++)
                    {
                        if (grid[i][j] == -1)
                            grid[i][j] = 0;
                        else if (grid[i][j] == 3 || grid[i][j] == 0)
                        {
                            tiles_player_2++;
                            grid[i][j] = 1;
                        }
                    }
                }
                player2TilesText.setString("Player 2 Tiles : " + to_string(tiles_player_2));
            }
        }

        // End game if enemy touches player trail
        for (int i = 0; i < enemyCount; i++)
        {
            int enemyX = enemies[i].x / ts;
            int enemyY = enemies[i].y / ts;
            if (grid[enemyY][enemyX] == 2 || grid[enemyY][enemyX] == 3)
                isGameRunning = false;
        }

        // Add 2 enemies every 20 seconds if difficulty is 3
        if (gameState.difficulty == 3 && enemySpawnClock.getElapsedTime().asSeconds() >= 20.0f)
        {
            for (int i = 0; i < 2 && enemyCount < MAX_ENEMIES; i++)
            {
                spawnEnemyAtRandomPosition(enemies, enemyCount, gameState.speedMultiplier);
            }
            enemySpawnClock.restart();
        }

        //////// DRAW SECTION /////////

        window.clear(Color::Black);

        // Draw grid tiles
        for (int i = 0; i < 25; i++)
            for (int j = 0; j < 40; j++)
            {

                if (grid[i][j] == 0)
                    continue;

                if (grid[i][j] == 1)
                    tileSprite.setTextureRect(IntRect(0, 0, 18, 18));

                else if (grid[i][j] == 2)
                    tileSprite.setTextureRect(IntRect(54, 0, 18, 18));

                else if (grid[i][j] == 3)
                    tileSprite.setTextureRect(IntRect(108, 0, 18, 18));

                tileSprite.setPosition(j * 18, i * 18);
                window.draw(tileSprite);

            }

        // Draw Player 1
        tileSprite.setTextureRect(IntRect(36, 0, 18, 18));
        tileSprite.setPosition(player_1_x * 18, player_1_y * 18);
        window.draw(tileSprite);

        // Draw Player 2
        tileSprite.setTextureRect(IntRect(72, 0, 18, 18));
        tileSprite.setPosition(player_2_x * 18, player_2_y * 18);
        window.draw(tileSprite);

        // Draw rotating enemies
        enemySprite.rotate(10);

        for (int i = 0; i < enemyCount; i++)
        {
            enemySprite.setPosition(enemies[i].x, enemies[i].y);
            window.draw(enemySprite);
        }
        // Draw Player 1 Tiles text
        window.draw(player1TilesText);

        // Draw Player 2 Tiles text
        window.draw(player2TilesText);

        // Draw Game Time text
        window.draw(gameTimeText);
        // Show game over screen if not running
        if (!isGameRunning)
            window.draw(gameOverSprite);

        // Display who won after game over
        if (!isGameRunning)
        {
            Text winnerText;
            winnerText.setFont(font);
            winnerText.setCharacterSize(30);
            winnerText.setFillColor(Color::White);
            winnerText.setStyle(Text::Bold);

            if (tiles_player_1 > tiles_player_2)
            {
                winnerText.setString("Player 1 Wins!");
                winnerText.setFillColor(Color::Magenta);
            }
            else if (tiles_player_2 > tiles_player_1)
            {
                winnerText.setString("Player 2 Wins!");
                winnerText.setFillColor(Color::Yellow);
            }
            else
            {
                winnerText.setString("It's a Tie!");
            }

            FloatRect textBounds = winnerText.getLocalBounds();
            winnerText.setPosition(
                (window.getSize().x - textBounds.width) / 2,
                (window.getSize().y - textBounds.height) / 2 + 50);

            window.draw(winnerText);
        }
        window.display();
    }
}

int askMode(RenderWindow &modeWindow)
{

    // Load background texture
    Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("images/background_startmenu.png"))
    {
        cerr << "Failed to load background texture!" << endl;
        return 1; // Default to 1 player mode if texture fails
    }

    Sprite backgroundSprite(backgroundTexture);
    backgroundSprite.setScale(
        modeWindow.getSize().x / backgroundSprite.getLocalBounds().width,
        modeWindow.getSize().y / backgroundSprite.getLocalBounds().height);

    // Load font
    Font font;
    if (!font.loadFromFile("fonts/ARIAL.TTF"))
    {
        cerr << "Failed to load font!" << endl;
        return 1; // Default to 1 player mode if font fails
    }

    // Create text
    Text modeText;
    modeText.setFont(font);
    modeText.setString("Press 1 for 1 Player\nPress 2 for 2 Players");
    modeText.setCharacterSize(24);
    modeText.setFillColor(Color::White);
    modeText.setStyle(Text::Bold);
    FloatRect textBounds = modeText.getLocalBounds();
    modeText.setPosition(
        (modeWindow.getSize().x - textBounds.width) / 2,
        (modeWindow.getSize().y - textBounds.height) / 2);

    while (modeWindow.isOpen())
    {
        Event event;
        while (modeWindow.pollEvent(event))
        {
            if (event.type == Event::Closed)
                modeWindow.close();

            if (event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::Num1 )
                {

                    return 1; // 1 player mode
                }
                if (event.key.code == Keyboard::Num2)
                {

                    return 2; // 2 player mode
                }
            }
        }

        modeWindow.clear();
        modeWindow.draw(backgroundSprite);
        modeWindow.draw(modeText);
        modeWindow.display();
    }

    return 1; // Default to 1 player mode if window is closed
}

int main()
{

    srand(time(0));

    RenderWindow window(VideoMode(N * ts, M * ts), "Xonix Game!");
    window.setFramerateLimit(60);

    // Load textures
    Texture t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11;
    if (!t1.loadFromFile("images/tiles.png") ||
        !t2.loadFromFile("images/gameover.png") ||
        !t3.loadFromFile("images/enemy.png") ||
        !t4.loadFromFile("images/background_startmenu.png") ||
        !t5.loadFromFile("images/play.png") ||
        !t6.loadFromFile("images/select_level.png") ||
        !t7.loadFromFile("images/scoreboard.png") ||
        !t8.loadFromFile("images/easy.png") ||
        !t9.loadFromFile("images/medium.png") ||
        !t10.loadFromFile("images/hard.png") ||
        !t11.loadFromFile("images/continuous.png"))
    {
        cerr << "Failed to load textures!" << endl;
        return -1;
    }

    // Create sprites
    Sprite sTile(t1), sGameover(t2), sEnemy(t3), sBackground(t4),
        sPlay(t5), sSelectlevel(t6), sScoreboard(t7),
        sEasy(t8), sMedium(t9), sHard(t10), sContinuous(t11);

    // Position menu items
    float windowWidth = N * ts;
    float windowHeight = M * ts;
    float centerX = (windowWidth - 200) / 2;

    sBackground.setPosition(0, 0);
    sPlay.setPosition(centerX, windowHeight * 0.3);
    sSelectlevel.setPosition(centerX, windowHeight * 0.45);
    sScoreboard.setPosition(centerX, windowHeight * 0.6);
    sGameover.setPosition(100, 100);
    sEnemy.setOrigin(20, 20);
    sEasy.setPosition(centerX, windowHeight * 0.3);
    sMedium.setPosition(centerX, windowHeight * 0.45);
    sHard.setPosition(centerX, windowHeight * 0.6);
    sContinuous.setPosition(centerX, windowHeight * 0.75);

    // Get bounds for click detection
    FloatRect playBounds = sPlay.getGlobalBounds();
    FloatRect selectlevelBounds = sSelectlevel.getGlobalBounds();
    FloatRect scoreboardBounds = sScoreboard.getGlobalBounds();
    FloatRect easyBounds = sEasy.getGlobalBounds();
    FloatRect mediumBounds = sMedium.getGlobalBounds();
    FloatRect hardBounds = sHard.getGlobalBounds();
    FloatRect continuousBounds = sContinuous.getGlobalBounds();

    // Load font
    Font font;
    if (!font.loadFromFile("fonts/ARIAL.TTF"))
    {
        cerr << "Failed to load font file!" << endl;
        return -1;
    }

    // Create text objects
    Text timerText, moveText;
    timerText.setFont(font);
    timerText.setCharacterSize(20);
    timerText.setFillColor(Color::Red);
    timerText.setPosition(10, 10);

    moveText.setFont(font);
    moveText.setCharacterSize(20);
    moveText.setFillColor(Color::Yellow);
    moveText.setPosition(N * ts - 150, 10);

    // Bold the timer and move text
    timerText.setStyle(Text::Bold);
    moveText.setStyle(Text::Bold);

    // Game variables
    GameState gameState;
    int enemyCount = 4;
    Enemy enemies[MAX_ENEMIES];
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i] = Enemy(gameState.speedMultiplier);
    }
    int x = 0, y = 0, dx = 0, dy = 0;
    float timer = 0, delay = 0.07;
    Clock clock;

    bool scoreboard = false;
    resetGame();
    // Array to store scores
    int scores[100];
    int scoreCount = 0;

    // Open the file "scores"
    ifstream scoreFile("scores.txt");

    int pl_tiles = 0 ;
    // Create text object for displaying Tile 1 count
    Text tile1Text;
    tile1Text.setFont(font);
    tile1Text.setCharacterSize(20);
    tile1Text.setFillColor(Color::Green);
    tile1Text.setStyle(Text::Bold);
    tile1Text.setPosition(N * ts - 150, 40); // Position below the moves text
    tile1Text.setString("Tile 1: 0");

    if (!scoreFile.is_open())
    {
        cout << "Failed to open scores file!" << endl;
    }
    else
    {
        string line;
        while (getline(scoreFile, line) && scoreCount < 100)
        {

            scores[scoreCount++] = stoi(line);
        }
        scoreFile.close();
    }

    // Bubble sort in descending order
    for (int i = 0; i < scoreCount - 1; i++)
    {
        for (int j = 0; j < scoreCount - i - 1; j++)
        {
            if (scores[j] < scores[j + 1])
            {
                // Manually swap scores[j] and scores[j + 1]
                int temp = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = temp;
            }
        }
    }

    // Create text object for displaying top 5 scores
    Text topScoresText;
    topScoresText.setFont(font);
    topScoresText.setCharacterSize(20);
    topScoresText.setFillColor(Color::White);
    topScoresText.setStyle(Text::Bold);
    topScoresText.setPosition(250, 100); // Adjust position as needed

    // Build the string for the top 5 scores
    stringstream scoresStream;
    scoresStream << "Top 5 Scores:\n";
    for (int i = 0; i < min(5, scoreCount); i++)
    {
        scoresStream << i + 1 << ". Score: " << scores[i] << "\n";
    }
    topScoresText.setString(scoresStream.str());

    int mode = 0;
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

            if (gameState.gameOver && e.type == Event::KeyPressed && e.key.code == Keyboard::Escape)
            {
                gameState.inMenu = true;
                gameState.inGame = false;
                gameState.gameOver = false;
                gameState.selectingDifficulty = false;
                gameState.difficulty = 1;
                resetGame();
            }
            
            if (scoreboard && e.type == Event::KeyPressed && e.key.code == Keyboard::Escape)
            {
                scoreboard = false;
                gameState.inMenu = true;
            }

            if (e.type == Event::MouseButtonPressed)
            {
                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                if (gameState.inMenu)
                {
                    if (playBounds.contains(mousePos))
                    {
                        mode = askMode(window);
                        if (mode == 2)
                        {
                            twoPlayer(window, gameState, sTile);

                            resetGame();
                            continue;
                        }
                        gameState.inMenu = false;
                        gameState.inGame = true;
                        gameState.gameOver = false;
                        gameState.selectingDifficulty = false;
                        gameState.gameTimer.restart();
                        gameState.speedIncreaseTimer.restart();
                        gameState.moveCount = 0;
                        gameState.isMoving = false;
                        gameState.speedMultiplier = 1.0f;
                        dx = dy = 0;
                        x = 10;
                        y = 0;
                        resetGame();

                        // Reinitialize enemies with proper positions and speeds
                        for (int i = 0; i < MAX_ENEMIES; i++)
                        {
                            enemies[i] = Enemy(gameState.speedMultiplier);
                        }
                    }
                    else if (selectlevelBounds.contains(mousePos))
                    {
                        gameState.inMenu = false;
                        gameState.selectingDifficulty = true;
                    }
                    else if (scoreboardBounds.contains(mousePos))
                    {
                        gameState.inMenu = false ;
                        scoreboard = true ;
                    }
                }
                else if (gameState.selectingDifficulty)
                {
                    if (easyBounds.contains(mousePos))
                    {
                        gameState.difficulty = 0;
                        enemyCount = 2;
                        gameState.selectingDifficulty = false;
                        gameState.inMenu = true;
                    }
                    else if (mediumBounds.contains(mousePos))
                    {
                        gameState.difficulty = 1;
                        enemyCount = 4;
                        gameState.selectingDifficulty = false;
                        gameState.inMenu = true;
                    }
                    else if (hardBounds.contains(mousePos))
                    {
                        gameState.difficulty = 2;
                        enemyCount = 6;
                        gameState.selectingDifficulty = false;
                        gameState.inMenu = true;
                    }
                    else if (continuousBounds.contains(mousePos))
                    {
                        gameState.difficulty = 3;
                        enemyCount = 2;
                        gameState.addenemies = 2;
                        gameState.enemyspawnclock.restart();
                        gameState.selectingDifficulty = false;
                        gameState.inMenu = true;
                    }
                }
            }

            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape)
            {
                if (gameState.inGame)
                {
                    gameState.inMenu = true;
                    gameState.inGame = false;
                    gameState.gameOver = false;
                    gameState.selectingDifficulty = false;
                    gameState.difficulty = 1;
                    resetGame();
                }
            }
        }

        // Game logic
        if (gameState.inGame)
        {
            // Check for speed increase every 20 seconds
            if (gameState.speedIncreaseTimer.getElapsedTime().asSeconds() >= 20.0f)
            {
                gameState.speedMultiplier *= 1.1f;
                for (int i = 0; i < enemyCount; i++)
                {
                    enemies[i].updateSpeed(gameState.speedMultiplier);
                }
                gameState.speedIncreaseTimer.restart();
            }

            // Switch half of enemies to geometric patterns after 30 seconds
            if (gameState.gameTimer.getElapsedTime().asSeconds() >= 30.0f && !gameState.patternsActivated)
            {
                gameState.patternsActivated = true;

                // Calculate 25% of enemies for each pattern
                int quarterEnemies = enemyCount / 4;

                // Set zigzag pattern for first quarter
                for (int i = 0; i < quarterEnemies; i++)
                {
                    enemies[i].setPattern(MOVEMENT_ZIGZAG);
                }

                // Set spiral pattern for second quarter
                for (int i = quarterEnemies; i < quarterEnemies * 2; i++)
                {
                    enemies[i].setPattern(MOVEMENT_SPIRAL);
                }

                // Remaining enemies stay linear
            }

            bool wasMoving = gameState.isMoving;
            gameState.isMoving = false;

            if (Keyboard::isKeyPressed(Keyboard::Left))
            {
                dx = -1;
                dy = 0;
                gameState.isMoving = true;
            }
            if (Keyboard::isKeyPressed(Keyboard::Right))
            {
                dx = 1;
                dy = 0;
                gameState.isMoving = true;
            }
            if (Keyboard::isKeyPressed(Keyboard::Up))
            {
                dx = 0;
                dy = -1;
                gameState.isMoving = true;
            }
            if (Keyboard::isKeyPressed(Keyboard::Down))
            {
                dx = 0;
                dy = 1;
                gameState.isMoving = true;
            }

            if (timer > delay)
            {
                // Store current position before moving
                int oldX = x;
                int oldY = y;

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

                // Update move count using the new function
                updateMoveCount(gameState, oldX, oldY, x, y, grid);

                if (grid[y][x] == 2)
                {
                    gameState.inGame = false;
                    gameState.gameOver = true;
                    ofstream scoreFile("scores.txt", ios::app);
                    if (scoreFile.is_open())
                    {
                        scoreFile << pl_tiles << endl; // Write the score to the file
                        scoreFile.close();
                    }
                    else
                    {
                        cerr << "Failed to open scores file for writing!" << endl;
                    }
                
                }
                if (grid[y][x] == 0)
                    grid[y][x] = 2;
                timer = 0;
            }

            // Update timer and move counter
            int totalSeconds = static_cast<int>(gameState.gameTimer.getElapsedTime().asSeconds());
            int minutes = totalSeconds / 60;
            int seconds = totalSeconds % 60;
            stringstream timerStream;
            timerStream << "Time: " << setw(2) << setfill('0') << minutes << ":"
                        << setw(2) << setfill('0') << seconds;
            timerText.setString(timerStream.str());

            stringstream moveStream;
            moveStream << "Moves: " << gameState.moveCount;
            moveText.setString(moveStream.str());

            // Handle continuous mode enemy spawning
            if (gameState.difficulty == 3 && gameState.enemyspawnclock.getElapsedTime().asSeconds() > 20.0f)
            {
                if (enemyCount < MAX_ENEMIES)
                {
                    // Try to find an empty position for the new enemy
                    int maxAttempts = 100; // Prevent infinite loop
                    int attempts = 0;
                    bool positionFound = false;

                    while (!positionFound && attempts < maxAttempts)
                    {
                        // Generate random position
                        int newX = (rand() % (N - 2) + 1) * ts;
                        int newY = (rand() % (M - 2) + 1) * ts;

                        // Check if position is empty
                        if (isPositionEmpty(newX, newY))
                        {
                            // Create new enemy at empty position
                            enemies[enemyCount] = Enemy(gameState.speedMultiplier);
                            enemies[enemyCount].x = newX;
                            enemies[enemyCount].y = newY;
                            enemyCount++;
                            positionFound = true;
                        }
                        attempts++;
                    }
                }
                gameState.enemyspawnclock.restart();
            }

            for (int i = 0; i < enemyCount; i++)
            {
                enemies[i].move();
            }

            if (grid[y][x] == 1)
            {
                dx = dy = 0;
                gameState.isMoving = false;

                for (int i = 0; i < enemyCount; i++)
                {
                    int enemyX = enemies[i].x / ts;
                    int enemyY = enemies[i].y / ts;
                    if (enemyX >= 0 && enemyX < N && enemyY >= 0 && enemyY < M)
                    {
                        drop(enemyY, enemyX);
                    }
                }

                for (int i = 0; i < M; i++)
                    for (int j = 0; j < N; j++)
                        if (grid[i][j] == -1)
                            grid[i][j] = 0;
                        else if (grid[i][j] == 2 || grid[i][j] == 0){
                            pl_tiles ++;
                            grid[i][j] = 1;
                        }
            }

            for (int i = 0; i < enemyCount; i++)
            {
                int enemyX = enemies[i].x / ts;
                int enemyY = enemies[i].y / ts;
                if (enemyX >= 0 && enemyX < N && enemyY >= 0 && enemyY < M)
                {
                    if (grid[enemyY][enemyX] == 2)
                    {
                        gameState.inGame = false;
                        gameState.gameOver = true;
                        // Open the file in append mode to write the score
                        ofstream scoreFile("scores.txt", ios::app);
                        if (scoreFile.is_open())
                        {
                            scoreFile << pl_tiles << endl; // Write the score to the file
                            scoreFile.close();
                        }
                        else
                        {
                            cerr << "Failed to open scores file for writing!" << endl;
                        }
                    }
                }
            }
        }

        // Drawing
        window.clear();

        if (gameState.inMenu)
        {
            window.draw(sBackground);
            window.draw(sPlay);
            window.draw(sSelectlevel);
            window.draw(sScoreboard);
        }
        else if (gameState.selectingDifficulty)
        {
            window.draw(sBackground);
            window.draw(sEasy);
            window.draw(sMedium);
            window.draw(sHard);
            window.draw(sContinuous);
        }
        else if (gameState.inGame)
        {
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
                sEnemy.setPosition(enemies[i].x, enemies[i].y);
                window.draw(sEnemy);
            }

            window.draw(timerText);
            window.draw(moveText);
            // Display the Tile 1 count
            tile1Text.setString("Tile 1: " + to_string(pl_tiles));
            window.draw(tile1Text);
        }
        else if (gameState.gameOver)
        {
            window.draw(sGameover);
        }
        else if (scoreboard)
        {

            // Draw the top scores text
            window.draw(topScoresText);
        }
        window.display();
    }

    return 0;
}
