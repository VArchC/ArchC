/*  VArchC Main Control
    Copyright (C) 2018  Isaías Felzmann
    Visit http://varchc.github.io/ for more information.

    This module is part of ArchC
    Copyright (C) 2002-2018  The ArchC Team

    Please see license file distributed with this file.
    
    http://www.archc.org/
*/

#include "vac_control.H"
#include <iostream>
#include "unistd.h"

template class VArchC::ModelList<VArchC::Wrappers::Instruction, void>;
template class VArchC::ModelList<VArchC::Wrappers::Data, void>;
template class VArchC::ModelList<VArchC::Wrappers::Energy, void>;

template<class W, typename T>
void VArchC::ModelList<W, T>::activateModel(uint8_t approx_id, W* obj, model_t func) {
  this->models.emplace_front(approx_id, obj, func);
  this->obj = (models.front()).obj;
  this->func = (models.front()).func;
}

template<class W, typename T>
void VArchC::ModelList<W, T>::deactivateModel(uint8_t approx_id) {
  this->models.remove(Wrapper<W, T>(approx_id));
  this->obj = (models.front()).obj;
  this->func = (models.front()).func;
}

template<class W, typename T>
void VArchC::ModelList<W, T>::clear() {
  this->models.remove_if([](const Wrapper<W,T>& cmp){return (cmp.approx_id != 0xff);});
  this->obj = (models.front()).obj;
  this->func = (models.front()).func;
}

VArchC::OPParam::OPParam(uint8_t _approx_id, const OPParam& previous, std::unordered_map<std::string, double> _params) : approx_id(_approx_id) {
  for (auto& x: previous.params) {
    params[x.first] = x.second;
  }
  for (auto& x: _params) {
    params[x.first] = x.second;
  }
}
double VArchC::OPParam::operator[] (const std::string& key) {
  if (params.find(key) == params.end()) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  else {
    return params[key];
  }
}

double VArchC::OPList::operator[] (const std::string& key) {
  if (cur_instr_id == NULL || params.find(*cur_instr_id) == params.end()) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  else {
    return params[*cur_instr_id].front()[key];
  }
}

void VArchC::OPList::activateOP(unsigned int instr, uint8_t approx_id, std::unordered_map<std::string, double> params) {
  if (this->params[instr].empty()) {
    this->params[instr].emplace_front(approx_id, params);
  }
  else {
    this->params[instr].emplace_front(approx_id, this->params[instr].front(), params);
  }
}

void VArchC::OPList::deactivateOP(uint8_t approx_id) {
  for (auto& x: this->params) {
    x.second.remove(OPParam(approx_id));
  }
}

void VArchC::OPList::clear() {
  for (auto& x: this->params) {
    x.second.remove_if([](const OPParam& cmp){return (cmp.approx_id != 0xff);});
  }
}

void VArchC::Control_base::issue(uint8_t cmd) {
  if (cmd < 64) {
      for (auto& x: IM) {
        x.second.deactivateModel(cmd);
      }
      for (auto& x: DM_regrd) {
        x.second.deactivateModel(cmd);
      }
      for (auto& x: DM_regwr) {
        x.second.deactivateModel(cmd);
      }
      for (auto& x: DM_rbnrd) {
        x.second.deactivateModel(cmd);
      }
      for (auto& x: DM_rbnwr) {
        x.second.deactivateModel(cmd);
      }
      for (auto& x: DM_memrd) {
        x.second.deactivateModel(cmd);
      }
      for (auto& x: DM_memwr) {
        x.second.deactivateModel(cmd);
      }
      for (auto& x: EM) {
        x.second.deactivateModel(cmd);
      }
      OP.deactivateOP(cmd);
    this->active_approx &= ~(static_cast<uint64_t>(0x1) << cmd);
  }
  else if (cmd < 128) {
    this->activateApprox( cmd & 0x3f );
  }
  else if (cmd == 0xff) {
    IM.clear();
    DM_regrd.clear();
    DM_regwr.clear();
    DM_rbnrd.clear();
    DM_rbnwr.clear();
    DM_memrd.clear();
    DM_memwr.clear();
    EM.clear();
    this->active_approx = 0;
    this->initModels();
  }
  else if (cmd == 0xfe) {
    this->vac_stats.section++;
  }
} // issue()

void VArchC::Control_base::instrDispatch(unsigned int instr, double energy) {
  vac_stats.incrInstrCounter(active_approx, instr);
  vac_stats.incrEnergyCounter(active_approx, instr, energy);
} // instrDispatch()

void VArchC::Control_base::saveStats(ac_dec_instr* instructions) {
  std::string fname;
  fname = app + "_varchc_counters_" + std::to_string(::getpid()) + ".csv";
  vac_stats.printInstrCounter(fname.c_str(), instructions, approxes);
  fname = app + "_varchc_energy_" + std::to_string(::getpid()) + ".csv";
  vac_stats.printEnergyCounter(fname.c_str(), instructions, approxes);
} // saveStats()
