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

static REQUEST_METHOD method_name_to_enum(char *name) {

	if (name == NULL) {
		return RM_INVALID;
	}

	for (int i = 0; i < RM_COUNT; i++) {
		if (strcmp(method_names[i], name) == 0) {
			return i;
		}
	}

	return RM_INVALID;
}

static const char *header_names[] = {
#define XX(num, name, string) string,
	PARSER_HEADER_MAP(XX)
#undef XX
};

static REQUEST_HEADER header_name_to_enum(char *name) {

	if (name == NULL) {
		return RH_INVALID;
	}

	for (int i = 0; i < RH_COUNT; i++) {
		if (strcmp(header_names[i], name) == 0) {
			return i;
		}
	}

	return RH_INVALID;
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
		char *value = parser->headers[i];
		if (value != NULL) {
			printf("%s: %s\n", header_names[i], value);
		}
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

static int parse_request_line(rp_parser *parser, char *line, int *bytes_leftover) {

	char *method;
	char *resource;
	char *version;

	// TODO: strtok is not thread safe, gotta fix this for multiple connections!!!
	if ((method = strtok(line, " ")) == NULL ||
		(resource = strtok(NULL, " ")) == NULL ||
		(version = strtok(NULL, " ")) == NULL) {

		return ERR_MAL_DATA;
	}

	// Set parser's request line fields
	parser->method = method_name_to_enum(method);

	parser->resource = (char *) malloc((strlen(resource) + 1) * sizeof(char));
	strcpy(parser->resource, resource);

	parser->version = (char *) malloc((strlen(version) + 1) * sizeof(char));
	strcpy(parser->version, version);

	// Update parser's progress
	parser->request_line_completed = 1;

	return 0;
}

static int parse_header_line(rp_parser *parser, char *line, int *bytes_leftover) {

	printf("Parsing header line\n");

	char *name;
	char *value;

	// TODO: change strtok to something thread safe
	if ((name = strtok(line, ": ")) == NULL ||
		(value = strtok(NULL, " ")) == NULL) {

		return ERR_MAL_DATA;
	}

	printf("Header name: %s\n", name);
	printf("Header value: %s\n", value);

	REQUEST_HEADER header = header_name_to_enum(name);
	if (header == RH_INVALID) {
		return ERR_MAL_DATA;
	}

	parser->headers[header] = (char *) malloc((strlen(value) + 1) * sizeof(char));
	if (parser->headers[header] == NULL) {
		return ERR_NO_MEM;
	}

	strcpy(parser->headers[header], value);

	return 0;
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

		int (*line_parser) (rp_parser*, char*, int*) = 
			(parser->request_line_completed) ? parse_header_line : parse_request_line;

		// Replace newline with null terminator
		*line_end = '\0';

		if ((error = line_parser(parser, line_start, bytes_leftover))) { return error; }

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