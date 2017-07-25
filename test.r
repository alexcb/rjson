#!/usr/bin/Rscript
install.packages( '/foo/rjson', repos=NULL )
library( rjson )

install.packages( 'RUnit' )
library( RUnit )

path <- system.file( "unittests", package="rjson" )
test.suite <- defineTestSuite( "json unittests", dirs = path, testFileRegexp = "^test\\..*\\.[rR]$" )
runTestSuite( test.suite, verbose = 100 )
