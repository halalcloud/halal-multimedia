#ifndef PUBLISHPOINT_H
#define PUBLISHPOINT_H
#include "stdafx.h"

class CPublishPoint : public IPublishPoint
{
public:
    DOM_DECLARE(CPublishPoint)
    //IPublishPoint
    STDMETHODIMP_(const char*) GetKey();
protected:
};

#endif // PUBLISHPOINT_H
