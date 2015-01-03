#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <vector>

#include "seek.hpp"

using namespace std;
using namespace LibSeek;

inline void sleep(float secs) {
	chrono::milliseconds dura(int(1000*secs));
	this_thread::sleep_for(dura);
}

int main() {
	setbuf(stdout, NULL);
	Imager iface;
	iface.init();

	Frame frame;

	iface.frame_init(frame);

	for (int i = 0; i < 71370537; i++) {
		//break;
		iface.frame_acquire(frame);

		int h = frame.height();
		int w = frame.width();
		vector<uint16_t> img(w*h);
		{
			int _max = 0;
			int _min = 0xffff;
#if 0
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++) {
					uint16_t v = frame.data()[y*w+x];
					if (v > _max) _max = v;
					if (v < _min) _min = v;
				}
			}

#elif 0
			_max = 0x8200;
			_min = 0x7e00;
#else
			_max = 0xffff;
			_min = 0x0;
#endif

			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++) {
#if 1
					float v = float(frame.data()[y*w+x] - _min) / (_max - _min);
					if (v < 0.0) { v = 0; }
					if (v > 1.0) { v = 1; }
					uint16_t o = 0xffff * v;
#else
					uint16_t o = frame.data()[y*w+x];
#endif
					//fprintf(stderr, " %4x", o);
					img[y*w+x] = o;
				}
				//fprintf(stderr, "\n");
			}
			//fprintf(stderr, "\n");
			fwrite((uint8_t*)img.data(), sizeof(uint16_t), w*h, stdout);
		}

		if (0) {
			char filename[30];
			sprintf(filename, "frame-%03d.pgm", i);
			FILE * f = fopen(filename, "wb");
			int res = fprintf(f, "P5 %d %d 65535\n", w, h);
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++) {
					uint16_t v = img[y*w+x];
					v = htobe16(v);
					res = fwrite((uint8_t*)&v, sizeof(uint16_t), 1, f);
				}
			}
			fclose(f);
		}

	}

	iface.frame_exit(frame);

	iface.exit();
}

