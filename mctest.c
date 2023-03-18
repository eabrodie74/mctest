#ifdef _WIN32
    #include <Winsock2.h>               /* before Windows.h, else Winsock 1 conflict */
    #include <Ws2tcpip.h>               /* needed for ip_mreq definition for multicast */
    #include <Windows.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <time.h>
#endif

#include <string.h>                     /* for strlen() */
#include <stdio.h>                      /* for printf() */
#include <stdlib.h>                     /* for atoi() */
#include <unistd.h>                     /* for close() */

#define MAX_BUF_LEN    1024             /* maximum string size to send */
#define MIN_MC_PORT    1024             /* minimum port allowed */
#define MAX_MC_PORT    65535            /* maximum port allowed */

int main(int argc, char *argv[]) {

    char *mc_ip_str;                    /* arg[2]: multicast IP address */
    char *if_ip_str;                    /* arg[3]: local interface IP address */
    unsigned short mc_port;             /* arg[4]: multicast port number */
    unsigned char mc_ttl;               /* arg[5]: time to live (hop count) */

    int sock;                           /* socket descriptor */
    struct sockaddr_in mc_ip;           /* socket address structure for group address */
    struct sockaddr_in tx_ip;           /* socket address structure for sender address */
    struct in_addr if_ip;               /* local interface address structure */
    unsigned int tx_ip_len;             /* source address length */
    char tx_ip_buf[INET_ADDRSTRLEN];    /* source address conversion buffer */

    char tx_buf[MAX_BUF_LEN];           /* buffer for string to send */
    char rx_buf[MAX_BUF_LEN+1];         /* buffer for string to receive */
    short int tx_buf_len;               /* length of string to send */
    short int rx_buf_len;               /* length of string received */

    struct ip_mreq mc_req;              /* multicast request structure */
    int flag_on;                        /* socket option flag */

    /* BEG: validate option and number of arguments */
    if ((argc == 1) || (0 == strcmp(argv[1], "transmit") && argc != 6) || (0 == strcmp(argv[1], "receive") && argc != 5)) {
        fprintf(stderr, "Multicast Testing Tool Version 1.5\n\n"
                "This tool is derived from multiple sources on network socket programming and has been enhanced to\n"
                "support the flow of multicast packets across multiple Windows and Linux-based operating systems.\n\n"
                "Usage: mctest [OPTIONS] [PARAMETERS]\n"
                "where  OPTIONS    := { transmit | receive }\n"
                "       PARAMETERS := transmit { mcast_ip | local_if_ip | mcast_port [1024-65535] | mcast_ttl [0-255] }\n"
                "                     receive  { mcast_ip | local_if_ip | mcast_port [1024-65535] }\n\n"
                "Usage example: To test the flow of channel 239.23.5.3 over UDP 48888 across multiple router boundaries\n"
                "               On the transmitting node: mctest transmit 239.23.5.3 10.23.238.10 48888 16\n"
                "               On the receiving node:    mctest receive 239.23.5.3 10.23.238.10 48888\n\n");
        exit(1);
    }
    /* END: validate option and number of arguments */

    /* BEG: "transmit" option */
    if ((0 == strcmp(argv[1], "transmit") && argc == 6)) {

        mc_ip_str = argv[2];      /* arg[2]: multicast IP address */
        if_ip_str = argv[3];      /* arg[3]: local interface IP address */
        mc_port = atoi(argv[4]);    /* arg[4]: multicast port number */
        mc_ttl = atoi(argv[5]);     /* arg[5]: time to live (hop count) */
        if_ip.s_addr = inet_addr(if_ip_str);

        /* BEG: validate the port range */
        if ((mc_port < MIN_MC_PORT) || (mc_port > MAX_MC_PORT)) {
            fprintf(stderr, "Invalid port number argument %d.\n", mc_port);
            fprintf(stderr, "Valid range is between %d and %d.\n", MIN_MC_PORT, MAX_MC_PORT);
            exit(1);
        }
        /* END: validate the port range */

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(0x0101, &wsaData)) {
        perror("WSAStartup");
        return 1;
    }
#endif

        /* BEG: create a socket for sending to the multicast address */
        if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
            perror("socket() failed");
            exit(1);
        }
        else
            printf("Create UDP multicast socket...OK\n");
        /* END: create socket for sending to multicast address */

        /* BEG: set local interface IP address */
        if ((setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, &if_ip, sizeof(if_ip))) < 0) {
            perror("setsockopt() failed");
            exit(1);
        }
        else
            printf("Transmit over IP %s...OK\n", if_ip_str);
        /* END: set local interface IP address */

        /* BEG: set TTL (time to live/hop count) for packet transmission */
        if ((setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*) &mc_ttl, sizeof(mc_ttl))) < 0) {
            perror("setsockopt() failed");
            exit(1);
        }
        else
            printf("Set TTL to %d...OK\n\n", mc_ttl);
        /* END: set TTL (time to live/hop count) for packet transmission */

        /* BEG: construct multicast address structure */
        memset(&mc_ip, 0, sizeof(mc_ip));
        mc_ip.sin_family = AF_INET;
        mc_ip.sin_addr.s_addr = inet_addr(mc_ip_str);
        mc_ip.sin_port = htons(mc_port);
        /* END: construct multicast address structure */

        printf("Begin typing each message, followed by RETURN. Type (\"eos\" to quit):\n\n");

        /* BEG: clear send buffer */
        memset(tx_buf, 0, sizeof(tx_buf));
        /* END: clear send buffer */

        while (fgets(tx_buf, MAX_BUF_LEN, stdin) && strcmp("eos\n", tx_buf) != 0) {
            tx_buf_len = strlen(tx_buf);

            /* BEG: send string to multicast address */
            if ((sendto(sock, tx_buf, tx_buf_len, 0, (struct sockaddr*) &mc_ip, sizeof(mc_ip))) != tx_buf_len) {
                perror("sendto() sent incorrect number of bytes");
                exit(1);
            }
            printf("Sent %hd bytes from %s\n\n", tx_buf_len, inet_ntoa(mc_ip.sin_addr));
            /* END: send string to multicast address */
        }

        /* BEG: clear send buffer */
        memset(tx_buf, 0, sizeof(tx_buf));
        /* END: clear send buffer */

    }
    /* END: "transmit" option */

    /* BEG: "receive" option */
    if ((0 == strcmp(argv[1], "receive") && argc == 5)) {

        mc_ip_str = argv[2];      /* arg[2]: multicast IP address */
        if_ip_str = argv[3];      /* arg[3]: local interface IP address */
        mc_port = atoi(argv[4]);    /* arg[4]: multicast port number */
        flag_on = 1;
        if_ip.s_addr = inet_addr(if_ip_str);

        /* BEG: validate the port range */
        if ((mc_port < MIN_MC_PORT) || (mc_port > MAX_MC_PORT)) {
            fprintf(stderr, "Invalid port number argument %d.\n", mc_port);
            fprintf(stderr, "Valid range is between %d and %d.\n", MIN_MC_PORT, MAX_MC_PORT);
            exit(1);
        }
        /* END: validate the port range */

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(0x0101, &wsaData)) {
        perror("WSAStartup");
        return 1;
    }
#endif

        /* BEG: create socket for joining multicast group */
        if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
            perror("socket() failed");
            exit(1);
        }
        else
            printf("Create UDP multicast socket...OK\n");
        /* END: create socket for joining multicast group */

        /* BEG: set local interface IP address */
        if ((setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, &if_ip, sizeof(if_ip))) < 0) {
            perror("setsockopt() failed");
            exit(1);
        }
        else
            printf("Receive over IP %s...OK\n", if_ip_str);
        /* END: set local interface IP address */

        /* BEG: set reuse port to on to allow multiple binds per host */
         if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag_on, sizeof(flag_on))) < 0) {
             perror("setsockopt() failed");
             exit(1);
         }
         /* END: set reuse port to on to allow multiple binds per host */

        /* BEG: construct multicast address structure */
        memset((char*) &mc_ip, 0, sizeof(mc_ip));               /* zero out structure */
        mc_ip.sin_family = AF_INET;                     /* Internet address family */
        mc_ip.sin_addr.s_addr = htonl(INADDR_ANY);   /* user-specified incoming interface */
        mc_ip.sin_port = htons(mc_port);                /* multicast port */
        /* END: construct multicast address structure */

        /* BEG: bind multicast address to socket */
         if ((bind(sock, (struct sockaddr *) &mc_ip, sizeof(mc_ip))) < 0) {
             perror("bind() failed");
             exit(1);
         }
         /* END: bind multicast address to socket */

        /* BEG: construct IGMP join request structure */
        mc_req.imr_multiaddr.s_addr = inet_addr(mc_ip_str);
        mc_req.imr_interface.s_addr = inet_addr(if_ip_str);
        /* END: construct IGMP join request structure */

        /* beg: send an ADD MEMBERSHIP message via setsockopt() */
        if ((setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*) &mc_req, sizeof(mc_req))) < 0) {
            perror("setsockopt() failed");
            exit(1);
        }
        /* end: send an ADD MEMBERSHIP message via setsockopt() */

        printf("\n\nWaiting to receive a stream of multicast packets over interface IP %s (CTRL-C to quit):\n\n", if_ip_str);

         for (;;) {
            /* BEG: clear receive buffers and structs */
            memset(rx_buf, 0, sizeof(rx_buf));
            rx_buf_len = sizeof(if_ip);
            memset(&if_ip, 0, rx_buf_len);
            /* END: clear receive buffers and structs */

            /* BEG: wait to receive a packet */
            if ((rx_buf_len = recvfrom(sock, rx_buf, MAX_BUF_LEN, 0, (struct sockaddr*) &tx_ip, (socklen_t*) &tx_ip_len)) < 0) {
                perror("recvfrom() failed");
                exit(1);
            }
            /* END: wait to receive a packet */

            /* BEG: output received string */
            if ((inet_ntop(AF_INET, &tx_ip.sin_addr.s_addr, tx_ip_buf, INET_ADDRSTRLEN)) == 0)
                tx_ip_buf[0] = '\0';
            printf("Received %hd bytes in group %s, from speaker %s: %s", rx_buf_len, mc_ip_str, tx_ip_buf, rx_buf);
            /* END: output received string */
        }

        /* BEG: send a DROP MEMBERSHIP message via setsockopt() */
        if ((setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*) &mc_req, sizeof(mc_req))) < 0) {
            perror("setsockopt() failed");
            exit(1);
        }
        /* END: send a DROP MEMBERSHIP message via setsockopt() */
        close(sock);

    }

#ifdef _WIN32
    WSACleanup();
#else
    close(sock);
#endif

    return 0;
}
