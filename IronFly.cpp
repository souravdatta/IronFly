// IronFly.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>


constexpr double GRAVITY = 9.8;
constexpr unsigned int WIDTH = 640;
constexpr unsigned int HEIGHT = 600;
const char *ironManSpriteName = "Ironman_spritesheet.png";
const char *ironManShootSound = "Gunshotrichochets.wav";
const char *enemySpriteName = "Enemy.png";
const char *backgroundSpriteName = "Background.png";

class ResourceLoader
{
private:
  std::string fileName(const char *file) {
    std::string s{file};
    return "resources/" + s;
  }
  
public:
  void loadSprite(const char *file, sf::Texture& texture, sf::Sprite& sprite)
  {
    auto f = this->fileName(file);
    if (!texture.loadFromFile(f.c_str())) {
      std::cerr << "Could not load spritesheet " << f << " ...!\n";
      std::exit(1);
    }
    
    sprite.setTexture(texture);
  }
  
  void loadSound(const char *file, sf::SoundBuffer& soundBuffer, sf::Sound& sound)
  {
    auto f = this->fileName(file);
    if (!soundBuffer.loadFromFile(f.c_str())) {
      std::cerr << "Could not load sound file " << f << " ...!\n";
      std::exit(1);
    }

    sound.setBuffer(soundBuffer);
  }
};


class Bullets {
private:
  std::vector<sf::CircleShape> circles;
public:
  Bullets() 
  {
    circles = {};
  }

  void addBullet(float x, float y) {
    auto bullet = sf::CircleShape{ 2 };
    bullet.setPosition(x, y);

    if (circles.size() > 100) {
      circles.clear();
    }

    circles.push_back(bullet);
  }

  void update() {
    for (auto& v : circles) {
      auto p = v.getPosition();
      p.x += 6.0;
      v.setPosition(p);
    }

    std::remove_if(circles.begin(), circles.end(), [=](sf::CircleShape c) -> bool {
	return c.getPosition().x >= (WIDTH + 60);
      });
  }

  void draw(sf::RenderWindow& window) {
    for (auto c : circles) {
      window.draw(c);
    }
  }

  void reset()
  {
    circles.clear();
  }

  std::vector<sf::Vector2f> positions() const
  {
    std::vector<sf::Vector2f> pos;

    for (auto c : circles) {
      pos.push_back(sf::Vector2f{ c.getPosition().x, c.getPosition().y });
    }

    return pos;
  }
};


class Enemy
{
private:
  sf::Texture texture;
  sf::Sprite sprite;
  Bullets& bulletsRef;
  std::default_random_engine generator;
  int score;
public:
  Enemy(Bullets& bullets) : bulletsRef{ bullets }
  {
    ResourceLoader loader;
    loader.loadSprite(enemySpriteName, this->texture, this->sprite);
    score = 0;
    this->place();
  }

  void place()
  {
    std::uniform_real_distribution<float> dist{ 40.0, HEIGHT - 200.0 };
    float ypos = dist(generator);
    sprite.setPosition(sf::Vector2f{ WIDTH + 70, ypos });
  }

  void update()
  {
    auto pos = sprite.getPosition();
    const auto bulletsPositions = bulletsRef.positions();

    for (auto v : bulletsPositions) {
      if ((v.x >= pos.x) && (v.x <= pos.x + 64) && (v.y >= pos.y) && (v.y <= pos.y + 64)) {
	this->place();
	score++;
	return;
      }
    }

    if (pos.x < 0) {
      this->place();
    }
    else {
      pos.x -= 0.1;
      sprite.setPosition(pos);
    }
  }

  void draw(sf::RenderWindow& window)
  {
    window.draw(sprite);
  }

  void reset()
  {
    this->place();
  }
};


class IronMan
{
private:
  sf::Texture texture;
  sf::Sprite sprite;
  sf::SoundBuffer buffer;
  sf::Sound shootSound;
  Bullets& bulletsRef;
  bool animating, shootingPose, flyover;
  sf::Clock clock;
  int curFrame;
  double curClockTime;
  int frameWidth, frameHeight;
  int animStartFrame, animEndFrame, shootFrame;
  double thrust;
  double acceleration;
  sf::Vector2f initPosition;

public:
  IronMan(Bullets& bullets) : bulletsRef{ bullets }
  {
    ResourceLoader loader;
    loader.loadSprite(ironManSpriteName, this->texture, this->sprite);
    loader.loadSound(ironManShootSound, this->buffer, this->shootSound);
    initPosition = sprite.getPosition();
    reset();
  }

  void reset()
  {
    animating = shootingPose = false;
    curFrame = 1;
    frameWidth = 128;
    frameHeight = 128;
    animStartFrame = 2;
    animEndFrame = 6;
    shootFrame = 7;
    curClockTime = 0.0;
    flyover = false;

    thrust = 100;
    acceleration = 0;
    sprite.setPosition(initPosition);
    clock.restart();
  }

  void draw(sf::RenderWindow& window) 
  {
    auto frameOffset = (curFrame - 1) * frameHeight;
    this->sprite.setTextureRect(sf::IntRect{ 0, frameOffset, frameWidth, frameHeight });
    window.draw(sprite);
  }

  void update() 
  {
    thrust -= 1.0;

    if (thrust < -600.0) {
      thrust = -600.0;
    }

    if (shootingPose) {
      curFrame = shootFrame;
      animating = false;
    }
    else if (animating) {
      auto clockTime = clock.getElapsedTime();
			
      if (clockTime.asSeconds() - curClockTime >= 0.001) {
	curClockTime = clockTime.asSeconds();

	if ((curFrame >= animStartFrame) && (curFrame <= animEndFrame)) {
	  if (curFrame == animEndFrame) {
	    curFrame = animStartFrame;
	  }
	  else {
	    curFrame += 1;
	  }
	}
	else {					
	  curFrame = animStartFrame;
	}
      }
    }
    else {
      curFrame = 1;
    }

    auto position = sprite.getPosition();
		
    if (position.y >= (HEIGHT - 128 + 14)) {
      flyover = true;
      animating = false;
      shootingPose = false;
      curFrame = 1;
      position.y = (HEIGHT - 128 + 14);
    }
    else {
      float new_y = static_cast<float>((0.98 * clock.getElapsedTime().asSeconds() * 0.1 * 0.5) - (thrust * 0.001));
      position.y += new_y;

      if (new_y > 0) {
	animating = false;
      }
      else {
	animating = true;
      }
    }
    sprite.setPosition(position);
  }

  void addThrust() 
  {
    thrust += 600.0;
    shootingPose = false;
  }

  void shoot()
  {
    shootingPose = true;

    if (!flyover) {
      shootSound.play();
      auto pos = sprite.getPosition();
      bulletsRef.addBullet(pos.x + 128, pos.y + 30);
    }
  }
};

class Background
{
private:
  sf::Texture texture;
  sf::Sprite sprite;
public:
  Background()
  {
    ResourceLoader loader;
    loader.loadSprite(backgroundSpriteName, this->texture, this->sprite);
  }

  void draw(sf::RenderWindow& window)
  {
    window.draw(sprite);
  }

  void update()
  {
  }

  void reset()
  {

  }
};


int main()
{
  sf::RenderWindow window{ sf::VideoMode{WIDTH, HEIGHT}, "IronFly" };
  window.setVerticalSyncEnabled(false);
  window.setFramerateLimit(200);

  Background background;
  Bullets bullets;
  IronMan ironMan{ bullets };
  Enemy enemy{ bullets };

  while (window.isOpen()) {
    background.update();
    bullets.update();
    ironMan.update();
    enemy.update();

    window.clear();
    background.draw(window);
    ironMan.draw(window);
    bullets.draw(window);
    enemy.draw(window);
    window.display();

    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
	window.close();
      }
      else if (event.type == sf::Event::KeyPressed) {
	auto key = event.key;
	if (key.code == sf::Keyboard::Right) {
	  ironMan.shoot();
	}
	else if (key.code == sf::Keyboard::Up) {
	  ironMan.addThrust();
	}
	else if (key.code == sf::Keyboard::R) {
	  background.reset();
	  bullets.reset();
	  ironMan.reset();
	  enemy.reset();
	}
      }
    }
  }

  return 0;
}

