/*  VArchC statistics extractor
    Copyright (C) 2018  Isaías Felzmann
    Visit http://varchc.github.io/ for more information.

    This module is part of ArchC
    Copyright (C) 2002-2018  The ArchC Team

    Please see license file distributed with this file.
    
    http://www.archc.org/
*/

#include "vac_stats.H"
#include <iostream>
#include <fstream>

void VArchC::Stats::printInstrCounter(const char* fname, ac_dec_instr* instructions, const std::unordered_map<uint8_t, std::string>& approxes) {
  std::ofstream fp;
  fp.open(fname, std::ios::out | std::ios::trunc);
  std::cerr << "Writing VArchC instruction counters to " << fname << std::endl;

  if (fp.is_open()) {
    fp << "\"Instruction/Approximation\",\"No approximation\"";
    for (int i = 1; i <= section; i++) {
      fp << ",";
    }
    for (auto& x : icounter[0]) {
      uint64_t a = x.first;
      if (a != 0x0) {
        std::string as("");
        for (auto& y: approxes) {
          if ((a & (static_cast<uint64_t>(0x01) << y.first)) != 0x0) {
            as = as + " | " + y.second;
          }
        }
        as.erase(0, 3);
        fp << ", \"" << as << "\"";
        for (int i = 1; i <= section; i++) {
          fp << ", ";
        }
      }
    }
    fp << std::endl;

    for (auto& y : icounter[0]) {
      for (int i = 0; i <= section; i++) {
        fp << ",\"Section " << i << "\"";
      }
    }
    fp << std::endl;

    fp << "\"-All instructions-\"";
    for (int i = 0; i <= section; i++) {
      fp << ", " << icounter[0][0x0][i];
    }
    for (auto& y : icounter[0]) {
      if (y.first != 0x0) {
        for (int i = 0; i <= section; i++) {
          fp << ", " << icounter[0][y.first][i];
        }
      }
    }
    fp << std::endl;

    for (auto&x : icounter) {
      if (x.first != 0) {
        fp <<"\"" << ac_dec_instr::GetInstrByID(instructions, x.first)->mnemonic << "\"";
        for (int i = 0; i <= section; i++) {
          fp << ", " << icounter[x.first][0x0][i];
        }
        for (auto& y : icounter[0]) {
          if (y.first != 0x0) {
            for (int i = 0; i <= section; i++) {
              fp << ", " << x.second[y.first][i];
            }
          }
        }
        fp << std::endl;
      }
    }

    fp.close();
  }
  else {
    std::cerr << "VArchC Error: Unable to open file \"" << fname << "\" to save instruction counters." << std::endl;
  }
}

void VArchC::Stats::printEnergyCounter(const char* fname, ac_dec_instr* instructions, const std::unordered_map<uint8_t, std::string>& approxes) {
  std::ofstream fp;
  fp.open(fname, std::ios::out | std::ios::trunc);
  std::cerr << "Writing VArchC energy counters to " << fname << std::endl;

  if (fp.is_open()) {
    fp << "\"Instruction/Approximation\",\"No approximation\"";
    for (int i = 1; i <= section; i++) {
      fp << ",";
    }
    for (auto& x : ecounter[0]) {
      uint64_t a = x.first;
      if (a != 0x0) {
        std::string as("");
        for (auto& y: approxes) {
          if ((a & (static_cast<uint64_t>(0x01) << y.first)) != 0x0) {
            as = as + " | " + y.second;
          }
        }
        as.erase(0, 3);
        fp << ", \"" << as << "\"";
        for (int i = 1; i <= section; i++) {
          fp << ", ";
        }
      }
    }
    fp << std::endl;

    for (auto& y : ecounter[0]) {
      for (int i = 0; i <= section; i++) {
        fp << ",\"Section " << i << "\"";
      }
    }
    fp << std::endl;

    fp << "\"-All instructions-\"";
    for (int i = 0; i <= section; i++) {
      fp << ", " << ecounter[0][0x0][i];
    }
    for (auto& y : ecounter[0]) {
      if (y.first != 0x0) {
        for (int i = 0; i <= section; i++) {
          fp << ", " << ecounter[0][y.first][i];
        }
      }
    }
    fp << std::endl;

    for (auto&x : ecounter) {
      if (x.first != 0) {
        fp <<"\"" << ac_dec_instr::GetInstrByID(instructions, x.first)->mnemonic << "\"";
        for (int i = 0; i <= section; i++) {
          fp << ", " << ecounter[x.first][0x0][i];
        }
        for (auto& y : ecounter[0]) {
          if (y.first != 0x0) {
            for (int i = 0; i <= section; i++) {
              fp << ", " << x.second[y.first][i];
            }
          }
        }
        fp << std::endl;
      }
    }

    fp.close();
  }
  else {
    std::cerr << "VArchC Error: Unable to open file \"" << fname << "\" to save energy counters." << std::endl;
  }
}
