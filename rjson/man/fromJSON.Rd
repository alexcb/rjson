\name{fromJSON}
\alias{fromJSON}
\title{Convert JSON To R}

\description{ Convert a JSON object into an R object. }

\usage{fromJSON( json_str, file, method = "C", unexpected.escape = "error" )}

\arguments{
\item{json_str}{a JSON object to convert}
\item{file}{the name of a file to read the json_str from; this can also be a URL. Only one of json_str or file must be supplied.}
\item{method}{use the \code{C} implementation, or the older slower (and one day to be depricated) \code{R} implementation}
\item{unexpected.escape}{changed handling of unexpected escaped characters. Handling value should be one of "error", "skip", or "keep"; on unexpected characters issue an \code{error}, \code{skip} the character, or \code{keep} the character}
}

\value{R object that corresponds to the JSON object}

\seealso{
\code{\link{toJSON}}
}

\examples{

#watch out for R turning null into FALSE
fromJSON( "[true, false, null]" )
##[1]  TRUE FALSE __FALSE__

#but if you use a hash, this will be avoided
fromJSON( '{"a":true, "b":false, "c":null}' )


#R vs C
x <- toJSON( iris )
system.time( y <- fromJSON(x) )
system.time( y2 <- fromJSON(x,method = "R") )

}

\keyword{interface}
