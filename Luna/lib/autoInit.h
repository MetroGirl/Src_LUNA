//
// automated initialization.
//

#ifndef LUNA_AUTOINIT_H_INCLUDED
#define LUNA_AUTOINIT_H_INCLUDED

namespace luna{
	namespace experimental{
		template<typename T>
		class init
		{
		public:
			init()
				: mValue()
			{
			}

			init(const init& rhs)
				: mValue(rhs.mValue)
			{
			}

			init(const T& value)
				: mValue(value);
			{
			}

			init(init&& rhs)
				: mValue(move(rhs.mValue))
			{

			}

			init(T&& value)
				: mValue(move(value));
			{

			}

		private:
			T mValue;
		};
	}
}

#endif // LUNA_AUTOINIT_H_INCLUDED