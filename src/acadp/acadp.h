/*  ADeLe Parser interface with ArchC parser
    Copyright (C) 2018  Isaías Felzmann
    Visit http://varchc.github.io/ for more information.

    This module is part of ArchC
    Copyright (C) 2002-2018  The ArchC Team

    Please see license file distributed with this file.
    
    http://www.archc.org/
*/

#ifndef _ACADP_H_
#define _ACADP_H_
#include <stdio.h>

#define ACADP_NUM_TYPE_MODELS 4

extern int adf_line_num;
int acadp_load(char *file);
void acadp_unload();
int acadp_run();

/* Modeling */
typedef struct _acadp_model AcadpModel;
typedef struct _acadp_model_arg AcadpModelArg;
struct _acadp_model_inst {
  AcadpModel* model;
  char *name;
  int n_params;
  char** param_list;
};
typedef struct _acadp_model_inst AcadpModelInst;
typedef enum {MODEL_DM = 0, MODEL_IM = 1, MODEL_PM = 2, MODEL_EM = 3} AcadpModelType;

AcadpModel* acadp_find_model(char* name, AcadpModelType type, int n_args);
AcadpModel* acadp_build_model(char* name);
int acadp_add_model(AcadpModel *model, AcadpModelType type);
//int acadp_add_arg_to_model(AcadpModel *model, char* name, char* type);
int acadp_add_arg_to_model(AcadpModel *model, char* arg);
AcadpModelInst* acadp_build_modelinst(char* name);
int acadp_add_param_to_modelinst(AcadpModelInst* model, char* param);
void acadp_delete_modelinst(AcadpModelInst* model);
char* acadp_get_modelinst(AcadpModelInst* model);
AcadpModel* acadp_get_model_list(AcadpModelType type);
void acadp_init_models();
void acadp_destroy_models();
void acadp_write_models_template(FILE *fp, AcadpModelType t);
void acadp_write_models_header(FILE *fp, AcadpModelType t, const char *indent);

/* Operating Parameters */
typedef struct _acadp_op AcadpOP;

AcadpOP* acadp_find_op(char* name);
int acadp_new_op(char *name, AcadpOP** op);
int acadp_add_param_to_op(char* name, char* value, AcadpOP* op);
int acadp_get_op(AcadpOP** op);
int acadp_op_get_size(AcadpOP* op);
char* acadp_op_get_key(int i);
char* acadp_op_get_value(AcadpOP* op, int i);
char* acadp_op_get_name(AcadpOP* op);
void acadp_init_op();
void acadp_destroy_op();

/* Approximations */
typedef struct _acadp_approx AcadpApprox;
typedef struct _acadp_model_param AcadpModelParam;
typedef enum{ST_OFF, ST_ON} AcadpDefaultState;

AcadpApprox* acadp_find_approx(char* name, AcadpApprox* level);
int acadp_new_approx(char *name, AcadpApprox **approx);
int acadp_add_group_to_approx(char *name, AcadpApprox **approx);
int acadp_add_instruction_to_approx(char* name, AcadpApprox** approx);
void acadp_close_approx(AcadpApprox** approx);
void acadp_approx_set_default_st(AcadpApprox* approx, AcadpDefaultState st);
int acadp_approx_add_energy(AcadpModelInst* model, AcadpApprox* approx);
int acadp_approx_add_op(char* op_name, AcadpApprox* approx);
int acadp_approx_add_probability(AcadpModelInst* model, AcadpApprox* approx);
int acadp_approx_add_regbank_read(AcadpModelInst* model, AcadpApprox* approx);
int acadp_approx_add_regbank_write(AcadpModelInst* model, AcadpApprox* approx);
int acadp_approx_add_mem_read(AcadpModelInst* model, AcadpApprox* approx);
int acadp_approx_add_mem_write(AcadpModelInst* model, AcadpApprox* approx);
int acadp_approx_add_reg_read(AcadpModelInst* model, AcadpApprox* approx);
int acadp_approx_add_reg_write(AcadpModelInst* model, AcadpApprox* approx);
int acadp_approx_add_pre_behavior(AcadpModelInst* model, AcadpApprox* approx);
int acadp_approx_add_post_behavior(AcadpModelInst* model, AcadpApprox* approx);
int acadp_approx_add_alt_behavior(AcadpModelInst* model, AcadpApprox* approx);
int acadp_get_approx(AcadpApprox** approx);
char* acadp_approx_get_name(AcadpApprox* approx);
AcadpDefaultState acadp_approx_get_default_st(AcadpApprox* approx);
char* acadp_approx_get_energy(AcadpApprox* approx, unsigned int instr);
AcadpOP* acadp_approx_get_op(AcadpApprox* approx, unsigned int instr);
char* acadp_approx_get_probability(AcadpApprox* approx, unsigned int instr);
char* acadp_approx_get_regbank_read(AcadpApprox* approx, unsigned int instr);
char* acadp_approx_get_regbank_write(AcadpApprox* approx, unsigned int instr);
char* acadp_approx_get_mem_read(AcadpApprox* approx, unsigned int instr);
char* acadp_approx_get_mem_write(AcadpApprox* approx, unsigned int instr);
char* acadp_approx_get_reg_read(AcadpApprox* approx, unsigned int instr);
char* acadp_approx_get_reg_write(AcadpApprox* approx, unsigned int instr);
char* acadp_approx_get_pre_behavior(AcadpApprox* approx, unsigned int instr);
char* acadp_approx_get_post_behavior(AcadpApprox* approx, unsigned int instr);
char* acadp_approx_get_alt_behavior(AcadpApprox* approx, unsigned int instr);
void acadp_init_approx();
void acadp_destroy_approx();

/* Groups */
typedef struct _acadp_group AcadpGroup;

AcadpGroup* acadp_find_group(char* name);
int acadp_new_group(char *name, AcadpGroup** group);
int acadp_add_instruction_to_group(char* name, AcadpGroup* group);
int acadp_group_add_energy(AcadpModelInst* model, AcadpGroup* group);
int acadp_group_add_op(char* op_name, AcadpGroup* group);
int acadp_get_group(AcadpGroup** group);
char* acadp_group_get_name(AcadpGroup* group);
char* acadp_group_get_energy(AcadpGroup* group);
AcadpOP* acadp_group_get_op(AcadpGroup* group);
int acadp_group_contains_instruction(unsigned int instr, AcadpGroup* group);
void acadp_init_group();
void acadp_destroy_group();

/* Defaults */
int acadp_set_default_energy(AcadpModelInst* model);
int acadp_set_default_op(char* op_name);
char* acadp_get_default_energy();
AcadpOP* acadp_get_default_op();

#endif /* _ACADP_H_ */
