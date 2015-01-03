#ifndef SEEK_PIMPL_H_H
#define SEEK_PIMPL_H_H

#include <memory>

namespace LibSeek {

template<typename T>
class pimpl {
private:
    std::unique_ptr<T> m;
public:
    pimpl();
    template<typename ...Args> pimpl( Args&& ... );
    ~pimpl();
    T* operator->();
    T& operator*();
};

} // LibSeek::

#endif // SEEK_PIMPL_H_H
