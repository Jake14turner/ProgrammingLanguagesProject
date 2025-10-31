#include "display.h"
#include "agents.h"
#include "simulator.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>

const int FRAMES_PER_SECOND = 10;

Display::Display(Simulation* sim, int width, int height)
    : simulation(sim), width(width), height(height) {
    initialized = false;
    window = nullptr;
    renderer = nullptr;
    font = nullptr;
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        return;
    }
    
    if (TTF_Init() < 0) {
        std::cerr << "TTF Init Error: " << TTF_GetError() << std::endl;
        return;
    }
    
    window = SDL_CreateWindow("Agent Movement Simulation",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              width, height,
                              SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        return;
    }
    
    font = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 15);
    if (!font) {
        std::cerr << "Font loading failed: " << TTF_GetError() << std::endl;
    }
    
    bgColor = {255, 255, 255, 255};
    initialized = true;
}

Display::~Display() {
    cleanup();
}

void Display::drawText(const std::string& text, int x, int y, SDL_Color color) {
    if (!font || !renderer) return;
    
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect dest = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

bool Display::render() {
    if (!initialized || !window || !renderer) {
        return false;
    }
for (const auto& [id, h] : simulation->human_agents) {
    std::cout << "  Human " << id << " at (" << h->location.x << ", " << h->location.y << ")" << std::endl;
}
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
    }
    
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderClear(renderer);

    for (auto* agent : simulation->animal_agents) {
        int x = static_cast<int>(agent->location.x);
        int y = static_cast<int>(agent->location.y);
        int radius = static_cast<int>(agent->radius);
        
        SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
        
        for (int w = 0; w < radius * 2; w++) {
            for (int h = 0; h < radius * 2; h++) {
                int dx = radius - w;
                int dy = radius - h;
                if ((dx*dx + dy*dy) <= (radius * radius)) {
                    SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                }
            }
        }
        
        SDL_Color green = {0, 200, 0, 255};
        std::string label = "A" + std::to_string(agent->id);
        drawText(label, x, y - (radius + 10), green);
    }
    
    for (const auto& [id, agent] : simulation->human_agents) {
        int x = static_cast<int>(agent->location.x);
        int y = static_cast<int>(agent->location.y);
        int radius = 5;
        
        SDL_Color color;
        if (agent->status == HumanStatus::SICK) {
            color = {255, 0, 0, 255};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        } else {
            color = {0, 0, 255, 255}; 
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        }
        
        for (int w = 0; w < radius * 2; w++) {
            for (int h = 0; h < radius * 2; h++) {
                int dx = radius - w;
                int dy = radius - h;
                if ((dx*dx + dy*dy) <= (radius * radius)) {
                    SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                }
            }
        }
        

        std::string label = "H" + std::to_string(agent->id);
        drawText(label, x, y - (radius + 10), color);
    }
    

    SDL_Color black = {0, 0, 0, 255};
    std::string simTime = "Sim time: t=" + std::to_string(simulation->time_step);
    drawText(simTime, 10, 10, black);
    
    std::string realTime = "Real time: t=" + std::to_string(static_cast<int>(simulation->get_current_real_time())) + "s";
    drawText(realTime, 10, 25, black);
    
    SDL_RenderPresent(renderer);
    

    SDL_Delay(1000 / FRAMES_PER_SECOND);
    
    return true;
}

void Display::cleanup() {
    if (font) { TTF_CloseFont(font); font = nullptr; }
    if (renderer) { SDL_DestroyRenderer(renderer); renderer = nullptr; }
    if (window) { SDL_DestroyWindow(window); window = nullptr; }
    TTF_Quit();
    SDL_Quit();
    initialized = false;
}