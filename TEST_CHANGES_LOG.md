# Test Changes Documentation

**Date**: 2025-11-13  
**Status**: Tests Modified and Executed  
**Result**: Test Run Completed (1 test executed, identified data mismatch issue)

---

## Changes Made to Tests

### 1. **spi_test.c** (Firmware)

#### Line 68-76: Added Expected RX Data Pattern and Validation
```c
// NEW: Define expected received data pattern
uint8_t test_rxdata[8] = {0x66, 0xBB, 0x23, 0x42, 0x78, 0xab, 0xbb, 0xCF};

// NEW: Validation logic with per-byte output
int pass = 1;
for (int i = 0; i < 8; i++) {
    vgpio_write_output(rx_data[i]);  // Output each received byte to vgpio
    if (rx_data[i] != test_rxdata[i]) {
        vgpio_write_output(0XEEEE);   // Error code on mismatch
        break;
    }
}
```

**Purpose**: 
- Validate received data against expected pattern
- Output each received byte to virtual GPIO for debugging
- Signal error (0xEEEE) if data mismatch occurs

#### Line 34-35: Added GPIO Comments
```c
GPIOs_configure(10, GPIO_MODE_USER_STD_OUTPUT);  // clk
GPIOs_configure(11, GPIO_MODE_USER_STD_OUTPUT);  // ss 
```

**Purpose**: Document GPIO pin assignments

---

### 2. **spi_test.py** (Python Testbench)

#### Line 37-38: Enable MISO GPIO Input
```python
caravelEnv.dut.gpio9_en.value  = 1  # enable gpio input
spi_miso.value = 0
```

**Purpose**: Enable GPIO9 as input to drive MISO signal from testbench

#### Line 31: Updated Test Data Pattern
```python
# Original pattern (kept for TX verification)
test_data = [0x55, 0xAA, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC]
```

#### Line 54-65: Added SPI MISO Data Transmission
```python
# NEW: Define loopback response pattern
test_data = [0x66, 0xBB, 0x23, 0x42, 0x78, 0xab, 0xbb, 0xCF]

# NEW: Drive MISO line with test pattern
for byte_idx in test_data:
    byte_val = bin(byte_idx)[2:].zfill(8)
    cocotb.log.info(f"byte val = {byte_val}")
    for bit_idx in range(8):
        await FallingEdge(spi_sck)
        if byte_val[bit_idx] == "1":
            spi_miso.value = 1
        else:
            spi_miso.value = 0
```

**Purpose**: 
- Simulate SPI slave response by driving MISO line
- Send back a different pattern to verify full-duplex operation
- Clock data out on falling edge of SCK (standard SPI mode 0)

#### Line 72-76: Updated Data Validation
```python
# NEW: Verify received data matches transmitted pattern
for i in range(8):
    if rx_data[i] != test_data[i]:
        cocotb.log.error(f"[TEST] Data mismatch at byte {i}: expected 0x{test_data[i]:02X}, got 0x{rx_data[i]:02X}")
        assert False, f"Data mismatch at byte {i}"
```

**Purpose**: Compare received MOSI data with expected TX pattern

---

## Test Execution Results

### Test Run: RTL-spi_test
- **Start**: 09:37:32 (Thu)
- **End**: 09:38:49 (Thu)  
- **Duration**: 76.30 seconds
- **Status**: ❌ **FAILED**
- **Seed**: 1763055454

### Test Flow (Observed in Logs)

| Time (ns) | Event | Details |
|-----------|-------|---------|
| 862,525 | Firmware Ready | vgpio=1 |
| 1,473,725 | SPI Enabled | vgpio=2, CS asserted |
| 1,585,000 - 2,406,800 | TX Phase | Received 8 bytes: 0x55, 0xAA, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC ✅ |
| 2,550,650 | TX Complete | vgpio=3 |
| 2,634,750 - 3,865,350 | RX Phase | Testbench drives MISO with pattern: 0x66, 0xBB, 0x23, 0x42, 0x78, 0xAB, 0xBB, 0xCF |
| 4,215,250 | Firmware Reads RX | Outputs rx_data[0] = 0x00 (not 0x66) |
| 4,299,150 | **Error Code** | Firmware outputs **0xEEEE** - data mismatch detected |
| 4,557,350 | Test End | vgpio=6 |

---

## Issue Identified

### Root Cause: SPI RX FIFO Not Receiving Data

**Problem**: 
- Firmware successfully transmitted 8 bytes (MOSI verified ✅)
- Python testbench drove MISO with response pattern
- **Firmware RX FIFO remained empty** - read data was 0x00 instead of 0x66
- Error code 0xEEEE was correctly triggered

**Possible Causes**:
1. **GPIO9 (MISO) not properly configured as input** to user_project
2. **SPI controller RX path issue** - MISO signal not connected to SPI controller input
3. **Timing issue** - MISO data setup/hold timing relative to SCK
4. **Test sequence** - RX happens during second loop, but FIFO may have been flushed

### Evidence from Logs:
```
4215250.00ns INFO     [GPIO MODEL] Write OUTPUT[15:0]: 0x0000  # rx_data[0] = 0x00 (WRONG)
4299150.00ns INFO     [GPIO MODEL] Write OUTPUT[15:0]: 0xeeee  # Error detected by firmware
4299150.00ns ERROR    [GPIO MODEL] received wrong code 0xeeee at virtual gpio
```

---

## Test Design: Expected Behavior

### Full-Duplex SPI Transaction

**Phase 1: Master TX (Firmware writes, testbench reads MOSI)**
- Firmware writes: `[0x55, 0xAA, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC]`
- Testbench reads MOSI ✅ **WORKING**

**Phase 2: Master RX (Testbench drives MISO, firmware reads)**
- Testbench drives MISO: `[0x66, 0xBB, 0x23, 0x42, 0x78, 0xAB, 0xBB, 0xCF]`
- Firmware reads RX FIFO ❌ **NOT WORKING** (reads 0x00)

---

## Current Test Code Status

### ✅ Working Components:
1. Firmware GPIO configuration
2. CF_SPI API usage (correct functions)
3. SPI TX path (MOSI output verified)
4. Python testbench MOSI monitoring
5. Error detection logic (0xEEEE code works)
6. Virtual GPIO communication

### ❌ Not Working:
1. SPI RX path (MISO → RX FIFO)
2. Data loopback verification

---

## Files Modified

1. **verilog/dv/cocotb/spi_test/spi_test.c** (87 lines)
   - Added expected RX data array
   - Added per-byte validation with vgpio output
   - Added error code (0xEEEE) on mismatch
   - Added GPIO pin comments

2. **verilog/dv/cocotb/spi_test/spi_test.py** (78 lines)
   - Enabled GPIO9 input for MISO
   - Added MISO data transmission loop
   - Updated validation to check both TX and RX

---

## Recommendations for Fixing

### 1. Check GPIO9 Configuration
Verify in `user_project_wrapper.v` that GPIO9 is properly connected as input:
```verilog
assign spi_miso = mprj_io_in[9];  // Should read FROM pad
```

### 2. Check user_project.v
Verify SPI controller MISO port is connected:
```verilog
.miso(spi_miso),  // Should be input to SPI controller
```

### 3. Check SPI Mode Configuration
Ensure CPHA/CPOL settings match between firmware and testbench:
- Current: CPOL=0, CPHA=0 (Mode 0)
- Data sampled on rising edge
- MISO should be driven on falling edge ✅ (testbench does this)

### 4. Add Waveform Debug
```bash
# View waveforms to debug MISO path
gtkwave verilog/dv/cocotb/sim/spi_test/RTL-spi_test/spi_test.vcd
```

Look for:
- `gpio9` signal
- `user_project.spi_miso` 
- `spi_controller.miso`
- `spi_controller.RX FIFO` writes

---

## Next Steps

1. ✅ **Document changes** (this file)
2. ✅ **Upload to repository** (SPI_TRIAL2)
3. ⏭️ **Debug MISO path** - Check GPIO9 → SPI controller connectivity
4. ⏭️ **Fix RX data path** - Ensure MISO signal reaches SPI controller
5. ⏭️ **Re-run test** - Verify full-duplex operation

---

## Test Statistics

- **Total Tests**: 1
- **Passed**: 0
- **Failed**: 1
- **Duration**: 76.35 seconds
- **Simulation Time**: 4.557 ms

---

**Summary**: Tests have been enhanced with full-duplex SPI verification including MISO loopback. The test successfully verifies TX (MOSI) path but identified an issue with RX (MISO) path - firmware RX FIFO not receiving data from GPIO9/MISO signal. Further debugging of the MISO signal path is required.
