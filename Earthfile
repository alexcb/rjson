deps:
    FROM rocker/r-devel
    RUN bash -c "Rscript <(echo 'install.packages(\"RUnit\", repos=\"http://cran.us.r-project.org\")')"

code:
    FROM +deps
    COPY --dir rjson /code/rjson
    WORKDIR /code

unittest:
    FROM +code
    COPY test.r .
    # validate this file is utf-8 encoded
    RUN file rjson/inst/unittests/test.unicode.r | grep 'test.unicode.r: Unicode text, UTF-8 text'
    RUN ./test.r

cran:
    FROM +code
    # first make sure there are no warnings
    RUN R CMD check --as-cran rjson
    # next make the tar.gz
    RUN R CMD build rjson
    SAVE ARTIFACT rjson_*.tar.gz AS LOCAL output/

# based on https://github.com/kalibera/rchk/blob/master/image/docker/Dockerfile
rcheck:
    FROM kalibera/rchk:latest
    COPY +cran/rjson_* .
    RUN test $(ls rjson_*.tar.gz | wc -l) = 1
    RUN chmod +x /container.sh
    RUN /container.sh rjson_*.tar.gz 2>&1 | tee output.txt
    # /container.sh doesn't return an exit code on failures, so we will test that ERROR
    # doesn't exist in the output
    RUN cat output.txt | grep -v ERROR

test:
    BUILD +unittest
    #BUILD +rcheck # TODO enable this once rcheck is fixed


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
