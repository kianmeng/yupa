// URI (Universal Resource Identifier) lib to parse and build URI.

#ifndef _URI_H
#define _URI_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define URI_SIZ 1024            // Max size of URI

// Protocols with cooresponding default ports.  In this lib protocol
// and port are essentially the same thing.  OFC you can have custom
// port number but protocol for it will still be a number being the
// default port number.
enum uri_protocol {
	URI_NUL    = 0,         // For empty or unknown protocol
	URI_FILE   = 1,         // Reading files from local drive
	URI_ABOUT  = 2,         // For browser generated pages
	URI_FTP    = 21,
	URI_SSH    = 22,
	URI_GOPHER = 70,
	URI_FINGER = 79,
	URI_HTTP   = 80,
	URI_HTTPS  = 443,
	URI_GEMINI = 1965,
};

// For given PROTOCOL return protocol name as string.
char *uri_protocol_str(enum uri_protocol protocol);

// Get protocol default port from URI string.
int uri_protocol(char *uri);

// Get host from URI string.  Return NULL if not found.  Return
// pointer to static string on success.
char *uri_host(char *uri);

// Get port from URI string.  Return 0 by default.
int uri_port(char *uri);

// Return pointer to first URI string path character.  Return NULL if
// not found.
char *uri_path(char *uri);

// Construct normalized URI in DST buffer from given URI parts, where
// only PATH can be NULL.
void uri_normalize(char dst[URI_SIZ], int protocol, char *host,
		   int port, char *path);

#endif // _URI_H
