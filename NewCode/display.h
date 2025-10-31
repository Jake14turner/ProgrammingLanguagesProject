#pragma once
#include <SDL.h>
#include <string>
#include <SDL_ttf.h>
#include "simulator.h"


class Display {
    public:
    Simulation* simulation;
    int width;
    int height;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Color bgColor;
    TTF_Font* font;
    bool initialized;


    Display(Simulation* sim, int width, int height);
    ~Display();

    bool render();
    void cleanup();
    void drawText(const std::string& text, int x, int y, SDL_Color color);
};