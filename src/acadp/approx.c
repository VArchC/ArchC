/*  ADeLe parsing structures for approximations
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

#ifndef ACADP_STANDALONE_TEST
#include "ac_decoder.h"
#endif

struct _acadp_approx{
  char *name;
  unsigned int id;
  int line;

  AcadpDefaultState default_st;
  AcadpApprox* groups;
  AcadpApprox* instructions;
  AcadpGroup* pgroup;

  AcadpModelInst* energy;
  AcadpModelInst* probability;
  AcadpModelInst* regbank_read;
  AcadpModelInst* regbank_write;
  AcadpModelInst* mem_read;
  AcadpModelInst* mem_write;
  AcadpModelInst* reg_read;
  AcadpModelInst* reg_write;
  AcadpModelInst* pre_behavior;
  AcadpModelInst* post_behavior;
  AcadpModelInst* alt_behavior;

  AcadpOP* op;
  
  AcadpApprox* parent;
  AcadpApprox* next;
  AcadpApprox* first;
};

AcadpApprox* first_approx;

AcadpApprox* acadp_find_approx(char* name, AcadpApprox* level) {
  AcadpApprox* p;

  if (level == NULL) {
    return NULL;
  }
  else {
    p = level->first;
  }
  
  while (p != NULL) {
    if (!strcmp(p->name, name)) {
      break;
    }
    p = p->next;
  }
  return p;
}

AcadpApprox* acadp_build_approx(char *name) {
  AcadpApprox* p;

  p = malloc(sizeof(AcadpApprox));
  p->name = malloc(sizeof(char) * (1 + strlen(name)));
  strcpy(p->name, name);
  p->line = adf_line_num;

  p->instructions = NULL;
  p->groups = NULL;
  p->pgroup = NULL;

  p->energy = NULL;
  p->probability = NULL;
  p->regbank_read = NULL;
  p->regbank_write = NULL;
  p->mem_read = NULL;
  p->mem_write = NULL;
  p->reg_read = NULL;
  p->reg_write = NULL;
  p->pre_behavior = NULL;
  p->post_behavior = NULL;
  p->alt_behavior = NULL;
  p->op = NULL;

  p->next = NULL;

  return p;
}

int acadp_new_approx(char *name, AcadpApprox **approx) {
  if ( (*approx = acadp_find_approx(name, first_approx)) != NULL) {
    return (*approx)->line;
  }

  AcadpApprox** p;
  int id = 0;

  *approx = acadp_build_approx(name);
  (*approx)->parent = NULL;
  
  p = &first_approx;
  while (*p != NULL) {
    p = &((*p)->next);
    id++;
  }
  (*approx)->id = id;
  *p = *approx;

  (*approx)->first = first_approx;

  return 0;
}

int acadp_add_group_to_approx(char *name, AcadpApprox **approx) {
  AcadpApprox *a;
  if ( (a = acadp_find_approx(name, (*approx)->groups)) != NULL) {
    *approx = a;
    return a->line;
  }

  AcadpGroup *group = acadp_find_group(name);
  if (group == NULL) {
    return -1;
  }

  AcadpApprox **p;
  AcadpApprox *ng;

  ng = acadp_build_approx(name);
  ng->parent = *approx;
  ng->pgroup = group;
  if ((*approx)->groups == NULL) {
    ng->first = ng;
    (*approx)->groups = ng;
    *approx = ng;
  }
  else {
    ng->first = (*approx)->groups;
    *approx = ng;

    p = &(ng->first);
    while (*p != NULL) {
      p = &((*p)->next);
    }
    *p = ng;
  }
  return 0;
}

int acadp_add_instruction_to_approx(char* name, AcadpApprox** approx) {
  AcadpApprox *a;
  if ( (a = acadp_find_approx(name, (*approx)->instructions)) != NULL) {
    *approx = a;
    return a->line;
  }

  extern ac_dec_instr *instr_list;
  ac_dec_instr *instr = GetInstrByMnemonic(instr_list, name);
  if (instr == NULL) {
    return -1;
  }

  AcadpApprox **p;
  AcadpApprox *ni;

  ni = acadp_build_approx(name);
  ni->parent = *approx;
  ni->id = instr->id;
  if ((*approx)->instructions == NULL) {
    ni->first = ni;
    (*approx)->instructions = ni;
    *approx = ni;
  }
  else {
    ni->first = (*approx)->instructions;
    *approx = ni;

    p = &(ni->first);
    while (*p != NULL) {
      p = &((*p)->next);
    }
    *p = ni;
  }
  return 0;
}

void acadp_close_approx(AcadpApprox** approx) {
  if (*approx != NULL) {
    *approx = (*approx)->parent;
  }
}

void acadp_approx_set_default_st(AcadpApprox* approx, AcadpDefaultState st) {
  approx->default_st = st;
}

int acadp_approx_add_energy(AcadpModelInst* model, AcadpApprox* approx) {
  if (approx == NULL) {
    acadp_delete_modelinst(model);
    return -1;
  }

  AcadpModel* m;
  m = acadp_find_model(model->name, MODEL_EM, model->n_params);
  if(m == NULL) {
    acadp_delete_modelinst(model);
    return 0;
  }

  model->model = m;

  approx->energy = model;
  return 1;
}

int acadp_approx_add_op(char* op_name, AcadpApprox* approx) {
  if (approx == NULL) {
    return -1;
  }

  AcadpOP* op;
  op = acadp_find_op(op_name);
  if(op == NULL) {
    return 0;
  }

  approx->op = op;
  return 1;
}

int acadp_approx_add_probability(AcadpModelInst* model, AcadpApprox* approx) {
  if (approx == NULL) {
    acadp_delete_modelinst(model);
    return -1;
  }

  AcadpModel* m;
  m = acadp_find_model(model->name, MODEL_PM, model->n_params);
  if(m == NULL) {
    acadp_delete_modelinst(model);
    return 0;
  }

  model->model = m;
  approx->probability = model;
  return 1;
}

int acadp_approx_add_regbank_read(AcadpModelInst* model, AcadpApprox* approx) {
  if (approx == NULL) {
    acadp_delete_modelinst(model);
    return -1;
  }

  AcadpModel* m;
  m = acadp_find_model(model->name, MODEL_DM, model->n_params + 2);
  if(m == NULL) {
    acadp_delete_modelinst(model);
    return 0;
  }
  acadp_add_param_to_modelinst(model, "source");
  acadp_add_param_to_modelinst(model, "data");

  model->model = m;
  approx->regbank_read = model;
  return 1;
}

int acadp_approx_add_regbank_write(AcadpModelInst* model, AcadpApprox* approx) {
  if (approx == NULL) {
    acadp_delete_modelinst(model);
    return -1;
  }

  AcadpModel* m;
  m = acadp_find_model(model->name, MODEL_DM, model->n_params + 2);
  if(m == NULL) {
    acadp_delete_modelinst(model);
    return 0;
  }
  acadp_add_param_to_modelinst(model, "source");
  acadp_add_param_to_modelinst(model, "data");

  model->model = m;
  approx->regbank_write = model;
  return 1;
}

int acadp_approx_add_mem_read(AcadpModelInst* model, AcadpApprox* approx) {
  if (approx == NULL) {
    acadp_delete_modelinst(model);
    return -1;
  }

  AcadpModel* m;
  m = acadp_find_model(model->name, MODEL_DM, model->n_params + 2);
  if(m == NULL) {
    acadp_delete_modelinst(model);
    return 0;
  }
  acadp_add_param_to_modelinst(model, "source");
  acadp_add_param_to_modelinst(model, "data");

  model->model = m;
  approx->mem_read = model;
  return 1;
}

int acadp_approx_add_mem_write(AcadpModelInst* model, AcadpApprox* approx) {
  if (approx == NULL) {
    acadp_delete_modelinst(model);
    return -1;
  }

  AcadpModel* m;
  m = acadp_find_model(model->name, MODEL_DM, model->n_params + 2);
  if(m == NULL) {
    acadp_delete_modelinst(model);
    return 0;
  }
  acadp_add_param_to_modelinst(model, "source");
  acadp_add_param_to_modelinst(model, "data");

  model->model = m;
  approx->mem_write = model;
  return 1;
}

int acadp_approx_add_reg_read(AcadpModelInst* model, AcadpApprox* approx) {
  if (approx == NULL) {
    acadp_delete_modelinst(model);
    return -1;
  }

  AcadpModel* m;
  m = acadp_find_model(model->name, MODEL_DM, model->n_params + 2);
  if(m == NULL) {
    acadp_delete_modelinst(model);
    return 0;
  }
  acadp_add_param_to_modelinst(model, "source");
  acadp_add_param_to_modelinst(model, "data");

  model->model = m;
  approx->reg_read = model;
  return 1;
}

int acadp_approx_add_reg_write(AcadpModelInst* model, AcadpApprox* approx) {
  if (approx == NULL) {
    acadp_delete_modelinst(model);
    return -1;
  }

  AcadpModel* m;
  m = acadp_find_model(model->name, MODEL_DM, model->n_params + 2);
  if(m == NULL) {
    acadp_delete_modelinst(model);
    return 0;
  }
  acadp_add_param_to_modelinst(model, "source");
  acadp_add_param_to_modelinst(model, "data");

  model->model = m;
  approx->reg_write = model;
  return 1;
}

int acadp_approx_add_pre_behavior(AcadpModelInst* model, AcadpApprox* approx) {
  if (approx == NULL) {
    acadp_delete_modelinst(model);
    return -1;
  }

  AcadpModel* m;
  m = acadp_find_model(model->name, MODEL_IM, model->n_params);
  if(m == NULL) {
    acadp_delete_modelinst(model);
    return 0;
  }

  model->model = m;
  approx->pre_behavior = model;
  return 1;
}

int acadp_approx_add_post_behavior(AcadpModelInst* model, AcadpApprox* approx) {
  if (approx == NULL) {
    acadp_delete_modelinst(model);
    return -1;
  }

  AcadpModel* m;
  m = acadp_find_model(model->name, MODEL_IM, model->n_params);
  if(m == NULL) {
    acadp_delete_modelinst(model);
    return 0;
  }

  model->model = m;
  approx->post_behavior = model;
  return 1;
}

int acadp_approx_add_alt_behavior(AcadpModelInst* model, AcadpApprox* approx) {
  if (approx == NULL) {
    acadp_delete_modelinst(model);
    return -1;
  }

  AcadpModel* m;
  m = acadp_find_model(model->name, MODEL_IM, model->n_params);
  if(m == NULL) {
    acadp_delete_modelinst(model);
    return 0;
  }

  model->model = m;
  approx->alt_behavior = model;
  return 1;
}

int acadp_get_approx(AcadpApprox** approx) {
  if (*approx == NULL) {
    *approx = first_approx;
  }
  else {
    *approx = (*approx)->next;
  }

  if (*approx == NULL) {
    return 0;
  }
  else {
    return 1;
  }
}

char* acadp_approx_get_name(AcadpApprox* approx) {
  return approx->name;
}

AcadpDefaultState acadp_approx_get_default_st(AcadpApprox* approx) {
  return approx->default_st;
}

char* acadp_approx_get_energy(AcadpApprox* approx, unsigned int instr) {
  if (approx == NULL) {
    return NULL;
  }

  AcadpModelInst *r;
  AcadpApprox *a;

  r = approx->energy;

  a = approx->groups;
  while (a != NULL) {
    if (a->energy != NULL && acadp_group_contains_instruction(instr, a->pgroup)) {
      r = a->energy;
      break;
    }
    a = a->next;
  }

  a = approx->instructions;
  while (a != NULL) {
    if (a->id == instr) {
      if (a->energy != NULL) {
        r = a->energy;
      }
      break;
    }
    a = a->next;
  }

  return acadp_get_modelinst(r);
}

AcadpOP* acadp_approx_get_op(AcadpApprox* approx, unsigned int instr) {
  if (approx == NULL) {
    return NULL;
  }

  AcadpOP *r;
  AcadpApprox *a;

  r = approx->op;

  a = approx->groups;
  while (a != NULL) {
    if (a->op != NULL && acadp_group_contains_instruction(instr, a->pgroup)) {
      r = a->op;
      break;
    }
    a = a->next;
  }

  a = approx->instructions;
  while (a != NULL) {
    if (a->id == instr) {
      if (a->op != NULL) {
        r = a->op;
      }
      break;
    }
    a = a->next;
  }

  return r;
}

char* acadp_approx_get_probability(AcadpApprox* approx, unsigned int instr) {
  if (approx == NULL) {
    return NULL;
  }

  AcadpModelInst *r;
  AcadpApprox *a;
  r = approx->probability;

  a = approx->groups;
  while (a != NULL) {
    if (a->probability != NULL && acadp_group_contains_instruction(instr, a->pgroup)) {
      r = a->probability;
      break;
    }
    a = a->next;
  }

  a = approx->instructions;
  while (a != NULL) {
    if (a->id == instr) {
      if (a->probability != NULL) {
        r = a->probability;
      }
      break;
    }
    a = a->next;
  }

  return acadp_get_modelinst(r);
}

char* acadp_approx_get_regbank_read(AcadpApprox* approx, unsigned int instr) {
  if (approx == NULL) {
    return NULL;
  }

  AcadpModelInst *r;
  AcadpApprox *a;
  r = approx->regbank_read;

  a = approx->groups;
  while (a != NULL) {
    if (a->regbank_read != NULL && acadp_group_contains_instruction(instr, a->pgroup)) {
      r = a->regbank_read;
      break;
    }
    a = a->next;
  }

  a = approx->instructions;
  while (a != NULL) {
    if (a->id == instr) {
      if (a->regbank_read != NULL) {
        r = a->regbank_read;
      }
      break;
    }
    a = a->next;
  }

  return acadp_get_modelinst(r);
}

char* acadp_approx_get_regbank_write(AcadpApprox* approx, unsigned int instr) {
  if (approx == NULL) {
    return NULL;
  }

  AcadpModelInst *r;
  AcadpApprox *a;
  r = approx->regbank_write;

  a = approx->groups;
  while (a != NULL) {
    if (a->regbank_write != NULL && acadp_group_contains_instruction(instr, a->pgroup)) {
      r = a->regbank_write;
      break;
    }
    a = a->next;
  }

  a = approx->instructions;
  while (a != NULL) {
    if (a->id == instr) {
      if (a->regbank_write != NULL) {
        r = a->regbank_write;
      }
      break;
    }
    a = a->next;
  }

  return acadp_get_modelinst(r);
}

char* acadp_approx_get_mem_read(AcadpApprox* approx, unsigned int instr) {
  if (approx == NULL) {
    return NULL;
  }

  AcadpModelInst *r;
  AcadpApprox *a;
  r = approx->mem_read;

  a = approx->groups;
  while (a != NULL) {
    if (a->mem_read != NULL && acadp_group_contains_instruction(instr, a->pgroup)) {
      r = a->mem_read;
      break;
    }
    a = a->next;
  }

  a = approx->instructions;
  while (a != NULL) {
    if (a->id == instr) {
      if (a->mem_read != NULL) {
        r = a->mem_read;
      }
      break;
    }
    a = a->next;
  }

  return acadp_get_modelinst(r);
}

char* acadp_approx_get_mem_write(AcadpApprox* approx, unsigned int instr) {
  if (approx == NULL) {
    return NULL;
  }

  AcadpModelInst *r;
  AcadpApprox *a;
  r = approx->mem_write;

  a = approx->groups;
  while (a != NULL) {
    if (a->mem_write != NULL && acadp_group_contains_instruction(instr, a->pgroup)) {
      r = a->mem_write;
      break;
    }
    a = a->next;
  }

  a = approx->instructions;
  while (a != NULL) {
    if (a->id == instr) {
      if (a->mem_write != NULL) {
        r = a->mem_write;
      }
      break;
    }
    a = a->next;
  }

  return acadp_get_modelinst(r);
}

char* acadp_approx_get_reg_read(AcadpApprox* approx, unsigned int instr) {
  if (approx == NULL) {
    return NULL;
  }

  AcadpModelInst *r;
  AcadpApprox *a;
  r = approx->reg_read;

  a = approx->groups;
  while (a != NULL) {
    if (a->reg_read != NULL && acadp_group_contains_instruction(instr, a->pgroup)) {
      r = a->reg_read;
      break;
    }
    a = a->next;
  }

  a = approx->instructions;
  while (a != NULL) {
    if (a->id == instr) {
      if (a->reg_read != NULL) {
        r = a->reg_read;
      }
      break;
    }
    a = a->next;
  }

  return acadp_get_modelinst(r);
}

char* acadp_approx_get_reg_write(AcadpApprox* approx, unsigned int instr) {
  if (approx == NULL) {
    return NULL;
  }

  AcadpModelInst *r;
  AcadpApprox *a;
  r = approx->reg_write;

  a = approx->groups;
  while (a != NULL) {
    if (a->reg_write != NULL && acadp_group_contains_instruction(instr, a->pgroup)) {
      r = a->reg_write;
      break;
    }
    a = a->next;
  }

  a = approx->instructions;
  while (a != NULL) {
    if (a->id == instr) {
      if (a->reg_write != NULL) {
        r = a->reg_write;
      }
      break;
    }
    a = a->next;
  }

  return acadp_get_modelinst(r);
}

char* acadp_approx_get_pre_behavior(AcadpApprox* approx, unsigned int instr) {
  if (approx == NULL) {
    return NULL;
  }

  AcadpModelInst *r;
  AcadpApprox *a;
  r = approx->pre_behavior;

  a = approx->groups;
  while (a != NULL) {
    if (a->pre_behavior != NULL && acadp_group_contains_instruction(instr, a->pgroup)) {
      r = a->pre_behavior;
      break;
    }
    a = a->next;
  }

  a = approx->instructions;
  while (a != NULL) {
    if (a->id == instr) {
      if (a->pre_behavior != NULL) {
        r = a->pre_behavior;
      }
      break;
    }
    a = a->next;
  }

  return acadp_get_modelinst(r);
}

char* acadp_approx_get_post_behavior(AcadpApprox* approx, unsigned int instr) {
  if (approx == NULL) {
    return NULL;
  }

  AcadpModelInst *r;
  AcadpApprox *a;
  r = approx->post_behavior;

  a = approx->groups;
  while (a != NULL) {
    if (a->post_behavior != NULL && acadp_group_contains_instruction(instr, a->pgroup)) {
      r = a->post_behavior;
      break;
    }
    a = a->next;
  }

  a = approx->instructions;
  while (a != NULL) {
    if (a->id == instr) {
      if (a->post_behavior != NULL) {
        r = a->post_behavior;
      }
      break;
    }
    a = a->next;
  }

  return acadp_get_modelinst(r);
}

char* acadp_approx_get_alt_behavior(AcadpApprox* approx, unsigned int instr) {
  if (approx == NULL) {
    return NULL;
  }

  AcadpModelInst *r;
  AcadpApprox *a;
  r = approx->alt_behavior;

  a = approx->groups;
  while (a != NULL) {
    if (a->alt_behavior != NULL && acadp_group_contains_instruction(instr, a->pgroup)) {
      r = a->alt_behavior;
      break;
    }
    a = a->next;
  }

  a = approx->instructions;
  while (a != NULL) {
    if (a->id == instr) {
      if (a->alt_behavior != NULL) {
        r = a->alt_behavior;
      }
      break;
    }
    a = a->next;
  }

  return acadp_get_modelinst(r);
}

void acadp_init_approx() {
  first_approx = NULL;
}

void acadp_delete_approx(AcadpApprox* approx) {
  if(approx != NULL) {
    free(approx->name);
    acadp_delete_approx(approx->groups);
    acadp_delete_approx(approx->instructions);

    acadp_delete_modelinst(approx->energy);
    acadp_delete_modelinst(approx->probability);
    acadp_delete_modelinst(approx->regbank_read);
    acadp_delete_modelinst(approx->regbank_write);
    acadp_delete_modelinst(approx->mem_read);
    acadp_delete_modelinst(approx->mem_write);
    acadp_delete_modelinst(approx->reg_read);
    acadp_delete_modelinst(approx->reg_write);
    acadp_delete_modelinst(approx->pre_behavior);
    acadp_delete_modelinst(approx->post_behavior);
    acadp_delete_modelinst(approx->alt_behavior);

    free(approx);
  }
}

void acadp_destroy_approx() {
  AcadpApprox* a1;
  AcadpApprox* a2;

  a2 = first_approx;
  while(a2 != NULL) {
    a1 = a2;
    a2 = a2->next;
    acadp_delete_approx(a1);
  }
  first_approx = NULL;
}
