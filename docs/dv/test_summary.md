# Verification Test Summary

## Overview
This document describes the cocotb-based verification tests created for the Caravel SPI User Project. The tests verify SPI controller functionality, GPIO pad connectivity, interrupt generation, and system integration.

## Test Infrastructure

### Virtual GPIO Model
- **File**: `VirtualGPIOModel.py`
- **Purpose**: Provides firmware-to-testbench synchronization through a virtual GPIO register at address `0x30FFFFFC`
- **Features**:
  - 16-bit output from firmware (bits [15:0])
  - 16-bit input to firmware (bits [31:16])
  - Milestone-based handshaking between C firmware and Python tests

### SPI Slave Model
- **File**: `SPISlaveModel.py`
- **Purpose**: Implements a cocotb-based SPI slave for loopback testing
- **Features**:
  - Command/address decoding (8-bit command + 24-bit address)
  - Read operation support (command 0x03)
  - Memory simulation with hex file loading
  - Configurable SPI mode (CPOL/CPHA)

## Test Cases

### Test 1: spi_test
**Location**: `verilog/dv/cocotb/spi_test/`

**Objective**: Verify SPI controller basic functionality with loopback data transfer

**Firmware Phases** (vgpio milestones):
1. `vgpio=1`: Firmware ready, GPIO pads configured
2. `vgpio=2`: SPI controller enabled and configured
   - CPOL=0, CPHA=0
   - Prescaler=4
   - TX/RX FIFO thresholds set
3. `vgpio=3`: Data transmission complete (8 bytes sent)
4. `vgpio=4`: Firmware verification PASSED / `vgpio=5`: FAILED
5. `vgpio=6`: SPI controller disabled

**Test Data**: `[0x55, 0xAA, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC]`

**Python Test Verification**:
- Monitors SPI signals on mprj_io[8:11]
- Implements SPI slave loopback (MOSI → MISO)
- Captures and validates 8 bytes of data
- Verifies firmware self-check result

**Expected Results**:
- All 8 bytes transmitted correctly on MOSI
- All 8 bytes received correctly on MISO
- Data integrity verified in firmware
- Proper SPI timing (clock, chip select)

---

### Test 2: system_test
**Location**: `verilog/dv/cocotb/system_test/`

**Objective**: Comprehensive system integration test including SPI, PIC, and interrupt handling

**Firmware Phases** (vgpio milestones):
1. `vgpio=1`: Firmware ready, GPIO pads configured
2. `vgpio=2`: PIC enabled, interrupts configured
   - Global interrupt enable set
   - SPI interrupt (IRQ0) enabled
3. `vgpio=3`: SPI controller enabled
   - TX empty interrupt enabled
4. `vgpio=4`: Data transmission complete (4 bytes sent)
5. `vgpio=5`: System verification PASSED / `vgpio=6`: FAILED
   - Data integrity check
   - Interrupt flag verification
6. `vgpio=7`: System disabled

**Test Data**: `[0xDE, 0xAD, 0xBE, 0xEF]`

**Python Test Verification**:
- Monitors SPI signals and user_irq[0]
- Implements SPI slave loopback
- Captures 4 bytes of data
- Monitors interrupt signal assertion
- Verifies firmware system check result

**Expected Results**:
- 4 bytes transmitted/received correctly
- PIC correctly routes SPI interrupt to user_irq[0]
- Firmware detects and acknowledges interrupt
- System operates without bus conflicts

---

## GPIO Pin Mapping

| Signal    | mprj_io Pin | Direction | Configuration              |
|-----------|-------------|-----------|----------------------------|
| SPI_MOSI  | [8]         | Output    | USER_STD_OUTPUT            |
| SPI_MISO  | [9]         | Input     | USER_STD_INPUT_NOPULL      |
| SPI_SCK   | [10]        | Output    | USER_STD_OUTPUT            |
| SPI_SS    | [11]        | Output    | USER_STD_OUTPUT            |

## Address Map

| Peripheral        | Base Address  | Size    | Description                    |
|-------------------|---------------|---------|--------------------------------|
| SPI Controller    | 0x3000_0000   | 64 KB   | CF_SPI_WB registers            |
| PIC               | 0x3001_0000   | 64 KB   | Interrupt controller registers |
| Virtual GPIO      | 0x30FF_FFFC   | 4 bytes | Test synchronization register  |

## Test Execution

### Prerequisites
1. Caravel cocotb environment setup:
   ```bash
   python verilog/dv/setup-cocotb.py /workspace/caravel_spi_project
   ```

2. Verify design_info.yaml configuration:
   - USER_PROJECT_ROOT: `/workspace/caravel_spi_project`
   - Clock period: 25 ns (40 MHz)

3. Ensure include files are properly configured:
   - `verilog/includes/includes.rtl.caravel_user_project`

### Running Tests

**Individual test execution**:
```bash
cd /workspace/caravel_spi_project/verilog/dv/cocotb
caravel_cocotb -t spi_test -tag spi_run
caravel_cocotb -t system_test -tag system_run
```

**Note**: Tests are provided but NOT executed as per user request. The user should run these commands when ready to verify the design.

## Expected Simulation Results

### Console Output Pattern
```
[TEST] start <test_name>
[GPIO MODEL] VirtualGPIOModel initialized
[TEST] Waiting for firmware ready signal (vgpio=1)
[GPIO MODEL] Write OUTPUT[15:0]: 0x0001
[TEST] Firmware ready
[TEST] Waiting for SPI peripheral enabled (vgpio=2)
[GPIO MODEL] Write OUTPUT[15:0]: 0x0002
...
[TEST] <test_name> PASSED
```

### Waveform Verification Points
- SPI CS (SS) assertion/deassertion timing
- SPI clock generation with correct prescaler
- MOSI data pattern matches test data
- MISO loopback data captured correctly
- user_irq[0] assertion (system_test only)

## Coverage

### Functional Coverage
- ✅ SPI GPIO pad connectivity
- ✅ SPI controller basic configuration
- ✅ SPI data transmission (TX FIFO)
- ✅ SPI data reception (RX FIFO)
- ✅ SPI loopback verification
- ✅ Interrupt generation (TX empty)
- ✅ PIC interrupt routing
- ✅ Wishbone bus arbitration (2 peripherals)
- ✅ Firmware-testbench synchronization

### Not Covered (Future Work)
- Multiple SPI modes (CPOL/CPHA combinations)
- SPI burst transfers
- FIFO overflow/underflow conditions
- Multiple interrupt sources
- Gate-level (GL) simulation
- SDF timing simulation

## Firmware APIs Used

### From `firmware_apis.h`
- `enableHkSpi(false)` - Disable housekeeping SPI
- `GPIOs_configure()` - Configure individual GPIO pads
- `GPIOs_loadConfigs()` - Apply GPIO configurations
- `User_enableIF()` - Enable Wishbone user interface

### From `CF_SPI.h` (CF_SPI IP)
- `CF_SPI_setGclkEnable()` - Enable SPI clock gating
- `CF_SPI_enable()` - Enable SPI controller
- `CF_SPI_disable()` - Disable SPI controller
- `CF_SPI_writePhase()` - Set clock phase (CPHA)
- `CF_SPI_writepolarity()` - Set clock polarity (CPOL)
- `CF_SPI_setPrescaler()` - Set clock prescaler
- `CF_SPI_enableRx()` - Enable RX functionality
- `CF_SPI_disableRx()` - Disable RX functionality
- `CF_SPI_assertCs()` - Assert chip select (active low)
- `CF_SPI_deassertCs()` - Deassert chip select
- `CF_SPI_writeData()` - Write byte to TX FIFO
- `CF_SPI_readData()` - Read byte from RX FIFO
- `CF_SPI_waitTxFifoEmpty()` - Wait until TX FIFO is empty
- `CF_SPI_waitRxFifoNotEmpty()` - Wait until RX FIFO has data
- `CF_SPI_readTxFifoEmpty()` - Check TX FIFO empty status
- `CF_SPI_readRxFifoEmpty()` - Check RX FIFO empty status
- `CF_SPI_setInterruptMask()` - Configure interrupt mask
- `CF_SPI_isBusy()` - Check if SPI is busy
- `CF_SPI_waitNotBusy()` - Wait until SPI is not busy
- `CF_SPI_FifoRxFlush()` - Flush RX FIFO

## Test Files Summary

| File                                    | Lines | Description                           |
|-----------------------------------------|-------|---------------------------------------|
| VirtualGPIOModel.py                     | 76    | Virtual GPIO synchronization model    |
| SPISlaveModel.py                        | 67    | SPI slave behavioral model            |
| spi_test/spi_test.c                     | 87    | SPI firmware test                     |
| spi_test/spi_test.py                    | 82    | SPI cocotb test                       |
| system_test/system_test.c               | 113   | System integration firmware           |
| system_test/system_test.py              | 99    | System integration cocotb test        |
| cocotb_tests.py                         | 2     | Test imports for runner               |

## Notes for Test Execution

1. **Timeout Configuration**: Tests use `timeout_cycles=1000000` to allow sufficient simulation time
2. **Clock Frequency**: All tests run at 40 MHz (25 ns period)
3. **Test Duration**: Each test expected to complete in < 10 minutes of simulation time
4. **Deterministic**: Tests use fixed test patterns for reproducible results
5. **Self-Checking**: Firmware performs data verification and reports pass/fail via vgpio

## Next Steps

1. Run RTL simulation tests to verify functionality
2. Fix any issues found during RTL verification
3. Update includes.gl.caravel_user_project for gate-level testing
4. Proceed to OpenLane hardening after passing all RTL tests
5. Re-run tests in GL mode after successful PnR

---

**Test Creation Date**: 2025-11-13  
**Design**: Caravel SPI User Project  
**Framework**: caravel-cocotb  
**Status**: Tests created, NOT executed (as per user request)
