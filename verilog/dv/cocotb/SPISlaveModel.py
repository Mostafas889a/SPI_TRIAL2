import cocotb
from cocotb.triggers import FallingEdge, RisingEdge
import cocotb.log

async def SPI_VIP(csb, clk, mosi, miso_en, miso_data, mem):
    while True:
        await FallingEdge(csb)
        cocotb.log.info("[SPI_VIP] CSB is asserted, operation has begun")
        op = await cocotb.start(SPI_op(clk, mosi, miso_en, miso_data, mem))
        await csb_watcher(csb, op)
        cocotb.log.info("[SPI_VIP] CSB is deasserted, operation has been killed")

async def csb_watcher(csb, thread):
    cocotb.log.info("[csb_watcher] start CSB watching")
    await RisingEdge(csb)
    thread.kill()

async def SPI_op(clk, mosi, miso_en, miso_data, mem):
    address = ""
    command = ""
    await RisingEdge(clk)
    miso_en.value = 1
    miso_data.value = 0

    for i in range(8):
        command = command + str(mosi.value)
        await RisingEdge(clk)
    cocotb.log.info(f"[SPI_VIP] [SPI_op] command = {command}")

    address = ""
    for i in range(8 * 3):
        address = address + str(mosi.value)
        if i != 23:
            await RisingEdge(clk)
    cocotb.log.info(f"[SPI_VIP] [SPI_op] address = {address}")
    address = int(address, 2)

    if command == "00000011":
        await FallingEdge(clk)
        while True:
            data = bin(mem[address])[2:].zfill(8)
            for i in range(8):
                miso_en.value = 1
                miso_data.value = int(data[i], 2)
                cocotb.log.debug(f"[SPI_VIP] [SPI_op] MISO = {data[i]}")
                await FallingEdge(clk)
            miso_en.value = 0

            cocotb.log.info(f"[SPI_VIP] [SPI_op] finish reading address {hex(address)} data = {hex(int(data,2))}")
            if address < 0xFF:
                address += 1

def read_mem(file_name):
    with open(file_name, "r") as file:
        lines = file.readlines()
        mem = dict()
        for line in lines:
            if line[0] == "@":
                address = int(line[1:], 16)
                cocotb.log.debug(f" found line = {line} address = {hex(address)}")
            else:
                line_no_space = line.strip().replace(" ", "")
                for i in range(0, len(line_no_space), 2):
                    cocotb.log.debug(f" i = {i} line_no_space[{i}:{i+2}] = {line_no_space[i:i+2]} address = {hex(address)}")
                    mem[address] = int(line_no_space[i:i + 2], 16)
                    address += 1
                cocotb.log.debug(f" found line = {line} line_no_space = {line_no_space} size = {len(line_no_space)}")
        cocotb.log.info(f"[read_mem] SPI mem = {mem}")
        return mem
