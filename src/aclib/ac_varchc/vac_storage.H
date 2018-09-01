/*  VArchC storage models
    Copyright (C) 2018  Isaías Felzmann
    Visit http://varchc.github.io/ for more information.

    This module is part of ArchC
    Copyright (C) 2002-2018  The ArchC Team

    Please see license file distributed with this file.
    
    http://www.archc.org/
*/

#ifndef _VAC_STORAGE_H_
#define _VAC_STORAGE_H_

#include <string>
#include "vac_control.H"

namespace VArchC {
  namespace Storage {
    
    struct source_t {
      typedef enum {MEM, REGBANK, REG} type_t;
      typedef enum {READ, WRITE} op_t;
      source_t(type_t _type, const std::string& _name, uint32_t _address, op_t _op, size_t _width) : type(_type), name(_name), address(_address), op(_op), width(_width) {};
      type_t type;
      const std::string& name;
      uint32_t address;
      op_t op;
      size_t width;
    };
    
    struct Encoder {
      Encoder(source_t::type_t type, const std::string& name, uint32_t address, source_t::op_t op, size_t width, uint64_t& _data) : source(type, name, address, op, width), data(_data) {};
      source_t source;
      uint64_t& data;
    };

    template <typename T>
    class Word {
      private:
        typedef std::unordered_map<unsigned int,ModelList<Wrappers::Data, void>> DM_t;
        T* value;
        T* bypass;
        T faulty;
        source_t::type_t type;
        std::string* name;
        uint32_t address;
        DM_t* DM_rd;
        DM_t* DM_wr;
        int* cur_instr_id;
       
        Word(const Word&) = delete;
      public:
        Word() {};
        Word(T* _value, T* _bypass, source_t::type_t _type, std::string* _name, uint32_t _address, DM_t* _DM_rd, DM_t* _DM_wr, int* _cur_instr_id) : value(_value), bypass(_bypass), type(_type), name(_name), address(_address), DM_rd(_DM_rd), DM_wr(_DM_wr), cur_instr_id(_cur_instr_id) {};

        void set(T* _value, T* _bypass, source_t::type_t _type, std::string* _name, uint32_t _address, DM_t* _DM_rd, DM_t* _DM_wr, int* _cur_instr_id) {
          value = _value;
          bypass = _bypass;
          type = _type;
          name = _name;
          address = _address;
          DM_rd = _DM_rd;
          DM_wr = _DM_wr;
          cur_instr_id = _cur_instr_id;
        }

        Word& operator = (Word& _value) {
          T __value = _value;
          *bypass = __value;
          uint64_t d = static_cast<uint64_t>(__value);
          Encoder e(type, *name, address, source_t::WRITE, sizeof(T), d);
          ((*DM_wr)[*cur_instr_id])(&e);
          *value = static_cast<T>(d);
          return *this;
        }

        Word& operator = (const T& _value) {
          *bypass = _value;
          uint64_t d = static_cast<uint64_t>(_value);
          Encoder e(type, *name, address, source_t::WRITE, sizeof(T), d);
          ((*DM_wr)[*cur_instr_id])(&e);
          *value = static_cast<T>(d);
          return *this;
        }

        operator const T&() {
          uint64_t d = static_cast<uint64_t>(*value);
          Encoder e(type, *name, address, source_t::READ, sizeof(T), d);
          ((*DM_rd)[*cur_instr_id])(&e);
          faulty = static_cast<T>(d);
          return faulty;
        }
    };
  }
}

typedef VArchC::Storage::source_t source_t;

#endif
