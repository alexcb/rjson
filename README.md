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

mkdir /tmp/rjson # to create a local place to avoid having to re-download RUnit each time
docker run -v `pwd`:/foo -v /tmp/rjson:/tmp/rjson -w /foo -ti --rm rocker/r-devel /foo/test.r

Packaging rjson for cran
------------------------

# first ensure there are no WARNINGs or NOTEs
docker run -v `pwd`:/foo -w /foo -ti --rm rocker/r-devel R CMD check --as-cran rjson

# Checking valgrind

docker run -v `pwd`:/foo -w /foo -ti -e VALGRIND_OPTS='--leak-check=full --show-reachable=yes' --rm --cap-add SYS_PTRACE rocker/r-devel-ubsan-clang R CMD check --use-valgrind rjson

# create source package with
docker run -v `pwd`:/foo -w /foo -ti --rm rocker/r-devel R CMD build rjson
