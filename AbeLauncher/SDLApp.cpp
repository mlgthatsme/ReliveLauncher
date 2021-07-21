#include "SDLApp.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTTF_IMPLEMENTATION
#include "stbttf.h"

SDLApp::SDLApp()
{
    g_AppInstance = this;
}

void SDLApp::Run()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    
    m_ScreenWidth = 1280;
    m_ScreenHeight = 720;

    m_pWindow = SDL_CreateWindow("Select Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_ScreenWidth,
        m_ScreenHeight,
        0);
    if (m_pWindow == nullptr)
    {
        ALIVE_FATAL("Failed to create GAME SELECT Window");
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (m_pRenderer == nullptr)
    {
        ALIVE_FATAL("Failed to create GAME SELECT Renderer");
    }

    m_LastFrameTick = SDL_GetTicks();

    m_pController = GetGameController();

    Initialize();

    while (m_AppRunning)
    {
        SDL_GetWindowSize(m_pWindow, &m_ScreenWidth, &m_ScreenHeight);

        int newTick = SDL_GetTicks();

        float frameDelta = ((newTick - m_LastFrameTick) / 1000.0f) * 60.0f;

        if (m_pController == nullptr)
            m_pController = GetGameController();

        // Get the next event
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                m_AppRunning = false;
            }

            if (m_pController == nullptr)
                m_pController = GetGameController();

            OnEvent(event);
        }

        UpdateTree(frameDelta);

        int w = 0;
        int h = 0;
        SDL_GetWindowSize(m_pWindow, &w, &h);
        DrawTree(frameDelta, { 0, 0, w, h });

        SDL_RenderPresent(m_pRenderer);

        m_LastFrameTick = newTick;
    }

    Shutdown();

    for (auto& t : m_LoadedTextures)
    {
        SDL_DestroyTexture(t.second->m_Texture);
        delete t.second;
    }

    m_LoadedTextures.clear();

    SDL_DestroyRenderer(m_pRenderer);
    SDL_DestroyWindow(m_pWindow);

    SDL_PauseAudio(1);
    SDL_CloseAudio();

    if (m_pController != nullptr)
    {
        SDL_GameControllerClose(m_pController);
    }
}

void SDLApp::Stop()
{
    m_AppRunning = false;
}

void UIElement::UpdateTree(float td)
{
    Update(td);

    for (auto& child : m_Children)
    {
        child->UpdateTree(td);
    }
}

void UIElement::DrawTree(float td, SDL_Rect region)
{
    Draw(td, region);

    for (auto& child : m_Children)
    {
        const SDL_Rect dst = { region.x + child->m_Transform.x, region.y + child->m_Transform.y, child->m_Transform.w, child->m_Transform.h };
        child->DrawTree(td, dst);
    }
}

void SDLApp::InitAudio(void* userData, SDL_AudioCallback callback)
{
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        m_AudioDeviceSpec.callback = callback;
        m_AudioDeviceSpec.format = AUDIO_S16;
        m_AudioDeviceSpec.channels = 2;
        m_AudioDeviceSpec.freq = 44100;
        m_AudioDeviceSpec.samples = 2048;
        m_AudioDeviceSpec.userdata = userData;

        if (SDL_OpenAudio(&m_AudioDeviceSpec, NULL) >= 0)
        {
            SDL_PauseAudio(0);
        }
    }
}


SDL_GameController* SDLApp::GetGameController()
{
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            SDL_GameController* controller = NULL;
            controller = SDL_GameControllerOpen(i);
            return controller;
        }
    }

    return nullptr;
}

Texture* SDLApp::LoadTexture(std::string path)
{
    FILE* fh = fopen(path.c_str(), "rb");

    int32_t x = 0, y = 0;
    int32_t comp = 0;
    const uint8_t* data = stbi_load_from_file(fh, &x, &y, &comp, 4);
    SDL_Texture* texture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, x, y);

    SDL_UpdateTexture(texture, NULL, data, x * 4);
    stbi_image_free((void*)data);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    fclose(fh);

    Texture * tex = new Texture(texture);
    m_LoadedTextures[path] = tex;

    return tex;
}

void SDLApp::DrawTexture(Texture* pTexture, Vec2 position, Vec2 size, Vec2 centre)
{
    SDL_Rect dst = { position.x - (size.x * centre.x), position.y - (size.y * centre.y), size.x, size.y};
    SDL_RenderCopy(m_pRenderer, pTexture->m_Texture, nullptr, &dst);
}

Texture::Texture(SDL_Texture* texture)
{
    m_Texture = texture;
    SDL_QueryTexture(texture, nullptr, nullptr, &m_Width, &m_Height);
}