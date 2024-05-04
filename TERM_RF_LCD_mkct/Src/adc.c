/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "adc.h"

/* USER CODE BEGIN 0 */
#include "control.h"
#include "setup.h"

u16 Aver_M1[ARR1_SIZE + 3];
u16 Aver_M2[ARR1_SIZE + 3];
u16 Aver_M3[ARR1_SIZE + 3];

volatile uint16_t adc_data[ARR1_SIZE*3];
float VIN1;
float VIN2;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	u16 i;
	s32 tmp[3] = {0,0,0};
	u16 arg;
    
    if(hadc->Instance == ADC1) //check if the interrupt comes from ACD1
    {
        for(i = 0; i < ARR1_SIZE*3; i+=3)
        {
            tmp[0] += adc_data[i];
            tmp[1] += adc_data[i+1];
            tmp[2] += adc_data[i+2];
        }
//--------------------------------------------
        arg = tmp[0]/ARR1_SIZE;
        arg = cont_Aver_u16(Aver_M1, arg, ARR1_SIZE);
        dev_var.adc_det[0] = arg;
        dev_var.vdev = (float)arg*Setup.bt_coef;
//--------------------------------------------
        arg = tmp[1]/ARR1_SIZE;
        arg = cont_Aver_u16(Aver_M2, arg, ARR1_SIZE);
        dev_var.adc_det[1] = arg;
        dev_var.level = (arg*100)/4092;
        if(dev_var.level == 0)
        {
            dev_var.lim_max = dev_var.lim_min = 0.0f;
        }
        else
        {
            dev_var.lim_max = ((float)dev_var.level*dev_var.quant)+0.003f;
            dev_var.lim_min = dev_var.lim_max-0.006f;
            if(dev_var.lim_min<=0.0f)dev_var.lim_min = 0.0005f;
        }
//--------------------------------------------
        arg = tmp[2]/ARR1_SIZE;
        arg = cont_Aver_u16(Aver_M3, arg, ARR1_SIZE);
        dev_var.adc_det[2] = arg;
        dev_var.curr = (Setup.coef_curr1*(float)arg+Setup.coef_curr2)-Setup.shift_curr;
//--------------------------------------------
    }
}
/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 3;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA3     ------> ADC1_IN3
    PA5     ------> ADC1_IN5
    PA6     ------> ADC1_IN6
    */
    GPIO_InitStruct.Pin = CURR_Pin|BAT_Pin|LEV_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc1);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PA3     ------> ADC1_IN3
    PA5     ------> ADC1_IN5
    PA6     ------> ADC1_IN6
    */
    HAL_GPIO_DeInit(GPIOA, CURR_Pin|BAT_Pin|LEV_Pin);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
