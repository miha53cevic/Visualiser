#ifndef APP_H
#define APP_H

#ifdef _WIN32
#include <SDL.h>
#elif __linux__
#include <SDL2/SDL.h>
#endif

#include <string>

class Clock
{
public:
    Clock();

    float restart();

private:
    Uint32 m_start, m_end;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

class App
{
public:
    App(const char* title, int width, int height);
    ~App();

    void Run();

protected:
    SDL_Window* m_window;
    SDL_GLContext m_maincontext;

    int m_screenWidth, m_screenHeight;
    std::string m_title;
    bool m_bFPSCounter;
    bool m_bFocus;

    const Uint8 *m_keys;

    Clock m_clock;

    void sdl_die(const char* message);
    void init_screen(const char* title);

    virtual bool Event(SDL_Event& e) = 0;
    virtual bool Setup() = 0;
    virtual bool Loop(float elapsedTime) = 0;

    void setClearColor(int r, int g, int b, int a);

    int ScreenWidth();
    int ScreenHeight();

    bool GetFocus();
    Uint8 GetKey(SDL_Scancode code);

    void FPSCounter(bool fpscounter);
    void WireFrame(bool wireframe);
    void VSync(bool vsync);
    void Culling(bool cull);
    void ShowCursor(bool cursor);

    bool MouseHold(int key);
};

#endif