rjson
=====

A C based JSON parser for R.
Released versions can be found at http://cran.r-project.org/web/packages/rjson/index.html

Alex Couture-Beil
rjson_pkg@mofo.ca


Installing from source
----------------------

install.packages('/foo/rjson', repos=NULL)


Running tests
-------------

docker run -v `pwd`:/foo -w /foo -ti --rm rocker/r-devel /foo/test.r

Packaging rjson
---------------

docker run -v `pwd`:/foo -w /foo -ti --rm rocker/r-devel /usr/bin/bash
R CMD check --as-cran rjson
R CMD build rjson
