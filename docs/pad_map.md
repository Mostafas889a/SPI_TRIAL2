# GPIO Pad Map

## SPI Signal Assignments

| Signal Name | Direction | mprj_io Index | Default Config | Description |
|------------|-----------|---------------|----------------|-------------|
| SPI_MOSI | Output | 8 | Push-pull output | Master Out Slave In |
| SPI_MISO | Input | 9 | Input | Master In Slave Out |
| SPI_SCK | Output | 10 | Push-pull output | Serial Clock |
| SPI_SS | Output | 11 | Push-pull output | Slave Select (active low) |

## Pad Configuration Details

### mprj_io[8] - SPI_MOSI (Output)
```verilog
assign mprj_io_out[8] = spi_mosi;
assign mprj_io_oeb[8] = 1'b0;  // Enable output driver
```
- **Type:** Digital output
- **Drive strength:** Standard cell
- **Initial state:** Driven by SPI core

### mprj_io[9] - SPI_MISO (Input)
```verilog
assign spi_miso = mprj_io_in[9];
assign mprj_io_out[9] = 1'b0;   // Tie output low
assign mprj_io_oeb[9] = 1'b1;   // Disable output driver (input mode)
```
- **Type:** Digital input
- **Pull:** External (depends on slave device)
- **Schmitt trigger:** Enabled (via pad configuration)

### mprj_io[10] - SPI_SCK (Output)
```verilog
assign mprj_io_out[10] = spi_sck;
assign mprj_io_oeb[10] = 1'b0;  // Enable output driver
```
- **Type:** Digital output
- **Drive strength:** Standard cell
- **Initial state:** Driven by SPI core (idles per CPOL setting)

### mprj_io[11] - SPI_SS (Output)
```verilog
assign mprj_io_out[11] = spi_ss;
assign mprj_io_oeb[11] = 1'b0;  // Enable output driver
```
- **Type:** Digital output
- **Drive strength:** Standard cell
- **Initial state:** High (inactive) when SPI disabled

## Reserved/Unused Pads

The following mprj_io pads are NOT used by this design and should be configured as inputs or tied off in user_project_wrapper:

| Index Range | Status | Configuration |
|------------|--------|---------------|
| mprj_io[0:7] | Unused | Configured as inputs (oeb=1, out=0) |
| mprj_io[12:37] | Unused | Configured as inputs (oeb=1, out=0) |

**Note:** Caravel reserves mprj_io[4:0] for specific functions. This design does not use those pads, but they should not be reconfigured without understanding Caravel requirements.

## Physical Routing Considerations

### Pin Placement Strategy
The SPI signals are assigned to consecutive pads (8-11) for several reasons:
1. **Timing closure:** Shorter routing within user_project_wrapper
2. **Signal integrity:** Minimal crosstalk due to grouped related signals
3. **External PCB routing:** Easier to route when pins are adjacent

### Suggested External Connections
```
Caravel GPIO Pad    →    External SPI Device
-------------------------------------------------
mprj_io[8] (MOSI)   →    SDI/MOSI/DIN
mprj_io[9] (MISO)   →    SDO/MISO/DOUT
mprj_io[10] (SCK)   →    SCK/SCLK
mprj_io[11] (SS)    →    CS#/SS#/CSB
```

## Changing Pin Assignments

To modify the pad assignments:

1. **Update user_project_wrapper.v:**
   - Locate the SPI signal connections to mprj_io vectors
   - Change the index numbers to desired pads
   - Ensure output enable (oeb) settings match signal direction

2. **Update this documentation:**
   - Modify the table above
   - Update any testbench/firmware that references specific GPIO numbers

3. **Constraints for OpenLane:**
   - Update pin_order.cfg to reflect new positions
   - Verify timing constraints account for new routing

### Example: Moving SPI to mprj_io[20:23]

In user_project_wrapper.v, change:
```verilog
// Old assignments
assign mprj_io_out[8] = spi_mosi;
assign spi_miso = mprj_io_in[9];
// ...

// New assignments
assign mprj_io_out[20] = spi_mosi;
assign spi_miso = mprj_io_in[21];
assign mprj_io_out[22] = spi_sck;
assign mprj_io_out[23] = spi_ss;
```

And update oeb accordingly.

## Caravel-Specific Notes

### Pad Type
All mprj_io pads are **bidirectional** by design. The user_project_wrapper configures direction via:
- `mprj_io_oeb[N]`: Output Enable Bar (active low)
  - `0` = Output mode
  - `1` = Input mode (high-Z)

### Power Domain
- All GPIO pads are powered from **VDDIO** and **VSSIO**
- Internal logic uses **vccd1/vssd1** for user_project_wrapper
- Level shifting (if needed) is handled by pad cells

### Management SoC Access
The Caravel management SoC can override GPIO configuration through housekeeping registers. For production use:
- Configure GPIO modes during firmware initialization
- Set `mprj_io_oeb` appropriately via management SoC before enabling user_project

### ESD Protection
All pads include built-in ESD protection structures. No external protection diodes are required for normal operation.

## Verification GPIO Mapping

For testbench purposes, the following mapping is used:

| Test Signal | mprj_io Index | Purpose |
|------------|---------------|---------|
| SPI_MOSI | 8 | Drive from DUT, monitor in testbench |
| SPI_MISO | 9 | Drive from SPI slave model |
| SPI_SCK | 10 | Monitor for clock integrity |
| SPI_SS | 11 | Monitor for transaction framing |

The cocotb testbench will instantiate a SPI slave model that monitors these GPIOs and responds accordingly.

---

**Last Updated:** 2025-11-13  
**Revision:** 1.0
