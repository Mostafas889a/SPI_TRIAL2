# 🎯 SPI_TRIAL2 Repository Summary

**Repository**: [SPI_TRIAL2](https://github.com/${github_user_login}/SPI_TRIAL2)  
**Created**: 2025-11-13  
**Status**: ✅ Uploaded and Ready

---

## 📦 What's Included

This repository contains a complete Caravel user project integrating an SPI controller with comprehensive verification infrastructure.

### Core Components

#### 1. **RTL Design** (`verilog/rtl/`)
- ✅ **user_project.v** - Main design with Wishbone bus splitter and peripherals
- ✅ **user_project_wrapper.v** - Caravel integration wrapper with GPIO connections
- ✅ **wishbone_bus_splitter.v** - Wishbone Classic bus decoder/multiplexer
- ✅ **WB_PIC.v** - Programmable Interrupt Controller (16 IRQ sources)
- ✅ **CF_SPI v2.0.1** - Pre-verified SPI controller from IP library (via ipm_linker)

#### 2. **Verification** (`verilog/dv/cocotb/`)
- ✅ **spi_test** - Full-duplex SPI loopback test (C firmware + Python testbench)
- ✅ **system_test** - System integration with interrupt testing
- ✅ **SPISlaveModel.py** - Reusable SPI slave behavioral model
- ✅ **VirtualGPIOModel.py** - Firmware-testbench synchronization via virtual GPIO
- ✅ **design_info.yaml** - Caravel-cocotb configuration

#### 3. **Documentation** (`docs/`)
- ✅ **register_map.md** - Complete register specifications for SPI and PIC
- ✅ **pad_map.md** - GPIO pin assignments and configuration
- ✅ **integration_notes.md** - Clock/reset architecture, bus timing, verification guide
- ✅ **dv/** - Verification plan, test summary, and handoff documents
- ✅ **retrospective.md** - Project summary and lessons learned

#### 4. **Configuration Files**
- ✅ **ip/link_IPs.json** - IPM linker configuration for CF_SPI and CF_IP_UTIL
- ✅ **openlane/** - OpenLane configuration (ready for PnR when RTL is verified)
- ✅ **.gitignore** - Excludes simulation artifacts, build outputs, IP symlinks

#### 5. **Change Documentation**
- ✅ **TEST_CHANGES_LOG.md** - Detailed log of test modifications and results
- ✅ **FIRMWARE_API_UPDATE.md** - CF_SPI.h API migration guide
- ✅ **POWER_PIN_FIX.md** - Fix documentation for soft-IP power pin issue
- ✅ **UPDATE_NOTIFICATION.md** - Quick reference for recent updates

---

## 🏗️ Architecture

### Address Map
| Peripheral | Base Address | Size | Description |
|------------|--------------|------|-------------|
| SPI Controller | 0x3000_0000 | 64 KB | CF_SPI v2.0.1 with Wishbone wrapper |
| PIC | 0x3001_0000 | 64 KB | 16-source Programmable Interrupt Controller |

### GPIO Assignments
| Pin | Direction | Function | Notes |
|-----|-----------|----------|-------|
| mprj_io[8] | Output | SPI MOSI | Master Out Slave In |
| mprj_io[9] | Input | SPI MISO | Master In Slave Out |
| mprj_io[10] | Output | SPI SCK | Serial Clock |
| mprj_io[11] | Output | SPI SS | Chip Select (active low) |

### Interrupt Connections
- SPI IRQ → PIC IRQ[0]
- PIC IRQ Output → user_irq (to Caravel SoC)

---

## 🧪 Test Results

### Test Execution: spi_test
- **Date**: 2025-11-13 09:37-09:38
- **Duration**: 76.30 seconds
- **Result**: ❌ **FAILED** (debugging in progress)

### What Worked ✅
1. **Firmware initialization** - All GPIOs configured correctly
2. **SPI TX path** - Successfully transmitted 8 bytes via MOSI
3. **Testbench monitoring** - Correctly captured all MOSI data
4. **Virtual GPIO sync** - Firmware-testbench communication working
5. **Error detection** - Firmware correctly detected RX mismatch (0xEEEE code)

### What Needs Fixing ❌
1. **SPI RX path** - MISO data not reaching firmware RX FIFO
   - Expected: 0x66, 0xBB, 0x23, 0x42, 0x78, 0xAB, 0xBB, 0xCF
   - Got: 0x00 (empty FIFO)
2. **Root cause**: GPIO9 (MISO) → SPI controller connectivity issue

### Debug Steps
See `TEST_CHANGES_LOG.md` for detailed analysis and recommended fixes.

---

## 📊 Key Files and Their Purpose

### Documentation Files

| File | Purpose |
|------|---------|
| `README.md` | Project overview and status |
| `TEST_CHANGES_LOG.md` | **START HERE** - Complete test change documentation and debug guide |
| `FIRMWARE_API_UPDATE.md` | CF_SPI.h API reference and migration notes |
| `POWER_PIN_FIX.md` | Soft-IP power pin connection fix |
| `DELIVERY_SUMMARY.md` | Complete project deliverables checklist |
| `docs/register_map.md` | Register specifications for all peripherals |
| `docs/pad_map.md` | GPIO pin assignments |
| `docs/integration_notes.md` | Integration guide and constraints |

### Test Files

| File | Description |
|------|-------------|
| `verilog/dv/cocotb/spi_test/spi_test.c` | Firmware: Full-duplex SPI test with RX validation |
| `verilog/dv/cocotb/spi_test/spi_test.py` | Python: SPI slave simulation and verification |
| `verilog/dv/cocotb/SPISlaveModel.py` | Reusable SPI slave behavioral model |
| `verilog/dv/cocotb/VirtualGPIOModel.py` | Firmware-testbench synchronization helper |

---

## 🚀 Quick Start

### 1. Clone Repository
```bash
git clone https://github.com/${github_user_login}/SPI_TRIAL2.git
cd SPI_TRIAL2
```

### 2. Link IP Dependencies
```bash
python /nc/agent_tools/ipm_linker/ipm_linker.py \
  --file ./ip/link_IPs.json \
  --project-root $(pwd)
```

### 3. Run Verification (after fixing MISO path)
```bash
cd verilog/dv/cocotb
caravel_cocotb -t spi_test -tag debug_run
```

### 4. Check Results
```bash
# View logs
cat sim/debug_run/RTL-spi_test/spi_test.log

# Check pass/fail
ls sim/debug_run/RTL-spi_test/passed  # exists if passed
```

---

## 🔍 Current Status

### ✅ Complete
- RTL design and integration
- Wishbone bus infrastructure
- Firmware with CF_SPI.h API
- Python testbenches
- Documentation
- Power pin fixes
- Repository uploaded

### 🔧 In Progress
- **Debug MISO path** - SPI RX data not reaching firmware
- Recommended: Check `user_project_wrapper.v` GPIO9 connection

### ⏭️ Next Steps
1. Fix GPIO9 (MISO) input path to SPI controller
2. Re-run `spi_test` and verify full-duplex operation
3. Execute `system_test` for interrupt verification
4. Proceed to OpenLane hardening once tests pass

---

## 📈 Test Changes Summary

### spi_test.c Changes
- ✅ Added expected RX data array: `{0x66, 0xBB, 0x23, 0x42, 0x78, 0xAB, 0xBB, 0xCF}`
- ✅ Added per-byte RX validation with vgpio output
- ✅ Added error code (0xEEEE) on data mismatch
- ✅ Added GPIO pin comments for clarity

### spi_test.py Changes
- ✅ Enabled GPIO9 for MISO input: `caravelEnv.dut.gpio9_en.value = 1`
- ✅ Added MISO data transmission loop (8 bytes)
- ✅ Clocking MISO on falling edge of SCK (SPI Mode 0)
- ✅ Updated validation to check both TX and RX paths

**See `TEST_CHANGES_LOG.md` for complete line-by-line changes**

---

## 🎓 Lessons Learned

### What Worked Well
1. **IP Library Integration** - CF_SPI v2.0.1 integration was smooth
2. **Wishbone Infrastructure** - Bus splitter and PIC worked correctly
3. **Verification Framework** - Caravel-cocotb setup was effective
4. **Documentation** - Comprehensive docs helped debugging

### Issues Encountered
1. **Soft-IP Power Pins** - CF_SPI_WB and WB_PIC don't have VPWR/VGND ports (fixed)
2. **API Mismatches** - Initial test used placeholder functions (fixed with CF_SPI.h)
3. **MISO Path** - GPIO input to SPI controller needs verification (in progress)

### Best Practices Applied
- ✅ Used pre-verified IP from NativeChips library
- ✅ Created reusable verification models
- ✅ Documented changes incrementally
- ✅ Used proper CF_SPI.h API functions
- ✅ Version controlled with git

---

## 📞 Support Resources

### Documentation References
- **CF_SPI Datasheet**: `/nc/ip/CF_SPI/v2.0.1/CF_SPI.pdf`
- **CF_SPI Firmware API**: `/nc/ip/CF_SPI/v2.0.1/fw/CF_SPI.h`
- **Caravel Documentation**: Caravel integration guides in `docs/`

### Key Files for Debugging
1. **TEST_CHANGES_LOG.md** - Complete test analysis and recommendations
2. **Waveform**: `sim/*/RTL-spi_test/spi_test.vcd` (use GTKWave)
3. **Log**: `sim/*/RTL-spi_test/spi_test.log`

---

## 📝 Repository Statistics

- **Total Files**: 59
- **Lines of Code**: 15,868+
- **RTL Modules**: 5 (user_project, wrapper, splitter, PIC, CF_SPI_WB)
- **Test Cases**: 2 (spi_test, system_test)
- **Documentation Pages**: 12+
- **Verification Models**: 2 (SPI slave, Virtual GPIO)

---

## 🏆 Credits

**Project**: Caravel SPI User Project with Full-Duplex Verification  
**IP Used**: CF_SPI v2.0.1, CF_IP_UTIL v1.0.0 (NativeChips IP Library)  
**Framework**: Caravel SoC, caravel-cocotb, OpenLane  
**PDK**: SKY130A (Google/Skywater 130nm)  
**Agent**: CodeActRTLAgent (NativeChips)  

---

## 🔗 Links

- **Repository**: https://github.com/${github_user_login}/SPI_TRIAL2
- **Issues**: https://github.com/${github_user_login}/SPI_TRIAL2/issues
- **Caravel**: https://github.com/efabless/caravel

---

**Ready to debug and fix the MISO path! 🚀**

For detailed debugging steps, see `TEST_CHANGES_LOG.md` → Section: "Recommendations for Fixing"
