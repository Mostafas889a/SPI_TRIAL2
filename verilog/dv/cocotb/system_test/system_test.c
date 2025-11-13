#include <firmware_apis.h>
#include <CF_SPI.h>

#define VGPIO_REG_ADDR 0x30FFFFFC

#define SPI_BASE 0x30000000
#define PIC_BASE 0x30010000

#define PIC_GIER_REG     (PIC_BASE + 0x00)
#define PIC_IER_REG      (PIC_BASE + 0x04)
#define PIC_ISR_REG      (PIC_BASE + 0x0C)

void vgpio_write_output(uint16_t value)
{
    volatile uint32_t *vgpio_reg = (volatile uint32_t *)VGPIO_REG_ADDR;
    uint32_t reg_val = *vgpio_reg;
    reg_val = (reg_val & 0xFFFF0000) | (value & 0xFFFF);
    *vgpio_reg = reg_val;
}

uint16_t vgpio_read_input(void)
{
    volatile uint32_t *vgpio_reg = (volatile uint32_t *)VGPIO_REG_ADDR;
    uint32_t reg_val = *vgpio_reg;
    return (uint16_t)((reg_val >> 16) & 0xFFFF);
}

void vgpio_wait_val(uint16_t val)
{
    while (vgpio_read_input() != val);
}

void main(void)
{
    enableHkSpi(false);
    
    GPIOs_configure(8, GPIO_MODE_USER_STD_OUTPUT);
    GPIOs_configure(9, GPIO_MODE_USER_STD_INPUT_NOPULL);
    GPIOs_configure(10, GPIO_MODE_USER_STD_OUTPUT);
    GPIOs_configure(11, GPIO_MODE_USER_STD_OUTPUT);
    GPIOs_loadConfigs();
    
    User_enableIF();
    
    vgpio_write_output(1);
    
    volatile uint32_t *gier = (volatile uint32_t *)PIC_GIER_REG;
    volatile uint32_t *ier = (volatile uint32_t *)PIC_IER_REG;
    volatile uint32_t *isr = (volatile uint32_t *)PIC_ISR_REG;
    
    *gier = 0x80000000;
    *ier = 0x0001;
    
    vgpio_write_output(2);
    
    CF_SPI_setGclkEnable(SPI_BASE, 1);
    CF_SPI_enable(SPI_BASE);
    CF_SPI_writePhase(SPI_BASE, false);
    CF_SPI_writepolarity(SPI_BASE, false);
    CF_SPI_setPrescaler(SPI_BASE, 4);
    CF_SPI_enableRx(SPI_BASE);
    
    CF_SPI_setInterruptMask(SPI_BASE, 0x01);
    
    CF_SPI_assertCs(SPI_BASE);
    
    vgpio_write_output(3);
    
    uint8_t test_data[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    
    for (int i = 0; i < 4; i++) {
        CF_SPI_writeData(SPI_BASE, test_data[i]);
    }
    
    CF_SPI_waitTxFifoEmpty(SPI_BASE);
    
    vgpio_write_output(4);
    
    uint8_t rx_data[4];
    for (int i = 0; i < 4; i++) {
        CF_SPI_waitRxFifoNotEmpty(SPI_BASE);
        rx_data[i] = CF_SPI_readData(SPI_BASE);
    }
    
    int pass = 1;
    for (int i = 0; i < 4; i++) {
        if (rx_data[i] != test_data[i]) {
            pass = 0;
            break;
        }
    }
    
    uint32_t isr_val = *isr;
    if (isr_val & 0x0001) {
        *isr = 0x0001;
        pass = pass && 1;
    } else {
        pass = 0;
    }
    
    if (pass) {
        vgpio_write_output(5);
    } else {
        vgpio_write_output(6);
    }
    
    CF_SPI_deassertCs(SPI_BASE);
    CF_SPI_disable(SPI_BASE);
    *gier = 0x00000000;
    
    vgpio_write_output(7);
}
