// Matrix falling letters effect using SDL2 in full-screen mode
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdlib>
#include <ctime>
#include <vector>

const int FONT_SIZE = 18;
const int TRAIL_LENGTH = 18;
const int SYMBOLS = 96; // printable ASCII
// Download font from https://github.com/powerline/fonts/blob/master/DejaVuSansMono/DejaVu%20Sans%20Mono%20for%20Powerline.ttf
const char * const FONT_NAME = "DejaVu Sans Mono for Powerline.ttf"; // "DejaVuSansMono.ttf";

// Simple font: use SDL_Rendwget https://github.com/dejavu-fonts/dejavu-fonts/raw/version_2_37/ttf/DejaVuSansMono.ttferDrawText if available, else draw rectangles as glyphs
char randomChar() {
    return (char)(32 + rand() % SYMBOLS); // printable ASCII
}

struct Column {
    int x;
    float y;
    float speed;
    std::vector<char> trail;
};

int screenWidth = 0;
int screenHeight = 0;

int main(int argc, char* argv[]) {
    srand((unsigned int)time(nullptr));
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return 1;
    }
    if (TTF_Init() < 0) {
        SDL_Log("SDL_ttf could not initialize! TTF_Error: %s", TTF_GetError());
        SDL_Quit();
        return 1;
    }
    TTF_Font* font = TTF_OpenFont(FONT_NAME, FONT_SIZE);
    if (!font) {
        SDL_Log("Failed to load font! TTF_Error: %s", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
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
        "Matrix Rain",
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

    int cols = screenWidth / FONT_SIZE;
    std::vector<Column> columns(cols);
    for (int i = 0; i < cols; ++i) {
        columns[i].x = i * FONT_SIZE;
        columns[i].y = rand() % screenHeight;
        columns[i].speed = 2.0f + (rand() % 100) / 40.0f;
        columns[i].trail.resize(TRAIL_LENGTH);
        for (int j = 0; j < TRAIL_LENGTH; ++j) {
            columns[i].trail[j] = randomChar();
        }
    }

    bool quit = false;
    SDL_Event e;
    int t = 0;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
            }
        }
        // Fade effect: draw a translucent black rectangle over the screen
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 60);
        SDL_RenderFillRect(renderer, NULL);
        // Draw falling letters
        for (int i = 0; i < cols; ++i) {
            Column& col = columns[i];
            col.y += col.speed;
            if (col.y > screenHeight + TRAIL_LENGTH * FONT_SIZE) {
                col.y = -(rand() % (TRAIL_LENGTH * FONT_SIZE));
                col.speed = 2.0f + (rand() % 100) / 40.0f;
            }
            // Update trail
            for (int j = TRAIL_LENGTH - 1; j > 0; --j) {
                col.trail[j] = col.trail[j - 1];
            }
            col.trail[0] = randomChar();
            // Draw trail
            for (int j = 0; j < TRAIL_LENGTH; ++j) {
                int y = (int)col.y - j * FONT_SIZE;
                if (y < 0 || y > screenHeight) continue;
                SDL_Color color;
                if (j == 0) {
                    color = {180, 255, 180, 255}; // head
                } else {
                    color = {0, 255, 70, (Uint8)(255 - j * (200 / TRAIL_LENGTH))};
                }
                char text[2] = {col.trail[j], '\0'};
                SDL_Surface* textSurface = TTF_RenderText_Blended(font, text, color);
                if (textSurface) {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                    if (textTexture) {
                        SDL_Rect dstRect = {col.x, y, FONT_SIZE, FONT_SIZE};
                        SDL_RenderCopy(renderer, textTexture, NULL, &dstRect);
                        SDL_DestroyTexture(textTexture);
                    }
                    SDL_FreeSurface(textSurface);
                }
            }
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
        ++t;
    }
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
// NOTE: Place a monospaced TTF font (e.g., DejaVuSansMono.ttf) in the project directory for best results.
