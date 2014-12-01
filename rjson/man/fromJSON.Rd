\name{fromJSON}
\alias{fromJSON}
\title{Convert JSON To R}

\description{ Convert a JSON object into an R object. }

\usage{fromJSON( json_str, file, method = "C", unexpected.escape = "error", simplify = TRUE )}

\arguments{
\item{json_str}{a JSON object to convert}
\item{file}{the name of a file to read the json_str from; this can also be a URL. Only one of json_str or file must be supplied.}
\item{method}{use the \code{C} implementation, or the older slower (and one day to be depricated) \code{R} implementation}
\item{unexpected.escape}{changed handling of unexpected escaped characters. Handling value should be one of "error", "skip", or "keep"; on unexpected characters issue an \code{error}, \code{skip} the character, or \code{keep} the character}
\item{simplify}{If TRUE, attempt to convert json-encoded lists into vectors where appropriate. If FALSE, all json-encoded lists will be wrapped in a list even if they are all of the same data type. }
}

\value{R object that corresponds to the JSON object}

\seealso{
\code{\link{toJSON}}
}

\examples{

fromJSON('[1,2,3]', simplify=True)
# returns c(1,2,3)
fromJSON('[1,2,3]', simplify=False)
# returns list(1,2,3)

#As a result, this will output "1"
toJSON(fromJSON('[1]', simplify=True))
#Compared with this which will output "[1]" as expected
toJSON(fromJSON('[1]', simplify=True))

#R vs C execution time
x <- toJSON( iris )
system.time( y <- fromJSON(x) )
system.time( y2 <- fromJSON(x,method = "R") )

}

\keyword{interface}
