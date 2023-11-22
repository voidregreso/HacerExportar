#ifndef MAPX_H
#define MAPX_H

#include <map>

namespace XLIB
{
    template <class _Kty,
        class _Ty,
        class _Pr = std::less<_Kty>,
        class _Alloc = std::allocator<std::pair<const _Kty, _Ty>>>
    class CXMap : public std::map<_Kty, _Ty, _Pr, _Alloc>
    {
    public:
        using _Mybase = std::map<_Kty, _Ty, _Pr, _Alloc>;

        bool Insert(const _Kty& key, const _Ty& value)
        {
            auto result = _Mybase::insert(std::make_pair(key, value));
            return result.second;
        }

        _Ty* Find(const _Kty& key, _Ty* pValue = nullptr)
        {
            auto item = _Mybase::find(key);
            if (item == _Mybase::end())
                return nullptr;

            if (pValue)
                *pValue = item->second;
            return &item->second;
        }
    };
}

#endif
