#include "sim300.h"
#include "board.h"

void USART1_IRQHandler(void);
void USART3_IRQHandler(void);
void at_send(char* buff, uint16_t length);

#define AT_BUFFER_SIZE 2048

char at_buffer_rx[AT_BUFFER_SIZE];
static uint32_t at_cnt = 0;

void USART1_IRQHandler()
{
	
    if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
    {
        
    }
    else
    {
        USART_ClearFlag(USART1, USART_FLAG_TC);
        USART_ClearFlag(USART1, USART_FLAG_TXE);
    }
}

void USART3_IRQHandler()
{
	
    if(USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET)
    {
        uint8_t value = USART_ReceiveData(USART3);
        
        at_buffer_rx[at_cnt] = value;
        USART_ClearFlag(USART3, USART_FLAG_RXNE);
        
        if(++at_cnt >= AT_BUFFER_SIZE)
        {
            at_cnt = 0;
        }
        
    }
    else
    {
        USART_ClearFlag(USART1, USART_FLAG_TC);
        USART_ClearFlag(USART1, USART_FLAG_TXE);
    }
}

void at_send(char* buff, uint16_t length)
{
    DMA_InitTypeDef DMA_InitStructure;
    
    DMA_ClearFlag(DMA1_FLAG_TC4 | DMA1_FLAG_TE4 | DMA1_FLAG_HT4 | DMA1_FLAG_GL4);
    USART_ClearFlag(USART3, USART_FLAG_TC);
    USART_ClearFlag(USART3, USART_FLAG_TXE);

    DMA_ClearITPendingBit(DMA1_IT_GL4);

    DMA_Cmd(DMA1_Channel2, DISABLE);
    
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)buff;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = length;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel2, &DMA_InitStructure);

    DMA_Cmd(DMA1_Channel2, ENABLE);
}


void main()
{
    board_init();
    at_send("AT+ECHO\r\n", sizeof("AT+ECHO\r\n") - 1);
    at_send("AT+ECHO\r\n", sizeof("AT+ECHO\r\n") - 1);
    while(1){}
}