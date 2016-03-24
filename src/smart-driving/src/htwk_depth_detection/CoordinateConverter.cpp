#include <cmath>

#include "CoordinateConverter.h"


CoordinateConverter::CoordinateConverter()
	: screenHeight(240), screenWidth(320), horizontalFieldOfView(58.0f), verticalFieldOfView(45.0f)
{
	setScreenCenter();
	setCoeffXAndCoeffY();
}


CoordinateConverter::CoordinateConverter(int scrHghtX, int scrWdthY, float hFV, float vFV)
	: screenHeight(scrHghtX), screenWidth(scrWdthY), horizontalFieldOfView(hFV), verticalFieldOfView(vFV)
{
	setScreenCenter();
	setCoeffXAndCoeffY();
}


void CoordinateConverter::setScreenCenter()
{
	screenCenterX = screenWidth / 2;
	screenCenterY = screenHeight / 2;
}


void CoordinateConverter::setCoeffXAndCoeffY()
{
	float xzFactor = (2.0f * tan(degToRad(horizontalFieldOfView/2.0f)));
	float yzFactor = (2.0f * tan(degToRad(verticalFieldOfView/2.0f)));

	f_x = static_cast<float>(screenWidth) / xzFactor;
	f_y = static_cast<float>(screenHeight) / yzFactor;
}


void CoordinateConverter::depthToWorld(int depthX, int depthY, int depthZ, float& worldX, float& worldY, float& worldZ)
{
	worldZ = rawDepthToMeters(depthZ);

	if ((screenCenterX - depthX) && (screenCenterY - depthY))
	{
		worldX = worldZ * static_cast<float>(screenCenterX - depthX) / f_x;
		worldY = worldZ * static_cast<float>(screenCenterY - depthY) / f_y;
	}
	else
	{
		worldX = 0.0f;
		worldY = 0.0f;
		worldZ = 0.0f;
	}
}


float CoordinateConverter::rawDepthToMeters(int rawDepth)
{
	float rawDepthF = static_cast<double>(rawDepth);

	if (rawDepth <= 0xFFFF)
	{
		// following values acquired during aadc xtion depth sensor calibration
		float depth = 3.33776840956e-05f * rawDepthF + -0.0674169620271f;
		return (depth);
	}

	return 0.0f;
}


float CoordinateConverter::degToRad(float degValue)
{
	return degValue * M_PI / 180.0f;
}
