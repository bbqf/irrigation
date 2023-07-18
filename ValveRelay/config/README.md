# How to prepare and flash the configuration
## Preparation
1. Rename `sensor_config.csv.tmpl` to `sensor_config.csv`
1. Rename `sensor_data.csv` to `sensor_data.csv`
1. Check and change the `ssid`, `password`, `mqtt*` and `ntpServer` data values in the `sensor_data.csv`
1. Start the tool [mfg_gen.py](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/mass_mfg.html) like
    
    mass_mfg.py generate sensor_config.csv sensor_data.csv VR 0x3000
1. Make sure the tool doesn't report any errors and `bin/VR.bin` file is successfully created
## Flashing
1. Flash the resulted `bin/VR-1.bin` file with the esptool.py at the address **0x9000**
    
    esptool.py -p COM9 write_flash 0x9000 bin\VR-1.bin
