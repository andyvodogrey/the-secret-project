
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {

public:
  LGFX();

private:
  lgfx::Panel_GC9A01 _panel_instance;
  lgfx::Bus_SPI _bus_instance;

  void setBusInstance();
  void setPanelInstance();
};
