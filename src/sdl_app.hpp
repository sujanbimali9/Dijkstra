#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <functional>
#include <iostream>

using namespace std;
class SdlApp {
private:
  SDL_Window *window;
  SDL_Renderer *renderer;
  TTF_Font *font;

public:
  SdlApp(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font);
  SdlApp();
  ~SdlApp();

  int init(int width, int height, const string &title = "SdlApp");
  int drawButton(int x, int y, int rad, SDL_Color color);
  int drawButtonFilled(int x, int y, int rad, SDL_Color color);
  int drawLine(int x1, int y1, int x2, int y2, SDL_Color color);
  int renderText(const string &text, int x, int y, SDL_Color color);
  int run(function<void(SDL_Event event)> eventHandler, const int fps = 60);
  int textSize(const char *text, int *w, int *h);
};

SdlApp::SdlApp(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font)
    : window(window), renderer(renderer), font(font) {}
SdlApp::SdlApp() : window(nullptr), renderer(nullptr), font(nullptr) {};

SdlApp::~SdlApp() {
  if (font) {
    TTF_CloseFont(font);
  }
  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
  if (window) {
    SDL_DestroyWindow(window);
  }
  TTF_Quit();
  SDL_Quit();
}

int SdlApp::init(int width, int height, const string &title) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    cerr << "SDL Initialization failed: " << SDL_GetError() << endl;
    return 1;
  }
  if (TTF_Init() != 0) {
    cerr << "FONT Initialization failed: " << TTF_GetError() << endl;
    return 1;
  }

  window =
      SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);

  if (!window) {
    cerr << "Window creation failed: " << SDL_GetError() << endl;
    return 1;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (!renderer) {
    cerr << "Renderer creation failed: " << SDL_GetError() << endl;
    return 1;
  }

  font = TTF_OpenFont("roboto.ttf", 16);

  if (!font) {
    cerr << "Font loading failed: " << TTF_GetError() << endl;
    return 1;
  }

  return 0;
}

int SdlApp::drawButton(int x, int y, int rad, SDL_Color color) {
  return aacircleRGBA(renderer, x, y, rad, color.r, color.g, color.b, color.a);
}

int SdlApp::drawButtonFilled(int x, int y, int rad, SDL_Color color) {
  filledCircleRGBA(renderer, x, y, rad, color.r, color.g, color.b, color.a);
  return aacircleRGBA(renderer, x, y, rad, color.r, color.g, color.b, color.a);
}

int SdlApp::drawLine(int x1, int y1, int x2, int y2, SDL_Color color) {
  return aalineRGBA(renderer, x1, y1, x2, y2, color.r, color.g, color.b,
                    color.a);
}

int SdlApp::renderText(const string &text, int x, int y, SDL_Color color) {
  SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), color);
  if (!surface) {
    cerr << "Text rendering failed: " << TTF_GetError() << endl;
    return 1;
  }
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!texture) {
    cerr << "Texture creation failed: " << SDL_GetError() << endl;
    SDL_FreeSurface(surface);
    return 1;
  }
  SDL_Rect dest = {x, y, surface->w, surface->h};
  SDL_RenderCopy(renderer, texture, NULL, &dest);
  SDL_FreeSurface(surface);
  SDL_DestroyTexture(texture);
  return 0;
}

int SdlApp::run(function<void(SDL_Event event)> eventHandler, const int fps) {

  bool running = true;
  SDL_Event event;
  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }

      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      SDL_RenderClear(renderer);
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

      eventHandler(event);
      SDL_RenderPresent(renderer);
      SDL_Delay(1000 / fps);
    }
  }
  return 0;
}

int SdlApp::textSize(const char *text, int *w, int *h) {
  return TTF_SizeText(font, text, w, h);
}
