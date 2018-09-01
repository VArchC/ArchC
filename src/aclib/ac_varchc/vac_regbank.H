/*  VArchC register bank proxy
    Copyright (C) 2018  Isaías Felzmann
    Visit http://varchc.github.io/ for more information.

    This module is part of ArchC
    Copyright (C) 2002-2018  The ArchC Team

    Please see license file distributed with this file.
    
    http://www.archc.org/
*/

#ifndef _VAC_REGBANK_H_
#define _VAC_REGBANK_H_

#include "ac_regbank.H"
#include "vac_storage.H"
#include "vac_control.H"
#include <string>

namespace VArchC {
  namespace Storage {

    template<int nregs, class ac_word, class ac_Dword>
    class Regbank {
      friend class Word<ac_word>;
      private:
        ac_regbank<nregs, ac_word, ac_Dword> proxy;
        Word<ac_word> data[nregs];
        Control_base& vac_control;

        void init_words() {
          for (int i = 0; i < nregs; i++) {
            data[i].set(&(proxy.Data[i]), &(bypass[i]), source_t::REGBANK, &(proxy.Name), i, &(vac_control.DM_rbnrd), &(vac_control.DM_rbnwr), vac_control.cur_instr_id);
          }
        }
      public:
        ac_word bypass[nregs];

        /// Default Constructor
        Regbank(string nm, Control_base& control): proxy(nm), vac_control(control) {init_words();}

#ifdef AC_DELAY
        /// Constructor with delay parameter.
        Regbank(string nm, double& ts, Control_base& control) : proxy(nm, ts), vac_control(control) {init_words();}
#endif 

        //!Reading the content of an address. Overloaded Method.
        ac_word read( unsigned address ){
          uint64_t d = static_cast<uint64_t>(proxy.read(address));
          Encoder e(source_t::REGBANK, proxy.Name, address, source_t::READ, sizeof(ac_word), d);
          vac_control.DM_rbnrd[*(vac_control.cur_instr_id)](&e);
          return static_cast<ac_word>(d);
        }

        //!Reading the content of an address. Overloaded Method.
        ac_Dword read_double( unsigned address ){
          uint64_t d = static_cast<uint64_t>(proxy.read_double(address));
          Encoder e(source_t::REGBANK, proxy.Name, address, source_t::READ, sizeof(ac_Dword), d);
          vac_control.DM_rbnrd[*(vac_control.cur_instr_id)](&e);
          return static_cast<ac_Dword>(d);
        }

        void write_double( unsigned address , ac_Dword datum  ){
          *((ac_Dword *)(bypass+((address)*sizeof(ac_Dword)))) = datum;
          uint64_t d = static_cast<uint64_t>(datum);
          Encoder e(source_t::REGBANK, proxy.Name, address, source_t::WRITE, sizeof(ac_Dword), d);
          vac_control.DM_rbnwr[*(vac_control.cur_instr_id)](&e);
          proxy.write_double(address, static_cast<ac_Dword>(d));
        }

        //!Writing to a register. Overloaded Method.
        void write( unsigned address , ac_word datum  ){
          bypass[address] = datum;
          uint64_t d = static_cast<uint64_t>(datum);
          Encoder e(source_t::REGBANK, proxy.Name, address, source_t::WRITE, sizeof(ac_word), d);
          vac_control.DM_rbnwr[*(vac_control.cur_instr_id)](&e);
          proxy.write(address, static_cast<ac_word>(d));
        }


        //!Dump the entire contents of a regbank device
        void dump(){
          proxy.dump();
        }

        //!Method to load device content from a file.
        void load( char* file ){
          proxy.load(file);
        }

#ifdef AC_DELAY
        //!Writing to a register. Overloaded Method.
        void write(unsigned address , ac_word datum, unsigned time) {
          bypass[address] = datum;
          uint64_t d = static_cast<uint64_t>(datum);
          Encoder e(source_t::REGBANK, proxy.Name, address, source_t::WRITE, sizeof(ac_word), d);
          vac_control.DM_rbnwr[*(vac_control.cur_instr_id)](&e);
          proxy.write(address, static_cast<ac_word>(d), time);
        }


        //!Method to commit delayed updates.
        void commit_delays( double time ){
          proxy.commit_delays(time);
        }
#endif

        /**
         * Individual register access operator.
         * @param reg Index of the register in the bank.
         * @return Reference to register data element.
         */
        Word<ac_word>& operator [] (int reg) {
          return data[reg];
        }
    };

  }
}

#endif //_VAC_REGBANK_H_
