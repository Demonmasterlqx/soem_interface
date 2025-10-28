/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * Headerfile for oshw.c
 */

#ifndef _oshw_
#define _oshw_

#ifdef __cplusplus
extern "C"
{
#endif

#include <kern.h>
#include "soem_rsl/soem_rsl/ethercattype.h"
#include "soem_rsl/oshw/linux/nicdrv.h"
#include "soem_rsl/soem_rsl/ethercatmain.h"


uint16 oshw_htons(uint16 host);
uint16 oshw_ntohs(uint16 network);

ec_adaptert * oshw_find_adapters(void);
void oshw_free_adapters(ec_adaptert * adapter);

#ifdef __cplusplus
}
#endif

#endif
