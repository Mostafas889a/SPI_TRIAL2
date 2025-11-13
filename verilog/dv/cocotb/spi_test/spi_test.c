#include <firmware_apis.h>
#include <CF_SPI.h>

#define VGPIO_REG_ADDR 0x30FFFFFC

#define SPI_BASE 0x30000000

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
    GPIOs_configure(10, GPIO_MODE_USER_STD_OUTPUT);  // clk
    GPIOs_configure(11, GPIO_MODE_USER_STD_OUTPUT);  // ss 
    GPIOs_loadConfigs();
    
    User_enableIF();
    
    vgpio_write_output(1);
    
    CF_SPI_setGclkEnable(SPI_BASE, 1);
    CF_SPI_enable(SPI_BASE);
    CF_SPI_writePhase(SPI_BASE, false);
    CF_SPI_writepolarity(SPI_BASE, false);
    CF_SPI_setPrescaler(SPI_BASE, 4);
    CF_SPI_enableRx(SPI_BASE);
    CF_SPI_assertCs(SPI_BASE);
    
    vgpio_write_output(2);
    
    uint8_t test_data[8] = {0x55, 0xAA, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};
    
    for (int i = 0; i < 8; i++) {
        CF_SPI_writeData(SPI_BASE, test_data[i]);
    }
    
    CF_SPI_waitTxFifoEmpty(SPI_BASE);
    
    vgpio_write_output(3);
    
    uint8_t rx_data[8];
    for (int i = 0; i < 8; i++) {
        CF_SPI_writeData(SPI_BASE, 0x0);
        CF_SPI_waitRxFifoNotEmpty(SPI_BASE);
        rx_data[i] = CF_SPI_readData(SPI_BASE);
    }
    uint8_t test_rxdata[8] = {0x66, 0xBB, 0x23, 0x42, 0x78, 0xab, 0xbb, 0xCF};
    int pass = 1;
    for (int i = 0; i < 8; i++) {
        vgpio_write_output(rx_data[i]);
        if (rx_data[i] != test_rxdata[i]) {
            vgpio_write_output(0XEEEE);
            break;
        }
    }
    
    
    CF_SPI_deassertCs(SPI_BASE);
    CF_SPI_disable(SPI_BASE);
    
    vgpio_write_output(6);
}
