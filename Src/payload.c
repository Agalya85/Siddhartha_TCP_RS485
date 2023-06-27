/*******************************************************************************
* Title                 :   System Payload Generation
* Filename              :   payload.c
* Author                :   Hrishikesh Limaye
* Origin Date           :   08/03/2022
* Version               :   1.0.0
* Compiler              :
* Target                :   STM32F437 - Tor4Eth
* Notes                 :   None
*
* Copyright (c) by KloudQ Technologies Limited.

  This software is copyrighted by and is the sole property of KloudQ
  Technologies Limited.
  All rights, title, ownership, or other interests in the software remain the
  property of  KloudQ Technologies Limited. This software may only be used in
  accordance with the corresponding license agreement. Any unauthorized use,
  duplication, transmission, distribution, or disclosure of this software is
  expressly forbidden.

  This Copyright notice may not be removed or modified without prior written
  consent of KloudQ Technologies Limited.

  KloudQ Technologies Limited reserves the right to modify this software
  without notice.
*
*
*******************************************************************************/
/*************** FILE REVISION LOG *****************************************
*
*    Date    Version   Author         	  Description
*  08/03/22   1.0.0    Hrishikesh Limaye   Initial Release.
*
*******************************************************************************/

/** @file  payload.c
 *  @brief Payload utility Functions
 */

/******************************************************************************
* Includes
*******************************************************************************/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "externs.h"
#include "payload.h"
#include "applicationdefines.h"
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include "gsmSim7600.h"

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
/**
 * Doxygen tag for documenting variables and constants
 */

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/

/******************************************************************************
* Module Typedefs
*******************************************************************************/

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
const char * gpu8NoString = ": 0,0,0,0,0";
const char * gpu8Norssi = "0,0";
const char * gpu8NoModbusData = "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";
const char * gpu8NoGPSData = "0,0,0,0,0,0,0,0,0";

char agpsLastLocationData[150] = "0,0,0,0,0,0,0,0,0";
char agpsLastLocationDataFormatted[150] = "0,0,0,0,0,0,0,0,0";
char gu32GPSLat[15] = "0000.000000";
char gu32GPSLon[15] = "0000.000000";

char gu32GPSLatformatted[15] = "0";
char gu32GPSLonformatted[15] = "0";

char gcSystemSupplyVoltage[5];
char gcBatterySupplyVoltage[5];
char gacMBDataPoints[5];
char gacMBDataPointsRTU[5];
char gacEthernetLinkStatus[2];
char gacModbusStatus[2]={0};
char gacModbusStatusRTU[2]={0};

extern float gfBatteryVoltage;
extern strctGSM gsmInstance;

float templat = 0.0f;
float templon = 0.0f;

uint32_t gu32ExtractGPSParamCounter = 0;

uint32_t u32TempVar;

uint32_t gu32TempLoopCounter = 0;
/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/
/******************************************************************************
* Function : getSystemDataString ()
*//**
* \b Description:
*
* This function is used to Fetch Devicepayload string
*
* PRE-CONDITION: Define STM32_UUID , STM32_FLASHSIZE and their MCU relevant
* 				 Address in deviceinfo.h
*
* POST-CONDITION: None
*
*
* @return 		None.
*
* \b Example Example:
* @code
*
* 	getSystemDataString();
*
* @endcode
*
* @see
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 08/03/2022 </td><td> 0.0.1            </td><td> HL100133 </td><td> Interface Created </td></tr>
*
* </table><br><br>
* <hr>
*
*******************************************************************************/
char * getSystemDataString(void)
{
	char * systemPayload = malloc(sizeof(char) * GSM_PAYLOAD_MAX_SIZE);
	char temp_array[50];
	//uint32_t u32TempVar;

	getrtcStamp();
	updateInputVoltage();
	itoa(gu32EthLinkAlert,gacEthernetLinkStatus,10);
	itoa(gu32MBDataByteCounterLimit,gacMBDataPoints,10);
	itoa(gu32MBDataByteCounterLimitRTU,gacMBDataPointsRTU,10);
	itoa(gu32MBClientConnectedFlag,gacModbusStatus,10);
	itoa(gu32MBRTUClientConnectedFlag,gacModbusStatusRTU,10);// gu8MBResponseFlag
	/* GPS Data Formatting */
	if(strlen(gsmInstance.agpsLocationData) > 10)
	{
		/*GPS available */
		memcpy(agpsLastLocationData,gsmInstance.agpsLocationData,strlen(gsmInstance.agpsLocationData));
		/* Formulate Lat and Lon in standard format
		 * 1830.101 = 18 + (30.101094/60) = 18.5016849
		 * 7348.878 = 73 + (48.877697 /60) = 73.8146 */
		formatGPSSring();
	}
	if(systemPayload != NULL)
	{
		memset(systemPayload,0x00,sizeof(char) * GSM_PAYLOAD_MAX_SIZE);

		if(gu32OperateModbusTCP == 1)
		{
			/* Start of Frame */
			strcpy((char *)systemPayload,(char * )START_OF_FRAME);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* Device UUID */
			strcat((char *)systemPayload,(char * )dinfo);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* Model Number */
			strcat((char *)systemPayload,(char * )MODEL_NUMBER);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* Local Time Stamp */
			strcat((char *)systemPayload,(char * )aShowDateStamp);
			strcat((char *)systemPayload,(char * )" ");
			strcat((char *)systemPayload,(char * )aShowTimeStamp);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* Firmware Version*/
			strcat((char *)systemPayload,(char * )FIRMWARE_VER);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* Ethernet Status*/
			strcat((char *)systemPayload,(char * )gacEthernetLinkStatus);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);
			/* Total Data points - TCP */
			strcat((char *)systemPayload,(char * )gacMBDataPoints);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* Input Supply Voltage */
			if((float)gfInputSupplyVoltage < 9.00f)
			{
				/* Device Switched to battery*/
				strcat((char *)systemPayload,(char * )gcBatterySupplyVoltage);
				strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);
			}
			else
			{
				strcat((char *)systemPayload,(char * )gcSystemSupplyVoltage);
				strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);
			}

			/* Modbus Status */
			strcat((char *)systemPayload,(char * )gacModbusStatus);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* Modbus Data IP */
			strcat((char *)systemPayload,"IP,");
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/*Location Data . Retains previous known location */
			strcat((char *)systemPayload,(char * )agpsLastLocationDataFormatted);

			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* RSSI */
			if(strlen(gsmInstance.agsmSignalStrength ) > 0)
				strcat((char *)systemPayload,(char * )gsmInstance.agsmSignalStrength);
			else
				strcat((char *)systemPayload,gpu8Norssi);

			u32TempVar = gu32GSMConfigCheckTimer / 1000;
			memset(temp_array, 0, sizeof(temp_array));
			itoa(u32TempVar, temp_array, 10);
			strcat((char *)systemPayload,(char * )",ConfigCheckTime:  ");
			strcat((char *)systemPayload,(char * )temp_array);

			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			strcat((char *)systemPayload,gau8TempMBPayloadString);

			strcat((char *)systemPayload,(char * )"0,0");	// Added for buffer identification
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);
			strcat((char *)systemPayload,(char * )END_OF_FRAME);

			gu32TempLoopCounter = 0;
			//memset(gau8TempMBPayloadString,0x00,sizeof(char) * gu32MBDataByteCounterLimit);
		}

/**************************** Second String RTU *****************************************************/
		if(gu32OperateModbus485 == 1)
		{
			strcat((char *)systemPayload,(char * )START_OF_FRAME);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* Device UUID */
			strcat((char *)systemPayload,(char * )dinfo);
			strcat((char *)systemPayload,(char * )"_RTU");
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* Model Number */
			strcat((char *)systemPayload,(char * )MODEL_NUMBER);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* Local Time Stamp */
			strcat((char *)systemPayload,(char * )gau8Date);
			strcat((char *)systemPayload,(char * )RTC_DATESEPARATOR);
			strcat((char *)systemPayload,(char * )gau8Month);
			strcat((char *)systemPayload,(char * )RTC_DATESEPARATOR);
			strcat((char *)systemPayload,(char * )gau8Year);
			strcat((char *)systemPayload,(char * )RTC_PARAM_SEPARATOR);
			strcat((char *)systemPayload,(char * )gau8Hour);
			strcat((char *)systemPayload,(char * )RTC_TIMESEPARATOR);
			strcat((char *)systemPayload,(char * )gau8Minutes);
			strcat((char *)systemPayload,(char * )RTC_TIMESEPARATOR);
			strcat((char *)systemPayload,(char * )gau8Seconds);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* Firmware Version*/
			strcat((char *)systemPayload,(char * )FIRMWARE_VER);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* Ethernet Status : Just to match the payload */
			strcat((char *)systemPayload,(char * )gacEthernetLinkStatus);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* Total Data points - RTU */
			strcat((char *)systemPayload,(char * )gacMBDataPointsRTU);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* Input Supply Voltage */
			if((float)gfInputSupplyVoltage < 9.00f)
			{
				/* Device Switched to battery*/
				strcat((char *)systemPayload,(char * )gcBatterySupplyVoltage);
				strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);
			}
			else
			{
				strcat((char *)systemPayload,(char * )gcSystemSupplyVoltage);
				strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);
			}
			/* Modbus Status */
			strcat((char *)systemPayload,(char * )gacModbusStatusRTU);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* Modbus Data - RTU */
			strcat((char *)systemPayload,"RT,");
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/*Location Data . Retains previous known location */
			strcat((char *)systemPayload,(char * )agpsLastLocationDataFormatted);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			/* RSSI */
			if(strlen(gsmInstance.agsmSignalStrength ) > 0)
				strcat((char *)systemPayload,(char * )gsmInstance.agsmSignalStrength);
			else
				strcat((char *)systemPayload,gpu8Norssi);

			u32TempVar = gu32GSMConfigCheckTimer / 1000;
			memset(temp_array, 0, sizeof(temp_array));
			itoa(u32TempVar, temp_array, 10);
			strcat((char *)systemPayload,(char * )",ConfigCheckTime ");
			strcat((char *)systemPayload,(char * )temp_array);
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);

			strcat((char *)systemPayload,gau8TempMBRTUPayloadString);

			strcat((char *)systemPayload,(char * )"0,0");	// Added for buffer identification
			strcat((char *)systemPayload,(char * )PAYLOAD_SEPARATOR);
			strcat((char *)systemPayload,(char * )END_OF_FRAME);
			gu32TempLoopCounter = 0;
		}
		return systemPayload;

	}
	else
	{
		/* Error in memory allocation */
		return NULL;
	}
}
/******************************************************************************
* Function : getSystemConfig ()
*//**
* \b Description:
*
* This function is used to get Device configuration string.
*
* PRE-CONDITION:
*
* POST-CONDITION:
*
*
* @return 		None.
*
* \b Example Example:
* @code
*
* 	getSystemConfig();
*
* @endcode
*
* @see
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 08/03/2022 </td><td> 0.0.1            </td><td> HL100133 </td><td> Interface Created </td></tr>
*
* </table><br><br>
* <hr>
*
*******************************************************************************/
char * getSystemConfig(void)
{
	char * systemConfig = malloc(sizeof(char) * GSM_PAYLOAD_MAX_SIZE);
	getrtcStamp();
	if(systemConfig != NULL)
	{
		memset(systemConfig,0x00,sizeof(char) * GSM_PAYLOAD_MAX_SIZE);
		char temp_array[50];

		/* Start of Frame */
		strcpy((char *)systemConfig,(char * )START_OF_FRAME);
		strcat((char *)systemConfig,(char * )PAYLOAD_SEPARATOR);

		/* Device UUID */
		strcat((char *)systemConfig,(char * )dinfo);
		strcat((char *)systemConfig,(char * )"_Conf");
		strcat((char *)systemConfig,(char * )PAYLOAD_SEPARATOR);

		/* Model Number */
		strcat((char *)systemConfig,(char * )MODEL_NUMBER);
		strcat((char *)systemConfig,(char * )PAYLOAD_SEPARATOR);

		/* Local Time Stamp */
		strcat((char *)systemConfig,(char * )gau8Date);
		strcat((char *)systemConfig,(char * )RTC_DATESEPARATOR);
		strcat((char *)systemConfig,(char * )gau8Month);
		strcat((char *)systemConfig,(char * )RTC_DATESEPARATOR);
		strcat((char *)systemConfig,(char * )gau8Year);
		strcat((char *)systemConfig,(char * )RTC_PARAM_SEPARATOR);
		strcat((char *)systemConfig,(char * )gau8Hour);
		strcat((char *)systemConfig,(char * )RTC_TIMESEPARATOR);
		strcat((char *)systemConfig,(char * )gau8Minutes);
		strcat((char *)systemConfig,(char * )RTC_TIMESEPARATOR);
		strcat((char *)systemConfig,(char * )gau8Seconds);
		strcat((char *)systemConfig,(char * )PAYLOAD_SEPARATOR);

		/* Firmware Version*/
		strcat((char *)systemConfig,(char * )FIRMWARE_VER);
		strcat((char *)systemConfig,(char * )PAYLOAD_SEPARATOR);

		strcat((char *)systemConfig,(char * )"SystemConfig :");
		strcat((char *)systemConfig,(char * )PAYLOAD_SEPARATOR);
		strcat((char *)systemConfig,(char * )gau8LastKnownConfiguration);
		strcat((char *)systemConfig,(char * )PAYLOAD_SEPARATOR);

		/* Config Error */
		memset(temp_array, 0, sizeof(temp_array));
		itoa(gu32ConfigNetworkErrorDatabase, temp_array, 10);
		strcat((char *)systemConfig,(char * )"NetworkErr ");
		strcat((char *)systemConfig,(char * )temp_array);
		strcat((char *)systemConfig,(char * )PAYLOAD_SEPARATOR);

		memset(temp_array, 0, sizeof(temp_array));
		itoa(gu32ConfigModbus485ErrorDatabase, temp_array, 10);
		strcat((char *)systemConfig,(char * )"485Err ");
		strcat((char *)systemConfig,(char * )temp_array);
		strcat((char *)systemConfig,(char * )PAYLOAD_SEPARATOR);

		memset(temp_array, 0, sizeof(temp_array));
		itoa(gu32ConfigModbusTCPErrorDatabase, temp_array, 10);
		strcat((char *)systemConfig,(char * )"MBTCPErr ");
		strcat((char *)systemConfig,(char * )temp_array);
		strcat((char *)systemConfig,(char * )PAYLOAD_SEPARATOR);

		strcat((char *)systemConfig,(char * )"0,0");	// Added for buffer identification
		strcat((char *)systemConfig,(char * )PAYLOAD_SEPARATOR);
		strcat((char *)systemConfig,(char * )END_OF_FRAME);

		return systemConfig;
	}
	else
		return NULL;
}
/******************************************************************************
* Function : updateInputVoltage ()
*//**
* \b Description:
*
* This function is used to update adc readings (Vin and Vbatt) into payload
*
* PRE-CONDITION: None
*
* POST-CONDITION: None.
*
*
* @return 		None.
*
* \b Example Example:
* @code
*
* 	updateInputVoltage();
*
* @endcode
*
* @see
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 08/03/2022 </td><td> 0.0.1            </td><td> HL100133 </td><td> Interface Created </td></tr>
*
* </table><br><br>
* <hr>
*
*******************************************************************************/
void updateInputVoltage(void)
{
	sprintf(gcSystemSupplyVoltage,"%.2f",gfInputSupplyVoltage);
	sprintf(gcBatterySupplyVoltage,"%.2f",gfBatteryVoltage);
}

/******************************************************************************
* Function : formatGPSSring ()
*//**
* \b Description:
*
* This function is used to format GPS string into Lat / Lon .
* 		Formulate Lat and Lon in standard format
		 1830.101 = 18 + (30.101094/60) = 18.5016849
		 7348.878 = 73 + (48.877697 /60) = 73.8146
*
* PRE-CONDITION: None
*
* POST-CONDITION: None.
*
*
* @return 		None.
*
* \b Example Example:
* @code
*
* 	formatGPSSring();
*
* @endcode
*
* @see
*
* <br><b> - HISTORY OF CHANGES - </b>
*
* <table align="left" style="width:800px">
* <tr><td> Date       </td><td> Software Version </td><td> Initials </td><td> Description </td></tr>
* <tr><td> 08/03/2022 </td><td> 0.0.1            </td><td> HL100133 </td><td> Interface Created </td></tr>
*
* </table><br><br>
* <hr>
*
*******************************************************************************/
void formatGPSSring(void)
{
	/*Formulate Lat and Lon in standard format
		 * 1830.101 = 18 + (30.101094/60) = 18.5016849
		 * 7348.878 = 73 + (48.877697 /60) = 73.8146 */

	char *configptr = strtok(agpsLastLocationData, ",");
	gu32ExtractGPSParamCounter = 0;

	while(gu32ExtractGPSParamCounter != 9) //reference : while(configptr != NULL)
	{
		switch(gu32ExtractGPSParamCounter)
		{
			case 0:
				/* Lat  */
				strcpy(gu32GPSLat,configptr);
				templat = atof(&gu32GPSLat[2]);
				templat /= 60;
				templat += (((gu32GPSLat[0]-'0') * 10) + (gu32GPSLat[1]-'0'));
				sprintf(gu32GPSLatformatted,"%.7f",templat);
				memset(agpsLastLocationDataFormatted,0x00,150);
				strcpy(agpsLastLocationDataFormatted,gu32GPSLatformatted);
				break;
			case 1:
				/* N/S*/
				strcat(agpsLastLocationDataFormatted,",");
				strcat(agpsLastLocationDataFormatted,configptr);
				strcat(agpsLastLocationDataFormatted,",");
				break;
			case 2:
				/* Lon */
				strcpy(gu32GPSLon,configptr);
				templon = atof(&gu32GPSLon[3]);
				templon /= 60;
				templon += (((gu32GPSLon[0]- '0') * 100 )+ ((gu32GPSLon[1]-'0') * 10) +(gu32GPSLon[2]-'0'));
				sprintf(gu32GPSLonformatted,"%.7f",templon);
				strcat(agpsLastLocationDataFormatted,gu32GPSLonformatted);

				break;
			case 3:
				/* E/W */
				strcat(agpsLastLocationDataFormatted,",");
				strcat(agpsLastLocationDataFormatted,configptr);

				break;
			case 4:
				/* data 5 */
				strcat(agpsLastLocationDataFormatted,",");
				strcat(agpsLastLocationDataFormatted,configptr);

				break;
			case 5:
				/* data 6 */
				strcat(agpsLastLocationDataFormatted,",");
				strcat(agpsLastLocationDataFormatted,configptr);

				break;
			case 6:
				/* data 7 */
				strcat(agpsLastLocationDataFormatted,",");
				strcat(agpsLastLocationDataFormatted,configptr);

				break;
			case 7:
				/* data 8 */
				strcat(agpsLastLocationDataFormatted,",");
				strcat(agpsLastLocationDataFormatted,configptr);

				break;
			case 8:
				/* data 9 . End of GPS Data string */
				strcat(agpsLastLocationDataFormatted,",");
				strcat(agpsLastLocationDataFormatted,configptr);
				break;

			default:

				break;
		}
		if(gu32ExtractGPSParamCounter != 9)
			configptr = strtok(NULL, ",");

		gu32ExtractGPSParamCounter++;

	}
}

//******************************* End of File *******************************************************************

