• Paruoškite STM32F429I-DISC1 projektą žaidimui „Catch the Byte“. Naudokite integruotą TFT
ekraną, PA0 USER mygtuką kaip START/FIRE, o du išorinius mygtukus PE2 ir PE3 kaip LEFT
ir RIGHT. Išorinius mygtukus junkite tarp pasirinkto išvado ir GND, o GPIO konfigūruokite su
vidiniu Pull-Up rezistoriumi.

• Atlikite projekto konfigūravimą grafinėje sąsajoje: palikite įjungtus LTDC, FMC/SDRAM,
DMA2D, SPI5 ir LCD valdymo GPIO; PE2 ir PE3 konfigūruokite kaip GPIO input; PA0
naudokite USER mygtukui; PG13 ir PG14 naudokite būsenos indikacijai; žaidimo laiko bazei
naudokite HAL_GetTick arba TIM3 20 ms periodą.
- Ekrane turi būti rodoma žaidimo zona, žaidėjo gaudyklė apačioje, krentantis baitas arba
simbolis, rezultatas SCORE ir gyvybės LIVES.
- Žaidimo taisyklės: PE2 perkelia gaudyklę į kairę, PE3 - į dešinę, PA0 pradeda arba
sustabdo žaidimą. Kas 150 ms objektas pasislenka žemyn. Pagavus objektą pridedamas
1 taškas, praleidus objektą atimama 1 gyvybė. Privalomoje dalyje mygtukus skaitykite
polling būdu, naudodami HAL_GPIO_ReadPin, o ekraną atnaujinkite tik tada, kai
pasikeičia žaidimo būsena (3,5 balo).
- Rekomenduojamos funkcijos: HAL_GPIO_ReadPin, HAL_GetTick, HAL_Delay,
BSP_LCD_Clear, BSP_LCD_SetTextColor, BSP_LCD_DisplayStringAt ir paprastos
pagalbinės funkcijos objektų piešimui ekrane.

• Privalomos dalies pabaigoje pademonstruokite, kad žaidimas veikia be pertraukčių: mygtukai
reaguoja, objektas juda, rezultatas ir gyvybės atnaujinami ekrane.

• Tą patį žaidimą įgyvendinkite su pertrauktimis: PE2, PE3 ir PA0 apdorokite per EXTI, o objekto
judėjimą valdykite per TIM3 periodinę pertrauktį. Naudokite HAL_TIM_Base_Start_IT,
HAL_GPIO_EXTI_Callback ir HAL_TIM_PeriodElapsedCallback. Pertrauktyse keiskite tik
žaidimo būsenos kintamuosius, o LCD perpiešimą atlikite pagrindiniame cikle. Papildomai
didinkite žaidimo greitį kas 5 pagautus objektus ir ekrane rodykite HIGH SCORE reikšmę.
