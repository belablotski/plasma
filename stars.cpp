// A simple cross-platform starfield (flight through stars) effect using SDL2 in full-screen mode
#include <SDL2/SDL.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

const int NUM_STARS = 2000;

struct Star {
    float x, y, z;
};

void initStar(Star& star, int screenWidth, int screenHeight) {
    star.x = (float)(rand() % screenWidth - screenWidth / 2);
    star.y = (float)(rand() % screenHeight - screenHeight / 2);
    star.z = (float)(rand() % screenWidth);
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
    int screenWidth = displayMode.w;
    int screenHeight = displayMode.h;

    SDL_Window* window = SDL_CreateWindow(
        "Starfield Effect",
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
        initStar(stars[i], screenWidth, screenHeight);
    }

    bool quit = false;
    SDL_Event e;
    float speed = 10.0f;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
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
            Uint8 color = (Uint8)(brightness * 255);
            SDL_SetRenderDrawColor(renderer, color, color, color, 255);
            SDL_RenderDrawPoint(renderer, sx, sy);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
