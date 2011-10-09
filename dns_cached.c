#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * dns_cached = simple wrapper for djbdns dnscache so that you don't
 * need to install daemontools, you can just use /etc/inittab
 *
 * USAGE:
 *	dns_cached [CONFIG_DIRECTORY]
 *
 * if you pointed dnscache-conf towards something other than /etc/dnscache
 * pass that directory in as CONFIG_DIRECTORY
 */

void strip(char *str) {
	int len = strlen(str);
	if (str[ len - 1 ] == '\n')
		str[ len - 1 ] = '\0';
}

void _setenv(char *key, char *value) {
	printf("setenv %s = %s\n", key, value);
	setenv( (const char *) key, (const char *) value, 1);
}

// import files in dir
int env_load_from_dir(char *dir) {
	struct dirent *de;
	char value[256];
	FILE *f;
	DIR *d = opendir(dir);

	chdir(dir);
	while ( (de = readdir(d)) != NULL ) {
		bzero(value, sizeof(value));
		if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
			continue;
		f = fopen(de->d_name, "r");
		if (f == NULL)
			continue;
		fread(value, sizeof(value), 1, f);
		strip(value);
		fclose(f);
		_setenv( de->d_name, value );
	}
	return 0;
}

int main(int argc, char **argv) {
	char home_dir[256];
	char env_dir[256];
	char seed_file[256];
	int ret;

	strcpy( home_dir, "/etc/dnscache");
	if (argc > 1)
		strncpy( home_dir, argv[1], strlen(home_dir) );

	strncpy( env_dir, home_dir, sizeof(env_dir) );
	strcat( env_dir, "/env" );

	strncpy( seed_file, home_dir, sizeof(seed_file) );
	strcat( seed_file, "/seed" );

	char *defaults[][2] = {
		{"UID",           "nobody"},
		{"GID",           "nobody"},
		{"IP",            "127.0.0.1"},
		{"IPSEND",        "0.0.0.0"},
		{"ROOT",          "/etc/dnscache/root"},
		{"CACHESIZE",     "1000000"},
		{"HIDETTL",       ""},
		{"DNS_CACHE_BIN", "/usr/local/bin/dnscache"},
		{NULL,            NULL},
	};

	int i;
	for ( i = 0; defaults[i][0] != NULL; i++) {
		setenv( (const char *)defaults[i][0], (const char *)defaults[i][1], 1);
	}
	env_load_from_dir(env_dir);
	chdir(home_dir);

	ret = open(seed_file, O_RDONLY);
	if (ret < 0)
		fprintf(stderr, "failed to open seed_file: %s\n", seed_file);

	ret = dup2(ret, 0);
	if (ret != 0)
		fprintf(stderr, "failed to dup2 open seed_file: %s\n", seed_file);

	return execl( getenv("DNS_CACHE_BIN"), getenv("DNS_CACHE_BIN"), (char *)NULL );
}
