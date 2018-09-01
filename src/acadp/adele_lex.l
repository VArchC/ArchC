/*  ADeLe Language Lexer for GNU Flex
    Copyright (C) 2018  Isaías Felzmann
    Visit http://varchc.github.io/ for more information.

    This module is part of ArchC
    Copyright (C) 2002-2018  The ArchC Team

    Please see license file distributed with this file.
    
    http://www.archc.org/
*/

%{
#include <stdlib.h>
#include "adele_grammar.h"
extern int adf_line_num;
int c_paren_depth;
%}

ID [a-zA-Z][_a-zA-Z0-9]*

%s SLCOMMENT
%s BLCOMMENT
%x C_EXP_WAIT
%s C_EXP
%x C_SSTR
%x C_DSTR

%option noyywrap
%option nounput
%option noinput

%%

 int return_state = INITIAL;
 /* Skip comments */

"//" {
  return_state = YY_START;
  BEGIN(SLCOMMENT);
}

"/*" {
  if(YY_START == BLCOMMENT){
    fprintf(stderr, "ADeLe Lexer - WARNING: Line %d: '/*' inside a block comment.\n", adf_line_num);
  }
  else{
    return_state = YY_START;
    BEGIN(BLCOMMENT);
  }
}

<SLCOMMENT>"\n" {
  adf_line_num++;
  BEGIN(return_state);
}

<SLCOMMENT>[^"\n"]* {}

<BLCOMMENT>"*/" {
  BEGIN(return_state);
}

<BLCOMMENT>. {}

<C_EXP_WAIT>{ID}+ {
  adflval.text = (char*) malloc(sizeof(char) * (strlen(yytext) + 1));
  strcpy(adflval.text, yytext);
  return ID;
}

<C_EXP_WAIT>"=" {
  return EQ;
}

<C_EXP_WAIT>"(" {
  BEGIN(C_EXP);
  c_paren_depth = 0;
  return LPAREN;
}

<C_EXP>[^\(\)\[\]{}<>\"',]* {
  adflval.text = (char*) malloc(sizeof(char) * (strlen(yytext) + 1));
  strcpy(adflval.text, yytext);
  return C_ID;
}

<C_EXP>"'" {
  BEGIN(C_SSTR);
}

<C_EXP>"\"" {
  BEGIN(C_DSTR);
}

<C_SSTR>([^\\']|\\'|\\)* {
  adflval.text = (char*) malloc(sizeof(char) * (strlen(yytext) + 1));
  strcpy(adflval.text, yytext);
  return SSTR;
}

<C_SSTR>"'" {
  BEGIN(C_EXP);
}

<C_DSTR>([^\\\"]|\\\"|\\)* {
  adflval.text = (char*) malloc(sizeof(char) * (strlen(yytext) + 1));
  strcpy(adflval.text, yytext);
  return DSTR;
}

<C_DSTR>"\"" {
  BEGIN(C_EXP);
}

 /* General symbols */

"\n" {
  adf_line_num++;
}

"(" {
  if (c_paren_depth != -1) {
    c_paren_depth++;
  }
  return LPAREN;
}

")" {
  if (c_paren_depth != -1) {
    if (c_paren_depth == 0) {
      c_paren_depth = -1;
      BEGIN(INITIAL);
    }
    else {
      c_paren_depth--;
    }
  }
  return RPAREN;
}

"[" {
  return LBRACK;
}

"]" {
  return RBRACK;
}

"{" {
  return LBRACE;
}

"}" {
  return RBRACE;
}

"<" {
  return LANG;
}

">" {
  return RANG;
}
"," {
  return COMMA;
}

";" {
  return SEMICOLON;
}

"=" {
  return EQ;
}

 /* ADeLe keywords */

<INITIAL>"DM" {
  BEGIN(C_EXP_WAIT);
  return DM;
}

<INITIAL>"IM" {
  BEGIN(C_EXP_WAIT);
  return IM;
}

<INITIAL>"PM" {
  BEGIN(C_EXP_WAIT);
  return PM;
}

<INITIAL>"EM" {
  BEGIN(C_EXP_WAIT);
  return EM;
}

<INITIAL>"OP" {
  return OP;
}

<INITIAL>"approximation" {
  return_state = YY_START;
  BEGIN(INITIAL);
  return APPROXIMATION;
}

<INITIAL>"group" {
  return GROUP;
}

<INITIAL>"initial" {
  return DEFAULT;
}

<INITIAL>"probability" {
  BEGIN(C_EXP_WAIT);
  return PROBABILITY;
}

<INITIAL>"regbank_read" {
  BEGIN(C_EXP_WAIT);
  return RGB_RD;
}

<INITIAL>"regbank_write" {
  BEGIN(C_EXP_WAIT);
  return RGB_WR;
}

<INITIAL>"mem_read" {
  BEGIN(C_EXP_WAIT);
  return MEM_RD;
}

<INITIAL>"mem_write" {
  BEGIN(C_EXP_WAIT);
  return MEM_WR;
}

<INITIAL>"reg_read" {
  BEGIN(C_EXP_WAIT);
  return REG_RD;
}

<INITIAL>"reg_write" {
  BEGIN(C_EXP_WAIT);
  return REG_WR;
}

<INITIAL>"pre_behavior" {
  BEGIN(C_EXP_WAIT);
  return PRE_BHV;
}

<INITIAL>"post_behavior" {
  BEGIN(C_EXP_WAIT);
  return PST_BHV;
}

<INITIAL>"alt_behavior" {
  BEGIN(C_EXP_WAIT);
  return ALT_BHV;
}

<INITIAL>"instruction" {
  return INSTRUCTION;
}

<INITIAL>"energy" {
  BEGIN(C_EXP_WAIT);
  return ENERGY;
}

<INITIAL>"parameters" {
  return PARAMETERS;
}
 /* Values and identifiers */

<INITIAL>"-"?[0-9]+ {
  adflval.text = (char*) malloc(sizeof(char) * (strlen(yytext) + 1));
  strcpy(adflval.text, yytext);
  return I_CONSTANT;
}

<INITIAL>"-"?[0-9]+"."[0-9]* {
  adflval.text = (char*) malloc(sizeof(char) * (strlen(yytext) + 1));
  strcpy(adflval.text, yytext);
  return F_CONSTANT;
}

<INITIAL>{ID}+ {
  adflval.text = (char*) malloc(sizeof(char) * (strlen(yytext) + 1));
  strcpy(adflval.text, yytext);
  return ID;
}

<INITIAL>{ID}*("."{ID}*)+ {
  adflval.text = (char*) malloc(sizeof(char) * (strlen(yytext) + 1));
  strcpy(adflval.text, yytext);
  return INSTR;
}

 /* Skip blanks and EOF handling */

<INITIAL,C_EXP_WAIT>[ \t]+ {}

<INITIAL><<EOF>> {
  yyterminate();
}

. {
  fprintf(stderr, "ADeLe Lexer - ERROR: Line %d: Unexpected '%s'.\n", adf_line_num, yytext);
  yyterminate();
}

%%

