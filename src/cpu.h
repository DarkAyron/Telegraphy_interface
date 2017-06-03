/*                  ___====-_  _-====___
              _--^^^#####//      \\#####^^^--_
           _-^##########// (    ) \\##########^-_
          -############//  |\^^/|  \\############-
        _/############//   (@::@)   \\############\_
       /#############((     \\//     ))#############\
      -###############\\    (oo)    //###############-
     -#################\\  / VV \  //#################-
    -###################\\/      \//###################-
   _#/|##########/\######(   /\   )######/\##########|\#_
   |/ |#/\#/\#/\/  \#/\##\  |  |  /##/\#/  \/\#/\#/\#| \|
   `  |/  V  V  `   V  \#\| |  | |/#/  V   '  V  V  \|  '
      `   `  `      `   / | |  | | \
                       (  | |  | |  )
                      __\ | |  | | /__
                     (vvv(VVV)(VVV)vvv)

cpu.h

Include cpu-specific header

Copyright (c) 2016 Ayron
*/

#ifndef CPU_H
#define CPU_H

#define F_CPU 12000000

#define __SAM3X8E__

/* Keys */
#define CKGR_MOR_KEY_VALUE	CKGR_MOR_KEY(0x37)

/* other defines */
#define ROM	__attribute__((section(".rodata")))
#define PACK	__attribute__((__packed__))

#include <chip.h>
#include <stdint.h>

extern uint32_t SystemCoreClock;

#endif
