Import("env")
import os

def clean_flash_bins(*args, **kwargs):
    print("Cleaning config folder")
    
    nvs_data_dir = os.path.join(env["PROJECT_DIR"], env.GetProjectOption("custom_nvs_data_dir"))
    nvs_config = os.path.join(nvs_data_dir, env.GetProjectOption("custom_nvs_config"))
    nvs_bin = os.path.join(nvs_data_dir, env.GetProjectOption("custom_nvs_bin"))
    try:
        os.remove(nvs_bin)
    except:
        print()
    
def build_flash(*args, **kwargs):
    nvs_data_dir = os.path.join(env["PROJECT_DIR"], env.GetProjectOption("custom_nvs_data_dir"))
    nvs_config = os.path.join(nvs_data_dir, env.GetProjectOption("custom_nvs_config"))
    nvs_bin = os.path.join(nvs_data_dir, env.GetProjectOption("custom_nvs_bin"))
    nvs_generator = os.path.join(env["PROJECT_PACKAGES_DIR"], 
                                 "framework-espidf",
                                 "components",
                                 "nvs_flash",
                                 "nvs_partition_generator",
                                 "nvs_partition_gen.py")
    os.system(nvs_generator + " generate " + nvs_config + " " + nvs_bin + " 0x3000")

def flash_nvs(*args, **kwargs):
    nvs_bin = os.path.join(env["PROJECT_DIR"], env.GetProjectOption("custom_nvs_data_dir"), env.GetProjectOption("custom_nvs_bin"))
    uploader_port = env.GetProjectOption("custom_nvs_port")
    os.system("esptool.py -p "+ uploader_port + " write_flash 0x9000 " + nvs_bin)

# Multiple actions
env.AddCustomTarget(
    name="build-flash",
    dependencies=None,
    actions=[clean_flash_bins, build_flash],
    title="Compile preferences",
    description="Clean-compile preferences"
)

env.AddCustomTarget(
    name="flash nvs",
    dependencies="build-flash",
    actions=[flash_nvs],
    title="Flash preferences",
    description="Flash preferences"
)