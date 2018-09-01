/*  VArchC models header
    Copyright (C) 2018  Isaías Felzmann
    Visit http://varchc.github.io/ for more information.

    This module is part of ArchC
    Copyright (C) 2002-2018  The ArchC Team

    Please see license file distributed with this file.
    
    http://www.archc.org/
*/

#ifndef _VAC_MODELS_H_
#define _VAC_MODELS_H_

#define VARCHC_MODEL_DM inline __attribute__((always_inline)) void VArchC::Models::Data::
#define VARCHC_MODEL_IM inline __attribute__((always_inline)) void VArchC::Models::Instruction::
#define VARCHC_MODEL_PM inline __attribute__((always_inline)) bool VArchC::Models::Probability::
#define VARCHC_MODEL_EM inline __attribute__((always_inline)) double VArchC::Models::Energy::

#endif
