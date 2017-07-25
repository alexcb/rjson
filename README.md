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

Packaging rjson for cran
------------------------

# first ensure there are no WARNINGs or NOTEs
docker run -v `pwd`:/foo -w /foo -ti --rm rocker/r-devel R CMD check --as-cran rjson

# create source package with
docker run -v `pwd`:/foo -w /foo -ti --rm rocker/r-devel R CMD build rjson
