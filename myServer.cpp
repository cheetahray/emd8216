#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "emd8216.h"

#define BUFLEN 512
#define NPACK 10
#define PORT 9930

void diep(char *s)
{
    perror(s);
    exit(1);
}

int main(void)
{
    struct sockaddr_in si_me, si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    u8 IpAddress[4];
    u8 CardType;
    bool is_EMD8216_initialize;
    u8 Status;
    IpAddress[0] = 192;
    IpAddress[1] = 168;
    IpAddress[2] = 11;
    IpAddress[3] = 216;
    is_EMD8216_initialize = false;
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        diep("socket");

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me))==-1)
        diep("bind");

    if(false == is_EMD8216_initialize)
    {
        Status = EMD8216_initial(0,IpAddress,15120,6936,1000,&CardType);
        if ( NO_ERROR == Status )
        {
            Status = EMD8216_security_unlock( 0, (u8*)"12345678");
            if ( NO_ERROR != Status )
            {
                diep("password error.");
            }
            for(u8 pp =0; pp <8 ;pp++)
            {
                Status = EMD8216_point_config_set(0, 0, pp, 0);
                if ( NO_ERROR != Status )
                {
                    diep("set in/out error.");
                }
                Status = EMD8216_point_config_set(0, 1, pp, 0);
                if ( NO_ERROR != Status )
                {
                    diep("set in/out error.");
                }
                Status = EMD8216_point_polarity_set(0, 0, pp, 0);
                if ( NO_ERROR != Status )
                {
                    diep("set +/- error.");
                }
                Status = EMD8216_point_polarity_set(0, 1, pp, 0);
                if ( NO_ERROR != Status )
                {
                    diep("set +/- error.");
                }
            }
        }
        else
        {
            printf("emd8216 initializes error.");
        }
    }

    //for (i=0; i<NPACK; i++) {
    for(;;){
        if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, (socklen_t*)&slen)==-1)
            diep("recvfrom()");
        printf("Received packet from %s:%d\nData: %s\n\n",
            inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
        if(false == is_EMD8216_initialize)
        {
            if(strncmp(buf,"p1on",4)==0)
                Status = EMD8216_point_set(0, 0, 0, 1);
            else if(strncmp(buf,"p1off",5)==0)
                Status = EMD8216_point_set(0, 0, 0, 0);
        }
    }

    close(s);
    return 0;
}
