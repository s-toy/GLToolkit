#define PI 3.1415926

#define WOIT_ENABLE_QUANTIZATION
#define WOIT_ENABLE_QERROR_CALCULATION
//#define ENABLE_DEPTH_REMAPPING

#define UNIFORM_QUANTIZATION		0
#define LLOYD_MAX_QUANTIZATION		1
#define QUANTIZATION_METHOD			UNIFORM_QUANTIZATION

#define FOURIER_BASIS	0
#define HAAR_BASIS		1
#define MEYER_BASIS		2
#define BASIS_TYPE		MEYER_BASIS

#if BASIS_TYPE == FOURIER_BASIS
#define BASIS_NUM 10
#elif BASIS_TYPE == HAAR_BASIS
#define BASIS_NUM 8
#elif BASIS_TYPE == MEYER_BASIS
#define BASIS_NUM 10
#endif

#define BASIS_SLICE_COUNT 2001
#define BASIS_SCALE 20
#define WOIT_FLT_PRECISION r16f

#ifdef ENABLE_SIGMA_AVERAGING
#define SIGMA_K(k, n) (sin(k*PI/n) / (k*PI/n))
#else
#define SIGMA_K(k, n) 1
#endif

const int PDF_SLICE_COUNT = 1024;

const float _IntervalMin = -100;
const float _IntervalMax = 100;
const float _Mu = 0;

float haar_phi_integral(float d)
{
	return d;
}

float haar_psi_integral(float d, float j, float k)
{
	const float value = pow(2.0f, j / 2.0f);
	const float intervalLength = 1.0f / pow(2.0f, j + 1);
	const float intervalMin = 2 * k * intervalLength;
	const float intervalMid = intervalMin + intervalLength;
	const float intervalMax = intervalMid + intervalLength;

	if (d < intervalMin)
		return 0;
	else if (d >= intervalMin && d < intervalMid)
		return value * (d - intervalMin);
	else if (d >= intervalMid && d < intervalMax)
		return value * (intervalMid - intervalMin) + (-value) * (d - intervalMid);
	else
		return 0;
}

float haar_phi(float x)
{
	return 1;
}

float haar_psi(float x, float j, float k)
{
	float value = pow(2.0f, j / 2.0f);
	//value *= value;

	const float intervalLength = 1.0f / pow(2.0f, j + 1);
	const float intervalMin = 2 * k * intervalLength;
	const float intervalMid = intervalMin + intervalLength;
	const float intervalMax = intervalMid + intervalLength;

	if (x >= intervalMin && x < intervalMid)
		return value;
	else if (x >= intervalMid && x < intervalMax)
		return -value;
	else
		return 0;
}

float expandFuncMiu(float x, float intervalMin, float intervalMax, float mu)
{
	if (mu < 1e-6) return x;

    x = (x - intervalMin) / (intervalMax - intervalMin) * 2 - 1;
    float y = sign(x) * log(1+mu*abs(x)) / log(1+mu);
    y = (y * 0.5 + 0.5) * (intervalMax - intervalMin) + intervalMin;
	return y;
}

float expandFuncMiuReverse(float x, float intervalMin, float intervalMax, float mu)
{
	if (mu < 1e-6) return x;

    x = (x - intervalMin) / (intervalMax - intervalMin) * 2 - 1;
    float y = sign(x) * (1 / mu) * (pow(1 + mu, abs(x)) - 1);
    y = (y * 0.5 + 0.5) * (intervalMax - intervalMin) + intervalMin;
	return y;
}