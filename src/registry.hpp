#ifndef WENTO_REGISTRY_HPP
#define WENTO_REGISTRY_HPP

#include "wento.hpp"
#include <optional>
#include <string>
#include <vector>

namespace wento {

class NoteRegistry {
public:
  NoteRegistry();

  std::optional<NoteInstrument> findNote(const std::string& name) const;
  std::vector<NoteInstrument> listAll() const;

  // Parse a .wnote file and publish it globally for all Wento users
  bool uploadNoteFile(const std::string& file_path, std::string& out_message);

  static bool parseWnoteFile(const std::string& file_path, NoteInstrument& out_inst, std::string& out_error);

private:
  std::vector<NoteInstrument> notes_;
  std::string localStoreDir() const;
  void loadBuiltins();
  void loadLocalNotes();
};

} // namespace wento

#endif // WENTO_REGISTRY_HPP
