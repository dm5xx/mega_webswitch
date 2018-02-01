/*
*  This is the mega webswitch version 2.1 for the remoteQth.com
*  If you need help, feel free to contact DM5XX@gmx.de
*  Sketch is developed with IDE Version 1.6.12 and later
*
*  This is free software. You can redistribute it and/or modify it under
*  the terms of Creative Commons Attribution 3.0 United States License if at least the version information says credits to remoteQTH.com :P
*
*  To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/us/
*  or send a letter to Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
*
*	Tribute to OL7M!
*  LLAP!
*/

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <Time.h>
#include <TimeLib.h>
#include <EepromUtil.h>

//#define FILLARRAY(a,n) a[0]=n, memcpy( ((char*)a)+sizeof(a[0]), a, sizeof(a)-sizeof(a[0]) );

#define READFROMSD // if this is set, the config data fromthe sd card is used
//#define DEBUG
#define DEBUGSERVER // serial print out server start information
#define PREVENTSAMESLOTDIFFERENTBANK // A slot cannot be switched on, if a slot with the same number on a different slot is already on - aka Antenna-Lock. Normally used for ANTSWITCHMODE - but who knows.. :P
#define ANTSWITCHMODE // set the firmware into antenna switch mode
#define ANTSWITCHMIXED // If you want to use the unused banks of a mega webswitch in ANTSWITCHMODE, you can define, which bank should be "there can be only one"-ON or where multiple relays can be on. No 5s/1s On/Off functionality than in standard mode! DONT FORGET TO DEFINE THE BANKS at excludeBankFromSlotLock below! DONT FORGET TO CHANGE JS-File :P

//#define USEGROUNDPIN // if your antenna switch has a ground pin relais/logic, you can switch it on there
//#define WRITETOEEPROM // store your current switch state in the eeprom every x minutes (see eepromWatchdogTimer in seconds) NOT TESTED IN THIS VERSION

#define SS_SD_CARD   4
#define SS_ETHERNET 10


//byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE }; // This is the mac adress of your device... IF YOU HAVE MORE THAN ONE DEVICE, make sure, this settings are unique...
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xAA, 0xFE, 0xEE }; // This is the mac adress of your device... IF YOU HAVE MORE THAN ONE DEVICE, make sure, this settings are unique...

String defaultIP = "192.168.1.178"; // This is the ip of your device... 
//String defaultPingIP = "192.168.1.178"; // This is the ip of your device... 

EthernetServer server(80);                                  
byte rip[] = { 0,0,0,0 };
char requestString[100];
File myFile;

// default values. will be overwritten, if you use a sd card
String ajaxUrl = "http://mmmedia.selfhost.eu"; // default: same as your ip above, but if you have a DynDNS - this is the url to call... dont forget to enable port forwarding on your router
String deviceIp = defaultIP; // same as your ip above...
//String pingIp = defaultPingIP;
String title = "RemoteQth WebSwitch"; 

#ifdef ANTSWITCHMODE

	#ifdef ANTSWITCHMIXED
		String jsUrl = "http://h.mmmedia-online.de/mega_webswitch_412_mix.js";
	#else
		String jsUrl = "http://h.mmmedia-online.de/mega_webswitch_412.js";
	#endif // ANT

	String contentUrl = "http://h.mmmedia-online.de/mega_webswitch_content_412.js";
	String cssUrl = "http://h.mmmedia-online.de/mega_webswitch_412.css";

	// arrays to store the state if a button shoud be stay online even if its switched of aka "5s-ON"-Buttons
	// everything is left as 16 part arrays. this means more ram, but less loops
	boolean stayOnPinsBank0[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	boolean stayOnPinsBank1[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	boolean stayOnPinsBank2[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	boolean stayOnPinsBank3[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

	//arrays to define which buttons are long-on-button. startin with 0, setting 1 to the long switch buttons. must be 1:1 matched to the js buttons/description
	boolean is5sPinBank0[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 3 in bank0
	boolean is5sPinBank1[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 3 in bank1
	boolean is5sPinBank2[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 6 in bank2
	boolean is5sPinBank3[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 0 in bank 3

	boolean isOffPinBank0[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // define if a button is a long-off-button. uses index from js file as everything else
	boolean isOffPinBank1[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	boolean isOffPinBank2[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	boolean isOffPinBank3[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

	byte indexOfOffPinPairBank0[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // definition which on-index the off-index belongs to
	byte indexOfOffPinPairBank1[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	byte indexOfOffPinPairBank2[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	byte indexOfOffPinPairBank3[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

	// push duration - Adjust the individual push duration of a "long push button" here - On & Off
	byte pushDurationBank0[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	byte pushDurationBank1[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	byte pushDurationBank2[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	byte pushDurationBank3[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

#else
	String jsUrl = "http://h.mmmedia-online.de/mega_webswitch.js";
	String contentUrl = "http://h.mmmedia-online.de/mega_webswitch_content.js";
	String cssUrl = "http://h.mmmedia-online.de/mega_webswitch.css";

	// arrays to store the state if a button shoud be stay online even if its switched of aka "5s-ON"-Buttons
	// everything is left as 16 part arrays. this means more ram, but less loops
	// here you can define everything u need... like relais who are switched on for some seconds and go off aka latch buttons, etc.. Ask for help if you dont know what to do.. :P
	boolean stayOnPinsBank0[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	boolean stayOnPinsBank1[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	boolean stayOnPinsBank2[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	boolean stayOnPinsBank3[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

	//arrays to define which buttons are long-on-button. startin with 0, setting 1 to the long switch buttons. must be 1:1 matched to the js buttons/description
	boolean is5sPinBank0[] = { 0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,0 }; // 3 in bank0
	boolean is5sPinBank1[] = { 0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0 }; // 3 in bank1
	boolean is5sPinBank2[] = { 0,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0 }; // 6 in bank2
	boolean is5sPinBank3[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 0 in bank 3

	boolean isOffPinBank0[] = { 0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0 }; // define if a button is a long-off-button. uses index from js file as everything else
	boolean isOffPinBank1[] = { 0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0 };
	boolean isOffPinBank2[] = { 0,0,0,1,0,1,0,1,0,1,0,0,0,1,0,1 };
	boolean isOffPinBank3[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

	byte indexOfOffPinPairBank0[] = { 0,0,0,0,0,0,5,0,0,8,0,0,11,0,0,0 }; // definition which on-index the off-index belongs to
	byte indexOfOffPinPairBank1[] = { 0,0,0,2,0,0,0,0,0,0,9,0,11,0,0,0 };
	byte indexOfOffPinPairBank2[] = { 0,0,0,2,0,4,0,6,0,8,0,0,0,12,0,14 };
	byte indexOfOffPinPairBank3[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

	// push duration - Adjust the individual push duration of a "long push button" here - On & Off
	byte pushDurationBank0[] = { 0,0,0,0,0,5,1,0,5,1,0,5,1,0,0,0 };
	byte pushDurationBank1[] = { 0,0,1,1,0,0,0,0,0,5,1,1,1,0,0,0 };
	byte pushDurationBank2[] = { 0,0,5,1,1,1,5,1,1,1,0,0,5,1,5,1 };
	byte pushDurationBank3[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
#endif

String faviconUrl ="http://h.mmmedia-online.de/favicon.ico";
String dotUrl="http://h.mmmedia-online.de/dot.png";
String jqueryUrl = "http://code.jquery.com/jquery-1.11.2.min.js";

// the Pins definition array. Remember this are the only pin numbers. allother array values belong to the indexs defined here.
byte pinsBank0[] = { 54, 2, 3, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18}; // remember do not use 0/1. 54 will be A0, 55 will be A1
byte pinsBank1[] = { 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34}; 
byte pinsBank2[] = { 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 53}; 
byte pinsBank3[] = { 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 1};

// Arrays to know whos on or off...
byte statusBank0[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
byte statusBank1[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
byte statusBank2[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
byte statusBank3[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

//
#ifdef PREVENTSAMESLOTDIFFERENTBANK
	boolean excludeBankFromSlotLock[4] = { 1,0,0,1 }; // exclude complete banks from locking in 4x12 mode to use banks in normal mode. Dont forget to change it in the js file!
#endif // 



#ifdef ANTSWITCHMODE // if you want to run the webswitch to controll only antenna switches... 
// 
byte bandAntennaMapping0[16] = { 160,80,81,40,41,30,20,21,17,15,12,10,0,0,0,0 };
byte bandAntennaMapping1[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
byte bandAntennaMapping2[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
byte bandAntennaMapping3[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

	#ifdef USEGROUNDPIN // set the pins you want to use as your groundpins here. if a a button of a bank is pressed, the groundpin of the bank is set to high
		byte groundPins[] = { 15, 31, 47, 67 };
		byte groundSlotNrs[] = { 12, 12, 12, 12 }; // array position 13 of 16... counting starts by 0... So dont forget to change this while changing number per layers... :P
	#endif // GROUNDPIN
byte numberOfAntennasPerLayer[] = { 12, 12, 12, 12 };
#endif // ANTSWITCH412MODE



// The long On Push -array contains the timestamp when the status should be cleared and the watchdog hits...
unsigned long longOnCleanUpTimeBank0[16];
unsigned long longOnCleanUpTimeBank1[16];
unsigned long longOnCleanUpTimeBank2[16];
unsigned long longOnCleanUpTimeBank3[16];

// The time the button is pushed - to be released after 1s. the time store is the time it has to be switched off and the watchdog hits, not the time is is added to the array!
unsigned long longOffCleanUpTimeBank0[16]; 
unsigned long longOffCleanUpTimeBank1[16]; 
unsigned long longOffCleanUpTimeBank2[16]; 
unsigned long longOffCleanUpTimeBank3[16];

unsigned long lastWatchdogRun;
long token = 0; // take any number between this number and one... the higher the better
char charToken[17] = "Abcdgfghij123!#*";
int salt = 3333;
//           	33196680 would be the shifted and slated endresult... for above values. You can find your values using the SecuGenerator.html

String strpinsBank0;
String strpinsBank1;
String strpinsBank2;
String strpinsBank3;
String stris5sPinBank0;
String stris5sPinBank1;
String stris5sPinBank2;
String stris5sPinBank3;
String strisOffPinBank0;
String strisOffPinBank1;
String strisOffPinBank2;
String strisOffPinBank3;
String strindexOfOffPinPairBank0;
String strindexOfOffPinPairBank1;
String strindexOfOffPinPairBank2;
String strindexOfOffPinPairBank3;
String strpushDurationBank0;
String strpushDurationBank1;
String strpushDurationBank2;
String strpushDurationBank3;
String strexcludeBankFromSlotLock;

#ifdef ANTSWITCHMODE // webswitch antennas only
String strbandAntennaMapping0;
String strbandAntennaMapping1;
String strbandAntennaMapping2;
String strbandAntennaMapping3;

	#ifdef USEGROUNDPIN
		String strGroundPins;
		String strGroundSlotNrs; // array position 13 of 16... counting starts by 0... :P
	#endif // USEGROUNDPIN


String strNumberOfAntennasPerLayer;
#endif // ANTSWITCHMODE


#ifdef WRITETOEEPROM 
EepromUtil ee;
const int STRINGSIZE = 24;
char lastState[STRINGSIZE];
const int eepromMemoryStartAddress = 0; // 0....31, 32....63, ... If you realize that your arduino is loosing memory (antenna states after restart), change the startaddress
unsigned long eepromWatchdogLastRun;
const unsigned long eepromWatchdogTimer = 300;
int changesDuringInverval = 0;
#endif // WR

void setup()
{
 
	pinMode(SS_SD_CARD, OUTPUT);
	pinMode(SS_ETHERNET, OUTPUT);
	digitalWrite(SS_SD_CARD, HIGH);  // SD Card not active
	digitalWrite(SS_ETHERNET, HIGH); // Ethernet not active

#ifdef DEBUGSERVER
  Serial.begin(115200);

  while (!Serial) {
	  ; // wait for serial port to connect. Needed for native USB port only
  }
#endif

#ifdef READFROMSD
// disable ethernet
  digitalWrite(SS_SD_CARD, LOW);  // SD Card ACTIVE

  Serial.print("Initializing SD card...");

  if (!SD.begin(SS_SD_CARD)) {
	  Serial.println("sd initialization failed!");
  }
  else
  {
	  Serial.println("sd cardinitialization done.");
	  readSDSettings("config_a.cfg");
	  initBanksFromSDCard();
  }
  delay(100);
  digitalWrite(SS_SD_CARD, HIGH); // SD Card not active
#endif

  byte ip0 = getPartOfStringBySeperatorAndAppearance(deviceIp, '.', 0).toInt();
  byte ip1 = getPartOfStringBySeperatorAndAppearance(deviceIp, '.', 1).toInt();
  byte ip2 = getPartOfStringBySeperatorAndAppearance(deviceIp, '.', 2).toInt();
  byte ip3 = getPartOfStringBySeperatorAndAppearance(deviceIp, '.', 3).toInt();

  IPAddress ip(ip0, ip1, ip2, ip3);

  digitalWrite(SS_ETHERNET, LOW);  // Ethernet ACTIVE
  Ethernet.begin(mac, ip);
  server.begin();          // Server starten
  
#ifdef DEBUGSERVER
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
#endif

  // set mode for all needed pins in each bank
  for (int out = 0; out < 16; out++)
  {
	  pinMode(pinsBank0[out], OUTPUT);
	  pinMode(pinsBank1[out], OUTPUT);
	  pinMode(pinsBank2[out], OUTPUT);
	  pinMode(pinsBank3[out], OUTPUT);
	  digitalWrite(pinsBank0[out], LOW);
	  digitalWrite(pinsBank1[out], LOW);
	  digitalWrite(pinsBank2[out], LOW);
	  digitalWrite(pinsBank3[out], LOW);
  }

  adjustTime(10); // dont start with 0... think about buffer underruns of longs :P 
  lastWatchdogRun = now(); // set the whatchdog to current time. since there is no timesync, its second "10" in the year 0 (1970)

  chrTokenConverter();

#ifdef WRITETOEEPROM
  initLaststate();
#endif // WRITETOEEPROM

}


/************************************************************************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++ LOOP ++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/************************************************************************************************************************/
void loop()
{
  // the the watchdog hit once a second. dont waste performance by calling to often...
  if(lastWatchdogRun <= now()-1)
  {
    WatchdogBank0();
    WatchdogBank1();
    WatchdogBank2();
	WatchdogBank3();
	lastWatchdogRun = now();
  }
  
#ifdef WRITETOEEPROM
  if (changesDuringInverval > 0)
	  writeToMemory(); // writes to memory
#endif // WRITETOEEPROM


  // Create a client connection
  EthernetClient client = server.available();
  if (client) {
	  client.getRemoteIP(rip);
	byte charIndex = 0;

    while (client.connected()) { 
      if (client.available()) {
        char c = client.read();
		
		if (charIndex < 100) {
			requestString[charIndex] = c;
			charIndex++;
		}

#ifdef DEBUG
       Serial.print(c);
#endif		
         //if HTTP request has ended
         if (requestString[0] == 'G' && c == '\n') {
			 long myToken;
			 boolean cmdSet = isSubStringIncluded(requestString, "Set/"); // see if its a set request
			 boolean cmdGet = isSubStringIncluded(requestString, "Get/"); // see if its a get request
			 boolean cmdGetAll = isSubStringIncluded(requestString, "GetAll"); // see if its a get request

#ifdef ANTSWITCHMODE
			 boolean cmdSetBand = isSubStringIncluded(requestString, "SetBand"); // see if its a get request
#endif


#ifdef DEBUG
		   Serial.println("Client IP is:");
		   for (int bcount = 0; bcount < 4; bcount++)
		   {
			   Serial.print(rip[bcount], DEC);
			   if (bcount<3) Serial.print(".");
		   }
		   Serial.println("-------------------------");
		   Serial.print("Set is ");
		   Serial.println(cmdSet);
		   Serial.print("Get is ");
		   Serial.println(cmdGet);
		   Serial.print("GetAll is ");
		   Serial.println(cmdGetAll);
		   Serial.print("SetBand is ");
#ifdef ANTSWITCHMODE
		   Serial.println(cmdSetBand);
#endif
#endif
 
		   if(cmdSet > 0)
           {			 
			 char set_bankNr[2] = { '\0','\0'};
			 char set_bankValues[6] = {'\0','\0', '\0', '\0', '\0', '\0'};
			 char set_currentToken[11] = { '\0','\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0', '\0' };

			 int set_curBank = 0;
			 long set_curPin = 0;
			 long set_myToken = 0;

			 getValuesByUrl(requestString, '/', 2, set_bankNr); // the 3nd part is the decimal-value to react on
			 getValuesByUrl(requestString, '/', 3, set_bankValues); // the 3nd part is the decimal-value to react on
			 getValuesByUrl(requestString, '/', 4, set_currentToken); // the 3nd part is the decimal-value to react on

			 set_curBank = atoi(set_bankNr);
			 set_curPin = atol(set_bankValues);
			 set_myToken = atol(set_currentToken);

			 if (!validateToken(set_myToken, client))
			 {
				 sendEmptyPage(client); // return the http status
				 return;
			 }
			 else
			 {
				 setPinsByStrValue(set_curBank, set_curPin, client);
			 }
           }
		   else if (cmdGetAll > 0)
		   {
			   char ga_currentToken[11] = { '\0','\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0', '\0' };
			   getValuesByUrl(requestString, '/', 2, ga_currentToken); // the 3nd part is the decimal-value to react on
			   long ga_myToken = atol(ga_currentToken);

			   GetAllBanksData(client, ga_myToken);
		   }
           else if(cmdGet > 0)
           {
			   char g_currentToken[11] = { '\0','\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0', '\0' };
			   getValuesByUrl(requestString, '/', 3, g_currentToken); // the 3nd part is the decimal-value to react on
			   long g_myToken = atol(g_currentToken);

			   char g_bankNr[2] = { '\0','\0' };
			   int g_curBank = 0;
			   getValuesByUrl(requestString, '/', 2, g_bankNr); // the 3nd part is the decimal-value to react on
			   g_curBank = atoi(g_bankNr);
				GetSingleBankData(client, g_curBank, g_myToken);
           }
           
#ifdef ANTSWITCHMODE
		   else if (cmdSetBand > 0)
		   {
			   char seta_bankNr[2] = { '\0','\0' };
			   char seta_bankValues[6] = { '\0','\0', '\0', '\0', '\0', '\0' };
			   char seta_currentToken[11] = { '\0','\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0', '\0' };

			   getValuesByUrl(requestString, '/', 2, seta_bankNr); // the 3nd part is the decimal-value to react on
			   getValuesByUrl(requestString, '/', 3, seta_bankValues); // the 3nd part is the decimal-value to react on
			   getValuesByUrl(requestString, '/', 4, seta_currentToken); // the 3nd part is the decimal-value to react on

			   int seta_curBank = atoi(seta_bankNr);
			   long seta_curBand = atol(seta_bankValues);
			   long seta_myToken = atol(seta_currentToken);

			   if (!validateToken(seta_myToken, client))
			   {
				   sendEmptyPage(client); // return the http status
				   return;
			   }
			   else
			   {
				   if (seta_curBank == 0)
				   {
					   for (byte s = 0; s < 16; s++)
					   {
						   if (bandAntennaMapping0[s] == seta_curBand)
							   setPinsBySlotNumber(seta_curBank, s, client);
					   }
				   }
				   else if (seta_curBank == 1)
				   {
					   for (byte s = 0; s < 16; s++)
					   {
						   if (bandAntennaMapping1[s] == seta_curBand)
							   setPinsBySlotNumber(seta_curBank, s, client);
					   }
				   }
				   else if (seta_curBank == 2)
				   {
					   for (byte s = 0; s < 16; s++)
					   {
						   if (bandAntennaMapping2[s] == seta_curBand)
							   setPinsBySlotNumber(seta_curBank, s, client);
					   }
				   }
				   else if (seta_curBank == 3)
				   {
					   for (byte s = 0; s < 16; s++)
					   {
						   if (bandAntennaMapping3[s] == seta_curBand)
							   setPinsBySlotNumber(seta_curBank, s, client);
					   }
				   }
			   }
		   }
#endif // ANTSWITCHMODE
           else
           {
			   char currentToken[11] = { '\0','\0', '\0', '\0', '\0', '\0', '\0','\0', '\0', '\0', '\0' };
			   getValuesByUrl(requestString, '/', 1, currentToken); // the 3nd part is the decimal-value to react on
			   long _myToken = atol(currentToken);

			   if (rip[0] == 192 || rip[0] == 10)
				   MainPage(client, true, _myToken);
			   else
				   MainPage(client, false, _myToken);
		   }
		   memset(requestString, 0, sizeof(requestString));
		   myToken = 0;
  
           delay(1);
           client.stop();
         }
       }
    }
}
}

/**********************************************************************************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Watchgogs +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**********************************************************************************************************************************/

void WatchdogBank0() // as im too lazy to use pointers, i copy pasted my code :P
{
    for(byte s = 0; s < 16; s++)
    {
      if(longOnCleanUpTimeBank0[s] > 0 && longOnCleanUpTimeBank0[s]<= now()) // default is 0 => only do something if there is something to do...
      {
        stayOnPinsBank0[s] = 1; // set the pin to permanent ON at index 
        digitalWrite(pinsBank0[s], LOW); // switch of this pin
        longOnCleanUpTimeBank0[s] = 0; // set its corresponding settingtime to default 0;
      }
      if(longOffCleanUpTimeBank0[s] > 0 && longOffCleanUpTimeBank0[s]<= now())
      {
        byte foundIndex = indexOfOffPinPairBank0[s];
        stayOnPinsBank0[foundIndex] = 0; // set the pin to OFF at index
        digitalWrite(pinsBank0[s], LOW); 
        digitalWrite(pinsBank0[foundIndex], LOW); // set the pins direkt if you push faster a Off-button than its released (remember, its a 5s "deleay" after an on pin is set to off again
        longOffCleanUpTimeBank0[s] = 0; // remove off timestamp
        longOnCleanUpTimeBank0[foundIndex] = 0; // remove on timestamp - same issue as with digital write above...
      }
    }
}

void WatchdogBank1()
{
    for(byte s = 0; s < 16; s++)
    {
      if(longOnCleanUpTimeBank1[s] > 0 && longOnCleanUpTimeBank1[s]<= now())
      {
        stayOnPinsBank1[s] = 1;  
        digitalWrite(pinsBank1[s], LOW); 
        longOnCleanUpTimeBank1[s] = 0; 
      }
      if(longOffCleanUpTimeBank1[s] > 0 && longOffCleanUpTimeBank1[s]<= now())
      {
        byte foundIndex = indexOfOffPinPairBank1[s];
        stayOnPinsBank1[foundIndex] = 0;
        digitalWrite(pinsBank1[s], LOW);
        digitalWrite(pinsBank1[foundIndex], LOW);
        longOffCleanUpTimeBank1[s] = 0;
        longOnCleanUpTimeBank1[foundIndex] = 0;
      }
    }
}

void WatchdogBank2()
{
    for(byte s = 0; s < 16; s++)
    {
      if(longOnCleanUpTimeBank2[s] > 0 && longOnCleanUpTimeBank2[s]<= now())
      {
        stayOnPinsBank2[s] = 1;  
        digitalWrite(pinsBank2[s], LOW); 
        longOnCleanUpTimeBank2[s] = 0; 
      }
      if(longOffCleanUpTimeBank2[s] > 0 && longOffCleanUpTimeBank2[s]<= now())
      {
        byte foundIndex = indexOfOffPinPairBank2[s];
        stayOnPinsBank2[foundIndex] = 0;
        digitalWrite(pinsBank2[s], LOW);
        digitalWrite(pinsBank2[foundIndex], LOW);
        longOffCleanUpTimeBank2[s] = 0;
        longOnCleanUpTimeBank2[foundIndex] = 0;
      }
    }
}

void WatchdogBank3()
{
	for (byte s = 0; s < 16; s++)
	{
		if (longOnCleanUpTimeBank3[s] > 0 && longOnCleanUpTimeBank3[s] <= now())
		{
			stayOnPinsBank3[s] = 1;
			digitalWrite(pinsBank3[s], LOW);
			longOnCleanUpTimeBank3[s] = 0;
		}
		if (longOffCleanUpTimeBank3[s] > 0 && longOffCleanUpTimeBank3[s] <= now())
		{
			byte foundIndex = indexOfOffPinPairBank3[s];
			stayOnPinsBank3[foundIndex] = 0;
			digitalWrite(pinsBank3[s], LOW);
			digitalWrite(pinsBank3[foundIndex], LOW);
			longOffCleanUpTimeBank3[s] = 0;
			longOnCleanUpTimeBank3[foundIndex] = 0;
		}
	}
}


/**********************************************************************************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++ HELPER METHODS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**********************************************************************************************************************************/

String getPartOfStringBySeperatorAndAppearance(String data, char separator, int index)
{
  int stringData = 0; 
  String dataPart = "";    
  for(int i = 0; i<data.length(); i++) 
  {      
    if(data[i]==separator) 
      stringData++;
    else if(stringData==index) 
      dataPart.concat(data[i]);
    else if(stringData>index) 
    {
      return dataPart;
      break;  
    }
  }
  return dataPart;
}

void getValuesByUrl(char* data, char separator, int index, char pdata[])
{
	byte seperatorCounter = 0;
	byte charcounter = 0;

	for (int i = 0; i < strlen(data); i++)
	{
		if (*(data + i) == separator || ((seperatorCounter == index && *(data + i) == ' ') || (seperatorCounter == index && *(data + i) == '?')))
			seperatorCounter++;
		else if (seperatorCounter == index)
		{
			pdata[charcounter] = *(data + i);
			charcounter++;
		}
		else if (seperatorCounter > index)
		{
			return;
		}
	}
}

char * convertIntegerValueInBinaryString(unsigned int x)
{
  static char mybuffer[17];
  for (int i=0; i<16; i++) mybuffer[15-i] = '0' + ((x & (1 << i)) > 0);
  mybuffer[16] ='\0';
  
  return mybuffer;
}

unsigned int convertBinaryStringInDecimalSum(String bin)
{
  unsigned int i = 1;
  unsigned int sum = 0;
 
  for (int m = 15; m >= 0; m--)  // do it from the back...
  {
    sum += (bin.charAt(m)-48) * i;
    i = i * 2;
  }     
  
  return sum;
}

String revertBinaryString(String aString)
{
  String temp;
  for (int a = 15; a >= 0; a--)
  {    
    temp += aString.charAt(a);
  }
  return temp;
}

int ipow(int base, int exp)
{
	int result = 1;
	while (exp)
	{
		if (exp & 1)
			result *= base;
		exp >>= 1;
		base *= base;
	}

	return result;
}


// easy way to convert binary arrays to bitshifted decimal values...
unsigned int getEncodedSum(byte* bin)
{
	unsigned int i = 1;
	unsigned int sum = 0;

	for (int m = 0; m < 16; m++)  // do it from the back...
	{
		byte temp = *(bin + m);
		sum += temp * i;
		i = i * 2;
	}
	return sum;
}

#ifdef READFROMSD
void readSDSettings(char *filename)
{
	char character;
	String settingName;
	String settingValue;
	myFile = SD.open(filename);
	if (myFile)
	{
		while (myFile.available())
		{
			character = myFile.read();
			while ((myFile.available()) && (character != '['))
			{
				character = myFile.read();
			}
			character = myFile.read();
			while ((myFile.available()) && (character != '='))
			{
				settingName = settingName + character;
				character = myFile.read();
			}
			character = myFile.read();
			while ((myFile.available()) && (character != ']'))
			{
				settingValue = settingValue + character;
				character = myFile.read();
			}
			if (character == ']')
			{

				//Debuuging Printing
#ifdef DEBUG
				Serial.print("Name:");
				Serial.println(settingName);
				Serial.print("Value :");
				Serial.println(settingValue);
#endif

				if (settingName == "ajaxUrl")
					ajaxUrl = settingValue;

				else if (settingName == "deviceIp")
					deviceIp = settingValue;

				else if (settingName == "title")
					title = settingValue;

				else if (settingName == "jsUrl")
					jsUrl = settingValue;

				else if (settingName == "contentUrl")
					contentUrl = settingValue;

				else if (settingName == "cssUrl")
					cssUrl = settingValue;

				else if (settingName == "faviconUrl")
					faviconUrl = settingValue;

				else if (settingName == "dotUrl")
					dotUrl = settingValue;
				else if (settingName == "jqueryUrl")
					jqueryUrl = settingValue;

				else if (settingName == "strpinsBank0")
					strpinsBank0 = settingValue;
				else if (settingName == "strpinsBank1")
					strpinsBank1 = settingValue;
				else if (settingName == "strpinsBank2")
					strpinsBank2 = settingValue;
				else if (settingName == "strpinsBank3")
					strpinsBank3 = settingValue;
				else if (settingName == "stris5sPinBank0")
					stris5sPinBank0 = settingValue;
				else if (settingName == "stris5sPinBank1")
					stris5sPinBank1 = settingValue;
				else if (settingName == "stris5sPinBank2")
					stris5sPinBank2 = settingValue;
				else if (settingName == "stris5sPinBank3")
					stris5sPinBank3 = settingValue;
				else if (settingName == "strisOffPinBank0")
					strisOffPinBank0 = settingValue;
				else if (settingName == "strisOffPinBank1")
					strisOffPinBank1 = settingValue;
				else if (settingName == "strisOffPinBank2")
					strisOffPinBank2 = settingValue;
				else if (settingName == "strisOffPinBank3")
					strisOffPinBank3 = settingValue;
				else if (settingName == "strindexOfOffPinPairBank0")
					strindexOfOffPinPairBank0 = settingValue;
				else if (settingName == "strindexOfOffPinPairBank1")
					strindexOfOffPinPairBank1 = settingValue;
				else if (settingName == "strindexOfOffPinPairBank2")
					strindexOfOffPinPairBank2 = settingValue;
				else if (settingName == "strindexOfOffPinPairBank3")
					strindexOfOffPinPairBank3 = settingValue;
				else if (settingName == "strpushDurationBank0")
					strpushDurationBank0 = settingValue;
				else if (settingName == "strpushDurationBank1")
					strpushDurationBank1 = settingValue;
				else if (settingName == "strpushDurationBank2")
					strpushDurationBank2 = settingValue;
				else if (settingName == "strpushDurationBank3")
					strpushDurationBank3 = settingValue;
				else if (settingName == "strexcludeBankFromSlotLock")
					strexcludeBankFromSlotLock = settingValue;
#ifdef ANTSWITCHMODE
				else if (settingName == "strbandAntennaMapping0")
					strbandAntennaMapping0 = settingValue;
				else if (settingName == "strbandAntennaMapping1")
					strbandAntennaMapping1 = settingValue;
				else if (settingName == "strbandAntennaMapping2")
					strbandAntennaMapping2 = settingValue;
				else if (settingName == "strbandAntennaMapping3")
					strbandAntennaMapping3 = settingValue;

			#ifdef USEGROUNDPIN
				else if (settingName == "strGroundPins")
					strGroundPins = settingValue;
				else if (settingName == "strGroundSlotNrs")
					strGroundSlotNrs = settingValue;
				else if (settingName == "strNumberOfAntennasPerLayer")
					strNumberOfAntennasPerLayer = settingValue;
			#endif
#endif // ANTSWITCHMODE

				else if (settingName == "strToken")
					settingValue.toCharArray(charToken, 17);
				else if (settingName == "intSalt")
					salt = settingValue.toInt();

				settingName = "";
				settingValue = "";
			}
		}
		myFile.close();
	}
	else
	{
#ifdef DEBUG
		// if the file didn't open, print an error:
		Serial.println("error opening settings.txt");
#endif
	}
}

#endif // READFROMSD

void getByteArray(String strResponse, byte tempByteArray[])
{
	String tempBy[16];
	getStringArray(strResponse, tempBy);

	for (int a = 0; a < 16; a++)
		tempByteArray[a] = (byte)tempBy[a].toInt();
}

void getBoolArray(String strResponse, boolean tempBoolArray[])
{
	String tempBol[16];
	getStringArray(strResponse, tempBol);

	for (int a = 0; a < 16; a++)
		tempBoolArray[a] = (boolean)tempBol[a].toInt();
}

void getStringArray(String strResponse, String tempResult[])
{
	// Convert from String Object to String.
	char buf[strResponse.length() + 1];
	strResponse.toCharArray(buf, sizeof(buf));
	char *p = buf;
	char *str;
	int cntr;
	while ((str = strtok_r(p, ",", &p)) != NULL) // delimiter is the semicolon
	{
		tempResult[cntr] = String(str);
		cntr += 1;
	}
}

boolean isSubStringIncluded(char *req, char *substring)
{
	char *sub;
	sub = strstr(req, substring);

	if (strlen(sub) > 1)
		return true;
	return false;
}


/**************************************************************************************************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++ INDEX AND PAGE CONTROL METHODS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**************************************************************************************************************************************************/

void setPinsOfBank(byte bankNr, char *aReverted)
{
	byte result[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	byte lengthOfBinary = strlen(aReverted);
	char* aRevertedBinaryString = strrev(aReverted);

#ifdef DEBUG
	Serial.println("Set Bank Pins of Bank reverted string");
	Serial.print(bankNr);
	Serial.print("# RevertedString: ");
	Serial.println(aRevertedBinaryString);
#endif

	for (byte b = 0; b < lengthOfBinary; b++)
	{
#ifdef ANTSWITCHMODE
	#ifdef USEGROUNDPIN
		if (b == groundSlotNrs[bankNr]) // lockpin cannot set from outside...
			continue;
	#endif
#endif;
		result[b] = *(aRevertedBinaryString + b); // get a char out of a sting at position. remember: the value is the char value not 0 or 1... -48!                        
		byte value = result[b] - 48;

#ifdef DEBUG
		Serial.println("Set Bank Pins of Bank with value");
		Serial.print(" Stelle ");
		Serial.print(b);
		Serial.print(" Value");
		Serial.print(value);
#endif

		// call the horizontal lock logic
#ifdef DEBUG
		Serial.println("Ist der Slot gelockt?");
		Serial.print("BankNr ");
		Serial.print(bankNr);
		Serial.print("Position ");
		Serial.print(b);
		Serial.print("Ich schalte AN(1) Aus(0) ");
		Serial.println(value);
		Serial.print("LockLevel Erlaubt(0) Verboten(1): ");
		Serial.println(lockSameSlot(bankNr, b, value));
#endif // DEBUG

		// call the horizontal lock logic
#ifdef PREVENTSAMESLOTDIFFERENTBANK
		if (excludeBankFromSlotLock[bankNr] == 0 && lockSameSlot(bankNr, b, value))
			return;
#endif // preventSameSlotDifferentBank

		if (value == 1) // if its 1 => turn pin on!
		{
			if (bankNr == 0)
			{
				if (is5sPinBank0[b] && !stayOnPinsBank0[b]) // set the timestamp when status should be changed only if its a long ON -button and its not allready set :P
					longOnCleanUpTimeBank0[b] = now() + pushDurationBank0[b];
				if (isOffPinBank0[b])
					longOffCleanUpTimeBank0[b] = now() + pushDurationBank0[b];
				if (!stayOnPinsBank0[b]) // if its not set as a already-on-button, than call the arduino pin. if its set to "already on", do nothing, it might be off meanwhile...
				{
					if (statusBank0[b] != 1)
					{
						digitalWrite(pinsBank0[b], HIGH);
						statusBank0[b] = 1;
					}
				}
			}
			if (bankNr == 1)
			{
				if (is5sPinBank1[b] && !stayOnPinsBank1[b])
					longOnCleanUpTimeBank1[b] = now() + pushDurationBank1[b];
				if (isOffPinBank1[b])
					longOffCleanUpTimeBank1[b] = now() + pushDurationBank1[b];
				if (!stayOnPinsBank1[b])
				{
					if (statusBank1[b] != 1)
					{
						digitalWrite(pinsBank1[b], HIGH);
						statusBank1[b] = 1;
					}
				}
			}
			if (bankNr == 2)
			{
				if (is5sPinBank2[b] && !stayOnPinsBank2[b])
					longOnCleanUpTimeBank2[b] = now() + pushDurationBank2[b];
				if (isOffPinBank2[b])
					longOffCleanUpTimeBank2[b] = now() + pushDurationBank2[b];
				if (!stayOnPinsBank2[b])
				{
					if (statusBank2[b] != 1)
					{
						digitalWrite(pinsBank2[b], HIGH);
						statusBank2[b] = 1;
					}
				}
			}
			if (bankNr == 3)
			{
				if (is5sPinBank3[b] && !stayOnPinsBank3[b])
					longOnCleanUpTimeBank3[b] = now() + pushDurationBank3[b];
				if (isOffPinBank3[b])
					longOffCleanUpTimeBank3[b] = now() + pushDurationBank3[b];
				if (!stayOnPinsBank3[b])
				{
					if (statusBank3[b] != 1)
					{
						digitalWrite(pinsBank3[b], HIGH);
						statusBank3[b] = 1;
					}
				}
			}
		}
		else
		{
			if (bankNr == 0)
			{
				if (statusBank0[b] != 0)
				{
					digitalWrite(pinsBank0[b], LOW);
					statusBank0[b] = 0;
				}
			}
			if (bankNr == 1)
				if (statusBank1[b] != 0)
				{
					digitalWrite(pinsBank1[b], LOW);
					statusBank1[b] = 0;
				}
			if (bankNr == 2)
				if (statusBank2[b] != 0)
				{
					digitalWrite(pinsBank2[b], LOW);
					statusBank2[b] = 0;
				}
			if (bankNr == 3)
				if (statusBank3[b] != 0)
				{
					digitalWrite(pinsBank3[b], LOW);
					statusBank3[b] = 0;
				}
		}
	}


#ifdef ANTSWITCHMODE
	#ifdef USEGROUDNPIN
		setLogSlut(bankNr, isAntennaActive(bankNr));
	#endif // USEGROUDNPIN
#endif;

}

#ifdef ANTSWITCHMODE
void setPinsBySlotNumber(byte bankNr, byte slot, EthernetClient client)
{
	setPinsByStrValue(bankNr, 1 << slot, client);
}

bool isAntennaActive(byte bankNr)
{
	byte *p;
	byte tempSum = 0;

	if (bankNr == 0)
		p = &statusBank0[0];
	if (bankNr == 1)
		p = &statusBank1[0];
	if (bankNr == 2)
		p = &statusBank2[0];
	if (bankNr == 3)
		p = &statusBank3[0];

	for (byte x = 0; x < numberOfAntennasPerLayer[bankNr]; x++)
	{
		tempSum += *(p + x);
	}

	if (tempSum > 0)
		return true;
	return false;
}

#ifdef USEGROUDPIN
	void setLdddogSlut(byte bankNr, bool toActive)
	{
		if (toActive)
			digitalWrite(groundPins[bankNr], HIGH);
		else
			digitalWrite(groundPins[bankNr], LOW);
	}
#endif // USEGROUDPIN


#endif // ANTSWITCHMODE

void setPinsByStrValue(byte bankNr, int value, EthernetClient client)
{
	char *theBinaryString = convertIntegerValueInBinaryString(value); // convert the int to a binary string
	setPinsOfBank(bankNr, theBinaryString); // set the pins on arduino using the index

#ifdef WRITETOEEPROM
	changesDuringInverval++;
#endif // DEBUG

	Send200OK(client); // return the http status
}


int getPinsOfBank(byte bankNr)
{
	byte temp1[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	for (int x = 15; x >= 0; x--)
	{
		if (bankNr == 0)
		{
			if (stayOnPinsBank0[x] == 1) // if its a already-on button, check this. if its on, dont call the arduino, because the digital status is off already.. just show him as on...
				temp1[x] = 1;
			else
				temp1[x] = digitalRead(pinsBank0[x]);
		}
		if (bankNr == 1)
			if (stayOnPinsBank1[x] == 1)
				temp1[x] = 1;
			else
				temp1[x] = digitalRead(pinsBank1[x]);
		if (bankNr == 2)
			if (stayOnPinsBank2[x] == 1)
				temp1[x] = 1;
			else
				temp1[x] = digitalRead(pinsBank2[x]);
		if (bankNr == 3)
			if (stayOnPinsBank3[x] == 1)
				temp1[x] = 1;
			else
				temp1[x] = digitalRead(pinsBank3[x]);
	}
	return getEncodedSum(temp1);
}

void getAllBankValues(unsigned int *result)
{
	for (int a = 0; a < 4; a++)
	{
		result[a] = getPinsOfBank(a);
	}
}

void initBanksFromSDCard()
{
	getByteArray(strpinsBank0, pinsBank0);
	getByteArray(strpinsBank1, pinsBank1);
	getByteArray(strpinsBank2, pinsBank2);
	getByteArray(strpinsBank3, pinsBank3);

	getBoolArray(stris5sPinBank0, is5sPinBank0);
	getBoolArray(stris5sPinBank1, is5sPinBank1);
	getBoolArray(stris5sPinBank2, is5sPinBank2);
	getBoolArray(stris5sPinBank3, is5sPinBank3);
	getBoolArray(strisOffPinBank0, isOffPinBank0);
	getBoolArray(strisOffPinBank1, isOffPinBank1);
	getBoolArray(strisOffPinBank2, isOffPinBank2);
	getBoolArray(strisOffPinBank3, isOffPinBank3);

	getByteArray(strindexOfOffPinPairBank0, indexOfOffPinPairBank0);
	getByteArray(strindexOfOffPinPairBank1, indexOfOffPinPairBank1);
	getByteArray(strindexOfOffPinPairBank2, indexOfOffPinPairBank2);
	getByteArray(strindexOfOffPinPairBank3, indexOfOffPinPairBank3);
	getByteArray(strpushDurationBank0, pushDurationBank0);
	getByteArray(strpushDurationBank1, pushDurationBank1);
	getByteArray(strpushDurationBank2, pushDurationBank2);
	getByteArray(strpushDurationBank3, pushDurationBank3);


	getByteArray(strpushDurationBank0, pushDurationBank0);
	getByteArray(strpushDurationBank1, pushDurationBank1);
	getByteArray(strpushDurationBank2, pushDurationBank2);
	getByteArray(strpushDurationBank3, pushDurationBank3);
	getBoolArray(strexcludeBankFromSlotLock, excludeBankFromSlotLock);

#ifdef ANTSWITCHMODE // webswitch antennas only
	getByteArray(strbandAntennaMapping0, bandAntennaMapping0);
	getByteArray(strbandAntennaMapping1, bandAntennaMapping1);
	getByteArray(strbandAntennaMapping2, bandAntennaMapping2);
	getByteArray(strbandAntennaMapping3, bandAntennaMapping3);
#endif // ANTSWITCHMODE
}

#ifdef PREVENTSAMESLOTDIFFERENTBANK
boolean lockSameSlot(byte changedBankNr, byte positionB, byte newValue)
{
	if (newValue == 1)
	{
		if (changedBankNr == 0)
		{
			if ((excludeBankFromSlotLock[1] == 0 && digitalRead(pinsBank1[positionB])) == 1 ||
				(excludeBankFromSlotLock[2] == 0 && digitalRead(pinsBank2[positionB])) == 1 ||
				(excludeBankFromSlotLock[3] == 0 && digitalRead(pinsBank3[positionB])) == 1)
				return true;
		}
		else if (changedBankNr == 1)
		{
			if ((excludeBankFromSlotLock[0] == 0 && digitalRead(pinsBank0[positionB])) == 1 ||
				(excludeBankFromSlotLock[2] == 0 && digitalRead(pinsBank2[positionB])) == 1 ||
				(excludeBankFromSlotLock[3] == 0 && digitalRead(pinsBank3[positionB])) == 1)
				return true;
		}
		else if (changedBankNr == 2)
		{
			if ((excludeBankFromSlotLock[1] == 0 && digitalRead(pinsBank1[positionB])) == 1 ||
				(excludeBankFromSlotLock[0] == 0 && digitalRead(pinsBank0[positionB])) == 1 ||
				(excludeBankFromSlotLock[3] == 0 && digitalRead(pinsBank3[positionB])) == 1)
			return true;
		}
		else
		{
			if ((excludeBankFromSlotLock[1] == 0 && digitalRead(pinsBank1[positionB])) == 1 ||
				(excludeBankFromSlotLock[2] == 0 && digitalRead(pinsBank2[positionB])) == 1 ||
				(excludeBankFromSlotLock[0] == 0 && digitalRead(pinsBank0[positionB])) == 1)
			return true;
		}
	}
	else
		return false;
	return false;
}
#endif // preventSameSlotDifferentBank

/*********************************************************************************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++ THE WEB PAGES ++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*********************************************************************************************************************************/

void sendEmptyPage(EthernetClient client)
{
	Send200OK(client);
}


void GetSingleBankData(EthernetClient client, byte bankNr, long myToken)
{
  if (!validateToken(myToken, client))
	return;
  unsigned int mu = getPinsOfBank(bankNr);
  Send200OK(client);
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println();
  client.print(F("myCB({'v':"));
  client.print(mu);
  client.print(F("})"));
}

void GetAllBanksData(EthernetClient client, long myToken)
{
  if (!validateToken(myToken, client))
	return;
  unsigned int result[4];
  getAllBankValues(result);
  Send200OK(client);
  client.println(F("Access-Control-Allow-Origin: *"));
  client.println(F(""));
  client.print(F("xx({\"v\": \""));
  for (byte e = 0; e < 4; e++)
  {
	  client.print(result[e]);
	  if (e < 3)
		  client.print(F("|"));
  }
  client.print(F("|"));
  client.print(freeRam());
  client.print(F("\"})"));
}


void Send200OK(EthernetClient client)
{
  client.println(F("HTTP/1.1 200 OK")); //send new page
  client.println(F("Content-Type: text/html"));
}

void MainPage(EthernetClient client, boolean isLocal, long myToken)
{
	if (!validateToken(myToken, client))
		return;

   Send200OK(client);
   client.println(F(""));
   client.println(F("<!DOCTYPE html>"));
   client.println(F("<HTML>"));
   client.println(F("<HEAD>"));
   client.println(F("<meta http-equiv=\"Cache-control\" content=\"no-cache\"><meta http-equiv=\"Expires\" content=\"0\">"));
   client.print(F("<script type=\"text/javascript\" src=\""));
   client.print(jqueryUrl);
   client.println(F("\"></script>"));
   client.print(F("<script>var configAddress='"));
   if (isLocal)
   {
	   client.print(F("http://"));
	   client.print(deviceIp);
   }
   else
      client.print(ajaxUrl);
   client.println(F("';</script>"));
   client.print(F("<script src=\""));
   client.print(jsUrl);
   client.println(F("\"></script>"));
   client.print(F("<script src=\""));
   client.print(contentUrl);
   client.println(F("\"></script>"));
   client.print(F("<link rel=\"Stylesheet\" href=\""));
   client.print(cssUrl);
   client.println(F("\" type=\"text/css\">"));
   client.print(F("<link rel=\"shortcut icon\" href=\""));
   client.print(faviconUrl);
   client.println(F("\">"));
   client.print(F("<TITLE>"));
   client.print(title);
   client.println(F("</TITLE>"));
   client.println(F("</HEAD>"));
   client.println(F("<BODY>"));
   client.println(F("<div id=\"content\"></div>"));
   client.println(F("<script>addContent();</script>"));
   client.println(F("</BODY>"));
   client.println(F("</HTML>"));
}


/*********************************************************************************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Token ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*********************************************************************************************************************************/

boolean validateToken(long myToken, EthernetClient client)
{
	long temp = (myToken/salt) >> 3;
#ifdef DEBUG
	Serial.print("trying with :");  
	Serial.println(myToken);  
	Serial.print("Got: ");  
	Serial.println(temp);
	if (token == temp)
		Serial.println("access erteillt!!!");
	else
		Serial.println("kein access!!!");
#endif
	if (token == temp)
		return true;

	sendEmptyPage(client);
	return false;
}

void chrTokenConverter()
{
	long temp;
	for (int i = 0; i<sizeof(charToken)-1; i++)
	{
		token += charToken[i];
	}
	Serial.println(F("Your token for javascipt is: "));
	Serial.println((token*salt) << 3);
}


/*********************************************************************************************************************************/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++ Eepromhandling ++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*********************************************************************************************************************************/

#ifdef WRITETOEEPROM
void writeToMemory()
{
	if (eepromWatchdogLastRun <= now() - eepromWatchdogTimer)
	{
		char myA[24];

		unsigned int result[4];
		getAllBankValues(result);

		byte counter = 0;

		for (byte e = 0; e < 4; e++)
		{
			myA[counter] = result[e];
			if (e < 3)
				myA[counter] = '|';

		}
		ee.eeprom_write_string(eepromMemoryStartAddress, myA);

#ifdef DEBUG
		Serial.println("Run the eeprom writer");
#endif // DEBUG

		eepromWatchdogLastRun = now();
		changesDuringInverval = 0;
	}
}

void readFromMemory()
{
	ee.eeprom_read_string(eepromMemoryStartAddress, lastState, STRINGSIZE);
}

void initLaststate()
{
	readFromMemory();
	
	char lastValueBank0[6] = { '\0','\0', '\0', '\0', '\0', '\0' };
	char lastValueBank1[6] = { '\0','\0', '\0', '\0', '\0', '\0' };
	char lastValueBank2[6] = { '\0','\0', '\0', '\0', '\0', '\0' };
	char lastValueBank3[6] = { '\0','\0', '\0', '\0', '\0', '\0' };

	getValuesByUrl(lastState, '|', 1, lastValueBank0);
	getValuesByUrl(lastState, '|', 2, lastValueBank1);
	getValuesByUrl(lastState, '|', 3, lastValueBank2);
	getValuesByUrl(lastState, '|', 4, lastValueBank3);


	setPinsByEeprom(lastValueBank0, 0);
	setPinsByEeprom(lastValueBank1, 1);
	setPinsByEeprom(lastValueBank2, 2);
	setPinsByEeprom(lastValueBank3, 3);
}

void setPinsByEeprom(char *intValueBinaryString, byte bankNr)
{
	unsigned int temp = atol(intValueBinaryString);
	char *theBinaryString = convertIntegerValueInBinaryString(temp); // convert the int to a binary string

	setPinsOfBank(bankNr, theBinaryString);
}
#endif

int freeRam() {
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

