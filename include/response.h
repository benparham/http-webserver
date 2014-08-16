#ifndef _RESPONSE_H_
#define _RESPONSE_H_

#include <request_parser.h>

//=========================== RESPONSE HEADERS ==============
#define RESPONSE_HEADER_MAP(XX)											\
	XX(0, INVALID, "Invalid")											\
	XX(1, ACCESS_CONTROL_ALLOW_ORIGIN, "Access-Control-Allow_Origin")	\
	XX(2, ACCEPT_RANGES, "Accept-Ranges")								\
	XX(3, AGE, "Age")													\
	XX(4, ALLOW, "Allow")												\
	XX(5, CACHE_CONTROL, "Cache-Control")								\
	XX(6, CONNECTION, "Connection")										\
	XX(7, CONTENT_ENCODING, "Conent-Encoding")							\
	XX(8, CONTENT_LANGUAGE, "Content-Language")							\
	XX(9, CONTENT_LENGTH, "Content-Length")								\
	XX(10, CONTENT_LOCATION, "Content-Location")						\
	XX(11, CONTENT_MD5, "Content-MD5")									\
	XX(12, CONTENT_DISPOSITION, "Content-Disposition")					\
	XX(13, CONTENT_RANGE, "Content-Range")								\
	XX(14, CONTENT_TYPE, "Content-Type")								\
	XX(15, DATE, "Date")												\
	XX(16, ETAG, "ETag")												\
	XX(17, EXPIRES, "Expires")											\
	XX(18, LAST_MODIFIED, "Last-Modified")								\
	XX(19, LINK, "Link")												\
	XX(20, LOCATION, "Location")										\
	XX(21, P3P, "P3P")													\
	XX(22, PRAGMA, "Pragma")											\
	XX(23, PROXY_AUTHENTICATE, "Proxy-Authenticate")					\
	XX(24, REFRESH, "refresh")											\
	XX(25, RETRY_AFTER, "Retry-After")									\
	XX(26, SERVER, "Server")											\
	XX(27, SET_COOKIE, "Set-Cookie")									\
	XX(28, STATUS, "Status")											\
	XX(29, STRICT_TRANSPORT_SECURITY, "Strict-Transport-Security")		\
	XX(30, TRAILER, "Trailer")											\
	XX(31, TE, "TE")													\
	XX(32, UPGRADE, "Upgrade")											\
	XX(33, VARY, "Vary")												\
	XX(34, VIA, "Via")													\
	XX(35, WARNING, "Warning")											\
	XX(36, WWW_AUTHENTICATE, "WWW-Authenticate")						\
	XX(37, X_FRAME_OPTIONS, "X-Frame-Options")							\

typedef enum RESPONSE_HEADER {
#define XX(num, name, string) RESH_##name = num,
	RESPONSE_HEADER_MAP(XX)
#undef XX	
	RESH_COUNT
} RESPONSE_HEADER;


//=========================== RESPONSE DATA STRUCTURE ==============
// Container for response information
typedef struct res_response {

	// Status Line Information
	char *version;
	int status_code;
	char *phrase;

	// Header Information
	char *headers[RESH_COUNT];

	// Response body
	/* 
	 * NULL indicates no body was set for response
	 */
	void *body;
	int body_bytes;

} res_response;

void res_response_print(res_response *response);


//=========================== RESPONSE FUNCTIONS ==============
int res_response_create(res_response** response);	// Allocates and initializes a new response
void res_response_destroy(res_response* response);	// Deallocates response

int res_response_generate(res_response *response, rp_parser *parser);	// Generate a response to the request held in 'parser'
int res_response_send(res_response *response, int socket_fd);

#endif