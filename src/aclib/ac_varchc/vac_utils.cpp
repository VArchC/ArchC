/*  VArchC general utilities
    Copyright (C) 2018  Isaías Felzmann
    Visit http://varchc.github.io/ for more information.

    This module is part of ArchC
    Copyright (C) 2002-2018  The ArchC Team

    Please see license file distributed with this file.
    
    http://www.archc.org/
*/

#include <iostream>
#include "vac_utils.H"
#include "elfio/elfio.hpp"

void VArchC::Storage::setAsControl(char *exec, uint32_t& vac_ctl_addr, bool& vac_ctl) {
  ELFIO::elfio reader;

  if (!reader.load(exec)) {
    std::cerr << "VArchC Warning: Unable to open ELF file \"" << exec << "\" to extract VArchC_ctl object." << std::endl;
  }
  else {

    const ELFIO::symbol_section_accessor symbols(reader, reader.sections[".symtab"]);
    std::string name;
    ELFIO::Elf64_Addr value;
    ELFIO::Elf_Xword size;
    unsigned char bind;
    unsigned char type;
    ELFIO::Elf_Half section_index;
    unsigned char other;
    for (unsigned int i = 0; i < symbols.get_symbols_num(); i++) {
      symbols.get_symbol(i, name, value, size, bind, type, section_index, other);
      if (name == "VArchC_ctl") {
        if (size == 8) {
          vac_ctl_addr = value;
          vac_ctl = true;
        }
        else {
          std::cerr << "VArchC Warning: VArchC_ctl expected to be a 64-bit object, " << (size * 8) << "-bit object found." << std::endl;
        }
        return;
      }
    }

    std::cerr << "VArchC Warning: VArchC_ctl object not found on \"" << exec << "\". Unable to provide application control over approximation state." << std::endl;
  }
}
