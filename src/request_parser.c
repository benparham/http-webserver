#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <request_parser.h>
#include <global.h>


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
	parser->body_bytes = 0;

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

static int parse_request_line(rp_parser *parser, char *line) {

	char *method;
	char *resource;
	char *version;

	// TODO: strtok is not thread safe, gotta fix this for multiple connections!!!
	if ((method = strtok(line, " ")) == NULL ||
		(resource = strtok(NULL, " ")) == NULL ||
		(version = strtok(NULL, CRLF)) == NULL) {

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

static int parse_header_line(rp_parser *parser, char *line) {

	char *name;
	char *value;

	// TODO: change strtok to something thread safe
	if ((name = strtok(line, ":")) == NULL ||
		(value = strtok(NULL, CRLF)) == NULL) {

		return ERR_MAL_DATA;
	}

	// Remove whitespace from value
	while(value[0] == ' ') {
		value += 1;
	}
	// Catch all whitespace value
	if (value == NULL) {
		return ERR_MAL_DATA;
	}

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

static int parser_has_body(rp_parser *parser) {

	char *content_length = parser->headers[RH_CONTENT_LENGTH];
	if (content_length != NULL &&
		atoi(content_length) > 0) {
		return 1;
	}

	char *transfer_encoding = parser->headers[RH_TE];
	if (transfer_encoding != NULL &&
		strcmp(transfer_encoding, "chunked") == 0) {
		return 1;
	}

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

	int initial_bytes = strlen(buf);
	*bytes_leftover = initial_bytes;

	// Parser request line and headers
	char *line_start = buf;
	char *line_end;
	while ((line_end = strstr(line_start, CRLF)) != NULL) {

		// Catch end of header fields
		if (line_end == line_start) {

			if (parser->headers_completed) {
				return ERR_MAL_DATA;
			}

			parser->headers_completed = 1;
			
			if (parser_has_body(parser)) {
				line_start += strlen(CRLF);
				if ((error = parse_body(parser, line_start))) { return error; }
			} else {
				parser->completed = 1;
			}

			*bytes_leftover = 0;
			return 0;
		}

		// Move line end to second half of CRLF
		line_end += 1;

		int line_length = (line_end - line_start) + 1;

		// Choose correct line parser
		int (*line_parser) (rp_parser*, char*) = 
			(parser->request_line_completed) ? parse_header_line : parse_request_line;

		// Parse line
		if ((error = line_parser(parser, line_start))) { return error; }

		// Adjust bytes leftover
		*bytes_leftover -= line_length;

		// Increment
		line_start = line_end + 1;
	}
	
	// Shift leftover bytes to beginning of buf
	int leftover_idx = initial_bytes - *bytes_leftover;

	if (leftover_idx > 0) {
		int i;
		for (i = 0; i < *bytes_leftover; i++) {
			buf[i] = buf[leftover_idx + i];
		}

		// Null terminate leftover bytes
		buf[i] = '\0';
	}

	printf("Returning leftover: %s\n", buf);

	return 0;
}