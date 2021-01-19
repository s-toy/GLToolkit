#define PI 3.1415926

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