/*  ADeLe parsing structures for default models
    Copyright (C) 2018  Isaías Felzmann
    Visit http://varchc.github.io/ for more information.

    This module is part of ArchC
    Copyright (C) 2002-2018  The ArchC Team

    Please see license file distributed with this file.
    
    http://www.archc.org/
*/

#include "acadp.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

AcadpModelInst* default_energy = NULL;
AcadpOP* default_op = NULL;

int acadp_set_default_energy(AcadpModelInst* model) {
  AcadpModel* m;
  m = acadp_find_model(model->name, MODEL_EM, model->n_params);
  if(m == NULL) {
    acadp_delete_modelinst(model);
    return 0;
  }

  default_energy = model;
  return 1;
}

int acadp_set_default_op(char* op_name) {
  AcadpOP* op;
  op = acadp_find_op(op_name);
  if(op == NULL) {
    return 0;
  }

  default_op = op;
  return 1;
}

char* acadp_get_default_energy() {
  if (default_energy != NULL) {
    return acadp_get_modelinst(default_energy);
  }
  else {
    return NULL;
  }
}

AcadpOP* acadp_get_default_op() {
  return default_op;
}
