#pragma once
#include "BitMatrix.h"
#include "ResultPoint.h"

using namespace ZXing;

namespace ZXing {


class CornerDetector
{
	const BitMatrix& image;
	const int height;
	const int width;
	int leftInit;
	int rightInit;
	int downInit;
	int upInit;
	int targetMatrixSize;
	bool FindCorners(int right, int left, int down, int up, std::array<ResultPoint, 4>& result) const;
	bool GetCornerFromArea(int left, int right, int top, int bottom, bool maximizeX, bool maximizeY, ResultPoint& result) const;
	bool ContainsBlackPoint(int a, int b, int fixed, bool horizontal) const;
public:
	CornerDetector(const BitMatrix& image, int initSize, int x, int y, int targetMatrixSize);
	bool Detect(std::array<ResultPoint, 4>& result);
};

}

