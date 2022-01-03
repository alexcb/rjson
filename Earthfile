deps:
    FROM alpine
    RUN apk add R R-dev make gcc g++
    RUN Rscript <(echo 'install.packages("RUnit", repos="http://cran.us.r-project.org")')

code:
    FROM +deps
    COPY --dir rjson /code/rjson

test:
    FROM +code
    WORKDIR /code
    COPY test.r .
    RUN ./test.r
