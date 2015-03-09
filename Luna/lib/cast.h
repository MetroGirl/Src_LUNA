//
// cast helper function.
//

#ifndef LUNA_CAST_H_INCLUDED
#define LUNA_CAST_H_INCLUDED

namespace luna{
	template<typename TO, typename FROM>
	TO as(FROM src)
	{
		return static_cast<TO>(src);
	}
}

#endif // LUNA_CAST_H_INCLUDED