//----------------------------------------------------------------------------------------------
//	Filename:	PointSet.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
// Todo: Generalise structures used... there is way too much repetition (copy-pasting and 
//	     tweaking) of functionality available elsewhere in the core.
//----------------------------------------------------------------------------------------------
#pragma once

#include <Maths/Montecarlo.h>
#include <Scene/Visibility.h>
#include "PointGrid.h"
#include "PointShader.h"
#include "Environment.h"
#include "MultithreadedCommon.h"

#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

using namespace Illumina::Core;

struct Dart
{
public:
	enum Fields
	{
		F_Irradiance = 0,
		F_Position = F_Irradiance + sizeof(float) * 3,
		F_Normal = F_Position + sizeof(float) * 3,
		F_Occlusion = F_Normal + sizeof(float) * 3,
		F_Radius = F_Occlusion + sizeof(float),
		F_Size = F_Radius + sizeof(float)
	};

public:
	Spectrum Irradiance;
	Vector3 Position;
	Vector3 Normal;
	float Radius;
	float Occlusion;

	int Accumulated;
	bool Invalid;

public:
	std::string ToString(void)
	{
		std::stringstream result;

		result << Irradiance[0] << ", " << Irradiance[1] << ", " << Irradiance[2] << ", " <<
				Normal.X << ", " << Normal.Y << ", " << Normal.Z << ", " <<
				Position.X << ", " << Position.Y << ", " << Position.Z << ", " <<
				Occlusion << ", " << Radius << std::endl;

		return result.str();
	}

	void FromString(std::string &p_strInput)
	{
		char separator; std::stringstream tokenStream(p_strInput);

		tokenStream >> Irradiance[0] >> separator >> Irradiance[1] >> separator >> Irradiance[2] >> separator
				>> Normal.X >> separator >> Normal.Y >> separator >> Normal.Z >> separator 
				>> Position.X >> separator >> Position.Y >> separator >> Position.Z >> separator 
				>> Occlusion >> separator >> Radius;

		//Normal.X = -Normal.X;
		//Position.X = -Position.X;

		Invalid = true;
	} 

	/*
	float* Pack(float *p_pBase)
	{
		float *pNext = p_pBase;
		
		*pNext++ = Irradiance[0];
		*pNext++ = Irradiance[1];
		*pNext++ = Irradiance[2];

		*pNext++ = Position.Element[0];
		*pNext++ = Position.Element[1];
		*pNext++ = Position.Element[2];

		*pNext++ = Normal.Element[0];
		*pNext++ = Normal.Element[1];
		*pNext++ = Normal.Element[2];

		*pNext++ = Occlusion;
		*pNext++ = Radius;

		return pNext;
	} */

	int Pack(std::vector<float> *p_pElementList)
	{
		p_pElementList->push_back(Irradiance[0]);
		p_pElementList->push_back(Irradiance[1]);
		p_pElementList->push_back(Irradiance[2]);

		p_pElementList->push_back(Position.Element[0]);
		p_pElementList->push_back(Position.Element[1]);
		p_pElementList->push_back(Position.Element[2]);

		p_pElementList->push_back(Normal.Element[0]);
		p_pElementList->push_back(Normal.Element[1]);
		p_pElementList->push_back(Normal.Element[2]);

		p_pElementList->push_back(Occlusion);
		p_pElementList->push_back(Radius);

		return GetPackedSize();
	}

	inline float* PackUpdate(float *p_pElement)
	{
		p_pElement[0] = Irradiance[0];
		p_pElement[1] = Irradiance[1];
		p_pElement[2] = Irradiance[2];

		return p_pElement + GetPackedSize();
	}

	inline static int GetPackedByteSize(void) {
		return Dart::F_Size;
	}

	inline static int GetPackedSize(void) {
		return Dart::F_Size >> 2;
	}
};

template <class T>
class PointSet
{
protected:
	PointGrid<T> m_grid;

	Scene *m_pScene;

	Vector3 m_subdivisions;

	int m_nMaxPointSources,
		m_nMaxPointsPerSource,
		m_nAzimuthStrata,
		m_nAltitudeStrata;

	float m_fMinRadius,
		m_fMaxRadius,
		m_fOcclusionRadius,
		m_fMinRadiusPercent,
		m_fMaxRadiusPercent,
		m_fOcclusionRadiusPercent,
		m_fReflectEpsilon;

public:
	PointSet(void) { }

	bool Save(const std::string &p_strFilename)
	{
		std::ofstream pointFile;
		pointFile.open(p_strFilename.c_str(), std::ios::binary);

		std::vector<T> pointList;
		m_grid.Get(pointList);
		
		for (auto point : pointList)
		{
			pointFile << point.ToString() << std::endl;
		}

		pointFile.close();

		std::cout << "PointSet<T> :: Written [" << pointList.size() << "] points." << std::endl;

		return true;
	}

	bool Load(const std::string &p_strFilename)
	{
		T point; std::string line;

		std::ifstream pointFile;
		pointFile.open(p_strFilename.c_str(), std::ios::binary);

		m_grid.Clear();
		
		while(std::getline(pointFile, line))
		{
			point.FromString(line);
			m_grid.Add(point.Position, point);
		}

		pointFile.close();

		std::cout << "PointSet<T> :: Read [" << m_grid.Size() << "] points." << std::endl;

		return true;
	}

	void Initialise(Scene *p_pScene, float p_fMinRadius, float p_fMaxRadius, float p_fOcclusionRadius, int p_nMaxPointSources, int p_nMaxPointsPerSource, int p_nAzimuthStrata, int p_nAltitudeStrata, float p_fReflectEpsilon, const Vector3 &p_subdivisions)
	{
		m_pScene = p_pScene;
		m_fMinRadiusPercent = p_fMinRadius;
		m_fMaxRadiusPercent = p_fMaxRadius;
		m_fOcclusionRadiusPercent = p_fOcclusionRadius; 
		m_nMaxPointSources = p_nMaxPointSources;
		m_nMaxPointsPerSource = p_nMaxPointsPerSource;
		m_nAzimuthStrata = p_nAltitudeStrata;
		m_nAltitudeStrata = p_nAltitudeStrata;
		m_fReflectEpsilon = p_fReflectEpsilon;
		m_subdivisions = p_subdivisions;

		IBoundingVolume *pBoundingVolume = m_pScene->GetSpace()->GetBoundingVolume();

		Vector3 centroid = pBoundingVolume->GetCentre();
		Vector3 size = pBoundingVolume->GetSize();

		m_grid.Initialise(centroid, size, m_subdivisions);

		m_fMinRadius = m_fMinRadiusPercent * size.MaxAbsComponent();
		m_fMaxRadius = m_fMaxRadiusPercent * size.MaxAbsComponent();
		m_fOcclusionRadius = m_fOcclusionRadiusPercent * size.MaxAbsComponent();
	}

	PointGrid<T>& GetContainerInstance(void) 
	{
		return m_grid;
	}

	void Generate(bool p_bSavePoints = true, bool p_bSavePointSources = true)
	{
		// Dart source (emitter) list
		std::vector<T> pointSourceList;

		// Clear grid : generating points
		m_grid.Clear();

		// Generate point sources
		std::cout << "Generating point sources..." << std::endl;
		GeneratePointSource(m_nMaxPointSources * 6, 6, pointSourceList);

		// Persist point sources
		if (p_bSaveDartSources)
		{
			std::ofstream pointSourceFile; pointSourceFile.open("Output//pointSources.asc", std::ios::binary);

			for (auto pointSource : pointSourceList)
				pointSourceFile << pointSource.Position.X << ", " << pointSource.Position.Y << ", " << pointSource.Position.Z << std::endl;

			pointSourceFile.close();

			std::cout << "Saved point sources." << std::endl;
		}

		// Throw darts
		std::cout << "Generating points..." << std::endl;
		GeneratePoints(pointSourceList);

		// Get points
		std::vector<T> points; m_grid.Get(points);

		// Persist
		if (p_bSavePoints)
		{
			std::ofstream pointFile; pointFile.open("Output//points.asc", std::ios::binary);
		
			for (auto point : points)
				pointFile << point.Position.X << ", " << point.Position.Y << ", " << point.Position.Z << std::endl;

			pointFile.close();

			std::cout << "Saved points." << std::endl;
		}

		std::cout << "Point cloud [" << points.size() << "] generated using [" << pointSourceList.size() << "] emitters." << std::endl;
	}

protected:
	/* 
	 * Generate points in final point cloud from dart sources 
	 */
	void GeneratePoints(std::vector<T> &p_pointSourceList)
	{
		Ray dartRay;
		Vector3 direction;
		Vector2 rayDetails, 
			sample;

		IBoundingVolume *pBoundingVolume = m_pScene->GetSpace()->GetBoundingVolume();
		Vector3 centroid = pBoundingVolume->GetCentre();
		Vector3 size = pBoundingVolume->GetSize();

		Intersection intersection;
		OrthonormalBasis basis;

		int discarded = 0,
			dartCount = 0, 
			dartMaxCount = m_nMaxPointSources * m_nMaxPointsPerSource;

		// Initialise grid with give parameters
		m_grid.Initialise(centroid, size, m_subdivisions);
		
		// Start throwing darts for each source
		for (auto dartSource : p_pointSourceList)
		{
			LowDiscrepancySampler directionSampler;

			for (int dartIndex = 0; dartIndex < m_nMaxPointsPerSource; dartIndex++)
			{
				// Sample new direction
				sample = directionSampler.Get2DSample();
				direction = Montecarlo::UniformSampleSphere(sample.U, sample.V);

				// Create a basis coordinate frame and project direction
				basis.InitFromW(dartSource.Normal);
				direction.Z = Maths::Abs(direction.Z);
				direction = basis.Project(direction);

				// Set ray direction
				dartRay.Set(dartSource.Position + direction * m_fReflectEpsilon, direction, m_fReflectEpsilon, Maths::Maximum);
				
				// Intersection
				if (m_pScene->Intersects(dart, intersection))
				{
					std::cout << "[" << dartCount++ << " of " << dartMaxCount << "] Index : " << dartIndex << ", Centre : " << intersection.Surface.PointWS.ToString() << std::endl; 

					// Reject trivially
					if (m_grid.Contains(intersection.Surface.PointWS, m_fMinRadius)) 
					{
						discarded++;
						continue;
					}

					T point;

					point.Invalid = true;
					point.Irradiance = 0.0f;
					point.Position = intersection.Surface.PointWS;
					point.Normal = intersection.Surface.ShadingBasisWS.W;
					point.Occlusion = ComputeAmbientOcclusion(intersection, m_fOcclusionRadius, rayDetails);
					point.Radius = dartPoint.Occlusion * m_fMaxRadius;

					std::cout << "Ambient occlusion computed [" << point.Occlusion << "] for radius [" << point.Radius << "]" << std::endl;
					if (point.Radius > m_fMinRadius && m_grid.Contains(intersection.Surface.PointWS, point.Radius)) 
					{
						discarded++;
						continue;
					}

					std::cout << "Point accepted" << std::endl;

					// Make sure contribution is not less than minradius
					point.Radius = Maths::Max(point.Radius, m_fMinRadius);

					m_grid.Add(point.Position, point);
				}
			}
		}

		std::cout << "Grid Generation :: Points discarded [" << discarded << "]" << std::endl;
	}

	/*
	 * Generate point sources by following photon emitters in scene
	 */
	void GeneratePointSources(int p_nMaxPaths, int p_nMaxBounces, std::vector<Dart> &p_pointSourceList)
	{
		Intersection intersection;
		IMaterial *pMaterial;
		BxDF::Type bxdfType = BxDF::All_Combined;
		T pointSource;
		Ray lightRay;

		Spectrum contribution, 
			alpha, f;

		Vector3 normal, 
			wOut, wIn;

		LowDiscrepancySampler positionSampler,
			directionSampler;

		Vector2 positionSample, 
			directionSample,
			sample;

		float continueProbability, 
			pdf;

		int intersections;

		// Trace either a maximum number of paths or virtual point lights
		for (int lightIndex = 0, nPathIndex = p_nMaxPaths; nPathIndex > 0 && p_pointSourceList.size() < m_nMaxPointSources; --nPathIndex)
		{
			// Get samples for initial position and direction
			positionSample = positionSampler.Get2DSample();
			directionSample = directionSampler.Get2DSample();
		
			// Get initial radiance, position and direction
			alpha = m_pScene->LightList[lightIndex]->SampleRadiance(
				m_pScene, positionSample.U, positionSample.V,
				directionSample.U, directionSample.V, lightRay, pdf);

			// If pdf or radiance are zero, choose a new path
			if (pdf == 0.0f || alpha.IsBlack())
				continue;

			// Scale radiance by pdf
			alpha /= pdf;

			// Start tracing virtual point light path
			for (intersections = 1; m_pScene->Intersects(lightRay, intersection); ++intersections)
			{
				wOut = -lightRay.Direction;
				pMaterial = intersection.GetMaterial();
				Spectrum Le = alpha * pMaterial->Rho(wOut, intersection.Surface) / Maths::Pi;

				// Set point light parameters
				pointSource.Position = intersection.Surface.PointWS;
				pointSource.Normal = intersection.Surface.GeometryBasisWS.W;

				// Push point light on list
				p_pointSourceList.push_back(pointSource);

				// Sample new direction
				sample = m_pScene->GetSampler()->Get2DSample();

				f = pMaterial->SampleF(intersection.Surface, wOut, wIn, sample.U, sample.V, &pdf, bxdfType);
			
				// If reflectivity or pdf are zero, end path
				if (f.IsBlack() || pdf == 0.0f || intersections > p_nMaxBounces)
					break;

				// Compute contribution of path
				contribution = f * Vector3::AbsDot(wIn, intersection.Surface.ShadingBasisWS.W) / pdf;

				// Possibly terminate virtual light path with Russian roulette
				continueProbability = Maths::Min(1.f, (contribution[0] + contribution[1] + contribution[2]) * 0.33f);
				if (m_pScene->GetSampler()->Get1DSample() > continueProbability)
						break;

				// Modify contribution accordingly
				alpha *= contribution / continueProbability;

				// Set new ray position and direction
				lightRay.Set(intersection.Surface.PointWS, wIn, m_fReflectEpsilon, Maths::Maximum);
			}

			// Increment light index, and reset if we traversed all scene lights
			if (++lightIndex == m_pScene->LightList.Size())
				lightIndex = 0;
		}

		// Just in case we traced more than is required
		if (p_pointSourceList.size() > m_nMaxPointSources)
			p_pointSourceList.erase(p_pointSourceList.begin() + m_nMaxPointSources, p_pointSourceList.end());
	}

	/*
	 * Compute ambient occlusion
	 */
	float ComputeAmbientOcclusion(Intersection &p_intersection, float p_fRadius, Vector2 &p_rayLengths)
	{
		Vector2 sample2D;
		Vector3 vH, wOutR;

		Ray ray;
		Intersection intersection;

		float totalLength = 0, harmonicMean = 0;
		int mn = m_nAzimuthStrata * m_nAltitudeStrata;
		
		p_rayLengths.Set(Maths::Minimum, Maths::Maximum);

		for (int altitudeIndex = 0; altitudeIndex < m_nAltitudeStrata; altitudeIndex++)
		{
			for (int azimuthIndex = 0; azimuthIndex < m_nAzimuthStrata; azimuthIndex++)
			{
				sample2D = m_pScene->GetSampler()->Get2DSample();

				vH = Montecarlo::CosineSampleHemisphere(sample2D.X, sample2D.Y, (float)altitudeIndex, (float)azimuthIndex, (float)m_nAltitudeStrata, (float)m_nAzimuthStrata); 

				BSDF::SurfaceToWorld(p_intersection.WorldTransform, p_intersection.Surface, vH, wOutR);
				ray.Set(p_intersection.Surface.PointWS + wOutR * m_fReflectEpsilon, wOutR, m_fReflectEpsilon, Maths::Maximum);
				
				if (m_pScene->Intersects(ray, intersection))
				{
					ray.Max = intersection.Surface.Distance;

					harmonicMean += 1.f / ray.Max;
					totalLength += Maths::Min(ray.Max, p_fRadius);

					p_rayLengths.U = Maths::Max(ray.Max, p_rayLengths.U);
					p_rayLengths.V = Maths::Min(ray.Max, p_rayLengths.V);
				}
			}
		}

		return Maths::Clamp(totalLength / (mn * p_fRadius), 0, 1);
	}
};
