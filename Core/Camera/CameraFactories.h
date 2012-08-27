//----------------------------------------------------------------------------------------------
//	Filename:	CameraFactories.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>
#include <string>
#include <iostream>

#include "Camera/Camera.h"
#include "Camera/PerspectiveCamera.h"
#include "Camera/ThinLensCamera.h"

namespace Illumina
{
	namespace Core
	{
		class PerspectiveCameraFactory : public Illumina::Core::Factory<Illumina::Core::ICamera>
		{
		public:
			Illumina::Core::ICamera *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			/*
			 * Arguments
			 * -- Id {String}
			 * -- Aspect {Float}
			 * -- Fov {Float}
			 * -- Position {Vector3}
			 * -- Target {Vector3}
			 * -- Up {Vector3}
			 * -- Frame {Vector4}
			 * -- Distance {Float}
			 */
			Illumina::Core::ICamera *CreateInstance(ArgumentMap &p_argumentMap)
			{
				#if defined(__PLATFORM_WINDOWS__)
				#pragma message ("Camera frame arguments are ignored at the moment.")
				#endif

				float aspect = 1.0f,
					fov = 60.0f,
					distance = 1.0f,
					left = -1.0f, right = 1.0f,
					top = 1.0f, bottom = -1.0f;

				Vector3 position(Vector3::UnitZNeg),
					target(Vector3::Zero),
					up(Vector3::UnitYPos);

				std::string strId;

				p_argumentMap.GetArgument("Aspect", aspect);
				p_argumentMap.GetArgument("Fov", fov);
				p_argumentMap.GetArgument("Position", position);
				p_argumentMap.GetArgument("Distance", distance);
				p_argumentMap.GetArgument("Target", target);
				p_argumentMap.GetArgument("Up", up);

				ICamera *pCamera = NULL;

				if (p_argumentMap.GetArgument("Id", strId))
				{
					pCamera = CreateInstance(strId, position, Vector3::Normalize(target - position), up, 
						left, right, bottom, top, distance);
				}
				else
				{
					pCamera = CreateInstance(position, Vector3::Normalize(target - position), up, 
						left, right, bottom, top, distance);
				}

				pCamera->SetFieldOfView(fov, aspect);

				return pCamera;
			}

			Illumina::Core::ICamera *CreateInstance(const std::string &p_strId, 
				const Vector3 &p_position, const Vector3 &p_direction, const Vector3 &p_up,
				float p_fLeft, float p_fRight, float p_fBottom, float p_fTop, float p_fDistance)
			{
				return new PerspectiveCamera(p_strId, p_position, p_direction, p_up, p_fLeft, p_fRight, p_fBottom, p_fTop, p_fDistance);
			}

			Illumina::Core::ICamera *CreateInstance(const Vector3 &p_position, const Vector3 &p_direction, 
				const Vector3 &p_up, float p_fLeft, float p_fRight, float p_fBottom, float p_fTop, float p_fDistance)
			{
				return new PerspectiveCamera(p_position, p_direction, p_up, p_fLeft, p_fRight, p_fBottom, p_fTop, p_fDistance);
			}
		};

		class ThinLensCameraFactory : public Illumina::Core::Factory<Illumina::Core::ICamera>
		{
		public:
			Illumina::Core::ICamera *CreateInstance(void)
			{
				throw new Exception("Method not supported!");
			}

			// Arguments
			// -- Id {String}
			// -- Aspect {Float}
			// -- Fov {Float}
			// -- Position {Vector3}
			// -- Target {Vector3}
			// -- Up {Vector3}
			// -- Frame {Vector4}
			// -- Distance {Float}
			Illumina::Core::ICamera *CreateInstance(ArgumentMap &p_argumentMap)
			{
				#if defined(__PLATFORM_WINDOWS__)
				#pragma message ("Camera frame arguments are ignored at the moment.")
				#endif

				float aperture = 0.0f, 
					aspect = 1.0f,
					fov = 60.0f,
					distance = 1.0f,
					left = -1.0f, right = 1.0f,
					top = 1.0f, bottom = -1.0f;

				Vector3 position(Vector3::UnitZNeg),
					target(Vector3::Zero),
					up(Vector3::UnitYPos);

				std::string strId;

				p_argumentMap.GetArgument("Aperture", aperture);
				p_argumentMap.GetArgument("Aspect", aspect);
				p_argumentMap.GetArgument("Fov", fov);
				p_argumentMap.GetArgument("Position", position);
				p_argumentMap.GetArgument("Distance", distance);
				p_argumentMap.GetArgument("Target", target);
				p_argumentMap.GetArgument("Up", up);

				ICamera *pCamera = NULL;

				if (p_argumentMap.GetArgument("Id", strId))
				{
					pCamera = CreateInstance(strId, position, Vector3::Normalize(target - position), up, 
						aperture, left, right, bottom, top, distance);
				}
				else
				{
					pCamera = CreateInstance(position, Vector3::Normalize(target - position), up, 
						aperture, left, right, bottom, top, distance);
				}

				pCamera->SetFieldOfView(fov, aspect);

				return pCamera;
			}

			Illumina::Core::ICamera *CreateInstance(const std::string &p_strId, 
				const Vector3 &p_position, const Vector3 &p_direction, const Vector3 &p_up, float p_fAperture, 
				float p_fLeft, float p_fRight, float p_fBottom, float p_fTop, float p_fDistance)
			{
				return new ThinLensCamera(p_strId, p_position, p_direction, p_up, p_fAperture, p_fLeft, p_fRight, p_fBottom, p_fTop, p_fDistance);
			}

			Illumina::Core::ICamera *CreateInstance(const Vector3 &p_position, const Vector3 &p_direction, const Vector3 &p_up, 
				float p_fAperture, float p_fLeft, float p_fRight, float p_fBottom, float p_fTop, float p_fDistance)
			{
				return new ThinLensCamera(p_position, p_direction, p_up, p_fAperture, p_fLeft, p_fRight, p_fBottom, p_fTop, p_fDistance);
			}
		};
	}
}