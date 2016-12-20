import shutil

lirc_replacement_sign = "__LIRC_DEVICE_FOR_REPLACE__"
spi_replacement_sign = "__SPI_DEVICE_FOR_REPLACE__"
irq_replacement_sign = "__IRQ_FOR_REPLACE__"
version_replacement_sign="__WB_VERSION_FOR_REPLACE__"

config_name_pref = "wb-homa-rfsniffer.conf"
config_template_name = config_name_pref + ".template"


def generate_config(version, lirc_device="/dev/lirc0", spi_device=None, spi_major=32766, spi_minor=0, irq=38):
    if (spi_device == None):
        spi_device = "/dev/spidev%d.%d" % (spi_major, spi_minor)
    
    config_name = config_name_pref + "." + version
    
    config = open(config_template_name, "r").read()
    config = config.replace(spi_replacement_sign, spi_device)
    config = config.replace(lirc_replacement_sign, lirc_device)
    config = config.replace(irq_replacement_sign, str(irq))
    config = config.replace(version_replacement_sign, version)
    config_file = open(config_name, "w")
    config_file.write(config)

# supported
generate_config("wb58")
generate_config("wb55")
generate_config("wb52") 

# may be supported
generate_config("wbsh3", spi_minor=5, irq=36) # smth
generate_config("wbsh5") # smth

# probably not supported
generate_config("cqc10")
generate_config("wbsh4")
generate_config("mka3")


