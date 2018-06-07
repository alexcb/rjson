#!/usr/bin/Rscript
install.packages( '/foo/rjson', repos=NULL )
library( rjson )

# change this to the latest version in https://cran.rstudio.com/web/packages/RUnit/index.html
RUnit = 'RUnit_0.4.32.tar.gz'
tmp_path = sprintf('/tmp/rjson/%s', RUnit )
if( !file.exists(tmp_path) ) {
	url = sprintf('https://cloud.r-project.org/src/contrib/%s', RUnit)
	cat('downloading ', url)
	download.file( url, tmp_path )
}

install.packages( tmp_path, repos=NULL )
library( RUnit )

path <- system.file( "unittests", package="rjson" )
test.suite <- defineTestSuite( "json unittests", dirs = path, testFileRegexp = "^test\\..*\\.[rR]$" )
runTestSuite( test.suite, verbose = 100 )
