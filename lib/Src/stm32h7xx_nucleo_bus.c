/**
  ******************************************************************************
  * @file    stm32h7xx_nucleo_bus.c
  * @author  MCD Application Team
  * @brief   This file provides set of Bus IO functions to manage:
  *          - LCD and microSD available on Adafruit 1.8" TFT LCD 
  *            shield (reference ID 802)
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */ 
  
/*
 * modified for preenfm3
 * Don't initialize SPI pins, it's already done
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_nucleo_bus.h"

/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup ADAFRUIT_802
  * @{
  */   
    
/** @addtogroup ADAFRUIT_802_BUS 
  * @brief This file provides set of firmware functions to manage the bus operations
  *        for adafruit shield
  * @{
  */ 

/** @defgroup ADAFRUIT_802_BUS_Exported_Variables BUS Exported Variables
  * @{
  */ 
SPI_HandleTypeDef sd_spi2;
/**
  * @}
  */ 

/** @defgroup ADAFRUIT_802_BUS_Private_Variables BUS Private Variables
  * @{
  */
#if (USE_HAL_SPI_REGISTER_CALLBACKS == 1)
static uint32_t IsSpiMspCbValid = 0;
#endif
/**
  * @}
  */ 

/** @defgroup ADAFRUIT_802_BUS_Private_FunctionsPrototypes BUS Private Functions Prototypes
  * @{
  */
static void SPI_MspInit(SPI_HandleTypeDef *hspi);
static void SPI_MspDeInit(SPI_HandleTypeDef *hspi);
static uint32_t SPI_GetPrescaler(uint32_t clock_src_freq, uint32_t baudrate_mbps);
/**
  * @}
  */ 

/** @addtogroup ADAFRUIT_802_BUS_Private_Functions
  * @{
  */ 

/**
  * @brief  Initializes SPI HAL.
  * @retval None
  */
int32_t BSP_SPI_Init(void)
{  
  if(HAL_SPI_GetState(&sd_spi2) == HAL_SPI_STATE_RESET)
  {
#if (USE_HAL_SPI_REGISTER_CALLBACKS == 0)
    /* Init the SPI Msp */
    SPI_MspInit(&sd_spi2);
#else
    if(IsSpiMspCbValid == 0U)
    {
      if(BSP_SPI_RegisterDefaultMspCallbacks() != BSP_ERROR_NONE)
      {
        return BSP_ERROR_MSP_FAILURE;
      }
    }
#endif
    
    if(MX_SPI_Init(&sd_spi2, SPI_GetPrescaler(HAL_RCC_GetHCLKFreq(), BUS_SPI_BAUDRATE)) != HAL_OK)
    {
      return BSP_ERROR_BUS_FAILURE;          
    }
  } 
  return BSP_ERROR_NONE;
}

/**
  * @brief  DeInitializes SPI HAL.
  * @retval None
  */
int32_t BSP_SPI_DeInit(void)
{  
#if (USE_HAL_SPI_REGISTER_CALLBACKS == 0)  
  SPI_MspDeInit(&sd_spi2);  
#endif /* (USE_HAL_SPI_REGISTER_CALLBACKS == 0) */
  
  /* Init the SPI */  
  if (HAL_SPI_DeInit(&sd_spi2) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  
  return BSP_ERROR_NONE;
}

/**
  * @brief  MX SPI initialization.
  * @param  hspi SPI handle
  * @retval HAL status
  */
__weak HAL_StatusTypeDef MX_SPI_Init(SPI_HandleTypeDef *phspi, uint32_t baudrate_presc)
{ 
  /* SPI Config */
  phspi->Instance               = SPI2;
  phspi->Init.BaudRatePrescaler = baudrate_presc;
  phspi->Init.Direction         = SPI_DIRECTION_2LINES;
  phspi->Init.CLKPhase          = SPI_PHASE_2EDGE;
  phspi->Init.CLKPolarity       = SPI_POLARITY_HIGH;
  phspi->Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  phspi->Init.CRCPolynomial     = 7;
  phspi->Init.DataSize          = SPI_DATASIZE_8BIT;
  phspi->Init.FirstBit          = SPI_FIRSTBIT_MSB;
  phspi->Init.NSS               = SPI_NSS_SOFT;
  phspi->Init.TIMode            = SPI_TIMODE_DISABLE;
  phspi->Init.Mode              = SPI_MODE_MASTER;
  phspi->Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;  /* Recommanded setting to avoid glitches */

  phspi->Init.FifoThreshold              = SPI_FIFO_THRESHOLD_08DATA;
  phspi->Init.CRCLength                  = SPI_CRC_LENGTH_8BIT;
  phspi->Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  phspi->Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  phspi->Init.NSSPolarity                = SPI_NSS_POLARITY_LOW;
  phspi->Init.NSSPMode                   = SPI_NSS_PULSE_DISABLE;
  phspi->Init.MasterSSIdleness           = 0x00000000;
  phspi->Init.MasterInterDataIdleness    = 0x00000000;
  phspi->Init.MasterReceiverAutoSusp     = 0x00000000;

  return HAL_SPI_Init(phspi);
}


/**
  * @brief  Send a byte to device
  * @param  pTxData pointer to TX buffer
  * @param  Length data length
  * @retval BSP status
  */
int32_t BSP_SPI_Send(uint8_t *pTxData, uint32_t Legnth)
{
  int32_t ret = BSP_ERROR_NONE;
  
  if(HAL_SPI_Transmit(&sd_spi2, pTxData, (uint16_t)Legnth, BUS_SPI_TIMEOUT_MAX) != HAL_OK)
  {
    ret = BSP_ERROR_BUS_FAILURE;
  }
  
  return ret;
}

/**
  * @brief  Send a byte to device
  * @param  pRxData pointer to RX buffer
  * @param  Length data length
  * @retval BSP status
  */
int32_t BSP_SPI_Recv(uint8_t *pRxData, uint32_t Legnth)
{
  int32_t ret = BSP_ERROR_NONE;
  
  if(HAL_SPI_Receive(&sd_spi2, pRxData, (uint16_t)Legnth, BUS_SPI_TIMEOUT_MAX) != HAL_OK)
  {
    ret = BSP_ERROR_BUS_FAILURE;
  }
  
  return ret;
}

/**
  * @brief  Send/Receive an amount of bytes
  * @param  pTxData pointer to TX buffer
  * @param  pRxData pointer to RX buffer
  * @param  Length data length
  * @retval BSP status
  */
int32_t BSP_SPI_SendRecv(uint8_t *pTxData, uint8_t *pRxData, uint32_t Legnth)
{
  int32_t ret = BSP_ERROR_NONE;
  
  if(HAL_SPI_TransmitReceive(&sd_spi2, pTxData, pRxData, (uint16_t)Legnth, BUS_SPI_TIMEOUT_MAX) != HAL_OK)
  {
    ret = BSP_ERROR_BUS_FAILURE;
  }
  
  return ret;
}

HAL_StatusTypeDef BSP_SPI_SendRecv_DMA(uint8_t *pTxData, uint8_t *pRxData, uint32_t Legnth)
{
  if (sd_spi2.Lock == HAL_LOCKED) {
      return HAL_BUSY;
  }
  //HAL_SPI_TransmitReceive_IT(&sd_spi2, pTxData, pRxData, (uint16_t)Legnth);
  return HAL_SPI_TransmitReceive_DMA(&sd_spi2, pTxData, pRxData, (uint16_t)Legnth);
}

/**
  * @brief  Delay function
  * @retval Tick value
  */
int32_t BSP_GetTick(void)
{
  return (int32_t)HAL_GetTick();
}

#if (USE_HAL_SPI_REGISTER_CALLBACKS == 1) 
/**
  * @brief Register Default SPI Bus Msp Callbacks
  * @retval BSP status
  */
int32_t BSP_SPI_RegisterDefaultMspCallbacks (void)
{
  int32_t ret = BSP_ERROR_NONE;
  
  __HAL_SPI_RESET_HANDLE_STATE(&sd_spi2);
  
  /* Register default MspInit/MspDeInit Callback */
  if(HAL_SPI_RegisterCallback(&sd_spi2, HAL_SPI_MSPINIT_CB_ID, SPI_MspInit) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else if(HAL_SPI_RegisterCallback(&sd_spi2, HAL_SPI_MSPDEINIT_CB_ID, SPI_MspDeInit) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    IsSpiMspCbValid = 1U;
  }
  
  /* BSP status */  
  return ret;
}

/**
  * @brief Register SPI Bus Msp Callback registering
  * @param Callbacks     pointer to SPI MspInit/MspDeInit callback functions
  * @retval BSP status
  */
int32_t BSP_SPI_RegisterMspCallbacks (BSP_SPI_Cb_t *Callback)
{
  int32_t ret = BSP_ERROR_NONE;
  
  __HAL_SPI_RESET_HANDLE_STATE(&sd_spi2);
  
  /* Register MspInit/MspDeInit Callbacks */
  if(HAL_SPI_RegisterCallback(&sd_spi2, HAL_SPI_MSPINIT_CB_ID, Callback->pMspSpiInitCb) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else if(HAL_SPI_RegisterCallback(&sd_spi2, HAL_SPI_MSPDEINIT_CB_ID, Callback->pMspSpiDeInitCb) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    IsSpiMspCbValid = 1U;
  }
  
  /* BSP status */  
  return ret; 
}
#endif /* USE_HAL_SPI_REGISTER_CALLBACKS */

/** @addtogroup ADAFRUIT_802_BUS_Private_Functions BUS Private Functions
  * @{
  */

/**
  * @brief  Initializes SPI MSP.
  * @param  hspi SPI handler
  * @retval None
  */
static void SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    // SD_CS GPIO already init  in preenfm3 main.c
}

/**
  * @brief  DeInitializes SPI MSP.
  * @param  hspi SPI handler
  * @retval None
  */
static void SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    // SD_CS GPIO already De-init  in preenfm3 main.c
}

/**
  * @brief  Set SPI frequency to calculate correspondent baud-rate prescaler.
  * @param  clock_src_freq  Frequency of clock source  
  * @param  baudrate_mbps Baudrate to set to set
  * @retval Baudrate prescaler
  */
static uint32_t SPI_GetPrescaler(uint32_t clock_src_freq, uint32_t baudrate_mbps)
{
  uint32_t divisor = 0; 
  uint32_t spi_clk = clock_src_freq; 
  uint32_t presc = 0; 
  static const uint32_t baudrate[]= 
  { SPI_BAUDRATEPRESCALER_2, 
    SPI_BAUDRATEPRESCALER_4, 
    SPI_BAUDRATEPRESCALER_8, 
    SPI_BAUDRATEPRESCALER_16, 
    SPI_BAUDRATEPRESCALER_32, 
    SPI_BAUDRATEPRESCALER_64,
    SPI_BAUDRATEPRESCALER_128,
    SPI_BAUDRATEPRESCALER_256,
  };
  
  while( spi_clk > baudrate_mbps)
  {
    presc = baudrate[divisor]; 
    if (++divisor > 7) 
      break; 
    
    spi_clk = ( spi_clk >> 1); 
  }
  
  return presc;
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
