#include <R.h>
#include <Rdefines.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#include "rjson.h"

static const R_CMethodDef cMethods[] = {
	{NULL, NULL, 0}
};

void R_init_rjson(DllInfo* info) {
  R_registerRoutines(info, cMethods, NULL, NULL, NULL);
  R_useDynamicSymbols(info, FALSE);
}
