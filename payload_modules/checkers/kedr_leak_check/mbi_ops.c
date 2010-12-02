/* mbi_ops.c
 * Operations with the storage for kedr_memblock_info structures:
 * addition to and removal from the storage, searching, etc.
 * 
 * The goal is for the analysis system to use the interface to this storage
 * that does not heavily depend on the underlying data structures.
 */

/* ========================================================================
 * Copyright (C) 2010, Institute for System Programming 
 *                     of the Russian Academy of Sciences (ISPRAS)
 * Authors: 
 *      Eugene A. Shatokhin <spectre@ispras.ru>
 *      Andrey V. Tsyvarev  <tsyvarev@ispras.ru>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 ======================================================================== */

#include <linux/kernel.h>

#include "mbi_ops.h"



/* ================================================================ */
