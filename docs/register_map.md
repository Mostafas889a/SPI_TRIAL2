# Register Map

## Memory Map Overview

| Peripheral | Base Address | Size | Description |
|-----------|--------------|------|-------------|
| SPI Controller | 0x3000_0000 | 64KB | CF_SPI Master Controller with FIFOs |
| PIC | 0x3001_0000 | 64KB | Programmable Interrupt Controller |

## Address Decoding

The Wishbone bus splitter uses bits [19:16] of the address to select peripherals:
- `wbs_adr_i[19:16] == 4'd0`: SPI Controller
- `wbs_adr_i[19:16] == 4'd1`: Programmable Interrupt Controller
- Other values: Invalid (returns 0xDEADBEEF on read, ACK but discard on write)

## SPI Controller Registers (Base: 0x3000_0000)

### Data Registers

| Register | Offset | Access | Reset | Description |
|----------|--------|--------|-------|-------------|
| RXDATA | 0x0000 | R | 0x00 | RX Data register (FIFO interface) |
| TXDATA | 0x0004 | W | 0x00 | TX Data register (FIFO interface) |

#### RXDATA [7:0] - Receive Data (Read-only)
Reading this register pops data from the RX FIFO.

#### TXDATA [7:0] - Transmit Data (Write-only)
Writing to this register pushes data to the TX FIFO.

### Configuration Registers

| Register | Offset | Access | Reset | Description |
|----------|--------|--------|-------|-------------|
| CFG | 0x0008 | W | 0x00 | Configuration Register |
| CTRL | 0x000C | W | 0x00 | Control Register |
| PR | 0x0010 | W | 0x02 | Clock Prescaler Register |

#### CFG Register
```
Bit 0: CPOL - Clock Polarity
  0: Clock idles low
  1: Clock idles high
  
Bit 1: CPHA - Clock Phase
  0: Data sampled on first edge
  1: Data sampled on second edge
  
Bits [31:2]: Reserved
```

#### CTRL Register
```
Bit 0: SS - Slave Select
  0: De-assert chip select (CSB = 1)
  1: Assert chip select (CSB = 0)
  
Bit 1: ENABLE - SPI Enable
  0: Disable SPI core
  1: Enable SPI core
  
Bit 2: RX_EN - Receive Enable
  0: Receive disabled
  1: Receive enabled
  
Bits [31:3]: Reserved
```

#### PR Register
```
Bits [15:0]: Prescaler Value
  SPI_CLK = WB_CLK / PR
  Minimum value: 2
  
Bits [31:16]: Reserved
```

### Status Register

| Register | Offset | Access | Reset | Description |
|----------|--------|--------|-------|-------------|
| STATUS | 0x0014 | R | 0x00 | Status Register |

#### STATUS Register
```
Bit 0: TIP - Transfer in Progress
  0: No transfer active
  1: Transfer in progress
  
Bit 1: BUSY - SPI Busy
  0: SPI idle
  1: SPI busy
  
Bits [31:2]: Status flags (see CF_SPI documentation)
```

### FIFO Registers

| Register | Offset | Access | Reset | Description |
|----------|--------|--------|-------|-------------|
| RX_FIFO_LEVEL | 0xFE00 | R | 0x00 | RX FIFO occupancy |
| RX_FIFO_THRESHOLD | 0xFE04 | W | 0x00 | RX FIFO threshold for interrupt |
| RX_FIFO_FLUSH | 0xFE08 | W | 0x00 | Write 1 to flush RX FIFO |
| TX_FIFO_LEVEL | 0xFE10 | R | 0x00 | TX FIFO occupancy |
| TX_FIFO_THRESHOLD | 0xFE14 | W | 0x00 | TX FIFO threshold for interrupt |
| TX_FIFO_FLUSH | 0xFE18 | W | 0x00 | Write 1 to flush TX FIFO |

### Interrupt Registers

| Register | Offset | Access | Reset | Description |
|----------|--------|--------|-------|-------------|
| IM | 0xFF00 | W | 0x00 | Interrupt Mask (1=enable) |
| MIS | 0xFF04 | R | 0x00 | Masked Interrupt Status |
| RIS | 0xFF08 | R | 0x00 | Raw Interrupt Status |
| IC | 0xFF0C | W | 0x00 | Interrupt Clear (write 1 to clear) |
| GCLK | 0xFF10 | W | 0x00 | Clock gating (1=enable, 0=disable) |

### Interrupt Flags

| Bit | Name | Description |
|-----|------|-------------|
| 0 | TIP | Transfer in progress interrupt |
| 1 | RX_FIFO_FULL | RX FIFO full |
| 2 | RX_FIFO_EMPTY | RX FIFO empty |
| 3 | RX_FIFO_THRESHOLD | RX FIFO level >= threshold |
| 4 | TX_FIFO_FULL | TX FIFO full |
| 5 | TX_FIFO_EMPTY | TX FIFO empty |
| 6 | TX_FIFO_THRESHOLD | TX FIFO level <= threshold |

## Programmable Interrupt Controller (Base: 0x3001_0000)

### Control Registers

| Register | Offset | Access | Reset | Description |
|----------|--------|--------|-------|-------------|
| IRQ_ENABLE_0_3 | 0x0000 | RW | 0x00 | Enable bits for IRQ[3:0] |
| IRQ_ENABLE_4_7 | 0x0004 | RW | 0x00 | Enable bits for IRQ[7:4] |
| IRQ_ENABLE_8_11 | 0x0008 | RW | 0x00 | Enable bits for IRQ[11:8] |
| IRQ_ENABLE_12_15 | 0x000C | RW | 0x00 | Enable bits for IRQ[15:12] |

#### IRQ_ENABLE Registers
```
Each register controls 4 IRQ lines:
Bits [7:0]: Enable for IRQ[N]
Bits [15:8]: Enable for IRQ[N+1]
Bits [23:16]: Enable for IRQ[N+2]
Bits [31:24]: Enable for IRQ[N+3]

Per-IRQ enable format:
  Bit 0: Enable (1=enabled, 0=masked)
  Bits [7:1]: Reserved
```

### Priority Registers

| Register | Offset | Access | Reset | Description |
|----------|--------|--------|-------|-------------|
| IRQ_PRIORITY_0_3 | 0x0010 | RW | 0x00 | Priority for IRQ[3:0] |
| IRQ_PRIORITY_4_7 | 0x0014 | RW | 0x00 | Priority for IRQ[7:4] |
| IRQ_PRIORITY_8_11 | 0x0018 | RW | 0x00 | Priority for IRQ[11:8] |
| IRQ_PRIORITY_12_15 | 0x001C | RW | 0x00 | Priority for IRQ[15:12] |

#### IRQ_PRIORITY Registers
```
Each register controls 4 IRQ priorities:
Bits [7:0]: Priority for IRQ[N]
Bits [15:8]: Priority for IRQ[N+1]
Bits [23:16]: Priority for IRQ[N+2]
Bits [31:24]: Priority for IRQ[N+3]

Per-IRQ priority format:
  Bits [1:0]: Priority level (0=highest, 3=lowest)
  Bits [7:2]: Reserved
```

### Type Registers

| Register | Offset | Access | Reset | Description |
|----------|--------|--------|-------|-------------|
| IRQ_TYPE_0_3 | 0x0020 | RW | 0x00 | Trigger type for IRQ[3:0] |
| IRQ_TYPE_4_7 | 0x0024 | RW | 0x00 | Trigger type for IRQ[7:4] |
| IRQ_TYPE_8_11 | 0x0028 | RW | 0x00 | Trigger type for IRQ[11:8] |
| IRQ_TYPE_12_15 | 0x002C | RW | 0x00 | Trigger type for IRQ[15:12] |

#### IRQ_TYPE Registers
```
Each register controls 4 IRQ trigger types:
Bits [7:0]: Type for IRQ[N]
Bits [15:8]: Type for IRQ[N+1]
Bits [23:16]: Type for IRQ[N+2]
Bits [31:24]: Type for IRQ[N+3]

Per-IRQ type format:
  Bit 0: Type (0=level high, 1=rising edge)
  Bits [7:1]: Reserved
```

### Status Registers

| Register | Offset | Access | Reset | Description |
|----------|--------|--------|-------|-------------|
| IRQ_STATUS | 0x0030 | R | 0x00 | Current IRQ status [15:0] |
| IRQ_PENDING | 0x0034 | R | 0x00 | Pending IRQ bits [15:0] |
| ACTIVE_IRQ | 0x0038 | R | 0xFF | Currently active IRQ number (0xFF=none) |

### Global Control

| Register | Offset | Access | Reset | Description |
|----------|--------|--------|-------|-------------|
| GLOBAL_ENABLE | 0x003C | RW | 0x00 | Global interrupt enable (Bit 0) |

## IRQ Mapping

| IRQ Line | Source | Description |
|----------|--------|-------------|
| 0 | SPI | SPI controller interrupt |
| 1-15 | Reserved | Available for future peripherals |

The PIC consolidates all interrupts into a single `irq_out` signal that connects to Caravel's `user_irq[0]`.

## Usage Examples

### Initialize SPI Master at 1 MHz (assuming 50 MHz WB clock)

```c
// Set prescaler: 50MHz / 50 = 1MHz
*(volatile uint32_t*)(0x30000010) = 50;

// Configure SPI: CPOL=0, CPHA=0
*(volatile uint32_t*)(0x30000008) = 0x00;

// Enable SPI and RX
*(volatile uint32_t*)(0x3000000C) = 0x06;
```

### Transfer Data

```c
// Assert CS, send byte
*(volatile uint32_t*)(0x3000000C) = 0x07;  // SS=1, EN=1, RX_EN=1
*(volatile uint32_t*)(0x30000004) = 0xAB;  // Write data

// Wait for transfer complete
while(*(volatile uint32_t*)(0x30000014) & 0x01);

// Read received data
uint8_t rx = *(volatile uint32_t*)(0x30000000);

// De-assert CS
*(volatile uint32_t*)(0x3000000C) = 0x06;  // SS=0, EN=1, RX_EN=1
```

### Enable SPI Interrupt

```c
// Enable SPI interrupt in PIC (IRQ0)
*(volatile uint32_t*)(0x30010000) = 0x01;  // Enable IRQ0

// Set priority to highest (0)
*(volatile uint32_t*)(0x30010010) = 0x00;

// Set as level-triggered
*(volatile uint32_t*)(0x30010020) = 0x00;

// Enable global interrupts
*(volatile uint32_t*)(0x3001003C) = 0x01;

// Enable SPI interrupt mask (e.g., TX empty)
*(volatile uint32_t*)(0x3000FF00) = 0x20;  // Bit 5: TX_FIFO_EMPTY
```

---

**Note:** All addresses are 32-bit word-aligned. Byte select signals (wbs_sel_i) should be honored by peripherals.
