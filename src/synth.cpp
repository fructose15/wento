#include "synth.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

#if defined(WENTO_OS_WINDOWS) && WENTO_OS_WINDOWS
  #include <windows.h>
  #include <mmsystem.h>
#endif

namespace wento {

namespace {
constexpr double kPi = 3.14159265358979323846;

std::string toLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return s;
}

double sampleWaveform(WaveformType wf, double phase, int harmonic_idx) {
  double p = phase - std::floor(phase);
  switch (wf) {
    case WaveformType::Sine:
      return std::sin(2.0 * kPi * p);
    case WaveformType::Triangle:
      return 1.0 - 4.0 * std::abs(std::round(p - 0.25) - (p - 0.25));
    case WaveformType::Sawtooth:
      return 2.0 * (p - std::floor(p + 0.5));
    case WaveformType::Square:
      return (p < 0.5) ? 1.0 : -1.0;
    case WaveformType::FmBell: {
      double mod = std::sin(2.0 * kPi * p * (2.0 + harmonic_idx * 0.5)) * 1.4;
      return std::sin(2.0 * kPi * p + mod);
    }
  }
  return std::sin(2.0 * kPi * p);
}

void writeUint32LE(std::ofstream& out, uint32_t v) {
  char buf[4] = {
    static_cast<char>(v & 0xFF),
    static_cast<char>((v >> 8) & 0xFF),
    static_cast<char>((v >> 16) & 0xFF),
    static_cast<char>((v >> 24) & 0xFF)
  };
  out.write(buf, 4);
}

void writeUint16LE(std::ofstream& out, uint16_t v) {
  char buf[2] = {
    static_cast<char>(v & 0xFF),
    static_cast<char>((v >> 8) & 0xFF)
  };
  out.write(buf, 2);
}

bool writeWavFile(const std::string& path, const std::vector<int16_t>& samples, uint32_t sample_rate) {
  std::ofstream out(path, std::ios::binary);
  if (!out) return false;

  uint32_t data_bytes = static_cast<uint32_t>(samples.size() * sizeof(int16_t));
  uint32_t riff_size = 36 + data_bytes;

  out.write("RIFF", 4);
  writeUint32LE(out, riff_size);
  out.write("WAVE", 4);
  out.write("fmt ", 4);
  writeUint32LE(out, 16);
  writeUint16LE(out, 1); // PCM
  writeUint16LE(out, 1); // Mono
  writeUint32LE(out, sample_rate);
  writeUint32LE(out, sample_rate * 2);
  writeUint16LE(out, 2);
  writeUint16LE(out, 16);
  out.write("data", 4);
  writeUint32LE(out, data_bytes);
  out.write(reinterpret_cast<const char*>(samples.data()), static_cast<std::streamsize>(data_bytes));
  return out.good();
}

} // namespace

double AudioSynth::parsePitchHz(const std::string& pitch_str) {
  std::string s = toLower(pitch_str);
  if (s == "rest" || s == "-" || s == "0") {
    return 0.0;
  }

  // Direct Hz format e.g. 440hz
  if (s.size() > 2 && s.substr(s.size() - 2) == "hz") {
    try {
      return std::stod(s.substr(0, s.size() - 2));
    } catch (...) {
      return -1.0;
    }
  }

  if (s.size() < 2) return -1.0;

  int semitone = 0;
  switch (s[0]) {
    case 'c': semitone = 0; break;
    case 'd': semitone = 2; break;
    case 'e': semitone = 4; break;
    case 'f': semitone = 5; break;
    case 'g': semitone = 7; break;
    case 'a': semitone = 9; break;
    case 'b': semitone = 11; break;
    default: return -1.0;
  }

  size_t idx = 1;
  if (idx < s.size() && (s[idx] == '#' || s[idx] == 'b')) {
    semitone += (s[idx] == '#') ? 1 : -1;
    idx++;
  }

  if (idx >= s.size()) return -1.0;
  int octave = 4;
  try {
    octave = std::stoi(s.substr(idx));
  } catch (...) {
    return -1.0;
  }

  int midi = (octave + 1) * 12 + semitone;
  return 440.0 * std::pow(2.0, (midi - 69) / 12.0);
}

bool AudioSynth::playSynthesizedNote(const NoteInstrument& inst, double freq_hz, double duration_ms) {
  if (duration_ms <= 0.0) return true;
  if (freq_hz <= 0.0) {
    sleepMs(duration_ms);
    return true;
  }

  const uint32_t sr = WENTO_SAMPLE_RATE;
  const size_t total_samples = static_cast<size_t>((duration_ms / 1000.0) * sr);
  std::vector<int16_t> buffer(total_samples, 0);

  double attack_s = std::max(0.002, inst.attack_ms / 1000.0);
  double decay_s = std::max(0.005, inst.decay_ms / 1000.0);
  double release_s = std::max(0.005, inst.release_ms / 1000.0);
  double total_s = duration_ms / 1000.0;

  double harmonic_norm = 0.0;
  for (double amp : inst.harmonics) harmonic_norm += std::abs(amp);
  if (harmonic_norm <= 0.0) harmonic_norm = 1.0;

  for (size_t i = 0; i < total_samples; ++i) {
    double t = static_cast<double>(i) / sr;
    double env = inst.sustain;
    if (t < attack_s) {
      env = t / attack_s;
    } else if (t < attack_s + decay_s) {
      double dt = (t - attack_s) / decay_s;
      env = 1.0 - (1.0 - inst.sustain) * dt;
    }
    if (total_s - t < release_s) {
      double rt = std::max(0.0, (total_s - t) / release_s);
      env *= rt;
    }

    double value = 0.0;
    for (size_t h = 0; h < inst.harmonics.size(); ++h) {
      double partial_freq = freq_hz * static_cast<double>(h + 1);
      if (partial_freq < sr * 0.45) {
        value += inst.harmonics[h] * sampleWaveform(inst.waveform, partial_freq * t, static_cast<int>(h));
      }
    }
    value = (value / harmonic_norm) * env * 0.72;
    value = std::max(-1.0, std::min(1.0, value));
    buffer[i] = static_cast<int16_t>(value * 32767.0);
  }

  std::string tmp_wav = "/tmp/wento_note.wav";
#if defined(WENTO_OS_WINDOWS) && WENTO_OS_WINDOWS
  tmp_wav = "wento_note_tmp.wav";
#endif

  if (!writeWavFile(tmp_wav, buffer, sr)) {
    sleepMs(duration_ms);
    return false;
  }

#if defined(WENTO_OS_WINDOWS) && WENTO_OS_WINDOWS
  PlaySoundA(tmp_wav.c_str(), NULL, SND_FILENAME | SND_SYNC);
  std::remove(tmp_wav.c_str());
#else
  std::string player = WENTO_AUDIO_PLAYER;
  if (player != "none" && !player.empty()) {
    std::string cmd = player + " \"" + tmp_wav + "\" >/dev/null 2>&1";
    int rc = std::system(cmd.c_str());
    (void)rc;
  } else {
    sleepMs(duration_ms);
  }
  std::remove(tmp_wav.c_str());
#endif
  return true;
}

bool AudioSynth::playHttpsAudio(const std::string& url) {
  if (url.rfind("https://", 0) != 0) {
    std::cerr << "[wento error] play audio requires an https:// URL.\n";
    return false;
  }

#if defined(WENTO_OS_WINDOWS) && WENTO_OS_WINDOWS
  std::string tmp_file = "wento_remote_audio.wav";
  std::string dl_cmd = "powershell -NoProfile -Command \"Invoke-WebRequest -Uri '" + url + "' -OutFile '" + tmp_file + "'\"";
  if (std::system(dl_cmd.c_str()) != 0) {
    std::cerr << "[wento error] Failed to download HTTPS audio: " << url << "\n";
    return false;
  }
  PlaySoundA(tmp_file.c_str(), NULL, SND_FILENAME | SND_SYNC);
  std::remove(tmp_file.c_str());
  return true;
#else
  std::string tmp_file = "/tmp/wento_remote_audio.bin";
  std::string dl_cmd = "curl -fsSL \"" + url + "\" -o \"" + tmp_file + "\"";
  if (std::system(dl_cmd.c_str()) != 0) {
    std::cerr << "[wento error] Failed to fetch HTTPS audio stream: " << url << "\n";
    return false;
  }
  std::string player = WENTO_AUDIO_PLAYER;
  if (player != "none" && !player.empty()) {
    std::string play_cmd = player + " \"" + tmp_file + "\" >/dev/null 2>&1";
    int rc = std::system(play_cmd.c_str());
    (void)rc;
  }
  std::remove(tmp_file.c_str());
  return true;
#endif
}

void AudioSynth::sleepMs(double duration_ms) {
  if (duration_ms <= 0.0) return;
  std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int64_t>(duration_ms * 1000.0)));
}

} // namespace wento
