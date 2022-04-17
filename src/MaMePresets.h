#pragma once

#include <array>

struct MaMePreset {
    float ratio;
    float thresholdDB;
    float attackMS;
    float releaseMS;
    float makeUpGainDB;
};

namespace MaMePresets
{
//                     name   ratio  thresh  attack  rel  mugain
const MaMePreset voice{2.0f, -24.0f, 15.0f, 40.0f, 2.0f};
const MaMePreset horns{3.0f, -10.0f, 100.0f, 250.0f, 2.0f};
const MaMePreset snare{5.0f, -4.0f, 5.0f, 150.0f, 3.0f};
const unsigned int numPresets{3};
const std::array<MaMePreset, numPresets> standardPresets{voice, horns, snare};
enum MaMePresetNames { CPN_VOICE, CPN_BRASS, CPN_SNARE, CPN_NUMPRESETS };
}  // namespace MaMePresets

#if 0  // not yet using this
// Dynamic extension of MaMePresets:
struct MaMePresetList {
  std::vector<MaMePreset*> presets;
  MaMePresetList() { // define some standard presets
    presets.push_back( new MaMePreset(MaMePresets::voice) );
    presets.push_back( new MaMePreset(MaMePresets::horns) );
    presets.push_back( new MaMePreset(MaMePresets::snare) );
  }
  ~MaMePresetList() = default;
};
#endif

// clang-format off


// clang-format on
