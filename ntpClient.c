#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>

#define NTP_TIMESTAMP_DELTA 2208988800ull
#define LI(packet)   (uint8_t) ((packet.li_vn_mode & 0xC0) >> 6) // (li   & 11 000 000) >> 6
#define VN(packet)   (uint8_t) ((packet.li_vn_mode & 0x38) >> 3) // (vn   & 00 111 000) >> 3
#define MODE(packet) (uint8_t) ((packet.li_vn_mode & 0x07) >> 0) // (mode & 00 000 111) >> 0


int  main (){
	typedef struct
	{

  		uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                           // li.   Two bits.   Leap indicator.
                           // vn.   Three bits. Version number of the protocol.
                           // mode. Three bits. Client will pick mode 3 for client.

  		uint8_t stratum;         // Eight bits. Stratum level of the local clock.
  		uint8_t poll;            // Eight bits. Maximum interval between successive messages.
  		uint8_t precision;       // Eight bits. Precision of the local clock.

		uint32_t rootDelay;      // 32 bits. Total round trip delay time.
  		uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
  		uint32_t refId;          // 32 bits. Reference clock identifier.

  		uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
  		uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

  		uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
  		uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

  		uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
  		uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

  		uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
 		uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

	} ntp_packet;
	ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	memset( &packet, 0, sizeof( ntp_packet ) );
	// Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.
	*( ( char * ) &packet + 0 ) = 0x1b; // Represents 27 in base 10 or 00011011 in base 2.    
	
	struct sockaddr_in server_address;
	const int server_port = 123;
	int sock,n;
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock<0){
		printf("could not create socket\n");
		exit(EXIT_FAILURE);
	}
	
	printf("Client connected\n");
	
	bzero((char*)&server_address,sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr=inet_addr("128.138.140.44");
	server_address.sin_port=htons(server_port);
	
	if ( connect( sock, ( struct sockaddr * ) &server_address, sizeof( server_address) ) < 0 ){
	
		printf("connected faied\n");
		exit(EXIT_FAILURE);	
	}
	
	printf("connected\n");

	n =write (sock,(char*)&packet,sizeof(ntp_packet));
	if (n<0){
		printf("sending failed\n");
		exit(EXIT_FAILURE);
	}
	
	printf("sent it\n");
	n =read(sock,(char*)&packet,sizeof(ntp_packet));
	if(n<0){
		printf("reading failed\n");
		exit(EXIT_FAILURE);
	}	
	
	printf("reading corrected\n\n\n");
	packet . txTm_s = ntohl ( packet . txTm_s );
	packet . txTm_f = ntohl ( packet . txTm_f );
	time_t txTm = ( time_t ) ( packet.txTm_s - NTP_TIMESTAMP_DELTA );

	printf( "Time: %s", ctime( ( const time_t* ) &txTm ) );
	
	close(sock);
	return 0;
}
