#pylint: disable = I,E,R,W,C,F
from os.path import isfile
from pathlib import Path

Import("env")  # type: ignore

WORKAROUND_TAG = "[ESP32-P4 SDMMC workaround]"


def patch_arduino_sd_mmc_for_hosted():
    pioenv = env.get("PIOENV", "")
    if not pioenv.startswith("esp32p4"):
        return

    framework_dir = env.PioPlatform().get_package_dir("framework-arduinoespressif32")
    if not framework_dir:
        return

    sd_mmc_cpp = Path(framework_dir) / "libraries" / "SD_MMC" / "src" / "SD_MMC.cpp"
    if not sd_mmc_cpp.is_file():
        print(f"{WORKAROUND_TAG} SD_MMC.cpp not found, skipping patch")
        return

    print(f"{WORKAROUND_TAG} Checking Arduino SD_MMC compatibility for ESP-Hosted over SDIO")
    print(f"{WORKAROUND_TAG} Reason: IDF v6 shared SDMMC host init conflict (see ESP-IDF issue #16233)")
    print(f"{WORKAROUND_TAG} Scope: ESP32-P4 environments only")

    original = sd_mmc_cpp.read_text(encoding="utf-8")
    updated = original

    include_anchor = '#include "esp32-hal-periman.h"\n'
    workaround_include = '''#include "esp32-hal-periman.h"\n#include "esp_idf_version.h"\n\n#if CONFIG_ESP_HOSTED_SDIO_HOST_INTERFACE && defined(CONFIG_IDF_TARGET_ESP32P4) && (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0))\n#define MWR_SDMMC_HOSTED_WORKAROUND 1\n#else\n#define MWR_SDMMC_HOSTED_WORKAROUND 0\n#endif\n'''
    if "MWR_SDMMC_HOSTED_WORKAROUND" not in updated:
        updated = updated.replace(include_anchor, workaround_include, 1)

    using_anchor = "using namespace fs;\n"
    workaround_functions = '''using namespace fs;\n\n#if MWR_SDMMC_HOSTED_WORKAROUND\nstatic esp_err_t mwr_sdmmc_host_init_dummy(void) {\n  return ESP_OK;\n}\n\nstatic esp_err_t mwr_sdmmc_host_deinit_dummy(void) {\n  return ESP_OK;\n}\n#endif\n'''
    if "mwr_sdmmc_host_init_dummy" not in updated:
        updated = updated.replace(using_anchor, workaround_functions, 1)

    slot_anchor = '''#else\n  host.slot = SDMMC_HOST_SLOT_1;\n#endif\n  host.max_freq_khz = sdmmc_frequency;\n'''
    slot_replacement = '''#else\n  host.slot = SDMMC_HOST_SLOT_1;\n#endif\n#if MWR_SDMMC_HOSTED_WORKAROUND\n  if (host.slot == SDMMC_HOST_SLOT_0) {\n    host.init = &mwr_sdmmc_host_init_dummy;\n    host.deinit = &mwr_sdmmc_host_deinit_dummy;\n  }\n#endif\n  host.max_freq_khz = sdmmc_frequency;\n'''
    if "host.init = &mwr_sdmmc_host_init_dummy;" not in updated:
        updated = updated.replace(slot_anchor, slot_replacement, 1)

    if updated != original:
        sd_mmc_cpp.write_text(updated, encoding="utf-8")
        print(f"{WORKAROUND_TAG} Applied patch to Arduino SD_MMC.cpp")
    else:
        print(f"{WORKAROUND_TAG} Already patched; no changes required")


patch_arduino_sd_mmc_for_hosted()

if isfile(".env"):
    try:
        f = open(".env", "r")
        lines = f.readlines()
        envs = []
        for line in lines:
            line.strip()
            if line.startswith('#'):
                continue
            envs.append("-D{}".format(line.strip()))
        env.Append(BUILD_FLAGS=envs) # type: ignore
    except IOError:
        print("File .env not accessible",)
    finally:
        f.close()
