#pragma once

#include <fstream>
#include <iostream>
#include <vector>
#include <map>

#include <SDL.h>

#include "stb/stb_image.h"

#define ALIVE_FATAL // TODO Remove this

struct MenuSampleStereo
{
    short Left;
    short Right;
};

struct Vec2
{
    float x;
    float y;
};

class Texture
{
public:
    Texture(SDL_Texture* texture);
    SDL_Texture* m_Texture = nullptr;
    int m_Width = 0;
    int m_Height = 0;
};

class UIElement
{
public:
    virtual void Initialize() = 0;
    virtual void OnEvent(SDL_Event& pEvent) = 0;
    virtual void Update(float td) = 0;
    virtual void Draw(float td, SDL_Rect region) = 0;

    void UpdateTree(float td);
    void DrawTree(float td, SDL_Rect region);

    std::vector<std::shared_ptr<UIElement>> m_Children;

    void Close()
    {
        m_Closing = true;
    }

    SDL_Rect m_Transform = { 0,0,64,64 };

private:
    bool m_Closing = false;
};

class SDLApp : public UIElement
{
public:
    SDLApp();
    void Run();
    void Stop();

    virtual void Shutdown() = 0;

    SDL_Window* m_pWindow = nullptr;
    SDL_Renderer* m_pRenderer = nullptr;

protected:
    SDL_AudioSpec m_AudioDeviceSpec = {};
    SDL_GameController* m_pController = nullptr;

    int m_ScreenWidth = 1280;
    int m_ScreenHeight = 720;

    void InitAudio(void * userData, SDL_AudioCallback callback);
    Texture* LoadTexture(std::string path);
    void DrawTexture(Texture* pTexture, Vec2 position, Vec2 size, Vec2 centre = { 0.5f, 0.5f });
private:

    Uint32 m_LastFrameTick;
    bool m_AppRunning = true;
    SDL_GameController* GetGameController();
    std::map<std::string, Texture*> m_LoadedTextures;
};

static SDLApp* g_AppInstance = nullptr;