#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define NTP_SERVER "74.50.98.131"
#define NTP_PORT 123
#define NTP_PACKET_SIZE 48
#define NTP_TIMESTAMP_DELTA 2208988800ull

typedef struct {
    uint8_t li_vn_mode;
    uint8_t stratum;
    uint8_t poll;
    uint8_t precision;
    uint32_t root_delay;
    uint32_t root_dispersion;
    uint32_t reference_id;
    uint32_t ref_ts_sec;
    uint32_t ref_ts_frac;
    uint32_t orig_ts_sec;
    uint32_t orig_ts_frac;
    uint32_t recv_ts_sec;
    uint32_t recv_ts_frac;
    uint32_t trans_ts_sec;
    uint32_t trans_ts_frac;
} ntp_packet;

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

time_t get_ntp_time() {
    int sockfd, n;
    struct sockaddr_in serv_addr;
    ntp_packet packet;

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
        error("ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(NTP_PORT);
    if (inet_pton(AF_INET, NTP_SERVER, &serv_addr.sin_addr) <= 0)
        error("ERROR invalid address");

    memset(&packet, 0, sizeof(packet));
    packet.li_vn_mode = (0x03 << 6) | (0x03 << 3) | 0x03;

    if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR sending packet");

    n = recv(sockfd, &packet, sizeof(packet), 0);
    if (n < 0)
        error("ERROR receiving response");

    close(sockfd);

    return (ntohl(packet.trans_ts_sec) - NTP_TIMESTAMP_DELTA);
}

int main() {
    time_t local_time, ntp_time;

    local_time = time(NULL);
    if (local_time == (time_t)-1)
        error("ERROR getting local time");

    ntp_time = get_ntp_time();

    printf("Local time: %s", ctime(&local_time));
    printf("NTP time  : %s", ctime(&ntp_time));

    double time_difference = difftime(local_time, ntp_time);
    printf("Time difference between NTP and local time: %.2f seconds\n", time_difference);

    return 0;
}
