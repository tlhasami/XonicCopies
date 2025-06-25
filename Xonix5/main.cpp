#include <SFML/Graphics.hpp>
#include <iostream>
#include <time.h>
#include <fstream>
#include <string>
#include <cmath>
using namespace sf;
using namespace std;

// Game Constants
const int M = 25;  // Grid height
const int N = 40;  // Grid width
const int ts = 18; // Tile size

// Game Grid (0-empty, 1-border, 2-player trail)
int grid[M][N] = {0};

int currentState = 1;
// Game States
// enum GameStates
// {
//   MENU,
//   LEVEL_SELECT,
//   PLAYER_SELECT,
//   GAMEPLAY,
//   GAMEOVER,
//   SCOREBOARD
// };

// Difficulty levels
// enum Difficulty
// {
//   EASY = 2,
//   MEDIUM = 4,
//   HARD = 6,
//   CONTINUOUS = 7 // Special mode starting with 2 enemies
// };

// Game Variables
int score = 0, score2 = 0;
int highScores[5] = {0};
// int highScores2[5] = {0};
std::string highScoreNames[5] = {"Player1", "Player2", "Player3", "Player4", "Player5"};
int highScoreTimes[5] = {0};
int tilesCapturedInMove = 0, tilesCapturedInMove2 = 0;
int bonusCounter = 0, bonusCounter2 = 0;
int powerUps = 0, powerUps2 = 0;
// bool powerUpActive = false;
int moveCounter = 0, moveCounter2 = 0;
bool moveInProgress = false, moveInProgress2 = false;
int difficultyLevel = 4;
bool continuousMode = false;
bool patternMovement = false;
bool initial = true;

bool player2include = false;

int previousHighScore;

struct Player
{
  int x, y, dx, dy, last_dx, last_dy, last_x, last_y;
};

Player P1 = {N - 1, M - 1, 0, 0, 0, 0};
Player P2 = {0, 0, 0, 0, 0, 0};

struct Enemy
{
  int x, y, dx, dy;
  int movementPattern; // 0 = linear, 1 = zigzag, 2 = circular
  float patternTimer;
  float angle; // For circular movement

  Enemy()
  {

    do
    {
      x = 2 + rand() % (N - 4);
      y = 2 + rand() % (M - 4);

    } while (grid[y][x] != 0);

    x *= ts;
    y *= ts;

    cout << "  x = " << x << " y = " << y << endl;

    dx = 4 - rand() % 8;
    dy = 4 - rand() % 8;
    if (dx == 0)
      dx = 1;
    if (dy == 0)
      dy = 1;
    movementPattern = 0;
    patternTimer = 0;
    angle = 0;
  }

  void linearMove()
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

  void zigzagMove()
  {
    patternTimer += 0.1f;
    if (patternTimer >= 2.0f)
    {
      // Change direction periodically
      dy = -dy;
      patternTimer = 0;
    }

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

  void circularMove()
  {
    angle += 0.05f;
    if (angle > 6.28f)
      angle = 0; // Reset after 2π

    // Store previous position
    int prevX = x;
    int prevY = y;

    // Calculate new position based on circular pattern
    int radius = 30;
    x += (dx / 2) + radius * cos(angle) * 0.2f;
    y += (dy / 2) + radius * sin(angle) * 0.2f;

    // Boundary collision
    if (grid[y / ts][x / ts] == 1)
    {
      // If collision, revert to previous position and change direction
      x = prevX;
      y = prevY;
      dx = -dx;
      dy = -dy;
    }
  }

  void move()
  {
    // if (powerUpTimer.getElapsedTime().asSeconds() < 3)
    // {
    //   return; // Don't move during power-up
    // }

    switch (movementPattern)
    {
    case 0: // Linear
      linearMove();
      break;
    case 1: // Zigzag
      zigzagMove();
      break;
    case 2: // Circular
      circularMove();
      break;
    default:
      linearMove();
    }
  }
};

// Array for enemies
Enemy enemies[10];
int enemyCount = 4; // Default medium difficulty

// System clocks
Clock gameClock;
Clock powerUpTimer;
Clock enemySpeedTimer;
Clock enemyPatternTimer;
Clock enemySpawnTimer;

// Game UI
sf::Font font;
sf::Text scoreText, scoreText2;
sf::Text highScoreText, highScoreText2;
sf::Text timerText;
sf::Text moveCounterText, moveCounterText2;
sf::Text powerUpText, powerUpText2;
sf::Text gameOverText;
sf::Text instructionText;

float enemySpeedMultiplier = 1.0f;

// Function prototypes
void loadHighScores();
void saveHighScores();
// void addHighScore(int score, int time);
void drop(int y, int x);
void updateScore();
void checkPowerUps();
void updateScoreDisplay();
void increaseEnemySpeed();
void changeEnemyPatterns();
void drawMainMenu(RenderWindow &window);
void drawLevelSelect(RenderWindow &window);
void drawScoreboard(RenderWindow &window);
void drawGameOver(RenderWindow &window, int finalScore, int gameTime);
void resetGame();
void spawnEnemies();

// Load the top 5 high scores from file
void loadHighScores()
{
  std::ifstream file("scoreboard.txt");
  if (file.is_open())
  {
    for (int i = 0; i < 5; i++)
    {
      if (!(file >> highScores[i] >> highScoreTimes[i] >> highScoreNames[i]))
      {
        break;
      }
    }
    file.close();
  }
}

// Save the top 5 high scores to file
void saveHighScores()
{
  std::ofstream file("scoreboard.txt");
  if (file.is_open())
  {
    for (int i = 0; i < 5; i++)
    {
      file << highScores[i] << " " << highScoreTimes[i] << " " << highScoreNames[i] << std::endl;
    }
    file.close();
  }
}

// // Add a new high score if it qualifies
// void addHighScore(int score, int time)
// {
//   // Check if the score qualifies for the top 5
//   for (int i = 0; i < 5; i++)
//   {
//     if (score > highScores[i])
//     {
//       // Shift lower scores down
//       for (int j = 4; j > i; j--)
//       {
//         highScores[j] = highScores[j - 1];
//         highScoreTimes[j] = highScoreTimes[j - 1];
//         highScoreNames[j] = highScoreNames[j - 1];
//       }
//       // Insert new score
//       highScores[i] = score;
//       highScoreTimes[i] = time;
//       highScoreNames[i] = "Player"; // Default name
//       saveHighScores();
//       break;
//     }
//   }
// }

// Recursively fill empty areas (flood fill)
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

// Update score based on captured tiles and apply bonuses
void updateScore()
{
  int points = tilesCapturedInMove; // Base points equals tiles captured

  // Apply multipliers based on bonus conditions
  if (bonusCounter >= 5 && tilesCapturedInMove > 5)
  {
    points *= 4; // 4x bonus
  }
  else if (bonusCounter >= 3 && tilesCapturedInMove > 5)
  {
    points *= 2; // 2x bonus
  }
  else if (tilesCapturedInMove > 10)
  {
    points *= 2; // 2x bonus
  }

  // Add points to score
  // score += points;

  // Update bonus counter if bonus was achieved
  if (tilesCapturedInMove > 10 || (bonusCounter >= 3 && tilesCapturedInMove > 5))
  {
    bonusCounter++;
  }

  // Reset tiles captured for next move
  tilesCapturedInMove = 0;

  int points2 = tilesCapturedInMove2; // Base points equals tiles captured

  // Apply multipliers based on bonus conditions
  if (bonusCounter2 >= 5 && tilesCapturedInMove2 > 5)
  {
    points2 *= 4; // 4x bonus
  }
  else if (bonusCounter2 >= 3 && tilesCapturedInMove2 > 5)
  {
    points2 *= 2; // 2x bonus
  }
  else if (tilesCapturedInMove2 > 10)
  {
    points2 *= 2; // 2x bonus
  }

  // Add points to score
  // score2 += points2;

  // Update bonus counter if bonus was achieved
  if (tilesCapturedInMove2 > 10 || (bonusCounter2 >= 3 && tilesCapturedInMove2 > 5))
  {
    bonusCounter2++;
  }

  // Reset tiles captured for next move
  tilesCapturedInMove2 = 0;

  // Check for power-up rewards
  checkPowerUps();

  // Update score display
  updateScoreDisplay();
}

// Check if player earned power-ups based on score
void checkPowerUps()
{
  static int target1 = 50;
  if (score >= target1 && target1 == 50)
  {
    powerUps++;
    target1 = 70;
  }
  else if (score >= target1)
  {
    powerUps++;
    target1 += 30;
  }

  static int target2 = 50;
  if (score2 >= target2 && target2 == 50)
  {
    powerUps2++;
    target2 = 70;
  }
  else if (score2 >= target2)
  {
    powerUps2++;
    target2 += 30;
  }

  // player 2
}

// Activate power-up to stop enemies
// void activatePowerUp()
// {
//   if (powerUps > 0 && !powerUpActive)
//   {
//     powerUps--;
//     powerUpActive = true;
//     powerUpTimer.restart();
//   }
// }

// // Update power-up status
// void updatePowerUp()
// {
//   if (powerUpActive && powerUpTimer.getElapsedTime().asSeconds() >= 3)
//   {
//     powerUpActive = false;
//   }
// }

// Update score display text
void updateScoreDisplay()
{
  scoreText.setString("P1 Score: " + std::to_string(score));
  powerUpText.setString("P1 Power-ups: " + std::to_string(powerUps));
  moveCounterText.setString("P1 Moves: " + std::to_string(moveCounter));

  scoreText2.setString("P2 Score: " + std::to_string(score2));
  powerUpText2.setString("P2 Power-ups: " + std::to_string(powerUps2));
  moveCounterText2.setString("P2 Moves: " + std::to_string(moveCounter2));
}

// Increase enemy speed every 20 seconds
void increaseEnemySpeed()
{
  if (enemySpeedTimer.getElapsedTime().asSeconds() >= 20)
  {
    enemySpeedMultiplier += 0.2f;
    enemySpeedTimer.restart();
  }
}

// Change enemy movement patterns after 30 seconds
void changeEnemyPatterns()
{
  // bool initial=true;
  if (initial || enemyPatternTimer.getElapsedTime().asSeconds() >= 30)
  {

    // initial = false;
    patternMovement = true;

    // Apply pattern movement to half of the enemies
    for (int i = 0; i < enemyCount; i += 2)
    {
      // Alternate between zigzag and circular patterns
      enemies[i].movementPattern = rand() % 3;
    }
    enemyPatternTimer.restart();
    cout << " ===\n";
  }
}

// Draw main menu
void drawMainMenu(RenderWindow &window)
{
  Text title, option1, option2, option3;

  title.setFont(font);
  title.setString("XONIX GAME");
  title.setCharacterSize(50);
  title.setFillColor(Color::White);
  title.setPosition(N * ts / 2 - 100, 50);

  option1.setFont(font);
  option1.setString("1. Start Game");
  option1.setCharacterSize(30);
  option1.setFillColor(Color::Green);
  option1.setPosition(N * ts / 2 - 80, 150);

  // option2.setFont(font);
  // option2.setString("2. Select Level");
  // option2.setCharacterSize(30);
  // option2.setFillColor(Color::Yellow);
  // option2.setPosition(N * ts / 2 - 80, 200);

  option3.setFont(font);
  option3.setString("2. Scoreboard");
  option3.setCharacterSize(30);
  option3.setFillColor(Color::Cyan);
  option3.setPosition(N * ts / 2 - 80, 250);

  window.draw(title);
  window.draw(option1);
  window.draw(option2);
  window.draw(option3);
}

// Draw level select menu
void drawLevelSelect(RenderWindow &window)
{
  Text title, option1, option2, option3, option4;

  title.setFont(font);
  title.setString("Select Difficulty");
  title.setCharacterSize(40);
  title.setFillColor(Color::White);
  title.setPosition(N * ts / 2 - 120, 50);

  option1.setFont(font);
  option1.setString("1. Easy (2 enemies)");
  option1.setCharacterSize(30);
  option1.setFillColor(Color::Green);
  option1.setPosition(N * ts / 2 - 100, 150);

  option2.setFont(font);
  option2.setString("2. Medium (4 enemies)");
  option2.setCharacterSize(30);
  option2.setFillColor(Color::Yellow);
  option2.setPosition(N * ts / 2 - 100, 200);

  option3.setFont(font);
  option3.setString("3. Hard (6 enemies)");
  option3.setCharacterSize(30);
  option3.setFillColor(Color::Red);
  option3.setPosition(N * ts / 2 - 100, 250);

  option4.setFont(font);
  option4.setString("4. Continuous Mode");
  option4.setCharacterSize(30);
  option4.setFillColor(Color::Magenta);
  option4.setPosition(N * ts / 2 - 100, 300);

  window.draw(title);
  window.draw(option1);
  window.draw(option2);
  window.draw(option3);
  window.draw(option4);
}

void drawPlayerSelect(RenderWindow &window)
{
  Text title, option1, option2, option3, option4;

  title.setFont(font);
  title.setString("Select Player");
  title.setCharacterSize(40);
  title.setFillColor(Color::White);
  title.setPosition(N * ts / 2 - 120, 50);

  option2.setFont(font);
  option2.setString("1. 1 Player");
  option2.setCharacterSize(30);
  option2.setFillColor(Color::Yellow);
  option2.setPosition(N * ts / 2 - 100, 200);

  option3.setFont(font);
  option3.setString("2. 2 Player ");
  option3.setCharacterSize(30);
  option3.setFillColor(Color::Red);
  option3.setPosition(N * ts / 2 - 100, 250);

  window.draw(title);
  window.draw(option1);
  window.draw(option2);
  window.draw(option3);
  window.draw(option4);
}

// Draw scoreboard
void drawScoreboard(RenderWindow &window)
{
  Text title, backOption;
  Text scores[5];

  title.setFont(font);
  title.setString("HIGH SCORES");
  title.setCharacterSize(40);
  title.setFillColor(Color::White);
  title.setPosition(N * ts / 2 - 100, 50);

  for (int i = 0; i < 5; i++)
  {
    int minutes = highScoreTimes[i] / 60;
    int seconds = highScoreTimes[i] % 60;

    scores[i].setFont(font);
    scores[i].setString(std::to_string(i + 1) + ". " + highScoreNames[i] +
                        " - " + std::to_string(highScores[i]) + " points - " +
                        std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds));
    scores[i].setCharacterSize(24);
    scores[i].setFillColor(Color::Yellow);
    scores[i].setPosition(N * ts / 2 - 150, 120 + i * 40);
  }

  backOption.setFont(font);
  backOption.setString("Press ESC to return to main menu");
  backOption.setCharacterSize(20);
  backOption.setFillColor(Color::White);
  backOption.setPosition(N * ts / 2 - 130, M * ts - 50);

  window.draw(title);
  for (int i = 0; i < 5; i++)
  {
    window.draw(scores[i]);
  }
  window.draw(backOption);
}

// Draw game over screen
void drawGameOver(RenderWindow &window, int finalScore, int gameTime)
{
  Text gameOverTitle, scoreDisplay, timeDisplay, instruction;

  gameOverTitle.setFont(font);
  gameOverTitle.setString("GAME OVER");
  gameOverTitle.setCharacterSize(50);
  gameOverTitle.setFillColor(Color::Red);
  gameOverTitle.setPosition(N * ts / 2 - 120, 100);
  window.draw(gameOverTitle);

  scoreDisplay.setFont(font);
  scoreDisplay.setString("Player 1 Score: " + std::to_string(score));
  scoreDisplay.setCharacterSize(30);
  scoreDisplay.setFillColor(Color::White);
  scoreDisplay.setPosition(N * ts / 2 - 100, 200);
  window.draw(scoreDisplay);

  if (player2include)
  {
    scoreDisplay.setFont(font);
    scoreDisplay.setString("Player 2 Score: " + std::to_string(score2));
    scoreDisplay.setCharacterSize(30);
    scoreDisplay.setFillColor(Color::White);
    scoreDisplay.setPosition(N * ts / 2 - 100, 230);
    window.draw(scoreDisplay);
  }
  int minutes = gameTime / 60;
  int seconds = gameTime % 60;
  timeDisplay.setFont(font);
  timeDisplay.setString("Time: " + std::to_string(minutes) + ":" +
                        (seconds < 10 ? "0" : "") + std::to_string(seconds));
  timeDisplay.setCharacterSize(30);
  timeDisplay.setFillColor(Color::White);
  timeDisplay.setPosition(N * ts / 2 - 100, 270);
  window.draw(timeDisplay);

  instruction.setFont(font);
  instruction.setString("Press R to Restart, M for Main Menu, ESC to Exit");
  instruction.setCharacterSize(20);
  instruction.setFillColor(Color::Yellow);
  instruction.setPosition(N * ts / 2 - 180, 350);
  window.draw(instruction);

  // Check if score is a new high score

  if (score > previousHighScore || score2 > previousHighScore)
  {
    Text newHighScore;
    newHighScore.setFont(font);
    newHighScore.setString("NEW HIGH SCORE = " + to_string(max(score, score2)));
    newHighScore.setCharacterSize(35);
    newHighScore.setFillColor(Color::Green);
    newHighScore.setPosition(N * ts / 2 - 120, 150);
    window.draw(newHighScore);
  }
}

// Reset the game to initial state
void resetGame()
{
  // Clear the grid except for borders
  for (int i = 1; i < M - 1; i++)
  {
    for (int j = 1; j < N - 1; j++)
    {
      grid[i][j] = 0;
    }
  }

  // Reset game variables
  initial = true;
  score = score2 = 0;
  tilesCapturedInMove2 = tilesCapturedInMove = 0;
  bonusCounter2 = bonusCounter = 0;
  powerUps2 = powerUps = 0;

  moveCounter2 = moveCounter = 0;
  moveInProgress2 = moveInProgress = false;

  enemySpeedMultiplier = 1.0f;
  patternMovement = false;

  P1 = {N - 1, M - 1, 0, 0, 0, 0};
  P2 = {0, 0, 0, 0, 0, 0};

  // Reset timers
  gameClock.restart();
  enemySpeedTimer.restart();
  enemyPatternTimer.restart();
  enemySpawnTimer.restart();

  // Reset enemy movement patterns
  for (int i = 0; i < 10; i++)
  {
    enemies[i] = Enemy();
  }

  // Update display
  updateScoreDisplay();
}

// Spawn additional enemies in continuous mode
void spawnEnemies()
{
  if (continuousMode && enemySpawnTimer.getElapsedTime().asSeconds() >= 20 && enemyCount < 10)
  {
    // Add 2 more enemies every 20 seconds
    enemies[enemyCount] = Enemy();
    enemyCount++;

    if (enemyCount < 10)
    {
      enemies[enemyCount] = Enemy();
      enemyCount++;
    }

    enemySpawnTimer.restart();
  }
}

int main()
{
  // Initialize random seed
  srand(time(0));

  // Create game window
  RenderWindow window(VideoMode(N * ts, M * ts), "Xonix Game!");
  window.setFramerateLimit(60);

  // Load font
  if (!font.loadFromFile("fonts/KnightWarrior.otf"))
  {
    std::cout << "Error loading font!\n";
    return -1;
  }

  // Initialize UI text elements
  scoreText.setFont(font);
  scoreText.setCharacterSize(18);
  scoreText.setFillColor(Color::White);
  scoreText.setPosition(N * ts - 150, 10);

  moveCounterText.setFont(font);
  moveCounterText.setCharacterSize(18);
  moveCounterText.setFillColor(Color::White);
  moveCounterText.setPosition(N * ts - 150, 30);

  powerUpText.setFont(font);
  powerUpText.setCharacterSize(18);
  powerUpText.setFillColor(Color::Cyan);
  powerUpText.setPosition(N * ts - 150, 50);

  ///  player 2
  scoreText2.setFont(font);
  scoreText2.setCharacterSize(18);
  scoreText2.setFillColor(Color::White);
  scoreText2.setPosition(10, 10);

  moveCounterText2.setFont(font);
  moveCounterText2.setCharacterSize(18);
  moveCounterText2.setFillColor(Color::White);
  moveCounterText2.setPosition(10, 30);

  powerUpText2.setFont(font);
  powerUpText2.setCharacterSize(18);
  powerUpText2.setFillColor(Color::Cyan);
  powerUpText2.setPosition(10, 50);

  // Other
  highScoreText.setFont(font);
  highScoreText.setCharacterSize(18);
  highScoreText.setFillColor(Color::Yellow);
  highScoreText.setPosition(N * ts / 2, 30);

  timerText.setFont(font);
  timerText.setCharacterSize(18);
  timerText.setFillColor(Color::White);
  timerText.setPosition(N * ts / 2, 10);

  instructionText.setFont(font);
  instructionText.setCharacterSize(16);
  instructionText.setFillColor(Color::Black);
  instructionText.setPosition(N * ts / 2 - 150, M * ts - 20);
  instructionText.setString("Press R_SHIFT & L_SHIFT to use power-up, ESC to reset");

  // Load textures
  Texture tileTex, gameoverTex, enemyTex;
  if (!tileTex.loadFromFile("images/tiles.png") ||
      !gameoverTex.loadFromFile("images/gameover.png") ||
      !enemyTex.loadFromFile("images/enemy.png"))
  {
    std::cout << "Error loading textures!\n";
    return -1;
  }

  // Create sprites
  Sprite sTile(tileTex), sGameover(gameoverTex), sEnemy(enemyTex);
  sGameover.setPosition(100, 100);
  sEnemy.setOrigin(20, 20);

  // Initialize border tiles
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      if (i == 0 || j == 0 || i == M - 1 || j == N - 1)
      {
        grid[i][j] = 1;
      }
    }
  }

  // Player position and movement

  // int x = 10, y = 0;
  // int dx = 0, dy = 0;
  // int lastDx = 0, lastDy = 0;

  // Game timing
  float timer = 0, delay = 0.07;
  Clock clock;

  // Load high scores
  loadHighScores();
  previousHighScore = highScores[0];

  // Set initial game state
  currentState = 1;

  // Main game loop
  while (window.isOpen())
  {
    float time = clock.getElapsedTime().asSeconds();
    clock.restart();
    timer += time;

    // Handle events
    Event e;
    while (window.pollEvent(e))
    {
      if (e.type == Event::Closed)
      {
        window.close();
      }

      if (e.type == Event::KeyPressed)
      {
        // Handle key presses based on current game state
        switch (currentState)
        {
        case 1:
          if (e.key.code == Keyboard::Num1)
          {
            // Start game with current difficulty
            currentState = 3;
            resetGame();
          }
          // else if (e.key.code == Keyboard::Num2)
          // {
          //   // Go to level select
          //   currentState = LEVEL_SELECT;
          // }
          else if (e.key.code == Keyboard::Num2)
          {
            // Show scoreboard
            currentState = 6;
          }
          else if (e.key.code == Keyboard::Escape)
          {
            window.close();
          }
          break;
        case 3:
          if (e.key.code == Keyboard::Num1)
          {
            // Start game with current difficulty
            currentState = 2;
            player2include = false;
            resetGame();
          }
          // else if (e.key.code == Keyboard::Num2)
          // {
          //   // Go to level select
          //   currentState = LEVEL_SELECT;
          // }
          else if (e.key.code == Keyboard::Num2)
          {
            // Show scoreboard
            currentState = 2;
            player2include = true;
          }
          else if (e.key.code == Keyboard::Escape)
          {
            window.close();
          }
          break;
        case 2:
          if (e.key.code == Keyboard::Num1)
          {
            // Easy difficulty
            difficultyLevel = 2;
            enemyCount = 2;
            continuousMode = false;
            currentState = 4;
            resetGame();
          }
          else if (e.key.code == Keyboard::Num2)
          {
            // Medium difficulty
            difficultyLevel = 4;
            enemyCount = 4;
            continuousMode = false;
            currentState = 4;
            resetGame();
          }
          else if (e.key.code == Keyboard::Num3)
          {
            // Hard difficulty
            difficultyLevel = 6;
            enemyCount = 6;
            continuousMode = false;
            currentState = 4;
            resetGame();
          }
          else if (e.key.code == Keyboard::Num4)
          {
            // Continuous mode
            difficultyLevel = 7;
            enemyCount = 2; // Start with 2 enemies
            continuousMode = true;
            currentState = 4;
            resetGame();
          }
          else if (e.key.code == Keyboard::Escape)
          {
            currentState = 1;
          }
          break;

        case 6:
          if (e.key.code == Keyboard::Escape)
          {
            currentState = 1;
          }
          break;

        case 4:
          if (e.key.code == Keyboard::Escape)
          {
            resetGame();
          }
          else if (e.key.code == Keyboard::RShift)
          {
            // activatePowerUp();
            if (powerUps > 0 && powerUpTimer.getElapsedTime().asSeconds() >= 3)
            {
              powerUps--;
              powerUpTimer.restart();
            }
          }
          else if (e.key.code == Keyboard::LShift)
          {
            if (powerUps2 > 0 && powerUpTimer.getElapsedTime().asSeconds() >= 3)
            {
              powerUps2--;
              powerUpTimer.restart();
            }
          }
          break;

        case 5:
          if (e.key.code == Keyboard::R)
          {
            // Restart game with same settings
            currentState = 4;
            // P1 = {N - 1, M - 1, 0, 0, 0, 0};
            // P2 = {0, 0, 0, 0, 0, 0};
            resetGame();
          }
          else if (e.key.code == Keyboard::M)
          {
            // Return to main menu
            currentState = 1;
          }
          else if (e.key.code == Keyboard::Escape)
          {
            window.close();
          }
          break;
        }
      }
    }

    // State-specific logic
    switch (currentState)
    {
    case 4:
      // Process player input
      if (Keyboard::isKeyPressed(Keyboard::Left))
      {
        if (!(P1.last_dx == 1 && grid[P1.y][P1.x] == 2))
        {
          P1.dx = -1;
          P1.dy = 0;
        }
      }
      if (Keyboard::isKeyPressed(Keyboard::Right))
      {
        if (!(P1.last_dx == -1 && grid[P1.y][P1.x] == 2))
        {
          P1.dx = 1;
          P1.dy = 0;
        }
      }
      if (Keyboard::isKeyPressed(Keyboard::Up))
      {
        if (!(P1.last_dy == 1 && grid[P1.y][P1.x] == 2))
        {
          P1.dx = 0;
          P1.dy = -1;
        }
      }
      if (Keyboard::isKeyPressed(Keyboard::Down))
      {
        if (!(P1.last_dy == -1 && grid[P1.y][P1.x] == 2))
        {
          P1.dx = 0;
          P1.dy = 1;
        }
      }

      if (player2include)
      {

        if (Keyboard::isKeyPressed(Keyboard::A))
        {
          if (!(P2.last_dx == 1 && grid[P2.y][P2.x] == 2))
          {
            P2.dx = -1;
            P2.dy = 0;
          }
        }
        if (Keyboard::isKeyPressed(Keyboard::D))
        {
          if (!(P2.last_dx == -1 && grid[P2.y][P2.x] == 2))
          {
            P2.dx = 1;
            P2.dy = 0;
          }
        }
        if (Keyboard::isKeyPressed(Keyboard::W))
        {
          if (!(P2.last_dy == 1 && grid[P2.y][P2.x] == 2))
          {
            P2.dx = 0;
            P2.dy = -1;
          }
        }
        if (Keyboard::isKeyPressed(Keyboard::S))
        {
          if (!(P2.last_dy == -1 && grid[P2.y][P2.x] == 2))
          {
            P2.dx = 0;
            P2.dy = 1;
          }
        }
      }

      // Move player at regular intervals
      if (timer > delay)
      {
        // Check if this is a new move
        if ((P1.dx != 0 || P1.dy != 0) && !moveInProgress && grid[P1.y][P1.x] == 1)
        {
          moveInProgress = true;
          // moveCounter++;
          updateScoreDisplay();
        }
        P1.last_dx = P1.dx;
        P1.last_dy = P1.dy;

        // Update player position
        P1.x += P1.dx;
        P1.y += P1.dy;

        // Keep player within bounds
        if (P1.x < 0)
          P1.x = 0;
        if (P1.x > N - 1)
          P1.x = N - 1;
        if (P1.y < 0)
          P1.y = 0;
        if (P1.y > M - 1)
          P1.y = M - 1;

        if (grid[P1.y][P1.x] == 2 || grid[P1.y][P1.x] == 3)
        {
          currentState = 5;
          int gameTime = static_cast<int>(gameClock.getElapsedTime().asSeconds());
          // addHighScore(score, gameTime);
        }

        // Mark player trail
        if (grid[P1.y][P1.x] == 0)
        {
          grid[P1.y][P1.x] = 2;
          tilesCapturedInMove++;
        }

        if (player2include)
        {
          if ((P2.dx != 0 || P2.dy != 0) && !moveInProgress2 && grid[P2.y][P2.x] == 1)
          {
            moveInProgress2 = true;
            // moveCounter2++;
            updateScoreDisplay();
          }
          // Store last direction for no-reverse check

          P2.last_dx = P2.dx;
          P2.last_dy = P2.dy;

          P2.x += P2.dx;
          P2.y += P2.dy;

          if (P2.x < 0)
            P2.x = 0;
          if (P2.x > N - 1)
            P2.x = N - 1;
          if (P2.y < 0)
            P2.y = 0;
          if (P2.y > M - 1)
            P2.y = M - 1;

          // Check for collision with player trail

          if (grid[P2.y][P2.x] == 2 || grid[P2.y][P2.x] == 3)
          {
            currentState = 5;
            int gameTime = static_cast<int>(gameClock.getElapsedTime().asSeconds());
            // addHighScore(score2, gameTime);
          }
          if (grid[P2.y][P2.x] == 0)
          {
            grid[P2.y][P2.x] = 3;
            tilesCapturedInMove2++;
          }
        }

        // Reset timer
        timer = 0;
      }

      // Check if player returned to border
      if (grid[P1.y][P1.x] == 1 && moveInProgress)
      {
        moveInProgress = false;
        updateScore();
        P1.dx = P1.dy = 0;

        // Flood fill from each enemy position
        for (int i = 0; i < enemyCount; i++)
        {
          drop(enemies[i].y / ts, enemies[i].x / ts);
        }

        bool inc = false;
        // Convert grid: -1 (flood filled) -> 0, others -> 1
        for (int i = 0; i < M; i++)
        {
          for (int j = 0; j < N; j++)
          {
            if (grid[i][j] == -1)
            {
              grid[i][j] = 0;
            }
            else if (grid[i][j] == 0 || grid[i][j] == 2)
            {
              grid[i][j] = 1;
              tilesCapturedInMove++;
              cout << "+";
              score++;
              checkPowerUps();
              inc = true;
            }
          }
        }
        if (inc)
        {
          moveCounter++;
          cout << endl;
        }
      }

      if (player2include)
      {
        if (grid[P2.y][P2.x] == 1 && moveInProgress2)
        {
          moveInProgress2 = false;
          updateScore();
          P2.dx = P2.dy = 0;

          // Flood fill from each enemy position
          for (int i = 0; i < enemyCount; i++)
          {
            drop(enemies[i].y / ts, enemies[i].x / ts);
          }

          bool inc = false;
          // Convert grid: -1 (flood filled) -> 0, others -> 1
          for (int i = 0; i < M; i++)
          {
            for (int j = 0; j < N; j++)
            {
              if (grid[i][j] == -1)
              {
                grid[i][j] = 0;
              }
              else if (grid[i][j] == 0 || grid[i][j] == 3)
              {
                grid[i][j] = 1;
                tilesCapturedInMove2++;
                cout << "+";
                score2++;
                checkPowerUps();
                inc = true;
              }
            }
          }
          if (inc)
          {
            moveCounter2++;
            cout << endl;
          }
        }
      }
      // Update enemies

      for (int i = 0; i < enemyCount; i++)
      {
        if (initial || powerUpTimer.getElapsedTime().asSeconds() >= 3)
        {
          enemies[i].move();
        }

        if (powerUpTimer.getElapsedTime().asSeconds() >= 3)
          initial = false;
        // Check if enemy hits player trail
        if (grid[enemies[i].y / ts][enemies[i].x / ts] == 2 || grid[enemies[i].y / ts][enemies[i].x / ts] == 3)
        {
          currentState = 5;
          int gameTime = static_cast<int>(gameClock.getElapsedTime().asSeconds());
          // addHighScore(score, gameTime);
          // addHighScore(score2, gameTime);
        }
      }

      // Update power-up status

      // Increase enemy speed every 20 seconds
      increaseEnemySpeed();

      // Change enemy patterns after 30 seconds
      changeEnemyPatterns();

      // Spawn additional enemies in continuous mode
      spawnEnemies();

      break;

    default:

      break;
    }

    // Clear window
    window.clear();

    // Draw based on current state
    switch (currentState)
    {
    case 1:
      drawMainMenu(window);
      break;

    case 3:
      drawPlayerSelect(window);
      break;
    case 2:
      drawLevelSelect(window);
      break;

    case 6:
      drawScoreboard(window);
      break;

    case 5:
      drawGameOver(window, score, static_cast<int>(gameClock.getElapsedTime().asSeconds()));
      break;

    case 4:
      // Draw grid
      for (int i = 0; i < M; i++)
      {
        for (int j = 0; j < N; j++)
        {
          if (grid[i][j] == 0)
            continue;
          if (grid[i][j] == 1)
            sTile.setTextureRect(IntRect(0, 0, ts, ts));
          if (grid[i][j] == 2 || grid[i][j] == 3)
            sTile.setTextureRect(IntRect(ts, 0, ts, ts));
          sTile.setPosition(j * ts, i * ts);
          window.draw(sTile);
        }
      }

      // Draw player
      sTile.setTextureRect(IntRect(ts * 2, 0, ts, ts));
      sTile.setPosition(P1.x * ts, P1.y * ts);
      window.draw(sTile);

      if (player2include)
      {
        sTile.setPosition(P2.x * ts, P2.y * ts);
        window.draw(sTile);
      }
      sEnemy.rotate(10);
      // Draw enemies
      for (int i = 0; i < enemyCount; i++)
      {
        sEnemy.setPosition(enemies[i].x, enemies[i].y);

        // Change enemy color based on movement pattern
        if (enemies[i].movementPattern == 1)
        {
          sEnemy.setColor(Color::Yellow); // Yellow for zigzag
        }
        else if (enemies[i].movementPattern == 2)
        {
          sEnemy.setColor(Color::Magenta); // Magenta for circular
        }
        else
        {
          sEnemy.setColor(Color::Red); // Red for linear
        }

        window.draw(sEnemy);
      }

      {
        // updating highscore
        int Highscore2[7];
        int Highscoretime2[7];

        for (int i = 0; i < 5; i++)
        {
          Highscore2[i] = highScores[i];
          Highscoretime2[i] = highScoreTimes[i];
        }

        Highscore2[5] = score;
        Highscoretime2[5] = Highscoretime2[6] = gameClock.getElapsedTime().asSeconds();
        Highscore2[6] = score2;

        for (int i = 0; i < 7; i++)
        {
          for (int j = i + 1; j < 7; j++)
          {
            if (Highscore2[i] < Highscore2[j])
            {
              swap(Highscore2[i], Highscore2[j]);
              swap(Highscoretime2[i], Highscoretime2[j]);
            }
          }
        }

        for (int i = 0; i < 5; i++)
        {

          highScores[i] = Highscore2[i];
          highScoreTimes[i] = Highscoretime2[i];
        }
      }
      // Draw UI elements
      // Update timer display
      int gameTimeSeconds = static_cast<int>(gameClock.getElapsedTime().asSeconds());
      int minutes = gameTimeSeconds / 60;
      int seconds = gameTimeSeconds % 60;
      timerText.setString("Time: " + std::to_string(minutes) + ":" +
                          (seconds < 10 ? "0" : "") + std::to_string(seconds));

      // Update high score display
      highScoreText.setString("High Score: " + std::to_string(max(highScores[0], max(score, score2))));

      updateScoreDisplay();
      // Draw UI
      window.draw(scoreText);
      window.draw(moveCounterText);
      window.draw(powerUpText);

      if (player2include)
      {
        window.draw(scoreText2);
        window.draw(moveCounterText2);
        window.draw(powerUpText2);
      }
      highScoreText.setPosition(N * ts / 2 - highScoreText.getGlobalBounds().width / 2, 30);
      window.draw(highScoreText);
      timerText.setPosition(N * ts / 2 - timerText.getGlobalBounds().width / 2, 10);
      window.draw(timerText);
      window.draw(instructionText);
      break;
    }

    // Display the window
    window.display();
  }

  return 0;
}