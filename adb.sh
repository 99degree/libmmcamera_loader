~/adb root
~/adb shell mount tmpfs -t tmpfs /debug_ramdisk
~/adb shell mount /vendor -o remount,rw
~/adb push libmmcamera /debug_ramdisk
~/adb shell chmod a+x /debug_ramdisk/libmmcamera
cd bin
~/adb push libmmcamera_imx362.so /debug_ramdisk/libmmcamera_imx362.so
~/adb push libmmcamera_imx350.so /debug_ramdisk/libmmcamera_imx350.so
~/adb push libmmcamera_imx362_rebuild.so /debug_ramdisk/libmmcamera_imx362_1.so
~/adb push libmmcamera_imx362_gt24c64a.so /debug_ramdisk/
#~/adb shell cp /vendor/lib/libmmcamera_imx362.so /debug_ramdisk/libmmcamera_imx362.so
