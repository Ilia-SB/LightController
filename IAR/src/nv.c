#include "nv.h"

/***********\
 * Defines *
\***********/

#define FLASH_SIZE ((uint32_t)(256UL * 1024))
#define FLASH_BANK_SIZE ((uint32_t)(32UL * 1024))
#define FLASH_PAGE_SIZE ((uint32_t)(2UL * 1024))
#define FLASH_WORD_SIZE ((uint32_t)(4UL))
#define FLASH_BANK_PER_FLASH ((uint32_t)(256UL / 32))
#define FLASH_PAGE_PER_BANK ((uint32_t)(32UL / 2))
#define FLASH_BASE (0x8000)
#define FLASH_BANK_BASE(b) ((b) * FLASH_BANK_SIZE)
#define FLASH_PAGE_BASE(p) (FLASH_BASE + (p) * FLASH_PAGE_SIZE)
#define FLASH_PAGE(p) ((uint8_t*)FLASH_PAGE_BASE((p)))
#define FLASH_PAGE_SEQ_NUM(b, p) (((b) * FLASH_PAGE_PER_BANK) + (p))
#define FLASH_PAGE_ADDR(b, p) (((b) * FLASH_BANK_SIZE) + ((p) * FLASH_PAGE_SIZE))

/*********\
 * Types *
\*********/
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef struct dma_desc_s
{
     uint8 src_addr_h;
     uint8 src_addr_l;
     uint8 dst_addr_h;
     uint8 dst_addr_l;
     uint8 len_h:5;
     uint8 len_v:3;
     uint8 len_l;
     uint8 trig:5;
     uint8 tmode:2;
     uint8 word_size:1;
     uint8 priority:2;
     uint8 m8:1;
     uint8 irq_mask:1;
     uint8 dst_inc:2;
     uint8 src_inc:2;
} dma_desc_t;

/***************\
 * Definitions *
\***************/

void flash_page_read(uint8_t bank, 
    uint8_t page, 
    uint16_t offset,
    void *buf, 
    uint16_t len)
{
    uint16_t tmp = len;
    uint8_t *buf_rd = FLASH_PAGE(page) + offset,
    *buf_wr = (uint8_t*)buf;
    uint8 memctr = MEMCTR;
    MEMCTR = (MEMCTR & 0xf8) | bank;
    while(tmp--)
        *(buf_wr++) = *(buf_rd++);
    MEMCTR = memctr;
}

void flash_page_write(uint8_t bank, 
    uint8_t page, 
    uint16_t offset,
    void *buf, 
    uint16_t len)
{
    uint32_t flash_addr = (FLASH_PAGE_ADDR(bank, page) + offset) / FLASH_WORD_SIZE;
    dma_desc_t dma = 
    {
       .src_addr_h = ((uint16)buf >> 8) & 0x00ff,
       .src_addr_l = (uint16)buf & 0x00ff,
       .dst_addr_h = (((uint16) & FWDATA) >> 8) & 0x00ff,
       .dst_addr_l = ((uint16) & FWDATA) & 0x00ff,
       .len_v      = 0,  /* Use LEN for transfer count */
       .word_size  = 0,  /* Transfer a byte at a time. */
       .tmode      = 0,  /* Transfer a single byte/word after each DMA trigger. */
       .trig       = 18, /* Flash data write complete. */
       .src_inc    = 1,  /* Increment source pointer by 1 bytes/words after each transfer. */
       .dst_inc    = 0,  /* Increment destination pointer by 0 bytes/words after each transfer. */
       .irq_mask   = 0,  /* Disable interrupt generation. */
       .m8         = 0,  /* Use all 8 bits for transfer count. */
       .priority   = 2   /* High, DMA has priority. */
    };
    dma.len_h = (len >> 8) & 0x00ff;
    dma.len_l = len & 0x00ff;

    //BSP_DISABLE_INTERRUPTS();
    EA = 0;
    while(FCTL & 0x80);

    FADDRH = (flash_addr >> 8) & 0x00ff;
    FADDRL = (flash_addr) & 0x00ff;
    DMA0CFGH = (((uint16)&dma) >> 8) & 0x00ff;
    DMA0CFGL = ((uint16)&dma) & 0x00ff;
    DMAARM |= 0x01;
    FCTL |= 0x02; 

    while(!(DMAIRQ & 0x01)); //Wait Until Write Complete
    DMAIRQ &= 0xfe; //Clear Any DMA IRQ on Channel 0 - Bit 0

    while(FCTL & (0x80));
    //BSP_ENABLE_INTERRUPTS();
    EA = 1;
}

void flash_page_erase(uint8_t bank, 
    uint8_t page)
{
    BSP_DISABLE_INTERRUPTS();
    while(FCTL & 0x80);

    FADDRH = FLASH_PAGE_SEQ_NUM(bank, page) << 1;
    FCTL |= 0x01;

    while(FCTL & (0x80));
    BSP_ENABLE_INTERRUPTS();
}