import cocotb
from caravel_cocotb.caravel_interfaces import test_configure, report_test
from cocotb.triggers import RisingEdge, FallingEdge, ClockCycles
import sys
sys.path.append('..')
from VirtualGPIOModel import VirtualGPIOModel

@cocotb.test()
@report_test
async def system_test(dut):
    caravelEnv = await test_configure(dut, timeout_cycles=1000000)
    cocotb.log.info("[TEST] start system_test")
    await caravelEnv.release_csb()
    
    vgpio = VirtualGPIOModel(caravelEnv)
    vgpio.start()
    
    spi_mosi = dut.uut.mprj_io[8]
    spi_miso = dut.uut.mprj_io[9]
    spi_sck = dut.uut.mprj_io[10]
    spi_ss = dut.uut.mprj_io[11]
    
    user_irq = dut.uut.user_irq[0]
    
    cocotb.log.info("[TEST] Waiting for firmware ready signal (vgpio=1)")
    await vgpio.wait_output(1)
    cocotb.log.info("[TEST] Firmware ready")
    
    cocotb.log.info("[TEST] Waiting for PIC enabled (vgpio=2)")
    await vgpio.wait_output(2)
    cocotb.log.info("[TEST] PIC enabled")
    
    cocotb.log.info("[TEST] Waiting for SPI peripheral enabled (vgpio=3)")
    await vgpio.wait_output(3)
    cocotb.log.info("[TEST] SPI peripheral enabled")
    
    test_data = [0xDE, 0xAD, 0xBE, 0xEF]
    rx_data = []
    
    cocotb.log.info("[TEST] Starting SPI slave loopback")
    
    await FallingEdge(spi_ss)
    cocotb.log.info("[TEST] SPI CS asserted")
    
    for byte_idx in range(4):
        byte_val = 0
        for bit_idx in range(8):
            await RisingEdge(spi_sck)
            bit = spi_mosi.value
            byte_val = (byte_val << 1) | int(bit)
            
            if bit_idx == 0:
                spi_miso.value = (test_data[byte_idx] >> 7) & 1
            else:
                spi_miso.value = (test_data[byte_idx] >> (7 - bit_idx)) & 1
        
        rx_data.append(byte_val)
        cocotb.log.info(f"[TEST] Received byte {byte_idx}: 0x{byte_val:02X}")
    
    await RisingEdge(spi_ss)
    cocotb.log.info("[TEST] SPI CS deasserted")
    
    cocotb.log.info("[TEST] Waiting for data transmission complete (vgpio=4)")
    await vgpio.wait_output(4)
    cocotb.log.info("[TEST] Data transmission complete")
    
    cocotb.log.info("[TEST] Checking user IRQ signal")
    irq_val = user_irq.value
    cocotb.log.info(f"[TEST] User IRQ value: {irq_val}")
    
    cocotb.log.info("[TEST] Waiting for firmware verification (vgpio=5 or 6)")
    output = vgpio.get_output()
    timeout = 0
    while output not in [5, 6]:
        await RisingEdge(caravelEnv.clk)
        output = vgpio.get_output()
        timeout += 1
        if timeout > 100000:
            cocotb.log.error("[TEST] Timeout waiting for firmware verification")
            assert False, "Timeout waiting for firmware verification"
    
    if output == 5:
        cocotb.log.info("[TEST] Firmware verification PASSED")
    else:
        cocotb.log.error("[TEST] Firmware verification FAILED")
        assert False, "Firmware verification failed"
    
    cocotb.log.info("[TEST] Waiting for peripheral disabled (vgpio=7)")
    await vgpio.wait_output(7)
    cocotb.log.info("[TEST] SPI peripheral disabled")
    
    for i in range(4):
        if rx_data[i] != test_data[i]:
            cocotb.log.error(f"[TEST] Data mismatch at byte {i}: expected 0x{test_data[i]:02X}, got 0x{rx_data[i]:02X}")
            assert False, f"Data mismatch at byte {i}"
    
    cocotb.log.info("[TEST] System integration test PASSED")
