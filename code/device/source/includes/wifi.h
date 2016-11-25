/*
 * wifi.h
 *
 * Created: 2012-05-11 5:26:56 PM
 *  Author: Justin
 */ 


#ifndef WIFI_H_
#define WIFI_H_

#define WIFI_ERR_OK						0
#define WIFI_ERR_NETWORK_NOT_FOUND		-1
#define WIFI_ERR_SECKEY_NEEDED			-2
#define WIFI_ERR_CONNECTION_ERR			-3
#define WIFI_ERR_NETWORK_ERR			-4
#define WIFI_ERR_INVALID_IN_PARAM		-5 /*Invalid input parameter*/
#define WIFI_ERR_INVALID_SOCKET			-6
#define WIFI_ERR_TIMEOUT_ERR			-7
#define WIFI_ERR_FAILED					-8
#define WIFI_ERR_WAITING				-10
#define WIFI_ERR_BUSY					-11

#define WIFI_SEC_NONE		1
#define WIFI_SEC_WEP		2
#define WIFI_SEC_WPA_PSK	4
#define WIFI_SEC_WPA2_PSK	8
#define WIFI_SEC_WPA_ENT	16
#define WIFI_SEC_WPA2_ENT	32


extern const char WIFI_STATEXT_NOT_SUPPORT[];
extern const char WIFI_STATEXT_WAIT_HW[];
extern const char WIFI_STATEXT_NOT_CONNECT[];
extern const char WIFI_STATEXT_NO_HW[];
extern const char WIFI_STATEXT_RADIO_OFF[];



typedef struct{/*Text Box*/
	char mac[20];
	char wstate[20];
	char bssid[20];
	char ssid[32];
	char channel[4];
	char security[25];
	char rssi[5];
	char ip[20];
	char subnet[20];
	char gateway[20];
	char dns1[20];
	char dns2[20];
	char rxcount[16];
	char txcount[16];
}atnstat;

typedef struct{
	char bssid[20];
	char ssid[32];
	char channel[3];
	char type[6];
	char rssi[5];
	char security[25];
}atwsitem;

#define SSID_SLOT_MAX_COUNT 30
typedef struct{
	int count;
	atwsitem items[SSID_SLOT_MAX_COUNT];
}atws;

//
// Status
//
// Obtian WIFI Network Status
extern atnstat	wifi_get_nstat(void);

// Obtain SSID detected
extern atws		wifi_get_ws(void);

// Obtain Signal strength of a connected WIFI
extern int		wifi_get_signal_strength(void);

// Obtain WIFI status lable (In General)
extern char*	wifi_get_status_label(void);

// Obtain wifi module firmware version
// out: VersionString. if not required output string, set it to zero
// return: 0-OK, Other-Failed
extern int wifi_get_firmware_version(char* outVersionString);

// WIFI Sync Time - To Sync Wifi Date Time to HammerHead DateTime
// in/out/ - null
// return 0-OK, Other-Failed
extern int wifi_sync_time(void);


//
// Connection
//
extern int wifi_radio_power(unsigned char status);
extern int wifi_connect(char* ssid, char* securitykey);
extern int wifi_connect_hidden_ssid(char* ssid, char* securitykey, int securitytype);//connection to hidden SSID, SecurityType must be provided - WIFI_SEC_XXX
extern int wifi_disconnect(void);
extern unsigned char wifi_is_connected(void);//1=connected, 0=notConnected

//
// Configurations
//
extern int wifi_config_dhcp(unsigned char enable);
extern int wifi_config_static_dns(char* dns1, char* dns2);
extern int wifi_config_static_ip(char* ip, char* subnet, char* gateway);
extern int wifi_config_save_as_default(void);

//
// DNS
//
//This function to get an IP address from a host name
//in: url - hostname to be identified
//in: param (optional) - 2 bytes. 1st byte indicate retry counter (range from 0 to 10), 2nd byte indicate timeout in second (Range from 0 to 20), set param=0 to use default where retry=0, timeout=2
//out: outIpString - is lookup successful, IP in string will be return here
//return: 0-OK, (-1)-Error. Possible Error: wifi not connected, DNS address not set
int wifi_dns_lookup(char* url, unsigned char* param, char* outIpString);



//
// Access
//
extern int wifi_ping(char* ipdata, char* respdata);

/****************************************************************************************************************
*	
*H2CORE - SOCKET FUNCTIONS
*
*****************************************************************************************************************/

// wifi_tcp_connect(), to make a connection to server, or to listen to a local port
// in: destip: destination ip in ascii, example, "192.168.1.5". for listening mode, set destip = 0;
// in: port: destination (or local for listening) port number in ascii. example "8090"
// ret:	Positivite Number: Connection ID. CID. Negative Number is Error
extern int wifi_tcp_connect(char* destip, char* port);
extern int wifi_tcp_connect_non_blocking(char* destip, char* port);
extern int wifi_tcp_connect_get_status(void);

// wifi_tcp_disconnect(). to disconnect the socket connection
// in: cid. The Connection ID (returned from wifi_tcp_connect())
// ret: 0-OK, else Failed
extern int wifi_tcp_disconnect(int cid);

// wifi_tcp_disconnect_all(). to disconnect and release all connnected TCP handle
// in: -
// ret: 0-OK, else Failed
extern int wifi_tcp_disconnect_all(void);

// wifi_tcp_write(). to write a stream data to the specify socket
// in: cid: Connection ID (returned from wifi_tcp_connect())
// in: data: pointer to data buffer to be write
// in: datelen: len of the data
// ret: 0-OK. Else failed
extern int wifi_tcp_write(int cid, unsigned char* data, int datalen);

// wifi_tcp_read(). to read data stream in the socket
// Note: after read and processing on data completed. Programmer must clean the data by using wifi_tcp_flush();
// in/out: cidptr. Pointer to CID. For listening mode connection, the function will overwrite this CID to tell which CID is actually stream the data
// out: data. pointer of data pointer
// int: timeoutms: the waiting time out value in ms to wait for incoming data stream
// ret: the length of stream data. Negative is Error
extern int wifi_tcp_read(int* cidptr, unsigned char** data, int timeoutms);

// wifi_tcp_flush(). to flush the data in instream strorage 
extern int wifi_tcp_flush(int cid, int flushlen);

/****************************************************************************************************************
*	
*WIFI - SSL FUNCTIONS
*
*****************************************************************************************************************/

// SSL - Delete cert 
// To delete the cert that has previously inserted
// in: certname
// return: 0-OK, Other-Failed
extern int ssl_delete_cert(char* certname);

// SSL - Add Cert by Binary Data (Hex bytes)
// in: certname. The reference cert name keep by system for further reference. The cert file inserted to local memory will be given with this name
// in: certdata. The cert data array
// in: certdatalen. The length of the cert data
// return: 0-OK, Other-Failed
extern int ssl_add_cert(char* certname, unsigned char* certdata, int certdatalen);

// SSL - Add Cert by File
// in: certname. The reference cert name keep by system for further reference. The cert file inserted to local memory will be given with this name
// in: certfile. The certfile name in hammerhead disk
// return: 0-OK, Other-Failed
extern int ssl_add_certfile(char* certname, char* certfile);

// Open SSL, client mode (HammerHead is a client to a TCP Connection)
// Method: 1Way or 2Ways
// 1Way: pass in ca_certname. client_certname & client_certkey set to 0
// 2Ways: pass in ca_certname, client_certname and client_certkey
// in: tcpConnectionid: The connection ID to establish SSL. this ID Return by wifi_tcp_connect()
// in: server_ca_certname: The server cert name (the cert shall be pre-added by using ssl_add_certfile()/ssl_add_cert())
// in: client_ca_certname: The client(HammerHead) cert name (the cert shall be pre-added by using ssl_add_certfile()/ssl_add_cert())
// in: client_ca_certkey: The client(HammerHead) cert key (the cert shall be pre-added by using ssl_add_certfile()/ssl_add_cert())
// return: 0-OK, Other-Failed
extern int ssl_open_clientmode(int tcpConnectionId, char* server_ca_certname, char* client_certname, char* client_certkey);

// SSL - Open SSL, Server Mode, to a listening TCP connection
// in: tcpConnectionid: The connection ID to establish SSL. this ID Return by wifi_tcp_connect()
// in:local_server_certname: The certname for local server cert(the cert shall be pre-added by using ssl_add_certfile()/ssl_add_cert())
// in:local_server_certkey: The certkey name for local server key(the cert shall be pre-added by using ssl_add_certfile()/ssl_add_cert())
// return 0-OK, Other-Failed
extern int ssl_open_servermode(int ListenerConnectionId, char* local_server_certname, char* local_server_certkey);

// SSL Close - Close the existing SSL connection.
// After a successful SSL CLOSE on CID, the Socket will be disconnected as well
// in:tcpConnectionId: The Connection ID to be close
// Return: 0-OK. Else-Error
extern int ssl_close(int tcpConnectionId);

//Non blocking mode
extern int ssl_open_clientmode_non_blocking(int tcpConnectionId, char* server_ca_certname, char* client_certname, char* client_certkey);
extern int ssl_open_servermode_non_blocking(int ListenerConnectionId, char* local_server_certname, char* local_server_certkey);
extern int ssl_open_get_status(void);

extern int ssl_close_nonblocking(int tcpConnectionId);
extern int ssl_close_get_status(void);



#endif /* WIFI_H_ */