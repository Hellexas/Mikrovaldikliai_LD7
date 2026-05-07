/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "ssd1306.h"
#include "ssd1306_fonts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include <stdlib.h> // rand() funkcijai
#include <stdio.h>  // sprintf() funkcijai

// Žaidimo būsenos
typedef enum { MENU, PLAYING, GAME_OVER } GameState;
GameState state = MENU;

// Gaudyklės (Player) kintamieji
int player_x = 54;       // Pradinė pozicija ekrano viduryje (128/2 - 20/2)
const int player_w = 20; // Gaudyklės plotis pikseliais
const int player_y = 58; // Gaudyklės Y pozicija (ekrano apačia)

// Krentančio objekto (Baito) kintamieji
int obj_x = 60;
int obj_y = 10;

// Statistika
int score = 0;
int lives = 3;

// Laikmačiai polling'ui ir debounce'ui
uint32_t last_fall_time = 0;
uint32_t last_btn_time = 0;

// Vėliavėlė (flag), nurodanti, ar reikia perpiešti ekraną
uint8_t needs_redraw = 1;

// Funkcija naujam objektui ekrano viršuje sukurti
void Spawn_Object() {
    // Atsitiktinė X pozicija nuo 0 iki (128 - raidės plotis)
    obj_x = rand() % 118;
    obj_y = 10; // Pradeda kristi po rezultatų juosta
}

// Funkcija žaidimui atstatyti
void Reset_Game() {
    score = 0;
    lives = 3;
    player_x = 54;
    Spawn_Object();
    state = PLAYING;
    needs_redraw = 1;
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

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  ssd1306_Init(); // Paleidžiame ekraną
  // Galime inicijuoti atsitiktinių skaičių generatorių pagal sistemos laiką
  srand(HAL_GetTick());
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
        // 1. START / STOP / PAUSE (PA0 Mygtukas)
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET && (HAL_GetTick() - last_btn_time > 200)) {
            if (state == MENU || state == GAME_OVER) {
                Reset_Game(); // Pradedame iš naujo
            } else if (state == PLAYING) {
                state = MENU; // Pauzė
                needs_redraw = 1;
            }
            last_btn_time = HAL_GetTick();
        }

        // 2. ŽAIDIMO LOGIKA (vyksta tik kai būsena PLAYING)
        if (state == PLAYING) {

            // LEFT (PE2 Mygtukas - aktyvus LOW dėl Pull-up)
            if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2) == GPIO_PIN_RESET && (HAL_GetTick() - last_btn_time > 50)) {
                if (player_x > 0) player_x -= 5; // Judam kairėn
                needs_redraw = 1;
                last_btn_time = HAL_GetTick();
            }

            // RIGHT (PE3 Mygtukas - aktyvus LOW dėl Pull-up)
            if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3) == GPIO_PIN_RESET && (HAL_GetTick() - last_btn_time > 50)) {
                if (player_x < (128 - player_w)) player_x += 5; // Judam dešinėn
                needs_redraw = 1;
                last_btn_time = HAL_GetTick();
            }

            // Objekto kritimas kas 150 ms (Pagal užduoties reikalavimą)
            if (HAL_GetTick() - last_fall_time > 150) {
                obj_y += 3; // Objektas slenka žemyn

                // Susidūrimo detekcija (kai objektas pasiekia gaudyklės aukštį)
                if (obj_y >= player_y - 8) {
                    // Tikriname X ašį: ar objektas (Baitas) pataikė į gaudyklės rėžius
                    if (obj_x >= (player_x - 5) && obj_x <= (player_x + player_w)) {
                        score++; // Pagavome!
                        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12); // Sumirksim žaliu LED (Sėkmė)
                    } else {
                        lives--; // Praleidome!
                        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14); // Sumirksim raudonu LED (Klaida)
                        if (lives <= 0) {
                            state = GAME_OVER;
                        }
                    }
                    Spawn_Object(); // Sukuriame naują krentantį objektą
                }
                needs_redraw = 1;
                last_fall_time = HAL_GetTick();
            }
        }

        // 3. EKRANO ATNAUJINIMAS (Piešiama TIK tada, kai pasikeičia būsena)
        if (needs_redraw) {
            ssd1306_Fill(Black); // Išvalome seną kadrą
            char buf[32]; // Buferis tekstui

            if (state == MENU) {
                ssd1306_SetCursor(15, 25);
                ssd1306_WriteString("PA0 to START", Font_7x10, White);
            }
            else if (state == GAME_OVER) {
                ssd1306_SetCursor(25, 15);
                ssd1306_WriteString("GAME OVER!", Font_7x10, White);
                sprintf(buf, "Score: %d", score);
                ssd1306_SetCursor(35, 40);
                ssd1306_WriteString(buf, Font_7x10, White);
            }
            else if (state == PLAYING) {
                // Statuso juosta viršuje (Score ir Lives)
                sprintf(buf, "Score:%d  Lives:%d", score, lives);
                ssd1306_SetCursor(0, 0);
                ssd1306_WriteString(buf, Font_7x10, White);

                // Gaudyklės piešimas pikseliais (storas blokas)
                for (int i = 0; i < 4; i++) {
                	for (int p = 0; p < player_w; p++) {
                		ssd1306_DrawPixel(player_x + p, player_y + i, White);
                	}
                }

                // Krentantis objektas (Simbolis 'B' - Baitas)
                ssd1306_SetCursor(obj_x, obj_y);
                ssd1306_WriteString("B", Font_7x10, White);
            }

            ssd1306_UpdateScreen(); // Išsiunčiame suformuotą kadrą į ekraną
            needs_redraw = 0; // Nuleidžiame vėliavėlę
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 8399;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE2 PE3 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD14 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
