VERSION 0.7

deps:
    FROM rocker/r-devel
    RUN bash -c "Rscript <(echo 'install.packages(\"RUnit\", repos=\"http://cran.us.r-project.org\")')"

code:
    FROM +deps
    COPY --dir rjson /code/rjson
    WORKDIR /code

unicodecheck:
    FROM alpine
    RUN apk add file
    COPY rjson/inst/unittests/test.unicode.r .
    RUN file test.unicode.r | grep 'test.unicode.r: Unicode text, UTF-8 text'

unittest:
    ARG R_VERSION=4.4.0
    FROM rocker/r-base:$R_VERSION
    RUN bash -c "Rscript <(echo 'install.packages(\"RUnit\", repos=\"http://cran.us.r-project.org\")')"
    COPY --dir rjson /code/rjson
    WORKDIR /code
    COPY test.r .
    RUN ./test.r

cran:
    FROM +code
    # first make sure there are no warnings
    RUN R CMD check --as-cran rjson
    # next make the tar.gz
    ENV _R_CXX_USE_NO_REMAP_=true
    RUN R CMD build rjson
    SAVE ARTIFACT rjson_*.tar.gz AS LOCAL output/

# based on https://github.com/kalibera/rchk/blob/master/image/docker/Dockerfile
rcheck:
    FROM kalibera/rchk:latest
    COPY +cran/rjson_* .
    RUN test $(ls rjson_*.tar.gz | wc -l) = 1
    RUN chmod +x /container.sh
    RUN echo "#!/bin/bash
# /container.sh doesn't return an exit code on failures, so we will test that ERROR
# doesn't exist in the output
/container.sh rjson_*.tar.gz 2>&1 | tee output.txt
cat output.txt | grep -v ERROR" > test.sh && chmod +x test.sh
    RUN ./test.sh

test:
    BUILD +unittest \
        --R_VERSION=4.0.0 \
        --R_VERSION=4.4.0
    BUILD +rcheck
    BUILD +unicodecheck


reformat:
    FROM ubuntu:20.04
    ENV DEBIAN_FRONTEND=noninteractive
    RUN apt-get update && \
        apt-get install -y \
        clang-format-7
    WORKDIR /code
    COPY .clang-format .
    COPY --dir rjson/src src
    RUN find . -type f -name '*.c' -exec clang-format-7 -i {} \;
    SAVE ARTIFACT src/* AS LOCAL rjson/src/
