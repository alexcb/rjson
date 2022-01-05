
test.issue24 <- function()
{
	# json unicode is escaped as \u018E; not uppercase \U
	bad_json <- "\"\\U018E\""
	x <- try( fromJSON( bad_json ), silent = TRUE )
	checkTrue( any( class( x ) == "try-error" ) )
	# TODO validate error message is ok: should contain "<simpleError in fromJSON(bad_json): not all data was parsed (0 chars were parsed out of a total of 8 chars)>"
}

