#ifndef SoundSystem_h__
#define SoundSystem_h__

#include <unordered_map>

#include "glm/common.hpp"
#include "glm/gtx/rotate_vector.hpp" // Calculate Up vector
#include "OpenAL/al.h"
#include "OpenAL/alc.h"

#include "Core/World.h"
#include "Core/EventBroker.h"
#include "Core/Transform.h" // Absolute transform
#include "Sound/Sound.h"
#include "Sound/EPlaySoundOnEntity.h"
#include "Sound/EPlaySoundOnPosition.h"
#include "Sound/EPlayBackgroundMusic.h"
#include "Sound/EPauseSound.h"
#include "Sound/EContinueSound.h"
#include "Sound/EStopSound.h"
#include "Sound/ESetBGMGain.h"
#include "Sound/ESetSFXGain.h"

enum class SoundType {
    SFX,
    BGM
};

struct Source
{
    Source() { }
    Sound* SoundResource = nullptr;
    ALuint ALsource;
    SoundType Type;
};

class SoundSystem
{
public:
    SoundSystem() { }
    SoundSystem(World* world, EventBroker* eventBroker, bool editorMode);
    ~SoundSystem();
    // Update emitters / listener
    void Update(double dt); 
private:
    // Help functions for working with OpenaAL
    void setListenerPos(glm::vec3 pos) { alListener3f(AL_POSITION, pos.x, pos.y, pos.z); };
    void setListenerVel(glm::vec3 vel) { alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z); };
    void setListenerOri(glm::vec3 ori);
    glm::vec3 listnerPos() { glm::vec3 pos; alGetListener3f(AL_POSITION, &pos.x, &pos.y, &pos.z); return pos; };
    glm::vec3 listenerVel() { glm::vec3 vel; alGetListener3f(AL_VELOCITY, &vel.x, &vel.y, &vel.z); return vel; };
    glm::vec3 listenerOri() { glm::vec3 ori; alGetListener3f(AL_ORIENTATION, &ori.x, &ori.y, &ori.z); return ori; };
    void setSourcePos(ALuint source, glm::vec3 pos) { ALfloat spos[3] = { pos.x, pos.y, pos.z };  alSourcefv(source, AL_POSITION, spos); };
    void setSourceVel(ALuint source, glm::vec3 vel) { ALfloat svel[3] = { vel.x, vel.y, vel.z };  alSourcefv(source, AL_VELOCITY, svel); };

    // Logic 
    void initOpenAL();
    void updateEmitters(double dt);
    void updateListener(double dt);
    void deleteInactiveEmitters();
    void addNewEmitters(double dt);
    Source* createSource(std::string filePath);
    void playSound(Source* source);
    void stopSound(Source* source);
    void stopEmitters();
    ALenum getSourceState(ALuint source);
    void setGain(Source* source, float gain);
    void setSoundProperties(ALuint source, ComponentWrapper* soundComponent);

    // OpenAL system variables
    ALCdevice* m_ALCdevice = nullptr;
    ALCcontext* m_ALCcontext = nullptr;

    // Logic
    World* m_World = nullptr;
    EventBroker* m_EventBroker = nullptr;
    std::unordered_map<EntityID, Source*> m_Sources;
    float m_BGMVolumeChannel = 1.0f;
    float m_SFXVolumeChannel = 1.f;
    bool m_EditorEnabled = false;

    // Events
    EventRelay<SoundSystem, Events::PlaySoundOnEntity> m_EPlaySoundOnEntity;
    bool OnPlaySoundOnEntity(const Events::PlaySoundOnEntity &e);
    EventRelay<SoundSystem, Events::PlaySoundOnPosition> m_EPlaySoundOnPosition;
    bool OnPlaySoundOnPosition(const Events::PlaySoundOnPosition &e);
    EventRelay<SoundSystem, Events::PlayBackgroundMusic> m_EPlayBackgroundMusic;
    bool OnPlayBackgroundMusic(const Events::PlayBackgroundMusic &e);
    EventRelay<SoundSystem, Events::PauseSound> m_EPauseSound;
    bool OnPauseSound(const Events::PauseSound &e);
    EventRelay<SoundSystem, Events::StopSound> m_EStopSound;
    bool OnStopSound(const Events::StopSound &e);
    EventRelay<SoundSystem, Events::ContinueSound> m_EContinueSound;
    bool OnContinueSound(const Events::ContinueSound &e);
    EventRelay<SoundSystem, Events::SetBGMGain> m_ESetBGMGain; 
    bool OnSetBGMGain(const Events::SetBGMGain &e); // Not tested
    EventRelay<SoundSystem, Events::SetSFXGain> m_ESetSFXGain; 
    bool OnSetSFXGain(const Events::SetSFXGain &e); // Not tested
};

#endif