#!/usr/bin/python

import shutil
import argparse
import os

working_directory = ""

lirc_replacement_sign = "__LIRC_DEVICE_FOR_REPLACE__"
spi_replacement_sign = "__SPI_DEVICE_FOR_REPLACE__"
irq_replacement_sign = "__IRQ_FOR_REPLACE__"
version_replacement_sign="__WB_VERSION_FOR_REPLACE__"

config_name_pref = "wb-homa-rfsniffer.conf"
generated_config_name_suffix = "generated"
config_template_name = config_name_pref + ".template"


def generate_config(version, lirc_device="/dev/lirc0", spi_device=None, spi_major=32766, spi_minor=0, irq=38):
    if (spi_device == None):
        spi_device = "/dev/spidev%d.%d" % (spi_major, spi_minor)
    
    config_name = working_directory + config_name_pref + "." + version + "." + generated_config_name_suffix
    
    # open template file
    config = open(working_directory + config_template_name, "r").read()
    
    # change special substrings to right data
    config = config.replace(spi_replacement_sign, spi_device)
    config = config.replace(lirc_replacement_sign, lirc_device)
    config = config.replace(irq_replacement_sign, str(irq))
    config = config.replace(version_replacement_sign, version)
    
    # save generated config
    config_file = open(config_name, "w")
    config_file.write(config)
    
    print("Generated " + config_name);




# create the top-level parser
parser = argparse.ArgumentParser(prog='generate_configs.py')
parser.add_argument('--directory', type=str, help='Directory where configs template is and generated configs will be stored')

group = parser.add_mutually_exclusive_group(required=True)
group.add_argument('--generate', action='store_true', help=' to generate configs')
group.add_argument('--clean', action='store_true', help=' to remove generated configs')

args = parser.parse_args()
#print(args)

working_directory = args.directory
if (working_directory[-1] != '/'):
    working_directory += '/'
    
if (args.generate):
    # supported
    generate_config("52")
    generate_config("55")
    generate_config("58")

    # may be supported
    generate_config("32", spi_minor=5, irq=36)
    generate_config("41", spi_minor=4, irq=92)
    generate_config("50")

    generate_config("default")

if (args.clean):
    os.system("rm -vf " + working_directory + "*." + generated_config_name_suffix)
