#include <stdio.h>
#include <string.h>
#include "acsim.h"
#include "acsim_varchc.h"

int HaveADF = 0;

FILE* CreateFile(const char* suffix) {
  extern char *project_name;
  int nc = strlen(project_name) + strlen(suffix) + 1;
  char *fpath = malloc(nc * sizeof(char));
  snprintf(fpath, nc, "%s%s", project_name, suffix);
  FILE* fp;
  if ( !(fp = fopen(fpath, "w")) ) {
    fprintf(stderr, "ArchC could not open output file %s%s.\n", project_name, suffix);
    free(fpath);
    exit(1);
  }
  free(fpath);
  return fp;
}

void EmitInstrBhvMethodCall_VArchC(FILE *output, int base_indent, ac_dec_instr *pinstr, ac_dec_format *pformat) {
  extern int ACDecCacheFlag;
  AcadpApprox* papprox = NULL;
  bool has_approx = false;
  
  while (acadp_get_approx(&papprox)) {
    if (acadp_approx_get_pre_behavior(papprox, pinstr->id) != NULL || acadp_approx_get_post_behavior(papprox, pinstr->id) != NULL || acadp_approx_get_alt_behavior(papprox, pinstr->id) != NULL) {
      has_approx = true;
      break;
    }
  }

  if( ACDecCacheFlag ) {
    fprintf(output, "%svac_control.EM[%d](&(instr_dec->F_%s));\n", INDENT[base_indent + 1], pinstr->id, pformat->name);
  }
  else {
    fprintf(output, "%svac_control.EM[%d](ins_cache);\n", INDENT[base_indent + 1], pinstr->id);
  }

  if (has_approx) {
    if( ACDecCacheFlag ) {
      fprintf(output, "%svac_control.IM[%d](&(instr_dec->F_%s));\n", INDENT[base_indent + 1], pinstr->id, pformat->name);
    }
    else {
      fprintf(output, "%svac_control.IM[%d](ins_cache);\n", INDENT[base_indent + 1], pinstr->id);
    }
  }
  else {
    EmitInstrBhvMethodCall(output, base_indent, pinstr, pformat);
  }
}

void CreateVArchCControlHeader() {
  FILE *fp = CreateFile("_varchc_control.H");
  extern char* upper_project_name, *project_name;
  print_comment(fp, "VArchC Control Header");

  fprintf(fp, "#ifndef _%s_VARCHC_CONTROL_H_\n", upper_project_name);
  fprintf(fp, "#define _%s_VARCHC_CONTROL_H_\n\n", upper_project_name);
  fprintf(fp, "#include \"vac_control.H\"\n");
  fprintf(fp, "#include \"%s_varchc_wrappers.H\"\n\n", project_name);

  fprintf(fp, "namespace VArchC {\n");
  fprintf(fp, "%sclass Arch;\n", INDENT[1]);
  fprintf(fp, "%sclass Control: public Control_base {\n", INDENT[1]);
  fprintf(fp, "%sprivate:\n", INDENT[2]);
  fprintf(fp, "%sWrappers::Instruction iwrappers;\n", INDENT[3]);
  fprintf(fp, "%sWrappers::Data dwrappers;\n", INDENT[3]);
  fprintf(fp, "%sWrappers::Energy ewrappers;\n\n", INDENT[3]);

  fprintf(fp, "%spublic:\n", INDENT[2]);
  fprintf(fp, "%sControl(Arch& ref, int *current_instruction, unsigned long long int *ac_instr_counter) : Control_base(current_instruction, ac_instr_counter), iwrappers(ref), dwrappers(ref), ewrappers(ref) {initModels();};\n", INDENT[3]);
  fprintf(fp, "%sControl(Arch& ref, unsigned long long int *ac_instr_counter) : Control_base(ac_instr_counter), iwrappers(ref), dwrappers(ref), ewrappers(ref) {initModels();};\n", INDENT[3]);
  fprintf(fp, "%svoid activateApprox(uint8_t approx_id);\n", INDENT[3]);
  fprintf(fp, "%svoid initModels();\n", INDENT[3]);
  fprintf(fp, "%svoid initStats(std::string app);\n", INDENT[3]);
  fprintf(fp, "%s}; // class VArchC::Control\n", INDENT[1]);
  fprintf(fp, "} // namespace VArchC\n\n");
  fprintf(fp, "#endif // _%s_VARCHC_CONTROL_H_\n", upper_project_name);
}

void print_op(FILE* fp, AcadpOP* op, int instr, int approxid, int indent) {
  if (op != NULL) {
    int op_size = acadp_op_get_size(op);
    char* value;
    bool first = true;
    fprintf(fp, "%sOP.activateOP(%d, %d, {", INDENT[indent], instr, approxid);
    for (int i = 0; i < op_size; i++) {
      value = acadp_op_get_value(op, i);
      if (value != NULL) {
        if (!first) {
          fprintf(fp, ", ");
        }
        fprintf(fp, "{\"%s\", %s}", acadp_op_get_key(i), value);
        first = false;
      }
    }
    fprintf(fp, "});\n");
  }
}

void CreateVArchCControlImpl() {
  extern char* project_name;
  FILE *fp = CreateFile("_varchc_control.cpp");
  AcadpApprox* papprox = NULL;
  int approxid = 0;
  extern ac_dec_instr *instr_list;
  ac_dec_instr *pinstr;
  bool has_approx;
  AcadpGroup* pgroup = NULL;
  AcadpOP* op = NULL;
  char* genergy = NULL;

  print_comment(fp, "VArchC Control Implementation");

  fprintf(fp, "#include \"%s_varchc_control.H\"\n\n", project_name);

  fprintf(fp, "void VArchC::Control::activateApprox(uint8_t approx_id) {\n");
  fprintf(fp, "%sswitch(approx_id) {\n", INDENT[1]);
  while (acadp_get_approx(&papprox)) {
    fprintf(fp, "%scase 0x%02x: // %s\n", INDENT[2], approxid, acadp_approx_get_name(papprox));
    for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
      if (acadp_approx_get_pre_behavior(papprox, pinstr->id) != NULL || acadp_approx_get_post_behavior(papprox, pinstr->id) != NULL || acadp_approx_get_alt_behavior(papprox, pinstr->id) != NULL) {
        fprintf(fp, "%sIM[%d].activateModel(%d, &iwrappers, &Wrappers::Instruction::%s_%s);\n", INDENT[3], pinstr->id, approxid, pinstr->name, acadp_approx_get_name(papprox));
      }
      if (acadp_approx_get_regbank_read(papprox, pinstr->id) != NULL) {
        fprintf(fp, "%sDM_rbnrd[%d].activateModel(%d, &dwrappers, &Wrappers::Data::%s_%s_rbnrd);\n", INDENT[3], pinstr->id, approxid, pinstr->name, acadp_approx_get_name(papprox));
      }
      if (acadp_approx_get_regbank_write(papprox, pinstr->id) != NULL) {
        fprintf(fp, "%sDM_rbnwr[%d].activateModel(%d, &dwrappers, &Wrappers::Data::%s_%s_rbnwr);\n", INDENT[3], pinstr->id, approxid, pinstr->name, acadp_approx_get_name(papprox));
      }
      if (acadp_approx_get_mem_read(papprox, pinstr->id) != NULL) {
        fprintf(fp, "%sDM_memrd[%d].activateModel(%d, &dwrappers, &Wrappers::Data::%s_%s_memrd);\n", INDENT[3], pinstr->id, approxid, pinstr->name, acadp_approx_get_name(papprox));
      }
      if (acadp_approx_get_mem_write(papprox, pinstr->id) != NULL) {
        fprintf(fp, "%sDM_memwr[%d].activateModel(%d, &dwrappers, &Wrappers::Data::%s_%s_memwr);\n", INDENT[3], pinstr->id, approxid, pinstr->name, acadp_approx_get_name(papprox));
      }
      if (acadp_approx_get_reg_read(papprox, pinstr->id) != NULL) {
        fprintf(fp, "%sDM_regrd[%d].activateModel(%d, &dwrappers, &Wrappers::Data::%s_%s_regrd);\n", INDENT[3], pinstr->id, approxid, pinstr->name, acadp_approx_get_name(papprox));
      }
      if (acadp_approx_get_reg_write(papprox, pinstr->id) != NULL) {
        fprintf(fp, "%sDM_regwr[%d].activateModel(%d, &dwrappers, &Wrappers::Data::%s_%s_regwr);\n", INDENT[3], pinstr->id, approxid, pinstr->name, acadp_approx_get_name(papprox));
      }
      if (acadp_approx_get_energy(papprox, pinstr->id) != NULL) {
        fprintf(fp, "%sEM[%d].activateModel(%d, &ewrappers, &Wrappers::Energy::%s_%s);\n", INDENT[3], pinstr->id, approxid, pinstr->name, acadp_approx_get_name(papprox));
      }
      op = acadp_approx_get_op(papprox, pinstr->id);
      if (op != NULL) {
        print_op(fp, op, pinstr->id, approxid, 3);
      }
    }
    fprintf(fp, "%sbreak; // %s\n\n", INDENT[3], acadp_approx_get_name(papprox));
    approxid++;
  }
  fprintf(fp, "%s}\n\n", INDENT[1]);
  fprintf(fp, "%sthis->active_approx |= static_cast<uint64_t>(0x1) << approx_id;\n", INDENT[1]);
  fprintf(fp, "}\n\n");

  fprintf(fp, "void VArchC::Control::initModels() {\n");
  fprintf(fp, "%sDM_rbnrd[0].activateModel(0xff, &dwrappers, &Wrappers::Data::__noapprox__);\n", INDENT[1]);
  fprintf(fp, "%sDM_rbnwr[0].activateModel(0xff, &dwrappers, &Wrappers::Data::__noapprox__);\n", INDENT[1]);
  fprintf(fp, "%sDM_memrd[0].activateModel(0xff, &dwrappers, &Wrappers::Data::__noapprox__);\n", INDENT[1]);
  fprintf(fp, "%sDM_memwr[0].activateModel(0xff, &dwrappers, &Wrappers::Data::__noapprox__);\n", INDENT[1]);
  fprintf(fp, "%sDM_regrd[0].activateModel(0xff, &dwrappers, &Wrappers::Data::__noapprox__);\n", INDENT[1]);
  fprintf(fp, "%sDM_regwr[0].activateModel(0xff, &dwrappers, &Wrappers::Data::__noapprox__);\n", INDENT[1]);
  fprintf(fp, "%sEM[0].activateModel(0xff, &ewrappers, &Wrappers::Energy::__default__);\n", INDENT[1]);
  print_op(fp, acadp_get_default_op(), 0, 0xff, 1);
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    has_approx = false;
    papprox = NULL;
    while (acadp_get_approx(&papprox)) {
      if (acadp_approx_get_pre_behavior(papprox, pinstr->id) != NULL || acadp_approx_get_post_behavior(papprox, pinstr->id) != NULL || acadp_approx_get_alt_behavior(papprox, pinstr->id) != NULL) {
        has_approx = true;
        break;
      }
    }
    if (has_approx) {
      fprintf(fp, "%sIM[%d].activateModel(0xff, &iwrappers, &Wrappers::Instruction::%s);\n", INDENT[1], pinstr->id, pinstr->name);
    }
    fprintf(fp, "%sDM_rbnrd[%d].activateModel(0xff, &dwrappers, &Wrappers::Data::__noapprox__);\n", INDENT[1], pinstr->id);
    fprintf(fp, "%sDM_rbnwr[%d].activateModel(0xff, &dwrappers, &Wrappers::Data::__noapprox__);\n", INDENT[1], pinstr->id);
    fprintf(fp, "%sDM_memrd[%d].activateModel(0xff, &dwrappers, &Wrappers::Data::__noapprox__);\n", INDENT[1], pinstr->id);
    fprintf(fp, "%sDM_memwr[%d].activateModel(0xff, &dwrappers, &Wrappers::Data::__noapprox__);\n", INDENT[1], pinstr->id);
    fprintf(fp, "%sDM_regrd[%d].activateModel(0xff, &dwrappers, &Wrappers::Data::__noapprox__);\n", INDENT[1], pinstr->id);
    fprintf(fp, "%sDM_regwr[%d].activateModel(0xff, &dwrappers, &Wrappers::Data::__noapprox__);\n", INDENT[1], pinstr->id);
    pgroup = NULL;
    genergy = NULL;
    op = NULL;
    while (acadp_get_group(&pgroup)) {
      if (acadp_group_contains_instruction(pinstr->id, pgroup)) {
        if (genergy == NULL && acadp_group_get_energy(pgroup) != NULL) {
          genergy = acadp_group_get_name(pgroup);
        }
        if (op == NULL) {
          op = acadp_group_get_op(pgroup);
        }
        if (genergy != NULL && op != NULL) {
          break;
        }
      }
    }
    if (genergy != NULL) {
      fprintf(fp, "%sEM[%d].activateModel(0xff, &ewrappers, &Wrappers::Energy::%s);\n", INDENT[1], pinstr->id, genergy);
    }
    else {
      fprintf(fp, "%sEM[%d].activateModel(0xff, &ewrappers, &Wrappers::Energy::__default__);\n", INDENT[1], pinstr->id);
    }
    if (op != NULL) {
      print_op(fp, op, pinstr->id, 0xff, 1);
    }
    else {
      print_op(fp, acadp_get_default_op(), pinstr->id, 0xff, 1);
    }
  }
  approxid = 0;
  papprox = NULL;
  while (acadp_get_approx(&papprox)) {
    if (acadp_approx_get_default_st(papprox) == ST_ON) {
      fprintf(fp, "%sactivateApprox(%d);\n", INDENT[1], approxid);
    }
    approxid++;
  }
  fprintf(fp, "}\n\n");

  approxid = 0;
  papprox = NULL;
  fprintf(fp, "void VArchC::Control::initStats(std::string app) {\n");
  fprintf(fp, "%sthis->app = app;\n", INDENT[1]);
  while (acadp_get_approx(&papprox)) {
    fprintf(fp, "%sapproxes[%d] = \"%s\";\n", INDENT[1], approxid, acadp_approx_get_name(papprox));
    approxid++;
  }
  fprintf(fp, "}\n\n");
}

void CreateVArchCWrappersHeader() {
  extern char *project_name, *upper_project_name;
  FILE *fp = CreateFile("_varchc_wrappers.H");
  print_comment(fp, "VArchC Wrappers header file");
  AcadpApprox* papprox = NULL;
  AcadpGroup* pgroup = NULL;
  extern int ACDecCacheFlag;
  extern ac_dec_instr *instr_list;
  ac_dec_instr *pinstr;
  bool has_approx = false;

  fprintf(fp, "#ifndef _%s_VARCHC_WRAPPERS_H_\n", upper_project_name);
  fprintf(fp, "#define _%s_VARCHC_WRAPPERS_H_\n\n", upper_project_name);

  fprintf(fp, "#include \"%s_varchc_arch_ref.H\"\n", project_name);
  fprintf(fp, "#include \"%s_varchc_models.H\"\n", project_name);

  fprintf(fp, "namespace VArchC {\n");
  fprintf(fp, "%sclass Arch;\n", INDENT[1]);
  fprintf(fp, "%snamespace Wrappers {\n", INDENT[1]);

  fprintf(fp, "%sclass Instruction: public Arch_ref {\n", INDENT[2]);
  fprintf(fp, "%sprivate:\n", INDENT[3]);
  fprintf(fp, "%sModels::Instruction models;\n", INDENT[4]);
  fprintf(fp, "%sModels::Probability prob;\n", INDENT[4]);
  fprintf(fp, "%spublic:\n", INDENT[3]);
  fprintf(fp, "%sInstruction(Arch &ref) : Arch_ref(ref), models(ref), prob(ref) {}\n\n", INDENT[4]);
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    has_approx = false;
    papprox = NULL;
    while (acadp_get_approx(&papprox)) {
      if (acadp_approx_get_pre_behavior(papprox, pinstr->id) != NULL || acadp_approx_get_post_behavior(papprox, pinstr->id) != NULL || acadp_approx_get_alt_behavior(papprox, pinstr->id) != NULL) {
        has_approx = true;
        fprintf(fp, "%svoid %s_%s(void* prm);\n", INDENT[4], pinstr->name, acadp_approx_get_name(papprox));
      }
    }
    if (has_approx) {
      fprintf(fp, "%svoid %s(void* prm);\n\n", INDENT[4], pinstr->name);
    }
  }
  fprintf(fp, "%s};// class VArchC::Wrappers::Instruction\n\n", INDENT[2]);

  fprintf(fp, "%sclass Data: public Arch_ref {\n", INDENT[2]);
  fprintf(fp, "%sprivate:\n", INDENT[3]);
  fprintf(fp, "%sModels::Data models;\n", INDENT[4]);
  fprintf(fp, "%sModels::Probability prob;\n", INDENT[4]);
  fprintf(fp, "%spublic:\n", INDENT[3]);
  fprintf(fp, "%sData(Arch &ref) : Arch_ref(ref), models(ref), prob(ref) {}\n\n", INDENT[4]);
  fprintf(fp, "%svoid __noapprox__(void *prm);\n\n", INDENT[4]);
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    papprox = NULL;
    while (acadp_get_approx(&papprox)) {
      has_approx = false;
      if (acadp_approx_get_regbank_read(papprox, pinstr->id) != NULL) {
        fprintf(fp, "%svoid %s_%s_rbnrd(void* prm);\n", INDENT[4], pinstr->name, acadp_approx_get_name(papprox));
        has_approx = true;
      }
      if (acadp_approx_get_regbank_write(papprox, pinstr->id) != NULL) {
        fprintf(fp, "%svoid %s_%s_rbnwr(void* prm);\n", INDENT[4], pinstr->name, acadp_approx_get_name(papprox));
        has_approx = true;
      }
      if (acadp_approx_get_mem_read(papprox, pinstr->id) != NULL) {
        fprintf(fp, "%svoid %s_%s_memrd(void* prm);\n", INDENT[4], pinstr->name, acadp_approx_get_name(papprox));
        has_approx = true;
      }
      if (acadp_approx_get_mem_write(papprox, pinstr->id) != NULL) {
        fprintf(fp, "%svoid %s_%s_memwr(void* prm);\n", INDENT[4], pinstr->name, acadp_approx_get_name(papprox));
        has_approx = true;
      }
      if (acadp_approx_get_reg_read(papprox, pinstr->id) != NULL) {
        fprintf(fp, "%svoid %s_%s_regrd(void* prm);\n", INDENT[4], pinstr->name, acadp_approx_get_name(papprox));
        has_approx = true;
      }
      if (acadp_approx_get_reg_write(papprox, pinstr->id) != NULL) {
        fprintf(fp, "%svoid %s_%s_regwr(void* prm);\n", INDENT[4], pinstr->name, acadp_approx_get_name(papprox));
        has_approx = true;
      }
    }
    if (has_approx) {
      fprintf(fp, "\n");
    }
  }
  fprintf(fp, "%s};// class VArchC::Wrappers::Data\n\n", INDENT[2]);

  fprintf(fp, "%sclass Energy: public Arch_ref {\n", INDENT[2]);
  fprintf(fp, "%sprivate:\n", INDENT[3]);
  fprintf(fp, "%sModels::Energy models;\n", INDENT[4]);
  fprintf(fp, "%sModels::Probability prob;\n", INDENT[4]);
  fprintf(fp, "%spublic:\n", INDENT[3]);
  fprintf(fp, "%sEnergy(Arch &ref) : Arch_ref(ref), models(ref), prob(ref) {}\n\n", INDENT[4]);
  fprintf(fp, "%svoid __default__(void *prm);\n\n", INDENT[4]);
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    papprox = NULL;
    while (acadp_get_approx(&papprox)) {
      if (acadp_approx_get_energy(papprox, pinstr->id) != NULL) {
        fprintf(fp, "%svoid %s_%s(void* prm);\n", INDENT[4], pinstr->name, acadp_approx_get_name(papprox));
      }
    }
  }
  while (acadp_get_group(&pgroup)) {
    if (acadp_group_get_energy(pgroup) != NULL) {
        fprintf(fp, "%svoid %s(void* prm);\n", INDENT[4], acadp_group_get_name(pgroup));
    }
  }
  fprintf(fp, "%s};// class VArchC::Wrappers::Energy\n\n", INDENT[2]);
  fprintf(fp, "%s}// namespace VArchC::Wrappers\n", INDENT[1]);
  fprintf(fp, "}// namespace VArchC\n\n");

  fprintf(fp, "#endif // _%s_VARCHC_WRAPPERS_H_\n", upper_project_name);
}

void CreateVArchCWrappersImpl() {
  extern char *project_name;
  FILE *fp = CreateFile("_varchc_wrappers.cpp");
  print_comment(fp, "VArchC Wrappers implementation file");
  AcadpApprox* papprox = NULL;
  AcadpGroup* pgroup = NULL;
  extern ac_dec_instr *instr_list;
  ac_dec_instr *pinstr;
  ac_dec_format *pformat;
  extern ac_dec_format *format_ins_list;
  ac_dec_field *pfield;
  bool has_approx;
  int indent = 1;
  extern int ACDecCacheFlag;

  char *energy;
  char *probability;
  char *pre_bhv;
  char *pos_bhv;
  char *alt_bhv;
  char *rbnrd;
  char *rbnwr;
  char *memrd;
  char *memwr;
  char *regrd;
  char *regwr;

  fprintf(fp, "#include \"%s_varchc_wrappers.H\"\n", project_name);
  fprintf(fp, "#include \"%s_varchc.H\"\n", project_name);
  fprintf(fp, "#include \"%s_varchc_models.H\"\n", project_name);
  fprintf(fp, "#include \"%s_parms.H\"\n", project_name);
  fprintf(fp, "#include \"%s_isa.H\"\n", project_name);
  fprintf(fp, "#include \"vac_storage.H\"\n\n");

  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    for (pformat = format_ins_list; (pformat != NULL) && strcmp(pinstr->format, pformat->name); pformat = pformat->next);
    has_approx = false;
    papprox = NULL;
    while (acadp_get_approx(&papprox)) {
      probability = acadp_approx_get_probability(papprox, pinstr->id);
      pre_bhv = acadp_approx_get_pre_behavior(papprox, pinstr->id);
      pos_bhv = acadp_approx_get_post_behavior(papprox, pinstr->id);
      alt_bhv = acadp_approx_get_alt_behavior(papprox, pinstr->id);
      if (pre_bhv != NULL || pos_bhv != NULL || alt_bhv != NULL) {
        has_approx = true;
        fprintf(fp, "void VArchC::Wrappers::Instruction::%s_%s(void* prm){\n", pinstr->name, acadp_approx_get_name(papprox));
        for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
          fprintf(fp, "%s", INDENT[1]);
          if (!pfield->sign) fprintf(fp, "u");
          if (pfield->size < 9) fprintf(fp, "int8_t");
          else if (pfield->size < 17) fprintf(fp, "int16_t");
          else if (pfield->size < 33) fprintf(fp, "int32_t");
          else fprintf(fp, "int64_t");
          fprintf(fp, " &%s = ", pfield->name);
          if(ACDecCacheFlag) {
            fprintf(fp, "reinterpret_cast<VArchC::%s::T_%s*>(prm)->%s;\n", project_name, pformat->name, pfield->name);
          }
          else {
            fprintf(fp, "reinterpret_cast<unsigned*>(prm)[%d];\n", pfield->id);
          }
        }
        if (probability != NULL) {
          fprintf(fp, "%sif (prob.%s) {\n", INDENT[indent++], probability);
        }
        if (pre_bhv != NULL) {
          fprintf(fp, "%smodels.%s;\n", INDENT[indent], pre_bhv);
        }
        if (alt_bhv != NULL) {
          fprintf(fp, "%smodels.%s;\n", INDENT[indent], alt_bhv);
        }
        if (pos_bhv != NULL) {
          fprintf(fp, "%smodels.%s;\n", INDENT[indent], pos_bhv);
        }
        if (probability != NULL) {
          fprintf(fp, "%s}\n", INDENT[--indent]);
          fprintf(fp, "%selse {\n", INDENT[indent++]);
          fprintf(fp, "%sISA.behavior_%s(", INDENT[indent], pinstr->name);
          for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next){
            fprintf(fp, "%s", pfield->name);
            if (pfield->next != NULL)
              fprintf(fp, ", ");
          }
          fprintf(fp, ");\n");
          fprintf(fp, "%s}\n", INDENT[--indent]);
        }
        fprintf(fp, "}\n");
      }
    }
    if (has_approx) {
      fprintf(fp, "void VArchC::Wrappers::Instruction::%s(void* prm){\n", pinstr->name);
      fprintf(fp, "%sISA.behavior_%s(", INDENT[1], pinstr->name);
      for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next){
        if(ACDecCacheFlag) {
          fprintf(fp, "reinterpret_cast<VArchC::%s::T_%s*>(prm)->%s", project_name, pformat->name, pfield->name);
        }
        else {
          fprintf(fp, "reinterpret_cast<unsigned*>(prm)[%d]", pfield->id);
        }
        if (pfield->next != NULL)
          fprintf(fp, ", ");
      }
      fprintf(fp, ");\n");
      fprintf(fp, "}\n\n");
    }
  }


  fprintf(fp, "void VArchC::Wrappers::Data::__noapprox__(void *prm) {}\n\n");
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    papprox = NULL;
    while (acadp_get_approx(&papprox)) {
      probability = acadp_approx_get_probability(papprox, pinstr->id);
      rbnrd = acadp_approx_get_regbank_read(papprox, pinstr->id);
      rbnwr = acadp_approx_get_regbank_write(papprox, pinstr->id);
      memrd = acadp_approx_get_mem_read(papprox, pinstr->id);
      memwr = acadp_approx_get_mem_write(papprox, pinstr->id);
      regrd = acadp_approx_get_reg_read(papprox, pinstr->id);
      regwr = acadp_approx_get_reg_write(papprox, pinstr->id);
      if (rbnrd != NULL) {
        fprintf(fp, "void VArchC::Wrappers::Data::%s_%s_rbnrd(void* prm){\n", pinstr->name, acadp_approx_get_name(papprox));
        fprintf(fp, "%ssource_t& source = reinterpret_cast<VArchC::Storage::Encoder*>(prm)->source;\n", INDENT[indent]);
        fprintf(fp, "%suint64_t& data = reinterpret_cast<VArchC::Storage::Encoder*>(prm)->data;\n", INDENT[indent]);
        if (probability != NULL) {
          fprintf(fp, "%sif (prob.%s) {\n", INDENT[indent++], probability);
        }
        fprintf(fp, "%smodels.%s;\n", INDENT[indent], rbnrd);
        if (probability != NULL) {
          fprintf(fp, "%s}\n", INDENT[--indent]);
        }
        fprintf(fp, "}\n\n");
      }
      if (rbnwr != NULL) {
        fprintf(fp, "void VArchC::Wrappers::Data::%s_%s_rbnwr(void* prm){\n", pinstr->name, acadp_approx_get_name(papprox));
        fprintf(fp, "%ssource_t& source = reinterpret_cast<VArchC::Storage::Encoder*>(prm)->source;\n", INDENT[1]);
        fprintf(fp, "%suint64_t& data = reinterpret_cast<VArchC::Storage::Encoder*>(prm)->data;\n", INDENT[indent]);
        if (probability != NULL) {
          fprintf(fp, "%sif (prob.%s) {\n", INDENT[indent++], probability);
        }
        fprintf(fp, "%smodels.%s;\n", INDENT[indent], rbnwr);
        if (probability != NULL) {
          fprintf(fp, "%s}\n", INDENT[--indent]);
        }
        fprintf(fp, "}\n\n");
      }
      if (memrd != NULL) {
        fprintf(fp, "void VArchC::Wrappers::Data::%s_%s_memrd(void* prm){\n", pinstr->name, acadp_approx_get_name(papprox));
        fprintf(fp, "%ssource_t& source = reinterpret_cast<VArchC::Storage::Encoder*>(prm)->source;\n", INDENT[indent]);
        fprintf(fp, "%suint64_t& data = reinterpret_cast<VArchC::Storage::Encoder*>(prm)->data;\n", INDENT[indent]);
        if (probability != NULL) {
          fprintf(fp, "%sif (prob.%s) {\n", INDENT[indent++], probability);
        }
        fprintf(fp, "%smodels.%s;\n", INDENT[indent], memrd);
        if (probability != NULL) {
          fprintf(fp, "%s}\n", INDENT[--indent]);
        }
        fprintf(fp, "}\n\n");
      }
      if (memwr != NULL) {
        fprintf(fp, "void VArchC::Wrappers::Data::%s_%s_memwr(void* prm){\n", pinstr->name, acadp_approx_get_name(papprox));
        fprintf(fp, "%ssource_t& source = reinterpret_cast<VArchC::Storage::Encoder*>(prm)->source;\n", INDENT[indent]);
        fprintf(fp, "%suint64_t& data = reinterpret_cast<VArchC::Storage::Encoder*>(prm)->data;\n", INDENT[indent]);
        if (probability != NULL) {
          fprintf(fp, "%sif (prob.%s) {\n", INDENT[indent++], probability);
        }
        fprintf(fp, "%smodels.%s;\n", INDENT[indent], memwr);
        if (probability != NULL) {
          fprintf(fp, "%s}\n", INDENT[--indent]);
        }
        fprintf(fp, "}\n\n");
      }
      if (regrd != NULL) {
        fprintf(fp, "void VArchC::Wrappers::Data::%s_%s_regrd(void* prm){\n", pinstr->name, acadp_approx_get_name(papprox));
        fprintf(fp, "%ssource_t& source = reinterpret_cast<VArchC::Storage::Encoder*>(prm)->source;\n", INDENT[indent]);
        fprintf(fp, "%suint64_t& data = reinterpret_cast<VArchC::Storage::Encoder*>(prm)->data;\n", INDENT[indent]);
        if (probability != NULL) {
          fprintf(fp, "%sif (prob.%s) {\n", INDENT[indent++], probability);
        }
        fprintf(fp, "%smodels.%s;\n", INDENT[indent], regrd);
        if (probability != NULL) {
          fprintf(fp, "%s}\n", INDENT[--indent]);
        }
        fprintf(fp, "}\n\n");
      }
      if (regwr != NULL) {
        fprintf(fp, "void VArchC::Wrappers::Data::%s_%s_regwr(void* prm){\n", pinstr->name, acadp_approx_get_name(papprox));
        fprintf(fp, "%ssource_t& source = reinterpret_cast<VArchC::Storage::Encoder*>(prm)->source;\n", INDENT[indent]);
        fprintf(fp, "%suint64_t& data = reinterpret_cast<VArchC::Storage::Encoder*>(prm)->data;\n", INDENT[indent]);
        if (probability != NULL) {
          fprintf(fp, "%sif (prob.%s) {\n", INDENT[indent++], probability);
        }
        fprintf(fp, "%smodels.%s;\n", INDENT[indent], regwr);
        if (probability != NULL) {
          fprintf(fp, "%s}\n", INDENT[--indent]);
        }
        fprintf(fp, "}\n\n");
      }
    }
  }
 


  energy = acadp_get_default_energy();
  fprintf(fp, "void VArchC::Wrappers::Energy::__default__(void* prm){\n");
  if (energy != NULL) {
    fprintf(fp, "%svac_control.instrDispatch(ISA.cur_instr_id, models.%s);\n", INDENT[1], energy);
  }
  else {
    fprintf(fp, "%svac_control.instrDispatch(ISA.cur_instr_id, 0.0);\n", INDENT[1]);
  }
  fprintf(fp, "}\n");
  for (pinstr = instr_list; pinstr != NULL; pinstr = pinstr->next) {
    for (pformat = format_ins_list; (pformat != NULL) && strcmp(pinstr->format, pformat->name); pformat = pformat->next);
    has_approx = false;
    papprox = NULL;
    while (acadp_get_approx(&papprox)) {
      energy = acadp_approx_get_energy(papprox, pinstr->id);
      if (energy != NULL) {
        has_approx = true;
        fprintf(fp, "void VArchC::Wrappers::Energy::%s_%s(void* prm){\n", pinstr->name, acadp_approx_get_name(papprox));
        for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
          fprintf(fp, "%s", INDENT[1]);
          if (!pfield->sign) fprintf(fp, "u");
          if (pfield->size < 9) fprintf(fp, "int8_t");
          else if (pfield->size < 17) fprintf(fp, "int16_t");
          else if (pfield->size < 33) fprintf(fp, "int32_t");
          else fprintf(fp, "int64_t");
          fprintf(fp, " &%s = ", pfield->name);
          if(ACDecCacheFlag) {
            fprintf(fp, "reinterpret_cast<VArchC::%s::T_%s*>(prm)->%s;\n", project_name, pformat->name, pfield->name);
          }
          else {
            fprintf(fp, "reinterpret_cast<unsigned*>(prm)[%d];\n", pfield->id);
          }
        }
        fprintf(fp, "%svac_control.instrDispatch(ISA.cur_instr_id, models.%s);\n", INDENT[1], energy);
        fprintf(fp, "}\n");
      }
    }
  }

  while(acadp_get_group(&pgroup)) {
    energy = acadp_group_get_energy(pgroup);
    if (energy != NULL) {
      fprintf(fp, "void VArchC::Wrappers::Energy::%s(void* prm){\n", acadp_group_get_name(pgroup));
      fprintf(fp, "%svac_control.instrDispatch(ISA.cur_instr_id, models.%s);\n", INDENT[1], energy);
      fprintf(fp, "}\n");
    }
  }

  fprintf(fp, "#include \"%s_isa.cpp\"\n", project_name);
  fprintf(fp, "#include \"%s_varchc_models.cpp\"\n", project_name);
}

void CreateVArchCModelsHeader() {
  extern char *upper_project_name, *project_name;
  FILE *fp = CreateFile("_varchc_models.H");
  print_comment(fp, "VArchC Models header file");

  fprintf(fp, "#ifndef _%s_VARCHC_MODELS_H_\n", upper_project_name);
  fprintf(fp, "#define _%s_VARCHC_MODELS_H_\n\n", upper_project_name);

  fprintf(fp, "#include \"vac_storage.H\"\n\n");

  fprintf(fp, "typedef VArchC::Storage::Word<%s_parms::ac_word>& word;\n", project_name);

  fprintf(fp, "namespace VArchC {\n");
  fprintf(fp, "%sclass Arch;\n", INDENT[1]);
  fprintf(fp, "%snamespace Models {\n", INDENT[1]);
  fprintf(fp, "%sclass Data: public Arch_ref {\n", INDENT[2]);
  fprintf(fp, "%spublic:\n", INDENT[3]);
  fprintf(fp, "%sData(Arch &ref) : Arch_ref(ref) {}\n\n", INDENT[4]);
  acadp_write_models_header(fp, MODEL_DM, INDENT[4]);
  fprintf(fp, "%s};\n\n", INDENT[2]);
  fprintf(fp, "%sclass Instruction: public Arch_ref {\n", INDENT[2]);
  fprintf(fp, "%spublic:\n", INDENT[3]);
  fprintf(fp, "%sInstruction(Arch &ref) : Arch_ref(ref) {}\n\n", INDENT[4]);
  acadp_write_models_header(fp, MODEL_IM, INDENT[4]);
  fprintf(fp, "%s};\n\n", INDENT[2]);
  fprintf(fp, "%sclass Probability: public Arch_ref {\n", INDENT[2]);
  fprintf(fp, "%spublic:\n", INDENT[3]);
  fprintf(fp, "%sProbability(Arch &ref) : Arch_ref(ref) {}\n\n", INDENT[4]);
  acadp_write_models_header(fp, MODEL_PM, INDENT[4]);
  fprintf(fp, "%s};\n\n", INDENT[2]);
  fprintf(fp, "%sclass Energy: public Arch_ref {\n", INDENT[2]);
  fprintf(fp, "%spublic:\n", INDENT[3]);
  fprintf(fp, "%sEnergy(Arch &ref) : Arch_ref(ref) {}\n\n", INDENT[4]);
  acadp_write_models_header(fp, MODEL_EM, INDENT[4]);
  fprintf(fp, "%s};\n", INDENT[2]);
  fprintf(fp, "%s}\n", INDENT[1]);
  fprintf(fp, "%s}\n", INDENT[0]);

  fprintf(fp, "#endif // _%s_VARCHC_MODELS_H_\n", upper_project_name);
  fclose(fp);
}

void CreateVArchCModelsImpl() {
  extern char *project_name;
  FILE *fp = CreateFile("_varchc_models.cpp.tmpl");
  print_comment(fp, "VArchC Models implementation templates");

  fprintf(fp, "#include \"vac_models.H\"\n\n");

  acadp_write_models_template(fp, MODEL_DM);
  acadp_write_models_template(fp, MODEL_IM);
  acadp_write_models_template(fp, MODEL_PM);
  acadp_write_models_template(fp, MODEL_EM);

  fclose(fp);
}

void CreateVArchCApplicationInterface() {
  extern char *upper_project_name;
  FILE *fp = CreateFile("_iface.h");
  AcadpApprox* papprox = NULL;
  int approxid = 0;
  print_comment(fp, "VArchC-Application control interface");

  fprintf(fp, "#ifndef _%s_IFACE_H_\n", upper_project_name);
  fprintf(fp, "#define _%s_IFACE_H_\n\n", upper_project_name);

  fprintf(fp, "volatile unsigned long long int VArchC_ctl;\n");
  fprintf(fp, "unsigned char* VArchC_cmd = ((unsigned char*)(&VArchC_ctl) + 7);\n\n");

  while (acadp_get_approx(&papprox)) {
    fprintf(fp, "#define VAC_APPROX_%s 0x%02x\n", acadp_approx_get_name(papprox), approxid++);
  }
  fprintf(fp, "\n");

  fprintf(fp, "#ifndef _VARCHC_SUPPRESS_\n\n");

  fprintf(fp, "#define vac_activate(approx) (*VArchC_cmd = ((0x40 | approx) & 0x7f))\n");
  
  fprintf(fp, "#define vac_deactivate(approx) (*VArchC_cmd = (approx & 0x3f))\n");
  
  fprintf(fp, "#define vac_clear() (*VArchC_cmd = 0x80)\n");
  
  fprintf(fp, "#define vac_get(approx) (VArchC_ctl & ((unsigned long long int)(0x1) << (approx & 0x3f)))\n\n");

  fprintf(fp, "#else\n\n");

  fprintf(fp, "#define vac_activate(approx)\n");
  
  fprintf(fp, "#define vac_deactivate(approx)\n");
  
  fprintf(fp, "#define vac_clear()\n");
  
  fprintf(fp, "#define vac_get(approx)\n\n");
  
  fprintf(fp, "#endif\n\n");

  fprintf(fp, "#define vac_newSection() (*VArchC_cmd = 0xfe)\n\n");

  fprintf(fp, "#endif // _%s_IFACE_H_\n", upper_project_name);

}

void CreateVArchCFiles() {
  CreateProcessorHeader(1);
  CreateProcessorImpl(1);

  CreateArchHeader(1);
  CreateArchImpl(1);
  CreateArchRefHeader(1);
  CreateArchRefImpl(1);

  CreateVArchCControlHeader();
  CreateVArchCControlImpl();

  CreateVArchCWrappersHeader();
  CreateVArchCWrappersImpl();

  CreateVArchCModelsHeader();
  CreateVArchCModelsImpl();

  CreateVArchCApplicationInterface();
}

void EmitArchUsingDirectives(FILE *fp, const char* classname, int indent, int syscall) {
    extern ac_sto_list *storage_list;
    extern char* project_name;
    extern int HaveTLMIntrPorts, HaveTLM2IntrPorts;
    extern int HaveADF;

    ac_sto_list *pstorage;

    fprintf(fp, "%susing %s::ac_pc;", INDENT[indent], classname);
    if (!syscall) fprintf(fp, "\n");
    fprintf(fp, "%susing %s::ac_id;", INDENT[indent], classname);
    if (!syscall) fprintf(fp, "\n");
    
    /* Declaring storage devices */
    for (pstorage = storage_list; pstorage != NULL; pstorage = pstorage->next) {
        fprintf(fp, "%susing %s::%s;", INDENT[indent], classname, pstorage->name);
        if (!syscall) fprintf(fp, "\n");
    }

    if (HaveTLMIntrPorts || HaveTLM2IntrPorts) {
        fprintf(fp, "%susing %s::intr_reg;", INDENT[indent], classname);
        if (!syscall) fprintf(fp, "\n");
    }

    fprintf(fp, "%susing %s::INST_PORT;", INDENT[indent], classname);
    if (!syscall) fprintf(fp, "\n");
    fprintf(fp, "%susing %s::DATA_PORT;", INDENT[indent], classname);
    if (!syscall) fprintf(fp, "\n");

    if (HaveADF) {
      fprintf(fp, "%susing %s::ISA;\n\n", INDENT[indent], classname);
    }
}

