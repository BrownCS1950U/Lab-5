#pragma once
#include <raudio.h>
#include <glm/vec3.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace gl {

class AudioEngine {
public:
    static AudioEngine& instance();

    /* life-cycle -------------------------------------------------- */
    void init();
    void shutdown();
    bool ready() const;

    /* loading ----------------------------------------------------- */
    void loadSound (const std::string& path, const std::string& name);
    void loadMusic (const std::string& path, const std::string& name, bool looping = true);
    void unloadSound(const std::string& name);
    void unloadMusic(const std::string& name);

    /* playback ---------------------------------------------------- */
    void playSound (const std::string& name,
                    float volume = 1.f, float pitch = 1.f, float pan = 0.5f);
    void playSound3D(const std::string& name, const glm::vec3& worldPos,
                     float pitch = 1.f);               // <-- NEW

    void playMusic  (const std::string& name);
    void pauseMusic (const std::string& name);
    void resumeMusic(const std::string& name);
    void stopMusic  (const std::string& name);
    void seekMusic  (const std::string& name, float seconds);

    /* per-asset controls ----------------------------------------- */
    void setSoundVolume(const std::string& name, float v);
    void setSoundPitch (const std::string& name, float p);
    void setSoundPan   (const std::string& name, float pan);
    void setMusicVolume(const std::string& name, float v);
    void setMusicPitch (const std::string& name, float p);
    void setMusicPan   (const std::string& name, float pan);

    /* global controls -------------------------------------------- */
    static void setMasterVolume(float v);
    static float masterVolume() ;

    /* 3-D listener ------------------------------------------------ */
    void setListener(const glm::vec3& worldPos);        // <-- NEW
    void setMaxDistance(float metres);                  // <-- NEW

    /* pump once per frame ---------------------------------------- */
    void update();

    /* low-level escape hatches ----------------------------------- */
    Sound  rawSound(const std::string& name)  const;
    Music& rawMusic(const std::string& name);

private:
    AudioEngine();
    ~AudioEngine();
    AudioEngine(const AudioEngine&)            = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    struct SoundEntry { Sound sound; };
    struct MusicEntry { Music music; };
    struct ActiveAlias
    {
        Sound alias;
        glm::vec3 pos;   // world-space, unused for 2-D
        bool spatial;
    };

    std::unordered_map<std::string, SoundEntry> m_sounds;
    std::unordered_map<std::string, MusicEntry> m_musics;
    std::vector<ActiveAlias>                    m_active;

    glm::vec3 m_listener   {0.0f};
    float     m_maxDist    = 25.0f;      // metres where volume hits 0
    bool      m_initialized = false;

    /* helpers */
    static bool  musicIsPlaying(const MusicEntry&) ;
    void  updateAliasSpatial(ActiveAlias& a);
};

} // namespace gl
