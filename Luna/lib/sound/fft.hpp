// fft.hpp
#ifndef FFT_HPP_DEFINED
#define FFT_HPP_DEFINED

namespace Core
{
  namespace fft
  {
    #define SIZE 256        // 信号サイズ
    #define PI 3.14159265    // 円周率

    // ビットを左右反転した配列を返す
    int* BitScrollArray(int arraySize)
    {
        int i, j;
        int* reBitArray = (int*)calloc(arraySize, sizeof(int));
        int arraySizeHarf = arraySize >> 1;

        reBitArray[0] = 0;
        for (i = 1; i < arraySize; i <<= 1)
        {
            for (j = 0; j < i; j++)
                reBitArray[j + i] = reBitArray[j] + arraySizeHarf;
            arraySizeHarf >>= 1;
        }
        return reBitArray;
    }

    // FFT
    void FFT2(double* inputRe, double* inputIm, double* outputRe, double* outputIm, int bitSize)
    {
      int i, j, stage, type;
      int dataSize = 1 << bitSize;
      int butterflyDistance;
      int numType;
      int butterflySize;
      int jp;
      int* reverseBitArray = BitScrollArray(dataSize);
      double wRe, wIm, uRe, uIm, tempRe, tempIm, tempWRe, tempWIm;

      // バタフライ演算のための置き換え
      for (i = 0; i < dataSize; i++)
      {
          outputRe[i] = inputRe[reverseBitArray[i]];
          outputIm[i] = inputIm[reverseBitArray[i]];
      }

      // バタフライ演算
      for (stage = 1; stage <= bitSize; stage++)
      {
          butterflyDistance = 1 << stage;
          numType = butterflyDistance >> 1;
          butterflySize = butterflyDistance >> 1;

          wRe = 1.0;
          wIm = 0.0;
          uRe = cos(PI / butterflySize);
          uIm = -sin(PI / butterflySize);

          for (type = 0; type < numType; type++)
          {
              for (j = type; j < dataSize; j += butterflyDistance)
              {
                  jp = j + butterflySize;
                  tempRe = outputRe[jp] * wRe - outputIm[jp] * wIm;
                  tempIm = outputRe[jp] * wIm + outputIm[jp] * wRe;
                  outputRe[jp] = outputRe[j] - tempRe;
                  outputIm[jp] = outputIm[j] - tempIm;
                  outputRe[j] += tempRe;
                  outputIm[j] += tempIm;
              }
              tempWRe = wRe * uRe - wIm * uIm;
              tempWIm = wRe * uIm + wIm * uRe;
              wRe = tempWRe;
              wIm = tempWIm;
          }
      }
    }

		f64 calcThetaForFFT(int nSamples)
		{
			return f64(-8.0 * atan(1.0) / (f64)nSamples);
		}

		// FFT: double theta = -8 * atan(1.0) / n
		//iFFT: double theta = 8 * atan(1.0) / n
		void fft(int nSamples, f64 theta, f64 ar[], f64 ai[])
		{
			int m, mh, i, j, k;
			f64 wr, wi, xr, xi;

			for (m = nSamples; (mh = m >> 1) >= 1; m = mh){
				for (i = 0; i < mh; i++){
					wr = cos(theta * i);
					wi = sin(theta * i);
					for (j = i; j < nSamples; j += m) 
					{
						k = j + mh;
						xr = ar[j] - ar[k];
						xi = ai[j] - ai[k];
						ar[j] += ar[k];
						ai[j] += ai[k];
						ar[k] = wr * xr - wi * xi;
						ai[k] = wr * xi + wi * xr;
					}
				}
				theta *= 2;
			}
    
			i = 0;
			for (j = 1; j < nSamples - 1; j++){
				for (k = nSamples >> 1; k > (i ^= k); k >>= 1);

				if (j < i){
					xr = ar[j];
					xi = ai[j];
					ar[j] = ar[i];
					ai[j] = ai[i];
					ar[i] = xr;
					ai[i] = xi;
				}
			}
		}
  }
}
#endif