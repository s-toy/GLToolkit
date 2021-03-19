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

#ifndef USE_IN_COMPUTE_SHADER

#if QUANTIZATION_METHOD == UNIFORM_QUANTIZATION

uint quantize(float data, float min, float delta)
{
	if (abs(data) < 1e-6) return 0;

	if (data <= min) 
	{
		return 1;
	}
	else if (data > min + delta * 253)
	{
		return 255;
	}
	else
	{
		return 1 + uint(ceil((data - min) / delta));
	}
}

float dequantize(uint data, float min, float delta)
{ 
	if (data == 0) return 0;

	return min + data * delta - 1.5 * delta;
}
#endif

#if QUANTIZATION_METHOD == LLOYD_MAX_QUANTIZATION

layout(binding = 3, r16f) uniform image2D uRepresentativeDataImage;
uniform float uRepresentativeData[514];

uint quantize(float vData)
{
	if (abs(vData) < 1e-6) return 0;

	int l = 0, r = 255;
	while (l < r)
	{
		int mid = (l + r) / 2;
		//float lBoundary = imageLoad(uRepresentativeDataImage, ivec2(mid, 0)).x;
		//float rBoundary = imageLoad(uRepresentativeDataImage, ivec2(mid + 1, 0)).x;
		float lBoundary = uRepresentativeData[mid];
		float rBoundary = uRepresentativeData[mid + 1];
		if (vData >= lBoundary && vData <= rBoundary) { return mid; }
		else if (vData < lBoundary) r = mid - 1;
		else if (vData > rBoundary) l = mid + 1;
	}

	return uint(clamp(l, 0, 255));
}

float dequantize(uint vData)
{
	if (vData == 0) return 0;

	ivec2 coord = ivec2(clamp(int(vData), 0, 255), 1);
	//return imageLoad(uRepresentativeDataImage, coord).x;
	return uRepresentativeData[coord.x + 257];
}
#endif

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

#endif //USE_IN_COMPUTE_SHADER