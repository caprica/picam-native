#
# Basic build script for Linux compilation
#
# You should NOT use this, see pi.sh instead
#
VERSION=2.0.1
LIBRARY=picam-$VERSION.so
JNI_INCLUDE=/usr/lib/jvm/default-java/include
JNI_LIB=/usr/lib/jvm/default-java/lib
MMAL_INCLUDE=/disks/store/linux/raspi/userland
OTHER_INCLUDE=/disks/store/linux/raspi/userland/interface/vcos/pthreads
SRC="uk_co_caprica_picam_Camera.c Camera.c Configuration.c Defaults.c Encoder.c Port.c"
gcc -I"$JNI_INCLUDE" -I"$JNI_INCLUDE/linux" -I"$OTHER_INCLUDE" -I"$MMAL_INCLUDE" -L"$JNI_LIB" -o $LIBRARY -shared -Wl,-soname,$LIBRARY $SRC -lc
