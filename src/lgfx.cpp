#include "lgfx.h"

LGFX::LGFX() {
  setBusInstance();
  setPanelInstance();
  setPanel(&_panel_instance);
}

void LGFX::setBusInstance() {
  auto cfg = _bus_instance.config();

  // SPI
  // cfg.spi_host = SPI1_HOST;
  // cfg.spi_mode = 0;             // SPI (0 ~ 3)
  cfg.freq_write = SPI_FREQUENCY;
  cfg.freq_read = SPI_READ_FREQUENCY;
  cfg.spi_3wire = true;
  cfg.use_lock = true;
  // cfg.dma_channel = SPI_DMA_CH_AUTO; //(0=DMA/ 1=1ch / 2=ch / SPI_DMA_CH_AUTO)

  cfg.pin_sclk = TFT_SCLK;
  cfg.pin_mosi = TFT_MOSI;
  cfg.pin_miso = -1;
  cfg.pin_dc = TFT_DC;

  _bus_instance.config(cfg);
  _panel_instance.setBus(&_bus_instance);
}

void LGFX::setPanelInstance() {
  auto cfg = _panel_instance.config();

  cfg.pin_cs = TFT_CS;
  cfg.pin_rst = TFT_RST;
  cfg.pin_busy = -1; // (-1 = disable)

  cfg.panel_width = TFT_WIDTH;
  cfg.panel_height = TFT_HEIGHT;
  cfg.memory_width = TFT_WIDTH;
  cfg.memory_height = TFT_HEIGHT;
  cfg.offset_x = 0;
  cfg.offset_y = 0;
  cfg.offset_rotation = 0; // 0~7
  cfg.dummy_read_pixel = 8;
  cfg.dummy_read_bits = 1;
  cfg.readable = true;
  cfg.invert = true;
  cfg.rgb_order = false;
  cfg.dlen_16bit = false;
  cfg.bus_shared = true;

  _panel_instance.config(cfg);
}
