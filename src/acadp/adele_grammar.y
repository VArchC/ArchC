/*  ADeLe Language Grammar for GNU Bison
    Copyright (C) 2018  Isaías Felzmann
    Visit http://varchc.github.io/ for more information.

    This module is part of ArchC
    Copyright (C) 2002-2018  The ArchC Team

    Please see license file distributed with this file.
    
    http://www.archc.org/
*/

%{

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "acadp.h"

extern FILE* adfin;
extern int adflex();

static int adf_parse_error = 0;
int adf_line_num;

AcadpModel* current_model = NULL;
AcadpOP* current_op = NULL;
AcadpApprox* current_approx = NULL;
AcadpGroup* current_group = NULL;
AcadpModelInst* current_modelinst = NULL;

static void yyerror(const char* format, ...){
  va_list args;
  va_start(args, format);
  adf_parse_error++;
  fprintf(stderr, "ADeLe Parser - ERROR: Line %d: ", adf_line_num);
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  va_end(args);
}

static void yywarn(const char* format, ...){
  va_list args;
  va_start(args, format);
  fprintf(stderr, "ADeLe Parser - WARNING: Line %d: ", adf_line_num);
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  va_end(args);
}

%}

/* General tokens */
//%token <i_const> I_CONSTANT
//%token <f_const> F_CONSTANT
%token <text> I_CONSTANT
%token <text> F_CONSTANT
%token <text> ID
%token <text> INSTR
%token <text> LPAREN "("
%token <text> RPAREN ")"
%token <text> LBRACK "["
%token <text> RBRACK "]"
%token <text> LBRACE "{"
%token <text> RBRACE "}"
%token <text> LANG "<"
%token <text> RANG ">"
%token <text> COMMA ","
%token <text> SEMICOLON ";"
%token <text> EQ "="

/* Simple C expression */
%type <text> c_list
%type <text> c_expression
%type <text> c_expression_single
%token <text> C_ID
%token <text> DSTR
%token <text> SSTR

/* Fundamental type */
%type <text> adf_list

/* Modeling declaration */
%type <text> model_decl
%type <text> model_arg_decl_list
%token <text> DM
%token <text> IM
%token <text> PM
%token <text> EM

/* Operating parameters declaration */
%type <text> op_decl
%type <text> op_list
%type <text> op_single
%token <text> OP

/* Modeling instantiation */
%type <text> defaults_single
%type <text> model_arg_inst_list
%token <text> DEFAULT
%token <text> ENERGY
%token <text> PARAMETERS
%token <text> RGB_RD
%token <text> RGB_WR
%token <text> MEM_RD
%token <text> MEM_WR
%token <text> REG_RD
%token <text> REG_WR
%token <text> PRE_BHV
%token <text> PST_BHV
%token <text> ALT_BHV
%token <text> GROUP
%token <text> INSTRUCTION
%token <text> APPROXIMATION

/* Groups */
%type <text> group_list
%type <text> group_instr_list

/* Approximations */
%type <text> approximation_list
%type <text> approximation_impl_list
%token <text> PROBABILITY

%start initial

%union
{
  char* text;
}

%%

initial
  : adf_list {
    if (adf_parse_error > 0) {
      YYERROR;
    }
  };

adf_list
  : adf_list model_decl {}
  | adf_list op_decl {}
  | adf_list defaults_single {}
  | adf_list GROUP ID LBRACE {
    int prev;
    if ( (prev = acadp_new_group($3, &current_group)) != 0) {
      yywarn("Group %s previously declared on line %d will be overloaded.", $3, prev);
    }
  } group_list RBRACE {
    current_group = NULL;
  }
  | adf_list APPROXIMATION ID LBRACE {
    int prev;
    if ( (prev = acadp_new_approx($3, &current_approx)) != 0) {
      yywarn("Approximation %s previously declared on line %d will be overloaded.", $3, prev);
    }
  } approximation_list RBRACE {
    acadp_close_approx(&current_approx);  
  }
  | /* empty */ {}
  ;

/* Simple C expression */
c_list
  : c_list COMMA c_expression {
    int nc = strlen($1) + strlen($3) + 1 + 1;
    char *r = malloc(nc * sizeof(char));
    snprintf(r, nc, "%s,%s", $1, $3);
    $$ = r;
    }
  | c_expression {$$ = $1;}
  | /* empty */ {$$ = "";}
  ;

c_expression
  : c_expression c_expression_single {
    int nc = strlen($1) + strlen($2) + 1;
    char *r = malloc(nc * sizeof(char));
    snprintf(r, nc, "%s%s", $1, $2);
    $$ = r;
    }
  | c_expression_single {$$ = $1;}
  ;

c_expression_single
  : C_ID {$$ = $1;}
  | LPAREN c_list RPAREN {
    int nc = strlen($2) + 2 + 1;
    char *r = malloc(nc * sizeof(char));
    snprintf(r, nc, "(%s)", $2);
    $$ = r;
    }
  | LBRACK c_list RBRACK {
    int nc = strlen($2) + 2 + 1;
    char *r = malloc(nc * sizeof(char));
    snprintf(r, nc, "[%s]", $2);
    $$ = r;
    }
  | LBRACE c_list RBRACE {
    int nc = strlen($2) + 2 + 1;
    char *r = malloc(nc * sizeof(char));
    snprintf(r, nc, "{%s}", $2);
    $$ = r;
    }
  | LANG c_list RANG {
    int nc = strlen($2) + 2 + 1;
    char *r = malloc(nc * sizeof(char));
    snprintf(r, nc, "<%s>", $2);
    $$ = r;
    }
  | DSTR {
    int nc = strlen($1) + 2 + 1;
    char *r = malloc(nc * sizeof(char));
    snprintf(r, nc, "\"%s\"", $1);
    $$ = r;
    }
  | SSTR {
    int nc = strlen($1) + 2 + 1;
    char *r = malloc(nc * sizeof(char));
    snprintf(r, nc, "'%s'", $1);
    $$ = r;
    }
  ;

/* Modeling declaration */
model_decl
  : DM ID LPAREN {
    current_model = acadp_build_model($2);
  } model_arg_decl_list RPAREN SEMICOLON {
    int prev;
    if ( (prev = acadp_add_model(current_model, MODEL_DM)) != 0) {
      yyerror("Redeclaration of Data Modifier %s, previously declared on line %d.", $2, prev);
    }
  }
  | IM ID LPAREN {
    current_model = acadp_build_model($2);
  } model_arg_decl_list RPAREN SEMICOLON {
    int prev;
    if ( (prev = acadp_add_model(current_model, MODEL_IM)) != 0) {
      yyerror("Redeclaration of Instruction Modifier %s, previously declared on line %d.", $2, prev);
    }
  }
  | PM ID LPAREN {
    current_model = acadp_build_model($2);
  } model_arg_decl_list RPAREN SEMICOLON {
    int prev;
    if ( (prev = acadp_add_model(current_model, MODEL_PM)) != 0) {
      yyerror("Redeclaration of Probability Model %s, previously declared on line %d.", $2, prev);
    }
  }
  | EM ID LPAREN {
    current_model = acadp_build_model($2);
  } model_arg_decl_list RPAREN SEMICOLON {
    int prev;
    if ( (prev = acadp_add_model(current_model, MODEL_EM)) != 0) {
      yyerror("Redeclaration of Energy Model %s, previously declared on line %d.", $2, prev);
    }
  }
  ;

model_arg_decl_list
  : model_arg_decl_list COMMA c_expression {
    acadp_add_arg_to_model(current_model, $3);
  }
  | c_expression {
    acadp_add_arg_to_model(current_model, $1);
  }
  | /* empty */ {}
  ;

/* Operating parameters declaration */

op_decl
  : OP ID EQ LBRACE {
    int prev;
    if ( (prev = acadp_new_op($2, &current_op)) != 0) {
      yywarn("Operating parameters %s previously declared on line %d will be overloaded.", $2, prev);
    }
  } op_list RBRACE SEMICOLON {
    current_op = NULL;
  }
  ;

op_list
  : op_list COMMA op_single {}
  | op_single {}
  ;

op_single
  : ID EQ I_CONSTANT {
    acadp_add_param_to_op($2, $3, current_op);
  }
  | ID EQ F_CONSTANT {
    acadp_add_param_to_op($2, $3, current_op);
  }
  | /* empty */ {}
  ;

/* Modeling instantiation */

defaults_single
  : ENERGY EQ ID LPAREN {
    current_modelinst = acadp_build_modelinst($3);
  } model_arg_inst_list RPAREN SEMICOLON {
    if (current_approx == NULL && current_group == NULL) {
      if (!acadp_set_default_energy(current_modelinst)) {
        yyerror("Energy Model %s has no declared candidate.", $3);
      }
    }
    else if(current_approx != NULL) {
      if (!acadp_approx_add_energy(current_modelinst, current_approx)) {
        yyerror("Energy Model %s has no declared candidate.", $3);
      }
    }
    else {
      if (!acadp_group_add_energy(current_modelinst, current_group)) {
        yyerror("Energy Model %s has no declared candidate.", $3);
      }
    }
  }
  | PARAMETERS EQ ID SEMICOLON {
    if (current_approx == NULL && current_group == NULL) {
      if (!acadp_set_default_op($3)) {
        yyerror("Operating Parameters %s has no declared candidate.", $3);
      }
    }
    else if(current_approx != NULL) {
      if (!acadp_approx_add_op($3, current_approx)) {
        yyerror("Operating Parameters %s has no declared candidate.", $3);
      }
    }
    else {
      if (!acadp_group_add_op($3, current_group)) {
        yyerror("Operating Parameters %s has no declared candidate.", $3);
      }
    }
  }
  ;

model_arg_inst_list
  : model_arg_inst_list COMMA c_expression {
    acadp_add_param_to_modelinst(current_modelinst, $3);
  }
  | c_expression {
    acadp_add_param_to_modelinst(current_modelinst, $1);
  }
  | /* empty */ {}
  ;

/* Groups */
group_list
  : group_list defaults_single {}
  | group_list INSTRUCTION EQ LBRACE group_instr_list RBRACE SEMICOLON {}
  | /* empty */ {}
  ;

group_instr_list
  : group_instr_list COMMA INSTR {
    int r = acadp_add_instruction_to_group($3, current_group);
    if (r == 0) {
      yywarn("Instruction %s was already in group %s.", $3, acadp_group_get_name(current_group));
    }
    else if (r == -1) {
      yyerror("Instruction %s does not exist in current ArchC architecture.", $3);
    }
  }
  | INSTR {
    int r = acadp_add_instruction_to_group($1, current_group);
    if (r == 0) {
      yywarn("Instruction %s was already in group %s.", $1, acadp_group_get_name(current_group));
    }
    else if (r == -1) {
      yyerror("Instruction %s does not exist in current ArchC architecture.", $1);
    }
  }
  | group_instr_list COMMA ID {
    int r = acadp_add_instruction_to_group($3, current_group);
    if (r == 0) {
      yywarn("Instruction %s was already in group %s.", $3, acadp_group_get_name(current_group));
    }
    else if (r == -1) {
      yyerror("Instruction %s does not exist in current ArchC architecture.", $3);
    }
  }
  | ID {
    int r = acadp_add_instruction_to_group($1, current_group);
    if (r == 0) {
      yywarn("Instruction %s was already in group %s.", $1, acadp_group_get_name(current_group));
    }
    else if (r == -1) {
      yyerror("Instruction %s does not exist in current ArchC architecture.", $1);
    }
  }
  ;

/* Approximations */
approximation_list
  : approximation_list DEFAULT EQ ID SEMICOLON {
    if (strlen($4) == 2 && strncmp($4, "on", 2) == 0) {
      acadp_approx_set_default_st(current_approx, ST_ON);
    }
    else if (strlen($4) == 3 && strncmp($4, "off", 3) == 0) {
      acadp_approx_set_default_st(current_approx, ST_OFF);
    }
    else {
      yyerror("Unrecognized initial state %s. Possible states are \"on\" and \"off\".", $4);
    }
  }
  | approximation_list GROUP ID LBRACE {
    int prev;
    if ( (prev = acadp_add_group_to_approx($3, &current_approx)) > 0) {
      yywarn("Group %s previously on line %d will be overloaded.", $3, prev);
    }
    else if (prev == -1) {
      yyerror("Group %s has no declared candidate.", $3);
    }
  } approximation_impl_list RBRACE {
    acadp_close_approx(&current_approx);
  }
  | approximation_list INSTRUCTION INSTR LBRACE {
    int prev;
    if ( (prev = acadp_add_instruction_to_approx($3, &current_approx)) > 0) {
      yywarn("Instruction %s previously on line %d will be overloaded.", $3, prev);
    }
    else if (prev == -1) {
      yyerror("Instruction %s does not exist in current ArchC architecture.", $3);
    }
  } approximation_impl_list RBRACE {
    acadp_close_approx(&current_approx);
  }
  | approximation_list INSTRUCTION ID LBRACE {
    int prev;
    if ( (prev = acadp_add_instruction_to_approx($3, &current_approx)) > 0) {
      yywarn("Instruction %s previously on line %d will be overloaded.", $3, prev);
    }
    else if (prev == -1) {
      yyerror("Instruction %s does not exist in current ArchC architecture.", $3);
    }
  } approximation_impl_list RBRACE {
    acadp_close_approx(&current_approx);
  }
  | approximation_list approximation_impl_list_member {}
  | /* empty */ {}
  ;

approximation_impl_list
  : approximation_impl_list approximation_impl_list_member {}
  | /* empty */ {}
  ;

approximation_impl_list_member
  : defaults_single {}
  | PROBABILITY EQ ID LPAREN {
    current_modelinst = acadp_build_modelinst($4);
  } model_arg_inst_list RPAREN SEMICOLON {
    if (!acadp_approx_add_probability(current_modelinst, current_approx)) {
      yyerror("Probability Model %s has no declared candidate.", $4);
    }
  }
  | RGB_RD EQ ID LPAREN {
    current_modelinst = acadp_build_modelinst($4);
  } model_arg_inst_list RPAREN SEMICOLON {
    if (!acadp_approx_add_regbank_read(current_modelinst, current_approx)) {
      yyerror("Data Modifier %s has no declared candidate.", $4);
    }
  }
  | RGB_WR EQ ID LPAREN {
    current_modelinst = acadp_build_modelinst($4);
  } model_arg_inst_list RPAREN SEMICOLON {
    if (!acadp_approx_add_regbank_write(current_modelinst, current_approx)) {
      yyerror("Data Modifier %s has no declared candidate.", $4);
    }
  }
  | MEM_RD EQ ID LPAREN {
    current_modelinst = acadp_build_modelinst($4);
  } model_arg_inst_list RPAREN SEMICOLON {
    if (!acadp_approx_add_mem_read(current_modelinst, current_approx)) {
      yyerror("Data Modifier %s has no declared candidate.", $4);
    }
  }
  | MEM_WR EQ ID LPAREN {
    current_modelinst = acadp_build_modelinst($4);
  } model_arg_inst_list RPAREN SEMICOLON {
    if (!acadp_approx_add_mem_write(current_modelinst, current_approx)) {
      yyerror("Data Modifier %s has no declared candidate.", $4);
    }
  }
  | REG_RD EQ ID LPAREN {
    current_modelinst = acadp_build_modelinst($4);
  } model_arg_inst_list RPAREN SEMICOLON {
    if (!acadp_approx_add_reg_read(current_modelinst, current_approx)) {
      yyerror("Data Modifier %s has no declared candidate.", $4);
    }
  }
  | REG_WR EQ ID LPAREN {
    current_modelinst = acadp_build_modelinst($4);
  } model_arg_inst_list RPAREN SEMICOLON {
    if (!acadp_approx_add_reg_write(current_modelinst, current_approx)) {
      yyerror("Data Modifier %s has no declared candidate.", $4);
    }
  }
  | PRE_BHV EQ ID LPAREN {
    current_modelinst = acadp_build_modelinst($4);
  } model_arg_inst_list RPAREN SEMICOLON {
    if (!acadp_approx_add_pre_behavior(current_modelinst, current_approx)) {
      yyerror("Instruction Modifier %s has no declared candidate.", $4);
    }
  }
  | PST_BHV EQ ID LPAREN {
    current_modelinst = acadp_build_modelinst($4);
  } model_arg_inst_list RPAREN SEMICOLON {
    if (!acadp_approx_add_post_behavior(current_modelinst, current_approx)) {
      yyerror("Instruction Modifier %s has no declared candidate.", $4);
    }
  }
  | ALT_BHV EQ ID LPAREN {
    current_modelinst = acadp_build_modelinst($4);
  } model_arg_inst_list RPAREN SEMICOLON {
    if (!acadp_approx_add_alt_behavior(current_modelinst, current_approx)) {
      yyerror("Instruction Modifier %s has no declared candidate.", $4);
    }
  }
  ;

%%
