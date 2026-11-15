#include "interpreter.hpp"
#include "synth.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

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

std::vector<std::string> tokenize(const std::string& line) {
  std::vector<std::string> tokens;
  std::istringstream iss(line);
  std::string tok;
  while (iss >> tok) {
    tokens.push_back(tok);
  }
  return tokens;
}
} // namespace

Interpreter::Interpreter(NoteRegistry& registry) : registry_(registry) {}

double Interpreter::parseDurationMs(const std::string& token) const {
  std::string s = lowerStr(trim(token));
  if (s.empty()) return -1.0;

  double beat_ms = 60000.0 / tempo_bpm_;

  try {
    if (s.size() > 2 && s.substr(s.size() - 2) == "ms") {
      return std::stod(s.substr(0, s.size() - 2));
    }
    if (s.size() > 1 && s.back() == 's') {
      return std::stod(s.substr(0, s.size() - 1)) * 1000.0;
    }
    if (s.size() > 1 && s.back() == 'b') {
      return std::stod(s.substr(0, s.size() - 1)) * beat_ms;
    }
    size_t slash = s.find('/');
    if (slash != std::string::npos) {
      double num = std::stod(s.substr(0, slash));
      double den = std::stod(s.substr(slash + 1));
      if (den <= 0.0) return -1.0;
      double quarter_beats = (num / den) * 4.0;
      return quarter_beats * beat_ms;
    }
    // Bare number defaults to milliseconds
    return std::stod(s);
  } catch (...) {
    return -1.0;
  }
}

bool Interpreter::executeLine(const std::string& raw_line, int line_number, std::string& out_error) {
  std::string line = trim(raw_line);
  if (line.empty() || line[0] == '#') {
    return true;
  }

  // Normalize "set tempo=120" -> "set tempo = 120"
  std::string normalized;
  for (char c : line) {
    if (c == '=') {
      normalized += " = ";
    } else {
      normalized += c;
    }
  }

  auto tokens = tokenize(normalized);
  if (tokens.empty()) return true;

  std::string kw = lowerStr(tokens[0]);

  if (kw == "set") {
    if (tokens.size() >= 4 && lowerStr(tokens[1]) == "tempo" && tokens[2] == "=") {
      try {
        double bpm = std::stod(tokens[3]);
        if (bpm < 20.0 || bpm > 400.0) {
          out_error = "Line " + std::to_string(line_number) + ": tempo must be between 20 and 400 BPM.";
          return false;
        }
        tempo_bpm_ = bpm;
        std::cout << "[tempo] " << tempo_bpm_ << " BPM\n";
        return true;
      } catch (...) {
        out_error = "Line " + std::to_string(line_number) + ": invalid BPM value '" + tokens[3] + "'.";
        return false;
      }
    }
    out_error = "Line " + std::to_string(line_number) + ": expected 'set tempo = <bpm>'.";
    return false;
  }

  if (kw == "play") {
    if (tokens.size() < 2) {
      out_error = "Line " + std::to_string(line_number) + ": expected 'play note ...' or 'play audio <https_url>'.";
      return false;
    }
    std::string sub = lowerStr(tokens[1]);
    if (sub == "note") {
      if (tokens.size() < 5) {
        out_error = "Line " + std::to_string(line_number) + ": expected 'play note <note_name> <pitch> <duration>'.";
        return false;
      }
      const std::string& note_name = tokens[2];
      const std::string& pitch_str = tokens[3];
      const std::string& dur_str = tokens[4];

      auto inst = registry_.findNote(note_name);
      if (!inst.has_value()) {
        out_error = "Line " + std::to_string(line_number) + ": unknown note instrument '" + note_name + "'.";
        return false;
      }

      double freq_hz = AudioSynth::parsePitchHz(pitch_str);
      if (freq_hz < 0.0) {
        out_error = "Line " + std::to_string(line_number) + ": invalid pitch '" + pitch_str + "'.";
        return false;
      }

      double dur_ms = parseDurationMs(dur_str);
      if (dur_ms <= 0.0) {
        out_error = "Line " + std::to_string(line_number) + ": invalid duration '" + dur_str + "'.";
        return false;
      }

      std::cout << "[note] " << inst->name << " " << pitch_str << " (" << freq_hz << " Hz) for " << dur_ms << " ms\n";
      AudioSynth::playSynthesizedNote(*inst, freq_hz, dur_ms);
      return true;
    }

    if (sub == "audio") {
      if (tokens.size() < 3) {
        out_error = "Line " + std::to_string(line_number) + ": expected 'play audio <https_url>'.";
        return false;
      }
      const std::string& url = tokens[2];
      if (url.rfind("https://", 0) != 0) {
        out_error = "Line " + std::to_string(line_number) + ": play audio requires an https:// URL.";
        return false;
      }
      std::cout << "[audio] streaming " << url << "\n";
      return AudioSynth::playHttpsAudio(url);
    }

    out_error = "Line " + std::to_string(line_number) + ": unknown play target '" + tokens[1] + "'.";
    return false;
  }

  if (kw == "wait") {
    if (tokens.size() < 2) {
      out_error = "Line " + std::to_string(line_number) + ": expected 'wait <duration>'.";
      return false;
    }
    double dur_ms = parseDurationMs(tokens[1]);
    if (dur_ms < 0.0) {
      out_error = "Line " + std::to_string(line_number) + ": invalid wait duration '" + tokens[1] + "'.";
      return false;
    }
    std::cout << "[wait] " << dur_ms << " ms\n";
    AudioSynth::sleepMs(dur_ms);
    return true;
  }

  out_error = "Line " + std::to_string(line_number) + ": unknown statement '" + tokens[0] + "'.";
  return false;
}

bool Interpreter::runFile(const std::string& file_path) {
  std::ifstream in(file_path);
  if (!in) {
    std::cerr << "[wento error] Unable to open script file: " << file_path << "\n";
    return false;
  }

  std::string line;
  int line_num = 0;
  while (std::getline(in, line)) {
    ++line_num;
    std::string err;
    if (!executeLine(line, line_num, err)) {
      std::cerr << "[wento error] " << err << "\n";
      return false;
    }
  }
  return true;
}

} // namespace wento
