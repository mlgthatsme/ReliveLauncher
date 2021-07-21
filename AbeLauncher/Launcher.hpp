#pragma once

#include "SDLApp.hpp"

#include "stbttf.h"

enum TextAlign : uint8_t
{
    TextAlign_Left = 0,
    TextAlign_Centre = 1,
    TextAlign_Right = 2,
};

enum LauncherState : uint8_t
{
    LauncherState_GameSelection = 0,
    LauncherState_GameSelected = 1,
};

class UIGameSelector : public UIElement
{

};

class Launcher : public SDLApp
{
public:
    virtual void Initialize() override;
    virtual void OnEvent(SDL_Event& pEvent) override;
    virtual void Update(float td) override;
    virtual void Draw(float td, SDL_Rect region) override;
    virtual void Shutdown() override;

    int m_SelectionIndex = 0;
    float m_SelectionSmooth = 0;

    void RenderAudio(MenuSampleStereo* pDstSamples, int lengthSamples);

    void SetState(LauncherState state);
    void SetProgress(float v);
    void SetProgressMessage(std::string message);

private:
    void DrawGameTile(SDL_Rect region, Texture* bgTex, Texture* logoTex, float bgOffX, float darkness);
    void DrawText(STBTTF_Font* font, std::string text, Vec2 position, uint8_t r, uint8_t g, uint8_t b, uint8_t a, TextAlign align);
    void DrawProgressBar(Vec2 pos, Vec2 size, float percent /* from 0 - 1*/);

    std::vector<unsigned char> m_MenuMusicAo;
    std::vector<unsigned char> m_MenuMusicAe;

    Texture* m_TexBG1 = nullptr;
    Texture* m_TexBG2 = nullptr;
    Texture* m_TexLogoAo = nullptr;
    Texture* m_TexLogoAe = nullptr;

    STBTTF_Font* m_FontSmall = nullptr;
    STBTTF_Font* m_FontMedium = nullptr;
    STBTTF_Font* m_FontLarge = nullptr;

    float m_LoadProgress = 0.0f;
    std::string m_LoadMessage = "";

    void StartGame();

    LauncherState m_State = LauncherState_GameSelection;
};