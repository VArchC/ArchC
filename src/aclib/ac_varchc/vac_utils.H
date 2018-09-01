/*  VArchC general utilities
    Copyright (C) 2018  Isaías Felzmann
    Visit http://varchc.github.io/ for more information.

    This module is part of ArchC
    Copyright (C) 2002-2018  The ArchC Team

    Please see license file distributed with this file.
    
    http://www.archc.org/
*/

#ifndef _VAC_UTILS_H_
#define _VAC_UTILS_H_

#include <cstdint>

namespace VArchC {
  namespace Storage {
    void setAsControl(char *exec, uint32_t& vac_ctl_addr, bool& vac_ctl);
  }
}

#endif
