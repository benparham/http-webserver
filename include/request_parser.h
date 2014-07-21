#ifndef _REQUEST_PARSER_H_
#define _REQUEST_PARSER_H_

//=========================== ERROR HANDLING ==============
#define PARSER_ERROR_MAP(XX)						\
	XX(0, NONE, "No error")							\
	XX(1, NOT_IMPLEMENTED, "Not yet implemented")	\
	XX(2, INV_ARG, "Invalid argument")				\
	XX(3, NO_MEM, "Not enough memory")				\

typedef enum PARSE_ERROR {
#define XX(num, name, string) ERR_##name = num,
	PARSER_ERROR_MAP(XX)
#undef XX
} PARSE_ERROR;

const char* rp_strerr(int errnum);		// Returns the parser error message corresponding to errno
// ========================================================


// Possible request methods
typedef enum REQUEST_METHOD {
	RM_OPTIONS,
	RM_GET,
	RM_HEAD,
	RM_POST,
	RM_PUT,
	RM_DELETE,
	RM_TRACE,
	RM_CONNECT,
	RM_COUNT
} REQUEST_METHOD;

// List of request headers
typedef enum REQUEST_HEADER {
	RH_ACCEPT,
	// TODO: add all possible request headers
	RH_COUNT
} REQUEST_HEADER;

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

} rp_parser;

int rp_parser_reset(rp_parser* parser);		// Reinitializes existing parser
int rp_parser_create(rp_parser** parser);	// Allocates and initializes a new parser
void rp_parser_destroy(rp_parser* parser);	// Deallocates parser

/*
 * Updates PARSER by parsing the contents of BUF
 * Fails if COMPLETED is not 0, sets COMPLETED to 1 if BUF contains end of request
 * Returns 0 on success, 1 on failure
 */
int rp_parse(rp_parser *parser, char *buf, int *bytes_left, int *completed);

#endif