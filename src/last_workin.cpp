#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <string>
#include <vector>

#define BUTTON_SIZE 40
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define WHITE (SDL_Color){255, 255, 255, 255}
#define BLACK (SDL_Color){0, 0, 0, 255}
#define RED (SDL_Color){255, 0, 0, 255}
#define HIGHLIGHT (SDL_Color){0, 0, 255, 255}
#define SELECTED (SDL_Color){0, 255, 0, 255}

using namespace std;

enum Mode { DRAW_BUTTON, DRAW_LINE, DEFINE_PRICE, GOTO_BUTTON };
Mode currentMode = DRAW_BUTTON;
string priceInput = "";
bool waitingForPrice = false;
bool inPathMode = false;

struct Button {
  int x, y;
  bool selected;
  string label;
};

vector<Button> buttons;
vector<int> selectedButtons;
vector<pair<int, int>> lines;
vector<pair<int, int>> highlightedLines;
map<pair<int, int>, string> prices;
map<int, vector<pair<int, int>>> adj;

TTF_Font *font;
SDL_Color textColor = WHITE;

vector<int> dijkstra(int start, int target) {
  map<int, int> dist;
  map<int, int> prev;
  for (auto &b : buttons)
    dist[&b - &buttons[0]] = numeric_limits<int>::max();

  priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;
  dist[start] = 0;
  pq.push({0, start});

  while (!pq.empty()) {
    int d = pq.top().first, u = pq.top().second;
    pq.pop();

    if (d > dist[u])
      continue;
    for (auto [v, w] : adj[u]) {
      int newDist = dist[u] + w;
      if (newDist < dist[v]) {
        dist[v] = newDist;
        prev[v] = u;
        pq.push({newDist, v});
      }
    }
  }

  vector<int> path;
  for (int at = target; at != start; at = prev[at])
    path.push_back(at);
  path.push_back(start);
  reverse(path.begin(), path.end());
  return path;
}

bool isInsideButton(int x, int y, const Button &button) {
  return x >= button.x && x <= button.x + BUTTON_SIZE && y >= button.y &&
         y <= button.y + BUTTON_SIZE;
}

int getButtonIndex(int x, int y) {
  for (int i = 0; i < buttons.size(); ++i) {
    if (isInsideButton(x, y, buttons[i]))
      return i;
  }
  return -1;
}

bool lineExists(int a, int b) {
  return prices.find({a, b}) != prices.end() ||
         prices.find({b, a}) != prices.end();
}

void removeLine(int a, int b) {
  lines.erase(remove(lines.begin(), lines.end(), make_pair(a, b)), lines.end());
  lines.erase(remove(lines.begin(), lines.end(), make_pair(b, a)), lines.end());
  prices.erase({a, b});
  prices.erase({b, a});
}

void renderText(SDL_Renderer *renderer, const string &text, int x, int y) {
  SDL_Surface *surface = TTF_RenderText_Solid(font, text.c_str(), textColor);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_Rect dest = {x, y, surface->w, surface->h};
  SDL_RenderCopy(renderer, texture, NULL, &dest);
  SDL_FreeSurface(surface);
  SDL_DestroyTexture(texture);
}

void handleMouseClick(int x, int y) {
  if (waitingForPrice)
    return;

  int index = getButtonIndex(x, y);

  if (currentMode == DRAW_BUTTON) {
    if (index == -1) {
      string label = "";
      if (buttons.size() < 26) {
        label = string(1, 'A' + buttons.size());
      }
      buttons.push_back({x, y, false, label});
    }
  } else if (currentMode == DRAW_LINE || currentMode == DEFINE_PRICE) {
    if (index != -1) {
      auto it = find(selectedButtons.begin(), selectedButtons.end(), index);
      if (it != selectedButtons.end()) {
        selectedButtons.erase(it);
        buttons[index].selected = false;
      } else {
        selectedButtons.push_back(index);
        buttons[index].selected = true;

        if (selectedButtons.size() == 2) {
          int a = selectedButtons[0], b = selectedButtons[1];

          if (currentMode == DRAW_LINE) {
            if (lineExists(a, b)) {
              removeLine(a, b);
            } else {
              lines.push_back({a, b});
              prices[{a, b}] = "1";
              prices[{b, a}] = "1";
              adj[a].push_back({b, stoi(prices[{a, b}])});
              adj[b].push_back({a, stoi(prices[{b, a}])});
            }
          } else if (currentMode == DEFINE_PRICE) {
            if (lineExists(a, b)) {
              waitingForPrice = true;
              priceInput = "";
            }
          }

          if (!waitingForPrice) {
            buttons[a].selected = false;
            buttons[b].selected = false;
            selectedButtons.clear();
          }
        }
      }
    }
  } else if (currentMode == GOTO_BUTTON && !inPathMode) {
    if (index != -1) {
      selectedButtons.push_back(index);
      buttons[index].selected = true;

      if (selectedButtons.size() == 2) {
        int a = selectedButtons[0], b = selectedButtons[1];
        vector<int> path = dijkstra(a, b);

        inPathMode = true;
        selectedButtons = path;
        highlightedLines.clear();
        for (int i = 0; i < path.size() - 1; ++i) {
          highlightedLines.push_back({path[i], path[i + 1]});
        }

        for (int i = 0; i < path.size() - 1; ++i) {
          cout << buttons[path[i]].label << " -> ";
        }
        cout << buttons[b].label << endl;
      }
    }
  }
}

void updateAdjacency(int a, int b, int weight) {
  adj[a].erase(remove_if(adj[a].begin(), adj[a].end(),
                         [b](const pair<int, int> &p) { return p.first == b; }),
               adj[a].end());

  adj[b].erase(remove_if(adj[b].begin(), adj[b].end(),
                         [a](const pair<int, int> &p) { return p.first == a; }),
               adj[b].end());

  adj[a].push_back({b, weight});
  adj[b].push_back({a, weight});
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  SDL_Window *window = SDL_CreateWindow("DIJKSTRA", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                                        SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  font = TTF_OpenFont("roboto.ttf", 16);

  bool running = true;
  SDL_Event event;

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        running = false;
      else if (event.type == SDL_MOUSEBUTTONDOWN &&
               event.button.button == SDL_BUTTON_LEFT) {
        handleMouseClick(event.button.x, event.button.y);
      } else if (event.type == SDL_KEYDOWN) {

        if (inPathMode) {
          if (event.key.keysym.sym == SDLK_ESCAPE) {
            inPathMode = false;
            selectedButtons.clear();
            highlightedLines.clear();
            for (auto &button : buttons) {
              button.selected = false;
            }
            currentMode = GOTO_BUTTON;
          }
        } else if (waitingForPrice) {
          if (event.key.keysym.sym == SDLK_RETURN) {
            int weight = stoi(priceInput);
            int a = selectedButtons[0], b = selectedButtons[1];

            prices[{a, b}] = priceInput;
            prices[{b, a}] = priceInput;
            updateAdjacency(a, b, weight);

            waitingForPrice = false;
            buttons[a].selected = false;
            buttons[b].selected = false;
            selectedButtons.clear();
          } else if (event.key.keysym.sym == SDLK_BACKSPACE &&
                     !priceInput.empty()) {
            priceInput.pop_back();
          }
        } else {
          if (event.key.keysym.sym == SDLK_l)
            currentMode = DRAW_LINE;
          else if (event.key.keysym.sym == SDLK_d)
            currentMode = DRAW_BUTTON;
          else if (event.key.keysym.sym == SDLK_p)
            currentMode = DEFINE_PRICE;
          else if (event.key.keysym.sym == SDLK_g)
            currentMode = GOTO_BUTTON;
        }
      } else if (event.type == SDL_TEXTINPUT && waitingForPrice) {
        priceInput += event.text.text;
      }
    }

    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, RED.r, RED.g, RED.b, RED.a);
    for (const auto &line : lines) {
      SDL_RenderDrawLine(renderer, buttons[line.first].x + BUTTON_SIZE / 2,
                         buttons[line.first].y + BUTTON_SIZE / 2,
                         buttons[line.second].x + BUTTON_SIZE / 2,
                         buttons[line.second].y + BUTTON_SIZE / 2);
    }

    SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
    for (const auto &[key, price] : prices) {
      int x = (buttons[key.first].x + buttons[key.second].x) / 2;
      int y = (buttons[key.first].y + buttons[key.second].y) / 2;
      cout << price << endl;
      renderText(renderer, price, x, y);
    }

    SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
    for (const auto &button : buttons) {
      SDL_Rect rect = {button.x, button.y, BUTTON_SIZE, BUTTON_SIZE};
      SDL_RenderDrawRect(renderer, &rect);
      if (button.selected) {
        SDL_SetRenderDrawColor(renderer, SELECTED.r, SELECTED.g, SELECTED.b,
                               SELECTED.a);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
      }

      int textWidth, textHeight;
      TTF_SizeText(font, button.label.c_str(), &textWidth, &textHeight);
      renderText(renderer, button.label,
                 button.x + (BUTTON_SIZE - textWidth) / 2,
                 button.y + (BUTTON_SIZE - textHeight) / 2);
    }

    if (inPathMode) {
      SDL_SetRenderDrawColor(renderer, HIGHLIGHT.r, HIGHLIGHT.g, HIGHLIGHT.b,
                             HIGHLIGHT.a);
      for (auto &line : highlightedLines) {
        SDL_RenderDrawLine(renderer, buttons[line.first].x + BUTTON_SIZE / 2,
                           buttons[line.first].y + BUTTON_SIZE / 2,
                           buttons[line.second].x + BUTTON_SIZE / 2,
                           buttons[line.second].y + BUTTON_SIZE / 2);
      }

      SDL_SetRenderDrawColor(renderer, HIGHLIGHT.r, HIGHLIGHT.g, HIGHLIGHT.b,
                             HIGHLIGHT.a);
      for (int index : selectedButtons) {
        auto button = buttons[index];
        SDL_Rect buttonRect = {button.x, button.y, BUTTON_SIZE, BUTTON_SIZE};
        SDL_RenderFillRect(renderer, &buttonRect);
        int textWidth, textHeight;
        TTF_SizeText(font, button.label.c_str(), &textWidth, &textHeight);
        renderText(renderer, button.label,
                   button.x + (BUTTON_SIZE - textWidth) / 2,
                   button.y + (BUTTON_SIZE - textHeight) / 2);
      }
    }

    renderText(renderer,
               "Press D: Draw Button | L: Draw Line | P: Define Price", 10, 10);
    string modeText = "Mode: " + string(currentMode == DRAW_BUTTON   ? "Button"
                                        : currentMode == DRAW_LINE   ? "Line"
                                        : currentMode == GOTO_BUTTON ? "Go TO "
                                                                     : "Price");
    renderText(renderer, modeText, 10, 30);

    if (waitingForPrice) {
      renderText(renderer, "Enter price: " + priceInput, 10, 50);
    }

    SDL_RenderPresent(renderer);
  }

  TTF_CloseFont(font);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  TTF_Quit();
  SDL_Quit();
  return 0;
}
