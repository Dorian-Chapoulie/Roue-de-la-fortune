#ifndef SOUNDBANK_H
#define SOUNDBANK_H

#include <QtMultimedia/QSound>
#include <map>

class SoundBank
{

public:
    enum SOUND {
        GOOD_CHOICE,
        BAD_CHOICE,
        CLICK,
        BANKRUPT,
        QUICK_RIDDLE_VICTORY,
        SENTENCE_RIDDLE_VICTORY,
        SAMBA_VICTORY,
        LOOSE,
        SPIN,
        HOLDUP,
        PASSE
    };

    static SoundBank* getInstance();
    void playSound(SOUND sound);
    void stopSpinSound();

private:
    SoundBank();    
    ~SoundBank();

    std::map<SOUND, std::string> soundsPath;
    QSound* spin = nullptr;

    static SoundBank* instance;
};

#endif
