This is a tool to load qcom libmmcamera sensor lib under arm/arm64 android8+ and dump the initialization register content of it. 
Also, this is a stub lib and wrapper for loading older sensor library too for example provided a rectified struct sensor_lib_t 
and recompile as share library.

To compile:
Download Android gcc from below:
https://android.googlesource.com/platform/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/+/refs/heads/android10-gsi

Also download Android sysroot too:
search from google "android-23 crtbegin_dynamic.o" or download from
https://github.com/EdgeApp/android-ndk-linux

A copy of camsdk is needed. A forked copy is at:
https://github.com/99degree/chromatix_demo/
https://github.com/99degree/chromatix_demo/blob/master/mm-camerasdk.zip

download it and put mm-camerasdk.zip\mm-camerasdk\includes\sdm660\* under camsdk/include/

search and modify below, as results:
#define MAX_RESOLUTION_MODES 14

At sensor_lib.h find below and add the line "char dummy":
typedef struct {
  /* sensor slave info */
  struct camera_sensor_slave_info sensor_slave_info;

char dummy[0x5fa4 - 4 - sizeof(struct camera_sensor_slave_info)];

.....


~/libmmcamera_loader# ./build_bin.sh

done!

.....

Usage:
/debug_ramdisk# libmmcamera imx350

and below will appear:
__attach()
Hello world ./libmmcamera imx350
usage: ./libmmcamera [cmos|imx350]
default 'imx350' for libmmcamera_imx350.so
e.g. ./libmmcamera s5k3p3sm for libmmcamera_s5k3p3sm.so
enter
lib_name ./libmmcamera_imx350.so
exit
sensor_load_library, return 0
msm_camera_sensor_slave_info:
  0000  69 6d 78 33 35 30 00 00 00 00 00 00 00 00 00 00  imx350..........

A sample output file dump is also available as imx350.txt, also the so library file also provided at bin/.

