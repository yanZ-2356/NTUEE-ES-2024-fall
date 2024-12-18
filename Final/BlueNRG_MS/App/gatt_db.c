/**
  ******************************************************************************
  * @file    App/gatt_db.c
  * @author  SRA Application Team
  * @brief   Functions to build GATT DB and handle GATT events
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bluenrg_def.h"
#include "gatt_db.h"
#include "bluenrg_conf.h"
#include "bluenrg_gatt_aci.h"

/** @brief Macro that stores Value into a buffer in Little Endian Format (2 bytes)*/
#define HOST_TO_LE_16(buf, val)    ( ((buf)[0] =  (uint8_t) (val)    ) , \
                                   ((buf)[1] =  (uint8_t) (val>>8) ) )

/** @brief Macro that stores Value into a buffer in Little Endian Format (4 bytes) */
#define HOST_TO_LE_32(buf, val)    ( ((buf)[0] =  (uint8_t) (val)     ) , \
                                   ((buf)[1] =  (uint8_t) (val>>8)  ) , \
                                   ((buf)[2] =  (uint8_t) (val>>16) ) , \
                                   ((buf)[3] =  (uint8_t) (val>>24) ) )

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
        uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
            uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
                uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
}while(0)

/* Hardware Characteristics Service */
#define COPY_HW_SENS_W2ST_SERVICE_UUID(uuid_struct)    COPY_UUID_128(uuid_struct,0x00,0x00,0x00,0x00,0x00,0x01,0x11,0xe1,0x9a,0xb4,0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_MIN_LIGHT_W2ST_CHAR_UUID(uuid_struct) 	   COPY_UUID_128(uuid_struct,0x00,0x00,0x00,0x00,0x00,0x01,0x11,0xe1,0xac,0x36,0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_TARGET_LIGHT_W2ST_CHAR_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x01,0x00,0x00,0x00,0x00,0x01,0x11,0xe1,0xac,0x36,0x00,0x02,0xa5,0xd5,0xc5,0x1b)
/* Software Characteristics Service */
#define COPY_SW_SENS_W2ST_SERVICE_UUID(uuid_struct)    COPY_UUID_128(uuid_struct,0x00,0x00,0x00,0x00,0x00,0x02,0x11,0xe1,0x9a,0xb4,0x00,0x02,0xa5,0xd5,0xc5,0x1b)

uint16_t HWServW2STHandle, MinLightHandle, TargetLightHandle;
uint16_t SWServW2STHandle;

/* UUIDS */
Service_UUID_t service_uuid;
Char_UUID_t char_uuid;

extern uint16_t connection_handle;
extern uint32_t start_time;
extern uint16_t MIN_LIGHT;
extern uint16_t TARGET_LIGHT;

/**
 * @brief  Add the 'HW' service (and the Environmental and AccGyr characteristics).
 * @param  None
 * @retval tBleStatus Status
 */
tBleStatus Add_HWServW2ST_Service(void)
{
  tBleStatus ret;
  uint8_t uuid[16];
  uint8_t buffer[2]; // buffer to write the initial values

  /* Add_HWServW2ST_Service */
  COPY_HW_SENS_W2ST_SERVICE_UUID(uuid);
  BLUENRG_memcpy(&service_uuid.Service_UUID_128, uuid, 16);
  ret = aci_gatt_add_serv(UUID_TYPE_128, service_uuid.Service_UUID_128, PRIMARY_SERVICE,
                          1+3*5, &HWServW2STHandle);
  if (ret != BLE_STATUS_SUCCESS)
    return BLE_STATUS_ERROR;

  /* Fill the MinLight BLE Characteristc */
  COPY_MIN_LIGHT_W2ST_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(HWServW2STHandle, UUID_TYPE_128, char_uuid.Char_UUID_128,
                           2,
                           CHAR_PROP_READ|CHAR_PROP_WRITE,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP|GATT_NOTIFY_WRITE_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 0, &MinLightHandle);
  if (ret != BLE_STATUS_SUCCESS)
    return BLE_STATUS_ERROR;

  // write the initial min value to the min light characteristic
  buffer[0] = MIN_LIGHT / 256;
  buffer[1] = MIN_LIGHT % 256;
  ret = aci_gatt_update_char_value(HWServW2STHandle, MinLightHandle, 0, 2, buffer);
  if (ret != BLE_STATUS_SUCCESS)
    return BLE_STATUS_ERROR;

  /* Fill the TargetLight BLE Characteristc */
  COPY_TARGET_LIGHT_W2ST_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(HWServW2STHandle, UUID_TYPE_128, char_uuid.Char_UUID_128,
                           2,
                           CHAR_PROP_READ|CHAR_PROP_WRITE,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP|GATT_NOTIFY_WRITE_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 0, &TargetLightHandle);
  if (ret != BLE_STATUS_SUCCESS)
    return BLE_STATUS_ERROR;

  // write the initial value
  buffer[0] = TARGET_LIGHT / 256;
  buffer[1] = TARGET_LIGHT % 256;
  ret = aci_gatt_update_char_value(HWServW2STHandle, TargetLightHandle, 0, 2, buffer);
  if (ret != BLE_STATUS_SUCCESS)
    return BLE_STATUS_ERROR;

  return BLE_STATUS_SUCCESS;
}

/**
 * @brief  Add the SW Feature service using a vendor specific profile
 * @param  None
 * @retval tBleStatus Status
 */
//tBleStatus Add_SWServW2ST_Service(void)
//{
//  tBleStatus ret;
//  int32_t NumberOfRecords=1;
//  uint8_t uuid[16];
//
//  COPY_SW_SENS_W2ST_SERVICE_UUID(uuid);
//  BLUENRG_memcpy(&service_uuid.Service_UUID_128, uuid, 16);
//  ret = aci_gatt_add_serv(UUID_TYPE_128, service_uuid.Service_UUID_128, PRIMARY_SERVICE,
//                          1+3*NumberOfRecords, &SWServW2STHandle);
//
//  if (ret != BLE_STATUS_SUCCESS) {
//    goto fail;
//  }
//  return BLE_STATUS_SUCCESS;
//
//fail:
//  return BLE_STATUS_ERROR;
//}

void Read_Request_CB(uint16_t handle)
{
  tBleStatus ret;

  if(handle == MinLightHandle + 1)
  {

  }
  else if (handle == TargetLightHandle + 1)
  {

  }

  if(connection_handle !=0)
  {
    ret = aci_gatt_allow_read(connection_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
      PRINTF("aci_gatt_allow_read() failed: 0x%02x\r\n", ret);
    }
  }
}

void Write_Request_CB(uint16_t handle, uint8_t data_length, uint8_t *data)
{
	uint16_t light = 256*data[0] + data[1];
	if(handle == MinLightHandle + 1)
	{
		PRINTF("Minimum light level changed to %d.\r\n", light);
		MIN_LIGHT = light;
		if(MIN_LIGHT > 7999){
			MIN_LIGHT = 7999;
		}
	}
	else if (handle == TargetLightHandle + 1)
	{
		PRINTF("Target light level changed to %d. \r\n", light);
		TARGET_LIGHT = light;
		if(TARGET_LIGHT > 4095){
			TARGET_LIGHT = 4095;
		}
	}
	aci_gatt_write_response(connection_handle, handle, 0x00, 0, data_length, data);
}
