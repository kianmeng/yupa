#include "walter.h"
#include "uri.h"

TEST("uri_protocol_str")
{
	SEQ("NONE",	uri_protocol_str(URI_PROTOCOL_NONE));
	SEQ("",		uri_protocol_str(URI_PROTOCOL_NUL));
	SEQ("gopher",	uri_protocol_str(URI_PROTOCOL_GOPHER));
	SEQ("gemini",	uri_protocol_str(URI_PROTOCOL_GEMINI));
	SEQ("https",	uri_protocol_str(URI_PROTOCOL_HTTPS));
	SEQ("http",	uri_protocol_str(URI_PROTOCOL_HTTP));
	SEQ("file",	uri_protocol_str(URI_PROTOCOL_FILE));
	SEQ("ssh",	uri_protocol_str(URI_PROTOCOL_SSH));
	SEQ("ftp",	uri_protocol_str(URI_PROTOCOL_FTP));
}

TEST("uri_protocol")
{
	OK(URI_PROTOCOL_NUL	== uri_protocol(""));
	OK(URI_PROTOCOL_NUL	== uri_protocol("test:70/1/path"));
	OK(URI_PROTOCOL_NUL	== uri_protocol("://test:70/1/path"));
	OK(URI_PROTOCOL_NUL	== uri_protocol("://"));
	OK(URI_PROTOCOL_NONE	== uri_protocol("aaa://test:70/1/path"));
	OK(URI_PROTOCOL_NONE	== uri_protocol("gopherR://test:70/1/path"));
	OK(URI_PROTOCOL_GOPHER	== uri_protocol("gopher://test"));
	OK(URI_PROTOCOL_GOPHER	== uri_protocol("Gopher://test"));
	OK(URI_PROTOCOL_GOPHER	== uri_protocol("GOPHER://test"));
	OK(URI_PROTOCOL_GEMINI	== uri_protocol("gemini://test"));
	OK(URI_PROTOCOL_HTTPS	== uri_protocol("https://test"));
	OK(URI_PROTOCOL_HTTP	== uri_protocol("http://test"));
	OK(URI_PROTOCOL_FILE	== uri_protocol("file://"));
	OK(URI_PROTOCOL_SSH	== uri_protocol("ssh://"));
	OK(URI_PROTOCOL_FTP	== uri_protocol("ftp://"));
}

TEST("uri_host")
{
	OK(0 == uri_host(""));
	OK(0 == uri_host("/"));
	OK(0 == uri_host(":80"));
	OK(0 == uri_host(":80/path/to/file"));
	OK(0 == uri_host("/path/to/file"));
	OK(0 == uri_host("http://:80"));
	SEQ("test", uri_host("test:80"));
	SEQ("test", uri_host("test:80/"));
	SEQ("test", uri_host("test:80/path/to/file"));
	SEQ("test", uri_host("test:/path/to/file"));
	SEQ("test", uri_host("test"));
	SEQ("test", uri_host("gopher://test"));
	SEQ("gabr.pl", uri_host("gopher://gabr.pl"));
	SEQ("gabr.pl", uri_host("gopher://gabr.pl"));
	SEQ("irek.gabr.pl", uri_host("gopher://irek.gabr.pl"));
	SEQ("irek.gabr.pl", uri_host("gopher://irek.gabr.pl/"));
	SEQ("irek.gabr.pl", uri_host("gopher://irek.gabr.pl:80"));
	SEQ("irek.gabr.pl", uri_host("gopher://irek.gabr.pl:80/"));
	SEQ("irek.gabr.pl", uri_host("gopher://irek.gabr.pl:80/path/to/file"));
	SEQ("irek.gabr.pl", uri_host("gopher://irek.gabr.pl/path/to/file"));
	SEQ("irek.gabr.pl", uri_host("irek.gabr.pl"));
	SEQ("irek.gabr.pl", uri_host("irek.gabr.pl/"));
	SEQ("irek.gabr.pl", uri_host("irek.gabr.pl:80"));
	SEQ("irek.gabr.pl", uri_host("irek.gabr.pl:80/"));
	SEQ("irek.gabr.pl", uri_host("irek.gabr.pl:80/path/to/file"));
	SEQ("irek.gabr.pl", uri_host("irek.gabr.pl/path/to/file"));
}

TEST("uri_port")
{
	OK(0 == uri_port(""));
	OK(0 == uri_port("http://"));
	OK(0 == uri_port("gopher://"));
	OK(0 == uri_port("gopher://test"));
	OK(0 == uri_port("gopher://irek.gabr.pl"));
	OK(0 == uri_port("gopher://irek.gabr.pl/"));
	OK(0 == uri_port("gopher://irek.gabr.pl/path/to/file"));
	OK(0 == uri_port("gopher://irek.gabr.pl/path/to/file"));
	OK(0 == uri_port("gopher://irek.gabr.pl:/path/to/file"));
	OK(0 == uri_port("gopher://irek.gabr.pl:"));
	OK(0 == uri_port("gopher://:"));
	OK(0 == uri_port(":"));
	OK(0 == uri_port(":/test/"));
	OK(70 == uri_port(":70"));
	OK(70 == uri_port(":70/test/path"));
	OK(70 == uri_port("host:70"));
	OK(70 == uri_port("sub.sub.sub.host:70"));
	OK(70 == uri_port("gopher://host:70"));
	OK(70 == uri_port("gopher://host:70/1/path/to/file"));
	OK(70 == uri_port("gopher://:70/1/path/to/file"));
	OK(70 == uri_port("gopher://:70"));
	OK(70 == uri_port("://:70/1/path/to/file"));
	OK(70 == uri_port("://:70"));
}

TEST("uri_path")
{
	OK(0 == uri_path(""));
	OK(0 == uri_path("gopher://"));
	OK(0 == uri_path("host"));
	OK(0 == uri_path("sub.sub.sub.host"));
	OK(0 == uri_path(":70"));
	OK(0 == uri_path("host:70"));
	OK(0 == uri_path("gopher://host:70"));
	OK(0 == uri_path("gopher://:70"));
	SEQ("/", uri_path("host/"));
	SEQ("/", uri_path(":70/"));
	SEQ("/", uri_path("/"));
	SEQ("/", uri_path("gopher:///"));
	SEQ("/", uri_path("gopher://:70/"));
	SEQ("/path", uri_path("/path"));
	SEQ("/path", uri_path("host/path"));
	SEQ("/path", uri_path(":70/path"));
	SEQ("/path", uri_path("host:70/path"));
	SEQ("/path", uri_path("gopher:///path"));
	SEQ("/path", uri_path("gopher://host/path"));
	SEQ("/path", uri_path("gopher://:70/path"));
	SEQ("/path", uri_path("gopher://host:70/path"));
	SEQ("/path/to/file", uri_path("gopher://host:70/path/to/file"));
}
