// 
// color. 
//

#ifndef LUNA_COLOR_H_INCLUDED
#define LUNA_COLOR_H_INCLUDED

namespace luna{
	inline XMFLOAT4 RGBtoHSV(XMFLOAT4 rgb)
	{
		const f32 r = rgb.x;
		const f32 g = rgb.y;
		const f32 b = rgb.z;

		f32 max = r > g ? r : g;
		max = max > b ? max : b;
		f32 min = r < g ? r : g;
		min = min < b ? min : b;
		f32 h = max - min;
		if (h > 0.0f) {
			if (max == r) {
				h = (g - b) / h;
				if (h < 0.0f) {
					h += 6.0f;
				}
			}
			else if (max == g) {
				h = 2.0f + (b - r) / h;
			}
			else {
				h = 4.0f + (r - g) / h;
			}
		}
		h /= 6.0f;
		f32 s = (max - min);
		if (max != 0.0f)
			s /= max;
		f32 v = max;

		return XMFLOAT4(h, s, v, rgb.w);
	}

	inline XMFLOAT4 HSVtoRGB(XMFLOAT4 hsv)
	{
		const f32 h = hsv.x * 6.f;
		const f32 s = hsv.y;
		const f32 v = hsv.z;

		f32 r = v;
		f32 g = v;
		f32 b = v;
		if (s > 0.0f) {
			const s32 i = (s32)h;
			const f32 f = h - (f32)i;
			switch (i) {
			default:
			case 0:
				g *= 1 - s * (1 - f);
				b *= 1 - s;
				break;
			case 1:
				r *= 1 - s * f;
				b *= 1 - s;
				break;
			case 2:
				r *= 1 - s;
				b *= 1 - s * (1 - f);
				break;
			case 3:
				r *= 1 - s;
				g *= 1 - s * f;
				break;
			case 4:
				r *= 1 - s * (1 - f);
				g *= 1 - s;
				break;
			case 5:
				g *= 1 - s;
				b *= 1 - s * f;
				break;
			}
		}

		return XMFLOAT4(r, g, b, hsv.w);
	}
}

#endif // LUNA_COLOR_H_INCLUDED