/**
 * @file    PixelDraw.hh
 * @author  maxrt101
 * @version 0.1
 * @brief   Simple sdl2 wrapper
*/

#pragma once

#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <string>

#define SDL_ERROR(msg) std::cerr << "[\033[1;31m ERROR \033[0m] " << msg << "(" << SDL_GetError() << ")\n";
#define ERROR(msg) std::cerr << "[\033[1;31m ERROR \033[0m] " << msg << "\n";
#define WARN(msg) std::clog << "[\033[1;33m WARN \0330m] " << msg << "\n";
#define INFO(msg) std::clog << "[ INFO ] " << msg << "\n";
#ifdef _DEBUG
    #define DEBUG(msg) std::clog << "[\033[1;34m DEBUG \033[0m] " << __PRETTY_FUNCTION__ << ": " << msg << "\n";
#else
    #define DEBUG(msg)
#endif

namespace mrt {
    template <typename T>
    struct vec2 {
        T x = 0;
        T y = 0;

        vec2() : x(0), y(0) {}
        vec2(T x, T y) : x(x), y(y) {}

        inline vec2<T>& operator=(const vec2<T>& v) {
            x = v.x;
            y = v.y;
            return *this;
        }

        inline bool operator==(const vec2<T>& v) const { return x==v.x && y==v.y; }
        inline bool operator!=(const vec2<T>& v) const { return x!=v.x || y!=v.y; }
    };

    typedef vec2<int> vec2i;
    typedef vec2<unsigned int> vec2u;
    typedef vec2<float> vec2f;
    typedef vec2<double> vec2d;

    template <typename T>
    struct vec3 {
        T x = 0;
        T y = 0;
        T z = 0;

        vec3() : x(0), y(0), z(0) {}
        vec3(int x, int y, int z) : x(x), y(y), z(z) {}

        inline vec3<T>& operator=(const vec3<T>& v) {
            x = v.x;
            y = v.y;
            z = v.z;
            return *this;
        }

        inline bool operator==(const vec3<T>& v) const { return x==v.x && y==v.y && z==v.z; }
        inline bool operator!=(const vec3<T>& v) const { return x!=v.x || y!=v.y || z!=v.z; }
    };

    typedef vec3<int> vec3i;
    typedef vec3<unsigned int> vec3u;
    typedef vec3<float> vec3f;
    typedef vec3<double> vec3d;


    class Texture {
    private:
        SDL_Texture* texture_ptr = nullptr;
        // SDL_Surface* pixels = nullptr;
        int w = 0;
        int h = 0;

    public:
        Texture(SDL_Renderer* renderer, const std::string& path);
        Texture(Texture&& t);
        ~Texture();

        SDL_Texture* get_sdl_texture() const;
        SDL_Surface* get_pixels() const;

        int get_width() const;
        int get_height() const;

        // void read_pixels();

        // void draw(SDL_Renderer* renderer, int x, int y);
        // void draw_sample(SDL_Renderer* renderer, int sx, int sy, int w, int h, int dx, int dy, int dw, int dh);
    };

    struct KeyState {
        bool pressed = false;
        bool held = false;
        bool released = false;

        KeyState();
        KeyState(bool p, bool h, bool r);
    };

    class PixelDraw {
    private:
        struct FrameKeyState {
            bool pressed = false;
            bool released = false;
        };

    private:
        bool initialised = false;
        bool running = false;

        std::string app_name;
        const int screen_width;
        const int screen_height;

        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;

        int cycle_count = 0;
        float fps_cap = 120;

        FrameKeyState keys[322];
        bool held_keys[322];

    public:
        PixelDraw(const std::string& name, int h, int w);
        ~PixelDraw();

        void stop();
        void run();

        int get_height() const;
        int get_width() const;

        void set_fps_cap(int cap);

    protected:
        SDL_Window* get_window() const;
        SDL_Renderer* get_renderer() const;

        KeyState get_key_state(SDL_Scancode sc) const;

        void clear_screen();
        void update_screen();

        Texture create_texture(const std::string& path) const;

    public: // Interface
        virtual void on_load() = 0;
        virtual void on_frame_update(float frame_time) = 0;
    };
}

//#define PIXELDRAW_IMPLEMENTATION

#ifdef PIXELDRAW_IMPLEMENTATION

#include <iostream>
#include <chrono>

namespace mrt {

    Texture::Texture(SDL_Renderer* renderer, const std::string& path) {
        texture_ptr = IMG_LoadTexture(renderer, path.c_str());
        SDL_QueryTexture(texture_ptr, NULL, NULL, &w, &h);
    }

    Texture::Texture(Texture&& t) {
        texture_ptr = t.texture_ptr;
        w = t.w;
        h = t.h;

        t.texture_ptr = nullptr;
    }

    Texture::~Texture() {
        if (texture_ptr) {
            DEBUG("Texture destroyed: " << texture_ptr);
            SDL_DestroyTexture(texture_ptr);
        }
    }

    SDL_Texture* Texture::get_sdl_texture() const {
        return texture_ptr;
    }

    int Texture::get_width() const {
        return w;
    }

    int Texture::get_height() const {
        return h;
    }

    KeyState::KeyState() {}

    KeyState::KeyState(bool p, bool h, bool r) : pressed(p), held(h), released(r) {}


    PixelDraw::PixelDraw(const std::string& name, int w, int h) : app_name(name), screen_width(w), screen_height(h) {
        std::cout << "mrt::PixelDraw v0.1\n";

        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            SDL_ERROR("SDL_Init failed");
            exit(EXIT_FAILURE);
        }

        if (IMG_Init(IMG_INIT_JPG) < 0) {
            SDL_ERROR("IMG_Init failed");
            exit(EXIT_FAILURE);
        }

        window = SDL_CreateWindow(app_name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_SHOWN);

        if (window == NULL) {
            SDL_ERROR("Window creation failed");
            exit(EXIT_FAILURE);
        }

        renderer = SDL_CreateRenderer(window, -1, 0);
        if (renderer == NULL) {
            SDL_ERROR("Renderer creation failed");
            exit(EXIT_FAILURE);
        }

        memset(&keys, 0, 322*sizeof(FrameKeyState));
        memset(&held_keys, 0, 322);

        INFO("PixelDraw initialized.");
        initialised = true;
    }

    PixelDraw::~PixelDraw() {
        running = false;
        if (initialised) {
            stop();
        }
    }

    void PixelDraw::stop() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        INFO("Engine stopped.");
    }

    void PixelDraw::run() {
        auto time1 = std::chrono::system_clock::now();
		auto time2 = std::chrono::system_clock::now();

        float frame_time = 0.1;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

        on_load();

        running = true;

        while (running) {
            cycle_count++;

            time2 = std::chrono::system_clock::now();
			std::chrono::duration<float> frame_duration = time2 - time1;
			time1 = time2;
			frame_time = frame_duration.count();

            // if (frame_time < 1000.0/fps_cap) {
            //     SDL_Delay(1000.0/fps_cap - frame_time);
            // }

            memset(&keys, 0, 322*sizeof(FrameKeyState));

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                } else if (event.type == SDL_KEYDOWN) {
                    if (!held_keys[event.key.keysym.scancode]) {
                        keys[event.key.keysym.scancode].pressed = true;
                        held_keys[event.key.keysym.scancode] = true;
                    }
                } else if (event.type == SDL_KEYUP) {
                    keys[event.key.keysym.scancode].released = true;
                    held_keys[event.key.keysym.scancode] = false;
                }
            }

            clear_screen();
            on_frame_update(frame_time);
            update_screen();

            // 1/30 s
            // 1/time = fps
            // time = 1/fps

            // DEBUG(frame_time << " " << 1/frame_time);

            /** @todo: optimize */
            char fps[4] {0};
            snprintf(fps, 4, "%03i", int(1/frame_time));
            std::string title = "mrt::PixelDraw FPS: ";
            title += fps;
            SDL_SetWindowTitle(window, (char*)(title.c_str()));
        }
    }

    int PixelDraw::get_height() const {
        return screen_height;
    }

    int PixelDraw::get_width() const {
        return screen_width;
    }

    void PixelDraw::set_fps_cap(int cap) {
        fps_cap = cap * (4.0f/3.0f);
    }

    SDL_Window* PixelDraw::get_window() const {
        return window;
    }

    SDL_Renderer* PixelDraw::get_renderer() const {
        return renderer;
    }

    KeyState PixelDraw::get_key_state(SDL_Scancode sc) const {
        return KeyState(keys[sc].pressed, held_keys[sc], keys[sc].released);
    }

    void PixelDraw::clear_screen() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
    }

    void PixelDraw::update_screen() {
        SDL_RenderPresent(renderer);
    }

    Texture PixelDraw::create_texture(const std::string& path) const {
        return Texture(renderer, path);
    }
}

#endif