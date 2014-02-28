#ifndef BBCONE_H
#define BBCONE_H

#include <kernel/mod2.h>

#include <gfanlib/gfanlib.h>
#include <libpolys/misc/intvec.h>
#include <libpolys/coeffs/bigintmat.h>

#include "Singular/ipid.h"

extern int coneID;

void bbcone_setup(SModulFunctions* p);

std::string toString(const gfan::ZCone* const c);

gfan::ZVector randomPoint(const gfan::ZCone* zc);
gfan::ZCone liftUp(const gfan::ZCone &zc);

#endif
