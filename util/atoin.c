/*
 * Copyright (c) 1997, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: atoin.c,v $
 * Revision 1.4  2002-08-26 09:25:56  adam
 * Buffer overflow fix
 *
 * Revision 1.3  2000/02/29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.2  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.1  1997/09/04 07:52:27  adam
 * Moved atoi_n function to separate source file.
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <ctype.h>
#include <yaz/yaz-util.h>

int atoi_n (const char *buf, int len)
{
    int val = 0;

    while (--len >= 0)
    {
        if (isdigit (*buf))
            val = val*10 + (*buf - '0');
	buf++;
    }
    return val;
}

/*
  UCS-4 <- ISO-8859-1
  (unsigned long) ch = ch & 255;

  ISO-8859-1 -> UCS-4
  if (ch > 255)
      invalid sequence
  else
      ch = ch;

  UCS-4 -> UTF-8
  if (ch <= 0x7f)
      ch = ch;
  else if c(h <= 0x7ff)
  {
      str[0] = 0xc0 + (ch & (0xff < 6));
      str[1] = 0x80 + ch & 0x7ff;
  }
  else if (ch <= 0xffff)
  {



  }



*/ 
  
    
