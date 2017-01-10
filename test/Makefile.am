bin_PROGRAMS = audiocapturemgrtestapp acm_ipout_testapp acm_musicid_testapp
audiocapturemgrtestapp_SOURCES = rmfAudioCaptureTestApp.cpp
audiocapturemgrtestapp_CPPFLAGS = -I$(top_srcdir)/include -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/media-utils/audioCapture/ -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/
audiocapturemgrtestapp_LDADD =  ${top_builddir}/src/libaudiocapturemgr.la

acm_ipout_testapp_SOURCES = ipOutTestApp.cpp
acm_ipout_testapp_CPPFLAGS = -I$(top_srcdir)/include -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/media-utils/audioCapture/ -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rdk/iarmbus/ -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/
acm_ipout_testapp_LDADD =  -L${RDK_FSROOT_PATH}/usr/local/lib -L${RDK_FSROOT_PATH}/usr/lib -lpthread -lIARMBus

acm_musicid_testapp_SOURCES = musicIdTestApp.cpp 
acm_musicid_testapp_CPPFLAGS = -I$(top_srcdir)/include -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/media-utils/audioCapture/ -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rdk/iarmbus/ -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/
acm_musicid_testapp_LDADD =  -L${RDK_FSROOT_PATH}/usr/local/lib -L${RDK_FSROOT_PATH}/usr/lib -lIARMBus
