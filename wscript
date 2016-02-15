#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, os, re
from waflib import Options

def options(opt):
	opt.load('compiler_cxx')

def configure(conf):
	conf.load('compiler_cxx')

	if conf.env.CXX_NAME in ('gcc', 'clang'):
		conf.env.CXXFLAGS += [ '-std=c++11', '-fpic' ]

	conf.check_cfg(package='libusb-1.0', args='--cflags --libs',
	 uselib_store="LIBUSB")

def build(bld):
	bld(
	 target='seek',
	 features='cxx cxxstlib',
	 source=[
	  'seek.cpp',
	 ],
	 use='LIBUSB',
	 install_path="${LIBDIR}",
	)

	bld.install_files("${PREFIX}/include",
	 ['seek.hpp', 'seek_pimpl_h.hpp'])

	bld(
	 source='seek.pc.in',
	 install_path='${LIBDIR}/pkgconfig/',
	)

	bld(
	 target='seek-test',
	 features='cxx cxxprogram',
	 source='seek-test.cpp',
	 use='seek',
	)

	bld(
	 target='seek-test-calib',
	 features='cxx cxxprogram',
	 source='seek-test-calib.cpp',
	 use='seek',
	)

