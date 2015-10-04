#!/usr/bin/env python
# -*- coding: utf-8 vi:ts=4:noet
# Bad pixel correction calibration for seek thermal camera

import sys, os, re, math
import pickle
import numpy as np
import cv2

class BPC_Static(object):
	"""
	Perform bad-pixels calibration and correction,
	assuming images have pixels in 0-1 range.
	"""
	def __init__(self, img):
		self._img = img
		self._bad_crazy = {}

	def identify_crazy(self):
		img = self._img
		h, w = img.shape

		def has_good_in_line(x1,y1,x2,y2):
			beg = x1,y1
			end = x2,y2

			#print("Paint from %d,%d to %d,%d" % (x1,y1,x2,y2))
			dx = x2 - x1
			dy = y2 - y1
			is_steep = abs(dy) > abs(dx)

			if is_steep:
				x1, y1 = y1, x1
				x2, y2 = y2, x2

			swapped = False
			if x1 > x2:
				x1, x2 = x2, x1
				y1, y2 = y2, y1
				swapped = True

			dx = x2 - x1
			dy = y2 - y1

			error = int(dx / 2.0)
			ystep = 1 if y1 < y2 else -1

			y = y1
			for x in range(x1, x2 + 1):
				xp, yp = (y, x) if is_steep else (x, y)

				if (xp,yp) not in (beg, end):
					#print("Paint %d,%d" % (xp,yp))
					if img[yp,xp] == 0:
						return True

				error -= abs(dy)
				if error < 0:
					y += ystep
					error += dx

			return False

		# max distance to look for good pixels
		max_d = math.hypot(w,h)
		max_d = 10

		bpc = []
		for y in range(0, h):
			for x in range(0, w):
				if img[y,x] == 1:# and (x,y) not in self._bad_single:
					bpc.append((x,y))

		img_dbg = np.zeros_like(img, dtype=np.float32)
		for x,y in bpc:
			img_dbg[y,x] = 1
		cv2.imwrite("bpc-crazy.png", img_dbg*255)

		for idx_bpc, (x,y) in enumerate(sorted(bpc)):
			"""
			Go in every possible direction to find a contour
			"""

			#if idx_bpc > 10:
			#	break

			print("[%03d/%03d] Finding recipe for bad pixel at (%d,%d)" \
			 % (idx_bpc+1, len(bpc), x,y))

			img_dbg = np.zeros_like(img, dtype=np.float32) + 0.1
			img_dbg[y,x] = 0.5
			exploratory_angle_ranges = [(0,math.pi*2)]
			contour = []
			y0, x0 = y, x
			contour = set()

			last_d = 0
			current_d = 1
			while True:
				#print("Current distance: %.3f" % (current_d))

				for y in range(h):
					for x in range(w):
						dist = math.hypot(y-y0,x-x0)
						if dist > last_d and dist <= current_d:
							if img[y,x] == 0: # good pixel
								"""
								Draw line to current point; if we don't
								encounter pixels in the countour, this
								pixel is part of contour
								"""
								if not has_good_in_line(x0,y0,x,y):
									contour.update([(x,y)])
									img_dbg[y,x] = 1.0

				# Found that we need to expand.. find the next distance
				next_d = math.hypot(w,h)
				for y in range(h):
					for x in range(w):
						dist = math.hypot(y-y0,x-x0)
						if dist > current_d and dist < next_d:
							next_d = dist

				if next_d > max_d:
					break

				last_d = current_d
				current_d = next_d

			weights = []
			for x, y in contour:
				dist = math.hypot(y-y0,x-x0)
				weight = 1.0/dist
				weights.append(weight)
			weights = np.array(weights)
			weights /= sum(weights)

			for idx_ct, (x,y) in enumerate(contour):
				print("- (%03d,%03d): %f" % (x,y,weights[idx_ct]))

			self._bad_crazy[x0,y0] = [ ((x-x0,y-y0), weights[idx_ct]) for idx_ct, (x,y) in enumerate(contour) ]

			cv2.imwrite("bpc-%03d-%03d.png" % (x0,y0), img_dbg*255)

	def correct_crazy(self, img):
		for x0, y0 in self._bad_crazy:
			v = 0
			for (x,y), w in self._bad_crazy[x0,y0]:
				v += img[y0+y,x0+x] * w
			img[y0,x0] = v

	def identify(self):
		self.identify_crazy()

	def correct(self, img):
		self.correct_crazy(img)

	def load(self):
		with open("bad_crazy.pickle", "r") as f:
			self._bad_crazy = pickle.load(f)

	def save(self):
		with open("bad_crazy.pickle", "w") as f:
			pickle.dump(self._bad_crazy, f)

if __name__ == '__main__':
	"""
	Process the file that was generated from test-calib.py
	"""
	img = np.float32(cv2.imread("1grid.png", cv2.IMREAD_GRAYSCALE))/255

	bpc = BPC_Static(img)

	"""
	bpc.identify()
	bpc.save()
	"""

	bpc.load()

	bpc_kinds = {}
	for x0, y0 in bpc._bad_crazy:
		print("Bad pixel %d,%d" % (x0,y0))
		for idx_ct, ((x,y), w) in enumerate(bpc._bad_crazy[x0,y0]):
			print(" - %3d,%3d: %f" % (x,y,w))
		k = tuple(sorted(bpc._bad_crazy[x0,y0]))
		bpc_kinds.setdefault(k, 0)
		bpc_kinds[k] += 1

	print(len(bpc_kinds))

	for k, v in sorted(bpc_kinds.items(), key=lambda x:x[1], reverse=True)[:10]:
		print(k, v)


	"""
	Store weights, kinds, and values
	"""
	with open("seek_bpc_2.dat", "w") as f:
		def log(x):
			f.write(x)

		ws = []
		for x0, y0 in sorted(bpc._bad_crazy, key=lambda x: (x[1], x[0])):
			for ((x,y), w) in sorted(bpc._bad_crazy[x0,y0], key=lambda x: (x[0][1],x[0][0])):
				if w not in ws:
					ws.append(w)
		ws.sort()
		log("%d\n" % len(ws))
		for w in ws:
			log("%f\n" % (w))

		log("%d\n" % len(bpc_kinds))
		bpc_kinds2 = list(bpc_kinds.keys())
		for cnts in bpc_kinds2:
			log("%d " % (len(cnts)))
			for ((x,y), w) in sorted(cnts, key=lambda x: (x[0][1],x[0][0])):
				log(" %3d %3d %d" % (y,x,ws.index(w)))
			log("\n")

		log("%d\n" % len(bpc._bad_crazy))
		for x0, y0 in sorted(bpc._bad_crazy, key=lambda x: (x[1], x[0])):
			cnts = tuple(sorted(bpc._bad_crazy[x0,y0]))
			log("%3d %3d %d\n" % (y0,x0, bpc_kinds2.index(cnts)))

	img = np.float32(cv2.imread("frame-000.pgm"))/255

	bpc.correct(img)

	cv2.imwrite("frame-000-corrected-bpc.png", img*255)
