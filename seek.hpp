/*!
  \file
  \brief libseek public interface
*/
#include <cinttypes>
#include <cstdio>

#include "seek_pimpl_h.hpp"

namespace LibSeek {

class Imager;

class Frame {
	friend class Imager;
 private:
	class impl;
	pimpl<impl> m;

 public:
	Frame();
	~Frame();

 public:
	int width();
	int height();
	uint16_t const * data();
};

class Imager {
 private:
	class impl;
	pimpl<impl> m;

 public:
	Imager();
	~Imager();

 public:
	//! Initialization
	void init();
	void exit();

 public:
	void frame_init(Frame & frame);
	void frame_acquire(Frame & img);
	void frame_exit(Frame & frame);

};

} // LibSeek::
