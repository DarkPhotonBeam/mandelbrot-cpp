/* points.c ... */

/*
 * This example creates an SDL window and renderer, and then draws some points
 * to it every frame.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "Mandelbrot.h"
#include "MandelbrotDouble.h"
#include "MPC.h"


#define ARGB(r, g, b, a) ((0xff << 24) | (r << 16) | (g << 8) | b)

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;
static SDL_Texture *texture = nullptr;

static Mandelbrot mbrot;
static MandelbrotDouble mbrotd;

constexpr bool useDouble = false;

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 1200

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
        SDL_SetAppMetadata("Mandelbrot", "1.0", "com.danphoton.mandelbrot");

        if (!SDL_Init(SDL_INIT_VIDEO)) {
                SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
                return SDL_APP_FAILURE;
        }

        if (!SDL_CreateWindowAndRenderer("Mandelbrot", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window,
                                         &renderer)) {
                SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
                return SDL_APP_FAILURE;
        }

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);
        if (!texture) {
                SDL_Log("Couldn't create texture: %s", SDL_GetError());
                return SDL_APP_FAILURE;
        }

        if (useDouble) {
                mbrotd.initialize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
                mbrotd.set_viewport(Pixeld{WINDOW_WIDTH/2, WINDOW_HEIGHT/2});
                mbrotd.print_info();
        } else {
                mbrot.initialize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
                mbrot.set_viewport(Pixel{WINDOW_WIDTH/2, WINDOW_HEIGHT/2});
                mbrot.print_info();
        }


        return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
        //std::cout << "event!!!\n";
        if (event->type == SDL_EVENT_QUIT) {
                return SDL_APP_SUCCESS; /* end the program, reporting success to the OS. */
        }
        if (event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
                std::cout << "Mouse Button Up, x: " << event->button.x << ", y: " << event->button.y << std::endl;
                int x_i = static_cast<int>(event->button.x);
                int y_i = static_cast<int>(event->button.y);
                if (useDouble) {
                        mbrotd.shrink_offset();
                        mbrotd.set_viewport(Pixeld{x_i, y_i});
                        mbrotd.print_info();
                } else {
                        mbrot.shrink_offset();
                        mbrot.set_viewport(Pixel{x_i, y_i});
                        mbrot.print_info();
                }

        }
        return SDL_APP_CONTINUE; /* carry on with the program! */
}

SDL_AppResult SDL_AppIterate(void *appstate) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, useDouble ? mbrotd.get_texture() : mbrot.get_texture(), nullptr, nullptr);
        SDL_RenderPresent(renderer);

        return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
        std::cout << "Shutdown\n";
        SDL_DestroyTexture(texture);
}
