#ifndef _COORDINATE_CONVERTER_H_
#define _COORDINATE_CONVERTER_H_

class CoordinateConverter
{
	public:
		void depthToWorld(int depthX, int depthY, int depthZ, float& worldX, float& worldY, float& worldZ);

		CoordinateConverter();
		CoordinateConverter(int scrCntrX, int scrCntrY, float hFV, float vFV);

	private:
		const int screenHeight;
		const int screenWidth;

		const float horizontalFieldOfView;
		const float verticalFieldOfView;

		int screenCenterX;
		int screenCenterY;

		float f_x;
		float f_y;

		float rawDepthToMeters(int rawDepth);
		float degToRad(float degValue);

		void setScreenCenter();
		void setCoeffXAndCoeffY();
};

#endif /* _COORDINATE_CONVERTER_H_ */
