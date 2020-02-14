COPY=cp -f
# search through passed header file, find first instance of typedef
# print ifndef __ASSEMBLER__
# with each open bracket incremement brktCnt
# with each close bracket decrement brktCnt
# if brktCnt is zero, we *could* place endif, but that would clutter every
# struct
# instead we check for brktCnt == 0, but also we flag against key words used by
# the assembler, define and ifdef, as these indicate we have entered code that
# can be read by an assembler, so we print endif and then reset, as we expect
# only defines
# we reset so we're ready for the next typdef and with the next typedef we
# print the ifndef __ASSEMBLER__ and repeat the search for the endif
print-% : ;@echo $* = $($*)
AWKFULL=awk 'BEGIN{brktCnt=0;printedAssembler=0;assemblerCode=0} /typedef/ {if(printedAssembler==0) {print "\#ifndef __ASSEMBLER__";printedAssembler=1;assemblerCode=0}}  /\{/ {brktCnt++;assemblerCode=0;} /\}/ {brktCnt--;} /define/ {assemblerCode=1;} /ifdef/ {assemblerCode=1} //{if( (brktCnt == 0) && (printedAssembler==1) && (assemblerCode==1) ) {print "\#endif // __ASSEMBLER__";printedAssembler=0;assemblerCode=0}} END{if(assemblerCode==0) {print "\#endif // __ASSEMBLER__";}}1' < $<

TARGET_TYPE     = dummy
TARGET = reg_def.$(TARGET_TYPE)

DEP_LIST= | \
		  ../inc/leon2_regs.h \
		  ../ipxact_xml/leon2_regs.ipxact.xml \
		  ../inc/uart_regs.h \
		  ../ipxact_xml/uart_regs.ipxact.xml \
		  ../inc/spi_flash_ctrl_regs.h \
		  ../ipxact_xml/spi_flash_ctrl_regs.ipxact.xml \
		  ../inc/i2c_master_regs.h \
		  ../ipxact_xml/i2c_master_regs.ipxact.xml \
		  ../inc/i2c_slave_regs.h \
		  ../ipxact_xml/i2c_slave_regs.ipxact.xml \
		  ../inc/mdio_master_regs.h \
		  ../ipxact_xml/mdio_master_regs.ipxact.xml \
		  ../inc/bb_core_regs.h \
		  ../ipxact_xml/bb_core_regs.ipxact.xml \
		  $(if $(findstring PLATFORM_A7,$(CFLAGS)), ../inc/bb_top_a7_regs.h \
		  ../ipxact_xml/bb_top_a7_regs.ipxact.xml \
		  ../inc/bb_chip_a7_regs.h \
		  ../ipxact_xml/bb_chip_a7_regs.ipxact.xml,) \
		  $(if $(findstring PLATFORM_K7,$(CFLAGS)), ../inc/bb_top_k7_regs.h \
		  ../ipxact_xml/bb_top_k7_regs.ipxact.xml \
		  ../inc/bb_chip_k7_regs.h \
		  ../ipxact_xml/bb_chip_k7_regs.ipxact.xml,) \
 		  ../inc/mca_core_regs.h \
		  ../ipxact_xml/mca_core_regs.ipxact.xml \
		  ../inc/mca_channel_regs.h \
		  ../ipxact_xml/mca_channel_regs.ipxact.xml \
		  ../inc/link_layer_tx_regs.h \
		  ../inc/link_layer_rx_regs.h \
		  ../inc/layer3_tx_regs.h \
		  ../inc/layer3_rx_regs.h \
		  ../inc/dp_aux_hpd_regs.h \
		  ../ipxact_xml/dp_aux_hpd_regs.ipxact.xml \
		  ../inc/dp_sink_regs.h \
		  ../ipxact_xml/dp_sink_regs.ipxact.xml \
		  ../inc/tico_encoder_regs.h \
		  ../ipxact_xml/tico_encoder_regs.ipxact.xml \
		  ../inc/dp_source_regs.h \
		  ../ipxact_xml/dp_source_regs.ipxact.xml \
		  ../inc/tico_decoder_regs.h \
		  ../ipxact_xml/tico_decoder_regs.ipxact.xml \
		  ../inc/ulp_phy_regs.h \
		  ../ipxact_xml/ulp_phy_regs.ipxact.xml \
		  ../inc/ulp_core_regs.h \
		  ../ipxact_xml/ulp_core_regs.ipxact.xml \
		  ../inc/xusb3_regs.h \
		  ../ipxact_xml/xusb3_regs.ipxact.xml \
		  ../inc/upp_regs.h \
		  ../ipxact_xml/upp_regs.ipxact.xml \
		  ../inc/gpio_ctrl_regs.h \
		  ../ipxact_xml/gpio_ctrl_regs.ipxact.xml \
		  ../ipxact_xml/link_layer_tx_regs.ipxact.xml \
		  ../ipxact_xml/link_layer_rx_regs.ipxact.xml \
		  ../ipxact_xml/layer3_tx_regs.ipxact.xml \
		  ../ipxact_xml/layer3_rx_regs.ipxact.xml \
		  ../inc/module_addresses_regs.h
ifneq (,$(findstring BB_ISO,$(CFLAGS)))
RTL_DIR = $(RTL_DIR_RAVEN)
else ifneq (,$(findstring BB_USB,$(CFLAGS)))
RTL_DIR = $(RTL_DIR_RAVEN)
else ifneq (,$(findstring PLATFORM_A7,$(CFLAGS)))
RTL_DIR = $(RTL_DIR_MAVERICK)
endif

# $(info VAR="$(RTL_DIR)")

$(TARGET):
	@touch "../register_definitions_lib/$@"

pre_build: $(DEP_LIST)
	@touch "../register_definitions_lib/$@"

../inc/leon2_regs.h: $(RTL_DIR)/m_leon2-1.0.32-xst/regs/ids/leon2_regs.h
	$(AWKFULL) > $@

../ipxact_xml/leon2_regs.ipxact.xml: $(RTL_DIR)/m_leon2-1.0.32-xst/regs/ids/leon2_regs.ipxact.xml
	$(COPY) $< $@

../inc/uart_regs.h: $(RTL_DIR)/m_uart/regs/ids/uart_regs.h
	$(AWKFULL) > $@

../ipxact_xml/uart_regs.ipxact.xml: $(RTL_DIR)/m_uart/regs/ids/uart_regs.ipxact.xml
	$(COPY) $< $@

../inc/spi_flash_ctrl_regs.h: $(RTL_DIR)/m_spi_flash_ctrl/regs/ids/spi_flash_ctrl_regs.h
	$(AWKFULL) > $@

../ipxact_xml/spi_flash_ctrl_regs.ipxact.xml: $(RTL_DIR)/m_spi_flash_ctrl/regs/ids/spi_flash_ctrl_regs.ipxact.xml
	$(COPY) $< $@

../inc/i2c_master_regs.h: $(RTL_DIR)/m_i2c_master/regs/ids/i2c_master_regs.h
	$(AWKFULL) > $@

../ipxact_xml/i2c_master_regs.ipxact.xml: $(RTL_DIR)/m_i2c_master/regs/ids/i2c_master_regs.ipxact.xml
	$(COPY) $< $@

../inc/i2c_slave_regs.h: $(RTL_DIR)/m_i2c_slave/regs/ids/i2c_slave_regs.h
	$(AWKFULL) > $@

../ipxact_xml/i2c_slave_regs.ipxact.xml: $(RTL_DIR)/m_i2c_slave/regs/ids/i2c_slave_regs.ipxact.xml
	$(COPY) $< $@

../inc/mdio_master_regs.h: $(RTL_DIR)/m_mdio_master/regs/ids/mdio_master_regs.h
	$(AWKFULL) > $@

../ipxact_xml/mdio_master_regs.ipxact.xml: $(RTL_DIR)/m_mdio_master/regs/ids/mdio_master_regs.ipxact.xml
	$(COPY) $< $@

../inc/bb_core_regs.h: $(RTL_DIR)/m_blackbird/regs/ids/bb_core_regs.h
	$(AWKFULL) > $@

../ipxact_xml/bb_core_regs.ipxact.xml: $(RTL_DIR)/m_blackbird/regs/ids/bb_core_regs.ipxact.xml
	$(COPY) $< $@

../inc/bb_top_a7_regs.h: $(RTL_DIR)/f_blackbird_a7_core/regs/ids/bb_top/bb_top_regs.h
	$(AWKFULL) > $@

../ipxact_xml/bb_top_a7_regs.ipxact.xml: $(RTL_DIR)/f_blackbird_a7_core/regs/ids/bb_top/bb_top_regs.ipxact.xml
	$(COPY) $< $@

../inc/bb_chip_a7_regs.h: $(RTL_DIR)/f_blackbird_a7_core/regs/ids/bb_chip/bb_chip_regs.h
	$(AWKFULL) > $@

../ipxact_xml/bb_chip_a7_regs.ipxact.xml: $(RTL_DIR)/f_blackbird_a7_core/regs/ids/bb_chip/bb_chip_regs.ipxact.xml
	$(COPY) $< $@

../inc/bb_top_k7_regs.h: $(RTL_DIR)/f_blackbird_kc705/regs/ids/bb_top/bb_top_regs.h
	$(AWKFULL) > $@

../ipxact_xml/bb_top_k7_regs.ipxact.xml: $(RTL_DIR)/f_blackbird_kc705/regs/ids/bb_top/bb_top_regs.ipxact.xml
	$(COPY) $< $@

../inc/bb_chip_k7_regs.h: $(RTL_DIR)/f_blackbird_kc705/regs/ids/bb_chip/bb_chip_regs.h
	$(AWKFULL) > $@

../ipxact_xml/bb_chip_k7_regs.ipxact.xml: $(RTL_DIR)/f_blackbird_kc705/regs/ids/bb_chip/bb_chip_regs.ipxact.xml
	$(COPY) $< $@

../inc/mca_channel_regs.h: $(RTL_DIR)/m_mca_v2/regs/ids/mca_channel_regs/mca_channel_regs.h
	$(AWKFULL) > $@

../ipxact_xml/mca_channel_regs.ipxact.xml: $(RTL_DIR)/m_mca_v2/regs/ids/mca_channel_regs/mca_channel_regs.ipxact.xml
	$(COPY) $< $@

../inc/mca_core_regs.h: $(RTL_DIR)/m_mca_v2/regs/ids/mca_core_regs/mca_core_regs.h
	$(AWKFULL) > $@

../ipxact_xml/mca_core_regs.ipxact.xml: $(RTL_DIR)/m_mca_v2/regs/ids/mca_core_regs/mca_core_regs.ipxact.xml
	$(COPY) $< $@

../inc/link_layer_tx_regs.h: $(RTL_DIR)/m_link_layer/regs/ids/link_layer_tx/link_layer_tx_regs.h
	$(AWKFULL) > $@

../ipxact_xml/link_layer_tx_regs.ipxact.xml: $(RTL_DIR)/m_link_layer/regs/ids/link_layer_tx/link_layer_tx_regs.ipxact.xml
	$(COPY) $< $@

../inc/link_layer_rx_regs.h: $(RTL_DIR)/m_link_layer/regs/ids/link_layer_rx/link_layer_rx_regs.h
	$(AWKFULL) > $@

../ipxact_xml/link_layer_rx_regs.ipxact.xml: $(RTL_DIR)/m_link_layer/regs/ids/link_layer_rx/link_layer_rx_regs.ipxact.xml
	$(COPY) $< $@

../inc/layer3_tx_regs.h: $(RTL_DIR)/m_layer3/regs/ids/layer3_tx/layer3_tx_regs.h
	$(AWKFULL) > $@

../ipxact_xml/layer3_tx_regs.ipxact.xml: $(RTL_DIR)/m_layer3/regs/ids/layer3_tx/layer3_tx_regs.ipxact.xml
	$(COPY) $< $@

../inc/layer3_rx_regs.h: $(RTL_DIR)/m_layer3/regs/ids/layer3_rx/layer3_rx_regs.h
	$(AWKFULL) > $@

../ipxact_xml/layer3_rx_regs.ipxact.xml: $(RTL_DIR)/m_layer3/regs/ids/layer3_rx/layer3_rx_regs.ipxact.xml
	$(COPY) $< $@

../inc/dp_aux_hpd_regs.h: $(RTL_DIR)/m_aux/regs/ids/dp_aux_hpd_regs.h
	$(AWKFULL) > $@

../ipxact_xml/dp_aux_hpd_regs.ipxact.xml: $(RTL_DIR)/m_aux/regs/ids/dp_aux_hpd_regs.ipxact.xml
	$(COPY) $< $@

../inc/dp_sink_regs.h: $(RTL_DIR)/m_dp_sink/regs/ids/dp_sink_regs.h
	$(AWKFULL) > $@

../ipxact_xml/dp_sink_regs.ipxact.xml: $(RTL_DIR)/m_dp_sink/regs/ids/dp_sink_regs.ipxact.xml
	$(COPY) $< $@

../inc/tico_encoder_regs.h: $(RTL_DIR)/m_dp_sink/regs/ids/tico_encoder/tico_encoder_regs.h
	$(AWKFULL) > $@

../ipxact_xml/tico_encoder_regs.ipxact.xml: $(RTL_DIR)/m_dp_sink/regs/ids/tico_encoder/tico_encoder_regs.ipxact.xml
	$(COPY) $< $@

../inc/dp_source_regs.h: $(RTL_DIR)/m_dp_source/regs/ids/dp_source_regs.h
	$(AWKFULL) > $@

../ipxact_xml/dp_source_regs.ipxact.xml: $(RTL_DIR)/m_dp_source/regs/ids/dp_source_regs.ipxact.xml
	$(COPY) $< $@

../inc/tico_decoder_regs.h: $(RTL_DIR)/m_dp_source/regs/ids/tico_decoder/tico_decoder_regs.h
	$(AWKFULL) > $@

../ipxact_xml/tico_decoder_regs.ipxact.xml: $(RTL_DIR)/m_dp_source/regs/ids/tico_decoder/tico_decoder_regs.ipxact.xml
	$(COPY) $< $@

../inc/ulp_phy_regs.h: $(RTL_DIR)/m_ulp/regs/ids/ulp_phy/ulp_phy_regs.h
	$(AWKFULL) > $@

../ipxact_xml/ulp_phy_regs.ipxact.xml: $(RTL_DIR)/m_ulp/regs/ids/ulp_phy/ulp_phy_regs.ipxact.xml
	$(COPY) $< $@

../inc/ulp_core_regs.h: $(RTL_DIR)/m_ulp/regs/ids/ulp_core/ulp_core_regs.h
	$(AWKFULL) > $@

../ipxact_xml/ulp_core_regs.ipxact.xml: $(RTL_DIR)/m_ulp/regs/ids/ulp_core/ulp_core_regs.ipxact.xml
	$(COPY) $< $@

../inc/xusb3_regs.h: $(RTL_DIR)/m_xusb3/regs/ids/xusb3_regs.h
	$(AWKFULL) > $@

../ipxact_xml/xusb3_regs.ipxact.xml: $(RTL_DIR)/m_xusb3/regs/ids/xusb3_regs.ipxact.xml
	$(COPY) $< $@

../inc/upp_regs.h: $(RTL_DIR)/m_upp/regs/ids/upp_regs.h
	$(AWKFULL) > $@

../ipxact_xml/upp_regs.ipxact.xml: $(RTL_DIR)/m_upp/regs/ids/upp_regs.ipxact.xml
	$(COPY) $< $@

../inc/gpio_ctrl_regs.h: $(RTL_DIR)/m_gpio_ctrl/regs/ids/gpio_ctrl_regs.h
	$(AWKFULL) > $@

../ipxact_xml/gpio_ctrl_regs.ipxact.xml: $(RTL_DIR)/m_gpio_ctrl/regs/ids/gpio_ctrl_regs.ipxact.xml
	$(COPY) $< $@

../inc/module_addresses_regs.h: ../inc/bb_chip_a7_regs.h
	python ../../../fg_icron_files/generate_module_addresses.py

clean:
	$(COPY) ../inc/bb_top_regs.h ../inc/bb_top_KEEP.h
	$(COPY) ../inc/bb_chip_regs.h ../inc/bb_chip_KEEP.h
	$(RM) -r ../inc/*_regs.h
	$(RM) -r ../ipxact_xml/*_regs.ipxact.xml
	$(RM) -rf ../*_lib
	$(COPY) ../inc/bb_top_KEEP.h ../inc/bb_top_regs.h
	$(COPY) ../inc/bb_chip_KEEP.h ../inc/bb_chip_regs.h

.PHONY: pre_build $(TARGET)

include $(IBUILD_RULES)
