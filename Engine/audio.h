#pragma once
#include "common.h"

class SoundBank;
class SoundEvent;

// FMOD Globals
extern API bool global_fmodLoaded;
extern API FMOD::Studio::System* fmodSystem;

extern API SoundBank* masterBank;
extern API SoundBank* stringsBank;

using namespace FMOD;

API void InitFMOD();

API void DestroyFMOD();

class SoundEvent
{
public:
    API SoundEvent(const char* name);

    API void Play();

    API void Reload();

    API int GetInstanceCount();

private:
    void Load(const char* name);

	FMOD::Studio::EventDescription* ev;
    const char* name;
};

class SoundBank
{
public:
    API SoundBank(const char* name);

    API void Load(const char* name);

    API void Reload();

    API ~SoundBank();

private:
    void Destroy();

    FMOD::Studio::Bank* bank;
    const char* name;
};