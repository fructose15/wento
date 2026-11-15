#ifndef WENTO_INTERPRETER_HPP
#define WENTO_INTERPRETER_HPP

#include "registry.hpp"
#include "wento.hpp"
#include <string>

namespace wento {

class Interpreter {
public:
  explicit Interpreter(NoteRegistry& registry);

  double tempo() const { return tempo_bpm_; }

  // Parse duration expressions: "1/4", "2b", "500ms", "1.5s"
  double parseDurationMs(const std::string& token) const;

  bool executeLine(const std::string& raw_line, int line_number, std::string& out_error);
  bool runFile(const std::string& file_path);

private:
  NoteRegistry& registry_;
  double tempo_bpm_ = 120.0;
};

} // namespace wento

#endif // WENTO_INTERPRETER_HPP
