#ifndef DRAGONFLY_BASE_UNCOPYABLE_H_
#define DRAGONFLY_BASE_UNCOPYABLE_H_

namespace dragonfly
{
    namespace detail
    {
        class uncopyable
        {
            protected:
                uncopyable(){}
                ~uncopyable(){}
            private:
                uncopyable(const uncopyable&);
                uncopyable& operator=(const uncopyable&);
        };
    }
    typedef detail::uncopyable  uncopyable;
}
#endif //DRAGONFLY_BASE_UNCOPYABLE_H_
