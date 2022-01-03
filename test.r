#!/usr/bin/Rscript

v <- function(...) cat(sprintf(...), sep='', file=stderr())

install.packages( '/code/rjson', repos=NULL )
library( rjson )

library( RUnit )
path <- system.file( "unittests", package="rjson" )
test.suite <- defineTestSuite( "json unittests", dirs = path, testFileRegexp = "^test\\..*\\.[rR]$" )
result <- getErrors(runTestSuite( test.suite, verbose = 100 ))

exit_code <- 0
if( result$nFail > 0 ) {
	v("%d test(s) failed\n", result$nFail)
	exit_code <- 1
}
if( result$nErr > 0 ) {
	v("%d test(s) had errors\n", result$nErr)
	exit_code <- 1
}
if( exit_code == 0 ) {
	v("tests passed\n")
}
quit(status=exit_code)
