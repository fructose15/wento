#ifndef WENTO_SYNTH_HPP
#define WENTO_SYNTH_HPP

#include "wento.hpp"
#include <string>

namespace wento {

class AudioSynth {
public:
  // Convert scientific pitch (e.g., "C4", "A#3", "Bb5", "440Hz") to Hz. Returns 0.0 for REST.
  static double parsePitchHz(const std::string& pitch_str);

  // Generate 16-bit mono PCM WAV samples for a note instrument and play via OS backend.
  static bool playSynthesizedNote(const NoteInstrument& inst, double freq_hz, double duration_ms);

  // Download (HTTPS only) and play an external audio file.
  static bool playHttpsAudio(const std::string& url);

  // Sleep accurately for duration_ms milliseconds.
  static void sleepMs(double duration_ms);
};

} // namespace wento

#endif // WENTO_SYNTH_HPP
