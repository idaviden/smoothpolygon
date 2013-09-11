#ifndef SmoothPolygon__h__
#define SmoothPolygon__h__

#include "lodepng.h"

struct PointType {
	PointType(int xx, int yy) { x = xx; y = yy; }
	int x;
	int y;
};

enum DirectionType
{
	DirectionNone,
	DirectionUp,
	DirectionLeft,
	DirectionDown,
	DirectionRight,
};

typedef bool (*TestFunc) (unsigned int r, unsigned int g, unsigned int b, unsigned int a);

inline bool TransparentTest(unsigned int r, unsigned int g, unsigned int b,  unsigned int a) {
	return a > 0;
}

struct PngBuffer {
	std::vector<unsigned char> buffer;
	int width;
	int height;
	bool Decode(const char *filename) {
		unsigned error = lodepng::decode(buffer, (unsigned&)width, (unsigned&)height, filename);
		return error == 0;
	}
	bool IsValid(int x, int y) {
		if (x < 0 || x >= width || y < 0 || y >= height) {
			return false;
		}
		return true;
	}
	void At(int x, int y, unsigned int &r, unsigned int &g, unsigned int &b, unsigned int &a) {
		if (!IsValid(x, y)) {
			return;
		}
		 r = buffer[4 * y * width + 4 * x + 0]; //red
		 g = buffer[4 * y * width + 4 * x + 1]; //green
		 b = buffer[4 * y * width + 4 * x + 2]; //blue
		 a = buffer[4 * y * width + 4 * x + 3]; //alpha
	}
};

class MarchingSquare {
public:
	bool Process(const char *filename, std::vector<PointType> *result, TestFunc func = TransparentTest);
private:
	bool findStartPosition(int& startX, int& startY);
	bool isPixelSolid(int x, int y);
	void step(int x, int y);
	void walk(int startX, int startY);
private:
	PngBuffer	m_png;
	int			m_previousStep;
	int			m_nextStep;
	TestFunc	m_test;
	std::vector<PointType> *m_result;
};

std::vector<PointType> RDP(const std::vector<PointType>& points, float epsilon);

inline std::vector<PointType> MakeSmoothPolygon(const char *pngfilename, float epsilon = 0.5f) {
	MarchingSquare ms;
	std::vector<PointType> tmp;
	ms.Process(pngfilename, &tmp);
	return RDP(tmp, epsilon);
}

#endif // 
