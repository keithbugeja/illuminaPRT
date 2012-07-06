#pragma once

#include <string>
#include <boost/shared_ptr.hpp>
#include "System/Platform.h"

namespace Illumina
{
	namespace Core
	{
		class EngineKernel;

		class Scene;
		class Environment;
		class IBoundingVolume;
		class IIntegrator;
		class IPrimitive;
		class IRenderer;
		class IMaterial;
		class ISampler;
		class IFilter;
		class ICamera;
		class IDevice;
		class IFragment;
		class IShape;
		class ILight;
		class BSDF;
		class BxDF;
		class RGBSpectrum;

		class ISpace;
		class ITriangleMesh;

		class RadianceContext;
		class RadianceBuffer;
		class Intersection;
		class DifferentialSurface;
		class VisibilityQuery;

		class OrthonormalBasis;
		class Transformation;
		class Interval;
		class Vector2;
		class Vector3;
		class Plane;
		class Ray;

		class Image;
		class IImageIO;

		class PlugInManager;

		typedef RGBSpectrum Spectrum;
	}
}
