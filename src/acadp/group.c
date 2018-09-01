/*  ADeLe parsing structures for groups
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

#include "ac_decoder.h"

struct _acadp_group_instr{
  unsigned int id;
  char *mnemonic;
  struct _acadp_group_instr* first;
  struct _acadp_group_instr* next;
};

struct _acadp_group{
  char *name;
  unsigned int id;
  int size;
  int line;

  struct _acadp_group_instr* instructions;

  AcadpModelInst* energy;
  AcadpOP* op;
  
  AcadpGroup* next;
  AcadpGroup* first;
};

AcadpGroup* first_group;

AcadpGroup* acadp_find_group(char* name) {
  AcadpGroup* p = first_group;
  
  while (p != NULL) {
    if (!strcmp(p->name, name)) {
      break;
    }
    p = p->next;
  }
  return p;
}

AcadpGroup* acadp_build_group(char *name) {
  AcadpGroup* p;

  p = malloc(sizeof(AcadpGroup));
  p->name = malloc(sizeof(char) * (1 + strlen(name)));
  strcpy(p->name, name);
  p->size = 0;
  p->line = adf_line_num;

  p->instructions = NULL;

  p->energy = NULL;
  p->op = NULL;

  p->next = NULL;

  return p;
}

int acadp_new_group(char *name, AcadpGroup** group) {
  if ( (*group = acadp_find_group(name)) != NULL) {
    return (*group)->line;
  }

  AcadpGroup** p;
  int id = 0;

  *group = acadp_build_group(name);
  
  p = &first_group;
  while (*p != NULL) {
    p = &((*p)->next);
    id++;
  }
  (*group)->id = id;
  *p = *group;

  (*group)->first = first_group;

  return 0;
}

int acadp_add_instruction_to_group(char* name, AcadpGroup* group) {
  extern ac_dec_instr *instr_list;
  ac_dec_instr *instr = GetInstrByMnemonic(instr_list, name);
  if (instr == NULL) {
    return -1;
  }
  
  struct _acadp_group_instr* ginstr;
  ginstr = malloc(sizeof(struct _acadp_group_instr));
  ginstr->id = instr->id;
  ginstr->mnemonic = name;
  ginstr->next = NULL;

  struct _acadp_group_instr** p = &(group->instructions);
  while (*p != NULL) {
    if ((*p)->id == ginstr->id) {
      return 0;
    }
    p = &((*p)->next);
  }
  *p = ginstr;
  ginstr->first = group->instructions;

  (group->size)++;

  return 1;
}

int acadp_group_add_energy(AcadpModelInst* model, AcadpGroup* group) {
  AcadpModel* m;
  m = acadp_find_model(model->name, MODEL_EM, model->n_params);
  if(m == NULL || group == NULL) {
    acadp_delete_modelinst(model);
    return 0;
  }

  group->energy = model;
  return 1;
}

int acadp_group_add_op(char* op_name, AcadpGroup* group) {
  AcadpOP* op;
  op = acadp_find_op(op_name);
  if(op == NULL || group == NULL) {
    return 0;
  }

  group->op = op;
  return 1;
}

int acadp_get_group(AcadpGroup** group) {
  if (*group == NULL) {
    *group = first_group;
  }
  else {
    *group = (*group)->next;
  }

  if (*group == NULL) {
    return 0;
  }
  else {
    return 1;
  }
}

char* acadp_group_get_name(AcadpGroup* group) {
  if (group != NULL) {
    return group->name;
  }
  else {
    return NULL;
  }
}

char* acadp_group_get_energy(AcadpGroup* group) {
  if (group != NULL) {
    return acadp_get_modelinst(group->energy);
  }
  else {
    return NULL;
  }
}

AcadpOP* acadp_group_get_op(AcadpGroup* group) {
  if (group != NULL) {
    return group->op;
  }
  else {
    return NULL;
  }
}

int acadp_group_contains_instruction(unsigned int instr, AcadpGroup* group) {
  struct _acadp_group_instr* i;

  i = group->instructions;
  while (i != NULL) {
    if (i->id == instr) {
      return 1;
    }
    i = i->next;
  }

  return 0;
}

void acadp_init_group() {
  first_group = NULL;
}

void acadp_delete_group(AcadpGroup* group) {
  if(group != NULL) {
    free(group->name);
    acadp_delete_modelinst(group->energy);

    struct _acadp_group_instr* i1 = group->instructions;
    struct _acadp_group_instr* i2;
    while (i1 != NULL) {
      free(i1->mnemonic);
      i2 = i1->next;
      free(i1);
      i1 = i2;
    }

    free(group);
  }
}

void acadp_destroy_group() {
  AcadpGroup* g1;
  AcadpGroup* g2;

  g2 = first_group;
  while(g2 != NULL) {
    g1 = g2;
    g2 = g2->next;
    acadp_delete_group(g1);
  }
  first_group = NULL;
}
