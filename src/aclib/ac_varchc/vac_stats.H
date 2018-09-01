/*  VArchC statistics extractor
    Copyright (C) 2018  Isaías Felzmann
    Visit http://varchc.github.io/ for more information.

    This module is part of ArchC
    Copyright (C) 2002-2018  The ArchC Team

    Please see license file distributed with this file.
    
    http://www.archc.org/
*/

#ifndef _VAC_STATS_H_
#define _VAC_STATS_H_

#include "ac_decoder_rt.H"
#include <unordered_map>
#include <string>

namespace VArchC {

  class Stats {
    private:
      class ICounter_t {
        private:
          uint64_t n;
        public:
          ICounter_t() : n(0) {};
          operator const uint64_t&() {
            return n;
          }
          const uint64_t& operator ++ () {
            return ++n;
          }
      };

      class ECounter_t {
        private:
          double e;
        public:
          ECounter_t() : e(0.0) {};
          operator const double&() {
            return e;
          }
          const double& operator += (const double& v) {
            return (e+=v);
          }
      };

      std::unordered_map<unsigned int, std::unordered_map<uint64_t, std::unordered_map<uint8_t, ICounter_t>>> icounter;
      std::unordered_map<unsigned int, std::unordered_map<uint64_t, std::unordered_map<uint8_t, ECounter_t>>> ecounter;

    public:
      uint8_t section;
      Stats() : section(0) {};
      void incrInstrCounter(uint64_t approx, unsigned int instr) {
        ++(icounter[instr][approx][section]);
        ++(icounter[0][approx][section]);
      }
      void incrEnergyCounter(uint64_t approx, unsigned int instr, double energy) {
        (ecounter[instr][approx][section]) += energy;
        (ecounter[0][approx][section]) += energy;
      }

      void printInstrCounter(const char* fname, ac_dec_instr* instructions, const std::unordered_map<uint8_t, std::string>& approxes);
      void printEnergyCounter(const char* fname, ac_dec_instr* instructions, const std::unordered_map<uint8_t, std::string>& approxes);
  };
}

#endif
