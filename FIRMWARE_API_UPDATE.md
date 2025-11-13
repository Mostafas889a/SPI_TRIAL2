# Firmware API Update - CF_SPI.h Functions

## Summary
All test firmware files have been updated to use the proper CF_SPI.h API functions from the CF_SPI v2.0.1 IP library.

---

## Changes Made

### 1. Include Statement
**Before:**
```c
#include "CF_SPI.h"
```

**After:**
```c
#include <CF_SPI.h>
```

### 2. SPI Configuration Functions

#### CPHA (Clock Phase) Setting
**Before:**
```c
CF_SPI_setCPHA(SPI_BASE, 0);
```

**After:**
```c
CF_SPI_writePhase(SPI_BASE, false);
```

#### CPOL (Clock Polarity) Setting
**Before:**
```c
CF_SPI_setCPOL(SPI_BASE, 0);
```

**After:**
```c
CF_SPI_writepolarity(SPI_BASE, false);
```

#### RX Enable
**Added:**
```c
CF_SPI_enableRx(SPI_BASE);
```

#### Chip Select Control
**Added:**
```c
CF_SPI_assertCs(SPI_BASE);    // Assert CS before transfer
CF_SPI_deassertCs(SPI_BASE);  // Deassert CS after transfer
```

### 3. Data Transfer Functions

#### Write Data
**Before:**
```c
CF_SPI_write(SPI_BASE, test_data[i]);
```

**After:**
```c
CF_SPI_writeData(SPI_BASE, test_data[i]);
```

#### Read Data
**No change needed** (was already correct):
```c
CF_SPI_readData(SPI_BASE);
```

### 4. FIFO Management Functions

#### TX FIFO Wait
**Before:**
```c
while (!CF_SPI_txFIFOEmpty(SPI_BASE));
```

**After:**
```c
CF_SPI_waitTxFifoEmpty(SPI_BASE);
```

#### RX FIFO Wait
**Before:**
```c
while (CF_SPI_getRxFIFOLevel(SPI_BASE) < 8);
```

**After:**
```c
for (int i = 0; i < 8; i++) {
    CF_SPI_waitRxFifoNotEmpty(SPI_BASE);
    rx_data[i] = CF_SPI_readData(SPI_BASE);
}
```

### 5. Interrupt Configuration (system_test only)

#### Interrupt Mask
**Before:**
```c
CF_SPI_enableInterrupt(SPI_BASE, CF_SPI_IM_TXEIM_MASK);
```

**After:**
```c
CF_SPI_setInterruptMask(SPI_BASE, 0x01);
```

### 6. FIFO Threshold Functions
**Removed** (not available in CF_SPI.h):
```c
CF_SPI_setTxFIFOThreshold(SPI_BASE, 1);  // REMOVED
CF_SPI_setRxFIFOThreshold(SPI_BASE, 1);  // REMOVED
```

---

## Complete CF_SPI.h API Reference

### Clock & Enable Functions
- `void CF_SPI_setGclkEnable(uint32_t spi_base, int value)` - Enable/disable clock gating
- `void CF_SPI_enable(uint32_t spi_base)` - Enable SPI controller
- `void CF_SPI_disable(uint32_t spi_base)` - Disable SPI controller

### Configuration Functions
- `void CF_SPI_writepolarity(uint32_t spi_base, bool polarity)` - Set CPOL
- `void CF_SPI_writePhase(uint32_t spi_base, bool phase)` - Set CPHA
- `void CF_SPI_setPrescaler(uint32_t spi_base, uint32_t pr_value)` - Set clock prescaler
- `void CF_SPI_enableRx(uint32_t spi_base)` - Enable RX functionality
- `void CF_SPI_disableRx(uint32_t spi_base)` - Disable RX functionality

### Data Transfer Functions
- `void CF_SPI_writeData(uint32_t spi_base, int data)` - Write byte to TX FIFO
- `int CF_SPI_readData(uint32_t spi_base)` - Read byte from RX FIFO

### Chip Select Control
- `void CF_SPI_assertCs(uint32_t spi_base)` - Assert chip select (active)
- `void CF_SPI_deassertCs(uint32_t spi_base)` - Deassert chip select (inactive)

### FIFO Status & Wait Functions
- `int CF_SPI_readTxFifoEmpty(uint32_t spi_base)` - Check if TX FIFO is empty
- `int CF_SPI_readRxFifoEmpty(uint32_t spi_base)` - Check if RX FIFO is empty
- `void CF_SPI_waitTxFifoEmpty(uint32_t spi_base)` - Wait until TX FIFO is empty
- `void CF_SPI_waitRxFifoNotEmpty(uint32_t spi_base)` - Wait until RX FIFO has data
- `void CF_SPI_FifoRxFlush(uint32_t spi_base)` - Flush RX FIFO

### Interrupt Functions
- `void CF_SPI_setInterruptMask(uint32_t spi_base, int mask)` - Configure interrupt mask

### Status Functions
- `int CF_SPI_isBusy(uint32_t spi_base)` - Check if SPI is busy
- `void CF_SPI_waitNotBusy(uint32_t spi_base)` - Wait until SPI is not busy

---

## Files Updated

1. **`verilog/dv/cocotb/spi_test/spi_test.c`** (87 lines)
   - Updated SPI configuration functions
   - Added chip select control
   - Changed to proper FIFO wait functions
   - Removed non-existent threshold functions

2. **`verilog/dv/cocotb/system_test/system_test.c`** (112 lines)
   - Same updates as spi_test
   - Updated interrupt configuration function

3. **`docs/dv/test_summary.md`**
   - Updated API reference section
   - Added complete list of CF_SPI.h functions

4. **`DELIVERY_SUMMARY.md`**
   - Updated test descriptions
   - Added API usage notes

5. **`README.md`**
   - Added update notification

---

## Benefits of Using CF_SPI.h APIs

1. **Correctness**: Functions match the actual CF_SPI IP implementation
2. **Maintainability**: Code uses vendor-provided API functions
3. **Portability**: Easier to update if IP version changes
4. **Robustness**: Wait functions provide proper synchronization
5. **Completeness**: All necessary hardware features are accessible

---

## Testing Status

**Tests have been updated but NOT executed** as per user request.

To run the tests:
```bash
cd /workspace/caravel_spi_project/verilog/dv/cocotb
caravel_cocotb -t spi_test -tag spi_run
caravel_cocotb -t system_test -tag system_run
```

---

## IP Library Reference

- **IP Name**: CF_SPI
- **Version**: v2.0.1
- **Source**: NativeChips verified IP library
- **Location**: `/nc/ip/CF_SPI/v2.0.1/`
- **Firmware API**: `/nc/ip/CF_SPI/v2.0.1/fw/CF_SPI.h`
- **Implementation**: `/nc/ip/CF_SPI/v2.0.1/fw/CF_SPI.c`
- **Registers**: `/nc/ip/CF_SPI/v2.0.1/fw/CF_SPI_regs.h`

---

**Document Generated**: 2025-11-13  
**Agent**: CodeActRTLAgent (Verification_Agent)  
**Project**: Caravel SPI User Project
