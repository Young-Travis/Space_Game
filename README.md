## requirements:

------

CMAKE

SFML

-----

I could not figure out a way to package SFML and CMAKE together in the project. I spent a while researching this and could not figure out a way to do this. Even if I could, it wouldn't create a graphical interface for the workspace users.

Theoretically it is possible to package SFML into the project to be built with the game. However, this would not be useful unless you used a local IDE.

Because of this limitation:
THIS PROJECT WILL NOT WORK IN CODESPACES *sorry :(*

## TUTORIAL
Unfortunately, because of the limitations above, it does require some installs. With this, I recommend the following steps if wanted:

It is important to note, that a setup similar to this is required to run C code, if you used similar tools in the programming in C class, then you may be able to just use the setup you had previously. This will probably take quite a bit of time, so I don't really recommend doing this during the class hour.

If you use a linux distro, you don't need to install the compatibility software (MYSYS2). You can do everything from your terminal.

## Student Gameplay Classes

The game creates `StudentPlayer`, `StudentEnemy`, and `StudentLaser`, which derive from the engine's `Player`, `Enemy`, and `Laser` classes. The base classes provide rendering, collision helpers, movement physics, health storage, and projectile management.

In `StudentPlayer.cpp`:

- `shoot()` connects player input to the base helper `Player::createLaserPair()`. The base `Player` owns the cooldown, aiming vectors, muzzle positions, and laser construction.
- `onTakeDamage(float damage)` is called when an enemy laser hits the player; call `Player::onTakeDamage(damage)` to apply the built-in health and hit feedback.
- `onEnemyHit(Enemy& enemy, float damage)` connects the player's laser collision to `enemy.takeDamage()` and the base score helper.
- `handleInput(float deltaTime)` can call `Player::handleDefaultControls(deltaTime)` for the sample controls without handling key mappings or angle math.

Edit the named settings near the top of the student class headers to tune game rules without changing engine math:

- `StudentPlayer::MAX_HEALTH` sets player health.
- `StudentPlayer::DAMAGE_TAKEN_MULTIPLIER` changes how much health an enemy shot removes.
- `StudentPlayer::POINTS_PER_ENEMY_HIT` sets points earned per player laser hit.
- `StudentLaser::DAMAGE` and `StudentLaser::SPEED` configure player projectile damage and speed; the base `Player` applies those values when it creates each projectile.
- `StudentEnemy::MAX_HEALTH`, `LASER_DAMAGE`, and `LASER_SPEED` set enemy health and its weapon values.

The player HUD always shows ten health squares; each square represents one tenth of the configured maximum health, including when health or damage values are fractional.

In `StudentEnemy.cpp`, `chooseFlightCommand()` selects among `evadePlayer()`, `attackPlayer()`, and `orbitPlayer()`. These base helpers handle the flight math; students can change the decisions without calculating vectors. `StudentLaser.cpp` derives from `Laser`, which provides projectile movement and collision behavior.

## THIS IS ALL SUBJECT TO CHANGE

## Step 1:
Install [MSYS2](https://www.msys2.org/)
It doesn't matter where you install it, the default directory (C:\msys64) would be ok given that you have enough space on your C drive.

Essentially MSYS2 is a terminal that uses linux commands, like a compatibility tool that allows you to install c libraries to import into your code. This is required for windows users to use online libraries like the rendering library used in this project (SFML)

## Step 2:
In MSYS2 run the command:
pacman -Syu
If it prompts you to restart MSYS2, reopen it and run
pacman -Su
(These commands just update the environment)

## Step 3:
To install the necessary files to run C and C++ code on your pc you need to install the required packages. Different projects will have different requirements, the base requirements for this project are as follows.

pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-sfml

Copy and paste the entire last command into the terminal

## Step 4:
This step is going to vary depending on your IDE. 

You technically *can* use workspaces after you install MSYS2 but you need all your code in a local directory on your computer. so it will be a lot of copy and pasting to put your code in the local directory. 

However, a really popular and useful IDE is [Visual Studio Code](https://code.visualstudio.com/download) (VS Code)

Select the operating system you use and proceed with the steps. The reason why I recommend this IDE is because it makes it really easy to install extensions that help with writing code.




