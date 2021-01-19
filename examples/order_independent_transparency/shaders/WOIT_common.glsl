#define PI 3.1415926

//#define WOIT_ENABLE_QUANTIZATION

#define LINEAR_QUANTIZATION			0
#define LOGARITHMIC_QUANTIZATION	1
#define LOG_LINEAR_QUANTIZATION		2
#define LLOYD_MAX_QUANTIZATION		3
#define QUANTIZATION_METHOD			LLOYD_MAX_QUANTIZATION

#define FOURIER_BASIS	0
#define HAAR_BASIS		1
#define MEYER_BASIS		2
#define SIN_BASIS		3
#define BASIS_TYPE		SIN_BASIS

#if BASIS_TYPE == FOURIER_BASIS
#define BASIS_NUM 7
#elif BASIS_TYPE == HAAR_BASIS
#define BASIS_NUM 8
#elif BASIS_TYPE == MEYER_BASIS
#define BASIS_NUM 8
#elif BASIS_TYPE == SIN_BASIS
#define BASIS_NUM 8
#endif

#define SLICE_COUNT 1000
#define WOIT_FLT_PRECISION rgba32f

#ifdef ENABLE_SIGMA_AVERAGING
#define SIGMA_K(k, n) (sin(k*PI/n) / (k*PI/n))
#else
#define SIGMA_K(k, n) 1
#endif

const int PDF_SLICE_COUNT = 100;

const float _IntervalMin = -50;
const float _IntervalMax = 50;
const float _IntervalLength = (_IntervalMax - _IntervalMin) / 256.0;

#ifndef USE_IN_COMPUTE_SHADER

#if QUANTIZATION_METHOD == LOGARITHMIC_QUANTIZATION
uint quantize(float vData)
{
	if (abs(vData) < 1e-6)
	{
		return 0;
	}
	else if (vData > 0)
	{
		uint c = uint(floor(log2(vData + 1) * 128.0 / log2(_IntervalMax + 1)));
		c = clamp(c, 0u, 127u);
		return c;
	}
	else
	{
		float data = abs(vData);
		uint c = uint(floor(log2(data + 1) * 128.0 / log2(-_IntervalMin + 1)));
		c = clamp(c, 0u, 127u);
		return c + 128;
	}
}

float dequantize(uint vData)
{
	if (vData == 0)
	{
		return 0.0;
	}
	else if (vData < 128)
	{
		float l = pow(2, vData * log2(_IntervalMax + 1) / 128) - 1;
		float r = pow(2, (vData + 1) * log2(_IntervalMax + 1) / 128) - 1;
		return (l + r) / 2;
	}
	else
	{
		vData -= 128;
		float l = pow(2, vData * log2(-_IntervalMin + 1) / 128) - 1;
		float r = pow(2, (vData + 1) * log2(-_IntervalMin + 1) / 128) - 1;
		return -(l + r) / 2;
	}
}
#endif

#if QUANTIZATION_METHOD == LINEAR_QUANTIZATION
uint quantize(float vData)
{
	if (abs(vData) < 1e-6)
	{
		return 0;
	}
	else if (vData > 0)
	{
		uint c = uint(floor(vData / _IntervalMax * 128.0));
		c = clamp(c, 0u, 127u);
		return c;
	}
	else
	{
		float data = abs(vData);
		uint c = uint(floor(vData / _IntervalMin * 128.0));
		c = clamp(c, 0u, 127u);
		return c + 128;
	}
}

float dequantize(uint vData)
{
	if (vData == 0)
	{
		return 0;
	}
	else if (vData < 128)
	{
		float l = vData * _IntervalMax / 128.0;
		float r = (vData + 1) * _IntervalMax / 128.0;
		return (l + r) / 2;
	}
	else
	{
		vData -= 128;
		float l = vData * _IntervalMin / 128.0;
		float r = (vData + 1) * _IntervalMin / 128.0;
		return (l + r) / 2;
	}
}
#endif

#if QUANTIZATION_METHOD == LOG_LINEAR_QUANTIZATION

const float K = 0.1;
const float C = 64;

uint quantize(float vData)
{
	if (abs(vData) < 1e-6)
	{
		return 0;
	}
	else if (vData > 0)
	{
		uint c;
		if (vData < K * _IntervalMax)
		{
			c = uint(floor(vData / (K * _IntervalMax) * C));
		}
		else
		{
			c = uint(floor(log2(vData + 1 - K * _IntervalMax) * (128.0 - C) / log2(_IntervalMax + 1 - K * _IntervalMax)) + C);
		}
		c = clamp(c, 0u, 127u);
		return c;
	}
	else
	{
		vData = abs(vData);
		uint c;
		if (vData < K * -_IntervalMin)
		{
			c = uint(floor(vData / (K * -_IntervalMin) * C));
		}
		else
		{
			c = uint(floor(log2(vData + 1 + K * _IntervalMin) * (128.0 - C) / log2(-_IntervalMin + 1 + K * _IntervalMin)) + C);
		}
		c = clamp(c, 0u, 127u);
		return c + 128;
	}
}

float dequantize(uint vData)
{
	if (vData == 0)
	{
		return 0;
	}
	else if (vData < 128)
	{
		if (vData < C)
		{
			float l = vData * K * _IntervalMax / C;
			float r = (vData + 1) * K * _IntervalMax / C;
			return (l + r) / 2;
		}
		else
		{
			vData -= uint(C);
			float l = pow(2, vData * log2((1 - K)*_IntervalMax + 1) / (128 - uint(C))) - 1;
			float r = pow(2, (vData + 1) * log2((1 - K)*_IntervalMax + 1) / (128 - uint(C))) - 1;
			return (l + r) / 2 + K * _IntervalMax;
		}
	}
	else
	{
		vData -= 128;
		float IntervalMin = -_IntervalMin;
		if (vData < C)
		{
			float l = vData * K * IntervalMin / C;
			float r = (vData + 1) * K * IntervalMin / C;
			return -(l + r) / 2;
		}
		else
		{
			vData -= uint(C);
			float l = pow(2, vData * log2((1 - K) * IntervalMin + 1) / (128 - uint(C))) - 1;
			float r = pow(2, (vData + 1) * log2((1 - K) * IntervalMin + 1) / (128 - uint(C))) - 1;
			return -((l + r) / 2 + K * IntervalMin);
		}
	}
}
#endif

#if QUANTIZATION_METHOD == LLOYD_MAX_QUANTIZATION

layout(binding = 3, r32f) uniform image2D uRepresentativeDataImage;
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

uvec4 quantizeVec4(vec4 vData)
{
	return uvec4(quantize(vData.x), quantize(vData.y), quantize(vData.z), quantize(vData.w));
}

vec4 dequantizeVec4(uvec4 vData)
{
	return vec4(dequantize(vData.x), dequantize(vData.y), dequantize(vData.z), dequantize(vData.w));
}

#endif