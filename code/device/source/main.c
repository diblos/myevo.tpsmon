/*
 * main.c
 *
 * Created: 2012-04-09 4:35:26 PM
 *  Author: Justin
 * 1.0.0.0		-			CW		First Release
 */ 

#include <avr32/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "font.h"
#include "color.h"
#include "h2framework.h"
#include "h2core.h"

#include "pcd.h"
#include "debug.h"

#include "ini.h"
#include "util.h"
#include "fposix.h"

#include "scom.h"
#include "tcp.h"

//typedef enum bool{false=0;true=1};
#define bool int
#define true 1
#define false 0

#define Y2K 946684800

int UPDATE_FLAG = 0;
int HB_RESET_FLAG = 0;

//========================================================================================================================================================================
void ShowProgressMessage(char* message1, char* message2, unsigned char beeptone);
void ShowProgressMessage(char* message1, char* message2, unsigned char beeptone)
{
	#define xStatus 0
	#define yStatus	224
	#define widthStatus 320
	#define heightStatus 16
	
	if(beeptone != 0)
		beepstd();
	lcd_draw_fillrect(xStatus, yStatus, widthStatus, heightStatus, BLACK);
	if(message1 != 0)
		lcd_draw_string(message1, font_Fixesys16, xStatus, yStatus, WHITE, TRANSPARENT);
	if(message2 != 0)
		lcd_draw_string(message2, font_Fixesys16, xStatus+160, yStatus, WHITE, TRANSPARENT);
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#define FS_ERR_OK      0
#define FS_ERR_CREATE -1
#define FS_ERR_WRITE  -2
#define FS_ERR_READ   -3
#define FS_ERR_APPEND -4
#define FS_ERR_DELETE -5

#define CONFIG_FILENAME "_config.ini"

#define STR_CFG 1
#define STR_STS 2
#define STR_ATT 3

#define CMD_CFG "CFG"
#define CMD_STS "STS"
#define CMD_ATT "ATT"

void read_config(int* periodHB){
	periodHB = ini_getint(CONFIG_FILENAME, "configurations", "duration", 60);
	ini_free_resource();
}
//void write_config(){}

void getDeviceID(char* deviceIdHex);
void getDeviceID(char* deviceIdHex){
	//unsigned char deviceID[8] ; // in binary
	//int iRet = 0;
	//iRet = pcd_get_id(deviceID);
	//if(iRet==0){
		//sprintf(deviceIdHex,"%02X%02X%02X%02X%02X%02X%02X%02X",deviceID[0],deviceID[1],deviceID[2],deviceID[3],deviceID[4],deviceID[5],deviceID[6],deviceID[7]);
	//} else  { 
		//sprintf(deviceIdHex,"");
	//}
	char* getid[20];//  = ini_getstring(CONFIG_FILENAME, "configurations", "deviceid", "57000");
	int idlen =0;
	
	pcd_get_info(PCD_EEA_PRODUCT_SN, getid, idlen);// PCD Serial Number
			
	for(int i = 0; i<idlen; i++){	
		sprintf(&deviceIdHex[strlen(deviceIdHex)], "%c", getid[i]);
	}
	
	memset(deviceIdHex,0,sizeof(deviceIdHex));
	sprintf(deviceIdHex,"%s", getid);
	//ini_free_resource();
}

unsigned int getPrevHB(unsigned int currentDT,unsigned int startHB,unsigned int periodHB);
unsigned int getPrevHB(unsigned int currentDT,unsigned int startHB,unsigned int periodHB){
	return (currentDT -((currentDT-startHB) % periodHB));
}

unsigned int getNextHB(unsigned int currentDT,unsigned int startHB,unsigned int periodHB);
unsigned int getNextHB(unsigned int currentDT,unsigned int startHB,unsigned int periodHB){
	return (currentDT -((currentDT-startHB) % periodHB) + periodHB);
}

void lcd_reset(void);
void lcd_reset(void){
	lcd_draw_fillrect( 0, 0, LCD_WIDTH, LCD_HEIGHT-16,BLACK);
};

void clear_lcd_draw_string(char *lcd_string, const unsigned char *font_style, unsigned short x, unsigned short y, unsigned short fcolor, int bcolor);
void clear_lcd_draw_string(char *lcd_string, const unsigned char *font_style, unsigned short x, unsigned short y, unsigned short fcolor, int bcolor)
{
	lcd_reset();
	lcd_draw_string(lcd_string, font_style, x,  y, fcolor, bcolor);
	
}


static unsigned short days[4][12] =
{
    {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335},
    { 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700},
    { 731, 762, 790, 821, 851, 882, 912, 943, 974,1004,1035,1065},
    {1096,1127,1155,1186,1216,1247,1277,1308,1339,1369,1400,1430},
};


unsigned int getTime(int timezone);
unsigned int getTime(int timezone)
{
	unsigned char sdatetime7[7];
	//char messageBuffer[20];
	int epoch_timezone = timezone*60*60;
					
    //                                                                                                               Y              M                D             H         M             SS              
	if (sys_get_datetime(sdatetime7) >0) //rtc=isl1208.h YYMMDDHH24MISS "20%02d-%02d-%02d %02d:%02d:%02d %d %d",sdatetime7[0],sdatetime7[1],sdatetime7[2],sdatetime7[3],sdatetime7[4],sdatetime7[5],sdatetime7[6]
	{  unsigned int second = sdatetime7[5];  // 0-59
       unsigned int minute = sdatetime7[4];  // 0-59
       unsigned int hour   = sdatetime7[3];    // 0-23
       unsigned int day    = sdatetime7[2]-1;   // 0-30
       unsigned int month  = sdatetime7[1]-1; // 0-11
       unsigned int year   = sdatetime7[0];    // 0-99
	   
	   

	   //rtc_to_short_string(sdatetime7,messageBuffer); // 16-10-10 10:59:59
	   //ShowProgressMessage(messageBuffer, 0, 0);
	   
	   return (((year/4*(365*4+1)+days[year%4][month]+day)*24+hour)*60+minute)*60+second+Y2K-epoch_timezone;
				
	} else {
	  lcd_draw_string("NO DATE !!!", font_MsSerif24, 0, 0, BLACK, TRANSPARENT);
	  sound_negative();
	  return 0;
	}
	
}

typedef struct
{
    unsigned char serverip;		// server IP
    unsigned char serverport;	// server Port
    unsigned char deviceid;		// device Identification
    unsigned int time;			// configured heartbeat time
    unsigned int duration;		// configured heartbeat duration
    unsigned char reset;		// update heartbeat after pair flag	
	unsigned int prevstart;		// previous heartbeat start
	unsigned int prevstop;		// previous heartbeat stop
}
configd;

typedef struct
{
    unsigned char second; // 0-59
    unsigned char minute; // 0-59
    unsigned char hour;   // 0-23
    unsigned char day;    // 1-31
    unsigned char month;  // 1-12
    unsigned char year;   // 0-99 (representing 2000-2099)
}
date_time_t;

void epoch_to_date_time(date_time_t* date_time,unsigned int epoch,int timezone);
void epoch_to_date_time(date_time_t* date_time,unsigned int epoch,int timezone)
{
	epoch=epoch-Y2K+(timezone*60*60);
    date_time->second = epoch%60; epoch /= 60;
    date_time->minute = epoch%60; epoch /= 60;
    date_time->hour   = epoch%24; epoch /= 24;

    unsigned int years = epoch/(365*4+1)*4; epoch %= 365*4+1;

    unsigned int year;
    for (year=3; year>0; year--)
    {
        if (epoch >= days[year][0])
            break;
    }

    unsigned int month;
    for (month=11; month>0; month--)
    {
        if (epoch >= days[year][month])
            break;
    }

    date_time->year  = years+year;
    date_time->month = month+1;
    date_time->day   = epoch-days[year][month]+1;
}

void getEventCode(char* eventcode,int ac,int batt,int tamper);
void getEventCode(char* eventcode,int ac,int batt,int tamper){
	/*
	900 = No Event
	901 = No Power
	902 = No Battery
	903 = Low Battery
	904 = Tampered Case
	*/
	sprintf(eventcode,"900");
	if(ac==0){sprintf(eventcode,"901");};
	if(batt==0){sprintf(eventcode,"902");};
	if(tamper==0){sprintf(eventcode,"904");};
}

void getCommand(char* display,int str_option,int timezone);	
void getCommand(char* display,int str_option,int timezone){
	/* STRING FORMAT
	unsigned char strCFG[] = "CFG:4{s:3:DEV;s:#IDLEN#:#IDSTRING#;s:3:DTM;i;#TIME#;}";
	unsigned char strSTS[] = "STS:12{s:3:DEV;s:3:123;s:3:EVT;s:3:900;s:3:AON;i:1;s:3:BON;i:1;s:3:TMP;i:1;s:3:DTM;i;1476853800;}";
	unsigned char strATT[] = "ATT:10{s:3:DEV;s:3:123;s:3:TAG;s:6:151263;s:3:DTM;i;1476853800;s:3:STT;i;1476853700;s:3:STP;i;1476853900;}";
	*/
	//**********************************************************************************************************
	char deviceID[] = "123";
	int deviceIDLen = strlen(deviceID);
	int dTimestamp = getTime(timezone);
		
	int ac_on = 1;
	int batt_on = 1;
	int tamper_on = 1;
	char eventCode[] = "900";
	
	char tagID[] = "RSN123456";
	int tagIDLen = strlen(tagID);
	
	int dHBStart = 1476853400;
	int dHBStop = 1476853900;
	//**********************************************************************************************************
	
	switch(str_option) {
		case 1  :
			sprintf(display,"CFG:4{s:3:DEV;s:%i:%s;s:3:DTM;i;%i;}",deviceIDLen,deviceID,dTimestamp);
			break; /* optional */
	
		case 2  :
			sprintf(display,"STS:12{s:3:DEV;s:%i:%s;s:3:EVT;s:3:%s;s:3:AON;i:%i;s:3:BON;i:%i;s:3:TMP;i:%i;s:3:DTM;i;%i;}",deviceIDLen,deviceID,eventCode,ac_on,batt_on,tamper_on,dTimestamp);
			break; /* optional */
  
		case 3  :
			sprintf(display,"ATT:10{s:3:DEV;s:%i:%s;s:3:TAG;s:%i:%s;s:3:DTM;i;%i;s:3:STT;i;%i;s:3:STP;i;%i;}",deviceIDLen,deviceID,tagIDLen,tagID,dTimestamp,dHBStart,dHBStop);
			break; /* optional */
	  
		default : /* Optional */
			strcpy(display,"");
	}
}
//========================================================================================================================================================================
int scom_Disconnect(){
	int scomDisconnectSt = 0;
	
	ShowProgressMessage("Turn Off 3G", 0, 0);
	
	scomDisconnectSt = scom_set_ppp(0,0,0,0);
	
	ShowProgressMessage("Turning Off 3G", 0, 0);
	sleep(2000);
	if(scomDisconnectSt == 0){
		ShowProgressMessage("Turn Off 3G-ed", 0, 0);	
		sleep(500);
		return 0;
	}else{
		ShowProgressMessage("Turn Off 3G Failed", 0, 1);
		return -1;
	}
}

int scom_Status(struct_pppStatus* outPppStatus, bool display);
int scom_Status(struct_pppStatus* outPppStatus, bool display){
	int scomFlagStatus = 0;
	char bufferError[10]; // strcpy(bufferError,"");
	char bufferSignal[10];
	int signalStrength = 0;
	
	// check signal
	if(display) ShowProgressMessage("Signal Status", 0, 0);
	signalStrength = scom_get_signal_strength();
	sprintf(bufferSignal,"Signal %d", signalStrength);
	if(display) ShowProgressMessage(bufferSignal, 0, 0);
	sleep(1000);//mandatory
	
	
	if(display) ShowProgressMessage("3G Status", 0, 0);
	scomFlagStatus = scom_get_status_flags(outPppStatus);

	if(display) ShowProgressMessage("3G Checking Status", 0, 0);
	// 0 means ok
	if(scomFlagStatus == 0){
		if(display){
			ShowProgressMessage("3G Checking scom Status OK", 0, 0);
			//display all Status Out
			lcd_draw_fillrect(120, 30, 80, 130, BLACK);
			lcd_draw_string(outPppStatus->active, font_Fixesys16, 120, 30, WHITE, TRANSPARENT);
			sprintf(bufferError,"%d",outPppStatus->Error);
			lcd_draw_string(bufferError, font_Fixesys16, 120, 45, WHITE, TRANSPARENT);
			lcd_draw_string(outPppStatus->dev, font_Fixesys16, 120, 60, WHITE, TRANSPARENT);
			lcd_draw_string(outPppStatus->module, font_Fixesys16, 120, 75, WHITE, TRANSPARENT);
			sprintf(bufferSignal,"%d",outPppStatus->Signal);
			lcd_draw_string(bufferSignal, font_Fixesys16, 120, 90, WHITE, TRANSPARENT);
			lcd_draw_string(outPppStatus->APN, font_Fixesys16, 120, 105, WHITE,TRANSPARENT);
			lcd_draw_string(outPppStatus->userId, font_Fixesys16, 120, 120, WHITE, TRANSPARENT);
			lcd_draw_string(outPppStatus->Password, font_Fixesys16, 120, 135, WHITE, TRANSPARENT);
		}
		return 0;
	}else{
		if(display) ShowProgressMessage("3G Checking scom Status RETURN1", 0, 0);
		return 1;
	}					
}

int scom_on_ppp(){
	int scomConnectSt = 0;
	
	ShowProgressMessage("3G Turn On", 0, 0);
	
	scomConnectSt = scom_set_ppp(1,0,0,0);
	
	ShowProgressMessage("3G Turning On", 0, 0);  
	
	//return scomConnectSt;
	
	if(scomConnectSt == 0){
		//sleep(60000);      //Wait 1.10min to turn on 3G
		sleep(10000);
		ShowProgressMessage("3G On!!!", 0, 0);
		sleep(500);
		return 0;
	}else{
		ShowProgressMessage("3G Turn On Failed", 0, 1);
		return -1;
	}
	
}

int scom_Tcp_Connect(int* outTCPID,char* ipAddress,char* port);
int scom_Tcp_Connect(int* outTCPID,char* ipAddress,char* port){
	int tcpCID = 0;
	
	ShowProgressMessage("TCP Connect", 0, 0);
	tcpCID = tcp_connect(ipAddress, port);
	ShowProgressMessage("TCP Connecting", 0, 0);
	if(tcpCID>0){
		*outTCPID = tcpCID;
	}else{
		ShowProgressMessage("TCP Connect Fail", 0, 0);
		return -1;
	}
	ShowProgressMessage("TCP Connected", 0, 0);
	return 0;	
}

int scom_Tcp_Write(int tcpCID,char* data,int datalen);
int scom_Tcp_Write(int tcpCID,char* data,int datalen){
	int scomtcpWriteSt = 0;
	
	ShowProgressMessage("TCP Write", 0, 0);
	scomtcpWriteSt = tcp_write(tcpCID,data,datalen);
	ShowProgressMessage("TCP Writing", 0 ,0);
	if(scomtcpWriteSt == 0){
		ShowProgressMessage("TCP Successful Write", 0, 0);
		return 0;
	}else{
		ShowProgressMessage("TCP Write Fail", 0, 0);
		return -1;
	}
}


int scom_Tcp_Read(int* intcpCID){
	char* respData = 0;
	unsigned char bufData[500] = {0};
	int tcpReadLength = 0;
	int i = 0;
	int errCode=0;
		
	ShowProgressMessage("TCP Reading...", 0, 0);
	
	tcpReadLength = tcp_read(intcpCID,&respData,20000);

	if(tcpReadLength>0){
		
		ShowProgressMessage("TCP Read Successful", 0, 0);
		
		// Answer of server is "1" (pb) or "0" (ok) only 
		if (tcpReadLength==1){
			
			switch(respData[0]) {
			   case 0X30  :// Acknowledge OK
					ShowProgressMessage("OK", 0 ,0);
					break; /* optional */
	
			   case 0X31  :// Acknowledge NOT OK
					ShowProgressMessage("NOT OK", 0 ,0);
					errCode=1;
					break; /* optional */
  
			   /* you can have any number of case statements */
			   default : /* Optional */
					ShowProgressMessage("Invalid response", 0 ,0);
					errCode=2;
			}
			
		}else 
		// Server answer "STS:0" (status ok) or maybe "STS:1" (nok) 
		if (tcpReadLength==5){
			
			if((respData[0]==0X53)&&(respData[1]==0X54)&&(respData[2]==0X53)&&(respData[4]==0X30)){
				ShowProgressMessage("STS OK response", 0 ,0);
			}else{
				ShowProgressMessage("Invalid response", 0 ,0);
				errCode=3;
			}
						
		}
		// Server Responses
		// CFG:4:{s:3:HBT;i:1476147600;s:3:HBD;i:1454;s:3:SNC;i:1476859314;s:3:HBU;i:1;}
		// ATT:1:{s:3:UPD;i:0;} (for the pairing serveice)
		else{
			
			char tag[100][500];
			int tagno = 0;
			
			// explode the strint
			for(i = 0; i<tcpReadLength; i++){
	
				if((respData[i]==0X7B)||(respData[i]==0X3A)||(respData[i]==0X3B)||(respData[i]==0X7D)){
					strcpy(tag[tagno++],bufData);
					memset(bufData,0,sizeof(bufData));
				}else{
					sprintf(&bufData[strlen(bufData)], "%c", respData[i]);
				}
				
			}	
						
			char curTag[3];
			if(strcmp(tag[0],"CFG") == 0){// Configuration Data response
				ShowProgressMessage("Configuration Data response", 0, 0);
				sleep(100);
				int item = atoi(tag[1]);			
					
				for(int i=2; i< tagno; i++){
						
					if(strcmp(tag[i],"s") == 0){
							
						if(strlen(tag[i+2])==atoi(tag[i+1])){
							sprintf(curTag, "%s", tag[i+2]);
						}// else - for string values							
							
					}else if(strcmp(tag[i],"i") == 0){
											
		    			if(strcmp(curTag,"HBT") == 0){// Heartbeat time							
							ini_setstring(CONFIG_FILENAME, "configurations", "time", tag[i+1]);	
							NULL;
						}
		    			if(strcmp(curTag,"HBD") == 0){// Heartbeat duration
							ini_setstring(CONFIG_FILENAME, "configurations", "duration", tag[i+1]);
							NULL;
						}
		    			if(strcmp(curTag,"SNC") == 0){// Server time
									
							//unsigned char sdatetime7[7];
							//char strdt[10];
							//int response=0;
							//response = sys_get_datetime(sdatetime7);//rtc=isl1208.h YYMMDDHH24MISS
							//if(response>0){
								//rtc_to_short_string(sdatetime7,bufData); // 16-10-10 10:59:59
								//lcd_draw_string(bufData, font_Fixesys16, 0, 10, BLACK, WHITE);
							//}
									
							date_time_t sTime;
							int servertime = atoi(tag[i+1]);
							epoch_to_date_time(&sTime,servertime,8);
							//sprintf(bufData,"DATE= (20%02u-%02u-%02u %02u:%02u:%02u)",sTime.year,sTime.month,sTime.day,sTime.hour,sTime.minute,sTime.second);
							//lcd_draw_string(bufData, font_Fixesys16, 0, 30, BLACK, WHITE);
									
							unsigned char newTime[7];
							newTime[0] = sTime.year;   //y
							newTime[1] = sTime.month;  //m
							newTime[2] = sTime.day;    //d
							newTime[3] = sTime.hour;   //h
							newTime[4] = sTime.minute; //m
							newTime[5] = sTime.second; //s
							newTime[6] = 0x01;         //z
																
							if(sys_set_datetime (newTime)!=0){ShowProgressMessage("sys_set_datetime Failed", 0, 0);};
									
							//response = sys_get_datetime(sdatetime7);//rtc=isl1208.h YYMMDDHH24MISS
							//if(response>0){
								//rtc_to_short_string(sdatetime7,bufData); // 16-10-10 10:59:59
								//lcd_draw_string(bufData, font_Fixesys16, 0, 50, BLACK, WHITE);
							//}
						}
		    			if(strcmp(curTag,"HBU") == 0){// Reset heartbeat after pairing
							if(atoi(tag[i+1])==0){
								ini_setstring(CONFIG_FILENAME, "configurations", "reset", "N");
								NULL;
							}else{
								ini_setstring(CONFIG_FILENAME, "configurations", "reset", "Y");
								NULL;
							}									
						}
	    				
		    			item=item-1;							
					}
					if(item==0)break;
				}
				
				// Other values
				
				
				
				ini_commit();// Commit INI file changes
				ini_free_resource();
					
				//int vtime, vduration;
				//char * vreset;					
				//vtime = ini_getint(CONFIG_FILENAME, "configurations", "time", 0);	
				//vduration = ini_getint(CONFIG_FILENAME, "configurations", "duration", 60);					
				//vreset = ini_getstring(CONFIG_FILENAME, "configurations", "reset", "N");
					//
				//sprintf(bufData,"time = %d\nduration = %d\nreset = %s", vtime,vduration,vreset);										
				//lcd_draw_string(bufData, font_Fixesys16, 0, 100, BLACK, WHITE);
					
			}else if(strcmp(tag[0],"ATT") == 0){// ATT Data response
				ShowProgressMessage("ATT Data response", 0, 0);
				sleep(100);
				int item = atoi(tag[1]);
					
				for(int i=2; i< tagno; i++){
						
					if(strcmp(tag[i],"s") == 0){
						if(strlen(tag[i+2])==atoi(tag[i+1])){
							sprintf(curTag, "%s", tag[i+2]);
						}							
					}else if(strcmp(tag[i],"i") == 0){
		    			if(strcmp(curTag,"UPD") == 0){
							if(strcmp(tag[i+1],"1") == 0){
								UPDATE_FLAG=1;
							}else{
								UPDATE_FLAG=0;
							}								
							item=item-1;
						}
					}
					if(item==0)break;
				}
					
			}else{
				ShowProgressMessage("Invalid response", 0, 0);
				errCode = 4;
			};
			
			// flush remainingbbuffer
			tcp_flush(*intcpCID, tcpReadLength);
			
		}
	
	}else{
		// zero lenght !
		ShowProgressMessage("TCP Read Failed", 0, 0);
		errCode = 5;
	}
	
	return errCode;
}

int scom_Tcp_Disconnect(int inTcpId);
int scom_Tcp_Disconnect(int inTcpId){
	int scomTcpDisconnectSt = 0;
	
	ShowProgressMessage("TCP Disconnect", 0, 0);
	scomTcpDisconnectSt = tcp_disconnect(inTcpId);
	ShowProgressMessage("TCP Disconnecting", 0, 0);
	
	if(scomTcpDisconnectSt == 0){
		ShowProgressMessage("TCP Successful Disconnected", 0, 0);
		//sleep(1000);
		return 0;
	}else{
		ShowProgressMessage("TCP Failed Disconnect", 0, 0);
		//sleep(5000);
		return 1;
	}
}

bool CheckConnection3G(struct_pppStatus outPppStatus);
bool CheckConnection3G(struct_pppStatus outPppStatus){
	int scomFlagStatus = 0;
	scomFlagStatus = scom_Status(&outPppStatus,false);
	if(scomFlagStatus != 0)
	{
		//scomDisconnectStatus = scom_Disconnect();
	    return false;
	}else{
		// chech ppp is active
		if(strcmp(outPppStatus.active,"On")!= 0)
		{   
			ShowProgressMessage("active is off", 0, 0);sleep(200);
		    return false;
		};
		return true;
	}
	
	
}

bool Connection3G(struct_pppStatus outPppStatus,bool notify);
bool Connection3G(struct_pppStatus outPppStatus,bool notify)
{
	//init length y axis display	
	int y_lcd = 30;
	int y_lcdst = 30;
	
	int counterDialing = 0;
	int scomDisconnectStatus = 0;
	int scomFlagStatus = 0;
	int scomConnectStatus = 0;
//0;
	//struct_pppStatus outPppStatus;
	
	unsigned int iTickWaitPppOn = 0;
	
	
	
	h2core_set_debug(H2HW_3G|H2HW_USB|H2HW_TTL);
	
	//Draw Something
	lcd_draw_fillrect(0, 0, LCD_WIDTH, LCD_HEIGHT, BLACK);
	lcd_draw_string("TPS COMM", font_MsSerif24, 0, 0, WHITE, TRANSPARENT);
	
	lcd_draw_string("Active  :", font_Fixesys16, 0, y_lcd, WHITE, TRANSPARENT);
	y_lcd += 15;
	lcd_draw_string("Error   :", font_Fixesys16, 0, y_lcd, WHITE, TRANSPARENT);
	y_lcd += 15;
	lcd_draw_string("Dev     :", font_Fixesys16, 0, y_lcd, WHITE, TRANSPARENT);
	y_lcd += 15;
	lcd_draw_string("Module  :", font_Fixesys16, 0, y_lcd, WHITE, TRANSPARENT);
	y_lcd += 15;
	lcd_draw_string("Signal  :", font_Fixesys16, 0, y_lcd, WHITE, TRANSPARENT);
	y_lcd += 15;
	lcd_draw_string("APN     :", font_Fixesys16, 0, y_lcd, WHITE, TRANSPARENT);
	y_lcd += 15;
	lcd_draw_string("UserID  :", font_Fixesys16, 0, y_lcd, WHITE, TRANSPARENT);
	y_lcd += 15;
	lcd_draw_string("Password:", font_Fixesys16, 0, y_lcd, WHITE, TRANSPARENT);
	
	h2core_task();//to turn off watchdog ???
	
	//
	//TURN ON PPP
	//
	RECONNECT:
	scomConnectStatus = scom_on_ppp();
	if(scomConnectStatus != 0){
		//failed to turn on
		//wait 2 sec and try again
		sleep(2000);
		
		scom_set_ppp(0,0,0,0);
		
		if(notify) sound_negative();
		goto RECONNECT;
	
	}
	iTickWaitPppOn = gettickcount() + 120000;//wait up to 120sec for module to register to network, subject to signal

	//Get SCOM Status
	RESTATUS1:
	scomFlagStatus = scom_Status(&outPppStatus,true);
	if(scomFlagStatus != 0)
	{
		ShowProgressMessage("scom error", 0, 0);
		if(notify) sound_negative();
		sleep(2000);
		scomDisconnectStatus = scom_Disconnect();
		lcd_draw_fillrect(0, 0, LCD_WIDTH, LCD_HEIGHT, BLACK);
	    return false;
	}
	
	if(strcmp(outPppStatus.active,"On")!= 0)
	{   
		ShowProgressMessage("active is off", 0, 0);sleep(200);
		if(gettickcount() > iTickWaitPppOn)
		{
			if(notify) sound_negative();
			sleep(2000);
			scomDisconnectStatus = scom_Disconnect();
			lcd_draw_fillrect(0, 0, LCD_WIDTH, LCD_HEIGHT, BLACK);
			return false;
		}
		else
		{
			sleep(1000);
			goto RESTATUS1;
		}
	}
		
		
   // ok
	
	
	beepstd();
	lcd_draw_fillrect(0, 0, LCD_WIDTH, LCD_HEIGHT, BLACK);
	return true;
	
	//
	//END: Exit App, return back to main application sector
	//
	//h2core_exit_to_main_sector();
}

int TCPSEND(char* ipAddress,char* port, char* toSendCommands);
int TCPSEND(char* ipAddress,char* port, char* toSendCommands){
	
	int tcpCID;
	int scomTcpConnectStatus = 0;
	int scomTcpWriteStatus = 0;
	int scomTcpReadStatus = 0;
	int scomTcpDisconnectStatus = 0;
	int TCPSEND_Response = true;

	int max_retry = 5;
	//Connect TCP
	RECONNECTTCP:
	scomTcpConnectStatus = scom_Tcp_Connect(&tcpCID,ipAddress,port);
	if(scomTcpConnectStatus != 0){
		if (max_retry>0){
			max_retry--;
			goto RECONNECTTCP;
		}else{
			//sound_negative();
			TCPSEND_Response = false;
			goto ENDTCP;
		}						
	}
					
	//Tcp Write
	scomTcpWriteStatus = scom_Tcp_Write(tcpCID,toSendCommands,strlen(toSendCommands));
	if(scomTcpWriteStatus != 0){
		TCPSEND_Response =false;
		goto DISCONNECTTCP;	
	}else{
		sleep(1000);
	}
		
	//Tcp Read
	scomTcpReadStatus = scom_Tcp_Read(&tcpCID);
	if(scomTcpReadStatus != 0){
		TCPSEND_Response=false;
		goto DISCONNECTTCP;
	}else{
		sleep(1000);
	}
	
	//TCPSEND_Response = true;		
	//TCP Disconnect
	DISCONNECTTCP:	
	scomTcpDisconnectStatus = scom_Tcp_Disconnect(tcpCID);
	if(scomTcpDisconnectStatus != 0){
		// error message;
	}
	                
	ENDTCP:NULL;
	return TCPSEND_Response;
}

int isReset(void);
int isReset(void){
	char* getid  = ini_getstring(CONFIG_FILENAME, "configurations", "reset", "N");
	ini_free_resource();
	if(strcmp(getid,"Y") == 0){
		return 1;
	}else{
		return 0;
	}	
}
//========================================================================================================================================================================
//
// START:
//
void (*entry)(void);
int main(void)
{
	int whileINT;
	int iRet = 0;
	int iRespLen = 0;
	unsigned char respbuf[100];
	char messageBuffer[500];
	
	char deviceIdHex[20];    // in string hex
	
	// Card info
	char ATQAHex[20]; // ATQA in HEX
	char UID[50]; // UID in HEX
	char SAK[10]; // SAK in HEX
	unsigned char SAKdec; // SAK in decimal
	
	// Date
	unsigned int dateTPS;
	unsigned int timezone=8;
	
	// hb config stuff
	unsigned int startHB=0;
	unsigned int periodHB=60;
	
	// Calculated HB stuff
	unsigned int previousHB =0;
	unsigned int nextHB =0 ;
		
	// communication info
	struct_pppStatus outPppStatus;
	
	int isConnected = 0;
    char toSendCommands[10][500];
	int toSendCommandsNb = 0;
	for(int i=0;i<10;i++){strcpy(toSendCommands[i],"");};
	int period_retry=300;
	
	//
	// Init all hardware
	//
	h2core_init(0);
	
	//Draw Something
	lcd_draw_fillrect(0, 0, LCD_WIDTH, LCD_HEIGHT, GREEN);
	clear_lcd_draw_string("Please Tab Card", font_MsSerif24, 0, 0, BLACK, TRANSPARENT);
	//beepstd();//Beep
	
	ShowProgressMessage("Turn On RF", 0, 0);
	//Turn ON RF
	pcd_rf_on();

	// READ CONFIG FILE - SERVER
	char* ipAddress = ini_getstring(CONFIG_FILENAME, "server", "ip", "192.168.40.110");
	char* port = ini_getstring(CONFIG_FILENAME, "server", "port", "57000");
	
	// READ CONFIG FILE - HEARTBEAT
	startHB = ini_getint(CONFIG_FILENAME, "configurations", "time", 0);
	periodHB = ini_getint(CONFIG_FILENAME, "configurations", "duration", 60);
	previousHB = ini_getint(CONFIG_FILENAME, "heartbeat", "prevstart", 0);
	nextHB = ini_getint(CONFIG_FILENAME, "heartbeat", "prevstop", 0);		
	
	ini_free_resource();
	
   // GET DEVICE ID
   getDeviceID(deviceIdHex);
   
	// Connect
	if (Connection3G(outPppStatus,true)){ShowProgressMessage("3g succesfully connected", 0, 0);	isConnected=1;
	 					//Send Configuration request
					sprintf(messageBuffer,"CFG:4{s:3:DEV;s:%d:%s;s:3:DTM;i;%u;}",
					strlen(deviceIdHex),deviceIdHex,
					getTime(timezone)
					);
						
					TCPSEND(ipAddress,port,messageBuffer);	
	};
		
	while(1)
	{
		//Mandatory core task
		h2core_task();
				
		//Exit when BACK button is pressed	
		
		// GET THE CARD ID		
		char UIDL1[10]; char UIDL2[10]; char UIDL3[10]; unsigned char resSAK[100];
		iRet = pcd_reqa(respbuf);
		//iRet = pcd_wupa(respbuf);
		if(iRet == 0)
		{	
			// read the ATQA	
			led_on(LED_1);
			sprintf(ATQAHex,"%02X:%02X",respbuf[1],respbuf[0]);
			sprintf(messageBuffer,"ATQA=(%s)               ",ATQAHex);
			clear_lcd_draw_string(messageBuffer, font_default16, 0, 50, BLUE, BLACK);
			//sleep(2000);
		
		    
		    // Get the UID L1
			iRet=pcd_anticol(CASLEVEL1,respbuf);
			if(iRet!=0)
			{ 
			  clear_lcd_draw_string("LEVEL 1.A ALERT", font_default16, 0, 50, BLUE, BLACK);
			  sound_negative();
			  continue;				
			};
			
			// debug
			//sprintf(messageBuffer,"ATQA debug1=(%s)               ",ATQAHex);	clear_lcd_draw_string(messageBuffer, font_default16, 0, 50, BLUE, BLACK); sleep(2000);
			
			if (respbuf[0]!=136)  // todo: wrong test !!!!
			
			{ sprintf(UIDL1,"%02X:%02X:%02X:%02X",respbuf[0],respbuf[1],respbuf[2],respbuf[3]);
			  sprintf(UID,"%s",UIDL1);
			  
			  // debug
			  sprintf(messageBuffer,"FINAL UIDL1=(%s) CT=(%u)\nAtqa=(%s)",UID,respbuf[0],ATQAHex);		
			  clear_lcd_draw_string(messageBuffer, font_default16, 0, 50, BLUE, BLACK);
			  //sleep(5000);
			
			   // get and check sak
			   iRet=pcd_select(CASLEVEL1,respbuf,resSAK);
			   if(iRet!=0)
				{ 
				  clear_lcd_draw_string("LEVEL 1.B1 ALERT", font_default16, 0, 50, BLUE, BLACK);
				  sound_negative();
				  continue;				
				};
			   sprintf(SAK,"%02X",resSAK[0]);	SAKdec=resSAK[0];
			   
			   //debug
			   sprintf(messageBuffer,"SAKL1=(%s)             ",SAK);		
			   clear_lcd_draw_string(messageBuffer, font_default16, 0, 50, BLUE, BLACK);
			
			   //sleep(3000);
			} else
			{   
                 sprintf(UIDL1,"%02X:%02X:%02X",respbuf[1],respbuf[2],respbuf[3]);
				 iRet=pcd_select(CASLEVEL1,respbuf,resSAK);
				 if(iRet!=0)
				 { 
					  clear_lcd_draw_string("LEVEL 1.B2 ALERT", font_default16, 0, 50, BLUE, BLACK);
					  sound_negative();
					  continue;				
				 };
				 
				 
				 	// debug
			      // sprintf(messageBuffer,"ATQA debug2=(%s)               ",ATQAHex);	clear_lcd_draw_string(messageBuffer, font_default16, 0, 50, BLUE, BLACK); sleep(2000);
				 
				 // LEVEL 2
				 // Get the UID L2
				iRet=pcd_anticol(CASLEVEL2,respbuf);
				if(iRet!=0)
				{ 
				  clear_lcd_draw_string("LEVEL 2.A ALERT", font_default16, 0, 50, BLUE, BLACK);
				  sound_negative();
				  continue;				
				};
					// debug
			      // sprintf(messageBuffer,"ATQA debug3=(%s)               ",ATQAHex);	clear_lcd_draw_string(messageBuffer, font_default16, 0, 50, BLUE, BLACK); sleep(2000);
			
			
				if (respbuf[0]!=136)
				{   sprintf(UIDL2,"%02X:%02X:%02X:%02X",respbuf[0],respbuf[1],respbuf[2],respbuf[3]);
					sprintf(UID,"%s:%s",UIDL1,UIDL2);
					
					//debug
					sprintf(messageBuffer,"FINAL UIDL2=(%s)\nATQA=(%s)",UID,ATQAHex);		
					clear_lcd_draw_string(messageBuffer, font_default16, 0, 50, BLUE, BLACK);
					//sleep(5000);
			
					// get and check sak L2
					iRet=pcd_select(CASLEVEL2,respbuf,resSAK);
					if(iRet!=0)
						{ 
						  clear_lcd_draw_string("LEVEL 2.B1 ALERT", font_default16, 0, 50, BLUE, BLACK);
						  sound_negative();
						  continue;				
						};
					sprintf(SAK,"%02X",resSAK[0]);SAKdec=resSAK[0];
					
					//debug
					sprintf(messageBuffer,"SAKL2=(%s)            ",SAK);		
					clear_lcd_draw_string(messageBuffer, font_default16, 0, 50, BLUE, BLACK);
					//sleep(3000);
				}			
				else
				{ sprintf(UIDL2,"%02X:%02X:%02X",respbuf[1],respbuf[2],respbuf[3]);
				  iRet=pcd_select(CASLEVEL2,respbuf,resSAK);
				  if(iRet!=0)
					{ 
					  clear_lcd_draw_string("LEVEL 2.B2 ALERT", font_default16, 0, 50, BLUE, BLACK);
					  sound_negative();
					  continue;				
					};
				  
				  // LEVEL3
				  iRet=pcd_anticol(CASLEVEL3,respbuf);
				  if(iRet!=0)
					{ 
					  clear_lcd_draw_string("LEVEL 3.A ALERT", font_default16, 0, 50, BLUE, BLACK);
					  sound_negative();
					  continue;				
					};
					
				  sprintf(UIDL3,"%02X:%02X:%02X:%02X",respbuf[0],respbuf[1],respbuf[2],respbuf[3]);
				  sprintf(UID,"%s:%s:%s",UIDL1,UIDL2,UIDL3);
				  
				  sprintf(messageBuffer,"FINAL UIDL3=(%s)",UID);		
						clear_lcd_draw_string(messageBuffer, font_default16, 0, 50, BLUE, BLACK);
						//sleep(3000);
			
						// get and check sak L3
						iRet=pcd_select(CASLEVEL3,respbuf,resSAK);
						if(iRet!=0)
						{ 
						  clear_lcd_draw_string("LEVEL 3.B ALERT", font_default16, 0, 50, BLUE, BLACK);
						  sound_negative();
						  continue;				
						};
						sprintf(SAK,"%02X",resSAK[0]);SAKdec=resSAK[0];
						
						// debug
						sprintf(messageBuffer,"SAKL3=(%s)            ",SAK);		
						clear_lcd_draw_string(messageBuffer, font_default16, 0, 50, BLUE, BLACK);
						//sleep(3000);
				}			
				 
			};
				
			dateTPS = getTime(timezone);
						
			// CARD ID IS =
			sprintf(messageBuffer,"FINAL CARD UID=(%s)\nATQA=(%s)\nSAK=(%s)\nDate=(%u)\nDevice=(%s)",UID,ATQAHex,SAK,dateTPS,deviceIdHex);	
			clear_lcd_draw_string(messageBuffer, font_default16, 0, 50, BLUE, BLACK);
		
		    // todo : check SAK for positive
			if (SAKdec==24 || SAKdec==4)
			{
				sound_negative();
			 	//sleep(5000);
			    lcd_reset();
				continue;
			};
			
			
			// bip bip
			sound_positive();
			
			char NEWUID[50];
			
			strcpy(NEWUID,UID);
			for(int i=0;i<strlen(NEWUID);i++)
			{
				if(NEWUID[i]==':')NEWUID[i]='-';
			}
			
			// TODO : Storage in queue
			sprintf(messageBuffer,"ATT:10{s:3:DEV;s:%d:%s;s:3:TAG;s:%d:%s;s:3:DTM;i;%u;s:3:STT;i;%u;s:3:STP;i;%u;}",
						strlen(deviceIdHex),deviceIdHex,
						strlen(NEWUID),NEWUID,
						dateTPS,
						previousHB,
						nextHB
				);
			strcpy(toSendCommands[toSendCommandsNb++],messageBuffer);
			
			sleep(500);
			lcd_reset();
			
			//RESET HEARTBEAT
			//startHB
			if(isReset()==1){startHB=dateTPS;};
			
			continue;
		} // type a
		
		iRet = pcd_reqb(respbuf, &iRespLen);
		if(iRet == 0)
		{
			led_on(LED_1);
			clear_lcd_draw_string("TYPE B CARD", font_MsSerif24, 0, 50, BLUE, BLACK);
			sleep(100);
			//sprintf(messageBuffer,"",pcd_get_id(respbuf));
		    //ShowProgressMessage(messageBuffer, 0, 0);
			continue;
		}
		
		//clear_lcd_draw_string(iRet, font_MsSerif24, 0, 50, BLUE, BLACK);
		//sleep(2000);
		//Else No Card
		led_off(LED_1);
		lcd_draw_string("     -- NO CARD --      ", font_MsSerif24, 0, 50, BLUE, BLACK);
		
		
		unsigned int currentDT = getTime(timezone);
		// get previous
		previousHB = getPrevHB(currentDT,startHB,periodHB);
		nextHB = getNextHB(currentDT,startHB,periodHB);
		
		int countdownSec = (nextHB - currentDT);
		//sprintf(messageBuffer,"DATE= (%u)   \nPREV= (%u)    \nNEXT= (%u)    \nCNT=  (%d)    ",currentDT, previousHB, nextHB, countdownSec);
		//convert epoct to time
		date_time_t currentDTTime,previousHBTime,nextHBTime;
		epoch_to_date_time(&currentDTTime,currentDT,timezone);
		epoch_to_date_time(&previousHBTime,previousHB,timezone);
		epoch_to_date_time(&nextHBTime,nextHB,timezone);
		
		//unsigned char second; // 0-59
		//unsigned char minute; // 0-59
		//unsigned char hour;   // 0-23
		//unsigned char day;    // 1-31
		//unsigned char month;  // 1-12
		//unsigned char year;   // 0-99 (representing 2000-2099)
		
		
		//sprintf(messageBuffer,"DATE= (%u)   \nPREV= (%u)    \nNEXT= (%u)    \nCNT=  (%d)    ",currentDT, previousHB, nextHB, countdownSec);
		
		sprintf(messageBuffer,"DATE= (20%02u-%02u-%02u %02u:%02u:%02u)   \nPREV= (20%02u-%02u-%02u %02u:%02u:%02u)    \nNEXT= (20%02u-%02u-%02u %02u:%02u:%02u)    \nCNT=  (%d)    ",
				currentDTTime.year,currentDTTime.month,currentDTTime.day,currentDTTime.hour,currentDTTime.minute,currentDTTime.second,
				previousHBTime.year,previousHBTime.month,previousHBTime.day,previousHBTime.hour,previousHBTime.minute,previousHBTime.second,
				nextHBTime.year,nextHBTime.month,nextHBTime.day,nextHBTime.hour,nextHBTime.minute,nextHBTime.second,
				countdownSec);
	
		lcd_draw_string(messageBuffer, font_default16, 50, 100, BLUE, BLACK);
	
	     // replace 	
		sprintf(messageBuffer,"                                               ");	
		ShowProgressMessage(messageBuffer, 0, 0);
		
		 // if not connected, reconnect
		 // todo : check connection still on currenttimeb mod 300
		 // check if cnnect every 5 min or if sometihing to send
		
		
		if (((currentDT % period_retry)==0) || (toSendCommandsNb != 0))
		{ 
			   // check
			   ShowProgressMessage("Check if 3G enabled", 0, 0);
			   
               if (CheckConnection3G(outPppStatus)==true) {
					isConnected=1;
					ShowProgressMessage("3G enabled", 0, 0);sleep(500);
				}else{
					// try reconnect
					ShowProgressMessage("3g disabled, try to reconnect", 0, 0);sleep(100);
					if (Connection3G(outPppStatus,true)){isConnected=1;ShowProgressMessage("3g connected", 0, 0); }else {ShowProgressMessage("3g failed", 0, 0);isConnected=0;};
				} 
		};
		        		 
		
		// is connected, send the buffer
		int isErrorSending = false;
		if (isConnected && toSendCommandsNb !=0 )
		{
			ShowProgressMessage("try to send tcp", 0, 0);
			int resTCP;
			
		    for	(int i=0; i< toSendCommandsNb; i++)
			{
				sprintf(messageBuffer,"TCP SEND Nb %d/%d",i+1,toSendCommandsNb);
				ShowProgressMessage(messageBuffer, 0, 0);
				sleep(500);
				resTCP = TCPSEND(ipAddress,port,toSendCommands[i]);
				if(resTCP==true){
					ShowProgressMessage("TCP SEND OK", 0, 0); sleep(500);
					//isErrorSending = false;
				}else{
					ShowProgressMessage("TCP SEND NOT OK", 0, 0); sleep(500);
					
					isErrorSending = true;
					
					// todo move array if not first element
					if (i!=0)
					{   int jdest=0; 
                        for (int jsrc=i; jsrc < toSendCommandsNb ; jsrc++)
						{ 
							sprintf(messageBuffer,"Copy Nb %d to Nb %d",jsrc,jdest);
							ShowProgressMessage(messageBuffer, 0, 0);

							strcpy(toSendCommands[jdest] , toSendCommands[jsrc]);
							jdest++;
						};
					
					    toSendCommandsNb=jdest;
					};					
					break;		
				}
				sleep(500);				
			};
			
			// if all went well
				//sprintf(messageBuffer,"End Sending error = %d",isErrorSending);
				//ShowProgressMessage(messageBuffer, 0, 0);sleep(500);
			
			if (isErrorSending==false) 
			{ toSendCommandsNb=0;
				//ShowProgressMessage("RESET queue to 0", 0, 0); sleep(500);
			
				//Configuration request layer					
				if(UPDATE_FLAG==1){
					ShowProgressMessage("Update available", 0, 0);sleep(100);

					//Send Configuration request
					sprintf(messageBuffer,"CFG:4{s:3:DEV;s:%d:%s;s:3:DTM;i;%u;}",
					strlen(deviceIdHex),deviceIdHex,
					dateTPS
					);
						
					TCPSEND(ipAddress,port,messageBuffer);
					
					read_config(periodHB);// Read new Heartbeat period
					//sprintf(messageBuffer,"period = %d",periodHB);
					//lcd_draw_string(messageBuffer, font_default16, 0, 10, BLUE, BLACK);		
					UPDATE_FLAG=0;
				}
			};			
		}	
		
		sleep(500);
	} // while
	
	ShowProgressMessage("Turn Off RF", 0, 0);
	pcd_rf_off();
	//beepstd();
	
	//
	//END: Exit App, return back to main application sector
	//
	h2core_exit_to_main_sector();
}

