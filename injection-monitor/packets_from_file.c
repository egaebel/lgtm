/*
 * (c) 2008-2011 Daniel Halperin <dhalperi@cs.washington.edu>
 */
#include <fcntl.h>
#include <linux/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <tx80211.h>
#include <tx80211_packet.h>

#include "util.h"

static void init_lorcon();

struct lorcon_packet
{
	__le16	fc;
	__le16	dur;
	u_char	addr1[6];
	u_char	addr2[6];
	u_char	addr3[6];
	__le16	seq;
	u_char	payload[0];
} __attribute__ ((packed));

struct tx80211	tx;
struct tx80211_packet	tx_packet;
static const size_t MAX_PACKET_SIZE = 100;

// MY CODE (egaebel), THIS IS A TEST, REMOVE LATER!!!
/*
static void generate_test_payloads(uint8_t *buffer, size_t buffer_size) {
	int i = 0;
	for (; i < buffer_size; i+=4) {
		buffer[i] = 0xDE;
		buffer[i + 1] = 0xAD;
		buffer[i + 2] = 0xBE;
		buffer[i + 3] = 0xEF;
	}	
	printf("Buffer to send:\n");
    for (i = 0; i < buffer_size; i++) {
		printf("%02x", buffer[i]);
	}
	printf("\n");
}
*/

// MY CODE (egaebel), THIS IS A TEST!
static void generate_payload_from_file(char *file_path, uint8_t **payload_buffer, uint32_t *packet_size) {
	struct stat stat_buffer;
	int input_fd;
	size_t file_buffer_size = 250;
	char file_buffer[file_buffer_size];
	int bytes_read;
	int i;
	int payload_buffer_pos;
	if (stat(file_path, &stat_buffer) != 0) {
		char error_buffer[100];
		snprintf(error_buffer, 100, "Error stat-ing file: %s", file_path);
		perror(error_buffer);
		exit(1);
	}
	*packet_size = stat_buffer.st_size;
	*payload_buffer = (uint8_t*) malloc(*packet_size);
	if (payload_buffer == NULL) {
		perror("payload_buffer returned null from malloc");
		exit(1);
	}
	printf("Reading file....\n");
    input_fd = open(file_path, O_RDONLY);
	if (input_fd < 0) {
		perror("Unable to open file!");
	}
	bytes_read = 0;
	payload_buffer_pos = 0;
	while ((bytes_read = read(input_fd, file_buffer, file_buffer_size))) {
		for (i = 0; i < bytes_read; i++, payload_buffer_pos++) {
			(*payload_buffer)[payload_buffer_pos] = file_buffer[i];
		}
	}
	close(input_fd);
}

int main(int argc, char** argv)
{
	uint8_t *payload_buffer;
	uint32_t num_packets = 0;
	uint32_t packet_size = 0;
	uint32_t payload_size = 0;
	char *file_path;
	struct lorcon_packet *packet;
	uint32_t i;
	int32_t ret;
	uint32_t mode;
	uint32_t delay_us;
	struct timespec start, now;
	int32_t diff;

	/* Parse arguments */
	if (argc > 4) {
		printf("Usage: random_packets <file_name> <mode: 0=my MAC, 1=injection MAC> <delay in us>\n");
		return 1;
	}
	if (argc < 4 || (1 != sscanf(argv[3], "%u", &delay_us))) {
		delay_us = 0;
	}
	if (argc < 3 || (1 != sscanf(argv[2], "%u", &mode))) {
		mode = 0;
	} else if (mode > 1) {
		printf("Usage: random_packets <file_name> <mode: 0=my MAC, 1=injection MAC> <delay in us>\n");
		return 1;
	}
	if (argc < 2 || strlen(argv[1]) == 0) {
		printf("Usage: random_packets <file_name> <mode: 0=my MAC, 1=injection MAC> <delay in us>\n");
		return 1;
	} else {
		file_path = argv[1];
	}
	
	/* Generate packet payloads */
	printf("Generating packet payload from file: %s \n", file_path);
	generate_payload_from_file(file_path, &payload_buffer, &payload_size);
	printf("Read payload, size is: %d\n", payload_size);
	
	// Set packet size and number of packets
	if (payload_size > MAX_PACKET_SIZE) {
		packet_size = MAX_PACKET_SIZE;
		num_packets = ceil((float) payload_size / (float) MAX_PACKET_SIZE);
	} else {
		packet_size = payload_size;
		num_packets = 1;
	}

	/* Setup the interface for lorcon */
	printf("Initializing LORCON\n");
	init_lorcon();

	/* Allocate packet */
	packet = malloc(sizeof(*packet) + packet_size);
	if (!packet) {
		perror("malloc packet");
		exit(1);
	}
	packet->fc = (0x08 /* Data frame */
				| (0x0 << 8) /* Not To-DS */);
	packet->dur = 0xffff;
	if (mode == 0) {
		memcpy(packet->addr1, "\x00\x16\xea\x12\x34\x56", 6);
		get_mac_address(packet->addr2, "mon0");
		memcpy(packet->addr3, "\x00\x16\xea\x12\x34\x56", 6);
	} else if (mode == 1) {
		memcpy(packet->addr1, "\x00\x16\xea\x12\x34\x56", 6);
		memcpy(packet->addr2, "\x00\x16\xea\x12\x34\x56", 6);
		memcpy(packet->addr3, "\xff\xff\xff\xff\xff\xff", 6);
	}
	packet->seq = 0;
	tx_packet.packet = (uint8_t *)packet;
	tx_packet.plen = sizeof(*packet) + packet_size;

	/* Send packets */
	printf("Sending %u packets of size %u (. every thousand)\n", num_packets, packet_size);
	if (delay_us) {
		/* Get start time */
		clock_gettime(CLOCK_MONOTONIC, &start);
	}
	for (i = 0; i < num_packets; ++i) {
		if (i == (num_packets - 1)) {
			printf("final packet size: %d\n", (payload_size - i * packet_size));
			memcpy(packet->payload, (payload_buffer + (i * packet_size)), (payload_size - i * packet_size));
			tx_packet.plen = sizeof(*packet) + (payload_size - i * packet_size);
		} else {
			memcpy(packet->payload, (payload_buffer + (i * packet_size)), packet_size);
		}
		if (delay_us) {
			clock_gettime(CLOCK_MONOTONIC, &now);
			diff = (now.tv_sec - start.tv_sec) * 1000000 +
			       (now.tv_nsec - start.tv_nsec + 500) / 1000;
			diff = delay_us*i - diff;
			if (diff > 0 && diff < delay_us)
				usleep(diff);
		}

		ret = tx80211_txpacket(&tx, &tx_packet);
		if (ret < 0) {
			fprintf(stderr, "Unable to transmit packet: %s\n",
					tx.errstr);
			exit(1);
		}

		if (((i+1) % 1000) == 0) {
			printf(".");
			fflush(stdout);
		}
		if (((i+1) % 50000) == 0) {
			printf("%dk\n", (i+1)/1000);
			fflush(stdout);
		}
	}

	return 0;
}

static void init_lorcon()
{
	/* Parameters for LORCON */
	int drivertype = tx80211_resolvecard("iwlwifi");

	/* Initialize LORCON tx struct */
	if (tx80211_init(&tx, "mon0", drivertype) < 0) {
		fprintf(stderr, "Error initializing LORCON: %s\n",
				tx80211_geterrstr(&tx));
		exit(1);
	}
	if (tx80211_open(&tx) < 0 ) {
		fprintf(stderr, "Error opening LORCON interface\n");
		exit(1);
	}

	/* Set up rate selection packet */
	tx80211_initpacket(&tx_packet);
}

