#include "Launcher.hpp"
#include <thread>
#include <string>

#define CONTENT_DIR "data/"

enum class GameType : int32_t
{
    eAo = 0,
    eAe = 1,
};

std::vector<unsigned char> LoadMusicFile(const char* filename)
{
    // open the file:
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        return std::vector<unsigned char>();
    }

    // read the data:
    return std::vector<unsigned char>((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
}

inline float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

inline MenuSampleStereo sampleLerp(MenuSampleStereo from, MenuSampleStereo to, float t)
{
    return { static_cast<short>(lerp(from.Left, to.Left, t)),static_cast<short>(lerp(from.Right, to.Right, t)) };
}

void Launcher::RenderAudio(MenuSampleStereo* pDstSamples, int lengthSamples)
{
    static int sampleIndex = 0;

    const MenuSampleStereo* pSoundDataAo = reinterpret_cast<MenuSampleStereo*>(m_MenuMusicAo.data());
    const MenuSampleStereo* pSoundDataAe = reinterpret_cast<MenuSampleStereo*>(m_MenuMusicAe.data());

    for (int i = 0; i < lengthSamples; i++)
    {

        pDstSamples[i] = sampleLerp(pSoundDataAo[sampleIndex], pSoundDataAe[sampleIndex], m_SelectionSmooth);

        sampleIndex++;

        if (sampleIndex >= m_MenuMusicAo.size() / sizeof(MenuSampleStereo))
        {
            sampleIndex = 0;
        }
    }
}

void Launcher::SetState(LauncherState state)
{
    m_State = state;
}

void Launcher::SetProgress(float v)
{
    m_LoadProgress = v;
}

void Launcher::SetProgressMessage(std::string message)
{
    m_LoadMessage = message;
}

void Launcher::DrawGameTile(SDL_Rect region, Texture* bgTex, Texture* logoTex, float bgOffX, float darkness)
{
    SDL_RenderSetClipRect(m_pRenderer, &region);

    int maxLogoSize = static_cast<int>(m_ScreenHeight / 1.5f);

    int scaledLogoX = region.w;
    if (scaledLogoX > maxLogoSize)
        scaledLogoX = maxLogoSize;
    if (scaledLogoX < m_ScreenWidth * 0.2f)
        scaledLogoX = static_cast<int>(m_ScreenWidth * 0.2f);

    int scaledLogoY = static_cast<int>(scaledLogoX * (static_cast<float>(logoTex->m_Height) / logoTex->m_Width));

    DrawTexture(bgTex, { bgOffX , static_cast<float>(region.y) }, { static_cast<float>(m_ScreenWidth), static_cast<float>(m_ScreenHeight) }, { 0,0 });
    DrawTexture(logoTex, { (region.x + (region.w / 2.0f)) , region.y + (m_ScreenHeight / 2.0f) }, { static_cast<float>(scaledLogoX), static_cast<float>(scaledLogoY) });

    SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, static_cast<Uint8>(darkness * 190));
    SDL_RenderFillRect(m_pRenderer, &region);
}

void Launcher::DrawText(STBTTF_Font* font, std::string text, Vec2 position, uint8_t r, uint8_t g, uint8_t b, uint8_t a, TextAlign align)
{
    float height = 0;
    const float textWidth = STBTTF_MeasureText(font, text.c_str(), &height);

    float alignment = 0;
    switch (align)
    {
    case TextAlign_Centre:
        alignment = textWidth / 2;
        break;
    case TextAlign_Right:
        alignment = textWidth;
        break;
    }

    SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, a);
    STBTTF_RenderText(m_pRenderer, font, (position.x + 2) - alignment, (position.y + height) + 2, text.c_str());
    SDL_SetRenderDrawColor(m_pRenderer, r, g, b, a);
    STBTTF_RenderText(m_pRenderer, font, position.x - alignment, (position.y + height), text.c_str());
}

void Launcher::DrawProgressBar(Vec2 pos, Vec2 size, float percent)
{
    const int padding = 2;
    SDL_Rect dst = { static_cast<int>(pos.x) ,static_cast<int>(pos.y) ,static_cast<int>(size.x) ,static_cast<int>(size.y) };
    SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 255);
    SDL_RenderFillRect(m_pRenderer, &dst);

    dst.x += padding;
    dst.y += padding;
    dst.w -= padding * 2;
    dst.h -= padding * 2;

    SDL_SetRenderDrawColor(m_pRenderer, 103 / 3, 44 / 3, 146 / 3, 255);
    SDL_RenderFillRect(m_pRenderer, &dst);

    dst.w = static_cast<int>(dst.w * percent);

    SDL_SetRenderDrawColor(m_pRenderer, 103, 44, 146, 255);
    SDL_RenderFillRect(m_pRenderer, &dst);
}

static std::thread workerThread;

void Launcher::StartGame()
{
    m_State = LauncherState_GameSelected;

    // Dummy thread to simulate how loading/converting will work.
    workerThread = std::thread([this] {
        SetProgressMessage("Starting Conversion...");
        SDL_Delay(2000);

        for (int i = 0; i < 1000; i++)
        {
            SetProgressMessage("Converting File " + std::to_string(i));
            SetProgress(i / 1000.0f);
            SDL_Delay(20);
        }

        SetProgressMessage("Done!");
        });
}

void SoundEngineCallback(void* userdata, Uint8* stream, int32_t len)
{
    memset(stream, 0, len);

    const int sampleCount = len / sizeof(MenuSampleStereo);
    reinterpret_cast<Launcher*>(userdata)->RenderAudio(reinterpret_cast<MenuSampleStereo*>(stream), sampleCount);
}

void Launcher::Initialize()
{
    m_MenuMusicAo = LoadMusicFile(CONTENT_DIR "ao.dat");
    m_MenuMusicAe = LoadMusicFile(CONTENT_DIR "ae.dat");

    InitAudio(this, SoundEngineCallback);

    m_TexBG1 = LoadTexture(CONTENT_DIR "bg1.png");
    m_TexBG2 = LoadTexture(CONTENT_DIR "bg2.png");
    m_TexLogoAo = LoadTexture(CONTENT_DIR "logo1.png");
    m_TexLogoAe = LoadTexture(CONTENT_DIR "logo2.png");

    m_FontSmall = STBTTF_OpenFont(m_pRenderer, CONTENT_DIR "oddfont.ttf", 32);
    m_FontMedium = STBTTF_OpenFont(m_pRenderer, CONTENT_DIR "oddfont.ttf", 64);
    m_FontLarge = STBTTF_OpenFont(m_pRenderer, CONTENT_DIR "oddfont.ttf", 128);
}

void Launcher::OnEvent(SDL_Event& pEvent)
{
    if (m_State == LauncherState_GameSelection)
    {
        if (pEvent.type == SDL_KEYDOWN)
        {
            if (pEvent.key.keysym.scancode == SDL_SCANCODE_LEFT)
                m_SelectionIndex = 0;
            else if (pEvent.key.keysym.scancode == SDL_SCANCODE_RIGHT)
                m_SelectionIndex = 1;
            if (pEvent.key.keysym.scancode == SDL_SCANCODE_SPACE || pEvent.key.keysym.scancode == SDL_SCANCODE_RETURN)
                StartGame();
        }
        else if (pEvent.type == SDL_CONTROLLERBUTTONDOWN)
        {
            if (pEvent.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT)
                m_SelectionIndex = 0;
            else if (pEvent.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
                m_SelectionIndex = 1;
            else if (pEvent.cbutton.button == SDL_CONTROLLER_BUTTON_A)
                StartGame();
        }
    }
}

void Launcher::Update(float td)
{
}

void Launcher::Draw(float td, SDL_Rect region)
{
    SDL_SetRenderDrawColor(m_pRenderer, 255, 0, 0, 255);
    SDL_RenderClear(m_pRenderer);
    SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_BLEND);

    m_SelectionSmooth = lerp(m_SelectionSmooth, static_cast<float>(m_SelectionIndex), 0.1f * td);
    float selectionCurtained = 0;
    float curtainTarget = 0.2f + (m_SelectionIndex * 0.6f);

    static float clipSmooth = 0;
    clipSmooth = lerp(clipSmooth, (m_State == LauncherState_GameSelected) ? m_SelectionIndex : curtainTarget, 0.1f * td);

    int clipXPos = static_cast<int>(m_ScreenWidth * clipSmooth);

    const SDL_Rect regions[2] = {
        { 0, 0, m_ScreenWidth - clipXPos, m_ScreenHeight },
        { m_ScreenWidth - clipXPos, 0, m_ScreenWidth - (m_ScreenWidth - clipXPos), m_ScreenHeight }
    };

    
    DrawGameTile(regions[0], m_TexBG1, m_TexLogoAo, m_SelectionSmooth * -(m_ScreenWidth / 6.0f), m_SelectionSmooth);
    DrawGameTile(regions[1], m_TexBG2, m_TexLogoAe, (1.0f - m_SelectionSmooth) * (m_ScreenWidth / 6.0f), (1.0f - m_SelectionSmooth));

    SDL_RenderSetClipRect(m_pRenderer, &regions[m_SelectionIndex]);
    
    STBTTF_Font* progressFont = m_FontMedium;

    if (m_State == LauncherState_GameSelected)
    {
        std::string text = m_LoadMessage;

        DrawProgressBar({ regions[m_SelectionIndex].x + (regions[m_SelectionIndex].w * 0.2f), regions[m_SelectionIndex].h / 1.1f }, { regions[m_SelectionIndex].w * 0.6f, 10.0f }, m_LoadProgress);
        DrawText(progressFont, text, { regions[m_SelectionIndex].x + (regions[m_SelectionIndex].w / 2.0f), regions[m_SelectionIndex].h / 1.2f }, 255, 255, 255, 255, TextAlign_Centre);
    }
    
    SDL_RenderSetClipRect(m_pRenderer, nullptr);
}

void Launcher::Shutdown()
{
    STBTTF_CloseFont(m_FontSmall);
    STBTTF_CloseFont(m_FontMedium);
    STBTTF_CloseFont(m_FontLarge);
}
