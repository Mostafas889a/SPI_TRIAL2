# Integration Notes

## Clock and Reset Architecture

### Clock Domain
This design operates in a **single clock domain**:
- **Clock source:** `wb_clk_i` from Caravel management SoC
- **Typical frequency:** 25-50 MHz (configurable via Caravel PLL)
- **All logic synchronous to:** `wb_clk_i`

**No clock domain crossings exist in this design.**

### SPI Clock Generation
The SPI serial clock (SCK) is generated internally by the CF_SPI core:
- **Formula:** `SPI_CLK_freq = wb_clk_i_freq / prescaler`
- **Prescaler range:** 2 to 65535
- **Configuration register:** PR (offset 0x0010)

**Example:**
```
wb_clk_i = 50 MHz
Desired SPI_CLK = 1 MHz
Prescaler = 50 MHz / 1 MHz = 50
```

### Reset Strategy
- **Reset signal:** `wb_rst_i` (synchronous, active-high)
- **Reset source:** Caravel power-on-reset or management SoC
- **Reset behavior:**
  - All registers reset to default values
  - FIFOs flushed
  - SPI outputs go to safe state (SS=high, SCK per CPOL)

**Note:** Caravel uses synchronous reset throughout. This design follows that convention.

### Reset Sequence
1. `wb_rst_i` asserted by management SoC
2. All user_project logic enters reset state
3. Wishbone bus becomes idle
4. `wb_rst_i` de-asserted
5. Firmware initializes peripherals via Wishbone writes

## Bus Timing and Latency

### Wishbone B4 Classic Protocol
This design implements Wishbone B4 classic (non-pipelined) mode:
- **Address decode:** Combinational (same cycle)
- **Data read latency:** 1 cycle (registered output)
- **Acknowledgment:** 1 cycle after STB assertion
- **No wait states:** All peripherals respond in fixed time

### Timing Diagram
```
         ___     ___     ___     ___     ___
clk   __|   |___|   |___|   |___|   |___|   |___

cyc   ________/‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\________

stb   ________/‾‾‾‾‾‾‾‾‾\___________________

addr  --------<  VALID  >-------------------

dat_o ----------------<  VALID  >----------

ack   ____________/‾‾‾‾‾\___________________
                  ^
                  |
            Data available
```

### Critical Paths
1. **Address decode to peripheral select:** 
   - `wbs_adr_i[19:16]` → decoder → peripheral `stb`
   - Must meet setup time for peripheral registers

2. **Read data multiplexing:**
   - Peripheral `dat_o` → bus mux → `wbs_dat_o`
   - Combinational path through case statement

3. **Acknowledge generation:**
   - Peripheral `ack_o` → OR gate → `wbs_ack_o`
   - Registered in each peripheral

### SPI Transfer Latency
From firmware write to TXDATA until byte transmission completes:
```
Latency = 1 (WB write) + 1 (SPI core processing) + 8 * (prescaler) cycles
```

For 1 MHz SPI (prescaler=50) at 50 MHz system clock:
```
Latency = 1 + 1 + 8*50 = 402 cycles = ~8 µs
```

## Wishbone Bus Splitter Configuration

The design uses the provided `wishbone_bus_splitter.v` module:

### Parameters
```verilog
wishbone_bus_splitter #(
    .NUM_PERIPHERALS(2),      // SPI + PIC
    .ADDR_WIDTH(32),
    .DATA_WIDTH(32),
    .SEL_WIDTH(4),
    .ADDR_SEL_LOW_BIT(16)     // Use bits [19:16] for decode
) splitter_inst (
    // Master interface (from Caravel)
    .m_wb_adr_i(wbs_adr_i),
    // ...
);
```

### Address Mapping
| Peripheral | Index | Address Decode | Base Address |
|-----------|-------|----------------|--------------|
| SPI | 0 | addr[19:16] == 0 | 0x3000_0000 |
| PIC | 1 | addr[19:16] == 1 | 0x3001_0000 |

**Invalid addresses:** `addr[19:16] >= 2` return error (0xDEADBEEF on read, ACK but discard write).

### Bus Splitter Behavior
- **CYC propagation:** `wbs_cyc_i` routed unmodified to all slaves
- **STB gating:** Only selected peripheral receives `stb=1`
- **ACK collection:** OR of all peripheral ACKs
- **Data multiplexing:** Selected peripheral's `dat_o` forwarded to master

**Critical rule:** Never gate `wbs_cyc_i` with address decode logic.

## Interrupt Routing

### Interrupt Flow
```
┌─────────┐      ┌─────┐      ┌─────────────┐
│  CF_SPI │─IRQ─→│ PIC │─irq─→│ Caravel     │
└─────────┘      │     │ out  │ user_irq[0] │
                 └─────┘      └─────────────┘
```

### PIC Configuration
- **16 interrupt sources** (only IRQ0 used by SPI)
- **4-level priority:** 0 (highest) to 3 (lowest)
- **Trigger modes:** Level-high or rising-edge
- **Output:** Single consolidated interrupt to Caravel

### SPI Interrupt Sources
The CF_SPI core generates interrupts for:
- Transfer in progress (TIP)
- RX FIFO full/empty/threshold
- TX FIFO full/empty/threshold

Enable interrupts via IM register (offset 0xFF00).

### Caravel IRQ Handling
- `user_irq[0]` connected to PIC output
- Management SoC firmware must:
  1. Read PIC ACTIVE_IRQ register (0x3001_0038)
  2. Service the interrupt source
  3. Clear interrupt in peripheral (e.g., SPI IC register)
  4. Clear PIC pending bit if edge-triggered

## Power Connections

### User Project Power Domains
```
user_project_wrapper uses: vccd1/vssd1
CF_SPI_WB macro uses:       vccd2/vssd2
```

### Power Pin Mapping
In user_project_wrapper:
```verilog
CF_SPI_WB spi_inst (
`ifdef USE_POWER_PINS
    .VPWR(vccd2),
    .VGND(vssd2),
`endif
    // ...
);
```

### PDN Configuration (OpenLane)
```json
"PDN_MACRO_CONNECTIONS": [
    "spi_inst vccd2 vssd2 VPWR VGND"
]
```

**Critical:** Power pin names in wrapper instantiation MUST match PDN_MACRO_CONNECTIONS.

## Simulation and Testbench Notes

### Caravel-Cocotb Framework
Tests use the caravel-cocotb framework for full-system simulation:
- **DUT:** `caravel.v` (includes management SoC + user_project_wrapper)
- **Test language:** Python (cocotb)
- **Firmware:** Compiled C code loaded into management SoC memory
- **Interaction:** Firmware writes to user_project via Wishbone

### design_info.yaml Structure
```yaml
name: caravel_spi_project
top_module: caravel
rtl_dir: ../../verilog/rtl
includes:
  - ../../verilog/rtl/user_project_wrapper.v
  - ../../verilog/rtl/user_project.v
  # ... more RTL files
fw_dir: ../../fw
```

### Test Infrastructure Files
- `cocotb_tests.py`: Registers test modules
- `design_info.yaml`: Configures DUT for caravel-cocotb
- `spi_test/spi_test.py`: Cocotb test logic
- `spi_test/spi_test.c`: Firmware for SPI test
- `spi_slave.py`: Reusable SPI slave model

### Running Tests
```bash
cd verilog/dv/cocotb
caravel-cocotb --sim icarus spi_test
```

### SPI Slave Model
The cocotb SPI slave model monitors GPIO pins and:
- **Responds to transactions:** Drives MISO based on internal register
- **Configurable modes:** CPOL, CPHA settings
- **Self-checking:** Verifies correct SPI timing and protocol

## Synthesis and P&R Considerations

### Target Technology
- **PDK:** SKY130A (Google/Skywater 130nm)
- **Standard cell library:** sky130_fd_sc_hd (high-density)
- **Tool:** OpenLane 2 (LibreLane)

### Timing Constraints
- **Target frequency:** 50 MHz (20 ns period)
- **Clock uncertainty:** 0.5 ns (base SDC)
- **Input delay:** 5 ns (from pad to first register)
- **Output delay:** 5 ns (from last register to pad)

### SDC Files
- **Base SDC:** `openlane/<macro>/base.sdc` (used during PnR)
- **Signoff SDC:** `openlane/user_project_wrapper/signoff.sdc` (stricter for final checks)

**Note:** Copy signoff.sdc to user_project directory for macro hardening to ensure timing budget compatibility.

### Area Constraints
- **user_project_wrapper area:** ~3000µm × 3600µm
- **Recommended die area:** 400µm × 400µm minimum (for pin placement)
- **Core utilization:** Do NOT set FP_CORE_UTIL; use fixed DIE_AREA instead

### Pin Placement
Use `pin_order.cfg` to control pad assignments:
- **Wishbone signals:** South edge (#S)
- **User IOs (mprj_io):** East/North/West edges per Caravel template
- **Remove unused pins from cfg**

### Common Issues and Solutions

#### Issue: Timing violations (setup/hold)
**Solution:**
- Increase DIE_AREA to reduce congestion
- Adjust base SDC: tighter clock period or increased uncertainty
- Use DESIGN_REPAIR_MAX_WIRE_LENGTH to limit wire lengths

#### Issue: DRC violations (via density, min area)
**Solution:**
- Reduce FP_PDN_VWIDTH/HWIDTH if power grid too dense
- Increase DIE_AREA to give router more freedom
- Check for macro placement conflicts

#### Issue: LVS mismatch
**Solution:**
- Verify PDN_MACRO_CONNECTIONS match wrapper power pins
- Check blackbox cells are properly defined in LEF
- Ensure all power pins are connected (no floating nets)

#### Issue: High slew/capacitance on pads
**Solution:**
- Set/decrease DESIGN_REPAIR_MAX_WIRE_LENGTH
- Set/decrease GRT_DESIGN_REPAIR_MAX_WIRE_LENGTH
- Add buffer insertion near pads

### Two-Stage Hardening Flow
1. **Stage 1:** Harden user_project (SPI + bus splitter + PIC)
   - Produces: GDS, LEF, LIB, SPEF
   - Treat as macro for stage 2

2. **Stage 2:** Harden user_project_wrapper
   - Instantiates user_project macro
   - Connects to Caravel pads
   - Final GDSII for tape-out

## Debug and Bring-Up

### Initial Firmware Tests
1. **Bus connectivity:** Read/write PIC registers
2. **SPI loopback:** Connect MOSI to MISO externally, verify echo
3. **Interrupt test:** Enable SPI TX_EMPTY interrupt, check user_irq[0]

### Waveform Inspection
Key signals to monitor:
- `wbs_cyc_i`, `wbs_stb_i`, `wbs_ack_o`: Bus handshake
- `wbs_adr_i[19:16]`: Peripheral selection
- `spi_mosi`, `spi_miso`, `spi_sck`, `spi_ss`: SPI protocol
- `irq_out`: Interrupt assertion

### Common Bugs
1. **No ACK on bus:** Check address decode and peripheral ACK generation
2. **Wrong data read:** Verify bus multiplexer case statement
3. **SPI not transmitting:** Ensure ENABLE and SS bits set in CTRL register
4. **No interrupt:** Check PIC enable, priority, and SPI IM register

## References

- **CF_SPI IP Documentation:** `/nc/ip/CF_SPI/v2.0.1/README.md`
- **Wishbone B4 Specification:** [OpenCores Wishbone spec](https://opencores.org/websvn/filedetails?repname=wishbone&path=%2Fwishbone%2Ftrunk%2Fdoc%2Fwbspec_b4.pdf)
- **Caravel User Project Guide:** Template README and examples
- **Caravel-cocotb Documentation:** Framework usage and best practices

---

**Last Updated:** 2025-11-13  
**Revision:** 1.0
