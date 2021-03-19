#include "WOIT_common.glsl"

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

uint quantize(float data, vec2 coord)
{
	if (abs(data) < 1e-6) return 0;

	int l = 0, r = 254;
	while (l < r)
	{
		int mid = (l + r) / 2;
		float lPartition = texture(uOptimalQuantizerParamsImage, vec3(coord, mid)).x;
		float rPartition = texture(uOptimalQuantizerParamsImage, vec3(coord, mid + 1)).x;
		if (data >= lPartition && data <= rPartition) { return mid + 1; }
		else if (data < lPartition) r = mid;
		else if (data > rPartition) l = mid + 1;
	}

	return uint(clamp(l + 1, 1, 255));
}

float dequantize(uint data, vec2 coord)
{
	if (data == 0) return 0;

	int index = clamp(int(data), 1, 255) + 255;
	return texture(uOptimalQuantizerParamsImage, vec3(coord, index)).x;
}

#endif
