/* vim:set ts=8 sts=4 sw=4 tw=0: */
/*
 * main.c - migemoライブラリテストドライバ
 *
 * Written By:  MURAOKA Taro <koron@tka.att.ne.jp>
 * Last Change: 23-Feb-2004.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>

#include "migemo.h"

#define MIGEMO_ABOUT "cmigemo - C/Migemo Library " MIGEMO_VERSION " Driver"
#define MIGEMODICT_NAME "migemo-dict"
#define MIGEMO_SUBDICT_MAX 8

int try_socket(const char *socket_path, const char *word);
int create_socket(migemo *pmigemo, const char *socket_path);

/*
 * main
 */

    int
query_loop(migemo* p, int quiet)
{
    while (!feof(stdin))
    {
	unsigned char buf[256], *ans;

	if (!quiet)
	    printf("QUERY: ");
	/* gets()を使っていたがfgets()に変更 */
	if (!fgets(buf, sizeof(buf), stdin))
	{
	    if (!quiet)
		printf("\n");
	    break;
	}
	/* 改行をNUL文字に置き換える */
	if ((ans = strchr(buf, '\n')) != NULL)
	    *ans = '\0';

	ans = migemo_query(p, buf);
	if (ans)
	    printf(quiet ? "%s\n" : "PATTERN: %s\n", ans);
	fflush(stdout);
	migemo_release(p, ans);
    }
    return 0;
}

    static void
help(char* prgname)
{
    printf( "\
%s \n\
\n\
USAGE: %s [OPTIONS]\n\
\n\
OPTIONS:\n\
  -c --socket <socket>	Use path <socket> as socket.\n\
  -d --dict <dict>	Use a file <dict> for dictionary.\n\
  -s --subdict <dict>	Sub dictionary files. (MAX %d times)\n\
  -q --quiet		Show no message except results.\n\
  -v --vim		Use vim style regexp.\n\
  -e --emacs		Use emacs style regexp.\n\
  -n --nonewline	Don't use newline match.\n\
  -w --word <word>	Expand a <word> and soon exit.\n\
  -h --help		Show this message.\n\
"
	  , MIGEMO_ABOUT, prgname, MIGEMO_SUBDICT_MAX);
    exit(0);
}

    int
main(int argc, char** argv)
{
    int mode_vim = 0;
    int mode_emacs = 0;
    int mode_nonewline = 0;
    int mode_quiet = 0;
    char* socket = NULL;
    char* dict = NULL;
    char* subdict[MIGEMO_SUBDICT_MAX];
    int subdict_count = 0;
    migemo *pmigemo;
    FILE *fplog = stdout;
    char *word = NULL;
    char *prgname = argv[0];

    memset(subdict, 0, sizeof(subdict));
    while (*++argv)
    {
	if (0)
	    ;
	else if (!strcmp("--vim", *argv) || !strcmp("-v", *argv))
	    mode_vim = 1;
	else if (!strcmp("--emacs", *argv) || !strcmp("-e", *argv))
	    mode_emacs = 1;
	else if (!strcmp("--nonewline", *argv) || !strcmp("-n", *argv))
	    mode_nonewline = 1;
	else if (argv[1] && (!strcmp("--socket", *argv) || !strcmp("-c", *argv)))
	    socket = *++argv;
	else if (argv[1] && (!strcmp("--dict", *argv) || !strcmp("-d", *argv)))
	    dict = *++argv;
	else if (argv[1]
		&& (!strcmp("--subdict", *argv) || !strcmp("-s", *argv))
		&& subdict_count < MIGEMO_SUBDICT_MAX)
	    subdict[subdict_count++] = *++argv;
	else if (argv[1] && (!strcmp("--word", *argv) || !strcmp("-w", *argv)))
	    word = *++argv;
	else if (!strcmp("--quiet", *argv) || !strcmp("-q", *argv))
	    mode_quiet = 1;
	else if (!strcmp("--help", *argv) || !strcmp("-h", *argv))
	    help(prgname);
    }

    if (socket && word) {
	if (try_socket(socket, word)) {
	    return 0;
	}
	/* if socket wasn't found, fallback to standalone */
	socket = NULL;
    }
#ifdef _PROFILE
    fplog = fopen("exe.log", "wt");
#endif

    /* 辞書をカレントディレクトリと1つ上のディレクトリから捜す */
    if (!dict)
    {
	pmigemo = migemo_open("./dict/" MIGEMODICT_NAME);
	if (!word && !mode_quiet)
	{
	    fprintf(fplog, "migemo_open(\"%s\")=%p\n",
		    "./dict/" MIGEMODICT_NAME, pmigemo);
	}
	if (!pmigemo || !migemo_is_enable(pmigemo))
	{
	    migemo_close(pmigemo); /* NULLをcloseしても問題はない */
	    pmigemo = migemo_open("../dict/" MIGEMODICT_NAME);
	    if (!word && !mode_quiet)
	    {
		fprintf(fplog, "migemo_open(\"%s\")=%p\n",
			"../dict/" MIGEMODICT_NAME, pmigemo);
	    }
	}
    }
    else
    {
	pmigemo = migemo_open(dict);
	if (!word && !mode_quiet)
	    fprintf(fplog, "migemo_open(\"%s\")=%p\n", dict, pmigemo);
    }
    /* サブ辞書を読み込む */
    if (subdict_count > 0)
    {
	int i;

	for (i = 0; i < subdict_count; ++i)
	{
	    int result;

	    if (subdict[i] == NULL || subdict[i][0] == '\0')
		continue;
	    result = migemo_load(pmigemo, MIGEMO_DICTID_MIGEMO, subdict[i]);
	    if (!word && !mode_quiet)
		fprintf(fplog, "migemo_load(%p, %d, \"%s\")=%d\n",
			pmigemo, MIGEMO_DICTID_MIGEMO, subdict[i], result);
	}
    }

    if (!pmigemo)
	return 1;
    else
    {
	if (mode_vim)
	{
	    migemo_set_operator(pmigemo, MIGEMO_OPINDEX_OR, "\\|");
	    migemo_set_operator(pmigemo, MIGEMO_OPINDEX_NEST_IN, "\\%(");
	    migemo_set_operator(pmigemo, MIGEMO_OPINDEX_NEST_OUT, "\\)");
	    if (!mode_nonewline)
		migemo_set_operator(pmigemo, MIGEMO_OPINDEX_NEWLINE, "\\_s*");
	}
	else if (mode_emacs)
	{
	    migemo_set_operator(pmigemo, MIGEMO_OPINDEX_OR, "\\|");
	    migemo_set_operator(pmigemo, MIGEMO_OPINDEX_NEST_IN, "\\(");
	    migemo_set_operator(pmigemo, MIGEMO_OPINDEX_NEST_OUT, "\\)");
	    if (!mode_nonewline)
		migemo_set_operator(pmigemo, MIGEMO_OPINDEX_NEWLINE, "[[:space:]\r\n]*");
	}
#ifndef _PROFILE
	if (word)
	{
	    unsigned char *ans;

	    ans = migemo_query(pmigemo, word);
	    if (ans)
		fprintf(fplog, mode_vim ? "%s" : "%s\n", ans);
	    migemo_release(pmigemo, ans);
	}
	else if (socket)
	{
	    create_socket(pmigemo, socket);
	}
	else
	{
	    if (!mode_quiet)
		printf("clock()=%f\n", (float)clock() / CLOCKS_PER_SEC);
	    query_loop(pmigemo, mode_quiet);
	}
#else
	/* プロファイル用 */
	{
	    unsigned char *ans;

	    ans = migemo_query(pmigemo, "a");
	    if (ans)
		fprintf(fplog, "  [%s]\n", ans);
	    migemo_release(pmigemo, ans);

	    ans = migemo_query(pmigemo, "k");
	    if (ans)
		fprintf(fplog, "  [%s]\n", ans);
	    migemo_release(pmigemo, ans);
	}
#endif
	migemo_close(pmigemo);
    }

    if (fplog != stdout)
	fclose(fplog);
    return 0;
}

int try_socket(const char *socket_path, const char *word)
{
    int    fd;
    struct sockaddr_un addr;
    unsigned char buf[2048];

    if ((fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
	perror("socket");
	return 0;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_path);

    if (connect(fd, (struct sockaddr *)&addr,
	     sizeof(addr.sun_family) + strlen(addr.sun_path)) < 0){
	perror("connect");
	return 0;
    }

    int n = strlen(word)+1;
    if (write(fd, word, n) != n) {
	perror("write");
	return 0;
    }
    while ((n = read(fd, buf, sizeof(buf)-1)) > 0) {
	if (buf[n-1] == 0) {
	    fwrite(buf, 1, n-1, stdout);
	    break;
	}
	fwrite(buf, 1, n, stdout);
    }
    close(fd);

    return 1;
}

int create_socket(migemo *pmigemo, const char *socket_path)
{
    int    fd1, fd2;
    struct sockaddr_un saddr;
    struct sockaddr_un caddr;
    unsigned char buf[2048], *ans;

    if ((fd1 = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
	perror("socket");
	return 0;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sun_family = AF_UNIX;
    strcpy(saddr.sun_path, socket_path);

    unlink(socket_path);
    if (bind(fd1, (struct sockaddr *)&saddr,
	     sizeof(saddr.sun_family) + strlen(saddr.sun_path)) < 0){
	perror("bind");
	return 0;
    }

    if (listen(fd1, 1) < 0) {
	perror("listen");
	return 0;
    }

    if (daemon(1, 0) == -1) {
	perror("daemon");
	return 0;
    }

    strcpy(buf, socket_path);
    strcat(buf, ".pid");
    pid_t pid = getpid();
    FILE *fp = fopen(buf, "wb");
    if (fp) {
	fprintf(fp, "%lu", (unsigned long)pid);
	fclose(fp);
    }

    int len = sizeof(caddr);
    while (1)
    {
	if ((fd2 = accept(fd1, (struct sockaddr *)&caddr, (socklen_t *)&len)) < 0)
	{
	    perror("accept");
	    return 0;
	}
	unsigned char byte;
	size_t pos = 0;
	while (pos < sizeof(buf)-1 && read(fd2, &byte, 1) > 0)
	{
	    if (byte == 0) break;
	    buf[pos++] = byte;
	}
	buf[pos] = 0;
	ans = migemo_query(pmigemo, buf);
	if (ans) {
	    write(fd2, ans, strlen(ans)+1);
	}
	migemo_release(pmigemo, ans);
	close(fd2);
    }
    close(fd1);

    return 0;
}

