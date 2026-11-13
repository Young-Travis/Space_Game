#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Graphics/GpuRenderer.hpp"
#include "Math/Quaternion.hpp"
#include "cockpit/Cockpit.hpp"
#include "ships/Enemy.hpp"
#include "ships/Laser.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <memory>
#include <vector>

class Player {
public:
    explicit Player(float maximumHealth = 10.0f, float laserSpeed = 180.0f, float laserDamage = 25.0f);
    virtual ~Player() = default;

    virtual void update(float deltaTime);
    virtual void reset();
    virtual void handleInput(float deltaTime);
    virtual void updateLasers(float deltaTime, const Vector3& playerDisplacement, std::vector<std::unique_ptr<Enemy>>& enemies);

    void yaw(float angle);
    void pitch(float angle);
    void roll(float angle);
    void thrust(float amount);
    virtual void shoot();
    virtual void renderWeapons(const GpuRenderer& renderer) const;
    virtual void renderHud(sf::RenderWindow& window) const;

    const Quaternion& getOrientation() const;
    float getThrottle() const;
    const Mesh& getCockpitMesh() const;
    Vector3 getCameraShakeOffset() const;
    virtual void takeHit(float damage = 1.0f);
    virtual float getHealth() const;
    float getMaximumHealth() const;
    virtual int getScore() const;
    virtual bool isDestroyed() const;

private:
    Quaternion orientation;
    float throttle{};
    float shotCooldown{};
    float maximumHealth;
    float health;
    float laserSpeed;
    float laserDamage;
    int score{};
    float hitShakeTimer{};
    float hitShakePhase{};
    sf::Font hudFont;
    bool hudFontLoaded{};
    Cockpit cockpit;
    std::vector<std::unique_ptr<Laser>> lasers;

protected:
    bool beginFiring();
    void fireLaser(const Vector3& origin, const Vector3& direction);
    void addScore(int points);
    void createLaserPair();
    void handleDefaultControls(float deltaTime);
    virtual void onTakeDamage(float damage);
    virtual void onEnemyHit(Enemy& enemy, float damage);

    std::unique_ptr<Laser> createLaser(const Vector3& origin, const Vector3& direction) const;
};

#endif