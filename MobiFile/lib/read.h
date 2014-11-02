/** @file read.h
 *
 * Copyright (c) 2014 Bartek Fabiszewski
 * http://www.fabiszewski.net
 *
 * This file is part of libmobi.
 * Licensed under LGPL, either version 3, or any later.
 * See <http://www.gnu.org/licenses/>
 */

#ifndef libmobi_read_h
#define libmobi_read_h

#include "config.h"
#include "mobi.h"
#include "memory.h"

MOBI_RET mobi_load_pdbheader(MOBIData *m, FILE *file);
MOBI_RET mobi_load_reclist(MOBIData *m, FILE *file);
MOBI_RET mobi_load_rec(MOBIData *m, FILE *file);
MOBI_RET mobi_load_recdata(MOBIPdbRecord *rec, FILE *file);

#endif
