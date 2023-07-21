// GPU driver via SDL2 FB 
#include <SDL.h>
#include "polyfills.h"
#include "display.h"
#include "keyscan.h"
#include "ui.h"
SDL_Window *window;
SDL_Surface *window_surface;
SDL_Surface *surface_332;
SDL_Renderer *fixed_fps_renderer;
SDL_GameController *gp;
uint8_t *pixels_332;
uint8_t *frame_bb;
#define BYTES_PER_PIXEL 1
int64_t frame_ticks = 0;
int8_t unix_display_flag = 0;
SDL_Keymod last_held_mod;


void unix_set_fps_from_parameters() {
    // use the screen res and clock to discern a new FPS, based on real life measurements on tulip cc
    float fps_map[20] = {14.98, 18.55, 23.19, 30.91, 46.37, 21.47, 34.36, 42.95, 57.26, 85.90,
                         19.91, 30.26, 37.82, 50.43, 75.65, 35.03, 56.05, 70.07, 93.45, 140.13};
    uint8_t clocks[5]= {10,14,18,22,28};
    uint16_t h_reses[2] = {1024, 512};
    uint16_t v_reses[2] = {600, 300};
    uint8_t fps_i = 0;
    for(uint8_t h_res_i = 0;h_res_i < 2;h_res_i++) {
        for(uint8_t v_res_i = 0;v_res_i < 2;v_res_i++) {
            for(uint8_t clock_i = 0;clock_i < 5;clock_i++) {
                if(H_RES==h_reses[h_res_i] && V_RES==v_reses[v_res_i] && PIXEL_CLOCK_MHZ==clocks[clock_i]) {
                    //fprintf(stderr, "Returning %2.2f FPS for res %d %d clock %d\n", fps_map[fps_i], H_RES, V_RES, PIXEL_CLOCK_MHZ);
                    reported_fps = fps_map[fps_i];
                    // get out of search 
                    clock_i = 10; v_res_i = 10; h_res_i = 10; 
                }
                fps_i++;
            }
        }
    }
    if(fps_i == 20) {
        fprintf(stderr, "Problem: could not find matching fps from res %d, %d clock %d. returning 30fps.\n", H_RES, V_RES, PIXEL_CLOCK_MHZ);
        reported_fps = 30;
    }


}

void unix_display_set_clock(uint8_t mhz) {  
    PIXEL_CLOCK_MHZ = mhz;
    unix_set_fps_from_parameters();
}


void unix_display_timings(uint32_t t0, uint32_t t1, uint32_t t2, uint32_t t3, uint32_t t4, uint32_t t5) {
    fprintf(stderr, "Stopping display task\n");
    H_RES = t0;
    V_RES = t1; 
    OFFSCREEN_X_PX = t2; 
    OFFSCREEN_Y_PX = t3; 
    H_RES_D = t4;
    V_RES_D = t5;
    TFB_ROWS = (V_RES_D/FONT_HEIGHT);
    TFB_COLS = (H_RES_D/FONT_WIDTH);
    BOUNCE_BUFFER_SIZE_PX = (H_RES*FONT_HEIGHT) ;

    unix_display_flag = -2; // restart display with new timings
}


void init_window(uint16_t w, uint16_t h) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
        fprintf(stderr,"SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    } else {
#ifdef __TULIP_IOS__
        window = SDL_CreateWindow("SDL Output", SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, w, h,
                                SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN_DESKTOP);
#else
        window = SDL_CreateWindow("SDL Output", SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, w, h,
                                SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

#endif
    }
    if (window == NULL) {
        fprintf(stderr,"Window could not be created! SDL_Error: %s\n", SDL_GetError());
    } else {
        int mh, mw;
        SDL_GL_GetDrawableSize(window, &mw, &mh);
        fprintf(stderr, "drawable area is %d %d\n", mw, mh);
        window_surface = SDL_GetWindowSurface(window);
        surface_332 = SDL_ConvertSurfaceFormat(window_surface, SDL_PIXELFORMAT_RGB332, 0);
        fprintf(stderr, "surface is %d %d\n", surface_332->w, surface_332->h);
        fixed_fps_renderer = SDL_CreateSoftwareRenderer( window_surface);
        pixels_332 = (uint8_t*) surface_332->pixels;
    }
    memset(pixels_332, 0, surface_332->pitch * V_RES);
    // If this is not set it prevents sleep on a mac (at least)
    SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");
    SDL_SetWindowTitle(window, "Tulip Desktop");
    SDL_StartTextInput();
}

void destrow_window() {
    SDL_DestroyWindow(window);
    SDL_Quit();    
}

uint16_t last_held_joy_mask = 0;

void update_joy(SDL_Event e) {
    if(e.type == SDL_CONTROLLERBUTTONDOWN || e.type == SDL_CONTROLLERBUTTONUP) {
        last_held_joy_mask = 0;
        for(uint8_t b=0;b<SDL_CONTROLLER_BUTTON_MAX;b++) {
            if(SDL_GameControllerGetButton(gp, b)) {
                if(b == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) last_held_joy_mask |= 2;
                if(b == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) last_held_joy_mask |= 4;
                if(b == SDL_CONTROLLER_BUTTON_X) last_held_joy_mask |= 8;
                if(b == SDL_CONTROLLER_BUTTON_A) last_held_joy_mask |= 16;
                if(b == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) last_held_joy_mask |= 32;
                if(b == SDL_CONTROLLER_BUTTON_DPAD_LEFT) last_held_joy_mask |= 64;
                if(b == SDL_CONTROLLER_BUTTON_DPAD_DOWN) last_held_joy_mask |= 128;
                if(b == SDL_CONTROLLER_BUTTON_DPAD_UP) last_held_joy_mask |= 256;
                if(b == SDL_CONTROLLER_BUTTON_START) last_held_joy_mask |= 512;
                if(b == SDL_CONTROLLER_BUTTON_BACK) last_held_joy_mask |= 1024;
                if(b == SDL_CONTROLLER_BUTTON_Y) last_held_joy_mask |= 2048;
                if(b == SDL_CONTROLLER_BUTTON_B) last_held_joy_mask |= 4096;
            }
        }
    }    
}

uint16_t check_joy() {
    return last_held_joy_mask;
}


void check_key() {
#ifndef MONITOR_APPLE 
    SDL_Event e;
    uint8_t was_touch = 0;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            unix_display_flag = -1; // tell main to quit
        } else if(e.type == SDL_KEYDOWN) {
            last_held_mod = SDL_GetModState();
            SDL_KeyboardEvent key = e.key; 
            if(key.keysym.scancode >= 0x04 && key.keysym.scancode <= 0x94) {
                send_key_to_micropython(scan_ascii(key.keysym.scancode, (uint32_t)last_held_mod));
            }
            uint8_t skip = 0;
            uint8_t pos = 10;
            for(uint8_t i=2;i<8;i++) {
                if(last_scan[i] == key.keysym.scancode) { skip = 1; }
                if(pos == 10 && last_scan[i] == 0) { pos = i; }
            }
            if(!skip && pos < 8) {
                last_scan[pos] = key.keysym.scancode;
            }
        }
        if(e.type == SDL_KEYUP) {
            SDL_KeyboardEvent key = e.key; 
            for(uint8_t i=2;i<8;i++) {
                if(key.keysym.scancode == last_scan[i]) {
                    last_scan[i] = 0;
                }
            }
        }
        int x,y;
        uint32_t button = SDL_GetMouseState(&x, &y);
        if(button) {
            last_touch_x[0] = (int16_t)x;
            last_touch_y[0] = (int16_t)y;
            was_touch = 1;
        } else { // release
            last_touch_x[0] = (int16_t)x;
            last_touch_y[0] = (int16_t)y;
            was_touch = 2;
        }
        update_joy(e);
    }
    if(was_touch) {
        send_touch_to_micropython(last_touch_x[0], last_touch_y[0], was_touch-1);
    }
#endif
}


void start_draw() { 
    SDL_LockSurface(surface_332); 

}

// Using our non-vsync software renderer, blit the surface into a texture and then out to the window
void end_draw() {
    SDL_UnlockSurface(surface_332);

    SDL_Texture *blit_texture = SDL_CreateTextureFromSurface(fixed_fps_renderer, surface_332);
    SDL_RenderCopy(fixed_fps_renderer, blit_texture, NULL, NULL);
    SDL_RenderPresent(fixed_fps_renderer);

    // Clean up and show
    SDL_DestroyTexture(blit_texture);
    SDL_UpdateWindowSurface(window);
    display_frame_done_generic();

    int64_t ticks_per_frame_ms = (int64_t) (1000.0 / reported_fps);

    //If frame finished early according to our FPS clock, pause a bit (still processing keys) until it's time
    while(get_ticks_ms() - frame_ticks < ticks_per_frame_ms) {
        SDL_Delay(1);
        check_key();
    }
}

int unix_display_draw() {
    frame_ticks = get_ticks_ms();
    check_key();
    start_draw();

    // bounce the entire screen at once to the 332 color framebuffer
    for(uint16_t y=0;y<V_RES;y=y+FONT_HEIGHT) {
        if(y+FONT_HEIGHT <= V_RES) {
            display_bounce_empty(frame_bb, y*H_RES, H_RES*FONT_HEIGHT, NULL);
            for (uint16_t row=0;row<FONT_HEIGHT;row++) {
                for(uint16_t x=0;x<H_RES;x++) {
                    pixels_332[((y+row)*surface_332->pitch)+x] = frame_bb[H_RES*row + x];
                }
            }
        }
    }
    end_draw();
    // Are we restarting the display for a mode change, or quitting
    if(unix_display_flag < 0) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        display_teardown();
        if(unix_display_flag==-2) {
            unix_display_flag = 0;
            return -2;
        } else {
            unix_display_flag = 0;
            return -1;
        }
    }    
    return 1;
}

void unix_display_init() {
    display_init();
    unix_set_fps_from_parameters();
    init_window(H_RES,V_RES); 
    frame_bb = (uint8_t *) malloc_caps(FONT_HEIGHT*H_RES*BYTES_PER_PIXEL,MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    gp = SDL_GameControllerOpen(0);
    if(!gp) {
        fprintf(stderr, "No gamepad detected. That's ok\n");
    } else {
        fprintf(stderr, "Gamepad detected\n");
    }
}
