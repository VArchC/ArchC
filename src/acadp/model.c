/*  ADeLe parsing structures for models
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

struct _acadp_model_arg {
  //char *name;
  //char *type;
  char *arg;
  AcadpModelArg* next;
};

struct _acadp_model {
  char *name;
  int line;
  int n_args;
  AcadpModelArg* first_arg;
  AcadpModelArg* last_arg;
  AcadpModel* next;
};

AcadpModel* first_model[ACADP_NUM_TYPE_MODELS];
AcadpModel* last_model[ACADP_NUM_TYPE_MODELS];

const char model_type_str[4][3] = {"DM", "IM", "PM", "EM"};

AcadpModel* acadp_find_model(char* name, AcadpModelType type, int n_args) {
  AcadpModel* p = NULL;

  p = first_model[type];
  while (p != NULL){
    if (!strcmp(p->name, name) && p->n_args == n_args) {
      break;
    }
    p = p->next;
  }
  return p;
}

AcadpModel* acadp_build_model(char* name) {
  AcadpModel* p;
  
  p = malloc(sizeof(AcadpModel));
  p->name = malloc(sizeof(char) * (1 + strlen(name)));
  strcpy(p->name, name);
  p->line = adf_line_num;
  p->n_args = 0;
  p->first_arg = NULL;
  p->last_arg = NULL;
  p->next = NULL;

  return p;
}

int acadp_add_model(AcadpModel *model, AcadpModelType type) {
  AcadpModel* p;

  if ( (p = acadp_find_model(model->name, type, model->n_args)) != NULL) {
    AcadpModelArg *a1 = NULL, *a2 = NULL;
    a1 = model->first_arg;
    while (a1 != NULL) {
      //free(a1->name);
      //free(a1->type);
      free(a1->arg);
      a2 = a1->next;
      free(a1);
      a1 = a2;
    }

    free(model->name);
    free(model);
    return p->line;
  }

  if (type == MODEL_DM) {
    acadp_add_arg_to_model(model, "source_t source");
    acadp_add_arg_to_model(model, "uint64_t& data");
    //acadp_add_arg_to_model(model, "source", "source_t");
    //acadp_add_arg_to_model(model, "data", "uint64_t&");
  }

  if (last_model[type] != NULL) {
    last_model[type]->next = model;
  }
  else {
    first_model[type] = model;
  }
  last_model[type] = model;

  return 0;
}

//int acadp_add_arg_to_model(AcadpModel* model, char* name, char* type) {
int acadp_add_arg_to_model(AcadpModel* model, char* arg) {
  AcadpModelArg* p;

  p = malloc(sizeof(AcadpModelArg));
  p->arg = malloc(sizeof(char) * (1 + strlen(arg)));
  strcpy(p->arg, arg);
  //p->name = malloc(sizeof(char) * (1 + strlen(name)));
  //strcpy(p->name, name);
  //p->type = malloc(sizeof(char) * (1 + strlen(type)));
  //strcpy(p->type, type);
  p->next = NULL;

  if (model->n_args > 0) {
    model->last_arg->next = p;
  }
  else {
    model->first_arg = p;
  }
  model->last_arg = p;
  model->n_args++;

  return 1;
}

AcadpModelInst* acadp_build_modelinst(char* name) {
  AcadpModelInst* p;
  
  p = malloc(sizeof(AcadpModelInst));
  p->name = malloc(sizeof(char) * (1 + strlen(name)));
  strcpy(p->name, name);
  p->n_params = 0;
  p->param_list = NULL;

  return p;
}

int acadp_add_param_to_modelinst(AcadpModelInst* model, char* param) {
  char **p;
  p = realloc(model->param_list, (model->n_params + 1)*sizeof(char*));
  if (p == NULL) {
    return 0;
  }
  model->param_list = p;

  model->param_list[model->n_params] = malloc(sizeof(char) * (1 + strlen(param)));
  strcpy(model->param_list[model->n_params], param);
  model->n_params++;
  return 1;
}

void acadp_delete_modelinst(AcadpModelInst* model) {
  if(model != NULL) {
    int i;
    free(model->name);
    for(i = 0; i < model->n_params; i++) {
      free(model->param_list[i]);
    }
    free(model->param_list);
    free(model);
  }
}

char* acadp_get_modelinst(AcadpModelInst* model) {
  if (model == NULL) {
    return NULL;
  }

  int nc = strlen(model->name)+3;
  int i;
  char* r = malloc(nc * sizeof(char));
  snprintf(r, nc, "%s(", model->name);
  if (model->n_params > 0) {
    nc += strlen(model->param_list[0]);
    r = realloc(r, nc * sizeof(char));
    strcat(r, model->param_list[0]);
  }
  for (i = 1; i < model->n_params; i++) {
    nc += strlen(model->param_list[i]) + 2;
    r = realloc(r, nc * sizeof(char));
    strcat(r, ", ");
    strcat(r, model->param_list[i]);
  }
  nc++;
  r = realloc(r, nc * sizeof(char));
  strcat(r, ")");
  return r;
}

AcadpModel* acadp_get_model_list(AcadpModelType type) {
  if (type >= ACADP_NUM_TYPE_MODELS) {
    return NULL;
  }

  return first_model[type];
}

void acadp_init_models() {
  int i;
  for (i = 0; i < ACADP_NUM_TYPE_MODELS; i++) {
    first_model[i] = NULL;
    last_model[i] = NULL;
  }
}

void acadp_destroy_models() {
  AcadpModel* m1 = NULL, *m2 = NULL;
  AcadpModelArg* a1 = NULL, *a2 = NULL;
  int i;

  for (i = 0; i < ACADP_NUM_TYPE_MODELS; i++) {
    m1 = first_model[i];
    while (m1 != NULL) {
      a1 = m1->first_arg;
      while (a1 != NULL) {
        //free(a1->name);
        //free(a1->type);
        free(a1->arg);
        a2 = a1->next;
        free(a1);
        a1 = a2;
      }

      free(m1->name);
      m2 = m1->next;
      free(m1);
      m1 = m2;
    }
  }

  acadp_init_models();
}

void acadp_write_model_signature(FILE *fp, AcadpModelType t, AcadpModel* m) {
  AcadpModelArg* a;

  fprintf(fp, "%s(", m->name);
  a = m->first_arg;
  while (a != NULL) {
    //fprintf(fp, "%s %s", a->type, a->name);
    fprintf(fp, "%s", a->arg);
    if (a->next != NULL) {
      fprintf(fp, ",");
    }
    a = a->next;
  }

  fprintf(fp, ")");
}

void acadp_write_models_template(FILE *fp, AcadpModelType t) {
  AcadpModel* m;
  m = first_model[t];

  while (m != NULL) {
    switch (t) {
      case MODEL_DM: fprintf(fp, "VARCHC_MODEL_DM "); break;
      case MODEL_IM: fprintf(fp, "VARCHC_MODEL_IM "); break;
      case MODEL_PM: fprintf(fp, "VARCHC_MODEL_PM "); break;
      case MODEL_EM: fprintf(fp, "VARCHC_MODEL_EM "); break;
    }
    acadp_write_model_signature(fp, t, m);
    fprintf(fp, "{\n\t// Implementation of %s\n\n}\n\n", m->name);
    m = m->next;
  }
}

void acadp_write_models_header(FILE *fp, AcadpModelType t, const char *indent) {
  AcadpModel* m;
  m = first_model[t];

  while (m != NULL) {
    switch (t) {
      case MODEL_DM: fprintf(fp, "%sinline __attribute__((always_inline)) void ", indent); break;
      case MODEL_IM: fprintf(fp, "%sinline __attribute__((always_inline)) void ", indent); break;
      case MODEL_PM: fprintf(fp, "%sinline __attribute__((always_inline)) bool ", indent); break;
      case MODEL_EM: fprintf(fp, "%sinline __attribute__((always_inline)) double ", indent); break;
    }
    acadp_write_model_signature(fp, t, m);
    fprintf(fp, ";\n");
    m = m->next;
  }
}
