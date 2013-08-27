\name{newJSONParser}
\alias{newJSONParser}
\title{Convert buffered JSON objects To R }

\description{ Convert a collection of JSON objects into R objects. }

\usage{newJSONParser(method = "R")}

\arguments{
\item{method}{use the \code{C} implementation, or the slower original \code{R} implementation}
}

\value{A list of functions used for parsing objects}

\seealso{
\code{\link{toJSON}}
}

\examples{

sample_json <- '
	{
		"breakfast" : [ "milk", "fruit loops", "juice" ],
		"lunch" : [ "left over sushi" ]
	}
'

parser <- newJSONParser()

parser$addData( sample_json )
food <- parser$getObject()
print( food )

#This is equivalent to using FromJSON( sample_json )
#However, sample_json can be split into several parts:

### EXAMPLE 2:

part_1 <- '{ "breakfast" : [ "milk", "fruit loops", "juice" ], '
part_2 <- '"lunch" : [ "left over sushi" ]'
# close off the first object, and create a 2nd JSON object, which is simply an 
# array
part_3 <- '} [1,2,3,4,5]' 

parser <- newJSONParser()
parser$addData( part_1 )
parser$getObject() #returns NULL - since part_1 isn't complete
parser$addData( part_2 )
parser$getObject() #returns NULL - since part_2 still isn't complete
parser$addData( part_3 )
parser$getObject() #returns the first food object
parser$getObject() #returns the second array




}

\keyword{interface}
