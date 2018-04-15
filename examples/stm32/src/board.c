#include "board.h"

void board_uart(void);
void board_uart2(void);
void board_uart3(void);
void board_vs1003_init(void);
void board_button_init(void);
void board_port1_init(void);
void init_led_pwm(void);

void board_vs1003_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef   SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = _PIN(PIN_SCLK) | _PIN(PIN_SI) | _PIN(PIN_SO);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(_PORT(PORT_SPI), &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = _PIN(PIN_CS);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(_PORT(PORT_CS), &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = _PIN(PIN_DREQ);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(_PORT(PORT_DREQ), &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = _PIN(PIN_XDCS);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(_PORT(PORT_XDCS), &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = _PIN(PIN_XRESET);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(_PORT(PORT_XRESET), &GPIO_InitStructure);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;

	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;

	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;

	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_Cmd(SPI1, ENABLE);


}

void board_uart()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    
    USART_InitStruct.USART_BaudRate = 921600;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    
    USART_Init(USART1, &USART_InitStruct);
    USART_Cmd(USART1, ENABLE);
    
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 14;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);

}

void board_uart2()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    
    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    
    USART_Init(USART2, &USART_InitStruct);
    USART_Cmd(USART2, ENABLE);
    
    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 14;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);

}

void board_uart3()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    
    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    
    USART_Init(USART3, &USART_InitStruct);
    USART_Cmd(USART3, ENABLE);
    
    NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 14;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);

}

void board_button_init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = _PIN(PIN_BTN_1) | _PIN(PIN_BTN_2) | _PIN(PIN_BTN_3);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(_PORT(PORT_BTN_1), &GPIO_InitStructure);
}

void board_port1_init()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
	/*fmcs*/
	
	GPIO_InitStruct.GPIO_Pin = _PIN(PIN_TFT_DB10) | 
							   _PIN(PIN_TFT_DB11) |
							   _PIN(PIN_TFT_DB12) |
							   _PIN(PIN_TFT_DB13) |
							   _PIN(PIN_TFT_DB14) |
							   _PIN(PIN_TFT_DB15) |
							   _PIN(PIN_TFT_DB16) |
							   _PIN(PIN_TFT_DB17) ;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(_PORT(PORT_TFT_DB), &GPIO_InitStruct);
	GPIO_SetBits(_PORT(PORT_TFT_DB), 0xFF00);
    
    GPIO_InitStruct.GPIO_Pin = _PIN(PIN_TFT_RESET);
	GPIO_Init(_PORT(PORT_TFT_RESET), &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = _PIN(PIN_TFT_NE1);
	GPIO_Init(_PORT(PORT_TFT_NE1), &GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin = _PIN(PIN_TFT_CLE_A16);
	GPIO_Init(_PORT(PORT_TFT_CLE_A16), &GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin = _PIN(PIN_TFT_NWE);
	GPIO_Init(_PORT(PORT_TFT_NWE), &GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin = _PIN(PIN_TFT_NOE);
	GPIO_Init(_PORT(PORT_TFT_NOE), &GPIO_InitStruct);
    
	
	//GPIO_SetBits(_PORT(PORT_TFT_NE1), _PIN(PIN_TFT_NE1));
	GPIO_SetBits(_PORT(PORT_TFT_CLE_A16), _PIN(PIN_TFT_CLE_A16));
	GPIO_SetBits(_PORT(PORT_TFT_NWE), _PIN(PIN_TFT_NWE));
	GPIO_SetBits(_PORT(PORT_TFT_NOE), _PIN(PIN_TFT_NOE));
	GPIO_SetBits(_PORT(PORT_TFT_RESET), _PIN(PIN_TFT_RESET));
	//GPIO_ResetBits(_PORT(PORT_TFT_RESET), _PIN(PIN_TFT_RESET));
    
    GPIO_ResetBits(_PORT(PORT_TFT_DB), _PIN(PIN_TFT_DB13));
    GPIO_ResetBits(_PORT(PORT_TFT_DB), _PIN(PIN_TFT_DB14));
    GPIO_ResetBits(_PORT(PORT_TFT_DB), _PIN(PIN_TFT_DB15));
}

void init_led_pwm()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(_PORT(PORT_TFT_DB), &GPIO_InitStruct);
	
    
    
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 720;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = 1000;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStruct);
    
    TIM_SelectOnePulseMode(TIM1, TIM_OPMode_Repetitive);
    
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_OutputNState = TIM_OutputNState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 10;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low    ;
    TIM_OCInitStruct.TIM_OCNPolarity = TIM_OCNPolarity_Low;
    TIM_OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OCInitStruct.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    
    TIM_OCInitStruct.TIM_Pulse = 0;
    TIM_OC1Init(TIM1, &TIM_OCInitStruct);
    TIM_OCInitStruct.TIM_Pulse = 0;
    TIM_OC2Init(TIM1, &TIM_OCInitStruct);
    TIM_OCInitStruct.TIM_Pulse = 10;
    TIM_OC3Init(TIM1, &TIM_OCInitStruct);
    
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    
    TIM_ARRPreloadConfig(TIM1, ENABLE);
    
    TIM_Cmd(TIM1, ENABLE);
}

void board_init()
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
	board_vs1003_init();
	board_button_init();
    board_uart();
    board_uart2();
    board_uart3();
    //board_port1_init();
    init_led_pwm();
}