#ifndef _ACSIM_VARCHC_H_
#define _ACSIM_VARCHC_H_

#include "acadp.h"

void CreateVArchCFiles();
void EmitInstrBhvMethodCall_VArchC(FILE *output, int base_indent, ac_dec_instr *pinstr, ac_dec_format *pformat);
void EmitArchUsingDirectives(FILE *fp, const char* classname, int indent, int syscall);
#endif
