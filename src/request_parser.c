#include <stdio.h>
#include <stdlib.h>

#include <request_parser.h>

//=========================== ERROR HANDLING ==============
static const char *error_messages[] = {
#define XX(num, name, string) string,
	PARSER_ERROR_MAP(XX)
#undef XX
};

const char* rp_strerr(int errnum) {
	return error_messages[errnum];
}
// ========================================================

int rp_parser_reset(rp_parser* parser) {

	// Check valid
	if (parser == NULL) {
		return ERR_INV_ARG;
	}

	// Initialize fields
	parser->resource = NULL;
	parser->version = NULL;

	for (int i = 0; i < RH_COUNT; i++) {
		parser->headers[i] = NULL;
	}

	parser->body = NULL;

	return 0;
}

int rp_parser_create(rp_parser** parser) {

	// Check valid
	if (*parser != NULL) {
		return ERR_INV_ARG;
	}

	// Allocate
	*parser = (rp_parser *) malloc(sizeof(rp_parser));
	if (*parser == NULL) {
		return ERR_NO_MEM;
	}

	return rp_parser_reset(*parser);
}

void rp_parser_destroy(rp_parser* parser) {

	if (parser != NULL) {

		if (parser->resource != NULL) { free(parser->resource); }
		if (parser->version != NULL) { free(parser->version); }

		for (int i = 0; i < RH_COUNT; i++) {
			if (parser->headers[i] != NULL) { free(parser->headers[i]); }
		}

		if (parser->body != NULL) { free(parser->body); }

		free(parser);
	}
}

int rp_parse(rp_parser *parser, char *buf, int *bytes_left, int *completed) {
	
	if (parser == NULL || buf == NULL || bytes_left == NULL || completed == NULL) {
		return ERR_INV_ARG;
	}

	return ERR_NOT_IMPLEMENTED;
}