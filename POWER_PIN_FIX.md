# Power Pin Fix Applied

## Issue
```
/workspace/caravel_spi_project/verilog/rtl/user_project.v:74: error: port ``VPWR'' is not a port of spi_controller.
```

## Root Cause
The `CF_SPI_WB` and `WB_PIC` modules from the IP library do **not** have `VPWR`/`VGND` power pins in their port lists. These are soft-IP RTL modules that don't require explicit power pin connections.

Only hard macros (like SRAM) require explicit power pin connections for PnR.

## Fix Applied

### File Modified
`verilog/rtl/user_project.v`

### Changes

**CF_SPI_WB Instance (lines 71-76):**
```verilog
// BEFORE (Incorrect - caused error)
CF_SPI_WB #(
    .CDW(8),
    .FAW(4)
) spi_controller (
`ifdef USE_POWER_PINS
    .VPWR(VPWR),
    .VGND(VGND),
`endif
    .clk_i(wb_clk_i),
    ...

// AFTER (Correct - no power pins)
CF_SPI_WB #(
    .CDW(8),
    .FAW(4)
) spi_controller (
    .clk_i(wb_clk_i),
    ...
```

**WB_PIC Instance (lines 94-96):**
```verilog
// BEFORE (Incorrect - caused error)
WB_PIC pic (
`ifdef USE_POWER_PINS
    .VPWR(VPWR),
    .VGND(VGND),
`endif
    .clk(wb_clk_i),
    ...

// AFTER (Correct - no power pins)
WB_PIC pic (
    .clk(wb_clk_i),
    ...
```

## Verification

The CF_SPI_WB module definition from `/nc/ip/CF_SPI/v2.0.1/hdl/rtl/bus_wrappers/CF_SPI_WB.v`:

```verilog
module CF_SPI_WB #(
    parameter CDW = 8,
    FAW = 4
) (
    input  wire         clk_i,      // No VPWR/VGND ports
    input  wire         rst_i,
    input  wire [ 31:0] adr_i,
    ...
);
```

The WB_PIC module definition from `verilog/rtl/WB_PIC.v`:

```verilog
module WB_PIC (
    input  wire        clk,         // No VPWR/VGND ports
    input  wire        rst_n,
    input  wire [15:0] irq_lines,
    ...
);
```

## Status
✅ **FIXED** - RTL no longer attempts to connect non-existent power pins

## Note
The `user_project` module still has `VPWR`/`VGND` ports for Caravel integration:
```verilog
module user_project (
`ifdef USE_POWER_PINS
    inout VPWR,
    inout VGND,
`endif
    ...
);
```

This is correct - these ports are required at the wrapper level for Caravel's power distribution, but they don't need to be explicitly connected to every internal soft-IP module.

## Build Status
- ✅ RTL syntax: Clean (power pin error resolved)
- ⚠️ IP warnings: Expected (from pre-verified CF_SPI IP - safe to ignore)
- ✅ User project logic: Clean
- ✅ Ready for testing and PnR

**Date**: 2025-11-13  
**Agent**: CodeActRTLAgent
