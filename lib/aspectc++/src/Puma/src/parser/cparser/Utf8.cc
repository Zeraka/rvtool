// This file is part of PUMA.
// Copyright (C) 1999-2003  The PUMA developer team.
//                                                                
// This program is free software;  you can redistribute it and/or 
// modify it under the terms of the GNU General Public License as 
// published by the Free Software Foundation; either version 2 of 
// the License, or (at your option) any later version.            
//                                                                
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
// GNU General Public License for more details.                   
//                                                                
// You should have received a copy of the GNU General Public      
// License along with this program; if not, write to the Free     
// Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
// MA  02111-1307  USA                                            

#include "Puma/Utf8.h"

namespace Puma {

void utf8_encode (LONG_LONG i, char *&p, long int &ct) {
  if (i <= 0x7f) {
    *p++ = (char)i;
    ct += 1;
  }
  else if (i <= 0x7ff) {
    *p++ = (0xc0 | ((i & 0x700) >> 6) | ((i & 0xc0) >> 6));
    *p++ = (0x80 | (i & 0x3f));
    ct += 2;
  }
  else if (i <= 0xffff) {
    *p++ = (0xe0 | ((i & 0xf000) >> 12));
    *p++ = (0x80 | ((i & 0x0f00) >> 6)  | ((i & 0xc0) >> 6));
    *p++ = (0x80 | (i & 0x3f));
    ct += 3;
  }
  else if (i <= 0x10ffff) {
    *p++ = (0xf0 | ((i & 0x1c0000) >> 18));
    *p++ = (0x80 | ((i & 0x3f000) >> 12));
    *p++ = (0x80 | ((i & 0xfc0) >> 6));
    *p++ = (0x80 | (i & 0x3f));
    ct += 4;
  }
}

LONG_LONG utf8_scan (const char *&s, bool eight_digits) {
  int digits = (eight_digits ? 8 : 4);
  LONG_LONG v = 0;
  while (*s && digits > 0) {
    if (*s >= '0' && *s <= '9')
      v = (v<<4) + *s++ - '0';
    else if (*s >= 'a' && *s <= 'f')
      v = (v<<4) + *s++ - 'a' + 10;
    else if (*s >= 'A' && *s <= 'F')
      v = (v<<4) + *s++ - 'A' + 10;
    else {
      v = -1; // This is an error => check with (ucs == true && result == -1)
      break;
    }
    digits--;
  }
  return v;
}

} // namespace Puma
