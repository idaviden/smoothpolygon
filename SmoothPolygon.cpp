#include "SmoothPolygon.h"
#include <math.h>

bool MarchingSquare::Process( const char *filename, std::vector<PointType> *result, TestFunc func /*= TransparentTest*/ )
{
	if (!m_png.Decode(filename))
		return false;
	unsigned int r, g, b, a;
	m_png.At(582, 1437, r, g, b, a);
	m_test = func;
	m_result = result;
	int startX, startY;
	if (!findStartPosition(startX, startY))
		return false;
	walk(startX, startY);
	return true;
}

bool MarchingSquare::findStartPosition( int& startX, int& startY )
{
	for (int y=0; y<m_png.height; y++) {
		for (int x=0; x<m_png.width; x++) {
			if (isPixelSolid(x, y)) {
				startX = x;
				startY = y;
				return true;
			}
		}
	}
	return false;
}

bool MarchingSquare::isPixelSolid( int x, int y )
{
	if (!m_png.IsValid(x, y)) {
		return false;
	}
	unsigned int r, g, b, a;
	m_png.At(x, y, r, g, b, a);
	if ((*m_test)(r, g, b, a)) {
		return true;
	}
	return false;
}

void MarchingSquare::step( int x, int y )
{
	auto upLeft = isPixelSolid(x-1, y-1);
	auto upRight = isPixelSolid(x, y-1);
	auto downLeft = isPixelSolid(x-1, y);
	auto downRight = isPixelSolid(x, y);

	m_previousStep = m_nextStep;

	auto state = 0;
	if (upLeft) {
		state |= 1;
	}
	if (upRight) {
		state |= 2;
	}
	if (downLeft) {
		state |= 4;
	}
	if (downRight) {
		state |= 8;
	}

	switch (state) {
	case 1: m_nextStep = DirectionUp; break;
	case 2: m_nextStep = DirectionRight; break;
	case 3: m_nextStep = DirectionRight; break;
	case 4: m_nextStep = DirectionLeft; break;
	case 5: m_nextStep = DirectionUp; break;
	case 6: 
		{
			if (m_previousStep == DirectionUp) {
				m_nextStep = DirectionLeft;
			} else {
				m_nextStep = DirectionRight;
			}
		}
		break;
	case 7: m_nextStep = DirectionRight; break;
	case 8: m_nextStep = DirectionDown; break;
	case 9: 
		{
			if (m_previousStep == DirectionRight) {
				m_nextStep = DirectionUp;
			} else {
				m_nextStep = DirectionDown;
			}
		}
		break;
	case 10: m_nextStep = DirectionDown; break;
	case 11: m_nextStep = DirectionDown; break;
	case 12: m_nextStep = DirectionLeft; break;
	case 13: m_nextStep = DirectionUp; break;
	case 14: m_nextStep = DirectionLeft; break;

	default:
		m_nextStep = DirectionNone;
		break;
	}
}

void MarchingSquare::walk( int startX, int startY )
{
	if (startX < 0) 
		startX = 0;
	if (startX > m_png.width)
		startX = m_png.width;
	if (startY < 0)
		startY = 0;
	if (startY > m_png.height)
		startY = m_png.height;

	auto x = startX;
	auto y = startY;
	while (true) {
		step(x, y);
		if (m_png.IsValid(x, y)) {
			m_result->push_back(PointType(x, y));
		}
		switch (m_nextStep)
		{
		case DirectionUp: y--; break;
		case DirectionLeft: x--; break;
		case DirectionDown: y++; break;
		case DirectionRight: x++; break;
		default:
			break;
		}
		if (x == startX && y == startY) {
			break;
		}
	}
}

float findPerpendicularDistance(PointType p, PointType p1, PointType p2) {
	float result = 0.0f;
	if (p1.x == p2.x) {
		result = abs(float(p.x - p1.x));		
	} else {
		float slope = float(p2.y - p1.y) / float(p2.x - p1.x);
		float intercept = float(p1.y) - (slope * float(p1.x));
		result = abs(slope * float(p.x) - float(p.y) + intercept) / sqrt(pow(slope, 2) + 1.0f);
	}
	return result;
}

std::vector<PointType> RDP( const std::vector<PointType>& points, float epsilon )
{
	if (points.size() < 3)
		return points;
	auto firstPoint = points[0];
	auto lastPoint = points[points.size() - 1];
	auto index = -1;
	float dist = 0.0f;

	for (int i=1; i<points.size()-1; i++) {
		auto cDist = findPerpendicularDistance(points[i], firstPoint, lastPoint);
		if (cDist > dist) {
			dist = cDist;
			index = i;
		}
	}
	if (dist > epsilon) {
		std::vector<PointType> l1, l2;
		l1.insert(l1.end(), points.begin(), points.begin() + index + 1);
		l2.insert(l2.end(), points.begin() + index, points.end());
		auto r1 = RDP(l1, epsilon);
		auto r2 = RDP(l2, epsilon);
		r1.pop_back();
		r1.insert(r1.end(), r2.begin(), r2.end());
		return r1;
	} else {
		std::vector<PointType> ret;
		ret.push_back(firstPoint);
		ret.push_back(lastPoint);
		return ret;
	}
	return std::vector<PointType>();
}
