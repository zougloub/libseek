#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, sys, re
import numpy as np
import cv2

if __name__ == '__main__':
	images = []

	files = sorted(os.listdir("."))

	for filename in files:
		m = re.match(r"frame-(\d{3})-raw-(?P<type>\d).pgm", filename)
		if m is None:
			continue

		code = int(m.group("type"))

		if code != 3:
			continue

		img = cv2.imread(filename, flags=cv2.IMREAD_ANYDEPTH)
		if img.dtype == np.uint16:
			img = np.float64(img)/65535
		if img.dtype == np.uint8:
			img = np.float64(img)/255
		images.append(img)

	"""
	Find out spots which are always the same...
	These pixels are really dead.
	"""
	img_avg = np.zeros_like(images[0])
	for i, img in enumerate(images):
		img_avg += img
	img_avg /= len(images)

	cv2.imwrite("calib-avg.png", img_avg/img_avg.max()*255)

	img_std = np.zeros_like(images[0])
	for i, img in enumerate(images):
		img_std += (img - img_avg)**2
	img_std = (img_std / len(images)) ** 0.5

	cv2.imwrite("calib-std.png", img_std/img_std.max()*255)

	print("Std. min: %.9f" % img_std.min())
	print("Std. max: %.9f" % img_std.max())
	print("Std. avg: %.9f" % img_std.mean())

	std_threshold = 0.0005
	img_dead = np.zeros_like(img_avg)
	img_dead[img_std < std_threshold] = 1

	cv2.imwrite("calib-bpc-dead.png", img_dead*255)

	raise SystemExit()

	"""
	OK, now what do we do with the rest... the std. image is really
	showing that we need to do something.
	"""

	grid = np.zeros_like(img_avg)
	for i, img in enumerate(images):
		a = np.abs(img-cv2.blur(img, (9,9)))
		cv2.imwrite("grid-%03d.png" % i, a/a.max()*255)

		# don't use right border
		#a[:,-3:] = 0

		grid += a

	thresh = 0.25

	grid[grid>thresh] = 1
	grid[grid<=thresh] = 0

	cv2.imwrite("1grid.png", grid*255)

	if 1:
		for i, img in enumerate(images):
			b = cv2.blur(img, (9,9))
			a = img * (1.0 - grid) + b * (grid)
			cv2.imwrite("frame-%03d-corrected.png" % i, a/a.max()*255)

