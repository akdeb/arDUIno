#include "AudioTools.h"
#include "BluetoothA2DPSink.h"

I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

void setup() {
    auto cfg = i2s.defaultConfig();
    cfg.pin_bck = 27;
    cfg.pin_ws = 26;
    cfg.pin_data = 25;
    i2s.begin(cfg);

    a2dp_sink.start("ESP_32");
}

void loop() {
}