#ifndef WOIT_COMMON
#define WOIT_COMMON

#include "global_macro.h"

const float _IntervalMin = -20;
const float _IntervalMax = 20;
const float _Delta = (_IntervalMax - _IntervalMin) / 256;
const float _Mu = 0.0;

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

uint quantize(float data)
{
	if (abs(data) < 1e-6) return 0;

	if (data <= _IntervalMin) 
	{
		return 1;
	}
	else if (data > _IntervalMin + _Delta * 253)
	{
		return 255;
	}
	else
	{
		return 1 + uint(ceil((data - _IntervalMin) / _Delta));
	}
}

float dequantize(uint data)
{ 
	if (data == 0) return 0;

	return _IntervalMin + data * _Delta - 1.5 * _Delta;
}

uvec4 quantize(vec4 data)
{
	return uvec4(quantize(data.x), quantize(data.y), quantize(data.z), quantize(data.w));
}

vec4 dequantize(uvec4 data)
{ 
	return vec4(dequantize(data.x), dequantize(data.y), dequantize(data.z), dequantize(data.w));
}

float expandFuncMu(float x, float intervalMin, float intervalMax, float mu)
{
	if (mu < 1e-6) return x;

	if (x < 0)
	{ 
		x = -x / intervalMin;
		float y = sign(x) * log(1+mu*abs(x)) / log(1+mu);
		y = -y * intervalMin;
		return y;
	}
	else
	{
		x = x  / intervalMax;
		float y = sign(x) * log(1+mu*abs(x)) / log(1+mu);
		y = y * intervalMax;
		return y;
	}
}

vec4 expandFuncMu(vec4 coeffs, float intervalMin, float intervalMax, float mu)
{
	return vec4(
		expandFuncMu(coeffs.x, intervalMin, intervalMax, mu),
		expandFuncMu(coeffs.y, intervalMin, intervalMax, mu),
		expandFuncMu(coeffs.z, intervalMin, intervalMax, mu),
		expandFuncMu(coeffs.w, intervalMin, intervalMax, mu)
	);
}

float expandFuncMuReverse(float x, float intervalMin, float intervalMax, float mu)
{
	if (mu < 1e-6) return x;

	if (x < 0)
	{ 
		x = -x / intervalMin;
		 float y = sign(x) * (1 / mu) * (pow(1 + mu, abs(x)) - 1);
		y = -y * intervalMin;
		return y;
	}
	else
	{
		x = x  / intervalMax;
		 float y = sign(x) * (1 / mu) * (pow(1 + mu, abs(x)) - 1);
		y = y * intervalMax;
		return y;
	}
}

vec4 expandFuncMuReverse(vec4 coeffs, float intervalMin, float intervalMax, float mu)
{
	return vec4(
		expandFuncMuReverse(coeffs.x, intervalMin, intervalMax, mu),
		expandFuncMuReverse(coeffs.y, intervalMin, intervalMax, mu),
		expandFuncMuReverse(coeffs.z, intervalMin, intervalMax, mu),
		expandFuncMuReverse(coeffs.w, intervalMin, intervalMax, mu)
	);
}

#endif