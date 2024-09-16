# pragma once

#include <functional>

// TODO: remove
void network(void);

namespace net
{
    int     startup(void);
    int     cleanup(void);

    template <typename TRetrurnType>
    inline int     checkError(std::function<TRetrurnType()>);
}