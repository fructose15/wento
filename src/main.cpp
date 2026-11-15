#include "wento.hpp"
#include "interpreter.hpp"
#include "registry.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

bool printDocFile(const std::string& filename, const char* fallback_text) {
  const std::string candidates[] = {
    filename,
    "/usr/local/share/wento/" + filename,
    "/usr/share/wento/" + filename
  };

  for (const auto& path : candidates) {
    std::ifstream in(path);
    if (in) {
      std::ostringstream ss;
      ss << in.rdbuf();
      std::cout << ss.str() << "\n";
      return true;
    }
  }
  std::cout << fallback_text << "\n";
  return true;
}

void printHelp() {
  std::cout
    << "Wento " << WENTO_VERSION << " (" << WENTO_PLATFORM_NAME << ") — Music Programming Language by " << WENTO_CREATOR << "\n\n"
    << "Usage:\n"
    << "  wento readme                   View project information and quickstart guide\n"
    << "  wento license                  View the MIT License\n"
    << "  wento manual                   Display the Wento language manual\n"
    << "  wento agents                   View AGENTS.md contributor guide\n"
    << "  wento run <file>               Run a .wento music script file\n"
    << "  wento command <statement>      Execute a single inline Wento statement\n"
    << "  wento upload-note <note_file>  Publish a .wnote instrument for all Wento users\n";
}

const char* kFallbackReadme =
  "Wento 1.0.0 — Created by Fructose\n"
  "A terminal-first music programming language written in C++17.\n"
  "Core commands: set tempo = <bpm>, play note <name> <pitch> <duration>, play audio <https_url>, wait <duration>.\n";

const char* kFallbackLicense =
  "MIT License\n\n"
  "Copyright (c) 2026 Fructose\n\n"
  "Permission is hereby granted, free of charge, to any person obtaining a copy\n"
  "of this software and associated documentation files (the \"Software\"), to deal\n"
  "in the Software without restriction, including without limitation the rights\n"
  "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
  "copies of the Software.\n";

const char* kFallbackManual =
  "Wento Language Manual (Created by Fructose)\n"
  "1. set tempo = <bpm>                       Set BPM (20..400)\n"
  "2. play note <name> <pitch> <duration>     Synthesize note (e.g., play note Piano C4 1/4)\n"
  "3. play audio <https_url>                  Stream HTTPS audio file\n"
  "4. wait <duration>                         Pause (e.g., wait 500ms, wait 1/2)\n";

const char* kFallbackAgents =
  "AGENTS.md — Wento Architecture by Fructose\n"
  "Modules: src/main.cpp, src/interpreter.cpp, src/synth.cpp, src/registry.cpp.\n"
  "Unix builds use ./configure -> Makefile + config.unix.h. Windows builds use config.h.\n";

} // namespace

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printHelp();
    return 0;
  }

  std::string cmd = argv[1];

  if (cmd == "--help" || cmd == "-h" || cmd == "help") {
    printHelp();
    return 0;
  }
  if (cmd == "--version" || cmd == "-v") {
    std::cout << "wento " << WENTO_VERSION << " (created by " << WENTO_CREATOR << ")\n";
    return 0;
  }
  if (cmd == "readme") {
    printDocFile("README.md", kFallbackReadme);
    return 0;
  }
  if (cmd == "license") {
    printDocFile("LICENSE", kFallbackLicense);
    return 0;
  }
  if (cmd == "manual") {
    printDocFile("MANUAL.md", kFallbackManual);
    return 0;
  }
  if (cmd == "agents") {
    printDocFile("AGENTS.md", kFallbackAgents);
    return 0;
  }

  wento::NoteRegistry registry;

  if (cmd == "run") {
    if (argc < 3) {
      std::cerr << "Usage: wento run <file.wento>\n";
      return 1;
    }
    wento::Interpreter interp(registry);
    return interp.runFile(argv[2]) ? 0 : 1;
  }

  if (cmd == "command") {
    if (argc < 3) {
      std::cerr << "Usage: wento command \"<statement>\"\n";
      return 1;
    }
    std::ostringstream stmt;
    for (int i = 2; i < argc; ++i) {
      if (i > 2) stmt << " ";
      stmt << argv[i];
    }
    wento::Interpreter interp(registry);
    std::string err;
    if (!interp.executeLine(stmt.str(), 1, err)) {
      std::cerr << "[wento error] " << err << "\n";
      return 1;
    }
    return 0;
  }

  if (cmd == "upload-note") {
    if (argc < 3) {
      std::cerr << "Usage: wento upload-note <file.wnote>\n";
      return 1;
    }
    std::string msg;
    if (!registry.uploadNoteFile(argv[2], msg)) {
      std::cerr << "[wento error] " << msg << "\n";
      return 1;
    }
    std::cout << msg << "\n";
    return 0;
  }

  std::cerr << "Unknown command '" << cmd << "'. Run 'wento --help' for usage.\n";
  return 1;
}
