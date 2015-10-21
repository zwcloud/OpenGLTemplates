#pragma once
struct Line
{
	float xMin;
	float yMin;
	float xMax;
	float yMax;
	void Set(float xBegin, float yBegin, float xEnd, float yEnd)
	{
		if (xBegin < xEnd)
		{
			xMin = xBegin;
			xMax = xEnd;
		}
		else
		{
			xMin = xEnd;
			xMax = xBegin;
		}

		if (yBegin < yEnd)
		{
			yMin = yBegin;
			yMax = yEnd;
		}
		else
		{
			yMin = yEnd;
			yMax = yBegin;
		}
	}

	float[] GetVertexes()
	{

	}
};

