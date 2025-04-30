#include <SFML/Graphics.hpp>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <string>
#include <iostream>
using namespace std;

// -------------------- CONFIGURABLE GAME CONSTANTS --------------------
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 900;

const float PLAYER_SPEED = 5.f;
const float PLAYER_START_X = 600.f;
const float PLAYER_START_Y = 700.f;
const float PLAYER_HEALTH = 1000.f;

const float ZOMBIE_SPEED = 2.f;
const int ZOMBIE_SPAWN_INTERVAL = 60;

const float BULLET_SPEED = 20.f;
const int FIRE_RATE_INTERVAL = 10;

const int INITIAL_AMMO = 100;
const int DAMAGE_PER_ZOMBIE = 100;
const int SCORE_PER_ZOMBIE = 5;

const sf::Color TEXT_COLOR = sf::Color::Red;
const sf::Color HEALTHBAR_COLOR = sf::Color::Red;
const sf::Color GAME_OVER_TEXT_COLOR = sf::Color::Red;

// -------------------- ENTITY BASE CLASS --------------------
class Entity {
protected:
    sf::Sprite sprite;
    float speed;
public:
    Entity(const sf::Texture& tex, float x, float y, float spd) : speed(spd) {
        sprite.setTexture(tex);
        sprite.setPosition(x, y);
    }
    virtual void update() = 0;
    virtual ~Entity() {}
    void draw(sf::RenderWindow& win) const { win.draw(sprite); }
    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }
    sf::Vector2f getPosition() const { return sprite.getPosition(); }
};

// -------------------- PLAYER --------------------
class Player : public Entity {
public:
    Player(const sf::Texture& tex, float x, float y) : Entity(tex, x, y, PLAYER_SPEED) {
        sprite.setScale(3.f, 3.f);
    }
    void update() override {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && sprite.getPosition().x > 0) sprite.move(-speed, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && sprite.getPosition().x < WINDOW_WIDTH - sprite.getGlobalBounds().width) sprite.move(speed, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && sprite.getPosition().y > 80) sprite.move(0, -speed);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && sprite.getPosition().y < WINDOW_HEIGHT - sprite.getGlobalBounds().height) sprite.move(0, speed);
    }
};

// -------------------- ZOMBIE --------------------
class Zombie : public Entity {
public:
    Zombie(const sf::Texture& tex, float x, float y) : Entity(tex, x, y, ZOMBIE_SPEED) {
        sprite.setScale(3.f, 3.f);
    }
    void update() override { sprite.move(0.f, speed); }
};

// -------------------- BULLET --------------------
class Bullet : public Entity {
public:
    Bullet(const sf::Texture& tex, float x, float y) : Entity(tex, x, y, BULLET_SPEED) {}
    void update() override { sprite.move(0.f, -speed); }
};

// -------------------- GAME CLASS --------------------
class Game {
private:
    sf::RenderWindow window;
    sf::Texture backgroundTex, playerTex, zombieTex, bulletTex;
    sf::Sprite background;
    sf::Font font;
    sf::Text scoreText, ammoText, gameOverText;
    sf::RectangleShape healthBar;

    Player* player;
    vector<Zombie> zombies;
    vector<Bullet> bullets;

    float health = PLAYER_HEALTH;
    int score = 0, ammo = INITIAL_AMMO;
    bool gameOver = false;
    float spawnTimer = 0, shootTimer = 0;

    void spawnZombie() {
        float x = 80 + rand() % (WINDOW_WIDTH - 160);
        zombies.emplace_back(zombieTex, x, 0);
    }

    void shootBullet() {
        float px = player->getPosition().x + player->getBounds().width / 2 - bulletTex.getSize().x / 2;
        float py = player->getPosition().y;
        bullets.emplace_back(bulletTex, px, py);
        ammo--;
    }

    void checkCollisions() {
        for (size_t i = 0; i < zombies.size();) {
            zombies[i].update();
            if (zombies[i].getBounds().intersects(player->getBounds())) {
                health -= DAMAGE_PER_ZOMBIE;
                zombies.erase(zombies.begin() + i);
            } else if (zombies[i].getPosition().y > WINDOW_HEIGHT) {
                zombies.erase(zombies.begin() + i);
            } else i++;
        }

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
    }

    void updateUI() {
        healthBar.setSize({ health, 20 });
        healthBar.setPosition(WINDOW_WIDTH / 2 - health / 2, WINDOW_HEIGHT - 40);
        scoreText.setString("Score: " + to_string(score));
        ammoText.setString("Ammo: " + to_string(ammo));
    }

    void setupText(sf::Text& text, const string& str, float x, float y, int size) {
        text.setFont(font);
        text.setString(str);
        text.setCharacterSize(size);
        text.setFillColor(TEXT_COLOR);
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
        zombies.clear();
        bullets.clear();
    }

public:
    Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Zombie Land") {
        window.setFramerateLimit(120);
        srand(time(0));

        // Load resources
        if (!backgroundTex.loadFromFile("background.png") || 
            !playerTex.loadFromFile("player.png") ||
            !zombieTex.loadFromFile("zombie.png") ||
            !bulletTex.loadFromFile("bullet.png") ||
            !font.loadFromFile("arial.ttf")) {
            window.close();
        }

        background.setTexture(backgroundTex);
        background.setScale((float)WINDOW_WIDTH / backgroundTex.getSize().x, (float)WINDOW_HEIGHT / backgroundTex.getSize().y);
        player = new Player(playerTex, PLAYER_START_X, PLAYER_START_Y);

        setupText(scoreText, "Score: 0", WINDOW_WIDTH / 2 - 180, 20, 36);
        setupText(ammoText, "Ammo: 100", WINDOW_WIDTH / 2 + 120, 20, 36);

        gameOverText.setFont(font);
        gameOverText.setString("Game Over!\nPress SPACE to Restart");
        gameOverText.setCharacterSize(60);
        gameOverText.setFillColor(TEXT_COLOR);
        sf::FloatRect bounds = gameOverText.getGlobalBounds();
        gameOverText.setOrigin(bounds.width / 2, bounds.height / 2);
        gameOverText.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

        healthBar.setFillColor(HEALTHBAR_COLOR);
        healthBar.setSize({ health, 20 });
        healthBar.setPosition(WINDOW_WIDTH / 2 - health / 2, WINDOW_HEIGHT - 40);
    }

    ~Game() { delete player; }

    void run() {
        while (window.isOpen()) {
            sf::Event e;
            while (window.pollEvent(e)) {
                if (e.type == sf::Event::Closed) window.close();
                if (gameOver && e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Space) resetGame();
            }

            if (!gameOver) {
                player->update();
                spawnTimer++; shootTimer++;

                if (spawnTimer >= ZOMBIE_SPAWN_INTERVAL) {
                    spawnZombie(); spawnTimer = 0;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && shootTimer >= FIRE_RATE_INTERVAL && ammo > 0) {
                    shootBullet(); shootTimer = 0;
                }

                if (ammo == 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::R)) ammo = INITIAL_AMMO;

                checkCollisions();
                updateUI();
                if (health <= 0) gameOver = true;
            }

            // Render all
            window.clear();
            window.draw(background);
            player->draw(window);
            for (auto& z : zombies) z.draw(window);
            for (auto& b : bullets) b.draw(window);
            window.draw(healthBar);
            window.draw(scoreText);
            window.draw(ammoText);
            if (gameOver) window.draw(gameOverText);
            window.display();
        }
    }
};

// -------------------- MAIN --------------------
int main() {
    Game game;
    game.run();
    return 0;
}
