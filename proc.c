//
//  proc.c
//
//  Data processing
//
//  Created by Michael Pastushkov on 9/7/22.
//

#include "tracker.h"

#define CLIENT_LIST "CLIENT_LIST"
#define TIME_FORMAT "%Y-%m-%d %H:%M:%S"

int check_session(session *ses, char *tf) {

    char query[1024];
    int num;
    sprintf(query, "SELECT count(*) FROM sessions WHERE \
            ip4 = '%s' AND port = %s AND stime = '%s'",
            ses->ip4, ses->port, tf);
    
    db_query(query);

    MYSQL_RES *result = mysql_store_result(con);
    if (!result) {
        log_printf(0, "SQL results error\n");
        return -1;
    }
    
    MYSQL_ROW row = mysql_fetch_row(result);
    num = atoi(row[0]);
    
    mysql_free_result(result);

    return num;
}

int record_session(session *ses) {
    
    char tf[32], tfn[32];
    char query[1024];
    time_t now;

    time(&now);
    strftime(tf, sizeof(tf), TIME_FORMAT, localtime(&ses->stime));
    strftime(tfn, sizeof(tfn), TIME_FORMAT, localtime(&now));

    if (check_session(ses, tf)) {
        // Update session numbers
        sprintf(query, "UPDATE sessions SET bin=%ld, bout=%ld, \
                cn = '%s', etime='%s', source='%s', country = '%s', city = '%s' \
                WHERE ip4 = '%s' AND port = %s AND stime = '%s'",
                ses->bin, ses->bout, ses->cn, tfn, ses->source,
                ses->country, ses->city, ses->ip4, ses->port, tf);
        if (db_query(query) != 0)
            return -1;
    } else {
        // New session
        sprintf(query, "INSERT INTO sessions \
                (cn, bin, bout, stime, etime, ip4, port, country, city, source) \
                VALUES ('%s', %ld, %ld, '%s', '%s', '%s', %s, '%s', '%s', '%s')",
                ses->cn, ses->bin, ses->bout, tf, tfn,
                ses->ip4, ses->port, ses->country, ses->city, ses->source);
        if (db_query(query) != 0)
            return -1;
    }
    
    return 0;
}

int proc_line(char *line, char *source) {

    int i, j, e = 0;
    char *s = line;
    session ses;

    memset(&ses, 0, sizeof(ses));
    ses.source = source;

    log_printf(3, " proc_line %s\n", line);

    if (strncmp(CLIENT_LIST, line, strlen(CLIENT_LIST)) != 0) {
        return -1;
    }
    
    int len = strlen(line);
    for (i=0; i<len; i++) {
        if (line[i] == ',') {
            line[i] = 0;
            switch (e) {
                case 1:
                    ses.cn = s;
                    remove_char(ses.cn, ' ');
                    break;
                case 2:
                    ses.ip4 = s;
                    for (j=0; ses.ip4[j]; j++) { // filter out port
                        if (ses.ip4[j] == ':') {
                            ses.ip4[j] = 0;
                            ses.port = ses.ip4 + j + 1;
                            break;
                        }
                    }
                    break;
                 case 5:
                    ses.bin = atol(s);
                    break;
                case 6:
                    ses.bout = atol(s);
                    break;
                case 8:
                    ses.stime = (time_t)atol(s);
                    break;
            }
            s = line + i + 1;
            e++;
        }

    }
    
    if (ses.ip4 && geoip_on) {
        get_ip4_info(&ses);
    }
    
    if (ses.cn) {
        if (record_session(&ses) != 0)
            return -1;
    }

    return 0;
}
