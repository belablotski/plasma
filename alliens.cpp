// alliens.cpp
// Combines spiral galaxy plasma, starfield, and animated aliens using SDL2 in full-screen mode
#include <SDL2/SDL.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

const int NUM_STARS = 1200;
const int NUM_ALIENS = 8;

struct Star {
    float x, y, z;
};

struct Alien {
    float x, y, angle, speed, size, phase;
    float vx, vy; // velocity for plate movement
};

void initStar(Star& star, int screenWidth, int screenHeight) {
    star.x = (float)(rand() % screenWidth - screenWidth / 2);
    star.y = (float)(rand() % screenHeight - screenHeight / 2);
    star.z = (float)(rand() % screenWidth);
}

// Forward declare screenWidth and screenHeight for plasmaColor
int screenWidth = 0;
int screenHeight = 0;

Uint32 plasmaColor(int x, int y, int t) {
    // Spiral galaxy effect without yellow core
    float cx = 0.5f * screenWidth;
    float cy = 0.5f * screenHeight;
    float dx = x - cx;
    float dy = y - cy;
    float r = sqrtf(dx * dx + dy * dy);
    float angle = atan2f(dy, dx);
    // Spiral arms: modulate angle by radius and time
    float spiral = sinf(4.0f * angle + r * 0.025f - t * 0.012f);
    float core = expf(-r * 0.002f);
    float arms = 0.5f + 0.5f * spiral;
    float brightness = 0.15f * core + 0.85f * arms * core;
    // Color: blueish/purple arms, no yellow core
    float c = brightness;
    Uint8 rC = (Uint8)(80 * c);
    Uint8 gC = (Uint8)(40 * c);
    Uint8 bC = (Uint8)(180 * c + 60 * core);
    return (rC << 16) | (gC << 8) | bC;
}

void drawAlien(SDL_Renderer* renderer, float x, float y, float size, float phase, int t) {
    // Simple animated alien: green head, two eyes, antennae
    int headRadius = (int)(size);
    int eyeRadius = (int)(size * 0.13f);
    int eyeOffsetX = (int)(size * 0.25f);
    int eyeOffsetY = (int)(size * 0.1f);
    int antennaLen = (int)(size * 0.5f);
    float antennaWiggle = sinf(t * 0.05f + phase) * size * 0.08f;
    // Head
    SDL_SetRenderDrawColor(renderer, 60, 255, 80, 255);
    for (int dy = -headRadius; dy <= headRadius; ++dy) {
        for (int dx = -headRadius; dx <= headRadius; ++dx) {
            if (dx * dx + dy * dy <= headRadius * headRadius) {
                SDL_RenderDrawPoint(renderer, (int)x + dx, (int)y + dy);
            }
        }
    }
    // Eyes
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int e = -1; e <= 1; e += 2) {
        int ex = (int)x + e * eyeOffsetX;
        int ey = (int)y - eyeOffsetY;
        for (int dy = -eyeRadius; dy <= eyeRadius; ++dy) {
            for (int dx = -eyeRadius; dx <= eyeRadius; ++dx) {
                if (dx * dx + dy * dy <= eyeRadius * eyeRadius) {
                    SDL_RenderDrawPoint(renderer, ex + dx, ey + dy);
                }
            }
        }
    }
    // Antennae
    SDL_SetRenderDrawColor(renderer, 60, 255, 80, 255);
    for (int e = -1; e <= 1; e += 2) {
        int ax0 = (int)x + e * (eyeOffsetX / 2);
        int ay0 = (int)y - headRadius;
        int ax1 = (int)(ax0 + antennaWiggle + e * antennaLen * 0.2f);
        int ay1 = (int)(ay0 - antennaLen);
        SDL_RenderDrawLine(renderer, ax0, ay0, ax1, ay1);
        // Antenna tip
        SDL_SetRenderDrawColor(renderer, 255, 200, 60, 255);
        for (int dy = -eyeRadius / 2; dy <= eyeRadius / 2; ++dy) {
            for (int dx = -eyeRadius / 2; dx <= eyeRadius / 2; ++dx) {
                if (dx * dx + dy * dy <= (eyeRadius * eyeRadius) / 4) {
                    SDL_RenderDrawPoint(renderer, ax1 + dx, ay1 + dy);
                }
            }
        }
        SDL_SetRenderDrawColor(renderer, 60, 255, 80, 255);
    }
}

int main(int argc, char* argv[]) {
    srand((unsigned int)time(nullptr));
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return 1;
    }

    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0) {
        SDL_Log("Could not get display mode! SDL_Error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    screenWidth = displayMode.w;
    screenHeight = displayMode.h;

    SDL_Window* window = SDL_CreateWindow(
        "Aliens in the Galaxy",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        screenWidth,
        screenHeight,
        SDL_WINDOW_FULLSCREEN
    );
    if (!window) {
        SDL_Log("Window could not be created! SDL_Error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, screenWidth, screenHeight);

    // Kiosk-like settings
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_SetWindowBordered(window, SDL_FALSE);
    SDL_SetWindowAlwaysOnTop(window, SDL_TRUE);
    SDL_SetWindowGrab(window, SDL_TRUE);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_RaiseWindow(window);
    SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1");
    SDL_SetWindowInputFocus(window);

    Star stars[NUM_STARS];
    for (int i = 0; i < NUM_STARS; ++i) {
        initStar(stars[i], screenWidth, screenHeight);
    }

    Alien aliens[NUM_ALIENS];
    for (int i = 0; i < NUM_ALIENS; ++i) {
        float angle = (2 * M_PI * i) / NUM_ALIENS;
        aliens[i].x = screenWidth / 2 + cosf(angle) * screenWidth * 0.3f;
        aliens[i].y = screenHeight / 2 + sinf(angle) * screenHeight * 0.18f;
        aliens[i].angle = angle;
        aliens[i].speed = 0.002f + 0.001f * (rand() % 100) / 100.0f;
        aliens[i].size = 32.0f + 16.0f * (rand() % 100) / 100.0f;
        aliens[i].phase = (float)(rand() % 1000) / 100.0f;
        // Plate velocity: random direction and speed
        float plateAngle = (float)(rand() % 628) / 100.0f; // 0..2pi
        float plateSpeed = 1.5f + 1.5f * (rand() % 100) / 100.0f;
        aliens[i].vx = cosf(plateAngle) * plateSpeed;
        aliens[i].vy = sinf(plateAngle) * plateSpeed;
    }

    bool quit = false;
    SDL_Event e;
    int t = 0;
    float speed = 28.0f;
    int plasmaStep = 2;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
            }
        }
        // Draw faint plasma background (skip pixels for speed)
        void* pixels;
        int pitch;
        SDL_LockTexture(texture, NULL, &pixels, &pitch);
        Uint32* buf = (Uint32*)pixels;
        for (int y = 0; y < screenHeight; y += plasmaStep) {
            for (int x = 0; x < screenWidth; x += plasmaStep) {
                Uint32 color = plasmaColor(x, y, t);
                for (int dy = 0; dy < plasmaStep && y + dy < screenHeight; ++dy) {
                    for (int dx = 0; dx < plasmaStep && x + dx < screenWidth; ++dx) {
                        buf[(y + dy) * (pitch / 4) + (x + dx)] = color;
                    }
                }
            }
        }
        SDL_UnlockTexture(texture);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        // Draw stars on top
        for (int i = 0; i < NUM_STARS; ++i) {
            Star& s = stars[i];
            s.z -= speed;
            if (s.z <= 1) {
                initStar(s, screenWidth, screenHeight);
                s.z = (float)screenWidth;
            }
            int sx = (int)((s.x / s.z) * (screenWidth / 2) + screenWidth / 2);
            int sy = (int)((s.y / s.z) * (screenHeight / 2) + screenHeight / 2);
            if (sx < 0 || sx >= screenWidth || sy < 0 || sy >= screenHeight) {
                initStar(s, screenWidth, screenHeight);
                s.z = (float)screenWidth;
                continue;
            }
            float brightness = 1.0f - (s.z / screenWidth);
            if (brightness < 0) brightness = 0;
            if (brightness > 1) brightness = 1;
            Uint8 color = (Uint8)(180 + brightness * 75);
            SDL_SetRenderDrawColor(renderer, color, color, color, 255);
            SDL_RenderDrawPoint(renderer, sx, sy);
        }
        // Animate and draw aliens
        for (int i = 0; i < NUM_ALIENS; ++i) {
            // Plate movement: move in a straight line, bounce off edges
            aliens[i].x += aliens[i].vx;
            aliens[i].y += aliens[i].vy;
            if (aliens[i].x < aliens[i].size || aliens[i].x > screenWidth - aliens[i].size) aliens[i].vx = -aliens[i].vx;
            if (aliens[i].y < aliens[i].size || aliens[i].y > screenHeight - aliens[i].size) aliens[i].vy = -aliens[i].vy;
            drawAlien(renderer, aliens[i].x, aliens[i].y, aliens[i].size, aliens[i].phase, t);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(10);
        ++t;
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
