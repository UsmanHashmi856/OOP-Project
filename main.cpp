#include <SFML/Graphics.hpp>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <string>
using namespace std;

// CONFIGURABLE GAME CONSTANTS 

// Game Window
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;

// Player
const float PLAYER_WIDTH = 40.f;
const float PLAYER_HEIGHT = 40.f;
const float PLAYER_SPEED = 10.f;
const float PLAYER_START_X = 600.f;
const float PLAYER_START_Y = 700.f;
const float PLAYER_HEALTH = 1000.f;
const sf::Color PLAYER_COLOR = sf::Color::Green;

// Zombie
const float ZOMBIE_WIDTH = 30.f;
const float ZOMBIE_HEIGHT = 30.f;
const float ZOMBIE_SPEED = 2.f;
const sf::Color ZOMBIE_COLOR = sf::Color::Magenta;
const int ZOMBIE_SPAWN_INTERVAL = 60; // in frames

// Bullet
const float BULLET_WIDTH = 5.f;
const float BULLET_HEIGHT = 10.f;
const float BULLET_SPEED = 20.f;
const sf::Color BULLET_COLOR = sf::Color::Yellow;
const int FIRE_RATE_INTERVAL = 10; // in frames

// Game Settings
const int INITIAL_AMMO = 100;
const int DAMAGE_PER_ZOMBIE = 10;
const int SCORE_PER_ZOMBIE = 5;

// UI
const sf::Color TEXT_COLOR = sf::Color::White;
const sf::Color HEALTHBAR_COLOR = sf::Color::Red;
const sf::Color GAME_OVER_TEXT_COLOR = sf::Color::Red;
// ================================================================

class Entity {
protected:
    sf::RectangleShape shape;
    float speed;

public:
    Entity(float w, float h, sf::Color color, float x, float y, float spd) : speed(spd) {
        shape.setSize(sf::Vector2f(w, h));
        shape.setFillColor(color);
        shape.setPosition(x, y);
    }

    virtual void update() = 0;
    virtual ~Entity() {}

    void draw(sf::RenderWindow& win) const { win.draw(shape); }
    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); }
    sf::Vector2f getPosition() const { return shape.getPosition(); }
    void setPosition(float x, float y) { shape.setPosition(x, y); }
};

class Player : public Entity {
public:
    Player(float x, float y) 
    : Entity(PLAYER_WIDTH, PLAYER_HEIGHT, PLAYER_COLOR, x, y, PLAYER_SPEED) {}

    void update() override {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && shape.getPosition().x > 0)
            shape.move(-speed, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && shape.getPosition().x < WINDOW_WIDTH - PLAYER_WIDTH)
            shape.move(speed, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && shape.getPosition().y > 80)
            shape.move(0, -speed);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && shape.getPosition().y < WINDOW_HEIGHT - PLAYER_HEIGHT)
            shape.move(0, speed);
    }
};

class Zombie : public Entity {
public:
    Zombie(float x, float y) 
    : Entity(ZOMBIE_WIDTH, ZOMBIE_HEIGHT, ZOMBIE_COLOR, x, y, ZOMBIE_SPEED) {}

    void update() override {
        shape.move(0.f, speed);
    }
};

class Bullet : public Entity {
public:
    Bullet(float x, float y) 
    : Entity(BULLET_WIDTH, BULLET_HEIGHT, BULLET_COLOR, x, y, BULLET_SPEED) {}

    void update() override {
        shape.move(0.f, -speed);
    }
};

class Game {
private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Text scoreText, ammoText, gameOverText;
    sf::RectangleShape healthBar;

    Player player;
    vector<Zombie> zombies;
    vector<Bullet> bullets;

    float health;
    int score;
    int ammo;
    bool isGameOver;

    float spawnTimer, shootTimer;

    void spawnZombie() {
        float x = 80 + rand() % (WINDOW_WIDTH - 160);
        zombies.push_back(Zombie(x, 0));
    }

    void shootBullet() {
        float px = player.getPosition().x + (PLAYER_WIDTH - BULLET_WIDTH) / 2;
        float py = player.getPosition().y;
        bullets.push_back(Bullet(px, py));
        ammo--;
    }

    void checkCollisions() {
        for (size_t i = 0; i < zombies.size();) {
            zombies[i].update();
            if (zombies[i].getBounds().intersects(player.getBounds())) {
                health -= DAMAGE_PER_ZOMBIE;
                zombies.erase(zombies.begin() + i);
            } else if (zombies[i].getPosition().y > WINDOW_HEIGHT) {
                zombies.erase(zombies.begin() + i);
            } else {
                i++;
            }
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
            else
                i++;
        }
    }

    void updateUI() {
        healthBar.setSize(sf::Vector2f(health, 20));
        scoreText.setString("Score: " + to_string(score));
        ammoText.setString("Ammo: " + to_string(ammo));
    }

    void setupText(sf::Text& text, const string& str, float x, float y, int size, sf::Color color) {
        text.setFont(font);
        text.setString(str);
        text.setCharacterSize(size);
        text.setPosition(x, y);
        text.setFillColor(color);
    }

    void resetGame() {
        player = Player(PLAYER_START_X, PLAYER_START_Y);
        health = PLAYER_HEALTH;
        score = 0;
        ammo = INITIAL_AMMO;
        isGameOver = false;
        spawnTimer = 0;
        shootTimer = 0;
        zombies.clear();
        bullets.clear();
    }

public:
    Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Zombie Land"), player(PLAYER_START_X, PLAYER_START_Y),
             health(PLAYER_HEALTH), score(0), ammo(INITIAL_AMMO), isGameOver(false),
             spawnTimer(0), shootTimer(0) {
        window.setFramerateLimit(60);

        if (!font.loadFromFile("arial.ttf")) {
            window.close();
        }

        setupText(scoreText, "Score: 0", 10, 10, 24, TEXT_COLOR);
        setupText(ammoText, "Ammo: 100", 200, 10, 24, TEXT_COLOR);
        setupText(gameOverText, "Game Over!\nPress SPACE to Restart", 300, 300, 48, GAME_OVER_TEXT_COLOR);

        healthBar.setFillColor(HEALTHBAR_COLOR);
        healthBar.setPosition(60, WINDOW_HEIGHT - 40);
        healthBar.setSize(sf::Vector2f(health, 20));
    }

    void run() {
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
                if (isGameOver && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
                    resetGame();
            }

            if (!isGameOver) {
                player.update();

                spawnTimer += 1;
                if (spawnTimer >= ZOMBIE_SPAWN_INTERVAL) {
                    spawnZombie();
                    spawnTimer = 0;
                }

                shootTimer += 1;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && shootTimer >= FIRE_RATE_INTERVAL && ammo > 0) {
                    shootBullet();
                    shootTimer = 0;
                }

                if (ammo == 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
                    ammo = INITIAL_AMMO;
                }

                checkCollisions();
                updateUI();

                if (health <= 0)
                    isGameOver = true;
            }

            window.clear();
            player.draw(window);
            for (Zombie& z : zombies) z.draw(window);
            for (Bullet& b : bullets) b.draw(window);
            window.draw(healthBar);
            window.draw(scoreText);
            window.draw(ammoText);
            if (isGameOver) window.draw(gameOverText);
            window.display();
        }
    }
};

int main() {
    srand(static_cast<unsigned>(time(0)));
    Game game;
    game.run();
    return 0;
}
