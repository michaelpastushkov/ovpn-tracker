//  tracker.c
//
//  Open VPN Server tracker
//  Capture usage statistics aand recording sessions
//  in MySQL database for future analysis
//
//  Created by Michael Pastushkov on 9/5/22.
//

#include "tracker.h"

int mode = MODE_REMOTE;
int repeat_time = 60;

int main( int argc, char *argv[] ) {

    time_t now;
 
    time(&now);
    printf("ovpn-tracker: %s", ctime(&now));

    if (read_config() != 0) {
        return -1;
    }

    printf("db_host: %s\n", db_host);
    
    if (db_open() != 0) {
        printf("can't connect to database");
        return -1;
    }

    while (1) {
        
        if (mode == MODE_REMOTE) {
            read_remote();
        } else {
            read_local();
        }
        
        check_alerts();
        
        if (!repeat_time)
            break;
        sleep(repeat_time);
    }
 
    db_close();
    
    return 0;
}
