lib_LTLIBRARIES = libaudiocapturemgr.la
libaudiocapturemgr_la_SOURCES = audio_buffer.cpp  audio_capture_manager.cpp  music_id.cpp 
libaudiocapturemgr_la_CPPFLAGS = -I$(top_srcdir)/include -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/media-utils/audioCapture/
libaudiocapturemgr_la_LIBADD = -lrmfAudioCapture -lpthread

bin_PROGRAMS = audiocapturemgr
audiocapturemgr_SOURCES =  acm_iarm_interface.cpp acm_main.cpp 
audiocapturemgr_CPPFLAGS = -I$(top_srcdir)/include -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/media-utils/audioCapture/ -I$(PKG_CONFIG_SYSROOT_DIR)${includedir}/rdk/iarmbus/
audiocapturemgr_LDADD =  libaudiocapturemgr.la -L${RDK_FSROOT_PATH}/usr/lib -L${RDK_FSROOT_PATH}/usr/local/lib -lIARMBus
