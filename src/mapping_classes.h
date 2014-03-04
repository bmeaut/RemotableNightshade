#ifndef _MAPPING_CLASSES_HPP_
#define _MAPPING_CLASSES_HPP_

#include "mapping.h"

class MappingPerspective : public Mapping
{
	friend class Mapping; // Allow parent to construct

public:
	std::string getName(void) const {return "perspective";} 
private:
    MappingPerspective(void);
	bool forward(Vec3d &win) const;
	bool backward(Vec3d &v) const;
	double fovToViewScalingFactor(double fov) const;
	double viewScalingFactorToFov(double vsf) const;
	double deltaZoom(double fov) const;
};

class MappingEqualArea : public Mapping
{
	friend class Mapping; // Allow parent to construct

public:
	std::string getName(void) const {return "equal_area";} 
private:
    MappingEqualArea(void);
	bool forward(Vec3d &win) const;
	bool backward(Vec3d &v) const;
	double fovToViewScalingFactor(double fov) const;
	double viewScalingFactorToFov(double vsf) const;
	double deltaZoom(double fov) const;
};

class MappingStereographic : public Mapping
{
	friend class Mapping; // Allow parent to construct

public:
	std::string getName(void) const {return "stereographic";} 
private:
    MappingStereographic(void);
	bool forward(Vec3d &win) const;
	bool backward(Vec3d &v) const;
	double fovToViewScalingFactor(double fov) const;
	double viewScalingFactorToFov(double vsf) const;
	double deltaZoom(double fov) const;
};

class MappingFisheye : public Mapping
{
	friend class Mapping; // Allow parent to construct

public:
	std::string getName(void) const {return "fisheye";} 
private:
    MappingFisheye(void);
	bool forward(Vec3d &win) const;
	bool backward(Vec3d &v) const;
	double fovToViewScalingFactor(double fov) const;
	double viewScalingFactorToFov(double vsf) const;
	double deltaZoom(double fov) const;
};

class MappingCylinder : public Mapping
{
	friend class Mapping; // Allow parent to construct

public:
	std::string getName(void) const {return "cylinder";} 
private:
    MappingCylinder(void);
	bool forward(Vec3d &win) const;
	bool backward(Vec3d &v) const;
	double fovToViewScalingFactor(double fov) const;
	double viewScalingFactorToFov(double vsf) const;
	double deltaZoom(double fov) const;
};

class MappingOrthographic : public Mapping
{
	friend class Mapping; // Allow parent to construct

public:
	std::string getName(void) const {return "orthographic";} 
private:
    MappingOrthographic(void);
	bool forward(Vec3d &win) const;
	bool backward(Vec3d &v) const;
	double fovToViewScalingFactor(double fov) const;
	double viewScalingFactorToFov(double vsf) const;
	double deltaZoom(double fov) const;
};

#endif
