#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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



static const char *method_names[] = {
#define XX(num, name, string) string,
	PARSER_METHOD_MAP(XX)
#undef XX
};

static const char *header_names[] = {
#define XX(num, name, string) string,
	PARSER_HEADER_MAP(XX)
#undef XX
};

static void header_print(REQUEST_HEADER header, void *value) {

	// Check valid
	if (value == NULL) {
		return;
	}

	// Print header name
	printf("%s: ", header_names[header]);

	// Print header value
	switch(header) {
		case RH_ACCEPT:
			printf("Accept header value\n");
		default:
			printf("Not yet implemented\n");
	}
}

void rp_parser_print(rp_parser *parser) {
	printf("========= Parser Printout =========\n");

	if (parser == NULL) {
		printf("Parser is NULL\n");
		return;
	}

	printf("Method: %s\n", method_names[parser->method]);
	printf("Resource: %s\n", parser->resource == NULL ? "NULL" : parser->resource);
	printf("Version: %s\n", parser->version == NULL ? "NULL" : parser->version);

	printf("Headers:\n");
	for (int i = 0; i < RH_COUNT; i++) {
		header_print(i, parser->headers[i]);
	}

	printf("===================================\n");
}



int rp_parser_reset(rp_parser* parser) {

	// Check valid
	if (parser == NULL) {
		return ERR_INV_ARG;
	}

	// Initialize fields
	parser->method = RM_INVALID;
	parser->resource = NULL;
	parser->version = NULL;

	for (int i = 0; i < RH_COUNT; i++) {
		parser->headers[i] = NULL;
	}

	parser->body = NULL;

	parser->request_line_completed = 0;
	parser->headers_completed = 0;
	parser->completed = 0;

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

static int parse_body(rp_parser *parser, char *buf) {

	return ERR_NOT_IMPLEMENTED;
}

static int parse_request_line(rp_parser *parser, char *line_start, char *line_end, int *bytes_leftover) {


	return ERR_NOT_IMPLEMENTED;
}

static int parse_header_line(rp_parser *parser, char *line_start, char *line_end, int *bytes_leftover) {

	return ERR_NOT_IMPLEMENTED;
}

int rp_parse(rp_parser *parser, char *buf, int *bytes_leftover) {
	
	int error;

	// Check valid
	if (parser == NULL || buf == NULL || bytes_leftover == NULL) {
		return ERR_INV_ARG;
	}

	// Parse request body
	if (parser->headers_completed) {
		if ((error = parse_body(parser, buf))) { return error; }
		
		*bytes_leftover = 0;
		return 0;
	}

	// Parser request line and headers
	char *line_start = buf;
	char *line_end;
	while ((line_end = strchr(line_start, '\n')) != NULL) {

		int (*line_parser) (rp_parser*, char*, char*, int*) = 
			(parser->request_line_completed) ? parse_header_line : parse_request_line;

		if ((error = line_parser(parser, line_start, line_end, bytes_leftover))) { return error; }

		line_start = line_end + 1;
		
		// Newline was the last char in buf
		if (line_start == NULL) {
			*bytes_leftover = 0;
			return 0;
		}
	}

	*bytes_leftover = strlen(line_start);
	
	// Nothing in buf needs to be shifted
	int leftover_idx = line_start - buf;
	if (leftover_idx == 0) {
		return 0;
	}

	// Shift leftover bytes to beginning of buf
	for (int i = 0; i < *bytes_leftover; i++) {
		buf[i] = buf[leftover_idx + i];
	}

	return 0;
}