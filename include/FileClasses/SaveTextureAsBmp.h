#pragma once

struct SDL_Renderer;
struct SDL_Texture;

void SaveTextureAsBmp(SDL_Renderer* renderer, SDL_Texture* texture, const char* filename);
