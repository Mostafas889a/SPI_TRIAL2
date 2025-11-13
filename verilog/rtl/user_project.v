`timescale 1ns / 1ps
`default_nettype none

module user_project (
`ifdef USE_POWER_PINS
    inout VPWR,
    inout VGND,
`endif
    input wire wb_clk_i,
    input wire wb_rst_i,
    input wire wbs_cyc_i,
    input wire wbs_stb_i,
    input wire wbs_we_i,
    input wire [3:0] wbs_sel_i,
    input wire [31:0] wbs_adr_i,
    input wire [31:0] wbs_dat_i,
    output wire [31:0] wbs_dat_o,
    output wire wbs_ack_o,
    output wire user_irq,
    input wire spi_miso,
    output wire spi_mosi,
    output wire spi_sck,
    output wire spi_ss
);

    localparam NUM_PERIPHERALS = 2;
    
    wire [NUM_PERIPHERALS-1:0] s_wb_cyc;
    wire [NUM_PERIPHERALS-1:0] s_wb_stb;
    wire [NUM_PERIPHERALS-1:0] s_wb_we;
    wire [NUM_PERIPHERALS*4-1:0] s_wb_sel;
    wire [NUM_PERIPHERALS*32-1:0] s_wb_adr;
    wire [NUM_PERIPHERALS*32-1:0] s_wb_dat_o;
    wire [NUM_PERIPHERALS*32-1:0] s_wb_dat_i;
    wire [NUM_PERIPHERALS-1:0] s_wb_ack;
    wire [NUM_PERIPHERALS-1:0] s_wb_err;
    
    wire spi_irq;
    wire [15:0] irq_lines;
    
    assign irq_lines[0] = spi_irq;
    assign irq_lines[15:1] = 15'b0;

    wishbone_bus_splitter #(
        .NUM_PERIPHERALS(NUM_PERIPHERALS),
        .ADDR_WIDTH(32),
        .DATA_WIDTH(32),
        .SEL_WIDTH(4),
        .ADDR_SEL_LOW_BIT(16)
    ) bus_splitter (
        .m_wb_adr_i(wbs_adr_i),
        .m_wb_dat_i(wbs_dat_i),
        .m_wb_dat_o(wbs_dat_o),
        .m_wb_we_i(wbs_we_i),
        .m_wb_sel_i(wbs_sel_i),
        .m_wb_cyc_i(wbs_cyc_i),
        .m_wb_stb_i(wbs_stb_i),
        .m_wb_ack_o(wbs_ack_o),
        .m_wb_err_o(),
        .s_wb_cyc_o(s_wb_cyc),
        .s_wb_stb_o(s_wb_stb),
        .s_wb_we_o(s_wb_we),
        .s_wb_sel_o(s_wb_sel),
        .s_wb_adr_o(s_wb_adr),
        .s_wb_dat_o(s_wb_dat_o),
        .s_wb_dat_i(s_wb_dat_i),
        .s_wb_ack_i(s_wb_ack),
        .s_wb_err_i(s_wb_err)
    );

    CF_SPI_WB #(
        .CDW(8),
        .FAW(4)
    ) spi_controller (
        .clk_i(wb_clk_i),
        .rst_i(wb_rst_i),
        .adr_i(s_wb_adr[31:0]),
        .dat_i(s_wb_dat_o[31:0]),
        .dat_o(s_wb_dat_i[31:0]),
        .sel_i(s_wb_sel[3:0]),
        .cyc_i(s_wb_cyc[0]),
        .stb_i(s_wb_stb[0]),
        .ack_o(s_wb_ack[0]),
        .we_i(s_wb_we[0]),
        .IRQ(spi_irq),
        .miso(spi_miso),
        .mosi(spi_mosi),
        .csb(spi_ss),
        .sclk(spi_sck)
    );
    
    assign s_wb_err[0] = 1'b0;

    WB_PIC pic (
        .clk(wb_clk_i),
        .rst_n(~wb_rst_i),
        .irq_lines(irq_lines),
        .irq_out(user_irq),
        .wb_adr_i(s_wb_adr[63:32]),
        .wb_dat_i(s_wb_dat_o[63:32]),
        .wb_dat_o(s_wb_dat_i[63:32]),
        .wb_sel_i(s_wb_sel[7:4]),
        .wb_cyc_i(s_wb_cyc[1]),
        .wb_stb_i(s_wb_stb[1]),
        .wb_we_i(s_wb_we[1]),
        .wb_ack_o(s_wb_ack[1])
    );
    
    assign s_wb_err[1] = 1'b0;

endmodule

`default_nettype wire
