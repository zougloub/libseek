#ifndef SEEK_PIMPL_IMPL_H
#define SEEK_PIMPL_IMPL_H

#include <utility>

namespace LibSeek {

template<typename T>
pimpl<T>::pimpl() : m{ new T{} } { }

template<typename T>
template<typename ...Args>
pimpl<T>::pimpl( Args&& ...args )
    : m{ new T{ std::forward<Args>(args)... } } { }

template<typename T>
pimpl<T>::~pimpl() { }

template<typename T>
T* pimpl<T>::operator->() { return m.get(); }

template<typename T>
T& pimpl<T>::operator*() { return *m.get(); }

} // LibSeek::

#endif // SEEK_PIMPL_IMPL_H
