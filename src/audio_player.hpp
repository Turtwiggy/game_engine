#pragma once

#include <SFML/Audio.hpp>
//Another source of error is when you try to create a huge number of sounds. 
//SFML internally has a limit; it can vary depending on the OS, but you should never exceed 256.

struct audio_player
{
public:

    void play_oneoff_sound(std::string sound_name = "res/audio/example_wav.wav", bool loop = false)
    {
        sound.stop();

        if (!buffer.loadFromFile(sound_name))
        {
            printf("error loading sound %s", sound_name);
            return;
        }

        sound.setBuffer(buffer);
        sound.setLoop(loop);

        sound.play();
    };

private:

    //The most common mistake is to let a sound buffer go out of scope
    //(and therefore be destroyed) while a sound still uses it.
    //Remember that a sound only keeps a pointer to the sound buffer 
    //that you give to it, it doesn't contain its own copy. You have to correctly 
    //manage the lifetime of your sound buffers so that they remain alive as long as they are used by sounds.
    sf::SoundBuffer buffer;
    sf::Sound sound;

};