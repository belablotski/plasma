#include <SDL2/SDL.h>
#include <cmath>
#include <cstdlib>

// Generate a color based on plasma effect
Uint32 plasmaColor(int x, int y, int t) {
    float value = 
        128.0f + (128.0f * sin(x / 16.0f)) +
        128.0f + (128.0f * sin(y / 8.0f)) +
        128.0f + (128.0f * sin((x + y + t) / 16.0f)) +
        128.0f + (128.0f * sin(sqrtf(x * x + y * y) / 8.0f));
    value /= 4.0f;
    Uint8 r = (Uint8)(128 + 127 * sin(0.02f * value + t * 0.02f));
    Uint8 g = (Uint8)(128 + 127 * sin(0.02f * value + 2 + t * 0.02f));
    Uint8 b = (Uint8)(128 + 127 * sin(0.02f * value + 4 + t * 0.02f));
    return (r << 16) | (g << 8) | b;
}

int main(int argc, char* argv[]) {
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
        "Plasma Effect",
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

    // Enable exclusive fullscreen and disable window decorations for kiosk mode
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_SetWindowBordered(window, SDL_FALSE);
    SDL_SetWindowAlwaysOnTop(window, SDL_TRUE);

    bool quit = false;
    SDL_Event e;
    int t = 0;
    // Grab input and hide cursor for kiosk mode
    SDL_SetWindowGrab(window, SDL_TRUE);
    SDL_ShowCursor(SDL_DISABLE);
    // Try to grab keyboard focus
    SDL_RaiseWindow(window);
    SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1");
    SDL_SetWindowInputFocus(window);
    // Event loop
    while (!quit) {
        // Only scan for ESC key
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
            }
        }
        void* pixels;
        int pitch;
        SDL_LockTexture(texture, NULL, &pixels, &pitch);
        Uint32* buf = (Uint32*)pixels;
        for (int y = 0; y < screenHeight; ++y) {
            for (int x = 0; x < screenWidth; ++x) {
                buf[y * (pitch / 4) + x] = plasmaColor(x, y, t);
            }
        }
        SDL_UnlockTexture(texture);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
        ++t;
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
