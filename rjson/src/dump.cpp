#include <string>
#include <sstream>
#include <iomanip>
#include <limits>

//must include these after STL files due to length macro in Rinternals being seen by a STL on OSX.
#include <R.h>
#include <Rdefines.h>

std::string escapeString( const char *s )
{
	std::ostringstream oss;
	oss << '"';

	while( *s ) {
		switch( *s ) {
			case '"':
				oss << "\\\"";
				break;
			case '\\':
				oss << "\\\\";
				break;
			case '\n':
				oss << "\\n";
				break;
			case '\r':
				oss << "\\r";
				break;
			case '\t':
				oss << "\\t";
				break;
			default:
				unsigned char ch = static_cast<unsigned char>(*s);
				if (( ch <= 0x1F ) || (ch == 0x7F)) {
					unsigned short val = ch;
					oss << "\\u" << std::setfill('0') << std::setw(4) << std::hex << val << std::dec;
				} else if( ch < 0x80 ) {
					// 0xxxxxxx
					oss << *s;
				} else if( (ch & 0xE0) == 0xC0 && s[1] ) {
					// 110xxxxx 10xxxxxx
					unsigned short val = (s[1] & 0x3F) + ((s[0] & 0x1F) << 6);
					oss << "\\u" << std::setfill('0') << std::setw(4) << std::hex << val << std::dec;
					s += 1;
				} else if( (ch & 0xF0) == 0xE0 && s[1] && s[2] ) {
					// 1110xxxx 10xxxxxx 10xxxxxx
					unsigned short val = (s[2] & 0x3F) + ((s[1] & 0x3F) << 6) + ((s[0] & 0x0F) << 12);
					oss << "\\u" << std::setfill('0') << std::setw(4) << std::hex << val << std::dec;
					s += 2;
				} else {
					error("unable to escape string. String is not utf8\n");
				}
		}
		s++;
	}
       
	oss << '"';
	return oss.str();
}

#define NO_CONTAINER 0
#define ARRAY_CONTAINER 1
#define OBJECT_CONTAINER 2

std::string toJSON2( SEXP x )
{
	if( x == R_NilValue )
		return "null";

	int i = 0;
	int n = length(x);
	SEXP names = GET_NAMES(x);

	//int container = NO_CONTAINER;
	std::string container_closer;
	std::ostringstream oss;
	
	if( names != NULL_USER_OBJECT ) {
		oss << "{";
		container_closer = "}";
		if( length(names) != n )
			error("number of names does not match number of elements\n");
	} else if( n != 1 || TYPEOF(x) == VECSXP ) {
		oss << "[";
		container_closer = "]";
	}

	SEXP levels;
	PROTECT( levels = GET_LEVELS(x));
	
	switch( TYPEOF(x) ) {
		case LGLSXP:
			for( i = 0; i < n; i++ ) {
				if( i > 0 )
					oss << ",";
				if( names != NULL_USER_OBJECT )
					oss << escapeString(CHAR(STRING_ELT(names, i))) << ":";
				if( LOGICAL(x)[i] == NA_INTEGER )
					oss << "\"NA\"";
				else if( ISNAN( LOGICAL(x)[i] ) )
					oss << "\"NaN\"";
				else if( LOGICAL(x)[i] )
					oss << "true";
				else
					oss << "false";
			}
			break;
		case INTSXP:
			for( i = 0; i < n; i++ ) {
				if( i > 0 )
					oss << ",";
				if( names != NULL_USER_OBJECT )
					oss << escapeString(CHAR(STRING_ELT(names, i))) << ":";
				if( INTEGER(x)[i] == NA_INTEGER )
					oss << "\"NA\"";
				else if( ISNAN( INTEGER(x)[i] ) )
					oss << "\"NaN\"";
				else if( levels != NULL_USER_OBJECT )
					oss << escapeString(CHAR(STRING_ELT(levels, INTEGER(x)[i] - 1 )));
				else
					oss << INTEGER(x)[i];
			}
			break;
		case REALSXP:
			for( i = 0; i < n; i++ ) {
				if( i > 0 ) {
					oss << ",";
				}
				if( names != NULL_USER_OBJECT ) {
					oss << escapeString(CHAR(STRING_ELT(names, i))) << ":";
				}
				if( ISNA(REAL(x)[i]) ) {
					oss << "\"NA\"";
				} else if( ISNAN(REAL(x)[i]) ) {
					oss << "\"NaN\"";
				} else if( R_FINITE(REAL(x)[i]) ) {
					oss << std::setprecision( std::numeric_limits<double>::digits10 ) << REAL(x)[i];
				} else {
					oss << (REAL(x)[i] > 0 ? "\"Inf\"" : "\"-Inf\"");
				}
			}
			break;
		case CPLXSXP:
			{
				SEXP p, p_names;
				PROTECT( p = allocVector( REALSXP, 2 ) );
				PROTECT( p_names = allocVector( STRSXP, 2 ) );

				SET_STRING_ELT( p_names, 0, mkChar("real") );
				REAL(p)[0] = COMPLEX(x)[i].r;
				SET_STRING_ELT( p_names, 1, mkChar("imaginary") );
				REAL(p)[1] = COMPLEX(x)[i].i;

				setAttrib( p, R_NamesSymbol, p_names );
				oss << toJSON2(p);
				UNPROTECT(2);
			}
			break;
		case STRSXP:
			for( i = 0; i < n; i++ ) {
				if( i > 0 )
					oss << ",";
				if( names != NULL_USER_OBJECT )
					oss << escapeString(CHAR(STRING_ELT(names, i))) << ":";
				if( STRING_ELT(x,i) == NA_STRING )
					oss << "\"NA\"";
				else
					oss << escapeString(CHAR(STRING_ELT(x,i)));
			}
			break;
		case VECSXP:
			for( i = 0; i < n; i++ ) {
				if( i > 0 )
					oss << ",";
				if( names != NULL_USER_OBJECT )
					oss << escapeString(CHAR(STRING_ELT(names, i))) << ":";
				oss << toJSON2( VECTOR_ELT(x,i) );
			}
			break;
		default:
			error("unable to convert R type %i to JSON\n", TYPEOF(x));
	}
	UNPROTECT(1);
	oss << container_closer;
	return oss.str();
}

extern "C" {
	SEXP toJSON( SEXP obj )
	{
		std::string buf = toJSON2( obj );
		SEXP p;
		PROTECT(p=allocVector(STRSXP, 1));
		SET_STRING_ELT(p, 0, mkCharCE( buf.c_str(), CE_UTF8 ));
		UNPROTECT( 1 );
		return p;
	}
}

