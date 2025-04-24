#include <SFML/Graphics.hpp>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <string>

using namespace std;

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
    Player(float x, float y) : Entity(40.f, 40.f, sf::Color::Green, x, y, 5.f) {}

    void update() override {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && shape.getPosition().x > 0)
            shape.move(-speed, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && shape.getPosition().x < 1160)
            shape.move(speed, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && shape.getPosition().y > 80)
            shape.move(0, -speed);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && shape.getPosition().y < 760)
            shape.move(0, speed);
    }
};

class Zombie : public Entity {
public:
    Zombie(float x, float y) : Entity(30.f, 30.f, sf::Color::Magenta, x, y, 2.f) {}

    void update() override {
        shape.move(0.f, speed);
    }
};

class Bullet : public Entity {
public:
    Bullet(float x, float y) : Entity(5.f, 10.f, sf::Color::Yellow, x, y, 15.f) {}

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
        float x = 80 + rand() % 1040;
        zombies.push_back(Zombie(x, 0));
    }

    void shootBullet() {
        float px = player.getPosition().x + 17.5f;  // center of 40px wide player
        float py = player.getPosition().y;
        bullets.push_back(Bullet(px, py));
        ammo--;
    }

    void checkCollisions() {
        for (size_t i = 0; i < zombies.size();) {
            zombies[i].update();
            if (zombies[i].getBounds().intersects(player.getBounds())) {
                health -= 10;
                zombies.erase(zombies.begin() + i);
            } else if (zombies[i].getPosition().y > 800) {
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
                    score += 5;
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

    void setupText(sf::Text& text, const string& str, float x, float y, int size) {
        text.setFont(font);
        text.setString(str);
        text.setCharacterSize(size);
        text.setPosition(x, y);
        text.setFillColor(sf::Color::White);
    }

    void resetGame() {
        player = Player(600.f, 700.f);
        health = 1000.f;
        score = 0;
        ammo = 30;
        isGameOver = false;
        spawnTimer = 0;
        shootTimer = 0;
        zombies.clear();
        bullets.clear();
    }

public:
    Game() : window(sf::VideoMode(1200, 800), "Zombie Shooter"), player(600.f, 700.f),
             health(1000.f), score(0), ammo(30), isGameOver(false),
             spawnTimer(0), shootTimer(0) {
        window.setFramerateLimit(60);

        if (!font.loadFromFile("arial.ttf")) {
            window.close();
        }

        setupText(scoreText, "Score: 0", 10, 10, 24);
        setupText(ammoText, "Ammo: 30", 200, 10, 24);
        setupText(gameOverText, "Game Over!\nPress SPACE to Restart", 300, 300, 48);
        gameOverText.setFillColor(sf::Color::Red);

        healthBar.setFillColor(sf::Color::Red);
        healthBar.setPosition(60, 760);
        healthBar.setSize(sf::Vector2f(health, 20));
    }

    void run() {
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
                if (isGameOver && event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Space)
                        resetGame();
                }
            }

            if (!isGameOver) {
                player.update();

                spawnTimer += 1;
                if (spawnTimer >= 60) {
                    spawnZombie();
                    spawnTimer = 0;
                }

                shootTimer += 1;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && shootTimer >= 10 && ammo > 0) {
                    shootBullet();
                    shootTimer = 0;
                }

                if (ammo == 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
                    ammo = 30;
                }

                checkCollisions();
                updateUI();

                if (health <= 0)
                    isGameOver = true;
            }

            window.clear();
            player.draw(window);
            for (size_t i = 0; i < zombies.size(); i++) zombies[i].draw(window);
            for (size_t i = 0; i < bullets.size(); i++) bullets[i].draw(window);

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
