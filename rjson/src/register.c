#include <R.h>
#include <R_ext/Rdynload.h>
#include <Rdefines.h>
#include <Rinternals.h>

#include "funcs.h"

static const R_CMethodDef cMethods[] = {
	{"fromJSON", (DL_FUNC)&fromJSON, 3}, {"toJSON", (DL_FUNC)&toJSON, 1}, {NULL, NULL, 0}};

void R_init_rjson( DllInfo* info )
{
	R_registerRoutines( info, cMethods, NULL, NULL, NULL );

	// TODO this should be changed to FALSE to disable
	// as per note on https://www.r-project.org/nosvn/R.check/r-devel-linux-x86_64-debian-clang/rjson-00check.html
	R_useDynamicSymbols( info, TRUE );
}
