#include "soundbank.h"
#include "config/config.h"

SoundBank* SoundBank::instance = nullptr;

SoundBank* SoundBank::getInstance() {
    if(instance == nullptr)
        instance = new SoundBank();
    return instance;
}
//inti some sounds
SoundBank::SoundBank()
{
    const std::string basePath = Config::getInstance()->baseRessourcesPath
            + "/"
            + Config::getInstance()->soundFolder;

    soundsPath.insert(std::make_pair(GOOD_CHOICE, basePath + "/BonChoix.wav"));
    soundsPath.insert(std::make_pair(BAD_CHOICE, basePath + "/MauvaisChoix.wav"));
    soundsPath.insert(std::make_pair(CLICK, basePath + "/click.wav"));
    soundsPath.insert(std::make_pair(BANKRUPT, basePath + "/bankrupt.wav"));
    soundsPath.insert(std::make_pair(QUICK_RIDDLE_VICTORY, basePath + "/Victory.wav"));
    soundsPath.insert(std::make_pair(SENTENCE_RIDDLE_VICTORY, basePath + "/Manche_gagne.wav"));
    soundsPath.insert(std::make_pair(SAMBA_VICTORY, basePath + "/SambaVictory.wav"));
    soundsPath.insert(std::make_pair(LOOSE, basePath + "/Loose.wav"));
    soundsPath.insert(std::make_pair(HOLDUP, basePath + "/holdup.wav"));
    soundsPath.insert(std::make_pair(PASSE, basePath + "/passe.wav"));

    spin = new QSound(QString::fromStdString(basePath + "/spin.wav"));
}

SoundBank::~SoundBank() {
    soundsPath.clear();
    delete instance;
    instance = nullptr;
}
//Play the sound
//The spin sound is special, because we need to monitor it.
void SoundBank::playSound(SoundBank::SOUND sound) {
    if(sound == SPIN) {
        spin->play();
    }else {
        QSound::play(QString::fromStdString(soundsPath[sound]));
    }
}

void SoundBank::stopSpinSound() {
    spin->stop();
}
