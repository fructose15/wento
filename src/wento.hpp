#ifndef WENTO_HPP
#define WENTO_HPP

#if defined(WENTO_USE_UNIX_CONFIG)
  #include "config.unix.h"
#elif defined(_WIN32) || defined(_WIN64) || defined(WENTO_USE_WIN32_CONFIG)
  #include "config.h"
#else
  #include "config.h"
#endif

#include <string>
#include <vector>

namespace wento {

enum class WaveformType {
  Sine,
  Triangle,
  Sawtooth,
  Square,
  FmBell
};

struct NoteInstrument {
  std::string name;
  std::string author;
  WaveformType waveform = WaveformType::Sine;
  double attack_ms = 12.0;
  double decay_ms = 120.0;
  double sustain = 0.65;
  double release_ms = 180.0;
  std::vector<double> harmonics = {1.0};
  std::string description;
};

} // namespace wento

#endif // WENTO_HPP
