#include <cassert>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <iomanip>

#include <endian.h>
#include <libusb.h>

#include "seek.hpp"
#include "seek_pimpl_impl.hpp"

using namespace std;
using namespace LibSeek;


#if !defined(DEBUG)
# define printf(...)
#else
# if defined(__ANDROID__)
#  include <android/log.h>
#  define LOG_TAG    "ZOUGLOUB-SEEK"
#  define printf(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
# else
#  define printf(...) fprintf(stderr, __VA_ARGS__)
# endif
#endif

class Frame::impl {
 public:
	int width = 208;
	int height = 156;
	vector<uint8_t> rawdata;
	vector<uint16_t> data;
	void process();
};

Frame::Frame()
{
}

Frame::~Frame()
{
}

void Frame::impl::process()
{
}

class Imager::impl {
 public:
	struct libusb_context * ctx = NULL;
	struct libusb_device_handle * handle = NULL;
	struct libusb_device * dev = NULL;
	struct libusb_device_descriptor desc;

	Frame calib;

 public:
	impl();
	~impl();
	void init();
	void exit();

 public:
	void vendor_transfer(bool direction, uint8_t req, uint16_t value,
	 uint16_t index, vector<uint8_t> & data, int timeout=1000);

	void frame_get_one(Frame & frame);
};

inline void sleep(float secs) {
	chrono::milliseconds dura(int(1000*secs));
	this_thread::sleep_for(dura);
}

int Frame::width() { return m->width; }

int Frame::height() { return m->height; }

uint16_t const * Frame::data() { return &m->data[0]; }


Imager::impl::impl()
{
	printf("%s:%d\n", __PRETTY_FUNCTION__, __LINE__);
}

Imager::impl::~impl()
{
	exit();
}

void Imager::impl::init()
{
	int res;

	if (handle != NULL) {
		throw runtime_error("dev should be null");
	}

	// Init libusb
	res = libusb_init(&ctx);
	if (res < 0) {
		throw runtime_error("Failed to initialize libusb");
	}

	struct libusb_device **devs;
	ssize_t cnt;

	// Get a list os USB devices
	cnt = libusb_get_device_list(ctx, &devs);
	if (cnt < 0) {
		throw runtime_error("No devices");
	}

	printf("\nDevice Count : %zd\n-------------------------------\n",cnt);

	bool found(false);
	for (int idx_dev = 0; idx_dev < cnt; idx_dev++) {
		dev = devs[idx_dev];
		res = libusb_get_device_descriptor(dev, &desc);
		if (res < 0) {
			libusb_free_device_list(devs, 1);
			throw runtime_error("Failed to get device descriptor");
		}

		res = libusb_open(dev, &handle);
		if (res < 0) {
			libusb_free_device_list(devs, 1);
			throw runtime_error("Failed to open device");
		}

		if (desc.idVendor == 0x289d && desc.idProduct == 0x0010) {
			found = true;
			printf("Found!\n");
			break;
		}

	} // for each device

	if (!found) {
		libusb_free_device_list(devs, 1);
		throw runtime_error("Seek not found");
	}

	printf("\nDevice found");

	int config2;
	res = libusb_get_configuration(handle, &config2);
	if (res != 0) {
		libusb_free_device_list(devs, 1);
		throw runtime_error("Couldn't get device configuration");
	}
	printf("\nConfigured value : %d",config2);

	if (config2 != 1) {
		res = libusb_set_configuration(handle, 1);
		if (res != 0) {
			libusb_free_device_list(devs, 1);
			throw runtime_error("Couldn't set device configuration");
		}
	}

	libusb_free_device_list(devs, 1);

	res = libusb_claim_interface(handle, 0);
	if (res < 0) {
		throw runtime_error("Couldn't claim interface");
	}
	printf("\nClaimed Interface\n");

	// device setup sequence
	try {
		vector<uint8_t> data = {0x01};
		vendor_transfer(0, 0x54, 0, 0, data);
	}
	catch (...) {
		// Try deinit device and repeat.
		vector<uint8_t> data = { 0x00, 0x00 };
		vendor_transfer(0, 0x3C, 0, 0, data);
		vendor_transfer(0, 0x3C, 0, 0, data);
		vendor_transfer(0, 0x3C, 0, 0, data);
		vendor_transfer(0, 0x54, 0, 0, data);
	}

	{
		vector<uint8_t> data = {0x00, 0x00};
		vendor_transfer(0, 0x3c, 0, 0, data);
	}

	{
		vector<uint8_t> data(4);
		vendor_transfer(1, 0x4e, 0, 0, data);
	}

	{
		vector<uint8_t> data(12);
		vendor_transfer(1, 0x36, 0, 0, data);
	}


	{
		vector<uint8_t> data = { 0x20, 0x00, 0x30, 0x00, 0x00, 0x00 };
		vendor_transfer(0, 0x56, 0, 0, data);
	}

	{
		vector<uint8_t> data(64);
		vendor_transfer(1, 0x58, 0, 0, data);
	}

	{
		vector<uint8_t> data = { 0x20, 0x00, 0x50, 0x00, 0x00, 0x00 };
		vendor_transfer(0, 0x56, 0, 0, data);
	}


	{
		vector<uint8_t> data(64);
		vendor_transfer(1, 0x58, 0, 0, data);
	}

	{
		vector<uint8_t> data = { 0x0c, 0x00, 0x70, 0x00, 0x00, 0x00 };
		vendor_transfer(0, 0x56, 0, 0, data);
	}


	{
		vector<uint8_t> data(24);
		vendor_transfer(1, 0x58, 0, 0, data);
	}

	{
		vector<uint8_t> data = { 0x06, 0x00, 0x08, 0x00, 0x00, 0x00 };
		vendor_transfer(0, 0x56, 0, 0, data);
	}

	{
		vector<uint8_t> data(12);
		vendor_transfer(1, 0x58, 0, 0, data);
	}

	{
		vector<uint8_t> data = { 0x08, 0x00 };
		vendor_transfer(0, 0x3E, 0, 0, data);
	}

	{
		vector<uint8_t> data(2);
		vendor_transfer(1, 0x3d, 0, 0, data);
	}

	{
		vector<uint8_t> data = { 0x08, 0x00 };
		vendor_transfer(0, 0x3E, 0, 0, data);
	}

	{
		vector<uint8_t> data = { 0x01, 0x00 };
		vendor_transfer(0, 0x3C, 0, 0, data);
	}

	{
		vector<uint8_t> data(2);
		vendor_transfer(1, 0x3d, 0, 0, data);
	}
}


void Imager::impl::exit()
{
	int res;

	if (handle == NULL) {
		return;
	}

	vector<uint8_t> data = { 0x00, 0x00 };
	vendor_transfer(0, 0x3C, 0, 0, data);
	vendor_transfer(0, 0x3C, 0, 0, data);
	vendor_transfer(0, 0x3C, 0, 0, data);

	if (handle != NULL) {
		res = libusb_release_interface(handle, 0);
		libusb_close(handle);
		handle = NULL;
	}

	if (ctx != NULL) {
		libusb_exit(ctx);
		ctx = NULL;
	}
}

void Imager::impl::vendor_transfer(bool direction,
 uint8_t req, uint16_t value, uint16_t index, vector<uint8_t> & data,
 int timeout)
{
	int res;
	uint8_t bmRequestType = (direction ? LIBUSB_ENDPOINT_IN : LIBUSB_ENDPOINT_OUT)
	 | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE;
	uint8_t bRequest = req;
	uint16_t wValue = value;
	uint16_t wIndex = index;

	if (data.size() == 0) {
		data.reserve(16);
	}

	uint8_t * aData = data.data();
	uint16_t wLength = data.size();
	if (!direction) {
		// to device
		printf("ctrl_transfer(0x%x, 0x%x, 0x%x, 0x%x, %d)",
		 bmRequestType, bRequest, wValue, wIndex, wLength);
		printf(" [");
		for (int i = 0; i < wLength; i++) {
			printf(" %02x", data[i]);
		}
		printf(" ]");

		res = libusb_control_transfer(handle, bmRequestType, bRequest,
		 wValue, wIndex, aData, wLength, timeout);

		if (res != wLength) {
			printf("\x1B[31;1mBad returned length: %d\x1B[0m\n", res);
		}
		printf("\n");
	}
	else {
		// from device
		printf("ctrl_transfer(0x%x, 0x%x, 0x%x, 0x%x, %d)",
		 bmRequestType, bRequest, wValue, wIndex, wLength);
		res = libusb_control_transfer(handle, bmRequestType, bRequest,
		 wValue, wIndex, aData, wLength, timeout);
		if (res != wLength) {
			printf("Bad returned length: %d\n", res);
		}
		printf(" -> [");
		for (auto & x: data) {
			printf(" %02x", x);
		}
		printf("]\n");
	}
}


Imager::Imager()
{
}

Imager::~Imager()
{
	m->exit();
}

void Imager::init()
{
	m->init();
}

void Imager::exit()
{
	m->exit();
}

void Imager::frame_init(Frame & frame)
{
	m->calib.m->rawdata.resize(frame.width()*frame.height()*2);
	m->calib.m->data.resize(frame.width()*frame.height());
	frame.m->rawdata.resize(frame.width()*frame.height()*2);
	frame.m->data.resize(frame.width()*frame.height());
}

void Imager::frame_exit(Frame & frame)
{
}


void Imager::impl::frame_get_one(Frame & frame)
{
	int res;

	int size = frame.width() * frame.height();

	{ // request a frame
		vector<uint8_t> data = { uint8_t(size & 0xff), uint8_t((size>>8)&0xff), 0, 0 };
		vendor_transfer(0, 0x53, 0, 0, data);
	}

	int bufsize = size * sizeof(uint16_t);

	{
		int todo = bufsize;
		int done = 0;

		while (todo != 0) {
			int actual_length = 0;
			printf("Asking for %d B of data at %d\n", todo, done);
			res = libusb_bulk_transfer(handle, 0x81, &frame.m->rawdata[done],
			 todo, &actual_length, 500);
			if (res != 0) {
				fprintf(stderr, "\x1B[31;1m%s: libusb_bulk_transfer returned %d\x1B[0m\n", __func__, res);
			}
			printf("Actual length %d\n", actual_length);
			todo -= actual_length;
			done += actual_length;
		}
	}

	int w = frame.width();
	int h = frame.height();
}

void Imager::frame_acquire(Frame & frame)
{
	while (true) {
		m->frame_get_one(frame);
		vector<uint8_t> & rawdata = frame.m->rawdata;

		uint8_t status = rawdata[20];
		printf("Status byte: %2x\n", status);

		if (status == 1) {
			m->calib.m->rawdata = frame.m->rawdata;
			vector<uint16_t> & data = m->calib.m->data;

			int h = frame.height();
			int w = frame.width();
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++) {
					uint16_t v = reinterpret_cast<uint16_t*>(rawdata.data())[y*w+x];
					v = le16toh(v);
					data[y*w+x] = v;
				}
			}
			printf("Calib\n");
			continue;
		}

		if (status != 3) {
			printf("Bad\n");
			continue;
		}

		vector<uint16_t> & data = frame.m->data;

		int h = frame.height();
		int w = frame.width();
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				uint16_t v = reinterpret_cast<uint16_t*>(rawdata.data())[y*w+x];
				v = le16toh(v);

				int a = v;
				a -= m->calib.m->data[y*w+x];

				if (a < 0) {
					a = 0;
				}
				if (a > 0xFFFF) {
					a = 0xFFFF;
				}

				v = a;

				data[y*w+x] = v;
			}
		}

		break;
	}
}

