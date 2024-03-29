//
//  tracker.h
//  
//
//  Created by Michael Pastushkov on 9/5/22.
//

#ifndef tracker_h
#define tracker_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <mysql/mysql.h>

#define MAX_REMOTE_HOSTS 4
#define MODE_LOCAL 1
#define MODE_REMOTE 2

int mode;
extern int repeat_time;
extern char status_log[256];

extern int check_cn;
extern int check_undef;
extern int kill_connections;

extern int daily_limit_mib;

extern MYSQL *con;
extern char db_host[32];
extern char db_user[32];
extern char db_pass[32];
extern char db_schema[32];

extern char geoip_data[256];
extern int geoip_on;
extern int log_level;

typedef struct {
    char host[32];
    int port;
} remote_host;
extern remote_host remote_hosts[];

typedef struct _session {
    char *cn;
    long bin;
    long bout;
    time_t stime;
    char *ip4;
    char *port;
    char country[64];
    char city[64];
    char *source;
} session;

int db_open();
void db_close();
int db_query(char *query);

int read_local();
int read_remote();
int connect_remote(char *mhost, int port);
int close_remote();
int read_line(int sockfd, char *line, size_t len);
remote_host* get_host_by_source(char *source);

int proc_line(char *line, char *source);
int read_config();
int get_ip4_info(session *ses);
int check_alerts();
int db_cleanup();

void remove_char(char* s, char c);
int log_printf (int level, const char *format, ...);

#endif /* tracker_h */
