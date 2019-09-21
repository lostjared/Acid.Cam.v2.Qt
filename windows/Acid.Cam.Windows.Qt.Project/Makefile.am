AM_CPPFLAGS= $(all_includes)
lib_LTLIBRARIES=libacidcam.la
libacidcam_includedir= $(includedir)/acidcam
libacidcam_include_HEADERS=ac.h fractal.h ac-filtercat.h
libacidcam_la_SOURCES=fractal.cpp ac-alpha.cpp ac-obj.cpp ac-util.cpp ac-square.cpp ac-particle.cpp ac-grid.cpp ac-basic.cpp ac-filter1.cpp  ac-filter2.cpp ac-filter3.cpp ac-filter4.cpp ac-filter5.cpp ac-filter6.cpp ac-filter7.cpp ac-filter8.cpp ac-filter9.cpp ac-filter10.cpp ac-filter11.cpp ac-image.cpp ac-box.cpp ac-filter12.cpp ac-filter13.cpp ac-filter14.cpp ac-filter15.cpp  ac-filter16.cpp ac-filter17.cpp ac-filter18.cpp ac-filter19.cpp ac-filter20.cpp ac-filter21.cpp ac-filter22.cpp ac-filter23.cpp ac-filter24.cpp ac-filter25.cpp ac-filter26.cpp ac-filter27.cpp ac-filter28.cpp ac-filtercat.cpp 
libacidcam_la_CXXFLAGS=-std=c++11 -O3 $(CV_INCLUDES)
libacidcam_la_LDFLAGS= -version-info 0:1:0
pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA= ../acidcam.pc
