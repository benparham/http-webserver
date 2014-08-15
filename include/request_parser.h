#ifndef _REQUEST_PARSER_H_
#define _REQUEST_PARSER_H_

#define CRLF	"\r\n"

//=========================== ERROR HANDLING ==============
#define PARSER_ERROR_MAP(XX)						\
	XX(0, NONE, "No error")							\
	XX(1, NOT_IMPLEMENTED, "Not yet implemented")	\
	XX(2, INV_ARG, "Invalid argument")				\
	XX(3, NO_MEM, "Not enough memory")				\
	XX(4, MAL_DATA, "Malformed data")				\

typedef enum PARSE_ERROR {
#define XX(num, name, string) ERR_##name = num,
	PARSER_ERROR_MAP(XX)
#undef XX
} PARSE_ERROR;

const char* rp_strerr(int errnum);		// Returns the parser error message corresponding to errno


//=========================== REQUEST METHODS ==============
#define PARSER_METHOD_MAP(XX)	\
	XX(0, INVALID, "INVALID")	\
	XX(1, OPTIONS, "OPTIONS")	\
	XX(2, GET, "GET")			\
	XX(3, HEAD, "HEAD")			\
	XX(4, POST, "POST")			\
	XX(5, PUT, "PUT")			\
	XX(6, DELETE, "DELETE")		\
	XX(7, TRACE, "TRACE")		\
	XX(8, CONNECT, "CONNECT")	\

typedef enum REQUEST_METHOD {
#define XX(num, name, string) RM_##name = num,
	PARSER_METHOD_MAP(XX)
#undef XX
	RM_COUNT
} REQUEST_METHOD;


//=========================== REQUEST HEADERS ==============
#define PARSER_HEADER_MAP(XX)							\
	XX(0, INVALID, "Invalid")							\
	XX(1, ACCEPT, "Accept")								\
	XX(2, ACCEPT_CHARSET, "Accept-Charset")				\
	XX(3, ACCEPT_ENCODING, "Accept-Encoding")			\
	XX(4, ACCEPT_LANGUAGE, "Accept-Language")			\
	XX(5, ACCEPT_DATETiME, "Accept-Datetime")			\
	XX(6, AUTHORIZATION, "Authorization")				\
	XX(7, CACHE_CONTROL, "Cache-Control")				\
	XX(8, CONNECTION, "Connection")						\
	XX(9, COOKIE, "Cookie")								\
	XX(10, CONTENT_LENGTH, "Content-Length")			\
	XX(11, CONTENT_MD5, "Content-MD5")					\
	XX(12, CONTENT_TYPE, "Content-Type")				\
	XX(13, DATE, "Date")								\
	XX(14, EXPECT, "Expect")							\
	XX(15, FROM, "From")								\
	XX(16, HOST, "Host")								\
	XX(17, IF_MATCH, "If-Match")						\
	XX(18, IF_MODIFIED_SINCE, "If-Modified-Since")		\
	XX(19, IF_NONE_MATCH, "If-None-Match")				\
	XX(20, IF_RANGE, "If-Range")						\
	XX(21, IF_UNMODIFIED_SINCE, "If-Unmodified-Since")	\
	XX(22, MAX_FORWARDS, "Max-Forwards")				\
	XX(23, ORIGIN, "Origin")							\
	XX(24, PRAGMA, "Pragma")							\
	XX(25, PROXY_AUTHORIZATION, "Proxy-Authorization")	\
	XX(26, RANGE, "Range")								\
	XX(27, REFERER, "Referer")							\
	XX(28, TE, "TE")									\
	XX(29, USER_AGENT, "User-Agent")					\
	XX(30, UPGRADE, "Upgrade")							\
	XX(31, VIA, "Via")									\
	XX(32, WARNING, "Warning")							\

typedef enum REQUEST_HEADER {
#define XX(num, name, string) RH_##name = num,
	PARSER_HEADER_MAP(XX)
#undef XX	
	RH_COUNT
} REQUEST_HEADER;


//=========================== PARSING DATA STRUCTURE ==============
// Container for all (parsed) request information
typedef struct rp_parser {

	// Start line information
	REQUEST_METHOD method;
	char *resource;
	char *version;

	// Request headers
	/*
	 * void* array points to values of request headers
	 * NULL indicates the value was not set by the request for header in question
	 */
	char *headers[RH_COUNT];

	// Request body
	/* 
	 * NULL indicates no body was set by request
	 */
	void *body;

	// Progress metadata
	char request_line_completed : 1;
	char headers_completed : 1;
	char completed : 1;

} rp_parser;

void rp_parser_print(rp_parser *parser);


//=========================== PARSING FUNCTIONS ==============
int rp_parser_reset(rp_parser* parser);		// Reinitializes existing parser
int rp_parser_create(rp_parser** parser);	// Allocates and initializes a new parser
void rp_parser_destroy(rp_parser* parser);	// Deallocates parser

/*
 * Updates PARSER by parsing the contents of BUF
 * Returns 0 on success, 1 on failure
 */
int rp_parse(rp_parser *parser, char *buf, int *bytes_leftover);

#endif