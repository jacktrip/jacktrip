#pragma once

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
  const CompressorPreset voice { 2.0f, -24.0f, 15.0f, 40.0f, 2.0f };
  const CompressorPreset horns { 3.0f, -10.0f, 100.0f, 250.0f, 2.0f };
  const CompressorPreset snare { 5.0f, -4.0f, 5.0f, 150.0f, 3.0f };
}

// Dynamic extension of CompressorPresets:
struct CompressorPresetList {
  std::vector<CompressorPreset*> presets;
  enum PresetNames { PN_VOICE, PN_BRASS, PN_SNARE, PN_NUMPRESETS };
  CompressorPresetList() { // define some standard presets
    presets.push_back( new CompressorPreset(CompressorPresets::voice));
    presets.push_back( new CompressorPreset(CompressorPresets::horns));
    presets.push_back( new CompressorPreset(CompressorPresets::snare));
  }
  ~CompressorPresetList() = default;
};
