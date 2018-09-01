/*  ADeLe parsing structures for operating parameters
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

struct _acadp_op{
  char *name;
  unsigned int id;
  int line;
  int size;

  char** values;

  AcadpOP* next;
  AcadpOP* first;
};

AcadpOP* first_op;
char** op_names;
int op_maxsize;

AcadpOP* acadp_find_op(char* name) {
  AcadpOP* p = first_op;
  
  while (p != NULL) {
    if (!strcmp(p->name, name)) {
      break;
    }
    p = p->next;
  }
  return p;
}

AcadpOP* acadp_build_op(char *name) {
  AcadpOP* p;
  int i;

  p = malloc(sizeof(AcadpOP));
  p->name = malloc(sizeof(char) * (1 + strlen(name)));
  strcpy(p->name, name);
  p->line = adf_line_num;
  p->size = op_maxsize;

  p->values = malloc(sizeof(char*) * (p->size));
  for (i = 0; i < p->size; i++) {
    p->values[i] = NULL;
  }

  p->next = NULL;

  return p;
}

int acadp_new_op(char *name, AcadpOP** op) {
  if ( (*op = acadp_find_op(name)) != NULL) {
    return (*op)->line;
  }

  AcadpOP** p;
  int id = 0;

  *op = acadp_build_op(name);
  
  p = &first_op;
  while (*p != NULL) {
    p = &((*p)->next);
    id++;
  }
  (*op)->id = id;
  *p = *op;

  (*op)->first = first_op;

  return 0;
}

int acadp_add_param_to_op(char* name, char* value, AcadpOP* op) {
  int param;
  int i;

  int glen = strlen(name);
  for (param = 0; param < op_maxsize; param++) {
    if (strlen(op_names[param]) == glen && strncmp(name, op_names[param], glen) == 0) {
      break;
    }
  }

  if (param == op_maxsize) {
    op_maxsize++;
    op_names = realloc(op_names, op_maxsize * sizeof(char*));
    op_names[param] = malloc(glen * sizeof(char));
    strcpy(op_names[param], name);
    op->values = realloc(op->values, op_maxsize * sizeof(char*));
    for (i = op->size; i < op_maxsize; i++) {
      op->values[i] = NULL;
    }
    op->size = op_maxsize;
  }

  op->values[param] = malloc(sizeof(char) * (1 + strlen(value)));
  strcpy(op->values[param], value);

  return 1;
}

int acadp_get_op(AcadpOP** op) {
  if (*op == NULL) {
    *op = first_op;
  }
  else {
    *op = (*op)->next;
  }

  if (*op == NULL) {
    return 0;
  }
  else {
    return 1;
  }
}

int acadp_op_get_size(AcadpOP* op) {
  return op->size;
}

char* acadp_op_get_key(int i) {
  if (i < op_maxsize) {
    return op_names[i];
  }
  else {
    return NULL;
  }
}

char* acadp_op_get_value(AcadpOP* op, int i) {
  if (i < op->size) {
    return op->values[i];
  }
  else {
    return NULL;
  }
}

char* acadp_op_get_name(AcadpOP* op) {
  return op->name;
}

void acadp_init_op() {
  first_op = NULL;
  op_names = NULL;
  op_maxsize = 0;
}

void acadp_destroy_op() {
  AcadpOP* o1;
  AcadpOP* o2;

  o2 = first_op;
  while(o2 != NULL) {
    o1 = o2;
    o2 = o2->next;
    free(o1->name);
    free(o1->values);
    free(o1);
  }
  first_op = NULL;

  int i;
  for (i = 0; i < op_maxsize; i++) {
    free(op_names[i]);
  }
  free(op_names);
  op_names = NULL;
  op_maxsize = 0;
}
