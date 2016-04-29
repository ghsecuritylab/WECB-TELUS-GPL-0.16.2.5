/*
 * Copyright (C) 2006 Rich Felker <dalias@aerifal.cx>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdlib.h>
#include <string.h>

/* Experimentally off - libc_hidden_proto(strchr) */
/* Experimentally off - libc_hidden_proto(strlen) */
/* Experimentally off - libc_hidden_proto(strncmp) */

int getsubopt(char **opt, char *const *keys, char **val)
{
	char *s = *opt;
	int i;

	*val = NULL;
	*opt = strchr(s, ',');
	if (*opt) *(*opt)++ = 0;
	else *opt = s + strlen(s);

	for (i=0; keys[i]; i++) {
		size_t l = strlen(keys[i]);
		if (strncmp(keys[i], s, l)) continue;
		if (s[l] == '=')
			*val = s + l;
		else if (s[l]) continue;
		return i;
	}
	return -1;
}
