// plasma_stars.cpp
// Combines plasma and starfield effects using SDL2 in full-screen mode
#include <SDL2/SDL.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

const int NUM_STARS = 1200;

struct Star {
    float x, y, z;
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
        "Plasma & Stars Universe",
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

    bool quit = false;
    SDL_Event e;
    int t = 0;
    // Increase speed and reduce per-frame work for better performance
    float speed = 28.0f; // increase star speed
    int plasmaStep = 2;  // skip every other pixel for plasma
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
                // Fill a small block for skipped pixels
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
        // Draw stars on top (brighter, faster)
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
            Uint8 color = (Uint8)(180 + brightness * 75); // brighter stars
            SDL_SetRenderDrawColor(renderer, color, color, color, 255);
            SDL_RenderDrawPoint(renderer, sx, sy);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(10); // reduce delay for higher FPS
        ++t;
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
