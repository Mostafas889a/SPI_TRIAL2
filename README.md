# Caravel SPI User Project

## Project Overview

**Initial User Prompt:**
> Build a caravel user with SPI connected to gpios and make a spi slave using cocotb to be used for verification add tests but don't run them just add them and notify me

**Objective:**
Create a Caravel user project that integrates an SPI controller peripheral connected to GPIO pads, with comprehensive cocotb-based verification including a reusable SPI slave model.

## Project Status

- **Stage:** RTL Development Complete
- **Status:** ✅ Ready for Testing
- **Last Updated:** 2025-11-13

## 🔔 Latest Updates

### ✅ Power Pin Fix (2025-11-13)
**Fixed**: Removed incorrect VPWR/VGND connections from CF_SPI_WB and WB_PIC soft-IP instances. See `POWER_PIN_FIX.md`.

### ✅ Firmware API Update (2025-11-13)
**Updated**: All test firmware now uses correct CF_SPI.h API functions from CF_SPI v2.0.1. See `FIRMWARE_API_UPDATE.md`.

## Requirements

### Functional Requirements
1. Integrate SPI controller from pre-verified IP library (CF_SPI)
2. Connect SPI signals (MOSI, MISO, SCK, SS) to Caravel GPIO pads
3. Implement Wishbone Classic B4 interface for SoC communication
4. Support programmable interrupt controller for SPI events
5. Provide memory-mapped register interface for SPI configuration

### Verification Requirements
1. Create reusable cocotb SPI slave model
2. Develop SPI peripheral integration test with firmware driver
3. Implement system integration test
4. Ensure all tests are ready to run (but not executed yet)

### Physical Design Requirements
1. RTL must pass linting (Verilator)
2. Clean synthesis with Yosys
3. Meet Caravel integration constraints
4. Ready for OpenLane hardening flow

## Architecture

### System Block Diagram
```
┌─────────────────────────────────────────────────────────────┐
│                    Caravel SoC (Management)                  │
│                                                               │
│  ┌────────────┐      Wishbone Bus       ┌─────────────┐    │
│  │ Management │◄────────────────────────►│ user_project│    │
│  │    SoC     │                          │             │    │
│  └────────────┘                          │  ┌────────┐ │    │
│                                          │  │  Bus   │ │    │
│                                          │  │Splitter│ │    │
│                                          │  └───┬────┘ │    │
│                                          │      │      │    │
│                                          │  ┌───▼────┐ │    │
│                                          │  │  SPI   │ │    │
│                                          │  │  Core  │ │    │
│                                          │  └───┬────┘ │    │
│                                          │      │      │    │
│                                          │  ┌───▼────┐ │    │
│                                          │  │  PIC   │ │    │
│                                          │  └────────┘ │    │
│                                          └─────┬───────┘    │
│                                                │            │
│  ┌──────────────────────────────────────┐     │            │
│  │     user_project_wrapper              │     │            │
│  │  ┌────────────────────────────────┐   │     │            │
│  │  │  GPIO Pads (mprj_io[])         │   │◄────┘            │
│  │  │  - MOSI, MISO, SCK, SS         │   │                  │
│  │  └────────────────────────────────┘   │                  │
│  └──────────────────────────────────────┘                   │
└─────────────────────────────────────────────────────────────┘
```

### Address Map
```
Base Address: 0x3000_0000
- 0x3000_0000 - 0x3000_FFFF: SPI Controller
- 0x3001_0000 - 0x3001_FFFF: Programmable Interrupt Controller (PIC)
```

### GPIO Pad Assignments
- mprj_io[8]: SPI_MOSI (Output)
- mprj_io[9]: SPI_MISO (Input)
- mprj_io[10]: SPI_SCK (Output)
- mprj_io[11]: SPI_SS (Output)

## Design Approach

### IP Integration Strategy
1. Use CF_SPI from NativeChips verified IP library
2. Leverage existing Wishbone wrapper from IP package
3. Use wishbone_bus_splitter.v for bus arbitration
4. Use WB_PIC.v for interrupt management

### Verification Strategy
1. Cocotb-based testbench with Python test infrastructure
2. Reusable SPI slave model for multiple test scenarios
3. C firmware drivers for realistic SoC interaction
4. Caravel-cocotb framework for full system simulation

### Physical Design Strategy
1. Harden SPI wrapper as macro
2. Integrate macro into user_project_wrapper
3. Follow Caravel timing and power constraints
4. Use SKY130 HD standard cell library

## Directory Structure
```
caravel_spi_project/
├── README.md                          # This file
├── docs/
│   ├── register_map.md               # SPI and PIC register definitions
│   ├── pad_map.md                    # GPIO pad assignments
│   ├── integration_notes.md          # Clocking, reset, timing notes
│   └── retrospective.md              # Post-project analysis
├── verilog/
│   ├── rtl/
│   │   ├── user_project.v            # Top-level user logic
│   │   ├── user_project_wrapper.v    # Caravel wrapper
│   │   ├── wishbone_bus_splitter.v   # Bus infrastructure
│   │   └── WB_PIC.v                  # Interrupt controller
│   └── dv/
│       └── cocotb/
│           ├── spi_slave.py          # Reusable SPI slave model
│           ├── spi_test/             # SPI peripheral test
│           ├── system_test/          # System integration test
│           ├── cocotb_tests.py       # Test registry
│           └── design_info.yaml      # Caravel-cocotb config
├── fw/
│   └── spi_driver.h                  # C firmware API
├── ip/
│   └── link_IPs.json                 # IPM linker configuration
└── openlane/
    ├── user_project/                 # SPI macro hardening
    └── user_project_wrapper/         # Wrapper hardening
```

## Progress Tracking

### Completed Tasks
- [x] Project structure setup
- [x] Initial documentation created

### In Progress
- [ ] IP integration (CF_SPI)

### Pending Tasks
- [ ] User project RTL development
- [ ] Wrapper creation
- [ ] Verification infrastructure
- [ ] Test development
- [ ] Documentation finalization

## Key Design Decisions

1. **SPI IP Selection:** Using CF_SPI from NativeChips library (verified IP)
2. **Bus Architecture:** Wishbone Classic B4 with centralized bus splitter
3. **Interrupt Strategy:** Hardware PIC supporting up to 16 sources
4. **GPIO Mapping:** Consecutive pads (8-11) for clean routing
5. **Address Decode:** 64KB windows using bits [19:16]

## References

- Caravel User Project Template: `/nc/templates/caravel_user_project/`
- CF_SPI IP Documentation: `/nc/ip/CF_SPI/`
- Wishbone B4 Specification: Caravel integration guide
- Caravel-cocotb Framework: Verification documentation

## Notes

- All RTL must be Verilog-2005 compliant
- Single clock domain design (wb_clk_i)
- Asynchronous active-high reset (wb_rst_i)
- Power domain: vccd1/vssd1 for wrapper, vccd2/vssd2 for macro

---

## Project Completion Status

### ✅ RTL Development - COMPLETE
- **user_project.v**: Integrates CF_SPI controller with Wishbone bus splitter and PIC
- **user_project_wrapper.v**: Connects SPI signals to mprj_io[8:11]
- All RTL modules pass Verilator lint checks
- Clean synthesis ready

### ✅ Verification Infrastructure - COMPLETE  
- **VirtualGPIOModel.py**: Firmware-testbench synchronization framework
- **SPISlaveModel.py**: Reusable cocotb SPI slave for loopback testing
- **spi_test**: Basic SPI loopback test (8 bytes)
- **system_test**: Integrated SPI + PIC + interrupt test (4 bytes)
- All test firmware and Python scripts created
- Include files configured for RTL simulation

### 📋 Tests Ready to Execute (UPDATED)
**Latest Update**: Firmware tests updated to use proper CF_SPI.h API functions from the CF_SPI IP library.

As requested, tests have been created but NOT run. To execute:

```bash
cd /workspace/caravel_spi_project/verilog/dv/cocotb

# Run individual tests
caravel_cocotb -t spi_test -tag spi_run
caravel_cocotb -t system_test -tag system_run

# Check results in
ls sim/spi_run/RTL-spi_test/
ls sim/system_run/RTL-system_test/
```

### 📁 Key Deliverables
| Category | File | Status |
|----------|------|--------|
| RTL | verilog/rtl/user_project.v | ✅ Complete |
| RTL | verilog/rtl/user_project_wrapper.v | ✅ Complete |
| Test Model | verilog/dv/cocotb/VirtualGPIOModel.py | ✅ Complete |
| Test Model | verilog/dv/cocotb/SPISlaveModel.py | ✅ Complete |
| Test | verilog/dv/cocotb/spi_test/* | ✅ Complete |
| Test | verilog/dv/cocotb/system_test/* | ✅ Complete |
| Docs | docs/register_map.md | ✅ Complete |
| Docs | docs/pad_map.md | ✅ Complete |
| Docs | docs/integration_notes.md | ✅ Complete |
| Docs | docs/dv/test_summary.md | ✅ Complete |

---

**Last Updated:** 2025-11-13  
**Project Stage:** RTL Documentation (Complete)  
**Agent:** Documentation_Agent

**READY FOR USER TO RUN TESTS**
