##
## --static

# can only work for android 8 up without --static
arm-linux-androideabi-gcc -D_IMX350_ -I includes/  -I sensors/includes/ -I libc/kernel/ -I libc/include/ -I kernel-headers/ -I ./libc/kernel/android/uapi/ -I ./libc/kernel/uapi/ -I./ -I ./libc/kernel/uapi/asm-arm/ -I./camsdk/includes -I./sensors/module/ -I./camsdk/includes/0310 --sysroot=/usr/local/platforms/android-23/arch-arm/  -llog -pie -fPIE -fPIC ov.c  -o libmmcamera
#arm-linux-androideabi-gcc -D_IMX362_ -I includes/  -I sensors/includes/ -I libc/kernel/ -I libc/include/ -I kernel-headers/ -I ./libc/kernel/android/uapi/ -I ./libc/kernel/uapi/ -I ./libc/kernel/uapi/asm-arm/ --sysroot=/usr/local/platforms/android-23/arch-arm/  -fPIE  ov.c --shared -shared -o libmmcamera_imx362.so 
#arm-linux-androideabi-gcc -D_S5K4H7YX_ -I includes/  -I sensors/includes/ -I libc/kernel/ -I libc/include/ -I kernel-headers/ -I ./libc/kernel/android/uapi/ -I ./libc/kernel/uapi/ -I ./libc/kernel/uapi/asm-arm/ --sysroot=/usr/local/platforms/android-23/arch-arm/  -fPIE  ov.c --shared -shared -o libmmcamera_s5k4h7yx.so 

#arm-linux-androideabi-gcc -rdynamic -fPIE --static -static ov.o -o a.out /usr/local/platforms/android-23/arch-arm/usr/lib/libdl.a --sysroot=/usr/local/platforms/android-23/arch-arm/
#~/adb push a.out /
#~/adb shell chmod a+x /a.out
#~/adb shell /a.out

#/usr/local/lib/gcc/arm-linux-androideabi/4.9.x/armv7-a/libgcc.a /usr/local/platforms/android-23/arch-arm/usr/lib/libc.a
