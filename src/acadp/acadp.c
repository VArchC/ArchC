/*  ADeLe Parser interface with ArchC parser
    Copyright (C) 2018  Isaías Felzmann
    Visit http://varchc.github.io/ for more information.

    This module is part of ArchC
    Copyright (C) 2002-2018  The ArchC Team

    Please see license file distributed with this file.
    
    http://www.archc.org/
*/

#include <stdio.h>
#include "acadp.h"

extern int adfparse();
extern FILE* adfin;
extern int adf_line_num;

int acadp_load(char *file) {
  acadp_init_models();
  acadp_init_approx();
  acadp_init_group();
  acadp_init_op();
  adfin = fopen(file, "r");
  if (adfin == NULL) {
    return 0;
  }
  else {
    return 1;
  }
}

void acadp_unload() {
  if (adfin != NULL) {
    fclose(adfin);
  }
  adfin = NULL;
  acadp_destroy_models();
  acadp_destroy_approx();
  acadp_destroy_group();
  acadp_destroy_op();
}

int acadp_run() {
  adf_line_num = 1;
  return adfparse();
}
