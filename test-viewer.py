#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, sys, re, subprocess
import numpy as np
import cv2

if __name__ == '__main__':
	cmd = ['./build/seek-test']
	proc = subprocess.Popen(cmd,
	 stdout=subprocess.PIPE,
	)

	grid_fn = '1grid.png'
	if os.path.exists(grid_fn):
		grid = cv2.imread('1grid.png')/255
		grid, a, b = cv2.split(grid)
	else:
		grid = np.zeros((156, 208))

	cv2.namedWindow('seek', cv2.WINDOW_NORMAL)

	_min = 0x7e00
	def minchange(x):
		global _min
		print("min=%s" % x)
		_min = x

	cv2.createTrackbar('min', 'seek', _min, 0xffff, minchange)

	_max = 0x8200
	def maxchange(x):
		global _max
		print("max=%s" % x)
		_max = x

	cv2.createTrackbar('max', 'seek', _max, 0xffff, maxchange)

	try:
		while True:
			data = proc.stdout.read(208*156*2)
			img = np.fromstring(data, dtype='<H').reshape((156, 208))
			img = np.float32(img)

			img -= _min
			img[img<0] = 0
			img /= (_max-_min)

			b = cv2.blur(img, (9,9))
			a = img * (1.0 - grid) + b * (grid)

			if 1:
				a = np.rot90(a, 3)

			cv2.imshow('seek', a)
			key = cv2.waitKey(1)
			if key == 113: # q
				break
	except:
		raise
	finally:
		proc.kill()
		proc.wait()


