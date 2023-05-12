/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "RG12864B.h"
#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_hal_cortex.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_uart.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    M_NEW_DRAWING,
    M_LOAD_DRAWING,
    M_SAVE_DRAWING,
    M_CHANGE_BRIGHTNESS
} MenuItem;

typedef enum {
    S_MENU,
    S_DRAWING,
    S_SAVE,
    S_CHANGE_BRIGHTNESS,
} State;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADC_PRECISION         4096
#define LCD_BITMAP_FLASH_ADDR 0x0800C100
#define RX_BUFF_SIZE          4
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim8;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
extern uint8_t    LCD_bitmap[8][LCD_WIDTH];
uint32_t          x_values[ADC_PRECISION] = {0};
uint32_t          y_values[ADC_PRECISION] = {0};
uint32_t          x, y;
volatile MenuItem selectedMenu          = M_NEW_DRAWING;
volatile State    state                 = S_MENU;
uint8_t           newDrawing            = 0;
uint8_t           Rx_buff[RX_BUFF_SIZE] = {0};
uint8_t           Rx_byte;
uint8_t           Rx_LCD_bitmap[8][LCD_WIDTH];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void        SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM8_Init(void);
static void MX_ADC2_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim1, 0); // set the counter value a 0
    while (__HAL_TIM_GET_COUNTER(&htim1) < us)
        ; // wait for the counter to reach the us input in the parameter
}

void updateFlash()
{
    HAL_FLASH_Unlock();
    FLASH_Erase_Sector(3, FLASH_VOLTAGE_RANGE_3);
    // brightness
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, LCD_BRIGHTNESS_FLASH_ADDR,
                      (uint64_t)LCD_BRIGHTNESS);
    // LCD_bitmap
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < LCD_WIDTH; j++) {
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,
                              LCD_BITMAP_FLASH_ADDR + i * LCD_WIDTH + j,
                              (uint64_t)LCD_bitmap[i][j]);
        }
    }
    HAL_FLASH_Lock();
}

void menuLoop()
{
    char *menus[] = {"- Start new drawing", "- Load drawing", "- Save drawing",
                     "- Change brightness"};
    LCD_Clear();

    LCD_DrawString("Main menu", 64 - 6 * 4, 0);

    LCD_DrawStringInverse("UP", 32 - 6, 7);
    LCD_DrawStringInverse("OK", 64 - 6, 7);
    LCD_DrawStringInverse("DOWN", 96 - 2 * 6, 7);

    while (state == S_MENU) {
        HAL_ADC_Start_DMA(&hadc1, x_values, ADC_PRECISION);
        HAL_ADC_Start_DMA(&hadc2, y_values, ADC_PRECISION);
        for (int i = 0; i < 4; i++) {
            switch (selectedMenu) {
                case M_NEW_DRAWING:
                    if (i == 0)
                        i++;
                    LCD_DrawStringInverse(menus[0], 1, 2);
                    break;
                case M_LOAD_DRAWING:
                    if (i == 1)
                        i++;
                    LCD_DrawStringInverse(menus[1], 1, 3);
                    break;
                case M_SAVE_DRAWING:
                    if (i == 2)
                        i++;
                    LCD_DrawStringInverse(menus[2], 1, 4);
                    break;
                case M_CHANGE_BRIGHTNESS:
                    if (i == 3)
                        continue;
                    LCD_DrawStringInverse(menus[3], 1, 5);
                    break;
            }
            LCD_DrawString(menus[i], 1, i + 2);
        }
    }
}

void drawingLoop()
{
    LCD_Clear();
    if (newDrawing) {
        newDrawing = 0;
        LCD_ClearBitmap();
    } else {
        memcpy(LCD_bitmap, (void *)LCD_BITMAP_FLASH_ADDR, 8 * LCD_WIDTH);
        LCD_DrawBitmap();
    }

    while (state == S_DRAWING) {
        if (newDrawing) {
            newDrawing = 0;
            LCD_Clear();
            LCD_ClearBitmap();
        }
        HAL_ADC_Start_DMA(&hadc1, x_values, ADC_PRECISION);
        HAL_ADC_Start_DMA(&hadc2, y_values, ADC_PRECISION);

        x = y = 0;
        for (int i = 0; i < ADC_PRECISION; i++) {
            x += x_values[i];
            y += y_values[i];
        }
        x /= ADC_PRECISION / 2;
        y /= ADC_PRECISION / 2;

        x = ~(x >> 6) & 0x7f;
        y = y >> 7;

        LCD_SetPixel(x, y);
    }
}

void brightnessLoop()
{
    LCD_Clear();

    LCD_DrawString("Brightness", 64 - 6 * 5, 0);

    LCD_DrawStringInverse("+", 32, 7);
    LCD_DrawStringInverse("OK", 64 - 6, 7);
    LCD_DrawStringInverse("-", 96 - 6, 7);

    char buff[5] = {0};
    while (state == S_CHANGE_BRIGHTNESS) {
        snprintf(buff, 5, "%3d%%", (LCD_BRIGHTNESS * 100) / 0xFFFF);
        LCD_DrawString(buff, 64 - 6 * 2, 3);

        if (!HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin)) {
            if (99 * 0xFFFF / 100 < LCD_BRIGHTNESS) {
                LCD_BRIGHTNESS = 0xFFFF;
            } else {
                LCD_BRIGHTNESS += 0xFFFF / 100;
            }
            HAL_Delay(25);
        }

        if (!HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin)) {
            if (1 * 0xFFFF / 100 >= LCD_BRIGHTNESS) {
                LCD_BRIGHTNESS = 0xFFFF / 100;
            } else {
                LCD_BRIGHTNESS -= 0xFFFF / 100;
            }
            HAL_Delay(25);
        }
    }

    if (LCD_BRIGHTNESS != LCD_BRIGHTNESS_FLASH) {
        updateFlash();
    }
}

void saveDrawing()
{
    state = S_MENU;
    LCD_Clear();
    LCD_DrawString("Saving...", 64 - 5 * 5, 3);
    //uart
    HAL_UART_Transmit(&huart2, "Hello mi?\n\r", 13, 100);
    updateFlash();
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU
     * Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the
     * Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_USART2_UART_Init();
    MX_TIM8_Init();
    MX_ADC2_Init();
    MX_TIM1_Init();
    /* USER CODE BEGIN 2 */
    HAL_TIM_Base_Start(&htim1);
    HAL_UART_Receive_IT(&huart2, &Rx_byte, 1);
    LCD_Init();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        switch (state) {
            case S_MENU:
                menuLoop();
                break;
            case S_DRAWING:
                drawingLoop();
                break;
            case S_SAVE:
                saveDrawing();
                break;
            case S_CHANGE_BRIGHTNESS:
                brightnessLoop();
                break;
        }
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = 16;
    RCC_OscInitStruct.PLL.PLLN            = 336;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ            = 2;
    RCC_OscInitStruct.PLL.PLLR            = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void)
{

    /* USER CODE BEGIN ADC1_Init 0 */

    /* USER CODE END ADC1_Init 0 */

    ADC_ChannelConfTypeDef sConfig = {0};

    /* USER CODE BEGIN ADC1_Init 1 */

    /* USER CODE END ADC1_Init 1 */

    /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
     */
    hadc1.Instance                   = ADC1;
    hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode          = ENABLE;
    hadc1.Init.ContinuousConvMode    = ENABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion       = 1;
    hadc1.Init.DMAContinuousRequests = ENABLE;
    hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        Error_Handler();
    }

    /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
     */
    sConfig.Channel      = ADC_CHANNEL_12;
    sConfig.Rank         = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN ADC1_Init 2 */

    /* USER CODE END ADC1_Init 2 */
}

/**
 * @brief ADC2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC2_Init(void)
{

    /* USER CODE BEGIN ADC2_Init 0 */

    /* USER CODE END ADC2_Init 0 */

    ADC_ChannelConfTypeDef sConfig = {0};

    /* USER CODE BEGIN ADC2_Init 1 */

    /* USER CODE END ADC2_Init 1 */

    /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
     */
    hadc2.Instance                   = ADC2;
    hadc2.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc2.Init.Resolution            = ADC_RESOLUTION_12B;
    hadc2.Init.ScanConvMode          = ENABLE;
    hadc2.Init.ContinuousConvMode    = ENABLE;
    hadc2.Init.DiscontinuousConvMode = DISABLE;
    hadc2.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc2.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    hadc2.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc2.Init.NbrOfConversion       = 1;
    hadc2.Init.DMAContinuousRequests = ENABLE;
    hadc2.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    if (HAL_ADC_Init(&hadc2) != HAL_OK) {
        Error_Handler();
    }

    /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
     */
    sConfig.Channel      = ADC_CHANNEL_15;
    sConfig.Rank         = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN ADC2_Init 2 */

    /* USER CODE END ADC2_Init 2 */
}

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void)
{

    /* USER CODE BEGIN TIM1_Init 0 */

    /* USER CODE END TIM1_Init 0 */

    TIM_ClockConfigTypeDef  sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig      = {0};

    /* USER CODE BEGIN TIM1_Init 1 */

    /* USER CODE END TIM1_Init 1 */
    htim1.Instance               = TIM1;
    htim1.Init.Prescaler         = 84 - 1;
    htim1.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim1.Init.Period            = 0xFFFF - 1;
    htim1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM1_Init 2 */

    /* USER CODE END TIM1_Init 2 */
}

/**
 * @brief TIM8 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM8_Init(void)
{

    /* USER CODE BEGIN TIM8_Init 0 */

    /* USER CODE END TIM8_Init 0 */

    TIM_ClockConfigTypeDef         sClockSourceConfig   = {0};
    TIM_MasterConfigTypeDef        sMasterConfig        = {0};
    TIM_OC_InitTypeDef             sConfigOC            = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    /* USER CODE BEGIN TIM8_Init 1 */

    /* USER CODE END TIM8_Init 1 */
    htim8.Instance               = TIM8;
    htim8.Init.Prescaler         = 0;
    htim8.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim8.Init.Period            = 65535;
    htim8.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim8.Init.RepetitionCounter = 0;
    htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim8) != HAL_OK) {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim8) != HAL_OK) {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    sConfigOC.OCMode       = TIM_OCMODE_PWM1;
    sConfigOC.Pulse        = 0;
    sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_4) !=
        HAL_OK) {
        Error_Handler();
    }
    sBreakDeadTimeConfig.OffStateRunMode  = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel        = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime         = 0;
    sBreakDeadTimeConfig.BreakState       = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity    = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.AutomaticOutput  = TIM_AUTOMATICOUTPUT_DISABLE;
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig) !=
        HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM8_Init 2 */

    /* USER CODE END TIM8_Init 2 */
    HAL_TIM_MspPostInit(&htim8);
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{

    /* USER CODE BEGIN USART2_Init 0 */

    /* USER CODE END USART2_Init 0 */

    /* USER CODE BEGIN USART2_Init 1 */

    /* USER CODE END USART2_Init 1 */
    huart2.Instance          = USART2;
    huart2.Init.BaudRate     = 115200;
    huart2.Init.WordLength   = UART_WORDLENGTH_8B;
    huart2.Init.StopBits     = UART_STOPBITS_1;
    huart2.Init.Parity       = UART_PARITY_NONE;
    huart2.Init.Mode         = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN USART2_Init 2 */

    /* USER CODE END USART2_Init 2 */
}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void)
{

    /* DMA controller clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA2_Stream0_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
    /* DMA2_Stream2_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* USER CODE BEGIN MX_GPIO_Init_1 */
    /* USER CODE END MX_GPIO_Init_1 */

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA,
                      LCD_R_W_Pin | LCD_RST_Pin | LCD_DB7_Pin | LCD_DB3_Pin |
                          LCD_DB4_Pin,
                      GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB,
                      LCD_DB2_Pin | xLCD_EN_Pin | LCD_DB1_Pin | LCD_DB0_Pin |
                          LCD_DB6_Pin | LCD_CS1_Pin | LCD_D_I_Pin,
                      GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOC, LCD_E_Pin | LCD_DB5_Pin | LCD_CS2_Pin,
                      GPIO_PIN_RESET);

    /*Configure GPIO pins : LCD_R_W_Pin LCD_RST_Pin LCD_DB7_Pin LCD_DB3_Pin
                             LCD_DB4_Pin */
    GPIO_InitStruct.Pin =
        LCD_R_W_Pin | LCD_RST_Pin | LCD_DB7_Pin | LCD_DB3_Pin | LCD_DB4_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pins : LCD_DB2_Pin xLCD_EN_Pin LCD_DB1_Pin LCD_DB0_Pin
                             LCD_DB6_Pin LCD_CS1_Pin LCD_D_I_Pin */
    GPIO_InitStruct.Pin = LCD_DB2_Pin | xLCD_EN_Pin | LCD_DB1_Pin |
                          LCD_DB0_Pin | LCD_DB6_Pin | LCD_CS1_Pin | LCD_D_I_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : LCD_E_Pin LCD_DB5_Pin LCD_CS2_Pin */
    GPIO_InitStruct.Pin   = LCD_E_Pin | LCD_DB5_Pin | LCD_CS2_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pins : BTN1_Pin BTN3_Pin BTN2_Pin */
    GPIO_InitStruct.Pin  = BTN1_Pin | BTN3_Pin | BTN2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    /* USER CODE BEGIN MX_GPIO_Init_2 */
    /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (state) {
        case S_MENU:
            if (GPIO_Pin == BTN3_Pin) {
                selectedMenu == M_CHANGE_BRIGHTNESS ? selectedMenu = 0
                                                    : selectedMenu++;
            } else if (GPIO_Pin == BTN1_Pin) {
                selectedMenu == 0 ? selectedMenu = M_CHANGE_BRIGHTNESS
                                  : selectedMenu--;
            } else if (GPIO_Pin == BTN2_Pin) {
                switch (selectedMenu) {
                    case M_NEW_DRAWING:
                        newDrawing = 1;
                    case M_LOAD_DRAWING:
                        state = S_DRAWING;
                        break;
                    case M_SAVE_DRAWING:
                        state = S_SAVE;
                        break;
                    case M_CHANGE_BRIGHTNESS:
                        state = S_CHANGE_BRIGHTNESS;
                        break;
                }
            }
            break;
        case S_DRAWING:
            if (GPIO_Pin == BTN3_Pin) {
                newDrawing = 1;
            } else if (GPIO_Pin == BTN1_Pin) {
                state = S_MENU;
            }
            break;
        case S_SAVE:
            break;
        case S_CHANGE_BRIGHTNESS:
            if (GPIO_Pin == BTN2_Pin) {
                state = S_MENU;
            }
            break;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    static uint8_t i = 0, f = 0;
    if (Rx_byte == '@') {
        i = 0;
        f = 1;
    } else if (f) {
        Rx_buff[i < RX_BUFF_SIZE ? i++ : (i = 0)] = Rx_byte;
    }
    if (i == RX_BUFF_SIZE) {
        i = f = 0;
        if (!strncmp((char *)Rx_buff, "SAVE", RX_BUFF_SIZE)) {
            // TODO: Send drawing
            //HAL_UART_Transmit_IT(&huart2, (uint8_t *)"SAVE\n\r", 6);
            //state.save = 1;
            //state.menu = state.change_brightness = state.drawing = 0;
        } else if (!strncmp((char *)Rx_buff, "LOAD", RX_BUFF_SIZE)) {
            // TODO: Receive drawing
            //LCD_Clear();
            //LCD_DrawString("Receiving...", 20, 3);
            //HAL_UART_Receive(&huart2, (uint8_t*)Rx_LCD_bitmap, 8*LCD_WIDTH, -1);
            //LCD_Clear();
            //memcpy(LCD_bitmap, Rx_LCD_bitmap, 8*LCD_WIDTH);
            //state.drawing    = 1;
            //state.newDrawing = state.menu = state.change_brightness =
            //    state.save                = 0;
        }
    }
    HAL_UART_Receive_IT(&huart2, &Rx_byte, 1);
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state
     */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line
       number,
       //ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
       line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
