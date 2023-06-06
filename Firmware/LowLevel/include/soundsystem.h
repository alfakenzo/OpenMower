// Created by Elmar Elflein on 18/07/22.
// Copyright (c) 2022 Elmar Elflein. All rights reserved.
//
// This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//
// Feel free to use the design in your private/educational projects, but don't try to sell the design or products based on it without getting my consent first.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
#ifndef _SOUND_SYSTEM_H_
#define _SOUND_SYSTEM_H_

#include <Arduino.h>
#include <stdint.h>
#include <list>
#include <DFMiniMp3.h>

#include "datatypes.h"

#define DFP_ADVERT_FOLDER 1U
#define DFP_ONLINE_TIMEOUT 5000
#define BUFFERSIZE 100

class MP3Sound;                               // forward declaration ...
typedef DFMiniMp3<SerialPIO, MP3Sound> DfMp3; // ... for a more readable/shorter DfMp3 typedef

// Non thread safe singleton MP3Sound class
class MP3Sound
{
protected:
    MP3Sound(); // Singleton constructors always should be private to prevent direct construction via 'new'

public:
    enum TrackTypes : uint8_t
    {
        background = 1, // Background tracks are stored in folder mp3 and might be interrupted/aborted by higher priority sounds
        advert,         // Advert tracks are stored in language specific folder, i.e. "01" US or "49" German, and interrupt/stop background sounds
        advert_raw,     // Raw-Advert tracks are stored in folder advert and interrupt/stop background or advert sounds.
                        // Due to DFPlayer incompatibilities, advert_raw should only be used if you know their drawbacks!
    };
    enum TrackFlags : uint8_t
    {
        repeat = 0x01,         // Repeat this track, till a new sound get played
        stopBackground = 0x02, // Stop replay of current running background track after this sound got played
    };
    struct TrackDef
    {
        uint16_t num;
        TrackTypes type;
        uint8_t flags = 0;
    };

    bool playing;

    static MP3Sound *GetInstance();
    MP3Sound(MP3Sound &other) = delete;        // Singletons should not be cloneable
    void operator=(const MP3Sound &) = delete; // Singletons should not be assignable

    bool begin();                                   // Init serial stream, soundmodule and sound_available_
    void playSound(TrackDef t_track_def);           // Play sound track number. This method writes sound track nr in a list, the method processSounds() (has to run in loop)
                                                    // will play the sounds according to the list
    void playSoundAdHoc(TrackDef t_track_def);      // Play sound track number immediately without waiting until the end of sound
    void setVolume(uint8_t t_vol);                  // Scales loudness from 0 to 100 %
    int sounds2play();                              // returns the number if sounds to play in the list
    void processSounds(ll_status t_status_message); // This method has to be called cyclic, e.g. every second.

    // DFMiniMP3 specific notification methods
    static void OnError(DfMp3 &mp3, uint16_t errorCode);
    static void OnPlayFinished(DfMp3 &mp3, DfMp3_PlaySources source, uint16_t track);
    static void OnPlaySourceOnline(DfMp3 &mp3, DfMp3_PlaySources source);
    static void OnPlaySourceInserted(DfMp3 &mp3, DfMp3_PlaySources source);
    static void OnPlaySourceRemoved(DfMp3 &mp3, DfMp3_PlaySources source);

private:
    std::list<TrackDef> active_sounds_;
    bool sound_available_; // Sound module available as well as SD-Card with some kind of files
    uint16_t last_error_code_ = 0;
    ll_status status_message_ = {0};      // Last processed LowLevel status message
    TrackDef background_track_def_ = {0}; // Current/last background track num
    TrackDef current_track_def_ = {0};    // Current playing background track num
};

#endif // _SOUND_SYSTEM_H_  HEADER_FILE
