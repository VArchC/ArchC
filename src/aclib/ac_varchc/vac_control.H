/*  VArchC main control header
    Copyright (C) 2018  Isaías Felzmann
    Visit http://varchc.github.io/ for more information.

    This module is part of ArchC
    Copyright (C) 2002-2018  The ArchC Team

    Please see license file distributed with this file.
    
    http://www.archc.org/
*/

#ifndef _VAC_CONTROL_H_
#define _VAC_CONTROL_H_

#include "vac_stats.H"
#include "ac_decoder_rt.H"
#include <list>
#include <unordered_map>
#include <string>
#include <limits>

namespace VArchC {

  namespace Wrappers {
    class Instruction;
    class Data;
    class Energy;
  }

  template<class W, typename T>
  class Wrapper {
    public:
      typedef T (W::*model_t)(void* v);
      uint8_t approx_id;
      W* obj;
      model_t func;
      bool operator== (const Wrapper<W,T>& cmp) {return (cmp.approx_id == approx_id);}
      Wrapper(uint8_t _approx_id, W* _obj, model_t _func) : approx_id(_approx_id), obj(_obj), func(_func) {}
      Wrapper(uint8_t _approx_id) : approx_id(_approx_id) {}
  };

  template<class W, typename T>
  class ModelList {
    private:
      typedef T (W::*model_t)(void* v);
      std::list<Wrapper<W, T>> models;
      W* obj;
      model_t func;
    public:
      ModelList() : models() {}

      T operator() (void *v) {return (obj->*func)(v);}

      void activateModel(uint8_t approx_id, W* obj, model_t func);

      void deactivateModel(uint8_t approx_id);

      void clear();
  };

  class OPParam {
    public:
      uint8_t approx_id;
      std::unordered_map<std::string, double> params;
      bool operator== (const OPParam& cmp) {return (cmp.approx_id == approx_id);}
      OPParam(uint8_t _approx_id, const OPParam& previous, std::unordered_map<std::string, double> _params);
      OPParam(uint8_t _approx_id, std::unordered_map<std::string, double> _params) : approx_id(_approx_id), params(_params) {};
      OPParam(uint8_t _approx_id) : approx_id(_approx_id) {}
      double operator[] (const std::string& key);
  };

  class OPList {
    private:
      std::unordered_map<unsigned int, std::list<OPParam>> params;
      int* cur_instr_id;
    public:
      OPList() : cur_instr_id(NULL) {}
      OPList(int* _cur_instr_id) : params(), cur_instr_id(_cur_instr_id) {}
      double operator[] (const std::string& key);
      void activateOP(unsigned int instr, uint8_t approx_id, std::unordered_map<std::string, double> params);
      void deactivateOP(uint8_t approx_id);
      void clear();
  };

  class Control_base{
    protected:
      virtual void activateApprox(uint8_t approx_id) =0;
      virtual void initModels() =0;
      Stats vac_stats;
      std::unordered_map<uint8_t, std::string> approxes;
      std::string app;

    public:
      uint64_t active_approx;
      int *cur_instr_id;
      unsigned long long int *instr_counter;
      std::unordered_map<unsigned int,ModelList<Wrappers::Instruction, void>> IM;
      std::unordered_map<unsigned int,ModelList<Wrappers::Data, void>> DM_regrd;
      std::unordered_map<unsigned int,ModelList<Wrappers::Data, void>> DM_regwr;
      std::unordered_map<unsigned int,ModelList<Wrappers::Data, void>> DM_rbnrd;
      std::unordered_map<unsigned int,ModelList<Wrappers::Data, void>> DM_rbnwr;
      std::unordered_map<unsigned int,ModelList<Wrappers::Data, void>> DM_memrd;
      std::unordered_map<unsigned int,ModelList<Wrappers::Data, void>> DM_memwr;

      std::unordered_map<unsigned int,ModelList<Wrappers::Energy, void>> EM;

      OPList OP;

      Control_base(int *current_instruction, unsigned long long int *ac_instr_counter) : vac_stats(), active_approx(0), cur_instr_id(current_instruction), instr_counter(ac_instr_counter), OP(current_instruction) {};
      Control_base(unsigned long long int *ac_instr_counter) : vac_stats(), active_approx(0), instr_counter(ac_instr_counter), OP() {};
      void issue(uint8_t cmd);
      void instrDispatch(unsigned int instr, double energy);
      void saveStats(ac_dec_instr* instructions);
  };

}

#endif
