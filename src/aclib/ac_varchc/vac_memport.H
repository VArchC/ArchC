/*  VArchC memory proxy
    Copyright (C) 2018  Isaías Felzmann
    Visit http://varchc.github.io/ for more information.

    This module is part of ArchC
    Copyright (C) 2002-2018  The ArchC Team

    Please see license file distributed with this file.
    
    http://www.archc.org/
*/

#ifndef _VAC_MEMPORT_H_
#define _VAC_MEMPORT_H_

#include "ac_memport.H"
#include "vac_storage.H"
#include "vac_control.H"
#include "vac_utils.H"

namespace VArchC {
  namespace Storage {

    template<typename ac_word, typename ac_Hword> 
    class Memport {

    private:
      ac_memport<ac_word, ac_Hword> proxy;
      Control_base& vac_control;
      uint32_t vac_ctl_addr;
      bool vac_ctl;

      template <typename T> void vac_write_intersection(uint32_t addr, T data) {
        if (vac_ctl && addr <= (vac_ctl_addr + 7) && (addr + sizeof(T)) > vac_ctl_addr) {
          if ((addr + sizeof(T)) > (vac_ctl_addr + 7)) {
            if (!proxy.ac_mt_endian) {
              data = proxy.byte_swap(data);
            }
            uint8_t *d = reinterpret_cast<uint8_t*>(&data);
            d += (vac_ctl_addr + 7 - addr);
            uint8_t cmd = *(reinterpret_cast<uint8_t*>(d));
            vac_control.issue(cmd);
          }
          uint64_t ap = vac_control.active_approx;
          uint8_t* bap = reinterpret_cast<uint8_t*>(&ap);
          if (!proxy.ac_mt_endian) {
            for(uint32_t i = vac_ctl_addr + 7; i >= vac_ctl_addr; i--) {
              proxy.write_byte(i, *bap);
              bap++;
            }
          }
          else {
            for (uint32_t i = vac_ctl_addr; i <= vac_ctl_addr +7; i++) {
              proxy.write_byte(i, *bap);
              bap++;
            }
          }
        }
      }

    public:

      ///Default constructor
      explicit Memport(ac_arch<ac_word, ac_Hword>& ref, Control_base& control) : proxy(ref), vac_control(control), vac_ctl(false) {}

      explicit Memport(ac_arch<ac_word, ac_Hword>& ref, ac_inout_if& stg, Control_base& control) : proxy(ref, stg), vac_control(control), vac_ctl(false) {}

      ac_memport<ac_word, ac_Hword>* get_proxy() {
        return &proxy;
      }

      void initializeBuffer () {
        proxy.initializeBuffer();
      }

      void setBlockSize (int cacheBlockSize) {
        proxy.setBlockSize(cacheBlockSize);
      }
      //////////////////////////////////////////////////////////////////////////////////////////

      sc_core::sc_time getTimeInfo() {
        return proxy.getTimeInfo();
      }

      void setTimeInfo(sc_core::sc_time time) { 
        proxy.setTimeInfo(time);
      }

      void setProcId(unsigned int id) {  
        proxy.setProcId(id);  
      }


      uint32_t byte_to_word(uint32_t a) {
        return a/sizeof(ac_word);
      }

      //Reads a word
      inline ac_word read(uint32_t address) {
        uint64_t data = static_cast<uint64_t>(proxy.read(address));
        Encoder e(source_t::MEM, (proxy.storage)->get_name(), address, source_t::READ, sizeof(ac_word), data);
        vac_control.DM_memrd[*(vac_control.cur_instr_id)](&e);
        return static_cast<ac_word>(data);
      }

      //Reads a byte
      inline uint8_t read_byte(uint32_t address) {
        uint64_t data = static_cast<uint64_t>(proxy.read_byte(address));
        Encoder e(source_t::MEM, (proxy.storage)->get_name(), address, source_t::READ, sizeof(uint8_t), data);
        vac_control.DM_memrd[*(vac_control.cur_instr_id)](&e);
        return static_cast<uint8_t>(data);
      }

      //Reads half word
      inline ac_Hword read_half(uint32_t address) {
        uint64_t data = static_cast<uint64_t>(proxy.read_half(address));
        Encoder e(source_t::MEM, (proxy.storage)->get_name(), address, source_t::READ, sizeof(ac_Hword), data);
        vac_control.DM_memrd[*(vac_control.cur_instr_id)](&e);
        return static_cast<ac_Hword>(data);
      }

      const ac_word *read_block(uint32_t address, unsigned l) {
        return proxy.read_block(address, l); 
      }

      //!Writing a word
      inline void write(uint32_t address, ac_word datum) {
        uint64_t data = static_cast<uint64_t>(datum);
        Encoder e(source_t::MEM, (proxy.storage)->get_name(), address, source_t::WRITE, sizeof(ac_word), data);
        vac_control.DM_memwr[*(vac_control.cur_instr_id)](&e);
        datum = static_cast<ac_word>(data);
        proxy.write(address, datum);
        vac_write_intersection<ac_word>(address, datum);
      }

     //!Writing a byte
      inline void write_byte(uint32_t address, uint8_t datum) {
        uint64_t data = static_cast<uint64_t>(datum);
        Encoder e(source_t::MEM, (proxy.storage)->get_name(), address, source_t::WRITE, sizeof(uint8_t), data);
        vac_control.DM_memwr[*(vac_control.cur_instr_id)](&e);
        datum = static_cast<uint8_t>(data);
        proxy.write_byte(address, datum);
        vac_write_intersection<uint8_t>(address, datum);
      }

      //!Writing a short int
      inline void write_half(uint32_t address, ac_Hword datum) {
        uint64_t data = static_cast<uint64_t>(datum);
        Encoder e(source_t::MEM, (proxy.storage)->get_name(), address, source_t::WRITE, sizeof(ac_Hword), data);
        vac_control.DM_memwr[*(vac_control.cur_instr_id)](&e);
        datum = static_cast<ac_Hword>(data);
        proxy.write_half(address, datum);
        vac_write_intersection<ac_Hword>(address, datum);
      }

      void write_block(uint32_t address, const ac_word *d, unsigned length) {
        proxy.write_block(address, d, length);
      }

#ifdef AC_DELAY
      //!Writing a word
      inline void write(uint32_t address, ac_word datum, uint32_t time) {
        proxy.write(address, datum, time);
      }

      //!Writing a byte 
      inline void write_byte(uint32_t address, uint8_t datum, uint32_t time) {
        proxy.write_byte(address, datum, time);
      }

      //!Writing a short int 
      inline void write_half(uint32_t address, ac_Hword datum, uint32_t time) {
        write_half(address, datum, time);
      }
#endif

      inline void lock() {
        proxy.lock();
      } 
      
      inline void unlock() {
        proxy.unlock();
      }

#ifdef AC_UPDATE_LOG
      //! Reset log lists.
      void reset_log() {
        proxy.reset_log();
      }
      
      //!Dump storage device log.
      int change_dump(ostream& output) {
        return proxy.change_dump(output);
      }
      
      //!Save storage device log.
      void change_save() {
        proxy.change_save();
      }
#endif

      //!Method to provide the name of the device.
      inline const char* get_name() {
        return proxy.get_name();
      }

      //!Method to provide the size of the device.
      uint32_t get_size() {
        return proxy.get_size();
      }

#ifdef AC_UPDATE_LOG
      //!Method to provide the change list.
      log_list* get_changes() {
        return proxy.get_changes();
      }
#endif

      /// Method to load device content from a file.
      /// Use local Data array
      inline void load(char* file) {
        proxy.load(file);
      }

      //!Method to load device content from an array.
      void load_array(uint8_t* d, const uint32_t s) {
        proxy.load_array(d, s);
      }


#ifdef AC_DELAY

      //!Committing delayed updates
      void commit_delays(double time) {
        proxy.commit_delays(time);
      }

#endif

      //!Dump the entire contents of a storage device
      void dump() {
        proxy.dump();
      }

      ///Binding operator
      inline void operator ()(ac_inout_if& stg) {
        proxy.storage = &stg;
      }

      void setAsControl(char *exec) {
        VArchC::Storage::setAsControl(exec, vac_ctl_addr, vac_ctl);
      }
    };

  }
}



#endif
