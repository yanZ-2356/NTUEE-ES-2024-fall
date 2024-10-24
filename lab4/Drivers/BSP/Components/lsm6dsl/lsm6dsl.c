/**
 ******************************************************************************
 * @file    lsm6dsl.c
 * @author  MCD Application Team
 * @brief   This file provides a set of functions needed to manage the LSM6DSL
 *          accelero and gyro devices
 ******************************************************************************
 * @attention
 *
 * <<h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "lsm6dsl.h"

//typedef enum
//{
//  HAL_OK       = 0x00,
//  HAL_ERROR    = 0x01,
//  HAL_BUSY     = 0x02,
//  HAL_TIMEOUT  = 0x03
//} HAL_StatusTypeDef;

I2C_HandleTypeDef hI2cHandler;


/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @defgroup LSM6DSL LSM6DSL
  * @{
  */

/** @defgroup LSM6DSL_Private_Variables LSM6DSL Private Variables
  * @{
  */ 
ACCELERO_DrvTypeDef Lsm6dslAccDrv =
{
  LSM6DSL_AccInit,
  LSM6DSL_AccDeInit,
  LSM6DSL_AccReadID,
  0,
  LSM6DSL_AccLowPower,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  LSM6DSL_AccReadXYZ
};

GYRO_DrvTypeDef Lsm6dslGyroDrv =
{
  LSM6DSL_GyroInit,
  LSM6DSL_GyroDeInit,
  LSM6DSL_GyroReadID,
  0,
  LSM6DSL_GyroLowPower,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  LSM6DSL_GyroReadXYZAngRate
};
/**
  * @}
  */ 

/** @defgroup LSM6DSL_ACC_Private_Functions LSM6DSL ACC Private Functions
  * @{
  */
/**
  * @brief  Set LSM6DSL Accelerometer Initialization.
  * @param  InitStruct: Init parameters
  */
void LSM6DSL_AccInit(uint16_t InitStruct)
{  
  uint8_t ctrl = 0x00;
  uint8_t tmp;

  /* Read CTRL1_XL */
  tmp = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL1_XL);

  /* Write value to ACC MEMS CTRL1_XL register: FS and Data Rate */
  ctrl = (uint8_t) InitStruct;
  tmp &= ~(0xFC);
  tmp |= ctrl;
  SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL1_XL, tmp);

  /* Read CTRL3_C */
  tmp = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL3_C);

  /* Write value to ACC MEMS CTRL3_C register: BDU and Auto-increment */
  ctrl = ((uint8_t) (InitStruct >> 8));
  tmp &= ~(0x44);
  tmp |= ctrl; 
  SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL3_C, tmp);
}

/**
  * @brief  LSM6DSL Accelerometer De-initialization.
  */
void LSM6DSL_AccDeInit(void)
{
  uint8_t ctrl = 0x00;
  
  /* Read control register 1 value */
  ctrl = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL1_XL);

  /* Clear ODR bits */
  ctrl &= ~(LSM6DSL_ODR_BITPOSITION);

  /* Set Power down */
  ctrl |= LSM6DSL_ODR_POWER_DOWN;
  
  /* write back control register */
  SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL1_XL, ctrl);
}

/**
  * @brief  Read LSM6DSL ID.
  * @retval ID 
  */
uint8_t LSM6DSL_AccReadID(void)
{  
  /* IO interface initialization */
  SENSOR_IO_Init();
  /* Read value at Who am I register address */
  return (SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_WHO_AM_I_REG));
}

/**
  * @brief  Set/Unset Accelerometer in low power mode.
  * @param  status 0 means disable Low Power Mode, otherwise Low Power Mode is enabled
  */
void LSM6DSL_AccLowPower(uint16_t status)
{
  uint8_t ctrl = 0x00;
  
  /* Read CTRL6_C value */
  ctrl = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL6_C);

  /* Clear Low Power Mode bit */
  ctrl &= ~(0x10);

  /* Set Low Power Mode */
  if(status)
  {
    ctrl |= LSM6DSL_ACC_GYRO_LP_XL_ENABLED;
  }else
  {
    ctrl |= LSM6DSL_ACC_GYRO_LP_XL_DISABLED;
  }
  
  /* write back control register */
  SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL6_C, ctrl);
}

/**
  * @brief  Read X, Y & Z Acceleration values 
  * @param  pData: Data out pointer
  */
void LSM6DSL_AccReadXYZ(int16_t* pData)
{
  int16_t pnRawData[3];
  uint8_t ctrlx= 0;
  uint8_t buffer[6];
  uint8_t i = 0;
  float sensitivity = 0;
  
  /* Read the acceleration control register content */
  ctrlx = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL1_XL);
  
  /* Read output register X, Y & Z acceleration */
  SENSOR_IO_ReadMultiple(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_OUTX_L_XL, buffer, 6);
  
  for(i=0; i<3; i++)
  {
    pnRawData[i]=((((uint16_t)buffer[2*i+1]) << 8) + (uint16_t)buffer[2*i]);
  }
  
  /* Normal mode */
  /* Switch the sensitivity value set in the CRTL1_XL */
  switch(ctrlx & 0x0C)
  {
  case LSM6DSL_ACC_FULLSCALE_2G:
    sensitivity = LSM6DSL_ACC_SENSITIVITY_2G;
    break;
  case LSM6DSL_ACC_FULLSCALE_4G:
    sensitivity = LSM6DSL_ACC_SENSITIVITY_4G;
    break;
  case LSM6DSL_ACC_FULLSCALE_8G:
    sensitivity = LSM6DSL_ACC_SENSITIVITY_8G;
    break;
  case LSM6DSL_ACC_FULLSCALE_16G:
    sensitivity = LSM6DSL_ACC_SENSITIVITY_16G;
    break;    
  }
  
  /* Obtain the mg value for the three axis */
  for(i=0; i<3; i++)
  {
    pData[i]=( int16_t )(pnRawData[i] * sensitivity);
  }
}

/**
  * @}
  */ 

/** @defgroup LSM6DSL_GYRO_Private_Functions LSM6DSL GYRO Private Functions
  * @{
  */

/**
  * @brief  Set LSM6DSL Gyroscope Initialization.
  * @param  InitStruct: pointer to a LSM6DSL_InitTypeDef structure 
  *         that contains the configuration setting for the LSM6DSL.
  */
void LSM6DSL_GyroInit(uint16_t InitStruct)
{  
  uint8_t ctrl = 0x00;
  uint8_t tmp;

  /* Read CTRL2_G */
  tmp = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL2_G);

  /* Write value to GYRO MEMS CTRL2_G register: FS and Data Rate */
  ctrl = (uint8_t) InitStruct;
  tmp &= ~(0xFC);
  tmp |= ctrl;
  SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL2_G, tmp);

  /* Read CTRL3_C */
  tmp = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL3_C);

  /* Write value to GYRO MEMS CTRL3_C register: BDU and Auto-increment */
  ctrl = ((uint8_t) (InitStruct >> 8));
  tmp &= ~(0x44);
  tmp |= ctrl; 
  SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL3_C, tmp);
}


/**
  * @brief LSM6DSL Gyroscope De-initialization
  */
void LSM6DSL_GyroDeInit(void)
{
  uint8_t ctrl = 0x00;
  
  /* Read control register 1 value */
  ctrl = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL2_G);

  /* Clear ODR bits */
  ctrl &= ~(LSM6DSL_ODR_BITPOSITION);

  /* Set Power down */
  ctrl |= LSM6DSL_ODR_POWER_DOWN;
  
  /* write back control register */
  SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL2_G, ctrl);
}

/**
  * @brief  Read ID address of LSM6DSL
  * @retval ID 
  */
uint8_t LSM6DSL_GyroReadID(void)
{
  /* IO interface initialization */
  SENSOR_IO_Init();  
  /* Read value at Who am I register address */
  return SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_WHO_AM_I_REG);
}

/**
  * @brief Set/Unset LSM6DSL Gyroscope in low power mode
  * @param  status 0 means disable Low Power Mode, otherwise Low Power Mode is enabled 
  */
void LSM6DSL_GyroLowPower(uint16_t status)
{  
  uint8_t ctrl = 0x00;
  
  /* Read CTRL7_G value */
  ctrl = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL7_G);

  /* Clear Low Power Mode bit */
  ctrl &= ~(0x80);

  /* Set Low Power Mode */
  if(status)
  {
    ctrl |= LSM6DSL_ACC_GYRO_LP_G_ENABLED;
  }else
  {
    ctrl |= LSM6DSL_ACC_GYRO_LP_G_DISABLED;
  }
  
  /* write back control register */
  SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL7_G, ctrl);
}

/**
* @brief  Calculate the LSM6DSL angular data.
* @param  pfData: Data out pointer
*/
void LSM6DSL_GyroReadXYZAngRate(float *pfData)
{
  int16_t pnRawData[3];
  uint8_t ctrlg= 0;
  uint8_t buffer[6];
  uint8_t i = 0;
  float sensitivity = 0;
  
  /* Read the gyro control register content */
  ctrlg = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL2_G);
  
  /* Read output register X, Y & Z acceleration */
  SENSOR_IO_ReadMultiple(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_OUTX_L_G, buffer, 6);
  
  for(i=0; i<3; i++)
  {
    pnRawData[i]=((((uint16_t)buffer[2*i+1]) << 8) + (uint16_t)buffer[2*i]);
  }
  
  /* Normal mode */
  /* Switch the sensitivity value set in the CRTL2_G */
  switch(ctrlg & 0x0C)
  {
  case LSM6DSL_GYRO_FS_245:
    sensitivity = LSM6DSL_GYRO_SENSITIVITY_245DPS;
    break;
  case LSM6DSL_GYRO_FS_500:
    sensitivity = LSM6DSL_GYRO_SENSITIVITY_500DPS;
    break;
  case LSM6DSL_GYRO_FS_1000:
    sensitivity = LSM6DSL_GYRO_SENSITIVITY_1000DPS;
    break;
  case LSM6DSL_GYRO_FS_2000:
    sensitivity = LSM6DSL_GYRO_SENSITIVITY_2000DPS;
    break;    
  }
  
  /* Obtain the mg value for the three axis */
  for(i=0; i<3; i++)
  {
    pfData[i]=( float )(pnRawData[i] * sensitivity);
  }
}

/**
  * @}
  */ 

/**
  * @}
  */ 
  
/**
  * @}
  */ 
  
/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

void SENSOR_IO_Init(void)
{
  I2Cx_Init(&hI2cHandler);
}

/**
  * @brief  DeInitializes Sensors low level.
  * @retval None
  */
void SENSOR_IO_DeInit(void)
{
  I2Cx_DeInit(&hI2cHandler);
}

/**
  * @brief  Writes a single data.
  * @param  Addr  I2C address
  * @param  Reg  Reg address
  * @param  Value  Data to be written
  * @retval None
  */
void SENSOR_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
  I2Cx_WriteMultiple(&hI2cHandler, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT,(uint8_t*)&Value, 1);
}

/**
  * @brief  Reads a single data.
  * @param  Addr  I2C address
  * @param  Reg  Reg address
  * @retval Data to be read
  */
uint8_t SENSOR_IO_Read(uint8_t Addr, uint8_t Reg)
{
  uint8_t read_value = 0;

  I2Cx_ReadMultiple(&hI2cHandler, Addr, Reg, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&read_value, 1);

  return read_value;
}

/**
  * @brief  Reads multiple data with I2C communication
  *         channel from TouchScreen.
  * @param  Addr  I2C address
  * @param  Reg  Register address
  * @param  Buffer  Pointer to data buffer
  * @param  Length  Length of the data
  * @retval HAL status
  */
uint16_t SENSOR_IO_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length)
{
 return I2Cx_ReadMultiple(&hI2cHandler, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length);
}

/**
  * @brief  Writes multiple data with I2C communication
  *         channel from MCU to TouchScreen.
  * @param  Addr  I2C address
  * @param  Reg  Register address
  * @param  Buffer  Pointer to data buffer
  * @param  Length  Length of the data
  * @retval None
  */
void SENSOR_IO_WriteMultiple(uint8_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length)
{
  I2Cx_WriteMultiple(&hI2cHandler, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length);
}

/**
  * @brief  Checks if target device is ready for communication.
  * @note   This function is used with Memory devices
  * @param  DevAddress  Target device address
  * @param  Trials  Number of trials
  * @retval HAL status
  */
HAL_StatusTypeDef SENSOR_IO_IsDeviceReady(uint16_t DevAddress, uint32_t Trials)
{
  return (I2Cx_IsDeviceReady(&hI2cHandler, DevAddress, Trials));
}

/**
  * @brief  Delay function used in Sensor low level driver.
  * @param  Delay  Delay in ms
  * @retval None
  */
void SENSOR_IO_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}

static void I2Cx_MspInit(I2C_HandleTypeDef *i2c_handler)
{
  GPIO_InitTypeDef  gpio_init_structure;

  /*** Configure the GPIOs ***/
  /* Enable GPIO clock */
  DISCOVERY_I2Cx_SCL_SDA_GPIO_CLK_ENABLE();

  /* Configure I2C Tx, Rx as alternate function */
  gpio_init_structure.Pin = DISCOVERY_I2Cx_SCL_PIN | DISCOVERY_I2Cx_SDA_PIN;
  gpio_init_structure.Mode = GPIO_MODE_AF_OD;
  gpio_init_structure.Pull = GPIO_PULLUP;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = DISCOVERY_I2Cx_SCL_SDA_AF;
  HAL_GPIO_Init(DISCOVERY_I2Cx_SCL_SDA_GPIO_PORT, &gpio_init_structure);

  HAL_GPIO_Init(DISCOVERY_I2Cx_SCL_SDA_GPIO_PORT, &gpio_init_structure);

  /*** Configure the I2C peripheral ***/
  /* Enable I2C clock */
  DISCOVERY_I2Cx_CLK_ENABLE();

  /* Force the I2C peripheral clock reset */
  DISCOVERY_I2Cx_FORCE_RESET();

  /* Release the I2C peripheral clock reset */
  DISCOVERY_I2Cx_RELEASE_RESET();

  /* Enable and set I2Cx Interrupt to a lower priority */
  HAL_NVIC_SetPriority(DISCOVERY_I2Cx_EV_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(DISCOVERY_I2Cx_EV_IRQn);

  /* Enable and set I2Cx Interrupt to a lower priority */
  HAL_NVIC_SetPriority(DISCOVERY_I2Cx_ER_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(DISCOVERY_I2Cx_ER_IRQn);
}

/**
  * @brief  DeInitializes I2C MSP.
  * @param  i2c_handler  I2C handler
  * @retval None
  */
static void I2Cx_MspDeInit(I2C_HandleTypeDef *i2c_handler)
{
  GPIO_InitTypeDef  gpio_init_structure;

  /* Configure I2C Tx, Rx as alternate function */
  gpio_init_structure.Pin = DISCOVERY_I2Cx_SCL_PIN | DISCOVERY_I2Cx_SDA_PIN;
  HAL_GPIO_DeInit(DISCOVERY_I2Cx_SCL_SDA_GPIO_PORT, gpio_init_structure.Pin);
  /* Disable GPIO clock */
  DISCOVERY_I2Cx_SCL_SDA_GPIO_CLK_DISABLE();

  /* Disable I2C clock */
  DISCOVERY_I2Cx_CLK_DISABLE();
}

static void I2Cx_Init(I2C_HandleTypeDef *i2c_handler)
{
  /* I2C configuration */
  i2c_handler->Instance              = DISCOVERY_I2Cx;
  i2c_handler->Init.Timing           = DISCOVERY_I2Cx_TIMING;
  i2c_handler->Init.OwnAddress1      = 0;
  i2c_handler->Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
  i2c_handler->Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
  i2c_handler->Init.OwnAddress2      = 0;
  i2c_handler->Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
  i2c_handler->Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

  /* Init the I2C */
  I2Cx_MspInit(i2c_handler);
  HAL_I2C_Init(i2c_handler);

  /**Configure Analogue filter */
  HAL_I2CEx_ConfigAnalogFilter(i2c_handler, I2C_ANALOGFILTER_ENABLE);
}

/**
  * @brief  DeInitializes I2C HAL.
  * @param  i2c_handler  I2C handler
  * @retval None
  */
static void I2Cx_DeInit(I2C_HandleTypeDef *i2c_handler)
{  /* DeInit the I2C */
  I2Cx_MspDeInit(i2c_handler);
  HAL_I2C_DeInit(i2c_handler);
}

static HAL_StatusTypeDef I2Cx_ReadMultiple(I2C_HandleTypeDef *i2c_handler, uint8_t Addr, uint16_t Reg, uint16_t MemAddress, uint8_t *Buffer, uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_I2C_Mem_Read(i2c_handler, Addr, (uint16_t)Reg, MemAddress, Buffer, Length, 1000);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* I2C error occurred */
    I2Cx_Error(i2c_handler, Addr);
  }
  return status;
}


/**
  * @brief  Writes a value in a register of the device through BUS in using DMA mode.
  * @param  i2c_handler  I2C handler
  * @param  Addr  Device address on BUS Bus.
  * @param  Reg  The target register address to write
  * @param  MemAddress  memory address
  * @param  Buffer  The target register value to be written
  * @param  Length  buffer size to be written
  * @retval HAL status
  */
static HAL_StatusTypeDef I2Cx_WriteMultiple(I2C_HandleTypeDef *i2c_handler, uint8_t Addr, uint16_t Reg, uint16_t MemAddress, uint8_t *Buffer, uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_I2C_Mem_Write(i2c_handler, Addr, (uint16_t)Reg, MemAddress, Buffer, Length, 1000);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Re-Initiaize the I2C Bus */
    I2Cx_Error(i2c_handler, Addr);
  }
  return status;
}

/**
  * @brief  Checks if target device is ready for communication.
  * @note   This function is used with Memory devices
  * @param  i2c_handler  I2C handler
  * @param  DevAddress  Target device address
  * @param  Trials  Number of trials
  * @retval HAL status
  */
static HAL_StatusTypeDef I2Cx_IsDeviceReady(I2C_HandleTypeDef *i2c_handler, uint16_t DevAddress, uint32_t Trials)
{
  return (HAL_I2C_IsDeviceReady(i2c_handler, DevAddress, Trials, 1000));
}

/**
  * @brief  Manages error callback by re-initializing I2C.
  * @param  i2c_handler  I2C handler
  * @param  Addr  I2C Address
  * @retval None
  */
static void I2Cx_Error(I2C_HandleTypeDef *i2c_handler, uint8_t Addr)
{
  /* De-initialize the I2C communication bus */
  HAL_I2C_DeInit(i2c_handler);

  /* Re-Initialize the I2C communication bus */
  I2Cx_Init(i2c_handler);
}
