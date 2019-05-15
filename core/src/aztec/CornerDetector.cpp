#include "CornerDetector.h"
#include <array>

using namespace ZXing;

CornerDetector::CornerDetector(const BitMatrix& image, int initSize, int x, int y, int targetMatrixSize)
: image(image), height(image.height()), width(image.width())
{
	int halfsize = initSize / 2;
	leftInit = x - halfsize;
	rightInit = x + halfsize;
	upInit = y - halfsize;
	downInit = y + halfsize;
	this->targetMatrixSize = targetMatrixSize * 2;
	if (upInit < 0 || leftInit < 0 || downInit >= height || rightInit >= width) {
		//
	}
}

bool CornerDetector::Detect(std::array<ResultPoint, 4>& result)
{
	int left = leftInit;
	int right = rightInit;
	int up = upInit;
	int down = downInit;
	bool sizeExceeded = false;
	bool aBlackPointFoundOnBorder = true;
	bool atLeastOneBlackPointFoundOnBorder = false;

	bool atLeastOneBlackPointFoundOnRight = false;
	bool atLeastOneBlackPointFoundOnBottom = false;
	bool atLeastOneBlackPointFoundOnLeft = false;
	bool atLeastOneBlackPointFoundOnTop = false;

	while (aBlackPointFoundOnBorder) {

		aBlackPointFoundOnBorder = false;

		// .....
		// . |
		// .....
		bool rightBorderNotWhite = true;
		while ((rightBorderNotWhite || !atLeastOneBlackPointFoundOnRight) && right < width) {
			rightBorderNotWhite = ContainsBlackPoint(up, down, right, false);
			if (rightBorderNotWhite) {
				right++;
				aBlackPointFoundOnBorder = true;
				atLeastOneBlackPointFoundOnRight = true;
			}
			else if (!atLeastOneBlackPointFoundOnRight) {
				right++;
			}
		}

		if (right >= width) {
			sizeExceeded = true;
			break;
		}

		// .....
		// . .
		// .___.
		bool bottomBorderNotWhite = true;
		while ((bottomBorderNotWhite || !atLeastOneBlackPointFoundOnBottom) && down < height) {
			bottomBorderNotWhite = ContainsBlackPoint(left, right, down, true);
			if (bottomBorderNotWhite) {
				down++;
				aBlackPointFoundOnBorder = true;
				atLeastOneBlackPointFoundOnBottom = true;
			}
			else if (!atLeastOneBlackPointFoundOnBottom) {
				down++;
			}
		}

		if (down >= height) {
			sizeExceeded = true;
			break;
		}

		// .....
		// | .
		// .....
		bool leftBorderNotWhite = true;
		while ((leftBorderNotWhite || !atLeastOneBlackPointFoundOnLeft) && left >= 0) {
			leftBorderNotWhite = ContainsBlackPoint(up, down, left, false);
			if (leftBorderNotWhite) {
				left--;
				aBlackPointFoundOnBorder = true;
				atLeastOneBlackPointFoundOnLeft = true;
			}
			else if (!atLeastOneBlackPointFoundOnLeft) {
				left--;
			}
		}

		if (left < 0) {
			sizeExceeded = true;
			break;
		}

		// .___.
		// . .
		// .....
		bool topBorderNotWhite = true;
		while ((topBorderNotWhite || !atLeastOneBlackPointFoundOnTop) && up >= 0) {
			topBorderNotWhite = ContainsBlackPoint(left, right, up, true);
			if (topBorderNotWhite) {
				up--;
				aBlackPointFoundOnBorder = true;
				atLeastOneBlackPointFoundOnTop = true;
			}
			else if (!atLeastOneBlackPointFoundOnTop) {
				up--;
			}
		}

		if (up < 0) {
			sizeExceeded = true;
			break;
		}

		if (aBlackPointFoundOnBorder) {
			atLeastOneBlackPointFoundOnBorder = true;
		}
	}

	if (!sizeExceeded && atLeastOneBlackPointFoundOnBorder) {
		return FindCorners(right, left, down, up, result);
	}
	else {
		return false;
	}
}


bool CornerDetector::FindCorners(int right, int left, int down, int up, std::array<ResultPoint, 4>& result) const
{
	//
	//      A------------              ------------B
	//      |           |      up      |           |
	//      |    -------|--------------|-------    |
	//      |    |      |              |      |    |
	//      |    |      |              |      |    |
	//      ------------AP            BP------------
	//           |                            |
	//           |                            |
	//      left |                            | right
	//           |                            |
	//           |                            |
	//      ------------DP            CP------------
	//      |    |      |             |       |    |
	//      |    |      |   down      |       |    |
	//      |    -------|-------------|--------    |
	//      |           |             |            |
	//      D-----------|             |------------C
	//


	float width = right - left;
	float height = down - up;
	float sampler = (float)16 / targetMatrixSize;
	float sampler2 = (float)4 / targetMatrixSize;
	int deltaX = (int)(width * sampler2);
	int deltaY = (int)(height * sampler2);
	int areaWidth = deltaX + (int)((right - left) * sampler);
	int areaHeight = deltaY + (int)((down - up) * sampler);

	ResultPoint a = ResultPoint(left - deltaX, up - deltaY);
	ResultPoint b = ResultPoint(right + deltaX, up - deltaY);
	ResultPoint c = ResultPoint(right + deltaX, down + deltaY);
	ResultPoint d = ResultPoint(left - deltaX, down + deltaY);

	ResultPoint ap = ResultPoint(a.x() + areaWidth, a.y() + areaHeight);
	ResultPoint bp = ResultPoint(b.x() - areaWidth, b.y() + areaHeight);
	ResultPoint cp = ResultPoint(c.x() - areaWidth, c.y() - areaHeight);
	ResultPoint dp = ResultPoint(d.x() + areaWidth, d.y() - areaHeight);

	ResultPoint topLeftCorner, topRightCorner, bottomRightCorner, bottomLeftCorner;

	if (!GetCornerFromArea((int)a.x(), (int)ap.x(), (int)a.y(), (int)ap.y(), false, false, topLeftCorner)
		|| !GetCornerFromArea((int)bp.x(), (int)b.x(), (int)b.y(), (int)bp.y(), true, false, topRightCorner)
		|| !GetCornerFromArea((int)cp.x(), (int)c.x(), (int)cp.y(), (int)c.y(), true, true, bottomRightCorner)
		|| !GetCornerFromArea((int)d.x(), (int)dp.x(), (int)dp.y(), (int)d.y(), false, true, bottomLeftCorner)
		)
		return false;

	float xCorrection = (topRightCorner.x() - topLeftCorner.x()) / targetMatrixSize;
	float yCorrection = (bottomRightCorner.y() - topRightCorner.y()) / targetMatrixSize;

	/*ResultPoint topLeftCornerCenter = ResultPoint(topLeftCorner.x() + xCorrection, topLeftCorner.y() + yCorrection);
	ResultPoint topRightCornerCenter = ResultPoint(topRightCorner.x() - xCorrection, topRightCorner.y() + yCorrection);
	ResultPoint bottomRightCornerCenter = ResultPoint(bottomRightCorner.x() - xCorrection, bottomRightCorner.y() - yCorrection);
	ResultPoint bottomLeftCornerCenter = ResultPoint(bottomLeftCorner.x() + xCorrection, bottomLeftCorner.y() - yCorrection);*/

	result[0].set(topLeftCorner.x() + xCorrection, topLeftCorner.y() + yCorrection);
	result[1].set(topRightCorner.x() - xCorrection, topRightCorner.y() + yCorrection);
	result[2].set(bottomRightCorner.x() - xCorrection, bottomRightCorner.y() - yCorrection);
	result[3].set(bottomLeftCorner.x() + xCorrection, bottomLeftCorner.y() - yCorrection);
	return true;
}

bool CornerDetector::GetCornerFromArea(int left, int right, int top, int bottom, bool maximizeX, bool maximizeY, ResultPoint& result) const
{
	int resX = maximizeX ? 0 : INT32_MAX;
	int resY = maximizeY ? 0 : INT32_MAX;
	for (int x = left; x < right; x++) {
		for (int y = top; y < bottom; y++) {
			if (x > 0 && y > 0 && x < width && y < height) {
				if (image.get(x, y)) {
					if (maximizeX) {
						if (x > resX) {
							resX = x;
						}
					}
					else {
						if (x < resX) {
							resX = x;
						}
					}
					if (maximizeY) {
						if (y > resY) {
							resY = y;
						}
					}
					else {
						if (y < resY) {
							resY = y;
						}
					}
				}
			}
		}
	}
	if (resX == 0 || resY == 0) {
		return false;
	}
	else {
		result.set(resX, resY);
		return true;
	}
}

bool CornerDetector::ContainsBlackPoint(int a, int b, int fixed, bool horizontal) const
{
	if (horizontal) {
		for (int x = a; x <= b; x++) {
			if (image.get(x, fixed)) {
				return true;
			}
		}
	}
	else {
		for (int y = a; y <= b; y++) {
			if (image.get(fixed, y)) {
				return true;
			}
		}
	}

	return false;
}
