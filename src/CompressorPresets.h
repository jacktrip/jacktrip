#pragma once

#include <array>

struct CompressorPreset {
  float ratio;
  float thresholdDB;
  float attackMS;
  float releaseMS;
  float makeUpGainDB;
  CompressorPreset(float r, float t, float a, float rel, float m)
    : ratio(r)
    , thresholdDB(t)
    , attackMS(a)
    , releaseMS(rel)
    , makeUpGainDB(m)
  {}
  ~CompressorPreset() = default;
};

namespace CompressorPresets
{
  //                     name   ratio  thresh  attack  rel  mugain
  const CompressorPreset voice { 2.0f, -24.0f, 15.0f, 40.0f, 2.0f };
  const CompressorPreset horns { 3.0f, -10.0f, 100.0f, 250.0f, 2.0f };
  const CompressorPreset snare { 5.0f, -4.0f, 5.0f, 150.0f, 3.0f };
  const uint numPresets { 3 };
  const std::array<CompressorPreset,numPresets> standardPresets { voice, horns, snare };
  enum CompressorPresetNames { CPN_VOICE, CPN_BRASS, CPN_SNARE, CPN_NUMPRESETS };
}

#if 0 // not yet using this
// Dynamic extension of CompressorPresets:
struct CompressorPresetList {
  std::vector<CompressorPreset*> presets;
  CompressorPresetList() { // define some standard presets
    presets.push_back( new CompressorPreset(CompressorPresets::voice) );
    presets.push_back( new CompressorPreset(CompressorPresets::horns) );
    presets.push_back( new CompressorPreset(CompressorPresets::snare) );
  }
  ~CompressorPresetList() = default;
};
#endif

/* Settings from http://www.anythingpeaceful.org/sonar/settings/comp.html

   Name     Thresh(dB) Att(ms) Rel(ms) Ratio:1 Gain(dB)    Comments
   Vocal 1     -20     31      342     2.5     2       Compressor for Solo Vocal
   Vocal 2     -8      26      331     2.5     1.5     Variation of Solo 1
   Full Comp 1 -8      60      2500    2.5     0       For Overall Volume Level
   Full Comp 2 -18     94      447     3.5     2.5     Variation of Total Comp 1: Harder ratio
   Full Comp 3 -16     11      180     6       6       Nearly a limiter effect
   Kick Comp   -24     9       58      3       5.5     Compressor for Acoustic Bass Drum
   Snare Comp  -17     8       12      2.5     3.5     Compressor for Acoustic Snare Drum
   Guitar      -10     5       238     2.5     1.5     Compressor for Acoustic Guitar
   Brass Sec   -18     18      226     1.7     4       Brass Sounds for Strong Attacks
   Bass 1      -12     15      470     2       4.5     Finger Picked Bass Guitar
   Bass 2      -12     6       133     1.7     4       Slap Electric Bass
   E Guitar    -8      7       261     3.5     2.5     Electric Guitar Compressor
   Piano 1     -9      17      238     2.5     1       Brightens Piano
   Piano 2     -18     7       174     3.5     6       Variation of Piano 1
   Kick        -14     2       35      2       3.5     For sampled Bass Drum
   Snare       -18     8       354     4       8       For sampled Snare Drum
   Strings 1   -11     33      749     2       1.5     For String instruments
   Strings 2   -12     93      2500    1.5     1.5     For Violas and Cellos
   Strings 3   -17     76      186     1.5     2.5     Cellos or DoubleBass
   Syn Bass    -10     9       250     3.5     3       Adjust level of Synth Bass
   Syn Pad     -13     58      238     2       2       Prevents diffusion of sound in synth pad
   Limiting    -1      0.1     325     20      0       Slow release limiter
   Chorusing   -9      39      225     1.7     2.5     For vocal Chorusing

   From https://www.dummies.com/art-center/music/recording-music/dynamic-music-compression-settings-for-horns-piano-and-percussion/

   Horns       –8      100     300     2.5-3   2       Brasses not normally compressed [jos gain estimate based on above table]
   Piano       -10     100-105 115     1.5-2   2       Normally not compressed ["]
   Kick        -6      40-50   200-300 4-6     3       Looks more like a limiter to me [jos] ["]
   Snare       -4      5-10    125-175 4-6     3       Crucial for a tight, punchy sound
   Bongos      -6      10-25   100-300 3-6     3       "Hand Drums" - protect against excess "slap"
   Perc.       -10     10-20   50      3-6     3       Transient overdrive protection in mix

*/
