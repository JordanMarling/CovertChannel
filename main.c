/* ========================================================================
   $SOURCE FILE
   $File: main.c $
   $Program: covert_channel $
   $Developer: Jordan Marling $
   $Created On: 2015/09/14 $
   $Functions: 
       int client(const char *covert_filename, const char *dummy_filename, const char *addr, const char *client_addr, int seconds_between_sends, int packet_size);
       int server(const char *covert_filename, const char *dummy_filename, const char *addr);
       void encode(const char *buffer, char *out_buffer);
       void decode(const char *buffer, char *out_buffer);
       void usage(const char *name)
       int main(int argc, char **argv)
       unsigned short udp_checksum(unsigned short *buf, int bytes)
   $
   $Description: This program communicates a message over UDP using covert
                 channels. The covert channel used is the source and dest-
                 ination ports. The program converts the characters in the
                 message down to 5 bits each so that 6 characters can fit
                 inside of the 32 bit slot.
                 To learn how to use the program, compile and type
                    ./assign1 -h
   $
   $Revisions: $
   ======================================================================== */

#include <arpa/inet.h>
#include <getopt.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MODE_NONE 0
#define MODE_SERVER 1
#define MODE_CLIENT 2

// NOTE: This alphabet can be randomized to provide obscurity. This is not encryption however.
//       There must be at most 32 characters in this array. The compiler should throw a warning
//       if this limit is passed. The client and server programs must have the same alphabet.
const char alphabet[32] = "abcdefghijklmnopqrstuvwxyz ?$&.";

int client(const char *covert_filename, const char *dummy_filename, const char *addr, const char *client_addr, int seconds_between_sends, int packet_size);
int server(const char *covert_filename, const char *dummy_filename, const char *addr);
void encode(const char *buffer, char *out_buffer);
void decode(const char *buffer, char *out_buffer);

/* ========================================================================
   $FUNCTION
   $Name: usage
   $Prototype: void usage(const char *name)
   $Params: 
       name: The name of the program
   $
   $Description: This outputs the usage code of the program. $
   ======================================================================== */
void usage(const char *name)
{
    printf("Usage: %s -i <seconds> -t <convert file> -d <dummy file> -c <destination IP> -s <source IP> -h -p <packet size> -a <client address>\n", name);
    printf("\t-i: The amount of seconds between sends. Default: 0 seconds\n");
    printf("\t-t: The text file of the data you want to send covertly.\n");
    printf("\t-d: The text file of the dummy data that you want to send in the main packet.\n");
    printf("\t-s: Puts the program in server mode.\n");
    printf("\t-c: Puts the program in client mode.\n");
    printf("\t-h: Prints this text.\n");
    printf("\t-p: Size of the packets sent by the client. Default: 100 bytes\n");
    printf("\t-a: Client address. This only needs to be used when using the c switch.\n");
}

/* ========================================================================
   $FUNCTION
   $Name: main
   $Prototype: int main(int argc, char **argv)
   $Params: 
   $
   $Description: This is the main entry point into the program. $
   ======================================================================== */
int main(int argc, char **argv)
{

    int opt, opt_index;
    struct option option_args[] = {
        { "help", no_argument, 0, 'h' },
        { "interval", required_argument, 0, 'i' },
        { "text", required_argument, 0, 't' },
        { "dummy", required_argument, 0, 'd' },
        { "server", required_argument, 0, 's' },
        { "client", required_argument, 0, 'c' },
        { "clientaddr", required_argument, 0, 'a' },
    };

    int client_interval = 0;
    char covert_filename[256] = {0};
    char dummy_filename[256] = {0};
    int mode = MODE_NONE;
    char addr[16] = {0};
    char client_addr[16] = {0};
    int packet_size = 100;

    while ((opt = getopt_long(argc, argv, "hi:t:d:s:c:p:a:", option_args, &opt_index)) != -1)
    {
        switch (opt)
        {
            case 'h':
            {
                usage(argv[0]);
                return 0;
            } break;

            case 'i':
            {
                if (sscanf(optarg, "%d", &client_interval) != 1)
                {
                    printf("Please input a correct interval to send at.\n");
                    usage(argv[0]);
                    return 1;
                }
            } break;

            case 't':
            {
                if (sscanf(optarg, "%s", covert_filename) != 1)
                {
                    printf("Please input a correct covert file.\n");
                    usage(argv[0]);
                    return 1;
                }
            } break;

            case 'd':
            {
                if (sscanf(optarg, "%s", dummy_filename) != 1)
                {
                    printf("Please input a correct dummy file.\n");
                    usage(argv[0]);
                    return 1;
                }
            } break;

            case 's':
            {
                if (sscanf(optarg, "%s", addr) != 1)
                {
                    printf("Please input an IP address with the server switch.\n");
                    usage(argv[0]);
                }
                mode = MODE_SERVER;
            } break;

            case 'c':
            {
                if (sscanf(optarg, "%s", addr) != 1)
                {
                    printf("Please input an IP address with the client switch.\n");
                    usage(argv[0]);
                }
                mode = MODE_CLIENT;
            } break;

            case 'p':
            {
                if (sscanf(optarg, "%d", &packet_size) != 1)
                {
                    printf("Please input a correct packet size to send.\n");
                    usage(argv[0]);
                    return 1;
                }
            } break;

            case 'a':
            {
                if (sscanf(optarg, "%s", client_addr) != 1)
                {
                    printf("Please input an IP address with the -a switch.\n");
                    usage(argv[0]);
                }
            } break;

        }
    }

    if (mode == MODE_NONE)
    {
        printf("You need to specify a client or server mode.\n");
        usage(argv[0]);
        return 1;
    }

    if (strlen(covert_filename) == 0)
    {
        printf("Please provide a covert file.\n");
        usage(argv[0]);
        return 1;
    }

    if (strlen(dummy_filename) == 0)
    {
        printf("Please provide a dummy file.\n");
        usage(argv[0]);
        return 1;
    }

    if (strlen(addr) == 0)
    {
        printf("Please supply an IP address to the client or server.\n");
        usage(argv[0]);
        return 1;
    }

    if (mode == MODE_SERVER)
    {
        server(covert_filename, dummy_filename, addr);
    }
    else if (mode == MODE_CLIENT)
    {
        if (strlen(client_addr) == 0)
        {
            printf("The client needs an address.\n");
            usage(argv[0]);
            return -1;
        }

        client(covert_filename, dummy_filename, addr, client_addr, client_interval, packet_size);
    }

    return 0;
}

/* ========================================================================
   $FUNCTION
   $Name: checksum
   $Prototype: unsigned short udp_checksum(unsigned short *buf, int bytes)
   $Params: 
       buf: The buffer to checksum
       bytes: How many bytes the buffer is
   $
   $Description: This function computes the checksum of the UDP pseudo
                 header to be used by the UDP header as a checksum. $
   ======================================================================== */
unsigned short udp_checksum(unsigned short *buf, int bytes)
{
    long sum;
    unsigned short oddbyte;
    short answer;

    sum = 0;
    while (bytes > 1)
    {
        sum += *buf++;
        bytes -= 2;
    }

    if (bytes == 1)
    {
        oddbyte = 0;
        *((u_char*)&oddbyte) = *(u_char*)buf;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum = sum + (sum >> 16);
    answer = (short)~sum;

    return answer;
}

/* ========================================================================
   $FUNCTION
   $Name: client
   $Prototype: int client(const char *covert_filename, const char *dummy_filename, const char *addr, const char *client_addr, int seconds_between_sends, int packet_size)
   $Params: 
       covert_filename: The file to send over the covert channel
       dummy_filename: Dummy data to be sent over UDP.
       addr: The destination address
       client_addr: The client computers address.
       seconds_between_sends: How many seconds between sends.
       packet_size: How much dummy data to include in each datagram.
   $
   $Description: This function sends UDP datagrams to the server.
                 It inserts the covert data into the source and
                 destination ports. $
   ======================================================================== */
int client(const char *covert_filename, const char *dummy_filename, const char *addr, const char *client_addr, int seconds_between_sends, int packet_size)
{
    // Program logic variables
    FILE *covert_file = 0;
    FILE *dummy_file = 0;
    char covert_buffer[7] = {0};
    int bytes_read;
    int bytes_to_read;
    int covert_running = 1;

    // Socket variables
    int sd;
    struct udphdr *udp_header;
    struct sockaddr_in sin;
    uint32_t source_addr;
    uint32_t dest_addr;
    char *buffer;
    int zero = 0;
    struct pseudo_header
    {
        uint32_t saddr;
        uint32_t daddr;
        uint8_t placeholder;
        uint8_t protocol;
        uint16_t udp_len;
    } udp_pseudo_header;


    // Open the files.
    if ((covert_file = fopen(covert_filename, "r")) == 0)
    {
        printf("Error opening covert file.\n");
        return -1;
    }
    if ((dummy_file = fopen(dummy_filename, "r")) == 0)
    {
        printf("Error opening dummy file.\n");
        return -1;
    }

    // Create the socket
    if ((sd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) == -1)
    {
        printf("Error creating raw socket.\n");
        return -1;
    }
    if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &zero, sizeof(zero)) < 0)
    {
        printf("Error setting sockopt.\n");
        return -1;
    }

    // Convert the source/dest ip addresses
    if (inet_pton(AF_INET, client_addr, &source_addr) != 1)
    {
        printf("Error converting %s to an IP.\n", client_addr);
        return -1;
    }
    if (inet_pton(AF_INET, addr, &dest_addr) != 1)
    {
        printf("Error converting %s to an IP.\n", addr);
        return -1;
    }
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = inet_addr(addr);

    // Allocate enough size for the buffer.
    buffer = (char*)malloc(sizeof(udp_header) + packet_size);
    memset(buffer, 0, sizeof(udp_header) + packet_size);

    printf("Sending data...\n");
    // Keep looping until the covert data has been sent.
    while (covert_running)
    {

        // Read covert data.
        bytes_to_read = 6;
        while (bytes_to_read > 0)
        {
            bytes_read = fread(covert_buffer + (6 - bytes_to_read), 1, bytes_to_read, covert_file);

            // Check if we have reached the end.
            if (bytes_read == 0)
            {
                covert_running = 0;
                break;
            }

            bytes_to_read -= bytes_read;
        }

        // Fill in the rest of the data with spaces.
        while (bytes_to_read > 0)
        {
            *(covert_buffer + (6 - bytes_to_read)) = ' ';
            bytes_to_read--;
        }

        // Read dummy data.
        bytes_to_read = packet_size;
        while (bytes_to_read > 0)
        {
            bytes_read = fread(buffer + sizeof(udp_header) + (packet_size - bytes_to_read), 1, bytes_to_read, dummy_file);

            // If we reach the end of the dummy data, start over.
            if (bytes_read == 0)
            {
                rewind(dummy_file);
            }
            bytes_to_read -= bytes_read;
        }


        // Put the UDP data in.
        udp_header = (struct udphdr*)buffer;
        encode(covert_buffer, (char*)&udp_header->source);
        encode(covert_buffer + 3, (char*)&udp_header->dest);

        udp_header->source = htons(udp_header->source);
        udp_header->dest = htons(udp_header->dest);
        udp_header->len = htons(sizeof(udp_header) + packet_size);
        udp_header->check = 0;

        // Create the pseudo header
        udp_pseudo_header.saddr = source_addr;
        udp_pseudo_header.daddr = dest_addr;
        udp_pseudo_header.placeholder = 0;
        udp_pseudo_header.protocol = IPPROTO_UDP;
        udp_pseudo_header.udp_len = htons(sizeof(udp_header) + packet_size);

        udp_header->check = udp_checksum((unsigned short *)&udp_pseudo_header, sizeof(udp_pseudo_header));

        if (sendto(sd, buffer, sizeof(udp_header) + packet_size, 0, (struct sockaddr*)&sin, sizeof(sin)) < 0)
        {
            printf("Error sending datagram.\n");
            return -1;
        }

        sleep(seconds_between_sends);

    }

    free(buffer);

    return -1;
}

/* ========================================================================
   $FUNCTION
   $Name: server
   $Prototype: int server(const char *covert_filename, const char *dummy_filename, const char *addr)
   $Params: 
       covert_filename: The file to send over the covert channel
       dummy_filename: Dummy data to be sent over UDP.
       addr: The clients address.
   $
   $Description: This function listens for UDP packets from a specified client
                 and then parses out the covert channel data. $
   ======================================================================== */
int server(const char *covert_filename, const char *dummy_filename, const char *addr)
{

#define MAX_PACKET_SIZE 70000

    // Program logic variables
    FILE *covert_file = 0;
    FILE *dummy_file = 0;
    char covert_text[7] = {0};
    int bytes_to_write;
    int bytes_written;

    // Socket variables
    int sd;
    int packet_length;
    char buffer[MAX_PACKET_SIZE]; // Size of the packet
    socklen_t client_len;
    struct sockaddr_in client;
    struct iphdr ip_header;
    struct udphdr udp_header;
    int listening_addr;

    // Open the files.
    if ((covert_file = fopen(covert_filename, "w")) == 0)
    {
        printf("Error creating covert file.\n");
        return -1;
    }
    if ((dummy_file = fopen(dummy_filename, "w")) == 0)
    {
        printf("Error creating dummy file.\n");
        return -1;
    }

    if ((sd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) == -1)
    {
        printf("Error creating raw socket.\n");
        return -1;
    }

    if (inet_pton(AF_INET, addr, &listening_addr) == 0)
    {
        printf("Error listening to IP: %s\n", addr);
        return -1;
    }
    client_len = sizeof(client);

    printf("Listening for packets from %s\n", addr);
    // Keep listening for incoming packets from address.
    while (1)
    {
        if ((packet_length = recvfrom(sd, buffer, MAX_PACKET_SIZE, 0, (struct sockaddr*)&client, &client_len)) < 0)
        {
            printf("Error receiving packet.\n");
            return -1;
        }

        // Copy the ip header into the struct
        memcpy(&ip_header, buffer, sizeof(ip_header));

        // Check to see if this packet is from the client we are listening to.
        if (ip_header.saddr != (unsigned int)listening_addr)
        {
            continue;
        }

        // Copy the udp header into the struct
        memcpy(&udp_header, buffer + (ip_header.ihl * 4), sizeof(udp_header));

        // Convert the source/dest ports
        udp_header.source = ntohs(udp_header.source);
        udp_header.dest = ntohs(udp_header.dest);

        // Decode the covert data from the ports.
        decode((char*)&udp_header.source, covert_text);
        decode((char*)&udp_header.dest, covert_text + 3);

        // Write the covert data to the file.
        bytes_to_write = 6;
        bytes_written = 0;
        while (bytes_to_write > bytes_written)
        {
            bytes_written += fwrite(covert_text + bytes_written, 1, bytes_to_write - bytes_written, covert_file);
        }

        // Write the dummy data to the file.
        bytes_to_write = ntohs(udp_header.len) - sizeof(udp_header);
        bytes_written = 0;
        while (bytes_to_write > bytes_written)
        {
            bytes_written = fwrite(buffer + (ip_header.ihl * 4) + sizeof(udp_header) + bytes_written, 1, bytes_to_write - bytes_written, dummy_file);
        }

        // Flush the files descriptors so that the operating system can see.
        fflush(covert_file);
        fflush(dummy_file);

        printf("Received: '%s'\n", covert_text);

    }

    return 0;
}

/* ========================================================================
   $FUNCTION
   $Name: encode
   $Prototype: void encode(const char *buffer, char *out_buffer)
   $Params: 
       buffer: The buffer to encode into 5 bit characters
       out_buffer: The output buffer.
   $
   $Description: This function converts an array of characters into an array
                 of 5 bit characters. This function assumes the input buffer
                 is 3 bytes and output buffer is 2 bytes. $
   ======================================================================== */
void encode(const char *buffer, char *out_buffer)
{
#define GETINDEX(c) (int)(strchr(alphabet, c) - alphabet)

    int byte = 0;
    int bit = 1;

    // Clear the out buffer.
    memset(out_buffer, 0, 2);

    // Set the initial bit padding
    out_buffer[0] = 0x80;

    // Store the alphabet indices in an array
    for(int i = 0; i < 3; i++)
    {
        int tmp = GETINDEX(buffer[i]);

        // If we cannot find the character we input a space.
        if (tmp < 0)
        {
            tmp = GETINDEX(' ');
        }

        // Insert the 5 bit character.
        for(int j = 0; j < 5; j++)
        {

            // Get the current bit and put it into the output buffer
            int tmp2 = (tmp >> (4 - j)) & 0x1;
            out_buffer[byte] |= tmp2 << (7 - bit);

            // Make sure we wrap to the next byte.
            bit++;
            if (bit > 7)
            {
                byte++;
                bit = 0;
            }
        }
    }
}

/* ========================================================================
   $FUNCTION
   $Name: decode
   $Prototype: void decode(const char *buffer, char *out_buffer)
   $Params: 
       buffer: The buffer to encode into 5 bit characters
       out_buffer: The output buffer.
   $
   $Description: This function converts an array of 5 bit characters into an array
                 of characters. This function assumes the input buffer
                 is 2 bytes and output buffer is 3 bytes. $
   ======================================================================== */
void decode(const char *buffer, char *out_buffer)
{
    int byte = 0;
    int bit = 1;

    // Clear the out buffer
    memset(out_buffer, 0, 3);

    int tmp_index;
    for(int i = 0; i < 3; i++)
    {
        tmp_index = 0;
        for(int j = 0; j < 5; j++)
        {

            // Get the current bit
            char tmp_char = buffer[byte];
            char tmp_bit = (tmp_char >> (7 - bit)) & 0x1;

            // Set the bit in the index, and shift the index over.
            tmp_index <<= 1;
            tmp_index |= tmp_bit;

            // Make sure we wrap to the next byte.
            bit++;
            if (bit > 7)
            {
                byte++;
                bit = 0;
            }
        }

        // Put the char in the output buffer.
        out_buffer[i] = alphabet[tmp_index];
    }
}
