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

struct CompressorPresets {
  std::vector<CompressorPreset*> presets;
  enum PresetNames { PN_VOICE, PN_BRASS, PN_SNARE, PN_NUMPRESETS };
  CompressorPresets() { // define some standard presets
    presets.push_back( new CompressorPreset(2.0f, -24.0f, 15.0f, 40.0f, 2.0f) ); // voice
    presets.push_back( new CompressorPreset(3.0f, -10.0f, 100.0f, 250.0f, 2.0f) ); // brass
    presets.push_back( new CompressorPreset(5.0f, -4.0f, 5.0f, 150.0f, 3.0f) ); // snare drum
  }
  ~CompressorPresets() = default;
};
