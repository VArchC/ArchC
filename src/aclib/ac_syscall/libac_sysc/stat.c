/**
 * @file      stat.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:20 -0300
 *
 * @brief     
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef ARM
#define stat _stat
#endif

#include <errno.h>
#include <ac_syscall_codes.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int stat(const char *path, struct stat *buf)
{
  int res = ac_syscall_wrapper(__NR_stat,path,buf);
//  if (!mode) {
//    errno = ac_syscall_geterrno();
//    return -1;
//  }
//  else {
//    //correct values:
//    buf->st_mode = mode;
//    //dummy values:
//    buf->st_size = 1;
//    buf->st_uid = 1;
//    buf->st_gid = 1;
//    return 0;
//  }
   return res;
}
