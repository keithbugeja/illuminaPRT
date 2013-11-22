#pragma once
#include <Maths/Montecarlo.h>
#include <Scene/Visibility.h>
#include "Environment.h"

using namespace Illumina::Core;

struct PhotonEmitter
{
	Spectrum Contribution;
	Vector3	Position;
	Vector3 Normal;
	Vector3 Direction;
	bool Invalid;
};

struct Dart
{
	Spectrum Irradiance;
	Vector3 Position;
	Vector3 Normal;
	float Radius;
	float Occlusion;
	bool Invalid;
};

template<class T>
class PointGrid
{
protected:
	std::vector<T*> m_pointList;
	std::map<Int64, std::vector<T*>> m_pointGrid;

	Vector3 m_centroid,
		m_size, m_partitions,
		m_origin;

	struct Hash 
	{
		Int64 X : 21;
		Int64 Y : 21;
		Int64 Z : 21;
	};

public:
	Int64 MakeKey(Vector3 &p_position)
	{
		Vector3 floatKey = (p_position - m_origin);
		floatKey.Set(floatKey.X / m_partitions.X, floatKey.Y / m_partitions.Y, floatKey.Z / m_partitions.Z); 
		
		Hash hash;

		hash.X = (int)Maths::Floor(floatKey.X) & 0xFFFFF;
		hash.Y = (int)Maths::Floor(floatKey.Y) & 0xFFFFF;
		hash.Z = (int)Maths::Floor(floatKey.Z) & 0xFFFFF;

		return *(Int64*)&hash;
	}

protected:
	void AddRange(T *p_pPoint) 
	{
		Vector3 extent(p_pPoint->Radius),
			iterator;

		Vector3 minExtent(p_pPoint->Position - extent),
				maxExtent(p_pPoint->Position + extent);

		Vector3 stepSize(m_size.X / m_partitions.X, m_size.Y / m_partitions.Y, m_size.Z / m_partitions.Y);

		for (iterator.X = minExtent.X; iterator.X < maxExtent.X; iterator.X+=stepSize.X)
		{
			for (iterator.Y = minExtent.Y; iterator.Y < maxExtent.Y; iterator.Y+=stepSize.Y)
			{
				for (iterator.Z = minExtent.Z; iterator.Z < maxExtent.Z; iterator.Z+=stepSize.Z)
				{
					Int64 key = MakeKey(iterator);
					m_pointGrid[key].push_back(p_pPoint);
				}
			}
		}
	}

public:
	PointGrid(void) { }

	PointGrid(Vector3 &p_centroid, Vector3 &p_size, Vector3 &p_partitions)
		: m_centroid(p_centroid)
		, m_size(p_size)
		, m_partitions(p_partitions)
		, m_origin(m_centroid - m_size / 2)
	{ }

	~PointGrid(void) {
		Clear();
	}

	void Initialise(Vector3 &p_centroid, Vector3 &p_size, Vector3 &p_partitions)
	{
		m_centroid = p_centroid;
		m_size = p_size;
		m_partitions = p_partitions;
		m_origin = m_centroid - m_size / 2;

		Clear();
	}

	void Clear(void) 
	{ 		
		for (auto point : m_pointList)
			delete point;

		m_pointList.clear();
		m_pointGrid.clear();
	}

	std::vector<T*>& Get(Int64 p_key) {
		return m_pointGrid[p_key];
	}

	std::vector<T*>& Get(Vector3 p_position) {
		return Get(MakeKey(p_position));
	}

	void Add(Int64 p_key, T& p_point) 
	{
		T* point = new T(p_point);
		m_pointList.push_back(point);

		AddRange(point);
	}

	void Add(Vector3 p_position, T& p_point) {
		Add(MakeKey(p_position), p_point);
	}

	void Get(std::vector<T> &p_pointList)
	{
		p_pointList.clear();

		for (auto point : m_pointList)
			p_pointList.push_back(*point);
	}

	std::vector<T*> Get(void) { 
		return m_pointList; 
	}

	bool Contains(Vector3 p_centre, float p_fRadius)
	{
		Vector3 minExtent(p_centre.X - p_fRadius,
			p_centre.Y - p_fRadius,
			p_centre.Z - p_fRadius),
				maxExtent(p_centre.X + p_fRadius,
			p_centre.Y + p_fRadius,
			p_centre.Z + p_fRadius);

		Vector3 stepSize(m_size.X / m_partitions.X, m_size.Y / m_partitions.Y, m_size.Z / m_partitions.Y);
		Vector3 iterator;

		for (iterator.X = minExtent.X; iterator.X < maxExtent.X; iterator.X+=stepSize.X)
		{
			for (iterator.Y = minExtent.Y; iterator.Y < maxExtent.Y; iterator.Y+=stepSize.Y)
			{
				for (iterator.Z = minExtent.Z; iterator.Z < maxExtent.Z; iterator.Z+=stepSize.Z)
				{
					// if (Vector3::Distance(iterator, p_centre) <= p_fRadius)
					{
						std::vector<T*> list = Get(iterator);
						for(auto point : list)
						{
							if (Vector3::Distance(point->Position, p_centre) < p_fRadius)
								return true;
						}
					}
				}
			}
		}

		return false;
	}

	size_t Size(void) { 
		return m_pointList.size();		
	}

};

class PointSet
{
protected:
	PointGrid<Dart> m_grid;

	Scene *m_pScene;
	Vector3 m_subdivisions;
	int m_nMaxDartSources,
		m_nMaxDartsPerSource,
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
		std::ofstream dartFile;
		dartFile.open(p_strFilename.c_str(), std::ios::binary);

		std::vector<Dart> dartList;
		m_grid.Get(dartList);
		
		for (auto dart : dartList)
		{
			dartFile << dart.Irradiance[0] << ", " << dart.Irradiance[1] << ", " << dart.Irradiance[2] << ", " <<
				dart.Normal.X << ", " << dart.Normal.Y << ", " << dart.Normal.Z << ", " <<
				dart.Position.X << ", " << dart.Position.Y << ", " << dart.Position.Z << ", " <<
				dart.Occlusion << ", " << dart.Radius << std::endl;
		}

		dartFile.close();

		return true;
	}

	bool Load(const std::string &p_strFilename)
	{
		Dart dart;
		char separator; float dummy;
		std::string line;

		std::ifstream dartFile;
		dartFile.open(p_strFilename.c_str(), std::ios::binary);

		m_grid.Clear();
		
		while(std::getline(dartFile, line))
		{
			std::istringstream tokenStream(line);
			
			tokenStream >> dart.Irradiance[0] >> separator >> dart.Irradiance[1] >> separator >> dart.Irradiance[2] >> separator
				>> dart.Normal.X >> separator >> dart.Normal.Y >> separator >> dart.Normal.Z >> separator 
				>> dart.Position.X >> separator >> dart.Position.Y >> separator >> dart.Position.Z >> separator 
				>> dart.Occlusion >> separator >> dart.Radius;

			m_grid.Add(dart.Position, dart);
		}

		dartFile.close();

		std::cout << "PointGrid :: [" << m_grid.Size() << "] points loaded." << std::endl;

		return true;
	}

	void Initialise(Scene *p_pScene, float p_fMinRadius, float p_fMaxRadius, float p_fOcclusionRadius, int p_nMaxDartSources, int p_nMaxDartsPerSource, int p_nAzimuthStrata, int p_nAltitudeStrata, float p_fReflectEpsilon, const Vector3 &p_subdivisions)
	{
		m_pScene = p_pScene;
		m_fMinRadiusPercent = p_fMinRadius;
		m_fMaxRadiusPercent = p_fMaxRadius;
		m_fOcclusionRadiusPercent = p_fOcclusionRadius; 
		m_nMaxDartSources = p_nMaxDartSources;
		m_nMaxDartsPerSource = p_nMaxDartsPerSource;
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

	PointGrid<Dart>& Get(void) {
		return m_grid;
	}

	void Generate(void)
	{
		m_grid.Clear();

		std::vector<Dart> dartSourceList;

		// Generate dart sources
		std::cout << "Generating dart sources..." << std::endl;
		GenerateDartSources(m_nMaxDartSources * 6, 6, dartSourceList);

		// Persist
		std::ofstream dartFile;
		dartFile.open("Output//dartCloud.asc", std::ios::binary);

		for (auto darts : dartSourceList)
			dartFile << darts.Position.X << ", " << darts.Position.Y << ", " << darts.Position.Z << std::endl;

		dartFile.close();

		// Throw darts
		std::cout << "Generate darts..." << std::endl;
		GenerateDarts(dartSourceList);

		// Persist
		dartFile.open("Output//pointCloud.asc", std::ios::binary);
		
		std::vector<Dart> pointCloud; m_grid.Get(pointCloud);
		for (auto dart : pointCloud)
			dartFile << dart.Position.X << ", " << dart.Position.Y << ", " << dart.Position.Z << std::endl;

		dartFile.close();
	}

protected:
	void GenerateDarts(std::vector<Dart> &p_dartSourceList)
	{
		Ray dart;
		Vector2 sample, rayDetails;
		Vector3 direction;
		Intersection intersection;

		IBoundingVolume *pBoundingVolume = m_pScene->GetSpace()->GetBoundingVolume();
		
		Vector3 centroid = pBoundingVolume->GetCentre();
		Vector3 size = pBoundingVolume->GetSize();
		OrthonormalBasis basis;

		int discarded = 0,
			dartCount = 0, 
			dartMaxCount = m_nMaxDartSources * m_nMaxDartsPerSource;

		m_grid.Initialise(centroid, size, m_subdivisions);
		
		for (auto dartSource : p_dartSourceList)
		{
			for (int dartIndex = 0; dartIndex < m_nMaxDartsPerSource; dartIndex++)
			{
				sample = m_pScene->GetSampler()->Get2DSample();
				
				direction = Montecarlo::UniformSampleSphere(sample.U, sample.V);

				basis.InitFromW(dartSource.Normal);
				direction.Z = Maths::Abs(direction.Z);
				direction = basis.Project(direction);

				dart.Set(dartSource.Position + direction * m_fReflectEpsilon, direction, m_fReflectEpsilon, Maths::Maximum);
				
				if (m_pScene->Intersects(dart, intersection))
				{
					std::cout << "[" << dartCount++ << " of " << dartMaxCount << "] Index : " << dartIndex << ", Centre : " << intersection.Surface.PointWS.ToString() << std::endl; 

					// Reject trivially
					/* */
					if (m_grid.Contains(intersection.Surface.PointWS, m_fMinRadius)) 
					{
						discarded++;
						continue;
					}
					/* */

					// std::cout << "Point not trivially rejected" << std::endl;

					Dart dartPoint;

					dartPoint.Position = intersection.Surface.PointWS;
					dartPoint.Normal = intersection.Surface.ShadingBasisWS.W;
					dartPoint.Occlusion = ComputeAmbientOcclusion(intersection, m_fOcclusionRadius, rayDetails);
					dartPoint.Radius = dartPoint.Occlusion * m_fMaxRadius;

					std::cout << "Ambient occlusion computed [" << dartPoint.Occlusion << "] for radius [" << dartPoint.Radius << "]" << std::endl;
					/* */
					if (dartPoint.Radius > m_fMinRadius && m_grid.Contains(intersection.Surface.PointWS, dartPoint.Radius)) 
					{
						discarded++;
						continue;
					}
					/* */

					std::cout << "Point accepted" << std::endl;

					// Make sure contribution is not less than minradius
					dartPoint.Radius = Maths::Max(dartPoint.Radius, m_fMinRadius);

					m_grid.Add(dartPoint.Position, dartPoint);
				}
			}
		}

		std::cout << "Grid Generation :: Points discarded [" << discarded << "]" << std::endl;
	}

	void GenerateDartSources(int p_nMaxPaths, int p_nMaxBounces, std::vector<Dart> &p_dartSourceList)
	{
		Intersection intersection;
		IMaterial *pMaterial;
		BxDF::Type bxdfType = BxDF::All_Combined;
		Dart dartThrower;
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
		for (int lightIndex = 0, nPathIndex = p_nMaxPaths; nPathIndex > 0 && p_dartSourceList.size() < m_nMaxDartSources; --nPathIndex)
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
				dartThrower.Position = intersection.Surface.PointWS;
				dartThrower.Normal = intersection.Surface.GeometryBasisWS.W;

				// Push point light on list
				p_dartSourceList.push_back(dartThrower);

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
		if (p_dartSourceList.size() > m_nMaxDartSources)
			p_dartSourceList.erase(p_dartSourceList.begin() + m_nMaxDartSources, p_dartSourceList.end());
	}

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

		//p_rayLengths.U = Maths::Clamp(mn / harmonicMean, m_fMinRadius, m_fMaxRadius);
		//return Maths::Min(p_rayLengths.V, p_fRadius) / p_fRadius;
		return Maths::Clamp(totalLength / (mn * p_fRadius), 0, 1);
	}
};

class PointShader
{
protected:
	int m_nAzimuthStrata,
		m_nAltitudeStrata,
		m_nShadowRayCount,
		m_nRayDepth;

	float m_fReflectEpsilon;

	Scene *m_pScene;

public:
	void TraceEmitters(std::vector<PhotonEmitter> &p_photonEmitterList, int p_nMaxEmitters, int p_nMaxPaths)
	{
		Ray lightRay;
		IMaterial *pMaterial;
		PhotonEmitter emitter;
		Intersection intersection;
		BxDF::Type bxdfType = 
			BxDF::All_Combined;

		Spectrum contribution, 
			alpha, f;

		Vector3 normal, 
			wOut, wIn;

		LowDiscrepancySampler positionSampler,
			directionSampler, continueSampler;

		Vector2 positionSample, 
			directionSample,
			sample;

		const int lightCount = m_pScene->LightList.Size();

		int intersections, index = 0;

		float continueProbability, pdf, 
			lightPdf = 1.f / lightCount;

		// Trace either a maximum number of paths or virtual point lights
		for (int lightIndex = 0, nPathIndex = p_nMaxPaths; nPathIndex > 0 && p_photonEmitterList.size() < p_nMaxEmitters; --nPathIndex)
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
			for (intersections = 1; !alpha.IsBlack() && m_pScene->Intersects(lightRay, intersection); ++intersections)
			{
				// sample = m_pScene->GetSampler()->Get2DSample();
				sample = directionSampler.Get2DSample();

				wOut = -lightRay.Direction;
				pMaterial = intersection.GetMaterial();

				// Set point light parameters
				emitter.Position = intersection.Surface.PointWS;
				emitter.Normal = intersection.Surface.ShadingBasisWS.W;
				emitter.Contribution = alpha * pMaterial->Rho(wOut, intersection.Surface) * Maths::InvPi;

				// std::cout << "Le = " << emitter.Contribution.ToString() << std::endl;

				// Push point light on list
				p_photonEmitterList.push_back(emitter);

				// Sample new direction
				f = pMaterial->SampleF(intersection.Surface, wOut, wIn, sample.U, sample.V, &pdf, bxdfType);
			
				// If reflectivity or pdf are zero, end path
				if (f.IsBlack() || pdf == 0.0f || intersections > m_nRayDepth)
					break;

				// Compute contribution of path
				contribution = alpha * f * Vector3::AbsDot(wIn, intersection.Surface.ShadingBasisWS.W) / pdf;

				// Possibly terminate virtual light path with Russian roulette
				continueProbability = Maths::Min(1.f, (contribution[0] + contribution[1] + contribution[2]) * 0.33f);
				if (continueSampler.Get1DSample() > continueProbability)
						break;

				// Modify contribution accordingly
				// alpha *= contribution / continueProbability;
				alpha = contribution / continueProbability;

				// Set new ray position and direction
				lightRay.Set(intersection.Surface.PointWS, wIn, m_fReflectEpsilon, Maths::Maximum);
			}

			// Increment light index, and reset if we traversed all scene lights
			if (++lightIndex == m_pScene->LightList.Size())
				lightIndex = 0;
		}

		// Just in case we traced more than is required
		if (p_photonEmitterList.size() > p_nMaxEmitters)
			p_photonEmitterList.erase(p_photonEmitterList.begin() + p_nMaxEmitters, p_photonEmitterList.end());
	}

protected:
	Spectrum Shade(Vector3 &p_position, Vector3 &p_normal)
	{
		Intersection isect;
		Vector3 direction;
		Vector2 sample2D;
		Ray ray;

		Spectrum E = 0;
	
		OrthonormalBasis basis; 
		basis.InitFromW(p_normal);
	
		// Cache this - it doesn't change!
		int mn = m_nAzimuthStrata * m_nAltitudeStrata;

		for (int altitudeIndex = 0; altitudeIndex < m_nAltitudeStrata; altitudeIndex++)
		{
			for (int azimuthIndex = 0; azimuthIndex < m_nAzimuthStrata; azimuthIndex++)
			{
				sample2D = m_pScene->GetSampler()->Get2DSample();

				direction = Montecarlo::CosineSampleHemisphere(sample2D.X, sample2D.Y, (float)altitudeIndex, (float)azimuthIndex, (float)m_nAltitudeStrata, (float)m_nAzimuthStrata); 
				direction = basis.Project(direction);

				ray.Set(p_position + direction * m_fReflectEpsilon, direction, m_fReflectEpsilon, Maths::Maximum);

				E += PathLi(ray);
			}
		}

		return E / mn;
	}

	Spectrum PathGather(Vector3 &p_position, Vector3 &p_normal, std::vector<PhotonEmitter> &p_emitterList, float p_fGTermMax)
	{
		VisibilityQuery emitterQuery(m_pScene);
		Spectrum Le(0), f;
		Vector3 wIn;
		int samplesUsed = 1;
		float indirectScale = 1.f;

		for (auto emitter : p_emitterList)
		{
			float d2 = Vector3::DistanceSquared(p_position, emitter.Position);

			// Smoothstep
			// TODO: Add smoothstep function
			float distScale = 1.f;
			// Smoothstep

			wIn = Vector3::Normalize(emitter.Position - p_position);

			f = distScale * Maths::InvPiTwo;

			float G = Vector3::AbsDot(wIn, p_normal) * Vector3::AbsDot(wIn, emitter.Normal) / d2;

			Spectrum Llight = (f * emitter.Contribution / (float)(p_emitterList.size())) * indirectScale * G;

			emitterQuery.SetSegment(p_position, m_fReflectEpsilon, emitter.Position, m_fReflectEpsilon);
			if (!emitterQuery.IsOccluded())
				Le += Llight;
				
			/*
			if (wIn.Dot(emitter.Normal) > 0.f)
			{
				emitterQuery.SetSegment(p_position, m_fReflectEpsilon, emitter.Position, m_fReflectEpsilon);
				
				if (emitterQuery.IsOccluded())
					continue;

				float d2 = 1.f / Vector3::DistanceSquared(p_position, emitter.Position);

				const float G = 
					Maths::Min(
					Vector3::Dot(emitter.Normal, wIn) *
					Vector3::Dot(p_normal, -wIn) * d2,
					p_fGTermMax);

				// Le += emitter.Contribution * G * Maths::InvPi;
				
				//const float G = Maths::Min(
				//		Vector3::Dot(emitter.Normal, wIn) * 
				//		Vector3::Dot(p_normal, wIn) * d2,
				//		p_fGTermMax);

				Le += emitter.Contribution * G * Maths::InvPi;
				
				samplesUsed++;
			}
			*/
		}

		return Le;
		// return Le / samplesUsed; //(float)p_emitterList.size();
	}

	Spectrum PathLi(Ray &p_ray)
	{
		IMaterial *pMaterial = NULL;
		bool specularBounce = false;
	
		Spectrum L(0.f),
			pathThroughput = 1.;
	
		Ray ray(p_ray); p_ray.Max = 1.f;
	
		BxDF::Type bxdfType;
	
		Vector3 wIn, wOut, 
			wInLocal, wOutLocal; 

		float pdf;

		// Trace
		for (int pathLength = 0; ; ++pathLength) 
		{
			// Find next vertex of path
			Intersection isect;
		
			if (!m_pScene->Intersects(ray, isect))
				break;

			// Get material
			if (!isect.HasMaterial()) 
				break;
		
			pMaterial = isect.GetMaterial();

			// Set distance if first bounce
			if (pathLength == 0)
				p_ray.Max = isect.Surface.Distance;

			wOut = -ray.Direction;

			// Possibly add emitted light at path vertex
			if (specularBounce)
			{
				if (isect.IsEmissive())
				{
					L += pathThroughput * 
						isect.GetLight()->Radiance(
							isect.Surface.PointWS, 
							isect.Surface.GeometryBasisWS.W, 
							wOut);
				}
			}
	
			// Sample illumination from lights to find path contribution
			L += pathThroughput * IIntegrator::SampleAllLights(m_pScene, isect, 
				isect.Surface.PointWS, isect.Surface.ShadingBasisWS.W, wOut, 
				m_pScene->GetSampler(), isect.GetLight(), m_nShadowRayCount);

			if (pathLength + 1 == m_nRayDepth) break;

			// Sample bsdf for next direction
			Vector2 sample = m_pScene->GetSampler()->Get2DSample();

			// Convert to surface coordinate system where (0,0,1) represents surface normal
			BSDF::WorldToSurface(isect.WorldTransform, isect.Surface, wOut, wOutLocal);

			// Sample new direction in wIn (remember we're tracing backwards)
			Spectrum f = pMaterial->SampleF(isect.Surface, wOutLocal, wInLocal, sample.U, sample.V, &pdf, BxDF::All_Combined, &bxdfType);

			// If the reflectivity or pdf are zero, terminate path
			if (f.IsBlack() || pdf == 0.0f) break;

			// Record if bounce is a specular bounce
			specularBounce = ((int)(bxdfType & BxDF::Specular)) != 0;

			// Convert back to world coordinates
			BSDF::SurfaceToWorld(isect.WorldTransform, isect.Surface, wInLocal, wIn);
		
			// Adjust path for new bounce
			// ray.Set(isect.Surface.PointWS + wIn * m_fReflectEpsilon, wIn, m_fReflectEpsilon, Maths::Maximum);
			ray.Set(isect.Surface.PointWS, wIn, m_fReflectEpsilon, Maths::Maximum);
		
			// Update path contribution at current stage
			pathThroughput *= f * Vector3::AbsDot(wIn, isect.Surface.ShadingBasisWS.W) / pdf;

			// Use Russian roulette to possibly terminate path
			if (pathLength > 2)
			{
				float continueProbability = Maths::Min(0.5f, 0.33f * (pathThroughput[0] + pathThroughput[1] + pathThroughput[2]));

				if (m_pScene->GetSampler()->Get1DSample() > continueProbability)
					break;

				pathThroughput /= continueProbability;
			}
		}

		return L * Maths::InvPi;
	}

public:
	void Initialise(Scene *p_pScene, float p_fReflectEpsilon, int p_nRayDepth, int p_nShadowRayCount, int p_nAzimuthStrata, int p_nAltitudeStrata)
	{
		m_pScene = p_pScene;

		m_fReflectEpsilon = p_fReflectEpsilon;

		m_nRayDepth = p_nRayDepth;
		m_nShadowRayCount = p_nShadowRayCount;
		m_nAltitudeStrata = p_nAltitudeStrata;
		m_nAzimuthStrata = p_nAzimuthStrata;
	}

	void Shade(std::vector<Dart*> &p_pointList, std::vector<PhotonEmitter> &p_emitterList, float p_fGTermMax)
	{
		std::cout << "Shading point using PathGather..." << std::endl;

		double total = Platform::GetTime();

		for (auto point : p_pointList)
		{
			double timestart = Platform::GetTime();
			point->Irradiance = PathGather(point->Position, point->Normal, p_emitterList, p_fGTermMax);
			std::cout << "Shaded in [" << Platform::ToSeconds(Platform::GetTime() - timestart) << "]" << std::endl;
		}

		std::cout << "Shaded [" << p_pointList.size() << "] in [" << Platform::ToSeconds(Platform::GetTime() - total) << "]" << std::endl;
	}

	void Shade(std::vector<Dart*> &p_pointList)
	{
		std::cout << "Shading point using PathLi..." << std::endl;

		double total = Platform::GetTime();

		for (auto point : p_pointList)
		{
			double timestart = Platform::GetTime();
			point->Irradiance = Shade(point->Position, point->Normal);
			std::cout << "Shaded in [" << Platform::ToSeconds(Platform::GetTime() - timestart) << "]" << std::endl;
		}

		std::cout << "Shaded [" << p_pointList.size() << "] in [" << Platform::ToSeconds(Platform::GetTime() - total) << "]" << std::endl;
	}
};