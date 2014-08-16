#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define CRLF	"\r\n"		// Carriage Return - Line Feed

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

static const char *error_messages[] = {
#define XX(num, name, string) string,
	PARSER_ERROR_MAP(XX)
#undef XX
};

// Returns the parser error message corresponding to errno
static inline const char* rp_strerr(int errnum) {
	return error_messages[errnum];
}

#endif