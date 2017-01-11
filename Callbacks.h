#ifndef DRAGONFLY_NET_CALLBACKS_H_
#define DRAGONFLY_NET_CALLBACKS_H_

#include <functional>
#include "Connector.h"

namespace dragonfly
{
    namespace net
    {
        typedef std::function<void(Conn*)> DataCallback;
        typedef std::function<void(Conn*,short)> EventCallback;
    }
}

#endif //DRAGONFLY_NET_CALLBACK_H_
