// starwars.cpp
// Star Wars inspired hyperspace effect using SDL2 in full-screen mode
#include <SDL2/SDL.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

const int NUM_STARS = 2000;

struct Star {
    float x, y, z, pz;
};

int screenWidth = 0;
int screenHeight = 0;

void initStar(Star& star) {
    star.x = (float)(rand() % screenWidth - screenWidth / 2);
    star.y = (float)(rand() % screenHeight - screenHeight / 2);
    star.z = (float)(screenWidth * 0.5f + rand() % (screenWidth / 2));
    star.pz = star.z;
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
        "Star Wars Hyperspace",
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
        initStar(stars[i]);
    }

    bool quit = false;
    SDL_Event e;
    float speed = 40.0f;
    int t = 0;
    bool inHyperspace = true;
    bool condensePhase = false;
    bool approachPhase = false;
    bool showDeathStar = false;
    int hyperspaceFrames = 100; // ~2.5 seconds
    int condenseFrames = 80;    // ~0.8 seconds
    int approachFrames = 120;   // ~1.2 seconds
    float condenseX = screenWidth / 2.0f;
    float condenseY = screenHeight / 2.0f;
    float condenseRadius = 8.0f;
    float approachRadius = screenHeight * 0.32f;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        if (inHyperspace) {
            // Draw hyperspace stars
            for (int i = 0; i < NUM_STARS; ++i) {
                Star& s = stars[i];
                s.pz = s.z;
                s.z -= speed;
                if (s.z < 1) {
                    initStar(s);
                    s.z = s.pz = (float)screenWidth;
                }
                float sx = (s.x / s.z) * (screenWidth / 2) + screenWidth / 2;
                float sy = (s.y / s.z) * (screenHeight / 2) + screenHeight / 2;
                float px = (s.x / s.pz) * (screenWidth / 2) + screenWidth / 2;
                float py = (s.y / s.pz) * (screenHeight / 2) + screenHeight / 2;
                float brightness = 1.0f - (s.z / (screenWidth * 0.5f));
                if (brightness < 0) brightness = 0;
                if (brightness > 1) brightness = 1;
                Uint8 color = (Uint8)(200 + brightness * 55);
                SDL_SetRenderDrawColor(renderer, color, color, color, 255);
                SDL_RenderDrawLine(renderer, (int)px, (int)py, (int)sx, (int)sy);
            }
            // Faint galaxy background
            for (int y = 0; y < screenHeight; y += 4) {
                for (int x = 0; x < screenWidth; x += 4) {
                    float cx = 0.5f * screenWidth;
                    float cy = 0.5f * screenHeight;
                    float dx = x - cx;
                    float dy = y - cy;
                    float r = sqrtf(dx * dx + dy * dy);
                    float angle = atan2f(dy, dx);
                    float spiral = sinf(4.0f * angle + r * 0.025f - t * 0.012f);
                    float core = expf(-r * 0.002f);
                    float arms = 0.5f + 0.5f * spiral;
                    float brightness = 0.08f * core + 0.25f * arms * core;
                    float c = brightness;
                    Uint8 rC = (Uint8)(40 * c);
                    Uint8 gC = (Uint8)(20 * c);
                    Uint8 bC = (Uint8)(90 * c + 30 * core);
                    SDL_SetRenderDrawColor(renderer, rC, gC, bC, 255);
                    SDL_RenderDrawPoint(renderer, x, y);
                }
            }
            --hyperspaceFrames;
            if (hyperspaceFrames <= 0) {
                inHyperspace = false;
                condensePhase = true;
            }
        } else if (condensePhase) {
            // Move all stars toward the center and draw them as a bright green object
            bool allArrived = true;
            for (int i = 0; i < NUM_STARS; ++i) {
                Star& s = stars[i];
                float sx = (s.x / s.z) * (screenWidth / 2) + screenWidth / 2;
                float sy = (s.y / s.z) * (screenHeight / 2) + screenHeight / 2;
                float dx = condenseX - sx;
                float dy = condenseY - sy;
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist > condenseRadius) {
                    sx += dx * 0.12f;
                    sy += dy * 0.12f;
                    allArrived = false;
                }
                SDL_SetRenderDrawColor(renderer, 80, 255, 80, 255);
                SDL_RenderDrawPoint(renderer, (int)sx, (int)sy);
                // Update star's position for next frame
                s.x = (sx - screenWidth / 2) * s.z / (screenWidth / 2);
                s.y = (sy - screenHeight / 2) * s.z / (screenHeight / 2);
            }
            // Draw a very bright green core
            SDL_SetRenderDrawColor(renderer, 120, 255, 120, 255);
            for (int y = -condenseRadius; y <= condenseRadius; ++y) {
                for (int x = -condenseRadius; x <= condenseRadius; ++x) {
                    if (x * x + y * y <= condenseRadius * condenseRadius) {
                        SDL_RenderDrawPoint(renderer, (int)(condenseX + x), (int)(condenseY + y));
                    }
                }
            }
            --condenseFrames;
            if (condenseFrames <= 0 || allArrived) {
                condensePhase = false;
                approachPhase = true;
            }
        } else if (approachPhase) {
            // Animate the green core growing larger (approaching)
            approachRadius += (screenHeight * 0.32f - approachRadius) * 0.12f + 1.0f;
            SDL_SetRenderDrawColor(renderer, 120, 255, 120, 255);
            for (int y = -approachRadius; y <= approachRadius; ++y) {
                for (int x = -approachRadius; x <= approachRadius; ++x) {
                    if (x * x + y * y <= approachRadius * approachRadius) {
                        SDL_RenderDrawPoint(renderer, (int)(condenseX + x), (int)(condenseY + y));
                    }
                }
            }
            if (approachRadius >= screenHeight * 0.32f - 2.0f) {
                approachPhase = false;
                showDeathStar = true;
            }
        } else if (showDeathStar) {
            // Draw the Death Star (greenish)
            float cx = screenWidth / 2.0f;
            float cy = screenHeight / 2.0f;
            float radius = screenHeight * 0.32f;
            // Main body
            SDL_SetRenderDrawColor(renderer, 120, 255, 120, 255);
            for (int y = -radius; y <= radius; ++y) {
                for (int x = -radius; x <= radius; ++x) {
                    if (x * x + y * y <= radius * radius) {
                        SDL_RenderDrawPoint(renderer, (int)(cx + x), (int)(cy + y));
                    }
                }
            }
            // Equatorial trench
            SDL_SetRenderDrawColor(renderer, 60, 180, 60, 255);
            for (int y = -4; y <= 4; ++y) {
                for (int x = -radius; x <= radius; ++x) {
                    if ((x * x + y * y <= radius * radius) && (abs(y) < 4)) {
                        SDL_RenderDrawPoint(renderer, (int)(cx + x), (int)(cy + y));
                    }
                }
            }
            // Superlaser dish
            float dishR = radius * 0.28f;
            float dishX = cx + radius * 0.38f;
            float dishY = cy - radius * 0.22f;
            SDL_SetRenderDrawColor(renderer, 40, 180, 40, 255);
            for (int y = -dishR; y <= dishR; ++y) {
                for (int x = -dishR; x <= dishR; ++x) {
                    if (x * x + y * y <= dishR * dishR) {
                        SDL_RenderDrawPoint(renderer, (int)(dishX + x), (int)(dishY + y));
                    }
                }
            }
            // Dish highlight
            SDL_SetRenderDrawColor(renderer, 180, 255, 180, 255);
            for (int y = -dishR/2; y <= dishR/2; ++y) {
                for (int x = -dishR/2; x <= dishR/2; ++x) {
                    if (x * x + y * y <= (dishR * dishR) / 4) {
                        SDL_RenderDrawPoint(renderer, (int)(dishX + x - dishR/4), (int)(dishY + y - dishR/4));
                    }
                }
            }
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(10);
        ++t;
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
