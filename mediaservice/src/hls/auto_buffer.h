#ifndef AUTO_BUFFER_H
#define AUTO_BUFFER_H
template<class T>
class auto_buffer
{
private:
    T* _buffer;

    /// no copy nor assignment
    auto_buffer(const auto_buffer&);
    auto_buffer& operator=(const auto_buffer&);
public:
    explicit auto_buffer (size_t size = 0) throw()
        : _buffer (size == 0 ? NULL : new T[size]())
    {
    }

    ~auto_buffer()
    {
        delete[] _buffer;
    }

    operator T*() const throw()
    {
        return _buffer;
    }

    operator void*() const throw()
    {
        return _buffer;
    }

    operator bool() const throw()
    {
        return _buffer != NULL;
    }

    T* operator->() const throw()
    {
        return _buffer;
    }

    T *get() const throw()
    {
        return _buffer;
    }

    T* release() throw()
    {
        T* temp = _buffer;
        _buffer = NULL;
        return temp;
    }
    void reset(T* t)
    {
        if ( t == _buffer)
            return;  // do nothing
        delete [] _buffer;
        _buffer = t;
    }
    void reset (size_t newSize = 0)
    {
        delete[] _buffer;
        // first set pointer to null so that it doesn't persist a dangling pointer
        // in case new[] throws an exception
        _buffer = 0;
        if (newSize > 0)
        {
            _buffer = new T[newSize];
        }
    }
};
#endif
