#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>

int screenWidth = 1200;  // Screen width
int screenHeight = 600; // Screen height
int characterWidth = 50; // Width of the character
int characterHeight = 50; // Height of the character

int player1Wins = 0;
int player2Wins = 0;
int currentRound = 1;

// Structure for Player with all states and animations
typedef struct {
    float x, y;             // Position
    float vx, vy;           // Velocity
    int health;             // Health
    int isJumping;          // Jumping state
    int isAttackingLight;   // Light attack state
    int isDefending;        // Defense state
    int facingRight;        // 1 for right, 0 for left
    int walkFrame, idleFrame, jumpFrame, lightAttackFrame, heavyAttackFrame; // Animation frames
    Uint32 lastAnimTime;    // Animation timer
} Player;

typedef struct {
    float x, y;             // Position
    float vx;               // Velocity (horizontal)
    int isActive;           // Active state
    SDL_Texture* texture;   // Texture
    int damage;             // Damage amount
} Projectile;

// Global variables
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// Textures for players, projectiles, and background
SDL_Texture* player1IdleTexture = NULL;
SDL_Texture* player1WalkTexture = NULL;
SDL_Texture* player1LightAttackTexture = NULL;
SDL_Texture* player2IdleTexture = NULL;
SDL_Texture* player2WalkTexture = NULL;
SDL_Texture* player2LightAttackTexture = NULL;
SDL_Texture* backgroundTexture = NULL;
SDL_Texture* lightProjectileTexture = NULL;
SDL_Texture* player1JumpTexture = NULL;
SDL_Texture* player2JumpTexture = NULL;
SDL_Texture* player1DefenseTexture = NULL;
SDL_Texture* player2DefenseTexture = NULL;
SDL_Texture* startScreenTexture = NULL;
SDL_Texture* startButtonTexture = NULL;
SDL_Texture* quitButtonTexture = NULL;

// Initialize SDL
void InitSDL() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("2-Player Fighting Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1200, 750, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

// Load texture
SDL_Texture* LoadTexture(const char* file) {
    SDL_Surface* tempSurface = IMG_Load(file);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    return texture;
}
void ResetPlayers(Player* player1, Player* player2) {
    // Reset player states
    player1->x = 50;
    player1->y = 500;
    player1->vx = 0;
    player1->vy = 0;
    player1->health = 100;
    player1->isJumping = 0;

    player2->x = 990;
    player2->y = 500;
    player2->vx = 0;
    player2->vy = 0;
    player2->health = 100;
    player2->isJumping = 0;
}

void CheckRoundWinner(Player* player1, Player* player2) {
    if (player1->health <= 0) {
        player2Wins++;
        SDL_Delay(1000); // Pause for 1 second to show the result
    }
    else if (player2->health <= 0) {
        player1Wins++;
        SDL_Delay(1000); // Pause for 1 second to show the result
    }

    if (player1Wins == 2 || player2Wins == 2) {
        // End game if one player wins 2 rounds
        printf("Player %d wins the match!\n", player1Wins == 2 ? 1 : 2);
        SDL_Quit();
        exit(0);
    }
    else {
        // Reset players and start the next round
        currentRound++;
        printf("Starting round %d!\n", currentRound);
        ResetPlayers(player1, player2);
    }
}

int StartScreen() {
    int choice = 0;  // 0 = no choice, 1 = start game, -1 = quit
    SDL_Event event;

    // Load start screen background and button textures
    SDL_Texture* startScreenTexture = LoadTexture("C:/Users/HP/Downloads/Sprite/StartScreenImage.png");  // Background image
    SDL_Texture* startButtonTexture = LoadTexture("C:/Users/HP/Downloads/Sprite/Start Game.png");  // Start button image
    SDL_Texture* quitButtonTexture = LoadTexture("C:/Users/HP/Downloads/Sprite/Exit Game.png");    // Quit button image

    // Define positions and sizes for the "Start Game" and "Quit" buttons
    SDL_Rect startButtonRect = { 400, 300, 400, 100 };  // Position and size of the "Start Game" button
    SDL_Rect quitButtonRect = { 500, 450, 200, 100 };   // Position and size of the "Quit" button

    while (!choice) {
        // Event handling for clicking the "Start Game" or "Quit" button
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                choice = -1;  // Quit the game if the window is closed
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;

                // Check if the "Start Game" button was clicked
                if (mouseX >= startButtonRect.x && mouseX <= startButtonRect.x + startButtonRect.w &&
                    mouseY >= startButtonRect.y && mouseY <= startButtonRect.y + startButtonRect.h) {
                    choice = 1;  // Start the game
                }

                // Check if the "Quit" button was clicked
                if (mouseX >= quitButtonRect.x && mouseX <= quitButtonRect.x + quitButtonRect.w &&
                    mouseY >= quitButtonRect.y && mouseY <= quitButtonRect.y + quitButtonRect.h){
                    choice = -1;  // Quit the application
                }
            }
        }

        // Render the start screen background
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, startScreenTexture, NULL, NULL);  // Full-screen background

        // Render the "Start Game" and "Quit" buttons as images
        SDL_RenderCopy(renderer, startButtonTexture, NULL, &startButtonRect);  // Draw "Start Game" button image
        SDL_RenderCopy(renderer, quitButtonTexture, NULL, &quitButtonRect);    // Draw "Quit" button image

        SDL_RenderPresent(renderer);
    }

    // Cleanup start screen and button textures
    SDL_DestroyTexture(startScreenTexture);
    SDL_DestroyTexture(startButtonTexture);
    SDL_DestroyTexture(quitButtonTexture);

    return choice;
}
// Render a texture at position with width and height
void RenderTexture(SDL_Texture* texture, int x, int y, int width, int height) {
    SDL_Rect dstRect = { x, y, width, height };
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
}

// Render player's health bar
void RenderHealthBar(Player* player, int x, int y) {
    int healthBarWidth = 200;
    int healthBarHeight = 20;
    int healthWidth = (player->health * healthBarWidth) / 100;

    // Draw the background of the health bar (dark gray)
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Dark gray background
    SDL_Rect healthBarBg = { x, y, healthBarWidth, healthBarHeight };
    SDL_RenderFillRect(renderer, &healthBarBg);

    // Draw the current health in different colors
    if (player->health > 60) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green
    }
    else if (player->health > 30) {
        SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255); // Orange
    }
    else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red
    }

    SDL_Rect healthBar = { x, y, healthWidth, healthBarHeight };
    SDL_RenderFillRect(renderer, &healthBar);
}

// Update animation for player
void UpdatePlayerAnimation(Player* player) {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - player->lastAnimTime > 100) { // Every 100ms change frame
        if (player->vx != 0) {
            player->walkFrame = (player->walkFrame + 1) % 4;  // Cycle through walk frames (4 frames)
        }
        else {
            player->idleFrame = (player->idleFrame + 1) % 4;  // Cycle through idle frames (4 frames)
        }
        player->lastAnimTime = currentTime;
    }
}

// Apply gravity to players
void ApplyGravity(Player* player) {
    if (player->isJumping) {
        player->y += player->vy;
        player->vy += 1; // Gravity effect
        if (player->y >= 500) { // Ground level
            player->y = 500;
            player->vy = 0;
            player->isJumping = 0;
        }
    }
}

// Throw projectile
void ThrowProjectile(Projectile* projectile, Player* player, SDL_Texture* projectileTexture, int damage) {
    if (!projectile->isActive && player->isAttackingLight) {
        projectile->x = player->x + (player->facingRight ? 30 : -50);  // Fire in direction of player
        projectile->y = player->y + 100;  // Slightly offset for better visual
        projectile->vx = player->facingRight ? 10 : -10;  // Horizontal velocity
        projectile->isActive = 1;
        projectile->texture = projectileTexture;
        projectile->damage = damage;

        player->isAttackingLight = 0;  // Reset attack state to prevent continuous firing
    }
}

// Update projectile position
void UpdateProjectile(Projectile* projectile, Player* target) {
    if (projectile->isActive) {
        projectile->x += projectile->vx;
        if (projectile->x < 0 || projectile->x > 1200) projectile->isActive = 0; // Deactivate if out of bounds
        if (projectile->x >= target->x && projectile->x <= target->x + 50 && projectile->y >= target->y && projectile->y <= target->y + 100) {
            if (!target->isDefending) { // Only damage if not defending
                target->health -= projectile->damage; // Reduce health on hit
            }
            projectile->isActive = 0;
        }
    }
}
void HandlePlayerMovement(Player* player, SDL_Keycode leftKey, SDL_Keycode rightKey, SDL_Keycode jumpKey, SDL_Keycode attackKey, SDL_Keycode defendKey) {
    const Uint8* keystate = SDL_GetKeyboardState(NULL);

    // Adjusted movement speed to a lower value (e.g., 2)
    if (keystate[SDL_GetScancodeFromKey(leftKey)]) {
        player->vx = -2;  // Reduced horizontal speed
        player->facingRight = 0;
    }
    else if (keystate[SDL_GetScancodeFromKey(rightKey)]) {
        player->vx = 2;  // Reduced horizontal speed
        player->facingRight = 1;
    }
    else {
        player->vx = 0;
    }

    if (keystate[SDL_GetScancodeFromKey(jumpKey)] && !player->isJumping) {
        player->vy = -10;
        player->isJumping = 1;
    }

    // Handle attack trigger (fire projectile) only once per key press
    if (keystate[SDL_GetScancodeFromKey(attackKey)] && !player->isAttackingLight) {
        player->isAttackingLight = 1;  // Start the attack (can only trigger once per key press)
    }

    if (keystate[SDL_GetScancodeFromKey(defendKey)]) {
        player->isDefending = 1;
    }
    else {
        player->isDefending = 0;
    }

    if (player->x < 0) {
        player->x = 0;  // Left boundary
    }
    else if (player->x + 150 > screenWidth) {
        player->x = screenWidth - 150;  // Right boundary
    }
    // Update player's position based on velocity for screen movement
    player->x += player->vx;
    player->y += player->vy;
}



// Main game loop
void GameLoop() {
    Player player1 = { 50, 500, 0, 0, 100, 0, 0, 0, 1, 0, 0, 0, 0, 0 };
    Player player2 = { 990, 500, 0, 0, 100, 0, 0, 0, 1, 0, 0, 0, 0, 0 };

    Projectile player1Projectile = { 0, 0, 0, 0, NULL, 10 };
    Projectile player2Projectile = { 0, 0, 0, 0, NULL, 10 };

    // Define round rectangles (You can adjust positions and sizes as needed)
    SDL_Rect roundRects[3] = {
        {500, 50, 50, 20},  // Round 1 rectangle
        {570, 50, 50, 20},  // Round 2 rectangle
        {640, 50, 50, 20}   // Round 3 rectangle
    };

    int quit = 0;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = 1;
        }
        
        // Handle player input
        HandlePlayerMovement(&player1, SDLK_a, SDLK_d, SDLK_w, SDLK_q, SDLK_s);
        HandlePlayerMovement(&player2, SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_RCTRL, SDLK_DOWN);

        // Update player animations
        UpdatePlayerAnimation(&player1);
        UpdatePlayerAnimation(&player2);

        // Apply gravity
        ApplyGravity(&player1);
        ApplyGravity(&player2);

        // Fire projectiles
        ThrowProjectile(&player1Projectile, &player1, lightProjectileTexture, 10);
        ThrowProjectile(&player2Projectile, &player2, lightProjectileTexture, 10);

        // Update projectile positions
        UpdateProjectile(&player1Projectile, &player2);
        UpdateProjectile(&player2Projectile, &player1);

        if (player1.health <= 0 || player2.health <= 0) {
            CheckRoundWinner(&player1, &player2);
        }

        // Render the game scene
        SDL_RenderClear(renderer);
        RenderTexture(backgroundTexture, 0, 0, 1200, 800);

        // Render players based on their state
        if (player1.vx != 0) {
            RenderTexture(player1WalkTexture, player1.x, player1.y, 150, 200);
        }
        else if (player1.isAttackingLight) {
            RenderTexture(player1LightAttackTexture, player1.x, player1.y, 150, 200);  // Render attack texture
        }
        else if (player1.isJumping) {
            RenderTexture(player1JumpTexture, player1.x, player1.y, 150, 200);
        }
        else if (player1.isDefending) {
            RenderTexture(player1DefenseTexture, player1.x, player1.y, 150, 200);
        }
        else {
            RenderTexture(player1IdleTexture, player1.x, player1.y, 150, 200);
        }

        if (player2.vx != 0) {
            RenderTexture(player2WalkTexture, player2.x, player2.y, 150, 200);
        }
        else if (player2.isAttackingLight) {
            RenderTexture(player2LightAttackTexture, player2.x, player2.y, 150, 200);  // Render attack texture
        }
        else if (player2.isJumping) {
            RenderTexture(player2JumpTexture, player2.x, player2.y, 150, 200);
        }
        else if (player2.isDefending) {
            RenderTexture(player2DefenseTexture, player2.x, player2.y, 150, 200);
        }
        else {
            RenderTexture(player2IdleTexture, player2.x, player2.y, 150, 200);
        }

        // Render projectiles if active
        if (player1Projectile.isActive) RenderTexture(player1Projectile.texture, player1Projectile.x, player1Projectile.y, 40, 40);
        if (player2Projectile.isActive) RenderTexture(player2Projectile.texture, player2Projectile.x, player2Projectile.y, 40, 40);

        // Render health bars
        RenderHealthBar(&player1, 50, 20);
        RenderHealthBar(&player2, 950, 20);

        // Render round status rectangles
        for (int i = 0; i < 3; i++) {
            // Check which player's color should be applied to the box
            if (i < player1Wins) {
                // Player 1 wins this round, turn red
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red for Player 1
            }
            else if (i < player2Wins + player1Wins) {
                // Player 2 wins this round, turn blue
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);  // Blue for Player 2
            }
            else {
                // This round has not been won by either player, keep it gray
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);  // Gray for undecided rounds
            }
            SDL_RenderFillRect(renderer, &roundRects[i]);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);  // ~60 FPS
    }
}


// Main function
int main(int argc, char* argv[]) {
    InitSDL();
    int startChoice = StartScreen();
    if (startChoice == -1) {  // Quit if the user chose "Quit" or closed the window
        SDL_Quit();
        return 0;
    }
    player1IdleTexture = LoadTexture("C:/Users/HP/Downloads/Sprite/sprite31.png");
    player1WalkTexture = LoadTexture("C:/Users/HP/Downloads/sprite/sprite21.png");
    player1JumpTexture = LoadTexture("C:/Users/HP/Downloads/Sprite/sprite51.png");
    player1LightAttackTexture = LoadTexture("C:/Users/HP/Downloads/Sprite/sprite32.png");
    player1DefenseTexture = LoadTexture("C:/Users/HP/Downloads/Sprite/sprite1.png");


    player2IdleTexture = LoadTexture("C:/Users/HP/Downloads/Sprite/Dew1.png");
    player2WalkTexture = LoadTexture("C:/Users/HP/Downloads/Sprite/Dew2.png");
    player2JumpTexture = LoadTexture("C:/Users/HP/Downloads/Sprite/Dew3.png");
    player2LightAttackTexture = LoadTexture("C:/Users/HP/Downloads/Sprite/Dew4.png");
    player2DefenseTexture = LoadTexture("C:/Users/HP/Downloads/Sprite/Dew5.png");

    backgroundTexture = LoadTexture("C:/Users/HP/Downloads/Sprite/Background2.png");
    lightProjectileTexture = LoadTexture("C:/Users/HP/Downloads/Sprite/sprite102.png");
    // Start game loop
    GameLoop();

    // Clean up and quit SDL
    SDL_DestroyTexture(player1IdleTexture);
    SDL_DestroyTexture(player1WalkTexture);
    SDL_DestroyTexture(player1LightAttackTexture);
    SDL_DestroyTexture(player1DefenseTexture);
    SDL_DestroyTexture(player2IdleTexture);
    SDL_DestroyTexture(player2WalkTexture);
    SDL_DestroyTexture(player2LightAttackTexture);
    SDL_DestroyTexture(player2DefenseTexture);
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(lightProjectileTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
