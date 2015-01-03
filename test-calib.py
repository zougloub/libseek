#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, sys, re
import numpy as np
import cv2

if __name__ == '__main__':
	images = []
	for i in range(1000):
		filename = 'frame-%03d.pgm' % i
		if not os.path.isfile(filename):
			print("Stop at %d" % i)
			break
		img = np.float32(cv2.imread(filename))/65535
		images.append(img)
	
	# Find out spots which are always the same
	s = np.zeros_like(images[0])
	for i, img in enumerate(images):
		s += img
	
	s /= len(images)

	cv2.imwrite("avg.png", s/s.max()*255)


	grid = np.zeros_like(s)
	for i, img in enumerate(images):
		a = np.abs(img-cv2.blur(img, (9,9)))
		cv2.imwrite("grid-%03d.png" % i, a/a.max()*255)

		# don't use right border
		a[:,-3:] = 0

		grid += a

	grid[grid>0.5] = 1
	grid[grid<=0.5] = 0

	grid /= grid.max()
	cv2.imwrite("1grid.png", grid*255)

	if 1:
		for i, img in enumerate(images):
			b = cv2.blur(img, (9,9))
			a = img * (1.0 - grid) + b * (grid)
			cv2.imwrite("frame-%03d-corrected.png" % i, a/a.max()*255)

