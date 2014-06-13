#include <R.h>
#include <Rdefines.h>

#define DEFAULT_VECTOR_START_SIZE 10 /* allocate vectors this size to start with, then grow them as needed */
#define MAX_NUMBER_BUF 256

#define UNEXPECTED_ESCAPE_ERROR 1 /* issue an error and stop */
#define UNEXPECTED_ESCAPE_SKIP 2 /* skip the unexpected char and move to the next character */
#define UNEXPECTED_ESCAPE_KEEP 3 /* include the unexpected char as a regular char and continue */

/* converts a single string into a list of (key, value) lists
   i.e. convert "nokey keyname=foo" into the following R list:
   list(list(value="nokey"), list(key="keyname", value="foo"))
   
   args: str - R character string
         env - R environment for use in calling R code
         fname - R character string of filename for use in error reporting
         lineNum - R numeric value for line number for use in error reporting
   
   return: R list of 1-element, or 2-element lists.
           i.e.: "nokey keyname=foo" becomes
           list(list(value="nokey"), list(key="keyname", value="foo"))
*/
SEXP parseValue( const char *s, const char **next_ch, const int unexpected_escape_handling );
SEXP parseNull( const char *s, const char **next_ch );
SEXP parseTrue( const char *s, const char **next_ch );
SEXP parseFalse( const char *s, const char **next_ch );
SEXP parseString( const char *s, const char **next_ch, const int unexpected_escape_handling );
SEXP parseNumber( const char *s, const char **next_ch );
SEXP parseArray( const char *s, const char **next_ch, const int unexpected_escape_handling );
SEXP parseList( const char *s, const char **next_ch, const int unexpected_escape_handling );

SEXP mkError( const char* format, ...);

int getUnexpectedEscapeHandlingCode( const char *s );


#define TRYERROR_CLASS "try-error"
SEXP mkError( const char* format, ...)
{
	SEXP p, classp;
	char buf[ 256 ];
	va_list args;
	va_start( args, format );
	vsnprintf( buf, 256, format, args );
	va_end( args );

	PROTECT( p = allocVector(STRSXP, 1) );
	SET_STRING_ELT( p, 0, mkCharCE( buf, CE_UTF8 ) );
	PROTECT( classp = allocVector(STRSXP, 1) );
	SET_STRING_ELT( classp, 0, mkChar( TRYERROR_CLASS ) );
	SET_CLASS( p, classp );
	UNPROTECT( 2 );
	return p;
}

#define INCOMPLETE_CLASS "incomplete"
SEXP addClass( SEXP p, const char * class )
{
	SEXP class_p;
	PROTECT( class_p = GET_CLASS( p ) );
	unsigned int size = GET_LENGTH( class_p );
	PROTECT( SET_LENGTH( class_p, size + 1 ) );
	SET_STRING_ELT( class_p, size, mkChar( class ) );
	SET_CLASS( p, class_p );
	UNPROTECT( 2 );
	return p;
}

int hasClass( SEXP p, const char * class )
{
	int i;
	SEXP class_p;
	PROTECT( class_p = GET_CLASS( p ) );
	unsigned int size = GET_LENGTH( class_p );
	for( i = 0; i < size; i++ ) {
		const char *s = CHAR(STRING_ELT(class_p, i));
		if( strcmp( s, class ) == 0 ) {
			UNPROTECT( 1 );
			return TRUE;
		}
	}
	UNPROTECT( 1 );
	return FALSE;
}

#define         MASKBITS                0x3F
#define         MASKBYTE                0x80
#define         MASK2BYTES              0xC0
#define         MASK3BYTES              0xE0

int UTF8Encode2BytesUnicode( unsigned short input, char * s )
{
	// 0xxxxxxx
	if( input < 0x80 )
	{
		s[ 0 ] = input;
		return 1;
	}
	// 110xxxxx 10xxxxxx
	else if( input < 0x800 )
	{
		s[ 0 ] = (MASK2BYTES | ( input >> 6 ) );
		s[ 1 ] = (MASKBYTE | ( input & MASKBITS ) );
		return 2;
	}
	// 1110xxxx 10xxxxxx 10xxxxxx
	else
	{
		s[ 0 ] = (MASK3BYTES | ( input >> 12 ) );
		s[ 1 ] = (MASKBYTE | ( ( input >> 6 ) & MASKBITS ) );
		s[ 2 ] = (MASKBYTE | ( input & MASKBITS ) );
		return 3;
	}
}

int getUnexpectedEscapeHandlingCode( const char *s )
{
	if( s != NULL ) {
		if( strcmp( s, "skip" ) == 0 ) {
			return UNEXPECTED_ESCAPE_SKIP;
		} else if( strcmp( s, "keep" ) == 0 ) {
			return UNEXPECTED_ESCAPE_KEEP;
		}
	}
	/* in all other cases, just use the standard original error */
	return UNEXPECTED_ESCAPE_ERROR;
}

SEXP fromJSON( SEXP str_in, SEXP unexpected_escape_handling_in )
{
	const char *s = CHAR(STRING_ELT(str_in,0));
	const char *next_ch;
	int unexpected_escape_handling = getUnexpectedEscapeHandlingCode(CHAR(STRING_ELT(unexpected_escape_handling_in,0)));
	SEXP p, next_i, list;
	PROTECT( p = parseValue( s, &next_ch, unexpected_escape_handling ) );

	PROTECT( list = allocVector( VECSXP, 2 ) );
	PROTECT( next_i = allocVector( INTSXP, 1 ) );
	//PROTECT( list_names = allocVector( STRSXP, DEFAULT_VECTOR_START_SIZE ) );
	
	//SET_STRING_ELT( list_names, list_i, STRING_ELT(key, 0) );
	SET_VECTOR_ELT( list, 0, p );

	INTEGER( next_i )[ 0 ] = next_ch - s;
	SET_VECTOR_ELT( list, 1, next_i );

	UNPROTECT( 3 );
	return list;
}

SEXP parseValue( const char *s, const char **next_ch, const int unexpected_escape_handling )
{
	/* ignore whitespace */
	while( *s == ' ' || *s == '\t' || *s == '\n' || *s == '\r' )
		s++;

	if( *s == '{' ) {
		return parseList( s, next_ch, unexpected_escape_handling );
	}
	if( *s == '[' ) {
		return parseArray( s, next_ch, unexpected_escape_handling );
	}
	if( *s == '\"' ) {
		return parseString( s, next_ch, unexpected_escape_handling );
	}
	if( ( *s >= '0' && *s <= '9' ) || *s == '-' ) {
		return parseNumber( s, next_ch );
	}
	if( *s == 't' ) {
		return parseTrue( s, next_ch );
	}
	if( *s == 'f' ) {
		return parseFalse( s, next_ch );
	}
	if( *s == 'n' ) {
		return parseNull( s, next_ch );
	}

	if( *s == '\0' ) {
		return addClass( mkError( "no data to parse\n" ), INCOMPLETE_CLASS );
	}

	return mkError( "unexpected character '%c'\n", *s );
}

SEXP parseNull( const char *s, const char **next_ch )
{
	if( strncmp( s, "null", 4 ) == 0 ) {
		*next_ch = s + 4;
		return R_NilValue;
	}

	//TODO should really look at subset of "null" (e.g. "nul", "nu" ), so that "not" fails before reaching 4 digits
	if( strlen( s ) < 4 )
		return addClass( mkError( "parseNull: expected to see 'null' - likely an unquoted string starting with 'n', or truncated null.\n" ), INCOMPLETE_CLASS );
	return mkError( "parseNull: expected to see 'null' - likely an unquoted string starting with 'n'.\n" );
}

SEXP parseTrue( const char *s, const char **next_ch )
{
	SEXP p;
	if( strncmp( s, "true", 4 ) == 0 ) {
		*next_ch = s + 4;
		PROTECT( p = NEW_LOGICAL( 1 ) );
		LOGICAL( p )[ 0 ] = TRUE;
		UNPROTECT( 1 );
		return p;
	}
	if( strlen( s ) < 4 )
		return addClass( mkError( "parseTrue: expected to see 'true' - likely an unquoted string starting with 't', or truncated true.\n" ), INCOMPLETE_CLASS );
	return mkError( "parseTrue: expected to see 'true' - likely an unquoted string starting with 't'.\n" );
}

SEXP parseFalse( const char *s, const char **next_ch )
{
	SEXP p;
	if( strncmp( s, "false", 5 ) == 0 ) {
		*next_ch = s + 5;
		PROTECT( p = NEW_LOGICAL( 1 ) );
		LOGICAL( p )[ 0 ] = FALSE;
		UNPROTECT( 1 );
		return p;
	}
	if( strlen( s ) < 5 )
		return addClass( mkError( "parseFalse: expected to see 'false' - likely an unquoted string starting with 'f', or truncated false.\n" ), INCOMPLETE_CLASS );
	return mkError( "parseFalse: expected to see 'false' - likely an unquoted string starting with 'f'.\n" );
}

SEXP parseString( const char *s, const char **next_ch, const int unexpected_escape_handling )
{
	SEXP p;
	/*assert( s[ 0 ] == '"' );*/
	int i = 1; /*skip the start quote*/

	int buf_size = 256;
	char *buf = (char*) malloc( buf_size );
	buf[0] = '\0';
	int buf_i = 0;
	if( buf == NULL )
		return mkError( "error allocating memory in parseString" );
	if( sizeof( char ) != 1 )
		return mkError( "parseString sizeof(char) != 1" );

	int copy_start = i;
	int bytes_to_copy;

	while( 1 ) {
		while( s[ i ] != '\\' && s[ i ] != '"' && s[ i ] != '\0' )
			i++;
		if( s[ i ] == '\0' ) {
			return addClass( mkError( "unclosed string\n" ), INCOMPLETE_CLASS );
		}

		if( s[ i ] == '\\' ) {
			if( s[ i + 1 ] == '\0' ) {
				return addClass( mkError( "unclosed string\n" ), INCOMPLETE_CLASS );
			}
			//TODO couldn't this be caught above (where s[ i ] == '\0')
			if( s[ i + 2 ] == '\0' ) {
				return addClass( mkError( "unclosed string\n" ), INCOMPLETE_CLASS );
			}

			/* grow memory */
			if( buf_size - 1 <= i ) {
				buf_size = 2 * ( buf_size + i );
				buf = realloc( buf, buf_size );
				if( buf == NULL )
					return mkError( "error allocating memory in parseString" );
			}
			
			/* save string chunk from copy_start to i-1 */
			bytes_to_copy = i - copy_start;
			if( bytes_to_copy > 0 ) {

				memcpy( buf + buf_i, s + copy_start, bytes_to_copy );
				buf_i += bytes_to_copy;
			}
			i++;

			/* save s[i] */
			switch( s[ i ] ) {
				case '"':
				case '\\':
				case '/':
					buf[ buf_i ] = s[ i ];
					break;
				case 'b':
					buf[ buf_i ] = '\b';
					break;
				case 'f':
					buf[ buf_i ] = '\f';
					break;
				case 'n':
					buf[ buf_i ] = '\n';
					break;
				case 'r':
					buf[ buf_i ] = '\r';
					break;
				case 't':
					buf[ buf_i ] = '\t';
					break;
				case 'u':
					for( int j = 1; j <= 4; j++ )
						if( ( ( s[ i + j ] >= 'a' && s[ i + j ] <= 'f' ) || 
						    ( s[ i + j ] >= 'A' && s[ i + j ] <= 'F' ) ||
						    ( s[ i + j ] >= '0' && s[ i + j ] <= '9' ) ) == FALSE ) {
							return mkError( "unexpected unicode escaped char '%c'; 4 hex digits should follow the \\u (found %i valid digits)", s[ i + j ], j - 1 );
						}

					unsigned short unicode;
					char unicode_buf[ 5 ]; /* to hold 4 digit hex (to prevent scanning a 5th digit accidentally */
					strncpy( unicode_buf, s + i + 1, 5 );
					unicode_buf[ 4 ] = '\0';
					sscanf( unicode_buf, "%hx", &unicode);
					buf_i += UTF8Encode2BytesUnicode( unicode, buf + buf_i ) - 1; /* -1 due to buf_i++ out of loop */

					i += 4; /* skip the four digits - actually point to last digit, which is then incremented outside of switch */

					break;
				default:
					if( unexpected_escape_handling == UNEXPECTED_ESCAPE_SKIP ) {
						//skip the character (by decreasing the buffer index as it will be increased below. in actuality we dont want it to change).
						buf_i--; 
						Rf_warning( "unexpected escaped character '\\%c' at pos %i. Skipping value.", s[ i ], i );
					} else if( unexpected_escape_handling == UNEXPECTED_ESCAPE_KEEP ) {
						//treat a "\y" as simply 'y'
						buf[ buf_i ] = s[ i ];
						Rf_warning( "unexpected escaped character '\\%c' at pos %i. Keeping value.", s[ i ], i );
					} else {
						//case of UNEXPECTED_ESCAPE_ERROR, or any other bad enum values
						return mkError( "unexpected escaped character '\\%c' at pos %i", s[ i ], i );
					}
					break;
			}

			i++; /* move to next char */
			copy_start = i;
			buf_i++;
		} else {
			/*must be a quote that caused us the exit the loop, first, save remaining string data*/
			if( buf_size - 1 <= i ) {
				/* grow memory */
				buf_size = 2 * ( buf_size + i );
				buf = realloc( buf, buf_size );
				if( buf == NULL )
					return mkError( "error allocating memory in parseString" );
			}
			bytes_to_copy = i - copy_start;
			if( bytes_to_copy > 0 ) {
				memcpy( buf + buf_i, s + copy_start, bytes_to_copy );
				buf_i += bytes_to_copy;
			}
			buf[ buf_i ] = '\0';
			break; /*exit the loop*/
		}
	}
	
	
	*next_ch = s + i + 1;
	PROTECT(p=allocVector(STRSXP, 1));
	SET_STRING_ELT(p, 0, mkCharCE( buf, CE_UTF8 ));
	free( buf );
	UNPROTECT( 1 );
	return p;
}

void setArrayElement( SEXP array, int unsigned i, SEXP val )
{
	if( IS_LOGICAL( array ) )
		LOGICAL( array )[ i ] = LOGICAL( val )[ 0 ];
	else if( IS_INTEGER( array ) )
		INTEGER( array )[ i ] = INTEGER( val )[ 0 ];
	else if( IS_NUMERIC( array ) )
		REAL( array ) [ i ] = REAL( val )[ 0 ];
	else if( IS_CHARACTER( array ) )
		SET_STRING_ELT( array, i, STRING_ELT(val, 0) ); /*TODO fixme val must be a single char, not vector*/
/*	else if( IS_COMPLEX( array ) )
		COMPLEX( array [ i ] = COMPLEX( val )[ 0 ]; */
	else
		Rprintf( "unsupported SEXPTYPE: %i\n", TYPEOF( array ) );
}

SEXP test( void )
{

	int i;
	SEXP p, array;
	PROTECT( array = allocVector( REALSXP, DEFAULT_VECTOR_START_SIZE ) );
	PROTECT( p = allocVector( REALSXP, 1 ) );
	REAL( p )[ 0 ] = 4;
	for( i = 0; i < 1000000; i++ ) {
		setArrayElement( array, i, p );
	}
	UNPROTECT( 2 );
	return array;
}

SEXP parseArray( const char *s, const char **next_ch, const int unexpected_escape_handling )
{
	PROTECT_INDEX p_index = -1, array_index = -1;
	SEXP p = NULL, array = NULL;
	/*assert( *s == '[' )*/
	s++; /*move past '['*/
	int objs = 0;
	int is_list = FALSE;
	SEXPTYPE  p_type = -1;
	unsigned int array_i = 0;

	while( 1 ) {
		/*ignore whitespace*/
		while( *s == ' ' || *s == '\t' || *s == '\n' || *s == '\r' )
			s++;
		if( *s == '\0' ) {
			UNPROTECT( objs );
			return addClass( mkError( "incomplete array\n" ), INCOMPLETE_CLASS );
		}

		if( *s == ']' ) {
			*next_ch = s + 1;
			return allocVector(VECSXP, 0);
		}

		/* parse element (and protect pointer - ugly) */
		if( p == NULL ) {
			PROTECT_WITH_INDEX( p = parseValue( s, next_ch, unexpected_escape_handling ), &p_index );
			objs++;
		} else {
			REPROTECT( p = parseValue( s, next_ch, unexpected_escape_handling ), p_index );
		}
		s = *next_ch;

		/* check p for errors */
		if( hasClass( p, TRYERROR_CLASS ) == TRUE ) {
			UNPROTECT( objs );
			return p;
		}

		if( array == NULL ) {
			/*create a vector of type that matches p*/
			if( GET_LENGTH( p ) != 1 )
				p_type = VECSXP;
			else
				p_type = TYPEOF( p );
			PROTECT_WITH_INDEX( array = allocVector( p_type, DEFAULT_VECTOR_START_SIZE ), &array_index );
			objs++;
			is_list = ( p_type == VECSXP );
		}

		/*check array type matches*/
		if( is_list == FALSE && ( TYPEOF( p ) != TYPEOF( array ) || GET_LENGTH( p ) != 1 ) ) {
			REPROTECT( array = coerceVector( array, VECSXP ), array_index );
			is_list = TRUE;
		}

		/*checksize*/
		unsigned int array_size = GET_LENGTH( array );
		if( array_i >= array_size ) {
			REPROTECT( SET_LENGTH( array, array_size * 2 ), array_index );
		}
		
		/*save element*/
		if( is_list == TRUE )
			SET_VECTOR_ELT( array, array_i, p);
		else
			setArrayElement( array, array_i, p );
		array_i++;
		
		/*ignore whitespace*/
		while( *s == ' ' || *s == '\t' || *s == '\n' || *s == '\r' )
			s++;
		
		if( *s == '\0' ) {
			UNPROTECT( objs );
			return addClass( mkError( "incomplete array\n" ), INCOMPLETE_CLASS );
		}

		
		/*end of array*/
		if( *s == ']' ) {
			break;
		}
		
		/*more elements to come*/
		if( *s == ',' ) {
			s++;
		} else if( *s == '\0' ) {
			UNPROTECT( objs );
			return addClass( mkError( "incomplete array\n" ), INCOMPLETE_CLASS );
		} else {
			UNPROTECT( objs );
			return mkError( "unexpected character: %c\n", *s );
		}
	}
	
	/*trim to the correct size - no need to protect here*/
	SET_LENGTH( array, array_i );
	
	*next_ch = s + 1;
	
	UNPROTECT( objs );
	return array;
}

SEXP parseList( const char *s, const char **next_ch, const int unexpected_escape_handling )
{
	PROTECT_INDEX key_index, val_index, list_index, list_names_index;
	SEXP key = NULL, val = NULL, list, list_names;
	/*assert( *s == '{' )*/
	s++; /*move past '{'*/
	int objs = 0;
	unsigned int list_i = 0;

	PROTECT_WITH_INDEX( list = allocVector( VECSXP, DEFAULT_VECTOR_START_SIZE ), &list_index );
	PROTECT_WITH_INDEX( list_names = allocVector( STRSXP, DEFAULT_VECTOR_START_SIZE ), &list_names_index );
	objs += 2;

	while( 1 ) {
		/*ignore whitespace*/
		while( *s == ' ' || *s == '\t' || *s == '\n' || *s == '\r' )
			s++;
		if( *s == '\0' ) {
			UNPROTECT( objs );
			return addClass( mkError( "incomplete list\n" ), INCOMPLETE_CLASS );
		}

		if( *s == '}' && list_i == 0 ) {
			UNPROTECT( objs );
			*next_ch = s + 1;
			return allocVector(VECSXP, 0);
		}

		/*get key*/

		if( *s != '\"' ) {
			UNPROTECT( objs );
			return mkError( "unexpected character \"%c\"; expecting opening string quote (\") for key value\n", *s );
		}

		if( key == NULL ) {
			PROTECT_WITH_INDEX( key = parseString( s, next_ch, unexpected_escape_handling ), &key_index );
			objs++;
		} else {
			REPROTECT( key = parseString( s, next_ch, unexpected_escape_handling ), key_index );
		}
		s = *next_ch;

		/* check key for errors */
		if( hasClass( key, TRYERROR_CLASS ) == TRUE ) {
			UNPROTECT( objs );
			return key;
		}

		if( IS_CHARACTER( key ) == FALSE ) {
			UNPROTECT( objs );
			return mkError( "list keys must be strings\n" );
		}

		/*ignore whitespace*/
		while( *s == ' ' || *s == '\t' || *s == '\n' || *s == '\r' )
			s++;
		if( *s != ':' ) {
			UNPROTECT( objs );
			if( *s == '\0' )
				return addClass( mkError( "incomplete list - missing :\n" ), INCOMPLETE_CLASS );
			return mkError( "incomplete list - missing :\n" );
		}
		s++; /*move past ':'*/

		/*ignore whitespace*/
		while( *s == ' ' || *s == '\t' || *s == '\n' || *s == '\r' )
			s++;
		if( *s == '\0' ) {
			UNPROTECT( objs );
			return addClass( mkError( "incomplete list\n" ), INCOMPLETE_CLASS );
		}

		/*get value*/
		if( val == NULL ) {
			PROTECT_WITH_INDEX( val = parseValue( s, next_ch, unexpected_escape_handling ), &val_index );
			objs++;
		} else {
			REPROTECT( val = parseValue( s, next_ch, unexpected_escape_handling ), val_index );
		}
		s = *next_ch;

		/* check val for errors */
		if( hasClass( val, TRYERROR_CLASS ) == TRUE ) {
			UNPROTECT( objs );
			return val;
		}

		/*checksize*/
		unsigned int list_size = GET_LENGTH( list );
		if( list_i >= list_size ) {
			REPROTECT( SET_LENGTH( list, list_size * 2 ), list_index );
			REPROTECT( SET_LENGTH( list_names, list_size * 2 ), list_names_index );
		}

		/*save key and value*/
		SET_STRING_ELT( list_names, list_i, STRING_ELT(key, 0) );
		SET_VECTOR_ELT( list, list_i, val );
		list_i++;

		/*ignore whitespace*/
		while( *s == ' ' || *s == '\t' || *s == '\n' || *s == '\r' )
			s++;
		if( *s == '\0' ) {
			UNPROTECT( objs );
			return addClass( mkError( "incomplete list\n" ), INCOMPLETE_CLASS );
		}

		/*end of list*/
		if( *s == '}' ) {
			break;
		}

		/*more elements to come*/
		if( *s == ',' ) {
			s++;
		} else {
			UNPROTECT( objs );
			return mkError( "unexpected character: %c\n", *s );
		}
	}

	/*trim to the correct size*/
	REPROTECT( SET_LENGTH( list, list_i ), list_index );
	REPROTECT( SET_LENGTH( list_names, list_i ), list_names_index );

	/*set names*/
	setAttrib( list, R_NamesSymbol, list_names );

	*next_ch = s + 1;

	UNPROTECT( objs );
	return list;

}

SEXP parseNumber( const char *s, const char **next_ch )
{
	SEXP p;
	const char *start = s;
	char buf[ MAX_NUMBER_BUF ];

	if( *s == '-' )
		s++;

	if( *s == '\0' ) {
		return addClass( mkError( "parseNumer error\n", *s ), INCOMPLETE_CLASS );
	}

	if( *s == '0' ) {
		s++;
		if( ( *s >= '0' && *s <= '9' ) || *s == 'x' ) {
			return mkError( "hex or octal is not valid json\n" );
		}
	}

	while( *s >= '0' && *s <= '9' ) {
		s++;
	}

	if( *s == '.' ) {
		s++;
		while( *s >= '0' && *s <= '9' )
			s++;
	}

	/*exponential*/
	if( *s == 'e' || *s == 'E' ) {
		s++;
		if( *s == '+' || *s == '-' )
			s++;
		while( *s >= '0' && *s <= '9' )
			s++;
	}
	
	unsigned int len = s - start;
	if( len >= MAX_NUMBER_BUF ) {
		return mkError( "buffer issue parsing number: increase MAX_NUMBER_BUF (in parser.c) current value is %i\n", MAX_NUMBER_BUF );
	}

	/*copy to buf, which is used with atof*/
	strncpy( buf, start, len );
	buf[ len ] = '\0';

	*next_ch = s;

	PROTECT( p = allocVector( REALSXP, 1 ) );
	REAL(p)[ 0 ] = atof( buf );
	UNPROTECT( 1 );
	return p;
}

