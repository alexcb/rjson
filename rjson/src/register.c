#include <R.h>
#include <Rdefines.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#include "funcs.h"

static const R_CMethodDef cMethods[] = {
	{"fromJSON", (DL_FUNC) &fromJSON, 3},
	{"toJSON", (DL_FUNC) &toJSON, 1},
	{NULL, NULL, 0}
};

void R_init_rjson(DllInfo* info) {
	R_registerRoutines(info, cMethods, NULL, NULL, NULL);
	R_useDynamicSymbols(info, TRUE);
}
