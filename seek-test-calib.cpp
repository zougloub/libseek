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

	for (int i = 0; i < 100; i++) {
		iface.frame_acquire_raw(frame);
		int h = frame.height();
		int w = frame.width();
		char filename[30];
		char status = frame.rawdata()[20];
		sprintf(filename, "frame-%03d-raw-%d.pgm", i, status);
		FILE * f = fopen(filename, "wb");
		int res = fprintf(f, "P5 %d %d 65535\n", w, h);
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				uint8_t lsb = frame.rawdata()[2*(y*w+x)];
				uint8_t msb = frame.rawdata()[2*(y*w+x)+1];
				res = fwrite(&msb, sizeof(uint8_t), 1, f);
				res = fwrite(&lsb, sizeof(uint8_t), 1, f);
			}
		}
		fclose(f);
	}

	iface.frame_exit(frame);
	iface.exit();
}
