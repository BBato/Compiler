deps_config := \
	/home/baranb2/esp/esp-idf/components/app_trace/Kconfig \
	/home/baranb2/esp/esp-idf/components/aws_iot/Kconfig \
	/home/baranb2/esp/esp-idf/components/bt/Kconfig \
	/home/baranb2/esp/esp-idf/components/driver/Kconfig \
	/home/baranb2/esp/esp-idf/components/esp32/Kconfig \
	/home/baranb2/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/home/baranb2/esp/esp-idf/components/esp_event/Kconfig \
	/home/baranb2/esp/esp-idf/components/esp_http_client/Kconfig \
	/home/baranb2/esp/esp-idf/components/esp_http_server/Kconfig \
	/home/baranb2/esp/esp-idf/components/ethernet/Kconfig \
	/home/baranb2/esp/esp-idf/components/fatfs/Kconfig \
	/home/baranb2/esp/esp-idf/components/freemodbus/Kconfig \
	/home/baranb2/esp/esp-idf/components/freertos/Kconfig \
	/home/baranb2/esp/esp-idf/components/heap/Kconfig \
	/home/baranb2/esp/esp-idf/components/libsodium/Kconfig \
	/home/baranb2/esp/esp-idf/components/log/Kconfig \
	/home/baranb2/esp/esp-idf/components/lwip/Kconfig \
	/home/baranb2/esp/esp-idf/components/mbedtls/Kconfig \
	/home/baranb2/esp/esp-idf/components/mdns/Kconfig \
	/home/baranb2/esp/esp-idf/components/mqtt/Kconfig \
	/home/baranb2/esp/esp-idf/components/nvs_flash/Kconfig \
	/home/baranb2/esp/esp-idf/components/openssl/Kconfig \
	/home/baranb2/esp/esp-idf/components/pthread/Kconfig \
	/home/baranb2/esp/esp-idf/components/spi_flash/Kconfig \
	/home/baranb2/esp/esp-idf/components/spiffs/Kconfig \
	/home/baranb2/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/home/baranb2/esp/esp-idf/components/vfs/Kconfig \
	/home/baranb2/esp/esp-idf/components/wear_levelling/Kconfig \
	/home/baranb2/esp/gas-esp-idf/components/arduino/Kconfig.projbuild \
	/home/baranb2/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/baranb2/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/baranb2/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/baranb2/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
