#define PI 3.1415926

#define FOIT_ENABLE_QUANTIZATION

#define LINEAR_QUANTIZATION		0
#define LOGARITHMIC_QUANTIZATION	1
#define LOG_LINEAR_QUANTIZATION		2

#define QUANTIZATION_METHOD	LOGARITHMIC_QUANTIZATION

#define FOIT_FLT_PRECISION rgba16f

float _returnNegativeZe(float depth, float near, float far)
{
	float z = depth * 2.0 - 1.0; // back to NDC 
	z = (2.0 * near * far) / (far + near - z * (far - near)); // range: near...far
	return z;
}

float _linearizeDepth(float depth, float near, float far)
{
	float z = _returnNegativeZe(depth, near, far);
	z = (z - near) / (far - near); // range: 0...1
	return z;
}

const int PDF_SLICE_COUNT = 10000;

const float _IntervalMin = -50;
const float _IntervalMax = 50;
const float _IntervalLength = (_IntervalMax - _IntervalMin) / 256.0;

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
			float l = pow(2, vData * log2((1-K)*_IntervalMax + 1) / (128 - uint(C))) - 1;
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

uvec4 quantizeVec4(vec4 vData)
{
	return uvec4(quantize(vData.x), quantize(vData.y), quantize(vData.z), quantize(vData.w));
}

vec4 dequantizeVec4(uvec4 vData)
{
	return vec4(dequantize(vData.x), dequantize(vData.y), dequantize(vData.z), dequantize(vData.w));
}