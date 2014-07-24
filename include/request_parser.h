#ifndef _REQUEST_PARSER_H_
#define _REQUEST_PARSER_H_

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
	XX(0, INVALID, "invalid")	\
	XX(1, OPTIONS, "options")	\
	XX(2, GET, "get")			\
	XX(3, HEAD, "head")			\
	XX(4, POST, "post")			\
	XX(5, PUT, "put")			\
	XX(6, DELETE, "delete")		\
	XX(7, TRACE, "trace")		\
	XX(8, CONNECT, "connect")	\

typedef enum REQUEST_METHOD {
#define XX(num, name, string) RM_##name = num,
	PARSER_METHOD_MAP(XX)
#undef XX
	RM_COUNT
} REQUEST_METHOD;


//=========================== REQUEST HEADERS ==============
#define PARSER_HEADER_MAP(XX)	\
	XX(0, INVALID, "invalid")	\
	XX(1, ACCEPT, "accept")		\
	// TODO: add all possible request headers

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
	void *headers[RH_COUNT];

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