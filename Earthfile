deps:
    FROM alpine
    RUN apk add R R-dev make gcc g++ file
    RUN Rscript <(echo 'install.packages("RUnit", repos="http://cran.us.r-project.org")')

code:
    FROM +deps
    COPY --dir rjson /code/rjson

test:
    FROM +code
    WORKDIR /code
    COPY test.r .
    # validate this file is utf-8 encoded
    RUN file rjson/inst/unittests/test.unicode.r | grep 'test.unicode.r: Unicode text, UTF-8 text'
    RUN ./test.r
