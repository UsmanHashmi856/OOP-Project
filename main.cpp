#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;
using namespace sf;

// === CONFIGURABLE VALUES ===
const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 1000;
const float PLAYER_SPEED = 6.f;
const float ZOMBIE_SPEED_LEVEL[] = { 0.f, 2.0f, 3.0f, 4.0f };
const int ZOMBIE_SPAWN_INTERVAL_LEVEL[] = { 0, 60, 30, 15 };
const int DAMAGE_PER_ZOMBIE_LEVEL[] = { 0, 25, 50, 100 };
const float BULLET_SPEED = 20.f;
const int INITIAL_AMMO = 100;
const int FIRE_RATE_INTERVAL = 10;
const int PLAYER_HEALTH = 500;
const int SCORE_PER_ZOMBIE = 10;
const int PLAYER_START_X = 375;
const int PLAYER_START_Y = 500;
const int HEALTH_DROP_INTERVAL = 1000; 
const int HEALTH_DROP_AMOUNT = 100;
const float HEALTH_DROP_SPEED = 2.f;
const int AMMO_DROP_INTERVAL = 1000; 
const int AMMO_DROP_AMOUNT = 30;
const float AMMO_DROP_SPEED = 2.f;
const int MAX_HEALTH = 500;
const Color HEALTHBAR_COLOR = Color::Red;
const Color TEXT_COLOR = Color::White;

// === SOUND VOLUMES ===
const int BGMUSIC_VOLUME = 10;
const int ZOMBIE_VOLUME = 60;
const int GUNSHOT_VOLUME = 50;
const int GAMEOVER_VOLUME = 50;
const int HEAL_VOLUME = 50;
const int AMMO_PICKUP_VOLUME = 50;

class Player {
public:
    Sprite sprite;
    Player(Texture& tex, float x, float y) {
        sprite.setTexture(tex);
        sprite.setPosition(x, y);
        sprite.setScale(3.f, 3.f);
    }
    void update() {
        float speed = PLAYER_SPEED;
        if ((Keyboard::isKeyPressed(Keyboard::Left)) && sprite.getPosition().x > 0)
            sprite.move(-speed, 0);
        if ((Keyboard::isKeyPressed(Keyboard::Right)) && sprite.getPosition().x < WINDOW_WIDTH - sprite.getGlobalBounds().width)
            sprite.move(speed, 0);
        if ((Keyboard::isKeyPressed(Keyboard::Up)) && sprite.getPosition().y > 80)
            sprite.move(0, -speed);
        if ((Keyboard::isKeyPressed(Keyboard::Down)) && sprite.getPosition().y < WINDOW_HEIGHT - sprite.getGlobalBounds().height)
            sprite.move(0, speed);
    }
    void draw(RenderWindow& win) { win.draw(sprite); }
    Vector2f getPosition() { return sprite.getPosition(); }
    FloatRect getBounds() { return sprite.getGlobalBounds(); }
};

class Zombie {
public:
    Sprite sprite;
    Zombie(Texture& tex, float x, float y) {
        sprite.setTexture(tex);
        sprite.setPosition(x, y);
        sprite.setScale(3.f, 3.f);
    }
    void update(float speed) { sprite.move(0.f, speed); }
    void draw(RenderWindow& win) { win.draw(sprite); }
    Vector2f getPosition() { return sprite.getPosition(); }
    FloatRect getBounds() { return sprite.getGlobalBounds(); }
};

class Bullet {
public:
    Sprite sprite;
    Bullet(Texture& tex, float x, float y) {
        sprite.setTexture(tex);
        sprite.setPosition(x, y);
        sprite.setScale(1.f, 1.f);
    }
    void update() { sprite.move(0.f, -BULLET_SPEED); }
    void draw(RenderWindow& win) { win.draw(sprite); }
    Vector2f getPosition() { return sprite.getPosition(); }
    FloatRect getBounds() { return sprite.getGlobalBounds(); }
};

class HealthDrop {
public:
    Sprite sprite;
    HealthDrop(Texture& tex, float x, float y) {
        sprite.setTexture(tex);
        sprite.setPosition(x, y);
        sprite.setScale(0.5f, 0.5f);
    }
    void update() { sprite.move(0.f, HEALTH_DROP_SPEED); }
    void draw(RenderWindow& win) { win.draw(sprite); }
    FloatRect getBounds() { return sprite.getGlobalBounds(); }
    Vector2f getPosition() { return sprite.getPosition(); }
};

class AmmoDrop {
public:
    Sprite sprite;
    AmmoDrop(Texture& tex, float x, float y) {
        sprite.setTexture(tex);
        sprite.setPosition(x, y);
        sprite.setScale(0.5f, 0.5f);
    }
    void update() { sprite.move(0.f, AMMO_DROP_SPEED); }
    void draw(RenderWindow& win) { win.draw(sprite); }
    FloatRect getBounds() { return sprite.getGlobalBounds(); }
    Vector2f getPosition() { return sprite.getPosition(); }
};

class Game {
private:
    RenderWindow window;
    Texture backgroundTex, playerTex, zombieTex, bulletTex, healthDropTex, ammoDropTex;
    Sprite background;
    Font font;
    Text scoreText, ammoText, gameOverText, levelText, startText, timeText;
    RectangleShape healthBar;

    Player* player = nullptr;
    vector<Zombie> zombies;
    vector<Bullet> bullets;
    vector<HealthDrop> healthDrops;
    vector<AmmoDrop> ammoDrops;

    float health = PLAYER_HEALTH;
    int score = 0, ammo = INITIAL_AMMO;
    bool gameOver = false;
    float spawnTimer = 0, shootTimer = 0, healthDropTimer = 0, ammoDropTimer = 0;
    int level = 1;
    bool levelSelected = false;
    Clock gameClock;

    Music bgMusic;
    SoundBuffer zombieBuf, gunshotBuf, gameOverBuf, healBuf, ammoPickupBuf;
    Sound zombieSound, gunshotSound, gameOverSound, healSound, ammoPickupSound;

    void spawnZombie() {
        float x = 80 + rand() % (WINDOW_WIDTH - 160);
        zombies.emplace_back(zombieTex, x, 0);
        if (rand() % 5 == 0) zombieSound.play();
    }

    void shootBullet() {
        float px = player->getPosition().x + player->getBounds().width / 2 - bulletTex.getSize().x / 2;
        float py = player->getPosition().y;
        bullets.emplace_back(bulletTex, px, py);
        ammo--;
        gunshotSound.play();
    }

    void spawnHealthDrop() {
        float x = 100 + rand() % (WINDOW_WIDTH - 200);
        healthDrops.emplace_back(healthDropTex, x, 0);
    }

    void spawnAmmoDrop() {
        float x = 100 + rand() % (WINDOW_WIDTH - 200);
        ammoDrops.emplace_back(ammoDropTex, x, 0);
    }

    void checkCollisions() {
        // Player-Zombie collisions
        for (size_t i = 0; i < zombies.size();) {
            zombies[i].update(ZOMBIE_SPEED_LEVEL[level]);
            if (zombies[i].getBounds().intersects(player->getBounds())) {
                health -= DAMAGE_PER_ZOMBIE_LEVEL[level];
                zombies.erase(zombies.begin() + i);
            } else if (zombies[i].getPosition().y > WINDOW_HEIGHT) {
                zombies.erase(zombies.begin() + i);
            } else i++;
        }

        // Bullet-Zombie collisions
        for (size_t i = 0; i < bullets.size();) {
            bullets[i].update();
            bool hit = false;
            for (size_t j = 0; j < zombies.size(); j++) {
                if (bullets[i].getBounds().intersects(zombies[j].getBounds())) {
                    zombies.erase(zombies.begin() + j);
                    score += SCORE_PER_ZOMBIE;
                    hit = true;
                    break;
                }
            }
            if (hit || bullets[i].getPosition().y < 0)
                bullets.erase(bullets.begin() + i);
            else i++;
        }

        // Health drop collisions
        for (size_t i = 0; i < healthDrops.size();) {
            healthDrops[i].update();
            if (healthDrops[i].getBounds().intersects(player->getBounds())) {
                health += HEALTH_DROP_AMOUNT;
                if (health > MAX_HEALTH) health = MAX_HEALTH;
                healthDrops.erase(healthDrops.begin() + i);
                healSound.play();
            } else if (healthDrops[i].getPosition().y > WINDOW_HEIGHT) {
                healthDrops.erase(healthDrops.begin() + i);
            } else i++;
        }

        // Ammo drop collisions
        for (size_t i = 0; i < ammoDrops.size();) {
            ammoDrops[i].update();
            if (ammoDrops[i].getBounds().intersects(player->getBounds())) {
                ammo += AMMO_DROP_AMOUNT;
                ammoDrops.erase(ammoDrops.begin() + i);
                ammoPickupSound.play();
            } else if (ammoDrops[i].getPosition().y > WINDOW_HEIGHT) {
                ammoDrops.erase(ammoDrops.begin() + i);
            } else i++;
        }
    }

    void updateUI() {
        healthBar.setSize({ health, 20 });
        healthBar.setPosition(WINDOW_WIDTH / 2 - health / 2, WINDOW_HEIGHT - 40);

        scoreText.setString("Score: " + to_string(score));
        centerText(scoreText, WINDOW_WIDTH / 2 - 250, 20);

        ammoText.setString("Ammo: " + to_string(ammo));
        centerText(ammoText, WINDOW_WIDTH / 2 + 250, 20);

        levelText.setString("Level: " + to_string(level));
        centerText(levelText, WINDOW_WIDTH / 2, 60);

        int seconds = (int)gameClock.getElapsedTime().asSeconds();
        timeText.setString("Time: " + to_string(seconds) + "s");
        centerText(timeText, WINDOW_WIDTH / 2, 20);
    }

    void setupText(Text& text, const string& str, float x, float y, int size) {
        text.setFont(font);
        text.setString(str);
        text.setCharacterSize(size);
        text.setFillColor(TEXT_COLOR);
        centerText(text, x, y);
    }

    void centerText(Text& text, float x, float y) {
        FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.width / 2, bounds.height / 2);
        text.setPosition(x, y);
    }

    void resetGame() {
        delete player;
        player = new Player(playerTex, PLAYER_START_X, PLAYER_START_Y);
        health = PLAYER_HEALTH;
        score = 0;
        ammo = INITIAL_AMMO;
        gameOver = false;
        spawnTimer = 0;
        shootTimer = 0;
        healthDropTimer = 0;
        ammoDropTimer = 0;
        zombies.clear();
        bullets.clear();
        healthDrops.clear();
        ammoDrops.clear();
        gameClock.restart();
    }

public:
    Game() : window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Zombie Land") {
        window.setFramerateLimit(120);
        srand(time(0));

        if (!backgroundTex.loadFromFile("background.png") ||
            !playerTex.loadFromFile("player.png") ||
            !zombieTex.loadFromFile("zombie.png") ||
            !bulletTex.loadFromFile("bullet.png") ||
            !healthDropTex.loadFromFile("health.png") ||
            !ammoDropTex.loadFromFile("ammo.png") ||  // New texture for ammo drops
            !font.loadFromFile("arial.ttf") ||
            !bgMusic.openFromFile("bgmusic.wav") ||
            !zombieBuf.loadFromFile("zombie.wav") ||
            !gunshotBuf.loadFromFile("shoot.wav") ||
            !gameOverBuf.loadFromFile("gameover.wav") ||
            !healBuf.loadFromFile("heal.wav") ||
            !ammoPickupBuf.loadFromFile("ammo.wav")) {  // New sound for ammo pickup
            window.close();
        }

        background.setTexture(backgroundTex);
        background.setScale((float)WINDOW_WIDTH / backgroundTex.getSize().x, (float)WINDOW_HEIGHT / backgroundTex.getSize().y);

        setupText(scoreText, "Score: 0", WINDOW_WIDTH / 2 - 250, 20, 36);
        setupText(ammoText, "Ammo: 100", WINDOW_WIDTH / 2 + 250, 20, 36);
        setupText(levelText, "Level: 1", WINDOW_WIDTH / 2, 60, 36);
        setupText(startText, "Select Level:\n1 - Easy\n2 - Medium\n3 - Hard", WINDOW_WIDTH / 2, 250, 48);
        setupText(timeText, "Time: 0s", WINDOW_WIDTH / 2, 20, 36);

        gameOverText.setFont(font);
        gameOverText.setString("Game Over!\nPress SPACE to Restart");
        gameOverText.setCharacterSize(60);
        gameOverText.setFillColor(TEXT_COLOR);
        centerText(gameOverText, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

        healthBar.setFillColor(HEALTHBAR_COLOR);

        bgMusic.setLoop(true);
        bgMusic.setVolume(BGMUSIC_VOLUME);
        bgMusic.play();

        zombieSound.setBuffer(zombieBuf);
        zombieSound.setVolume(ZOMBIE_VOLUME);

        gunshotSound.setBuffer(gunshotBuf);
        gunshotSound.setVolume(GUNSHOT_VOLUME);

        gameOverSound.setBuffer(gameOverBuf);
        gameOverSound.setVolume(GAMEOVER_VOLUME);

        healSound.setBuffer(healBuf);
        healSound.setVolume(HEAL_VOLUME);

        ammoPickupSound.setBuffer(ammoPickupBuf);  // Initialize ammo pickup sound
        ammoPickupSound.setVolume(AMMO_PICKUP_VOLUME);
    }

    ~Game() { delete player; }

    void run() {
        while (window.isOpen()) {
            Event e;
            while (window.pollEvent(e)) {
                if (e.type == Event::Closed) window.close();
                if (gameOver && e.type == Event::KeyPressed && e.key.code == Keyboard::Space) {
                    resetGame();
                    bgMusic.play();
                }
            }

            if (!levelSelected) {
                if (Keyboard::isKeyPressed(Keyboard::Num1)) { level = 1; levelSelected = true; player = new Player(playerTex, PLAYER_START_X, PLAYER_START_Y); gameClock.restart(); }
                if (Keyboard::isKeyPressed(Keyboard::Num2)) { level = 2; levelSelected = true; player = new Player(playerTex, PLAYER_START_X, PLAYER_START_Y); gameClock.restart(); }
                if (Keyboard::isKeyPressed(Keyboard::Num3)) { level = 3; levelSelected = true; player = new Player(playerTex, PLAYER_START_X, PLAYER_START_Y); gameClock.restart(); }
            }

            if (levelSelected && !gameOver) {
                player->update();
                spawnTimer++; shootTimer++; healthDropTimer++; ammoDropTimer++;

                if (spawnTimer >= ZOMBIE_SPAWN_INTERVAL_LEVEL[level]) {
                    spawnZombie(); spawnTimer = 0;
                }

                if (healthDropTimer >= HEALTH_DROP_INTERVAL) {
                    spawnHealthDrop(); healthDropTimer = 0;
                }

                if (ammoDropTimer >= AMMO_DROP_INTERVAL) {
                    spawnAmmoDrop(); ammoDropTimer = 0;
                }

                if (Keyboard::isKeyPressed(Keyboard::Space) && shootTimer >= FIRE_RATE_INTERVAL && ammo > 0) {
                    shootBullet(); shootTimer = 0;
                }

                checkCollisions();
                updateUI();
                if (health <= 0) {
                    gameOver = true;
                    gameOverSound.play();
                }
            }

            window.clear();
            window.draw(background);
            if (!levelSelected)
                window.draw(startText);
            else {
                player->draw(window);
                for (auto& z : zombies) z.draw(window);
                for (auto& b : bullets) b.draw(window);
                for (auto& h : healthDrops) h.draw(window);
                for (auto& a : ammoDrops) a.draw(window);  // Draw ammo drops
                window.draw(healthBar);
                window.draw(scoreText);
                window.draw(ammoText);
                window.draw(levelText);
                window.draw(timeText);
                if (gameOver) window.draw(gameOverText);
            }
            window.display();
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}
