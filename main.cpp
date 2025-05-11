#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <sstream>
#include <iostream>
#include <string>
using namespace sf;
using namespace std;

const int WIDTH = 1200;
const int HEIGHT = 900;

const float PLAYER_SPEED = 6.f;
const float BULLET_SPEED = 20.f;
const float DROP_SPEED = 2.f;

const int FIRE_RATE = 10;
const int INIT_AMMO = 100;
const int MAX_AMMO = 100;
const int PLAYER_HP = 500;
const int MAX_HP = 500;
const int SCORE_PER_ZOMBIE = 10;

const int HEALTH_DROP_INTERVAL = 1000;
const int HEALTH_DROP_AMOUNT = 100;

const int AMMO_DROP_INTERVAL = 1000;
const int AMMO_DROP_AMOUNT = 30;

const int DROP_START_DELAY = 1000;

const float ZOMBIE_SPEEDS[] = {0.f, 2.f, 3.f, 4.f};
const int SPAWN_INTERVALS[] = {0, 60, 30, 15};
const int ZOMBIE_DAMAGE[] = {0, 25, 50, 100};

const int MAX_BULLETS = 100;
const int MAX_ZOMBIES = 50;
const int MAX_DROPS = 20;

const Color TEXT_COLOR = Color::White;
const Color BOX_COLOR = Color(0, 0, 0, 200);
const Color BOX_OUTLINE_COLOR = Color::White;
const float BOX_OUTLINE_THICKNESS = 2.f;
const Color HEALTHBAR_COLOR = Color::Red;

const float VOLUME_BGMUSIC = 15.f;
const float VOLUME_SHOOT = 50.f;
const float VOLUME_ZOMBIE = 60.f;
const float VOLUME_GAMEOVER = 50.f;
const float VOLUME_HEAL = 50.f;
const float VOLUME_AMMO = 50.f;

class Entity {
protected:
    Sprite sprite;
    bool active;
public:
    Entity() : active(false) {}
    virtual ~Entity() {}
    virtual void update() {}
    virtual void draw(RenderWindow& window) {
        if (active) window.draw(sprite);
    }
    FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }
    Vector2f getPosition() const {
        return sprite.getPosition();
    }
    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
    }
    void move(float dx, float dy) {
        sprite.move(dx, dy);
    }
    void setActive(bool a) {
        active = a;
    }
    bool isActive() const {
        return active;
    }
    void setTexture(Texture& tex) {
        sprite.setTexture(tex);
    }
    void setScale(float s) {
        sprite.setScale(s, s);
    }
};

class Player : public Entity {
public:
    Player(Texture& tex, float x, float y) {
        setTexture(tex);
        setPosition(x, y);
        setScale(3.f);
        setActive(true);
    }
    void update() override {
        Vector2f pos = getPosition();
        FloatRect bounds = getBounds();

        if (Keyboard::isKeyPressed(Keyboard::Left)) pos.x -= PLAYER_SPEED;
        if (Keyboard::isKeyPressed(Keyboard::Right)) pos.x += PLAYER_SPEED;
        if (Keyboard::isKeyPressed(Keyboard::Up)) pos.y -= PLAYER_SPEED;
        if (Keyboard::isKeyPressed(Keyboard::Down)) pos.y += PLAYER_SPEED;

        if (pos.x < 0) pos.x = 0;
        if (pos.x > WIDTH - bounds.width) pos.x = WIDTH - bounds.width;
        if (pos.y < 80) pos.y = 80;
        if (pos.y > HEIGHT - bounds.height) pos.y = HEIGHT - bounds.height;

        setPosition(pos.x, pos.y);
    }
};

class Bullet : public Entity {
public:
    Bullet() { setActive(false); }
    Bullet(Texture& tex) { setTexture(tex); setActive(false); }
    void init(Texture& tex, float x, float y) {
        setTexture(tex);
        setPosition(x, y);
        setActive(true);
    }
    void update() override {
        if (!isActive()) return;
        move(0, -BULLET_SPEED);
        if (getPosition().y < 0) setActive(false);
    }
};

class Zombie : public Entity {
public:
    Zombie() { setActive(false); }
    Zombie(Texture& tex) { setTexture(tex); setScale(3.f); setActive(false); }
    void init(Texture& tex, float x, float y) {
        setTexture(tex);
        setPosition(x, y);
        setScale(3.f);
        setActive(true);
    }
    void update(float speed) {
        if (!isActive()) return;
        move(0, speed);
        if (getPosition().y > HEIGHT) setActive(false);
    }
};

class Drop : public Entity {
public:
    Drop() { setActive(false); }
    Drop(Texture& tex) { setTexture(tex); setScale(0.5f); setActive(false); }
    void init(Texture& tex, float x, float y) {
        setTexture(tex);
        setPosition(x, y);
        setScale(0.5f);
        setActive(true);
    }
    void update() override {
        if (!isActive()) return;
        move(0, DROP_SPEED);
        if (getPosition().y > HEIGHT) setActive(false);
    }
};

class SoundManager {
    Music bgMusic;
    SoundBuffer shootBuf, zombieBuf, gameOverBuf, healBuf, ammoBuf;
    Sound shootSound, zombieSound, gameOverSound, healSound, ammoSound;
public:
    bool load() {
        return shootBuf.loadFromFile("shoot.wav") &&
               zombieBuf.loadFromFile("zombie.wav") &&
               gameOverBuf.loadFromFile("gameover.wav") &&
               healBuf.loadFromFile("heal.wav") &&
               ammoBuf.loadFromFile("ammo.wav") &&
               bgMusic.openFromFile("bgmusic.wav");
    }
    void setup() {
        bgMusic.setLoop(true);
        bgMusic.setVolume(VOLUME_BGMUSIC);
        bgMusic.play();

        shootSound.setBuffer(shootBuf);
        shootSound.setVolume(VOLUME_SHOOT);

        zombieSound.setBuffer(zombieBuf);
        zombieSound.setVolume(VOLUME_ZOMBIE);

        gameOverSound.setBuffer(gameOverBuf);
        gameOverSound.setVolume(VOLUME_GAMEOVER);

        healSound.setBuffer(healBuf);
        healSound.setVolume(VOLUME_HEAL);

        ammoSound.setBuffer(ammoBuf);
        ammoSound.setVolume(VOLUME_AMMO);
    }
    void playShoot() { shootSound.play(); }
    void playZombie() { zombieSound.play(); }
    void playGameOver() { gameOverSound.play(); }
    void playHeal() { healSound.play(); }
    void playAmmo() { ammoSound.play(); }
};

class UI {
    Font font;
    Text text;
    RectangleShape box;
    RectangleShape healthBar;
    RectangleShape fpsBox;
    Text fpsText;
    RectangleShape menuBox;
    const float pad = 10.f;
    const float menuPad = 30.f;
public:
    UI() {
        if (!font.loadFromFile("PixelEmulator.ttf")) {
            cerr << "Error loading font PixelEmulator.ttf" << endl;
            // Handle error appropriately here
        }
        text.setFont(font);
        text.setCharacterSize(24);
        text.setFillColor(TEXT_COLOR);
        box.setFillColor(BOX_COLOR);
        box.setOutlineColor(BOX_OUTLINE_COLOR);
        box.setOutlineThickness(BOX_OUTLINE_THICKNESS);

        healthBar.setFillColor(HEALTHBAR_COLOR);

        fpsText.setFont(font);
        fpsText.setCharacterSize(20);
        fpsText.setFillColor(Color::Yellow);

        fpsBox.setFillColor(BOX_COLOR);
        fpsBox.setOutlineColor(BOX_OUTLINE_COLOR);
        fpsBox.setOutlineThickness(BOX_OUTLINE_THICKNESS);

        menuBox.setFillColor(BOX_COLOR);
        menuBox.setOutlineColor(BOX_OUTLINE_COLOR);
        menuBox.setOutlineThickness(BOX_OUTLINE_THICKNESS);
    }
    const Font& getFont() const { return font; }
    void drawBox(RenderWindow& w, const string& s, float x, float y) {
        text.setString(s);
        FloatRect b = text.getLocalBounds();
        box.setSize({b.width + 2 * pad, b.height + 2 * pad});
        box.setPosition(x, y);
        text.setPosition(x + pad, y + pad / 2);
        w.draw(box);
        w.draw(text);
    }
    void drawMetrics(RenderWindow& w, int score, int level, int ammo, float time) {
        float x = 50, y = 20;
        drawBox(w, "Score: " + to_string(score), x, y);
        x += box.getSize().x + 20;
        drawBox(w, "Level: " + to_string(level), x, y);
        x += box.getSize().x + 20;
        drawBox(w, "Ammo: " + to_string(ammo), x, y);
        x += box.getSize().x + 20;
        drawBox(w, "Time: " + to_string((int)time) + "s", x, y);
    }
    void drawHealthBar(RenderWindow& w, float health) {
        float width = MAX_HP;
        RectangleShape back({width + 2 * pad, 20 + 2 * pad});
        back.setFillColor(BOX_COLOR);
        back.setOutlineColor(BOX_OUTLINE_COLOR);
        back.setOutlineThickness(BOX_OUTLINE_THICKNESS);
        back.setPosition(WIDTH / 2.f - width / 2.f - pad, HEIGHT - 30 - pad);
        w.draw(back);

        healthBar.setSize({health, 20});
        healthBar.setPosition(WIDTH / 2.f - health / 2.f, HEIGHT - 30);
        w.draw(healthBar);
    }
    void drawFPS(RenderWindow& w, float fps) {
        ostringstream ss;
        ss.precision(2);
        ss << fixed << "FPS: " << fps;
        fpsText.setString(ss.str());

        FloatRect b = fpsText.getLocalBounds();
        fpsBox.setSize({b.width + 2 * pad, b.height + 2 * pad});
        fpsBox.setPosition(WIDTH - fpsBox.getSize().x - 10, 10);
        w.draw(fpsBox);

        fpsText.setPosition(fpsBox.getPosition().x + pad, fpsBox.getPosition().y + pad / 2);
        w.draw(fpsText);
    }
    void drawTextCentered(RenderWindow& w, const string& s, int size, float x, float y, Color color = TEXT_COLOR) {
    Text t(s, font, size);
    t.setFillColor(color);
    FloatRect bounds = t.getLocalBounds();
    t.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    t.setPosition(x, y);
    w.draw(t);
    }
    void drawMenuBox(RenderWindow& w, const string& s, int size, float x, float y) {
        Text t(s, font, size);
        t.setFillColor(TEXT_COLOR);
        FloatRect b = t.getLocalBounds();
        menuBox.setSize({b.width + 2 * menuPad, b.height + 2 * menuPad});
        menuBox.setOrigin(menuBox.getSize().x / 2.f, menuBox.getSize().y / 2.f);
        menuBox.setPosition(x, y);

        t.setOrigin(b.width / 2.f, b.height / 2.f);
        t.setPosition(x, y);

        w.draw(menuBox);
        w.draw(t);
    }
};

class Game {
    RenderWindow window;
    Texture bgTex, startTex, playerTex, zombieTex, bulletTex, healthTex, ammoTex;
    Player* player;
    Bullet bullets[MAX_BULLETS];
    Zombie zombies[MAX_ZOMBIES];
    Drop healthDrops[MAX_DROPS];
    Drop ammoDrops[MAX_DROPS];
    SoundManager soundManager;
    UI ui;

    int score, ammo, level;
    float health;
    bool gameOver, levelSelect, paused;

    Clock gameClock, fpsClock;
    int frameCount, shootTimer, spawnTimer, healthDropTimer, ammoDropTimer;
    float currentFPS, timeSurvivedAtGameOver;

public:
    Game() : window(VideoMode(WIDTH, HEIGHT), "Zombie Land") {
        srand(time(nullptr));
        bgTex.loadFromFile("background.png");
        startTex.loadFromFile("start.png");
        playerTex.loadFromFile("player.png");
        zombieTex.loadFromFile("zombie.png");
        bulletTex.loadFromFile("bullet.png");
        healthTex.loadFromFile("health.png");
        ammoTex.loadFromFile("ammo.png");

        player = new Player(playerTex, WIDTH / 2.f, HEIGHT - 100);

        for (int i = 0; i < MAX_BULLETS; i++) bullets[i] = Bullet();
        for (int i = 0; i < MAX_ZOMBIES; i++) zombies[i] = Zombie();
        for (int i = 0; i < MAX_DROPS; i++) {
            healthDrops[i] = Drop();
            ammoDrops[i] = Drop();
        }

        if (!soundManager.load())
            cerr << "Failed to load sounds\n";
        soundManager.setup();

        resetGame();
        window.setFramerateLimit(120);
    }

    ~Game() {
        delete player;
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            window.clear();

            if (levelSelect) {
                Sprite startBg(startTex);
                startBg.setScale(WIDTH / (float)startTex.getSize().x, HEIGHT / (float)startTex.getSize().y);
                window.draw(startBg);

                ui.drawTextCentered(window, "Zombie Land", 72, WIDTH / 2.f, 250.f, Color::Red);
                ui.drawMenuBox(window, "Select Level:\n1 - Easy\n2 - Medium\n3 - Hard", 36, WIDTH / 2.f, 600.f);
            } else {
                Sprite bg(bgTex);
                bg.setScale(WIDTH / (float)bgTex.getSize().x, HEIGHT / (float)bgTex.getSize().y);
                window.draw(bg);

                if (gameOver) {
                    ostringstream oss;
                    oss << "Game Over!\nScore: " << score << "\nTime Survived: " << (int)timeSurvivedAtGameOver
                        << " seconds\nPress SPACE to restart";
                    ui.drawMenuBox(window, oss.str(), 36, WIDTH / 2.f, HEIGHT / 2.f);
                } else {
                    if (!paused) {
                        updateGame();
                        drawGame();
                    } else {
                        drawGame();
                        ui.drawTextCentered(window, "Paused\nPress ESC to resume", 48, WIDTH / 2.f, HEIGHT / 2.f);
                    }
                }
            }

            // Debug test text to verify font & drawing (remove after confirming)
            /*
            Text testText;
            testText.setFont(ui.getFont());
            testText.setString("TEST TEXT");
            testText.setCharacterSize(30);
            testText.setFillColor(Color::White);
            testText.setPosition(100, 100);
            window.draw(testText);
            */

            window.display();

            frameCount++;
            if (fpsClock.getElapsedTime().asSeconds() >= 1.f) {
                currentFPS = frameCount / fpsClock.getElapsedTime().asSeconds();
                frameCount = 0;
                fpsClock.restart();
            }
        }
    }

private:
    void resetGame() {
        score = 0; ammo = INIT_AMMO; health = PLAYER_HP; level = 1;
        gameOver = false; levelSelect = true; paused = false;
        shootTimer = spawnTimer = healthDropTimer = ammoDropTimer = 0;
        timeSurvivedAtGameOver = 0;
        frameCount = 0; currentFPS = 0;
        gameClock.restart(); fpsClock.restart();

        player->setPosition(WIDTH / 2.f, HEIGHT - 100);

        for (int i = 0; i < MAX_BULLETS; i++) bullets[i].setActive(false);
        for (int i = 0; i < MAX_ZOMBIES; i++) zombies[i].setActive(false);
        for (int i = 0; i < MAX_DROPS; i++) {
            healthDrops[i].setActive(false);
            ammoDrops[i].setActive(false);
        }
    }

    void startGame(int lvl) {
        level = (lvl >= 1 && lvl <= 3) ? lvl : 1;
        gameOver = false; levelSelect = false; paused = false;
        score = 0; ammo = INIT_AMMO; health = PLAYER_HP;
        shootTimer = spawnTimer = healthDropTimer = ammoDropTimer = 0;
        timeSurvivedAtGameOver = 0;
        frameCount = 0; currentFPS = 0;
        gameClock.restart(); fpsClock.restart();

        player->setPosition(WIDTH / 2.f, HEIGHT - 100);

        for (int i = 0; i < MAX_BULLETS; i++) bullets[i].setActive(false);
        for (int i = 0; i < MAX_ZOMBIES; i++) zombies[i].setActive(false);
        for (int i = 0; i < MAX_DROPS; i++) {
            healthDrops[i].setActive(false);
            ammoDrops[i].setActive(false);
        }
    }

    void handleEvents() {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) window.close();
            else if (e.type == Event::KeyPressed) {
                if (levelSelect && e.key.code >= Keyboard::Num1 && e.key.code <= Keyboard::Num3)
                    startGame(e.key.code - Keyboard::Num0);
                else if (gameOver && e.key.code == Keyboard::Space)
                    resetGame();
                else if (!levelSelect && !gameOver && e.key.code == Keyboard::Escape)
                    paused = !paused;
            }
        }
    }

    void updateGame() {
        player->update();

        if (Keyboard::isKeyPressed(Keyboard::Space) && shootTimer >= FIRE_RATE && ammo > 0) {
            shootTimer = 0;
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!bullets[i].isActive()) {
                    Vector2f p = player->getPosition();
                    bullets[i].init(bulletTex, p.x + player->getBounds().width / 2 - bulletTex.getSize().x / 2, p.y);
                    soundManager.playShoot();
                    ammo--;
                    break;
                }
            }
        }
        shootTimer++;

        for (int i = 0; i < MAX_BULLETS; i++) bullets[i].update();

        if (spawnTimer >= SPAWN_INTERVALS[level]) {
            spawnTimer = 0;
            for (int i = 0; i < MAX_ZOMBIES; i++) {
                if (!zombies[i].isActive()) {
                    float x = rand() % (WIDTH - 100);
                    zombies[i].init(zombieTex, x, -50);
                    if (rand() % 5 == 0) soundManager.playZombie();
                    break;
                }
            }
        }
        spawnTimer++;

        float zombieSpeed = ZOMBIE_SPEEDS[level];
        for (int i = 0; i < MAX_ZOMBIES; i++) {
            if (zombies[i].isActive()) zombies[i].update(zombieSpeed);
        }

        if (gameClock.getElapsedTime().asMilliseconds() > DROP_START_DELAY) {
            if (healthDropTimer >= HEALTH_DROP_INTERVAL) {
                healthDropTimer = 0;
                for (int i = 0; i < MAX_DROPS; i++) {
                    if (!healthDrops[i].isActive()) {
                        float x = rand() % (WIDTH - 100);
                        healthDrops[i].init(healthTex, x, -50);
                        break;
                    }
                }
            }
            if (ammoDropTimer >= AMMO_DROP_INTERVAL) {
                ammoDropTimer = 0;
                for (int i = 0; i < MAX_DROPS; i++) {
                    if (!ammoDrops[i].isActive()) {
                        float x = rand() % (WIDTH - 100);
                        ammoDrops[i].init(ammoTex, x, -50);
                        break;
                    }
                }
            }
        }
        healthDropTimer++;
        ammoDropTimer++;

        for (int i = 0; i < MAX_DROPS; i++) {
            if (healthDrops[i].isActive()) healthDrops[i].update();
            if (ammoDrops[i].isActive()) ammoDrops[i].update();
        }

        // Collisions
        for (int i = 0; i < MAX_ZOMBIES; i++) {
            if (zombies[i].isActive() && zombies[i].getBounds().intersects(player->getBounds())) {
                health -= ZOMBIE_DAMAGE[level];
                zombies[i].setActive(false);
            }
        }

        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].isActive()) continue;
            for (int j = 0; j < MAX_ZOMBIES; j++) {
                if (zombies[j].isActive() && bullets[i].getBounds().intersects(zombies[j].getBounds())) {
                    zombies[j].setActive(false);
                    bullets[i].setActive(false);
                    score += SCORE_PER_ZOMBIE;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_DROPS; i++) {
            if (healthDrops[i].isActive() && healthDrops[i].getBounds().intersects(player->getBounds())) {
                health += HEALTH_DROP_AMOUNT;
                if (health > MAX_HP) health = MAX_HP;
                healthDrops[i].setActive(false);
                soundManager.playHeal();
            }
            if (ammoDrops[i].isActive() && ammoDrops[i].getBounds().intersects(player->getBounds())) {
                ammo += AMMO_DROP_AMOUNT;
                if (ammo > MAX_AMMO) ammo = MAX_AMMO;
                ammoDrops[i].setActive(false);
                soundManager.playAmmo();
            }
        }

        if (health <= 0 && !gameOver) {
            gameOver = true;
            timeSurvivedAtGameOver = gameClock.getElapsedTime().asSeconds();
            soundManager.playGameOver();
        }
    }

    void drawGame() {
        player->draw(window);
        for (int i = 0; i < MAX_BULLETS; i++) bullets[i].draw(window);
        for (int i = 0; i < MAX_ZOMBIES; i++) zombies[i].draw(window);
        for (int i = 0; i < MAX_DROPS; i++) {
            healthDrops[i].draw(window);
            ammoDrops[i].draw(window);
        }
        float elapsed = gameOver ? timeSurvivedAtGameOver : gameClock.getElapsedTime().asSeconds();
        ui.drawMetrics(window, score, level, ammo, elapsed);
        ui.drawHealthBar(window, health);
        ui.drawFPS(window, currentFPS);
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}
