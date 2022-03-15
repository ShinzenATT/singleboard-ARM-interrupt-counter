#define GPIO_D_BASE 0x40020C00
#define GPIO_D_MODER ((volatile unsigned long*)GPIO_D_BASE)
#define GPIO_D_ODR_LOW ((unsigned char*)GPIO_D_BASE + 0x14)

#define GPIO_E_BASE 0x40021000
#define GPIO_E_MODER ((volatile unsigned long*)GPIO_E_BASE)
#define GPIO_E_IDR_LOW ((unsigned char*)GPIO_E_BASE + 0x10)
#define GPIO_E_ODR_LOW ((unsigned char*)GPIO_E_BASE + 0x14)

#define SYSCFG 0x40013800
#define EXTICR1 ((unsigned long*)(SYSCFG + 0x8))

#define EXTI 0x40013C00
#define EXTI_IMR ((unsigned long*)EXTI)
#define EXTI_RTSR ((unsigned long*)(EXTI + 0x8))
#define EXTI_FTSR ((unsigned long*)(EXTI + 0xC))
#define EXTI_SWIER ((volatile unsigned long*)(EXTI + 0x10))
#define EXTI_PR ((volatile unsigned long*)(EXTI + 0x14))

#define NVIC 0xE000E100
#define NVIC_ISER0 ((unsigned long*)NVIC)

#define SCB_VTOR ((volatile unsigned long*)0xE000ED08)

/*
 * 	startup.c
 */
__attribute__((naked)) __attribute__((section(".start_section"))) void startup(void)
{
    __asm__ volatile(" LDR R0,=0x2001C000\n"); /* set stack */
    __asm__ volatile(" MOV SP,R0\n");
    __asm__ volatile(" BL main\n");   /* call main */
    __asm__ volatile(".L1: B .L1\n"); /* never return */
}

// Setup the GPIO D & E ports and the EXTI0-2 interupt handelers
void appInit();

// Stores the value that is altered by the EXTI0-2 interupts
static volatile unsigned int count;

void main(void)
{
    appInit();
	// This variable is used for debugging purposes
    unsigned int c;
	
	// Send the current count value to a hexdisplay
    while(1) {
	c = count;
	*GPIO_D_ODR_LOW = c;
    }
}

void appInit()
{
    *EXTICR1 &= ~0xFFF;
    *EXTICR1 |= 0x444;

    *EXTI_IMR |= 7;
    *EXTI_FTSR &= ~7;
    *EXTI_RTSR |= 7;
    //*EXTI_PR |= ~8;

	// Change vectortable adress
    unsigned long targetVektorAdr = 0x2001C000;
    *SCB_VTOR = targetVektorAdr;

    // Handeler for the IRQ0 interupt
    void exti0IrqHandeler();
    *((void (**)(void))0x2001C058) = exti0IrqHandeler;
	// Handeler for the IRQ1 interupt
    void exti1IrqHandeler();
    *((void (**)(void))0x2001C05C) = exti1IrqHandeler;
	// Handeler for the IRQ2 interupt
    void exti2IrqHandeler();
    *((void (**)(void))0x2001C060) = exti2IrqHandeler;

    *NVIC_ISER0 |= 0x1C0;
    //*EXTI_SWIER |= 8;

    *GPIO_D_MODER |= 0x55555555;
    *GPIO_E_MODER |= 0x1500;

    count = 0;
}

// Increments the counter when this handeler is run
void exti0IrqHandeler()
{
    count++;

    *EXTI_PR |= 1;
    *GPIO_E_ODR_LOW |= 0x10;
    *GPIO_E_ODR_LOW &= ~0x10;
}

// Sets the count to 0 when this handeler is run
void exti1IrqHandeler()
{
    count = 0;

    *EXTI_PR |= 2;
    *GPIO_E_ODR_LOW |= 0x20;
    *GPIO_E_ODR_LOW &= ~0x20;
}

// Toggles the counter between 0 and #FF when this handeler is run
void exti2IrqHandeler()
{
    if(count == 0) {
	count = 0xFF;
    } else {
	count = 0;
    }

    *EXTI_PR |= 4;
    *GPIO_E_ODR_LOW |= 0x40;
    *GPIO_E_ODR_LOW &= ~0x40;
}