/* $Id$ */

/***
  This file is part of polypaudio.

  polypaudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2 of the License,
  or (at your option) any later version.

  polypaudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with polypaudio; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "client-conf.h"
#include "xmalloc.h"
#include "log.h"
#include "conf-parser.h"
#include "util.h"
#include "authkey.h"

#ifndef DEFAULT_CONFIG_DIR
# ifndef OS_IS_WIN32
#  define DEFAULT_CONFIG_DIR "/etc/polypaudio"
# else
#  define DEFAULT_CONFIG_DIR "%POLYP_ROOT%"
# endif
#endif

#ifndef OS_IS_WIN32
# define PATH_SEP "/"
#else
# define PATH_SEP "\\"
#endif

#define DEFAULT_CLIENT_CONFIG_FILE DEFAULT_CONFIG_DIR PATH_SEP "client.conf"
#define DEFAULT_CLIENT_CONFIG_FILE_USER ".polypaudio" PATH_SEP "client.conf"

#define ENV_CLIENT_CONFIG_FILE "POLYP_CLIENTCONFIG"
#define ENV_DEFAULT_SINK "POLYP_SINK"
#define ENV_DEFAULT_SOURCE "POLYP_SOURCE"
#define ENV_DEFAULT_SERVER "POLYP_SERVER"
#define ENV_DAEMON_BINARY "POLYP_BINARY"
#define ENV_COOKIE_FILE "POLYP_COOKIE"

static const pa_client_conf default_conf = {
    .daemon_binary = NULL,
    .extra_arguments = NULL,
    .default_sink = NULL,
    .default_source = NULL,
    .default_server = NULL,
    .autospawn = 0,
    .cookie_file = NULL,
    .cookie_valid = 0
};

pa_client_conf *pa_client_conf_new(void) {
    pa_client_conf *c = pa_xmemdup(&default_conf, sizeof(default_conf));
    
    c->daemon_binary = pa_xstrdup(POLYPAUDIO_BINARY);
    c->extra_arguments = pa_xstrdup("--log-target=syslog --exit-idle-time=5");
    c->cookie_file = pa_xstrdup(PA_NATIVE_COOKIE_FILE);
    
    return c;
}

void pa_client_conf_free(pa_client_conf *c) {
    assert(c);
    pa_xfree(c->daemon_binary);
    pa_xfree(c->extra_arguments);
    pa_xfree(c->default_sink);
    pa_xfree(c->default_source);
    pa_xfree(c->default_server);
    pa_xfree(c->cookie_file);
    pa_xfree(c);
}
int pa_client_conf_load(pa_client_conf *c, const char *filename) {
    FILE *f = NULL;
    char *fn = NULL;
    int r = -1;

    /* Prepare the configuration parse table */
    pa_config_item table[] = {
        { "daemon-binary",          pa_config_parse_string,  NULL },
        { "extra-arguments",        pa_config_parse_string,  NULL },
        { "default-sink",           pa_config_parse_string,  NULL },
        { "default-source",         pa_config_parse_string,  NULL },
        { "default-server",         pa_config_parse_string,  NULL },
        { "autospawn",              pa_config_parse_bool,    NULL },
        { "cookie-file",            pa_config_parse_string,  NULL },
        { NULL,                     NULL,                    NULL },
    };

    table[0].data = &c->daemon_binary;
    table[1].data = &c->extra_arguments;
    table[2].data = &c->default_sink;
    table[3].data = &c->default_source;
    table[4].data = &c->default_server;
    table[5].data = &c->autospawn;
    table[6].data = &c->cookie_file;

    f = filename ?
        fopen((fn = pa_xstrdup(filename)), "r") :
        pa_open_config_file(DEFAULT_CLIENT_CONFIG_FILE, DEFAULT_CLIENT_CONFIG_FILE_USER, ENV_CLIENT_CONFIG_FILE, &fn);

    if (!f && errno != EINTR) {
        pa_log(__FILE__": WARNING: failed to open configuration file '%s': %s\n", filename, strerror(errno));
        goto finish;
    }
    
    r = f ? pa_config_parse(fn, f, table, NULL) : 0;

    if (!r)
        r = pa_client_conf_load_cookie(c);
    

finish:
    pa_xfree(fn);

    if (f)
        fclose(f);
    
    return r;
}

int pa_client_conf_env(pa_client_conf *c) {
    char *e;
    
    if ((e = getenv(ENV_DEFAULT_SINK))) {
        pa_xfree(c->default_sink);
        c->default_sink = pa_xstrdup(e);
    }

    if ((e = getenv(ENV_DEFAULT_SOURCE))) {
        pa_xfree(c->default_source);
        c->default_source = pa_xstrdup(e);
    }

    if ((e = getenv(ENV_DEFAULT_SERVER))) {
        pa_xfree(c->default_server);
        c->default_server = pa_xstrdup(e);
    }
    
    if ((e = getenv(ENV_DAEMON_BINARY))) {
        pa_xfree(c->daemon_binary);
        c->daemon_binary = pa_xstrdup(e);
    }

    if ((e = getenv(ENV_COOKIE_FILE))) {
        pa_xfree(c->cookie_file);
        c->cookie_file = pa_xstrdup(e);

        return pa_client_conf_load_cookie(c);
    }
    
    return 0;
}

int pa_client_conf_load_cookie(pa_client_conf* c) {
    assert(c);

    c->cookie_valid = 0;

    if (!c->cookie_file)
        return -1;

    if (pa_authkey_load_auto(c->cookie_file, c->cookie, sizeof(c->cookie)) < 0)
        return -1;

    c->cookie_valid = 1;
    return 0;
}

