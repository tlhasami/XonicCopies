#include <SFML/Graphics.hpp>
#include <time.h>
#include <iostream>
using namespace std;
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
    int score = 0;
    bool move = false;
    
    int Movecount = 0;
    bool inMenu = true;
    bool inDifficulty = false;
    bool continuousmode = false;
    bool selectlayermode = false;
    bool inscore = false;
    // poweer ky liay

    int powerUps = 0;
    int nextPowerUpScore = 50;
    bool enemiesPaused = false;
    float pauseStartTime = 0;
    bool enemyFrozen = false;
    float freezeStartTime = 0;
    float freezeDuration = 3.0f; // 3 seconds

    // end
    srand(time(0));
    Clock enemyAddClock;
    RenderWindow window(VideoMode(N * ts, M * ts), "Xonix Game!");
    window.setFramerateLimit(60);

    // Load font
    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"))
    {
        std::cerr << "Error loading font" << std::endl;
        return -1;
    }

    // score
    Text scoreTextDisplay;
    scoreTextDisplay.setFont(font);
    scoreTextDisplay.setString("Score =");
    scoreTextDisplay.setCharacterSize(20);
    scoreTextDisplay.setFillColor(Color::Cyan);
    scoreTextDisplay.setPosition(200, 40);
    // power up
    Text powerupDisplay;
    powerupDisplay.setFont(font);
    powerupDisplay.setString("Score =");
    powerupDisplay.setCharacterSize(20);
    powerupDisplay.setFillColor(Color::Cyan);
    powerupDisplay.setPosition(200, 80);
    // Create menu text
    sf::Text text1("AHMAD PROJECT", font, 30);
    sf::Text text2("Press 1 :: Start Game", font, 20);
    sf::Text text3("Press 2 :: Select Level", font, 20);
    sf::Text text4("Press 3 :: Show Scoreboard", font, 20);
    // for dificulty level
    sf::Text text5("Press 4 :: Easy Level", font, 20);
    sf::Text text6("Press 5 :: Medium Level", font, 20);
    sf::Text text7("Press 6 :: Hard Level", font, 20);
    sf::Text text8("Press 7 :: Continuous Mode", font, 20);
    sf::Text text9("Move count=", font, 15);
    sf::Text text10("Score=", font, 15);
    sf::Text text11("Power UP=", font, 15);
    sf::Text textComingSoon("Coming Soon!", font, 20); // Font and size for "Coming Soon"
    textComingSoon.setFillColor(sf::Color::Red);       // Set the color to red
    textComingSoon.setPosition(100, 300);              // Set its position
    sf::Text text12("Press esc to go menue", font, 30);
    // Set positions for the text
    text2.setPosition(100, 150); // Position for Start Game
    text3.setPosition(100, 200); // Position for Select Level
    text4.setPosition(100, 250); // Position for Show Scoreboard
    text1.setPosition(100, 100);
    // difficulty level
    text5.setPosition(100, 100);
    text6.setPosition(100, 150);
    text7.setPosition(100, 200);
    text8.setPosition(100, 250);
    // score step
    text10.setPosition(100, 100);
    text11.setPosition(100, 150);
    // counter to check move
    text9.setPosition(300, 20);
    text12.setPosition(100, 250);
    // Set text color
    text1.setFillColor(sf::Color::Black);
    text2.setFillColor(sf::Color::Blue);
    text3.setFillColor(sf::Color::Red);
    text4.setFillColor(sf::Color::Magenta);
    text5.setFillColor(sf::Color::Green);
    text6.setFillColor(sf::Color::Yellow);
    text7.setFillColor(sf::Color::Red);
    text8.setFillColor(sf::Color::Blue);
    text9.setFillColor(sf::Color::Green);
    text10.setFillColor(sf::Color::Green);
    text11.setFillColor(sf::Color::Green);
    text12.setFillColor(sf::Color::Green);
    // power ky liay

    sf::Text onePlayerText("Press O for One Player", font, 30);
    onePlayerText.setFillColor(sf::Color::Red);
    onePlayerText.setPosition(150, 200);

    sf::Text twoPlayerText("Press T for Two Player", font, 30);
    twoPlayerText.setFillColor(sf::Color::Magenta);
    twoPlayerText.setPosition(150, 250);

    Texture t1, t2, t3, t4, t5, t6;
    t1.loadFromFile("images/menuepic.png");
    t2.loadFromFile("images/tiles.png");
    t3.loadFromFile("images/gameover.png");
    t4.loadFromFile("images/enemy.png");
    t5.loadFromFile("images/difficulty.png");
    t6.loadFromFile("images/score.png");
    Sprite sMenue(t1), sTile(t2), sGameover(t3), sEnemy(t4), sDifficulty(t5), sScore(t6);
    sGameover.setPosition(100, 100);
    sEnemy.setOrigin(20, 20);
    sDifficulty.setScale(3.f, 3.f);
    sScore.setScale(3.f, 3.f);
    sScore.setPosition(N * ts / 2, M * ts / 2);

    int enemyCount = 4;
    Enemy a[20];

    int playmode = 1;
    bool Game = true;
    int x = 0, y = 0, dx = 0, dy = 0;
    float timer = 0, delay = 0.07;
    Clock clock;
    Clock freezeClock; // add this at the top

    // player 2 variables
    int x2 = N - 1, y2 = M - 1, dx2 = 0, dy2 = 0;

    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            if (i == 0 || j == 0 || i == M - 1 || j == N - 1)
                grid[i][j] = 1;

    int nextPowerScore = 50;

    int tiles_collected = 0;
    while (window.isOpen())
    {

        if (x == x2 && y == y2)
        {
            cout << "Game" << endl;
            Game = false;
        }

        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        // score end
        timer += time;

        Event e;

        while (window.pollEvent(e))
        {

            if (e.type == Event::Closed)
                window.close();

            if (e.type == Event::KeyPressed)
            {
                if (e.key.code == Keyboard::P && powerUps > 0 && !enemyFrozen)
                {
                    enemyFrozen = true;
                    freezeClock.restart(); // <-- fixed: use separate clock
                    powerUps--;
                    cout << "Powerup activated" << endl;
                }
            }

            if (inMenu)
            {
                if (e.key.code == Keyboard::Num1)
                {
                    score = 0;
                    powerUps = 0;    
                          // Reset power-ups
                    nextPowerUpScore = 50; // Reset to first threshold
                    inMenu = false;
                    inDifficulty = false;
                    inscore = false;
                    x = 0;
                    y = 0;
                    dx = 0;
                    dy = 0;
                    for (int i = 0; i < M; i++)
                        for (int j = 0; j < N; j++)
                            if (i == 0 || j == 0 || i == M - 1 || j == N - 1)
                                grid[i][j] = 1;
                            else
                                grid[i][j] = 0;
                    selectlayermode = true;
                }
                if (e.key.code == Keyboard::Num2) // If 2 is pressed in menu
                {
                    inscore = false;
                    inMenu = false;
                    inDifficulty = true;

                    // Skip the game logic and stay in the menu
                }
                if (e.key.code == Keyboard::Num3) // If 3 is pressed in menu
                {
                    inMenu = false;
                    inDifficulty = false;
                    inscore = true;
                }
            }
            else if (inDifficulty)
            {
                if (e.key.code == Keyboard::Num4)
                {
                    inMenu = false;
                    inDifficulty = false;
                    inscore = false;
                    enemyCount = 2;
                    score = 0;
                    powerUps = 0;          // Reset power-ups
                    nextPowerUpScore = 50; // Reset to first threshold
                    x = 0;
                    y = 0;
                    dx = 0;
                    dy = 0;
                    Game = true;
                }
                else if (e.key.code == Keyboard::Num5)
                {
                    inMenu = false;
                    inDifficulty = false;
                    inscore = false;
                    enemyCount = 4;
                    score = 0;
                    powerUps = 0;          // Reset power-ups
                    nextPowerUpScore = 50; // Reset to first threshold
                    x = 0;
                    y = 0;
                    dx = 0;
                    dy = 0;
                    Game = true;
                }
                else if (e.key.code == Keyboard::Num6)
                {
                    inMenu = false;
                    inDifficulty = false;
                    inscore = false;
                    enemyCount = 6;
                    score = 0;
                    powerUps = 0;          // Reset power-ups
                    nextPowerUpScore = 50; // Reset to first threshold
                    x = 0;
                    y = 0;
                    dx = 0;
                    dy = 0;
                    Game = true;
                }
                else if (e.key.code == Keyboard::Num7)
                {
                    inMenu = false;
                    inDifficulty = false;
                    inscore = false;
                    enemyCount = 2;
                    continuousmode = true;
                    enemyAddClock.restart();
                    score = 0;
                    powerUps = 0;          // Reset power-ups
                    nextPowerUpScore = 50; // Reset to first threshold
                    x = 0;
                    y = 0;
                    dx = 0;
                    dy = 0;
                    Game = true;
                }
            }
            else if (selectlayermode)
            {

                if (e.type == sf::Event::Closed)
                    window.close();

                if (e.type == sf::Event::KeyPressed)
                {
                    if (e.key.code == sf::Keyboard::O)
                    {
                        // Handle one player mode
                        selectlayermode = false;
                        playmode = 1;
                        Game = true;
                        // Start one player game logic here
                    }
                    else if (e.key.code == sf::Keyboard::T)
                    {
                        // Handle two player mode
                        selectlayermode = false;
                        Game = true;
                        playmode = 2;
                        // Start two player game logic here
                    }
                }
            }
            if (e.key.code == Keyboard::Escape) // already given
            {
                for (int i = 1; i < M - 1; i++)
                    for (int j = 1; j < N - 1; j++)
                        grid[i][j] = 0;

                x = 0;
                y = 0;
                Game = true;
            }

            // Player movemnt
        }

        if (Keyboard::isKeyPressed(Keyboard::A))
        {
            dx = -1;
            dy = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::D))
        {
            dx = 1;
            dy = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::W))
        {
            dx = 0;
            dy = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::S))
        {
            dx = 0;
            dy = 1;
        }

        if (Keyboard::isKeyPressed(Keyboard::Left) && playmode == 2)
        {
            dx2 = -1;
            dy2 = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Right) && playmode == 2)
        {
            dx2 = 1;
            dy2 = 0;
        }
        if (Keyboard::isKeyPressed(Keyboard::Up) && playmode == 2)
        {
            dx2 = 0;
            dy2 = -1;
        }
        if (Keyboard::isKeyPressed(Keyboard::Down) && playmode == 2)
        {
            dx2 = 0;
            dy2 = 1;
        }

        if (inDifficulty)
        {
            window.clear();
            window.draw(sDifficulty); // Difficulty wali image
            window.draw(text5);       // Easy Level
            window.draw(text6);       // Medium Level
            window.draw(text7);       // Hard Level
            window.draw(text8);       // Continuous Mode
            window.display();
            continue;
        }
        // for menue
        if (inMenu)
        {
            window.clear();
            window.draw(sMenue); // Menu background image
            window.draw(text1);
            window.draw(text2);
            window.draw(text3);
            window.draw(text4);
            window.display();
            continue; // Important: neeche ka game logic skip karo
        }
        // for score.
        if (inscore)
        {
            window.clear();
            window.draw(sScore); // Menu background image
            text10.setString("Score = " + std::to_string(score));
            text11.setString("Power UP = " + std::to_string(powerUps));

            window.clear();
            window.draw(text10);
            window.draw(text11);
            window.display();
            bool show = 0;
            if (Keyboard::isKeyPressed(Keyboard::Escape))
                show = 1;

            if (show)
            {
                inMenu = true;
                inscore = false;
                continue;
            }

            continue; // Important: neeche ka game logic skip karo
        }

        if (selectlayermode)
        {
            window.clear();
            window.draw(sMenue);
            window.draw(onePlayerText);
            window.draw(twoPlayerText);
            window.display();
            continue;
        }
        // Score
        if (enemyFrozen)
        {
            if (freezeClock.getElapsedTime().asSeconds() >= freezeDuration)
            {
                enemyFrozen = false;
            }
        }

        // powerup

        //  end

        // if continous mode was selected
        if (continuousmode)
        {
            if (enemyAddClock.getElapsedTime().asSeconds() >= 20.0f && enemyCount < 20)
            {
                a[enemyCount].x = a[enemyCount - 1].x;
                a[enemyCount].y = a[enemyCount - 1].y;

                a[enemyCount + 1].x = a[enemyCount - 2].x;
                a[enemyCount + 1].y = a[enemyCount - 2].y;

                enemyCount += 2;
                enemyAddClock.restart(); // Reset the clock for next addition
            }
        }

        // Trail (2) ko filled area (1) mein badal do
        if (Keyboard::isKeyPressed(Keyboard::Escape))
        {
            x = 0, y = 0, x2 = N - 1, y2 = M - 1;
            dx = dx2 = dy = dy2 = 0;
            inMenu = true;
        }
        if (!Game)
        {

            window.draw(sGameover);
            window.draw(text12);
            window.display();
            continue;
        }

        if (timer > delay)
        {
            x += dx;
            y += dy;

            x2 += dx2;
            y2 += dy2;

            if (Keyboard::isKeyPressed(Keyboard::S))
                cout << score << endl;

            if (x < 0)
                x = 0;
            if (x > N - 1)
                x = N - 1;
            if (y < 0)
                y = 0;
            if (y > M - 1)
                y = M - 1;

            if (x2 < 0)
                x2 = 0;
            if (x2 > N - 1)
                x2 = N - 1;
            if (y2 < 0)
                y2 = 0;
            if (y2 > M - 1)
                y2 = M - 1;

            if (grid[y][x] == 2)
            {
                Game = false;
            }
            else if (grid[y][x] == 0)
            {
                grid[y][x] = 2;
                
                cout << "score : " << score <<endl;
            }

            if (grid[y2][x2] == 3)
            {
                Game = false;
            }
            else if (grid[y2][x2] == 0)
            {
                grid[y2][x2] = 3;
            }

            timer = 0;
        }

        if (!enemyFrozen)
        {
            for (int i = 0; i < enemyCount; i++)
                a[i].move();
        }

        if (grid[y][x] == 1)
        {
            dx = dy = 0;

            static int capturedTiles = 0;
            static int bonusCounter = 0;
            int moveScore = 0; // score for this move

            // Mark areas to be filled using the drop function
            for (int i = 0; i < enemyCount; i++)
                drop(a[i].y / ts, a[i].x / ts);

            int tiles = 0 ;
            // Convert unmarked areas to filled (1) and count captured tiles
            for (int i = 0; i < M; i++)
                for (int j = 0; j < N; j++)
                    if (grid[i][j] == -1)
                        grid[i][j] = 0; // Reset temporary mark
                    else if (grid[i][j] == 0 || grid[i][j] == 2)
                    {
                        capturedTiles++;
                        //if (grid[i][j] == 2){
                            
                            tiles++;
                            cout << "Score : " << score << endl;
                        
                        grid[i][j] = 1; // Capture the tile
                        
                    }

            score += tiles;

            if (bonusCounter >= 5 && capturedTiles > 5)
            {
                moveScore = capturedTiles * 4;
                bonusCounter++;
            }
            else if (bonusCounter >= 3 && capturedTiles > 5)
            {
                moveScore = capturedTiles * 2;
                bonusCounter++;
            }
            else if (capturedTiles > 10)
            {
                moveScore = capturedTiles * 2;
                bonusCounter++;
            }
            else
            {
                moveScore = capturedTiles; // normal score
            }

            
            capturedTiles = 0;  // Reset for next mov

            // Check for power-up thresholds
            while (score >= nextPowerUpScore)
            {
                powerUps++;
                if (nextPowerUpScore == 50)
                    nextPowerUpScore = 70;
                else if (nextPowerUpScore == 70)
                    nextPowerUpScore = 100;
                else if (nextPowerUpScore == 100)
                    nextPowerUpScore = 130;
                else
                    nextPowerUpScore += 30; // Subsequent thresholds increase by 30
            }

            // Optional: Log for debugging
            // std::cout << "Captured " << capturedTiles << " tiles, Score: " << score << ", PowerUps: " << powerUps << endl;
        }

        if (grid[y2][x2] == 1)
        {
            dx2 = dy2 = 0;

            // static int capturedTiles = 0;
            // static int bonusCounter = 0;
            // int moveScore = 0; // score for this move

            // Mark areas to be filled using the drop function
            for (int i = 0; i < enemyCount; i++)
                drop(a[i].y / ts, a[i].x / ts);

            // Convert unmarked areas to filled (1) and count captured tiles
            for (int i = 0; i < M; i++)
                for (int j = 0; j < N; j++)
                    if (grid[i][j] == -1)
                        grid[i][j] = 0; // Reset temporary mark
                    else if (grid[i][j] == 0 || grid[i][j] == 3)
                    {
                        grid[i][j] = 1; // Capture the tile
                        // capturedTiles++;
                    }

            // if (bonusCounter >= 5 && capturedTiles > 5)
            // {
            //     moveScore = capturedTiles * 4;
            //     bonusCounter++;
            // }
            // else if (bonusCounter >= 3 && capturedTiles > 5)
            // {
            //     moveScore = capturedTiles * 2;
            //     bonusCounter++;
            // }
            // else if (capturedTiles > 10)
            // {
            //     moveScore = capturedTiles * 2;
            //     bonusCounter++;
            // }
            // else
            // {
            //     moveScore = capturedTiles; // normal score
            // }

            // score += moveScore; // Add to total score
            // capturedTiles = 0;  // Reset for next mov

            // // Check for power-up thresholds
            // while (score >= nextPowerUpScore)
            // {
            //     powerUps++;
            //     if (nextPowerUpScore == 50)
            //         nextPowerUpScore = 70;
            //     else if (nextPowerUpScore == 70)
            //         nextPowerUpScore = 100;
            //     else if (nextPowerUpScore == 100)
            //         nextPowerUpScore = 130;
            //     else
            //         nextPowerUpScore += 30; // Subsequent thresholds increase by 30
            // }

            // Optional: Log for debugging
            // std::cout << "Captured " << capturedTiles << " tiles, Score: " << score << ", PowerUps: " << powerUps << endl;
        }

        /*  if(capturedTiles>10){
          score+=capturedTiles*2;
          }
          else{
          score+=capturedTiles;
          }
    */
        for (int i = 0; i < enemyCount; i++)
            if (grid[a[i].y / ts][a[i].x / ts] == 2 || (grid[a[i].y / ts][a[i].x / ts] == 3 && playmode == 2))
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

                // check if it is 2nd player tails
                if (grid[i][j] == 3)                              // For Player 2's trail
                    sTile.setTextureRect(IntRect(ts, 0, ts, ts)); // Adjust texture as needed

                sTile.setPosition(j * ts, i * ts);
                window.draw(sTile);
            }

        if (playmode == 2)
        {

            // Player 1 boxes
            sTile.setTextureRect(IntRect(36, 0, ts, ts));
            sTile.setPosition(x * ts, y * ts);
            window.draw(sTile);

            // Player 2 boxes
            sTile.setTextureRect(IntRect(36 + ts + ts, 0, ts, ts));
            sTile.setPosition(x2 * ts, y2 * ts); // Player 2 position offset
            window.draw(sTile);
        }
        else
        {
            // player 1 boxes
            sTile.setTextureRect(IntRect(36, 0, ts, ts));
            sTile.setPosition(x * ts, y * ts);
            window.draw(sTile);
        }

        sEnemy.rotate(10);
        for (int i = 0; i < enemyCount; i++)
        {
            sEnemy.setPosition(a[i].x, a[i].y);
            window.draw(sEnemy);
        }

        text9.setString("Move count = " + to_string(Movecount));
        window.draw(text9);

        scoreTextDisplay.setString("Score = " + std::to_string(score));
        window.draw(scoreTextDisplay);
        powerupDisplay.setString("Power up = " + std::to_string(powerUps));
        window.draw(powerupDisplay);
        window.display();

        if (!Game)
        {
            cout << "GameOver" << endl;
            window.draw(sGameover);
            window.draw(text12);
        }
    }

    // for difficulty.
    return 0;
}
