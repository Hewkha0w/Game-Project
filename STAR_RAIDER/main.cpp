#include <iostream>
#include <math.h>
#include <cstdlib>
#include <vector>
#include <time.h>
#include <algorithm>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>

using namespace sf;

class Bullet
{
public:
	Sprite shape;

	Bullet(Texture* texture, Vector2f pos)
	{
		this->shape.setTexture(*texture);
		this->shape.setScale(0.05f, 0.05f);
		this->shape.setPosition(pos);
	}

	~Bullet() {}
};
class Player
{
public:
	Sprite shape;
	Texture* texture;
	int HP;
	int HPMax;
	std::vector<Bullet> bullets;
	Vector2f curVelocity;
	Vector2f dir;
	float maxVelocity = 10.f;
	float acc = 1.f;
	float drag = 0.5f;
	int boostBullet = 100;
	int bulletDmg;
	std::string name = "";
	int level = 1;
	int exp = 0;
	int skPoint = 3;

	Player(Texture* texture)
	{
		this->HPMax = 10;
		this->HP = this->HPMax;
		this->texture = texture;
		this->bulletDmg = 1;
		this->shape.setTexture(*texture);
		this->shape.setScale(0.1f, 0.1f);
	}

	~Player() {}
};
class Enemy
{
public:
	Sprite shape;
	int HP;
	int HPMax;

	Enemy(Texture* texture, Vector2u windowSize, int boostE)
	{
		this->HPMax = rand() % 3 + 5 + boostE;
		this->HP = this->HPMax;
		this->shape.setTexture(*texture);
		this->shape.setScale(0.1f, 0.1f);
		this->shape.setPosition(static_cast<float>(rand() % (int)(windowSize.x - this->shape.getGlobalBounds().width)), -100.f);
	}

	~Enemy() {}
};
class Enemy2
{
public:
	Sprite shape;
	int HP;
	int HPMax;
	std::vector<Bullet> bullets;

	Enemy2(Texture* texture, Vector2u windowSize, int boostE)
	{
		this->HPMax = rand() % 10 + 30 + boostE;
		this->HP = this->HPMax;
		this->shape.setTexture(*texture);
		this->shape.setScale(0.25f, 0.25f);
		this->shape.setPosition(static_cast<float>(rand() % (int)(windowSize.x - this->shape.getGlobalBounds().width)), -700.f);
	}

	~Enemy2() {}
};
class Item
{
public:
	Sprite shape;
	int type;
	bool isCollected;

	Item(Texture* texture, Vector2f pos, int itemType)
	{
		this->shape.setTexture(*texture);
		this->shape.setScale(0.05f, 0.05f);
		this->shape.setPosition(pos);
		this->type = itemType;
		this->isCollected = false;
	}
};
enum class GameState
{
	MainMenu,
	Playing,
	GameOver,
	Paused,
	StatUpgrade,
	ScoreBoard,
	HowToPlay,
	Credit,
	EnterName
};
enum class GunMode
{
	Normal,
	Boost
};
enum class MainMenuState
{
	Play,
	Scoreboard,
	HowToPlay,
	Credit,
	Exit
};
enum class PauseState
{
	Resume,
	Restart,
	Mainmenu
};

struct ScoreEntry {
	std::string playerName;
	int score = 0;

	bool operator<(const ScoreEntry& other) const
	{
		return score > other.score;
	}
};

std::vector<ScoreEntry> highScores;

void drawText(RenderWindow& window, const std::string& text, const Font& font,
	int characterSize, const Vector2f& position, const Color& color);
void loadHighScores();
void saveHighScores();
void updateHighScores(const std::string& playerName, int score);

int main()
{
	srand(static_cast<unsigned int>(time(NULL)));
	RenderWindow window(VideoMode(1920, 1080), "STAR RAIDER", Style::Fullscreen);
	window.setMouseCursorVisible(false);
	window.setFramerateLimit(60);

	Clock clock;
	float dt;
	float multiplier = 60.f;

	//Init text
	Font fontUI, fontLogo;
	fontUI.loadFromFile("Fonts/Orbitron-Black.ttf");
	fontLogo.loadFromFile("Fonts/spacerangerhalf.ttf");

	//Sounds
	SoundBuffer shootingBuffer, itemBulletBuffer, itemHealthBuffer, enemyDestroiedBuffer, playerDamagedBuffer,
		menuSelectBuffer, typingBuffer, itemBoosterBuffer, itemExpBuffer, skillUpgradeBuffer, levelUpBuffer;
	shootingBuffer.loadFromFile("Sounds/shoot.wav");
	itemBulletBuffer.loadFromFile("Sounds/itemBullet.wav");
	itemHealthBuffer.loadFromFile("Sounds/itemHealth.wav");
	enemyDestroiedBuffer.loadFromFile("Sounds/enemyDestroied.wav");
	playerDamagedBuffer.loadFromFile("Sounds/playerDamaged.wav");
	menuSelectBuffer.loadFromFile("Sounds/menuSelect.wav");
	typingBuffer.loadFromFile("Sounds/typing.wav");
	itemBoosterBuffer.loadFromFile("Sounds/itemBooster.wav");
	itemExpBuffer.loadFromFile("Sounds/itemExp.wav");
	skillUpgradeBuffer.loadFromFile("Sounds/skillUpgrade.wav");
	levelUpBuffer.loadFromFile("Sounds/levelUP.wav");

	Sound shootingSound, itemBulletSound, itemHealthSound, enemyDestroiedSound, playerDamagedSound, menuSelectSound,
		typingSound, itemBoosterSound, itemExpSound, skillUpgradeSound, levelUpSound;
	shootingSound.setBuffer(shootingBuffer);
	shootingSound.setVolume(20);
	itemBulletSound.setBuffer(itemBulletBuffer);
	itemBulletSound.setVolume(50);
	itemHealthSound.setBuffer(itemHealthBuffer);
	itemHealthSound.setVolume(35);
	enemyDestroiedSound.setBuffer(enemyDestroiedBuffer);
	enemyDestroiedSound.setVolume(50);
	playerDamagedSound.setBuffer(playerDamagedBuffer);
	menuSelectSound.setBuffer(menuSelectBuffer);
	menuSelectSound.setVolume(50);
	typingSound.setBuffer(typingBuffer);
	itemBoosterSound.setBuffer(itemBoosterBuffer);
	itemExpSound.setBuffer(itemExpBuffer);
	skillUpgradeSound.setBuffer(skillUpgradeBuffer);
	levelUpSound.setBuffer(levelUpBuffer);

	Music menuMusic, backgroundMusic;
	menuMusic.openFromFile("Sounds/menuMusic.ogg");
	menuMusic.setLoop(true);
	backgroundMusic.openFromFile("Sounds/backgroundMusic.ogg");
	backgroundMusic.setLoop(true);
	backgroundMusic.setVolume(50);
	menuMusic.play();

	//Init texture
	Texture playerTex, enemyTex, bulletTex, backgroundTex1, backgroundTex2, itemHealthTex, itemBulletTex, boostedBulletTex, mainMenuBackgroundTex,
		enemy2Tex, itemBoosterTex, itemEXPTex, skHPTex, skDmgTex, skSpeedTex;
	playerTex.loadFromFile("Textures/ship.png");
	enemyTex.loadFromFile("Textures/enemy.png");
	bulletTex.loadFromFile("Textures/bullet01.png");
	boostedBulletTex.loadFromFile("Textures/bullet02.png");
	backgroundTex1.loadFromFile("Textures/background01.png");
	backgroundTex2.loadFromFile("Textures/background02.png");
	mainMenuBackgroundTex.loadFromFile("Textures/menuBackground.png");
	itemHealthTex.loadFromFile("Textures/itemHealth.png");
	itemBulletTex.loadFromFile("Textures/itemBullet.png");
	enemy2Tex.loadFromFile("Textures/enemy2.png");
	itemBoosterTex.loadFromFile("Textures/itemBooster.png");
	itemEXPTex.loadFromFile("Textures/itemEXP.png");
	skHPTex.loadFromFile("Textures/skillHP.png");
	skDmgTex.loadFromFile("Textures/skillDmg.png");
	skSpeedTex.loadFromFile("Textures/skillSpeed.png");

	//Item
	std::vector<Item> items;
	int itemType = 0;
	int dropChance;
	int boostSpeed = 0;
	float boosterTimer = 5;

	//Filter
	RectangleShape dim, dimSmall;
	dim.setSize(Vector2f(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)));
	dim.setFillColor(Color(0, 0, 0, 32));
	dimSmall.setSize(Vector2f(static_cast<float>(window.getSize().x) - 800.f, static_cast<float>(window.getSize().y - 400.f)));
	dimSmall.setPosition(Vector2f(static_cast<float>(400.f), static_cast<float>(200.f)));
	dimSmall.setFillColor(Color(0, 0, 0, 255));
	dimSmall.setOutlineColor(Color::Cyan);
	dimSmall.setOutlineThickness(5.f);

	//Background init
	Sprite background1, background2;
	background1.setTexture(backgroundTex1);
	background2.setTexture(backgroundTex2);
	background1.setPosition(Vector2f(0.f, 0.f));
	background2.setPosition(Vector2f(0.f, -1080.f));

	//UI init
	Text scoreText, bulletText, levelText, expText, bulletDmgText, MaxHpText, shootSpeedText, skPointText, hpLevelText, dmgLevelText, speedLevelText;
	scoreText.setFont(fontUI);
	scoreText.setCharacterSize(20);
	scoreText.setFillColor(Color::White);
	scoreText.setPosition(10.f, 10.f);

	bulletText.setFont(fontUI);
	bulletText.setCharacterSize(20);
	bulletText.setFillColor(Color::White);
	bulletText.setPosition(10.f, 1000.f);

	RectangleShape healthBar;
	healthBar.setSize(Vector2f(200.f, 20.f));
	healthBar.setFillColor(Color::Green);
	healthBar.setPosition(10.f, window.getSize().y - 30.f);

	levelText.setFont(fontUI);
	levelText.setCharacterSize(20);
	levelText.setFillColor(Color::White);
	levelText.setPosition(10.f, 970.f);

	expText.setFont(fontUI);
	expText.setCharacterSize(20);
	expText.setFillColor(Color::White);
	expText.setPosition(10.f, 940.f);

	bulletDmgText.setFont(fontUI);
	bulletDmgText.setCharacterSize(36);
	bulletDmgText.setFillColor(Color::White);
	bulletDmgText.setPosition(450.f, 725.f);

	MaxHpText.setFont(fontUI);
	MaxHpText.setCharacterSize(36);
	MaxHpText.setFillColor(Color::White);
	MaxHpText.setPosition(450.f, 650.f);

	shootSpeedText.setFont(fontUI);
	shootSpeedText.setCharacterSize(36);
	shootSpeedText.setFillColor(Color::White);
	shootSpeedText.setPosition(450.f, 800.f);

	skPointText.setFont(fontUI);
	skPointText.setCharacterSize(36);
	skPointText.setFillColor(Color::White);
	skPointText.setPosition(450.f, 575.f);

	hpLevelText.setFont(fontUI);
	hpLevelText.setCharacterSize(18);
	hpLevelText.setFillColor(Color::White);
	hpLevelText.setPosition(600.f, 450.f);

	dmgLevelText.setFont(fontUI);
	dmgLevelText.setCharacterSize(18);
	dmgLevelText.setFillColor(Color::White);
	dmgLevelText.setPosition(900.f, 450.f);

	speedLevelText.setFont(fontUI);
	speedLevelText.setCharacterSize(18);
	speedLevelText.setFillColor(Color::White);
	speedLevelText.setPosition(1200.f, 450.f);

	//Player Init
	int score = 0;
	float shootTimer = 20;
	Player player(&playerTex);
	player.shape.setPosition(Vector2f(static_cast<float>(window.getSize().x) / 2, static_cast<float>(window.getSize().y) - 200.f));
	float fireSpeed = 1;

	//Enemy init
	int enemySpawnTimer = 0;
	int boostE = 0;
	std::vector<Enemy> enemies;
	std::vector<Enemy2> enemies2;
	int spawnChange = 0;
	int count = 0;

	//Gun mode
	GunMode currentGunMode = GunMode::Normal;

	//Skill point
	Sprite HP, Dmg, Speed;
	HP.setTexture(skHPTex);
	HP.setScale(0.5f, 0.5f);
	HP.setPosition(570.f, 275.f);
	Dmg.setTexture(skDmgTex);
	Dmg.setScale(0.5f, 0.5f);
	Dmg.setPosition(870.f, 275.f);
	Speed.setTexture(skSpeedTex);
	Speed.setScale(0.5f, 0.5f);
	Speed.setPosition(1170.f, 275.f);
	float skSpeed = 0;
	int skHP = 0;
	int skDamage = 0;
	int hpLevel = 0;
	int dmgLevel = 0;
	int speedLevel = 0;

	//Main menu
	GameState currentState = GameState::MainMenu;
	MainMenuState currentMenuState = MainMenuState::Play;
	Sprite mainMenuBackground;
	mainMenuBackground.setTexture(mainMenuBackgroundTex);

	//Pause
	PauseState currentPauseState = PauseState::Resume;
	Text pauseText;
	pauseText.setFont(fontUI);
	pauseText.setCharacterSize(40);
	pauseText.setFillColor(Color::Red);
	pauseText.setPosition(window.getSize().x / 2 - 130.f, window.getSize().y / 2 - 50.f);
	pauseText.setString("--- PAUSE ---");

	//Game Over
	Text gameOverText;
	gameOverText.setFont(fontUI);
	gameOverText.setCharacterSize(40);
	gameOverText.setFillColor(Color::Red);
	gameOverText.setPosition(window.getSize().x / 2 - 200.f, window.getSize().y / 2 - 50.f);
	gameOverText.setString("--- GAME OVER ---");

	loadHighScores();

	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
				window.close();

			if (event.type == Event::LostFocus && currentState == GameState::Playing)
			{
				currentState = GameState::Paused;
			}

			if (event.type == Event::TextEntered && currentState == GameState::EnterName)
			{
				if (event.text.unicode == 8)
				{
					if (!player.name.empty())
					{
						player.name.pop_back();
					}
					typingSound.play();
				}
				else if (event.text.unicode >= 32 && event.text.unicode <= 126 && player.name.length() < 20)
				{
					player.name += static_cast<char>(event.text.unicode);
					typingSound.play();
				}
			}

			if (event.type == Event::KeyPressed)
			{
				if (currentState == GameState::MainMenu)
				{
					if (event.key.code == Keyboard::W)
					{
						if (currentMenuState > MainMenuState::Play)
						{
							currentMenuState = static_cast<MainMenuState>(static_cast<int>(currentMenuState) - 1);
							menuSelectSound.play();
						}
					}
					else if (event.key.code == Keyboard::S)
					{
						if (currentMenuState < MainMenuState::Exit)
						{
							currentMenuState = static_cast<MainMenuState>(static_cast<int>(currentMenuState) + 1);
							menuSelectSound.play();
						}
					}
					else if (event.key.code == Keyboard::Return)
					{
						if (currentMenuState == MainMenuState::Play)
						{
							menuSelectSound.play();
							currentState = GameState::EnterName;
						}
						else if (currentMenuState == MainMenuState::Scoreboard)
						{
							menuSelectSound.play();
							currentState = GameState::ScoreBoard;
						}
						else if (currentMenuState == MainMenuState::HowToPlay)
						{
							menuSelectSound.play();
							currentState = GameState::HowToPlay;
						}
						else if (currentMenuState == MainMenuState::Credit)
						{
							menuSelectSound.play();
							currentState = GameState::Credit;
						}
						else if (currentMenuState == MainMenuState::Exit)
						{
							menuSelectSound.play();
							window.close();
						}
					}
				}

				if (currentState == GameState::ScoreBoard)
				{
					if (event.key.code == Keyboard::Escape)
					{
						menuSelectSound.play();
						currentMenuState = MainMenuState::Play;
						currentState = GameState::MainMenu;
					}
				}

				if (currentState == GameState::HowToPlay)
				{
					if (event.key.code == Keyboard::Escape)
					{
						menuSelectSound.play();
						currentMenuState = MainMenuState::Play;
						currentState = GameState::MainMenu;
					}
				}

				if (currentState == GameState::Credit)
				{
					if (event.key.code == Keyboard::Escape)
					{
						menuSelectSound.play();
						currentMenuState = MainMenuState::Play;
						currentState = GameState::MainMenu;
					}
				}

				if (currentState == GameState::EnterName)
				{
					if (event.key.code == Keyboard::Return && player.name != "")
					{
						menuSelectSound.play();
						player.HPMax = 10;
						player.HP = player.HPMax;
						player.bullets.clear();
						enemies.clear();
						enemies2.clear();
						items.clear();
						score = 0;
						player.boostBullet = 100;
						boostE = 0;
						player.exp = 0;
						player.level = 1;
						player.skPoint = 0;
						fireSpeed = 1;
						count = 0;
						skSpeed = 0;
						skHP = 0;
						skDamage = 0;
						hpLevel = 0;
						dmgLevel = 0;
						speedLevel = 0;
						currentGunMode = GunMode::Normal;
						player.shape.setPosition(sf::Vector2f(static_cast<float>(window.getSize().x) / 2, static_cast<float>(window.getSize().y) - 200.f));
						background1.setPosition(Vector2f(0.f, 0.f));
						background2.setPosition(Vector2f(0.f, -1080.f));
						menuSelectSound.play();
						for (int i = 0; i < 75; i++)
						{
							window.draw(dim);
							window.display();
						}
						menuMusic.stop();
						backgroundMusic.play();
						currentState = GameState::Playing;
					}

					if (event.key.code == Keyboard::Escape)
					{
						menuSelectSound.play();
						currentState = GameState::MainMenu;
					}
				}

				if (currentState == GameState::Playing)
				{
					if (event.key.code == Keyboard::J)
					{
						if (currentGunMode == GunMode::Normal && player.boostBullet > 0)
						{
							currentGunMode = GunMode::Boost;
							player.bulletDmg = 3 + (player.level / 2) + skDamage;
						}
						else
						{
							currentGunMode = GunMode::Normal;
							player.bulletDmg = 1 + (player.level / 2) + skDamage;
						}
					}

					if (event.key.code == Keyboard::U)
					{
						if (currentState == GameState::Playing)
						{
							for (int i = 0; i < 4; i++)
							{
								window.draw(dim);
							}
							menuSelectSound.play();
							currentState = GameState::StatUpgrade;
							break;
						}
					}

					if (event.key.code == Keyboard::Escape)
					{
						if (currentState == GameState::Playing)
						{
							for (int i = 0; i < 4; i++)
							{
								window.draw(dim);
							}
							menuSelectSound.play();
							currentState = GameState::Paused;
							break;
						}
					}
				}

				if (currentState == GameState::Paused)
				{
					if (event.key.code == Keyboard::W)
					{
						if (currentPauseState > PauseState::Resume)
						{
							currentPauseState = static_cast<PauseState>(static_cast<int>(currentPauseState) - 1);
							menuSelectSound.play();
						}
					}
					else if (event.key.code == Keyboard::S)
					{
						if (currentPauseState < PauseState::Mainmenu)
						{
							currentPauseState = static_cast<PauseState>(static_cast<int>(currentPauseState) + 1);
							menuSelectSound.play();
						}
					}
					else if (event.key.code == Keyboard::Return)
					{
						if (currentPauseState == PauseState::Resume)
						{
							menuSelectSound.play();
							currentState = GameState::Playing;
							break;
						}
						else if (currentPauseState == PauseState::Restart)
						{
							menuSelectSound.play();
							player.HPMax = 10;
							player.HP = player.HPMax;
							player.bullets.clear();
							enemies.clear();
							enemies2.clear();
							items.clear();
							score = 0;
							player.boostBullet = 100;
							boostE = 0;
							player.exp = 0;
							player.level = 1;
							player.skPoint = 0;
							fireSpeed = 1;
							count = 0;
							skSpeed = 0;
							skHP = 0;
							skDamage = 0;
							hpLevel = 0;
							dmgLevel = 0;
							speedLevel = 0;
							currentGunMode = GunMode::Normal;
							player.shape.setPosition(sf::Vector2f(static_cast<float>(window.getSize().x) / 2, static_cast<float>(window.getSize().y) - 200.f));
							background1.setPosition(Vector2f(0.f, 0.f));
							background2.setPosition(Vector2f(0.f, -1080.f));
							backgroundMusic.stop();
							for (int i = 0; i < 75; i++)
							{
								window.draw(dim);
								window.display();
							}
							backgroundMusic.play();
							currentState = GameState::Playing;
							currentPauseState = PauseState::Resume;
						}
						else if (currentPauseState == PauseState::Mainmenu)
						{
							menuSelectSound.play();
							backgroundMusic.stop();
							for (int i = 0; i < 75; i++)
							{
								window.draw(dim);
								window.display();
							}
							menuMusic.play();
							currentState = GameState::MainMenu;
							currentPauseState = PauseState::Resume;
						}
					}

					if (event.key.code == Keyboard::Escape)
					{
						currentState = GameState::Playing;
						currentPauseState = PauseState::Resume;
					}
				}

				if (currentState == GameState::StatUpgrade)
				{
					if (event.key.code == Keyboard::U || event.key.code == Keyboard::Escape)
					{
						menuSelectSound.play();
						currentState = GameState::Playing;
					}

					if (player.skPoint > 0)
					{
						if (event.key.code == Keyboard::Z)
						{
							player.HPMax += 2;
							player.skPoint--;
							hpLevel++;
							skillUpgradeSound.play();
							break;
						}
						if (event.key.code == Keyboard::X)
						{
							skDamage++;
							player.skPoint--;
							dmgLevel++;
							skillUpgradeSound.play();
							break;
						}
						if (event.key.code == Keyboard::C)
						{
							skSpeed += 0.2f;
							player.skPoint--;
							speedLevel++;
							skillUpgradeSound.play();
							break;
						}
					}
				}

				if (currentState == GameState::GameOver)
				{
					if (event.key.code == Keyboard::Return)
					{
						backgroundMusic.stop();
						menuSelectSound.play();
						player.name = "";
						for (int i = 0; i < 75; i++)
						{
							window.draw(dim);
							window.display();
						}
						menuMusic.play();
						currentState = GameState::MainMenu;
					}
				}
			}
		}

		dt = clock.restart().asSeconds();

		if (currentState == GameState::Playing)
		{
			if (player.HP > 0)
			{
				// ======================================== Player ========================================
				player.dir = Vector2f(0.f, 0.f);
				if (Keyboard::isKeyPressed(Keyboard::W))
				{
					player.dir.y = -1.f;

					if (player.curVelocity.y > -player.maxVelocity)
						player.curVelocity.y += player.acc * player.dir.y * dt * multiplier;
				}
				if (Keyboard::isKeyPressed(Keyboard::A))
				{
					player.dir.x = -1.f;

					if (player.curVelocity.x > -player.maxVelocity)
						player.curVelocity.x += player.acc * player.dir.x * dt * multiplier;
				}
				if (Keyboard::isKeyPressed(Keyboard::S))
				{
					player.dir.y = 1.f;

					if (player.curVelocity.y < player.maxVelocity)
						player.curVelocity.y += player.acc * player.dir.y * dt * multiplier;
				}
				if (Keyboard::isKeyPressed(Keyboard::D))
				{
					player.dir.x = 1.f;

					if (player.curVelocity.x < player.maxVelocity)
						player.curVelocity.x += player.acc * player.dir.x * dt * multiplier;
				}

				// ======================================== Collision with window ========================================
				//left
				if (player.shape.getPosition().x <= 0)
					player.shape.setPosition(0.f, player.shape.getPosition().y);
				//right
				if (player.shape.getPosition().x >= window.getSize().x - player.shape.getGlobalBounds().width)
					player.shape.setPosition(window.getSize().x - player.shape.getGlobalBounds().width, player.shape.getPosition().y);
				//top
				if (player.shape.getPosition().y <= 0)
					player.shape.setPosition(player.shape.getPosition().x, 0.f);
				//buttom
				if (player.shape.getPosition().y >= window.getSize().y - player.shape.getGlobalBounds().height)
					player.shape.setPosition(player.shape.getPosition().x, window.getSize().y - player.shape.getGlobalBounds().height);

				// ======================================== UPDATE Controls ========================================
				if (shootTimer < 6 - boostSpeed - skSpeed)
					shootTimer++;

				if (currentGunMode == GunMode::Boost)
				{
					player.bulletDmg = 3 + (player.level / 2) + skDamage;
				}
				else
				{
					player.bulletDmg = 1 + (player.level / 2) + skDamage;
				}

				if (Keyboard::isKeyPressed(Keyboard::K) && shootTimer >= 6 - boostSpeed - skSpeed)
				{
					if (currentGunMode == GunMode::Boost && player.boostBullet > 0)
					{
						player.bullets.push_back(Bullet(&boostedBulletTex, Vector2f(player.shape.getPosition().x + 9, player.shape.getPosition().y)));
						player.bullets.push_back(Bullet(&boostedBulletTex, Vector2f(player.shape.getPosition().x + 53, player.shape.getPosition().y)));
						player.boostBullet--;
						shootingSound.play();
					}
					else
					{
						player.bullets.push_back(Bullet(&bulletTex, Vector2f(player.shape.getPosition().x + 9, player.shape.getPosition().y)));
						player.bullets.push_back(Bullet(&bulletTex, Vector2f(player.shape.getPosition().x + 53, player.shape.getPosition().y)));
						shootingSound.play();
					}

					if (player.boostBullet <= 0)
					{
						player.bulletDmg = 1 + (player.level / 2) + skDamage;
						currentGunMode = GunMode::Normal;
					}

					shootTimer = 0;
				}

				// ======================================== Item ========================================
				for (size_t i = 0; i < items.size(); i++)
				{
					//Move
					items[i].shape.move(0.f, 1.5f * dt * multiplier);

					//Out of window bounds
					if (items[i].shape.getPosition().y >= window.getSize().y - items[i].shape.getGlobalBounds().height + 100.f)
					{
						items.erase(items.begin() + i);
					}

					for (size_t i = 0; i < items.size(); i++)
					{
						if (!items[i].isCollected && player.shape.getGlobalBounds().intersects(items[i].shape.getGlobalBounds()))
						{
							// item benefit
							switch (items[i].type)
							{
							case 0: // Health
								if (player.HP + 3 > 10)
								{
									player.HP = player.HPMax;
								}
								else
								{
									player.HP += 3;
								}
								itemHealthSound.play();
								break;
							case 1: // Bullet
								player.boostBullet += 30;
								itemBulletSound.play();
								break;
							case 2:
								boosterTimer = 0;
								itemBoosterSound.play();
								break;
							case 3:
								player.exp += player.level * 25;
								itemExpSound.play();
								break;
							}

							items[i].isCollected = true;
							items.erase(items.begin() + i);
						}
					}
				}

				if (boosterTimer < 5)
				{
					boostSpeed = 1;
				}
				else
				{
					boostSpeed = 0;
				}

				// ======================================== Bullets ========================================
				for (size_t i = 0; i < player.bullets.size(); i++)
				{
					//Move
					player.bullets[i].shape.move(0.f, -40.f * dt * multiplier);

					//Out of window bounds
					if (player.bullets[i].shape.getPosition().y < -2)
					{
						player.bullets.erase(player.bullets.begin() + i);
						break;
					}

					//Enemy collision
					for (size_t k = 0; k < enemies.size(); k++)
					{
						if (player.bullets[i].shape.getGlobalBounds().intersects(enemies[k].shape.getGlobalBounds()))
						{
							enemies[k].HP -= player.bulletDmg;

							if (enemies[k].HP <= 1)
							{
								score += enemies[k].HPMax;

								dropChance = rand() % 100;
								if (dropChance < 20)
								{
									itemType = rand() % 100;
									if (itemType <= 40)
									{
										items.push_back(Item(&itemHealthTex, enemies[k].shape.getPosition(), 0));
									}
									else if (itemType <= 65)
									{
										items.push_back(Item(&itemBulletTex, enemies[k].shape.getPosition(), 1));
									}
									else if (itemType <= 90)
									{
										items.push_back(Item(&itemEXPTex, enemies[k].shape.getPosition(), 3));
									}
									else if (itemType <= 100)
									{
										items.push_back(Item(&itemBoosterTex, enemies[k].shape.getPosition(), 2));
									}
								}

								player.exp += rand() % enemies[k].HPMax;
								enemyDestroiedSound.play();
								enemies.erase(enemies.begin() + k);
							}

							player.bullets.erase(player.bullets.begin() + i);
							break;
						}
					}

					for (size_t l = 0; l < enemies2.size(); l++)
					{
						if (player.bullets[i].shape.getGlobalBounds().intersects(enemies2[l].shape.getGlobalBounds()))
						{
							enemies2[l].HP -= player.bulletDmg;

							if (enemies2[l].HP <= 1)
							{
								score += enemies2[l].HPMax;

								dropChance = rand() % 100;
								if (dropChance < 35)
								{
									itemType = rand() % 100;
									if (itemType <= 30)
									{
										items.push_back(Item(&itemHealthTex, enemies2[l].shape.getPosition(), 0));
									}
									else if (itemType <= 60)
									{
										items.push_back(Item(&itemBulletTex, enemies2[l].shape.getPosition(), 1));
									}
									else if (itemType <= 85)
									{
										items.push_back(Item(&itemEXPTex, enemies[l].shape.getPosition(), 3));
									}
									else if (itemType <= 100)
									{
										items.push_back(Item(&itemBoosterTex, enemies[l].shape.getPosition(), 2));
									}
								}

								player.exp += rand() % enemies2[l].HPMax;
								enemyDestroiedSound.play();
								enemies2.erase(enemies2.begin() + l);
							}

							player.bullets.erase(player.bullets.begin() + i);
							break;
						}
					}
				}

				//Levels
				if (player.exp >= player.level * 100)
				{
					int temp = 0;
					temp = player.exp - player.level * 100;
					player.exp = 0;
					player.exp += temp;
					temp = 0;
					player.level++;
					player.skPoint++;
					player.HP = player.HPMax;
					levelUpSound.play();
				}

				// ======================================== Enemies ========================================
				if (enemySpawnTimer < 30 - boostE)
					enemySpawnTimer++;

				if (enemySpawnTimer >= 30 - boostE)
				{
					enemies.push_back(Enemy(&enemyTex, window.getSize(), boostE));
					if (player.level >= 5)
					{
						spawnChange = rand() % 100;
						if (spawnChange < 10)
						{
							enemies2.push_back(Enemy2(&enemy2Tex, window.getSize(), boostE));
						}
					}
					enemySpawnTimer = 0;
					boosterTimer += 0.5;
				}

				for (size_t i = 0; i < enemies.size(); i++)
				{
					enemies[i].shape.move(0.f, 3.f * dt * multiplier);

					if (enemies[i].shape.getPosition().y >= window.getSize().y - enemies[i].shape.getGlobalBounds().height)
					{
						enemies.erase(enemies.begin() + i);
						count++;
					}

					if (enemies[i].shape.getGlobalBounds().intersects(player.shape.getGlobalBounds()))
					{
						player.HP -= enemies[i].HPMax;
						playerDamagedSound.play();
						enemies.erase(enemies.begin() + i);
						enemyDestroiedSound.play();
					}
				}

				for (size_t i = 0; i < enemies2.size(); i++)
				{
					enemies2[i].shape.move(0.f, 3.f * dt * multiplier);

					if (enemies2[i].shape.getPosition().y >= window.getSize().y - enemies2[i].shape.getGlobalBounds().height)
					{
						enemies2.erase(enemies2.begin() + i);
						count += 2;
					}

					if (enemies2[i].shape.getGlobalBounds().intersects(player.shape.getGlobalBounds()))
					{
						player.HP -= enemies2[i].HPMax;
						playerDamagedSound.play();
						enemies2.erase(enemies2.begin() + i);
						enemyDestroiedSound.play();
					}
				}

				if (count >= 5)
				{
					playerDamagedSound.play();
					player.HP -= static_cast<int>(player.HPMax * 0.30f);
					count = 0;
				}

				boostE = player.level / 3 * 2;

				// ======================================== BackGround Update ========================================
				background1.move(0.f, 2.f * dt * multiplier);
				background2.move(0.f, 2.f * dt * multiplier);
				if (background1.getPosition().y > window.getSize().y)
				{
					background1.setPosition(Vector2f(0.f, -1075.f));
				}
				if (background2.getPosition().y > window.getSize().y)
				{
					background2.setPosition(Vector2f(0.f, -1075.f));
				}

				// ======================================== UI Update ========================================
				scoreText.setString("Score : " + std::to_string(score));
				bulletText.setString("Bullet : " + std::to_string(player.boostBullet));
				levelText.setString("Level : " + std::to_string(player.level));
				expText.setString("EXP : " + std::to_string(player.exp) + "/" + std::to_string(player.level * 100));
				skPointText.setString("Upgrade Points : " + std::to_string(player.skPoint));
				skPointText.setCharacterSize(20);
				skPointText.setPosition(10.f, 910.f);

				//HP bar
				float healthBarWidth = (static_cast<float>(player.HP) / player.HPMax) * 300.f;
				healthBar.setSize(Vector2f(healthBarWidth, 20.f));
			}

			if (player.HP <= 0)
			{
				updateHighScores(player.name, score);
				currentState = GameState::GameOver;
			}

			//Drag
			if (player.curVelocity.x > 0.f)
			{
				player.curVelocity.x -= player.drag * dt * multiplier;

				if (player.curVelocity.x < 0.f)
					player.curVelocity.x = 0.f;
			}
			else if (player.curVelocity.x < 0.f)
			{
				player.curVelocity.x += player.drag * dt * multiplier;

				if (player.curVelocity.x > 0.f)
					player.curVelocity.x = 0.f;
			}
			if (player.curVelocity.y > 0.f)
			{
				player.curVelocity.y -= player.drag * dt * multiplier;

				if (player.curVelocity.y < 0.f)
					player.curVelocity.y = 0.f;
			}
			else if (player.curVelocity.y < 0.f)
			{
				player.curVelocity.y += player.drag * dt * multiplier;

				if (player.curVelocity.y > 0.f)
					player.curVelocity.y = 0.f;
			}

			//Final move
			player.shape.move(player.curVelocity.x * dt * multiplier, player.curVelocity.y * dt * multiplier);

			//DRAW
			window.clear();
			window.draw(background1);
			window.draw(background2);

			//Player
			window.draw(player.shape);

			//Bullets
			for (size_t i = 0; i < player.bullets.size(); i++)
			{
				window.draw(player.bullets[i].shape);
			}

			//Enemies
			for (size_t i = 0; i < enemies.size(); i++)
			{
				window.draw(enemies[i].shape);
			}
			for (size_t i = 0; i < enemies2.size(); i++)
			{
				window.draw(enemies2[i].shape);
			}

			//UI
			window.draw(scoreText);
			window.draw(bulletText);
			window.draw(healthBar);
			window.draw(levelText);
			window.draw(expText);
			window.draw(skPointText);

			//Item
			for (size_t i = 0; i < items.size(); i++)
			{
				window.draw(items[i].shape);
			}

			window.display();
		}

		if (currentState == GameState::Paused)
		{
			if (currentPauseState == PauseState::Resume)
				drawText(window, "Resume", fontUI, 48, Vector2f(30.f, 600.f), Color::Red);
			else
				drawText(window, "Resume", fontUI, 48, Vector2f(30.f, 600.f), Color::White);

			if (currentPauseState == PauseState::Restart)
				drawText(window, "Restart", fontUI, 48, Vector2f(30.f, 725.f), Color::Red);
			else
				drawText(window, "Restart", fontUI, 48, Vector2f(30.f, 725.f), Color::White);

			if (currentPauseState == PauseState::Mainmenu)
				drawText(window, "Main Menu", fontUI, 48, Vector2f(30.f, 850.f), Color::Red);
			else
				drawText(window, "Main Menu", fontUI, 48, Vector2f(30.f, 850.f), Color::White);

			window.display();
		}

		if (currentState == GameState::StatUpgrade)
		{
			skPointText.setPosition(450.f, 575.f);
			skPointText.setCharacterSize(36);

			bulletDmgText.setString("Fire Power : " + std::to_string(player.bulletDmg + skDamage));
			MaxHpText.setString("Max HP : " + std::to_string(player.HPMax));
			shootSpeedText.setString("Fire Speed : " + std::to_string(fireSpeed + skSpeed));
			skPointText.setString("Upgrade Points : " + std::to_string(player.skPoint));
			hpLevelText.setString("Level : " + std::to_string(hpLevel));
			dmgLevelText.setString("Level : " + std::to_string(dmgLevel));
			speedLevelText.setString("Level : " + std::to_string(speedLevel));

			window.draw(dimSmall);
			window.draw(HP);
			window.draw(Dmg);
			window.draw(Speed);
			drawText(window, "Z", fontUI, 18, Vector2f(640.f, 480.f), Color::White);
			drawText(window, "X", fontUI, 18, Vector2f(940.f, 480.f), Color::White);
			drawText(window, "C", fontUI, 18, Vector2f(1240.f, 480.f), Color::White);
			window.draw(skPointText);
			window.draw(bulletDmgText);
			window.draw(MaxHpText);
			window.draw(shootSpeedText);
			window.draw(hpLevelText);
			window.draw(dmgLevelText);
			window.draw(speedLevelText);
			window.display();
		}

		if (currentState == GameState::MainMenu)
		{
			window.clear();

			window.draw(mainMenuBackground);
			player.name = "";
			drawText(window, "STAR RAIDER", fontLogo, 144, Vector2f(50.f, 50.f), Color::White);

			if (currentMenuState == MainMenuState::Play)
				drawText(window, "Play", fontUI, 48, Vector2f(30.f, 500.f), Color::Red);
			else
				drawText(window, "Play", fontUI, 48, Vector2f(30.f, 500.f), Color::White);

			if (currentMenuState == MainMenuState::Scoreboard)
				drawText(window, "Scoreboard", fontUI, 48, Vector2f(30.f, 625.f), Color::Red);
			else
				drawText(window, "Scoreboard", fontUI, 48, Vector2f(30.f, 625.f), Color::White);

			if (currentMenuState == MainMenuState::HowToPlay)
				drawText(window, "How To Play", fontUI, 48, Vector2f(30.f, 750.f), Color::Red);
			else
				drawText(window, "How To Play", fontUI, 48, Vector2f(30.f, 750.f), Color::White);

			if (currentMenuState == MainMenuState::Credit)
				drawText(window, "Credit", fontUI, 48, Vector2f(30.f, 875.f), Color::Red);
			else
				drawText(window, "Credit", fontUI, 48, Vector2f(30.f, 875.f), Color::White);

			if (currentMenuState == MainMenuState::Exit)
				drawText(window, "Exit", fontUI, 48, Vector2f(30.f, 1000.f), Color::Red);
			else
				drawText(window, "Exit", fontUI, 48, Vector2f(30.f, 1000.f), Color::White);

			window.display();
		}

		if (currentState == GameState::ScoreBoard)
		{
			window.clear();
			window.draw(mainMenuBackground);

			drawText(window, "Scoreboard", fontUI, 48, Vector2f(window.getSize().x / 2 - 155.f, 75.f), Color::White);
			drawText(window, "Name", fontUI, 36, Vector2f(500.f, 175.f), Color::White);
			drawText(window, "Score", fontUI, 36, Vector2f(1350.f, 175.f), Color::White);

			for (size_t i = 0; i < highScores.size(); i++)
			{
				std::string playerNameText = highScores[i].playerName;
				std::string scoreText = std::to_string(highScores[i].score);
				drawText(window, playerNameText, fontUI, 36, Vector2f(500.f, 275.f + i * 60.f), Color::White);
				drawText(window, scoreText, fontUI, 36, Vector2f(1350.f, 275.f + i * 60.f), Color::White);
			}

			drawText(window, "<<< ESC", fontUI, 24, Vector2f(50.f, 975.f), Color::White);

			window.display();
		}

		if (currentState == GameState::HowToPlay)
		{
			window.clear();
			window.draw(mainMenuBackground);

			drawText(window, "How to Play:", fontUI, 36, Vector2f(30.f, 600.f), Color::White);
			drawText(window, "1. W, A, S, D : MOVE", fontUI, 24, Vector2f(30.f, 650.f), Color::White);
			drawText(window, "2. K : FIRE", fontUI, 24, Vector2f(30.f, 700.f), Color::White);
			drawText(window, "3. J : CHANGE FIRE MODE", fontUI, 24, Vector2f(30.f, 750.f), Color::White);
			drawText(window, "4. U : OPEN UPGRADE PAGE", fontUI, 24, Vector2f(30.f, 800.f), Color::White);
			drawText(window, "<<< ESC", fontUI, 24, Vector2f(50.f, 975.f), Color::White);

			window.display();
		}

		if (currentState == GameState::Credit)
		{
			window.clear();
			window.draw(mainMenuBackground);

			drawText(window, "Credit", fontUI, 48, Vector2f(900.f, 425.f), Color::White);
			drawText(window, "Saharat Anuson", fontUI, 48, Vector2f(760.f, 500.f), Color::White);
			drawText(window, "66010843", fontUI, 48, Vector2f(840.f, 550.f), Color::White);
			drawText(window, "<<< ESC", fontUI, 24, Vector2f(50.f, 975.f), Color::White);

			window.display();
		}

		if (currentState == GameState::EnterName)
		{
			window.clear();
			window.draw(mainMenuBackground);

			if (currentState == GameState::EnterName)
			{
				std::string enterNameText = "Enter Your Name : " + player.name;
				drawText(window, enterNameText, fontUI, 36, Vector2f(30.f, 600.f), Color::White);
			}

			window.display();
		}

		if (currentState == GameState::GameOver)
		{
			drawText(window, "GAME OVER", fontUI, 48, Vector2f(window.getSize().x / 2 - 200.f, window.getSize().y / 2 - 50.f), Color::Red);

			window.display();
		}
	}

	return 0;
}

void drawText(RenderWindow& window, const std::string& text, const Font& font, int characterSize, const Vector2f& position, const Color& color)
{
	Text drawText;
	drawText.setFont(font);
	drawText.setCharacterSize(characterSize);
	drawText.setFillColor(color);
	drawText.setString(text);
	drawText.setPosition(position);
	window.draw(drawText);
}

void updateHighScores(const std::string& playerName, int score)
{
	ScoreEntry entry;
	entry.playerName = playerName;
	entry.score = score;

	highScores.push_back(entry);
	std::sort(highScores.begin(), highScores.end());

	if (highScores.size() > 10)
	{
		highScores.pop_back();
	}

	saveHighScores();
}

void loadHighScores()
{
	highScores.clear();

	std::ifstream file("highscores.txt");
	if (file.is_open())
	{
		ScoreEntry entry;
		while (file >> entry.playerName >> entry.score)
		{
			highScores.push_back(entry);
		}
		file.close();
	}
}

void saveHighScores()
{
	std::ofstream file("highscores.txt");
	if (file.is_open())
	{
		for (const ScoreEntry& entry : highScores)
		{
			file << entry.playerName << " " << entry.score << std::endl;
		}
		file.close();
	}
}