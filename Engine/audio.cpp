#include "audio.h"

API bool global_fmodLoaded = NULL;
API FMOD::Studio::System* fmodSystem = NULL;

API SoundBank* masterBank = NULL;
API SoundBank* stringsBank = NULL;

// SOUNDEVENT
API inline SoundEvent::SoundEvent(const char* name)
{
    Load(name);
    this->name = name;
}

API inline void SoundEvent::Play()
{
	FMOD::Studio::EventInstance* inst;
	ev->createInstance(&inst);
	inst->start();
	inst->setCallback([](FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE* ev, void* parameters) -> FMOD_RESULT
		{
			((FMOD::Studio::EventInstance*)ev)->release();
			return FMOD_OK;
		}, FMOD_STUDIO_EVENT_CALLBACK_STOPPED);
}

API int SoundEvent::GetInstanceCount()
{
	int count = 0;
	ev->getInstanceCount(&count);
	return count;
}

API void SoundEvent::Reload()
{
    ev->releaseAllInstances();
    Load(name);
}

void SoundEvent::Load(const char* name)
{
    FMOD_RESULT res = fmodSystem->getEvent(name, &ev);
    if (res != FMOD_OK)
    {
        DebugOut(L"[FMOD]: Couldn't find event with name %s! Check the spelling.\n", get_utf16(name));
    }
}


// GLOBALS
API void InitFMOD()
{
    if (global_fmodLoaded)
    {
        if (masterBank)
        {
            masterBank->Reload();
        }

        if (stringsBank)
        {
            stringsBank->Reload();
        }

        DebugOut(L"Reloaded FMOD.\n");
    }

    HRESULT res = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    assert(SUCCEEDED(res));

    FMOD_RESULT result = FMOD::Studio::System::create(&fmodSystem);

    assert(result == FMOD_OK);

    result = fmodSystem->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0);

    assert(result == FMOD_OK);

    masterBank = new SoundBank("sounds/Master.bank");
    stringsBank = new SoundBank("sounds/Master.strings.bank");

    global_fmodLoaded = true;
    DebugOut(L"FMOD initialized.\n");
}

API void DestroyFMOD()
{
    if (!global_fmodLoaded)
    {
        return;
    }

    if (masterBank)
    {
        delete masterBank;
    }

    if (stringsBank)
    {
        delete stringsBank;
    }

    if (fmodSystem)
    {
        fmodSystem->release();
    }
}


// SOUNDBANK
API SoundBank::SoundBank(const char* name)
{
    this->name = name;
    Load(name);

    bank->loadSampleData();
}

API void SoundBank::Load(const char* name)
{
    FMOD_RESULT res = fmodSystem->loadBankFile(name, FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
    if (res != FMOD_OK)
    {
        auto str = std::string(FMOD_ErrorString(res));
        auto c = get_utf16(str);
        DebugOut(L"[FMOD]::Error! Could not load bank file. Description: %s\n", c.c_str());
        exit(1);
    }
}

API void SoundBank::Reload()
{
    Destroy();
    Load(name);
    bank->loadSampleData();
}

void SoundBank::Destroy()
{
    if (bank)
    {
        bank->unloadSampleData();
        bank->unload();
    }
}

API SoundBank::~SoundBank()
{
    Destroy();
}
