# rjson

A C based JSON parser for R.
Released versions can be found at http://cran.r-project.org/web/packages/rjson/index.html

Alex Couture-Beil
rjson_pkg@mofo.ca


## Development notes

rjson uses [earthly](http://github.com/earthly/earthly) to containerize common development tasks.

### test

To run a comprehensive test, run:

    earthly +test

This will run various sub-tests, against multiple versions of R.

### unit tests

To run the unit tests, run:

    earthly +unittest

or against a particular version of R, e.g. 4.0.0:

    earthly +unittest --R_VERSION=4.0.0

### rcheck

To run rcheck, run:

    earthly +rcheck

### Packaging rjson for cran


To create a source `rjson_<version>.tar.gz`, run:

    earthly +cran

This will output a compressed source archive under `output/`.

#### Non-earthly tasks

To run R check with valgrind, in the past I have run:

    docker run -v `pwd`:/foo -w /foo -ti -e VALGRIND_OPTS='--leak-check=full --show-reachable=yes' --rm --cap-add SYS_PTRACE rocker/r-devel-ubsan-clang R CMD check --use-valgrind rjson

(This should be moved into the Earthfile to make it easier to run).
