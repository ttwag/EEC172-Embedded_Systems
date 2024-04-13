//
// MOSI             Pin 7
// SCK              Pin 5
// Data/Command     Pin 61
// Reset            Pin 62
// OledCS           Pin 63
//

// Enable the SPI module clock
MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);

// Reset the peripheral
MAP_PRCMPeripheralReset(PRCM_GSPI);

// Reset SPI
MAP_SPIReset(GSPI_BASE);

// Configure SPI interface
MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
        SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
        (SPI_SW_CTRL_CS |
        SPI_4PIN_MODE |
        SPI_TURBO_OFF |
        SPI_CS_ACTIVEHIGH |
        SPI_WL_8));

// Enable SPI for communication
MAP_SPIEnable(GSPI_BASE);

// Send the string to slave. Chip Select(CS) needs to be
// asserted at start of transfer and deasserted at the end.
MAP_SPITransfer(GSPI_BASE,g_ucTxBuff,g_ucRxBuff,50,
        SPI_CS_ENABLE|SPI_CS_DISABLE);

// Enable Chip select
MAP_SPICSEnable(GSPI_BASE);

while (NOT_END) {
    // Push the character over SPI
    MAP_SPIDataPut(GSPI_BASE, ulUserData);

    // Clean up the receive register into a dummy
    // variable
    MAP_SPIDataGet(GSPI_BASE,&ulDummy);
}

// Disable chip select
MAP_SPICSDisable(GSPI_BASE);