#define NAME    "Yupa"
#define VERSION "v1.3"
#define AUTHOR  "irek@gabr.pl"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include "lib/arg.h"
#include "lib/le.h"
#include "lib/nav.h"
#include "lib/net.h"
#include "lib/tab.h"
#include "lib/uri.h"
#include "lib/util.h"
// Protocols
#include "lib/gmi.h"
#include "lib/gph.h"

#define PROTOCOL URI_GOPHER     // Default protocol

static struct tab  s_tab={0};   // Browser tabs
static char       *s_pager;     // Pager command
char              *argv0;       // First program arg, for arg.h

static const char *s_help =
	NAME " " VERSION " by " AUTHOR "\n"
	"\n"
	"Gopher protocol CLI browser with tabs and browsing history.\n"
	"Browse by inserting absolute URI or link index from current page.\n"
	"Press RETURN to open navigation menu or insert command upfront.\n"
	"Prompt indicate (current_tab_number/number_of_all_tabs).\n"
	"Run program with -h flag to read about arguments and env vars.\n"
	"\n";

// Print usage help message.
static void
usage(void)
{
	printf("usage: %s [-v] [-h] [uri..]\n"
	       "\n"
	       "	-v	Print version.\n"
	       "	-h	Print this help message.\n"
	       "	[uri..]	List of URIs to open on startup.\n"
	       "env	PAGER	Pager cmd (less -XI).\n"
	       , argv0);
}

//
static char *
link_get(int index)
{
	char *uri = 0;
	FILE *raw;
	if (index <= 0) {
		return 0;
	}
	if (s_tab.open->protocol != URI_GOPHER) {
		WARN("Only gopher protocol is supported");
		return 0;
	}
	if (!(raw = fopen(s_tab.open->raw, "r"))) {
		ERR("fopen %s:", s_tab.open->raw);
	}
	switch (s_tab.open->protocol) {
	case URI_GOPHER:
		uri = gph_uri(raw, index);
		break;
	case URI_GEMINI:
		// TODO(irek): The time has come.
		break;
	case URI_FILE:
	case URI_ABOUT:
	case URI_FTP:
	case URI_SSH:
	case URI_FINGER:
	case URI_HTTP:
	case URI_HTTPS:
	case URI_NUL:
	default:
		WARN("Unsupported protocol %d %s", s_tab.open->protocol,
		     uri_protocol_str(s_tab.open->protocol));
	}
	if (fclose(raw) == EOF) {
		ERR("fclose %s:", s_tab.open->raw);
	}
	return uri;
}

//
static int
onuri(char *uri)
{
	enum uri_protocol protocol;
	int sfd, port;
	char buf[4096]={0}, *host, *path, *tmp, item='1', *show;
	FILE *raw, *fmt;
	ssize_t ssiz;
	LOG("%s", uri);
	if (!uri || !uri[0]) {
		return 0;
	}
	assert(strlen(uri) <= URI_SIZ);
	protocol = uri_protocol(uri);
	host = uri_host(uri);
	port = uri_port(uri);
	path = uri_path(uri);
	if (!port) port = protocol;
	if (!port) port = PROTOCOL;
	if (!protocol) protocol = port;
	switch (protocol) {
	case URI_GOPHER:
		if (path && path[1]) {
			item = path[1];
			path += 2;
		}
		break;
	case URI_GEMINI:
		// TODO(irek): Should I do somethin in here?
		break;
	case URI_FILE:
	case URI_ABOUT:
	case URI_FTP:
	case URI_SSH:
	case URI_FINGER:
	case URI_HTTP:
	case URI_HTTPS:
	case URI_NUL:
	default:
		WARN("Unsupported protocol %d %s", s_tab.open->protocol,
		     uri_protocol_str(s_tab.open->protocol));
		return 0;
	}
	//////////////////////////////////////////////////////////////
	//
	// TOOD(irek): This whole "item" logic from Gopher is now leak
	// into general logic used by other protocols.  I should moved
	// it to gph.h somehow.
	//
	// In Gemini similar logic has to be handled on response not
	// on request.  Hmm, maybe what I need is a function for each
	// protocol that is executed before and after request?  Maybe
	// each protocol should have it's own request and response
	// functions.  For example protocol URI_FILE will not do any
	// work on sockets.  So it also makes no sense to have tcp()
	// function as part of the generic logic.
	//
	// Yea, I should do that.
	//
	//////////////////////////////////////////////////////////////
	if (item == '7') {
		// TODO(irek): Make sure there is a way to cancel.
		fputs("enter search query: ", stdout);
		fflush(stdout);
		fgets(buf, sizeof(buf), stdin);
		buf[strlen(buf)-1] = 0;
		if (!buf[0]) { // Empty search
			return 0;
		}
		tmp = JOIN(path, "\t", buf);
		path = tmp;
	}
	if ((sfd = req(host, port, path)) == 0) {
		printf("Invalid URI %s\n", uri);
		return 0;
	}
	if (!(raw = fopen(s_tab.open->raw, "w+"))) {
		ERR("fopen '%s' '%s':", uri, s_tab.open->raw);
	}
	while ((ssiz = recv(sfd, buf, sizeof(buf), 0)) > 0) {
		if (fwrite(buf, 1, ssiz, raw) != (size_t)ssiz) {
			ERR("fwrite %s:", uri);
		}
	}
	if (ssiz < 0) {
		ERR("recv %s:", uri);
	}
	if (close(sfd)) {
		ERR("close %s %d:", uri, sfd);
	}
	// Set tab value only after successful request.
	s_tab.open->protocol = protocol;
	switch (item) {
	case '0':
		show = s_tab.open->raw;
		break;
	case '1':
	case '7':
		show = s_tab.open->fmt;
		break;
	default:
		show = 0;
	}
	if (show == 0) {
		// TODO(irek): Flow of closing this file is ugly.
		// This probably could be refactored with some good
		// old goto.
		if (fclose(raw) == EOF) {
			ERR("fclose %s %s:", uri, s_tab.open->raw);
		}
		printf("Not a Gopher submenu and not a text file\n");
		return 0;
	}
	if (show == s_tab.open->fmt) {
		if (!(fmt = fopen(s_tab.open->fmt, "w"))) {
			ERR("fopen %s %s:", uri, s_tab.open->fmt);
		}
		rewind(raw);
		gph_fmt(raw, fmt);
		if (fclose(fmt) == EOF) {
			ERR("fclose %s %s:", uri, s_tab.open->fmt);
		}
	}
	if (fclose(raw) == EOF) {
		ERR("fclose %s %s:", uri, s_tab.open->raw);
	}
	cmd_run(s_pager, show);
	return 1;
}

//
static void
onprompt(size_t siz, char *buf)
{
	// TODO(irek): I don't like passing the SIZ and the creating
	// LAST buffer with size that is potentially different.  This
	// whole approach is fundamentally wrong.  I should use const
	// value for both.
	static char last[4096] = {0};
	char *arg, *uri;
	int i;
	switch (nav_cmd(buf, &arg)) {
	case CMD_QUIT:
		while (s_tab.n) {
			tab_close(&s_tab, 0);
		}
		exit(0);
		break;
	case CMD_HELP:
		printf(s_help);
		break;
	case CMD_SH_RAW:
		cmd_run(arg, s_tab.open->raw);
		break;
	case CMD_SH_FMT:
		cmd_run(arg, s_tab.open->fmt);
		break;
	case CMD_REPEAT:
		if (*last) {
			strcpy(buf, last);
			onprompt(siz, buf);
		}
		return; // Return to avoid defining CMD_REPEAT as last cmd
	case CMD_URI:
		if (onuri(buf)) {
			past_set(s_tab.open->past, buf);
		}
		break;
	case CMD_LINK:
		uri = link_get(atoi(buf));
		if (onuri(uri)) {
			past_set(s_tab.open->past, uri);
		}
		break;
	case CMD_PAGE_GET:
		onuri(past_get(s_tab.open->past, 0));
		break;
	case CMD_PAGE_RAW:
		cmd_run(s_pager, s_tab.open->raw);
		break;
	case CMD_TAB_GOTO:
		if (arg && (i = atoi(arg))) {
			tab_goto(&s_tab, i-1);
		} else {
			tab_print(&s_tab);
		}
		break;
	case CMD_TAB_ADD:
		tab_open(&s_tab);
		break;
	case CMD_TAB_PREV:
		tab_goto(&s_tab, s_tab.i -1);
		break;
	case CMD_TAB_NEXT:
		tab_goto(&s_tab, s_tab.i +1);
		break;
	case CMD_TAB_OPEN:
		// TODO(irek): Tab duplication should also copy
		// history from current tab.
		if (arg) {
			uri = (i = atoi(arg)) ? link_get(i) : arg;
		} else {
			uri = past_get(s_tab.open->past, 0);
		}
		tab_open(&s_tab);
		if (onuri(uri)) {
			past_set(s_tab.open->past, uri);
		}
		break;
	case CMD_TAB_CLOSE:
		if (s_tab.n <= 1) {
			printf("Can't close last tab\n");
			break;
		}
		tab_close(&s_tab, arg ? atoi(arg)-1 : s_tab.i);
		break;
	case CMD_HIS_LIST:
		// TODO(irek): It might be possible that when I
		// implement the file:// protocol then I could have
		// global history list (for all tabs from current
		// session and the past) as a file in one of protocols
		// like GOPHER.  Then openine history list will be
		// just an opening a file and serving it as regular
		// page in current tab.
		WARN("TODO");
		break;
	case CMD_HIS_PREV:
		uri = past_get(s_tab.open->past, -1);
		onuri(uri);
		break;
	case CMD_HIS_NEXT:
		uri = past_get(s_tab.open->past, +1);
		onuri(uri);
		break;
	case CMD_GET_RAW:
		copy(s_tab.open->raw, arg);
		break;
	case CMD_GET_FMT:
		copy(s_tab.open->fmt, arg);
		break;
	case CMD_CANCEL:
	case CMD_NUL:
		return;
	default:
		ERR("Unreachable");
	}
	strcpy(last, buf);
}

int
main(int argc, char *argv[])
{
	char *env, buf[4096];
	int i;
	ARGBEGIN {
	case 'v':
		printf(VERSION "\n");
		return 0;
	case 'h':
		usage();
		return 0;
	default:
		usage();
		return 1;
	} ARGEND
	s_pager = (env = getenv("PAGER")) ? env : "less -XI";
	for (i = 0; i < argc; i++) {
		tab_open(&s_tab);
		if (onuri(argv[i])) {
			past_set(s_tab.open->past, argv[i]);
		}
	}
	if (!s_tab.n) {
		tab_open(&s_tab);
	}
	while (1) {
		printf("yupa(%d/%d)> ", s_tab.i+1, s_tab.n);
		if (!fgets(buf, sizeof(buf), stdin)) {
			WARN("fgets:");
			continue;
		}
		buf[strlen(buf)-1] = 0;
		onprompt(sizeof(buf), buf);
	}
	return 0;
}
