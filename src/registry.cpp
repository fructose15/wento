#include "registry.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

namespace wento {

namespace {
std::string trim(const std::string& s) {
  size_t start = s.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) return "";
  size_t end = s.find_last_not_of(" \t\r\n");
  return s.substr(start, end - start + 1);
}

std::string lowerStr(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return s;
}

WaveformType parseWaveform(const std::string& raw) {
  std::string w = lowerStr(trim(raw));
  if (w == "triangle") return WaveformType::Triangle;
  if (w == "saw" || w == "sawtooth") return WaveformType::Sawtooth;
  if (w == "square") return WaveformType::Square;
  if (w == "fm_bell" || w == "bell") return WaveformType::FmBell;
  return WaveformType::Sine;
}

std::string waveformToString(WaveformType wf) {
  switch (wf) {
    case WaveformType::Sine: return "sine";
    case WaveformType::Triangle: return "triangle";
    case WaveformType::Sawtooth: return "sawtooth";
    case WaveformType::Square: return "square";
    case WaveformType::FmBell: return "fm_bell";
  }
  return "sine";
}
} // namespace

NoteRegistry::NoteRegistry() {
  loadBuiltins();
  loadLocalNotes();
}

std::string NoteRegistry::localStoreDir() const {
#if defined(WENTO_OS_WINDOWS) && WENTO_OS_WINDOWS
  const char* appdata = std::getenv("APPDATA");
  std::string base = appdata ? std::string(appdata) : ".";
  return base + "\\Wento\\notes";
#else
  const char* home = std::getenv("HOME");
  std::string base = home ? std::string(home) : ".";
  return base + "/.wento/notes";
#endif
}

void NoteRegistry::loadBuiltins() {
  notes_.push_back({"Piano", "Fructose", WaveformType::Triangle, 8, 180, 0.42, 240, {1.0, 0.5, 0.25, 0.12}, "Warm acoustic-style electric piano"});
  notes_.push_back({"Sine", "Fructose", WaveformType::Sine, 10, 80, 0.80, 140, {1.0}, "Pure fundamental sine oscillator"});
  notes_.push_back({"Saw", "Fructose", WaveformType::Sawtooth, 12, 120, 0.70, 180, {1.0, 0.35}, "Bright analog sawtooth lead"});
  notes_.push_back({"Square", "Fructose", WaveformType::Square, 6, 90, 0.65, 120, {1.0}, "Classic 8-bit chiptune square wave"});
  notes_.push_back({"Triangle", "Fructose", WaveformType::Triangle, 14, 100, 0.72, 160, {1.0, 0.2}, "Soft woodwind-inspired triangle tone"});
  notes_.push_back({"SubBass", "Fructose", WaveformType::Sine, 15, 160, 0.75, 220, {1.0, 0.15}, "Deep low-end sub oscillator"});
  notes_.push_back({"CrystalBell", "Fructose", WaveformType::FmBell, 4, 320, 0.25, 420, {1.0, 0.45, 0.22}, "Metallic FM bell resonator"});
}

void NoteRegistry::loadLocalNotes() {
  std::string dir = localStoreDir();
  std::error_code ec;
  if (!fs::exists(dir, ec)) return;

  for (const auto& entry : fs::directory_iterator(dir, ec)) {
    if (entry.path().extension() == ".wnote") {
      NoteInstrument inst;
      std::string err;
      if (parseWnoteFile(entry.path().string(), inst, err)) {
        auto it = std::find_if(notes_.begin(), notes_.end(), [&](const NoteInstrument& existing) {
          return lowerStr(existing.name) == lowerStr(inst.name);
        });
        if (it != notes_.end()) {
          *it = inst;
        } else {
          notes_.push_back(inst);
        }
      }
    }
  }
}

std::optional<NoteInstrument> NoteRegistry::findNote(const std::string& name) const {
  std::string target = lowerStr(name);
  for (const auto& n : notes_) {
    if (lowerStr(n.name) == target) {
      return n;
    }
  }
  return std::nullopt;
}

std::vector<NoteInstrument> NoteRegistry::listAll() const {
  return notes_;
}

bool NoteRegistry::parseWnoteFile(const std::string& file_path, NoteInstrument& out_inst, std::string& out_error) {
  std::ifstream in(file_path);
  if (!in) {
    out_error = "Could not open note file: " + file_path;
    return false;
  }

  std::string line;
  while (std::getline(in, line)) {
    line = trim(line);
    if (line.empty() || line[0] == '#') continue;
    size_t eq = line.find('=');
    if (eq == std::string::npos) continue;

    std::string key = lowerStr(trim(line.substr(0, eq)));
    std::string val = trim(line.substr(eq + 1));

    if (key == "name") out_inst.name = val;
    else if (key == "author") out_inst.author = val;
    else if (key == "waveform") out_inst.waveform = parseWaveform(val);
    else if (key == "attack_ms") out_inst.attack_ms = std::stod(val);
    else if (key == "decay_ms") out_inst.decay_ms = std::stod(val);
    else if (key == "sustain") out_inst.sustain = std::stod(val);
    else if (key == "release_ms") out_inst.release_ms = std::stod(val);
    else if (key == "description") out_inst.description = val;
    else if (key == "harmonics") {
      out_inst.harmonics.clear();
      std::stringstream ss(val);
      std::string item;
      while (std::getline(ss, item, ',')) {
        item = trim(item);
        if (!item.empty()) {
          out_inst.harmonics.push_back(std::stod(item));
        }
      }
    }
  }

  if (out_inst.name.empty()) {
    out_error = "Missing required 'name=' field in .wnote file.";
    return false;
  }
  if (out_inst.author.empty()) {
    out_inst.author = "Community";
  }
  if (out_inst.harmonics.empty()) {
    out_inst.harmonics = {1.0};
  }
  return true;
}

bool NoteRegistry::uploadNoteFile(const std::string& file_path, std::string& out_message) {
  NoteInstrument inst;
  std::string err;
  if (!parseWnoteFile(file_path, inst, err)) {
    out_message = err;
    return false;
  }

  std::error_code ec;
  std::string dir = localStoreDir();
  fs::create_directories(dir, ec);

  std::string dest = dir + std::string(1, WENTO_PATH_SEP) + inst.name + ".wnote";
  fs::copy_file(file_path, dest, fs::copy_options::overwrite_existing, ec);

  // Sync to remote Wento Community Registry when WENTO_REGISTRY_URL is configured
  const char* env_url = std::getenv("WENTO_REGISTRY_URL");
  std::string registry_url = env_url ? std::string(env_url) : std::string(WENTO_DEFAULT_REGISTRY_URL);

  std::ostringstream json;
  json << "{"
       << "\"name\":\"" << inst.name << "\","
       << "\"author\":\"" << inst.author << "\","
       << "\"waveform\":\"" << waveformToString(inst.waveform) << "\","
       << "\"attackMs\":" << inst.attack_ms << ","
       << "\"decayMs\":" << inst.decay_ms << ","
       << "\"sustain\":" << inst.sustain << ","
       << "\"releaseMs\":" << inst.release_ms << ","
       << "\"harmonics\":[";
  for (size_t i = 0; i < inst.harmonics.size(); ++i) {
    if (i > 0) json << ",";
    json << inst.harmonics[i];
  }
  json << "],\"description\":\"" << inst.description << "\"}";

  if (env_url != nullptr) {
    std::string cmd = "curl -fsS -X POST -H \"Content-Type: application/json\" -d '" + json.str() + "' \"" + registry_url + "\" >/dev/null 2>&1";
    int rc = std::system(cmd.c_str());
    if (rc == 0) {
      out_message = "Uploaded note '" + inst.name + "' to global Wento registry (" + registry_url + ") and installed locally.";
      return true;
    }
  }

  out_message = "Installed note '" + inst.name + "' to shared local registry (" + dest + "). Set WENTO_REGISTRY_URL to publish remotely.";
  return true;
}

} // namespace wento
