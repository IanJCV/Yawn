#pragma once
#include "common.h"


namespace engine::audio
{
    class SoundBank;
    class SoundEvent;

    // FMOD Globals
    extern ENGINE_API bool global_fmodLoaded;
    extern ENGINE_API FMOD::Studio::System* fmodSystem;

    extern ENGINE_API SoundBank* masterBank;
    extern ENGINE_API SoundBank* stringsBank;

    using namespace FMOD;

    ENGINE_API void InitFMOD();

    ENGINE_API void DestroyFMOD();

    class SoundEvent
    {
    public:
        ENGINE_API SoundEvent(const char* name);

        ENGINE_API void Play();

        ENGINE_API void Reload();

        ENGINE_API int GetInstanceCount();

    private:
        void Load(const char* name);

        FMOD::Studio::EventDescription* ev;
        const char* name;
    };

    class SoundBank
    {
    public:
        ENGINE_API SoundBank(const char* name);

        ENGINE_API void Load(const char* name);

        ENGINE_API void Reload();

        ENGINE_API ~SoundBank();

    private:
        void Destroy();

        FMOD::Studio::Bank* bank;
        const char* name;
    };
}