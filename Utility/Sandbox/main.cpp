#include <vector>
#include <iostream>
#include <omp.h>

//#include "tbb/parallel_for.h"
//#include "tbb/blocked_range.h"

#include "boost/timer.hpp"

#include "System/Platform.h"
#include "System/EngineKernel.h"
#include "System/Dummy.h"

#include "Geometry/Vector2.h"
#include "Geometry/Vector3.h"
#include "Geometry/Matrix3x3.h"
#include "Geometry/Matrix4x4.h"
#include "Geometry/Transform.h"
#include "Geometry/Ray.h"

#include "Sampler/Sampler.h"
#include "Sampler/RandomSampler.h"
#include "Sampler/JitterSampler.h"

#include "Image/ImagePPM.h"
#include "Image/Image.h"
#include "Image/RGBPixel.h"

#include "Texture/Texture.h"
#include "Texture/MarbleTexture.h"
#include "Texture/ImageTexture.h"
#include "Texture/NoiseTexture.h"

#include "Camera/Camera.h"
#include "Camera/PerspectiveCamera.h"
#include "Camera/ThinLensCamera.h"

#include "Shape/ShapeFactory.h"
#include "Shape/TriangleMesh.h"
#include "Shape/BasicMesh.h"
#include "Shape/VertexFormats.h"
#include "Shape/IndexedTriangle.h"
#include "Shape/Triangle.h"
#include "Shape/Sphere.h"
#include "Shape/BVHMesh.h"
#include "Shape/KDTreeMesh.h"
#include "Shape/BIHMesh.h"
#include "Shape/HybridMesh.h"

#include "Space/BasicSpace.h"
#include "Space/BVHSpace.h"

#include "Material/Material.h"
#include "Material/Matte.h"
#include "Material/Mirror.h"
#include "Material/Glass.h"
#include "Material/MaterialGroup.h"
#include "Material/MaterialManager.h"

#include "Staging/Scene.h"
#include "Staging/GeometricPrimitive.h"
#include "Staging/EmissivePrimitive.h"

#include "Threading/Atomic.h"
#include "Threading/AtomicReference.h"
#include "Threading/LinkedList.h"
#include "Threading/Spinlock.h"
#include "Threading/Monitor.h"
#include "Threading/List.h"
#include "Threading/Queue.h"

#include "Light/PointLight.h"
#include "Light/DiffuseAreaLight.h"

#include "Integrator/PathIntegrator.h"
#include "Integrator/WhittedIntegrator.h"

#include "Renderer/BasicRenderer.h"
#include "Renderer/DistributedRenderer.h"

#include "Device/ImageDevice.h"

#include "Filter/Filter.h"
#include "Filter/BoxFilter.h"
#include "Filter/TentFilter.h"

#include "Object/Object.h"

#include "Distributed/Tile.h"

using namespace std;
using namespace Illumina::Core;

// TODO:
// DistributedRenderer should not instantiate MPI - change it to have it passed to the object
// Scene should provide more than one kind of sampler

void RayTracer(int p_nOMPThreads, bool p_bVerbose = true)
{
	//----------------------------------------------------------------------------------------------
	// Set number of OMP Threads
	//----------------------------------------------------------------------------------------------
	//std::cout << "Initialising OMP thread count : [Threads = " << p_nOMPThreads << "]" << std::endl;
	//omp_set_num_threads(p_nOMPThreads);

	//----------------------------------------------------------------------------------------------
	// Setup camera
	//----------------------------------------------------------------------------------------------
	//PerspectiveCamera camera(
		ThinLensCamera camera(
		Vector3(-20.0, 10.0, -20.0), Vector3(1.0f, -0.5f, 1.0f), Vector3::UnitYPos, 
		0.0f, -1.3f, 1.3f, -1.f, 1.f, 1.0f);

	if (p_bVerbose)
		std::cout << "Setting up camera : [" << camera.ToString() << "]" << std::endl;

	//----------------------------------------------------------------------------------------------
	// Setup textures
	//----------------------------------------------------------------------------------------------

	/*
	// Create marble texture
	std::cout << "Setting up textures..." << std::endl;
	MarbleTexture marbleTexture(0.002f, 1.0f, 4);

	// Create image texture
	ImagePPM ppmLoader;

	#if defined(__PLATFORM_WINDOWS__)
		boost::shared_ptr<ITexture> imgTexture(new ImageTexture("D:\\Development\\IlluminaPRT\\Resource\\Texture\\texture.ppm", ppmLoader));
	#elif defined(__PLATFORM_LINUX__)
		boost::shared_ptr<ITexture> imgTexture(new ImageTexture("../../../Resource/Texture/texture.ppm", ppmLoader));
	#endif
	*/

	//----------------------------------------------------------------------------------------------
	// Engine Kernel
	//----------------------------------------------------------------------------------------------
	EngineKernel engineKernel;

	//----------------------------------------------------------------------------------------------
	// Textures
	//----------------------------------------------------------------------------------------------
	engineKernel.GetTextureManager()->RegisterFactory("Image", new ImageTextureFactory());
	engineKernel.GetTextureManager()->RegisterFactory("Noise", new NoiseTextureFactory());
	engineKernel.GetTextureManager()->RegisterFactory("Marble", new MarbleTextureFactory());

	//----------------------------------------------------------------------------------------------
	// Materials
	//----------------------------------------------------------------------------------------------
	engineKernel.GetMaterialManager()->RegisterFactory("Matte", new MatteMaterialFactory());
	engineKernel.GetMaterialManager()->RegisterFactory("Mirror", new MirrorMaterialFactory());
	engineKernel.GetMaterialManager()->RegisterFactory("Glass", new GlassMaterialFactory());
	engineKernel.GetMaterialManager()->RegisterFactory("Group", new MaterialGroupFactory());

	//----------------------------------------------------------------------------------------------
	// Setup scene objects
	//----------------------------------------------------------------------------------------------
	// Initialising scene objects
	if (p_bVerbose)
		std::cout << "Initialising scene objects..." << std::endl;

	// Load Model
	#if defined(__PLATFORM_WINDOWS__)
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\tests\\test_axes.obj");
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\sibenik\\sibenik.obj");
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\sponza\\original\\sponza.obj");
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\sponza\\clean\\sponza_clean.obj");
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\sponza\\crytek\\sponza.obj");
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\kalabsha\\kalabsha12.obj");
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\cornell\\cornellbox.obj");
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\cornell\\cornell.obj");
		std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\cornell\\cornell_empty.obj");
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\cornell\\cornell_glass.obj");
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\cornell\\cornellsymmetric.obj");
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\bunny\\bunny.obj");
	#elif defined(__PLATFORM_LINUX__)
		//std::string fname_model01("../../../Resource/Model/tests/testAxes.obj");
		//std::string fname_model01("../../../Resource/Model/sibenik/sibenik.obj");
		//std::string fname_model01("../../../Resource/Model/sponza/original/sponza.obj");
		//std::string fname_model01("../../../Resource/Model/sponza/clean/sponza_clean.obj");
		//std::string fname_model01("../../../Resource/Model/sponza/crytek/sponza.obj");
		//std::string fname_model01("../../../Resource/Model/kalabsha/kalabsha12.obj");
		//std::string fname_model01("../../../Resource/Model/cornell/cornellbox.obj");
		//std::string fname_model01("../../../Resource/Model/cornell/cornell.obj");
		//std::string fname_model01("../../../Resource/Model/cornell/cornell_empty.obj");
		std::string fname_model01("../../../Resource/Model/cornell/cornell_glass.obj");
		//std::string fname_model01("../../../Resource/Model/cornell/cornellsymmetric.obj");
		//std::string fname_model01("../../../Resource/Model/bunny/bunny.obj");
	#endif

	if (p_bVerbose)
		std::cout << "-- Load object : [" << fname_model01 << "]" << std::endl;

	MaterialGroup *pMeshMaterialGroup = NULL, 
		*pMeshMaterialGroup2 = NULL;

	//boost::shared_ptr<BasicMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh1 =
	//	ShapeFactory::LoadMesh<BasicMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(
	//	fname_model01, engineKernel.GetMaterialManager(), &pMeshMaterialGroup );
	boost::shared_ptr<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh1 =
		ShapeFactory::LoadMesh<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(
		fname_model01, &engineKernel, &pMeshMaterialGroup );
	//boost::shared_ptr<BVHMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh1 =
	//	ShapeFactory::LoadMesh<BVHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(
	//	fname_model01, &engineKernel, &pMeshMaterialGroup );
	//boost::shared_ptr<BIHMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh1 =
	//	ShapeFactory::LoadMesh<BIHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(
	//	fname_model01, engineKernel.GetMaterialManager(), &pMeshMaterialGroup );
	//boost::shared_ptr<PBIHMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh1 =
	//	ShapeFactory::LoadMesh<PBIHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(
	//	fname_model01, engineKernel.GetMaterialManager(), &pMeshMaterialGroup );
	//boost::shared_ptr<GridMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh1 =
	//	ShapeFactory::LoadMesh<GridMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(
	//  fname_model01, engineKernel.GetMaterialManager(), &pMeshMaterialGroup );

	//std::cout << "Save object : [sibenik.obj]" << std::endl;
	//ShapeFactory::SaveMesh<BVHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>("D:\\Assets\\object_out.obj", shape_mesh1);

	// sphere arealight
	// Sponza, et al.
	//Sphere shape_mesh2(Vector3(0, 7.0f, 0), 2.0f);
	//Sphere shape_mesh2(Vector3(0.0, 15.0f, 0.0), 0.5f);
	//Sphere shape_mesh2(Vector3(0.0, 16.5f, 0.0), 0.5f);
	//DiffuseAreaLight diffuseLight2(NULL, &shape_mesh2, Spectrum(1e+2, 1e+2, 1e+2));
	//DiffuseAreaLight diffuseLight2(NULL, &shape_mesh2, Spectrum(1e+3, 1e+3, 1e+3));
	
	// crytek sponza
	//Sphere shape_mesh2(Vector3(0.0, 1700.0f, 0.0), 100.0f);
	//DiffuseAreaLight diffuseLight2(NULL, &shape_mesh2, Spectrum(1e+7, 1e+7, 1e+7));
	//DiffuseAreaLight diffuseLight2(NULL, &shape_mesh2, Spectrum(1e+7, 1e+7, 1e+7));

	// Cornell Box
	//Sphere shape_mesh2(Vector3(0, 30.0f, 0), 2.0f);
	//DiffuseAreaLight diffuseLight2(NULL, &shape_mesh2, Spectrum(1e+4, 1e+4, 1e+4));
	//DiffuseAreaLight diffuseLight2(NULL, &shape_mesh2, Spectrum(1000, 1000, 1000));
	//Sphere shape_mesh2(Vector3(0.0, 30.0f, 0.0), 5.0f);
	//DiffuseAreaLight diffuseLight2(NULL, &shape_mesh2, Spectrum(1e+3, 1e+3, 1e+3));

	//boost::shared_ptr<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>> shape_boxLight =
	//	ShapeFactory::CreateBox<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(Vector3(-4, 40 - 1E-2, -4), Vector3(4, 40, 4));
	//DiffuseAreaLight diffuseBoxLight(NULL, (IShape*)shape_boxLight.get(), Spectrum(1e+3, 1e+3, 1e+3));

	boost::shared_ptr<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>> shape_boxLight =
		ShapeFactory::CreateQuad<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>
		(Vector3(-6, 40 - 1E-4, -6), Vector3(6, 40 - 1E-4, -6), Vector3(-6, 40 - 1E-4, 6), Vector3(6, 40 - 1E-4, 6));
	DiffuseAreaLight diffuseBoxLight(NULL, (IShape*)shape_boxLight.get(), Spectrum(4.5e+2, 4.5e+2, 4.5e+2));

	// box sky
	//boost::shared_ptr<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>> shape_boxLight =
	//	ShapeFactory::CreateQuad<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>
	//	(Vector3(-6, 15 - 1E-4, -6), Vector3(6, 15 - 1E-4, -6), Vector3(-6, 15 - 1E-4, 6), Vector3(6, 15 - 1E-4, 6));
	//DiffuseAreaLight diffuseBoxLight(NULL, (IShape*)shape_boxLight.get(), Spectrum(1.5e+3, 1.5e+3, 1.5e+3));
	//boost::shared_ptr<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh3 =
	//	ShapeFactory::CreateBox<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(Vector3(-5, 14.5, -5), Vector3(5, 15, 5));
	//DiffuseAreaLight diffuseLight1(NULL, (IShape*)shape_mesh3.get(), Spectrum(40000, 40000, 40000));

	//----------------------------------------------------------------------------------------------
	// Compute bounding volumes
	//----------------------------------------------------------------------------------------------
	if (p_bVerbose)
		std::cout << "Computing bounding volumes..." << std::endl;
	
	shape_mesh1->ComputeBoundingVolume();
	//shape_mesh2.ComputeBoundingVolume();
	//shape_mesh3->ComputeBoundingVolume();
	shape_boxLight->ComputeBoundingVolume();
	//std::cout << std::endl;

	//----------------------------------------------------------------------------------------------
	// Compute normals
	//----------------------------------------------------------------------------------------------
	if (p_bVerbose)
		std::cout << "Computing mesh normals..." << std::endl;
	
	//shape_mesh1->UpdateNormals();
	//shape_mesh3->UpdateNormals();
	//std::cout << std::endl;

	//----------------------------------------------------------------------------------------------
	// Compile meshes
	//----------------------------------------------------------------------------------------------
	if (p_bVerbose)
		std::cout << "Compiling acceleration structure-based meshes..." << std::endl;

	boost::timer compileTimer;
	compileTimer.restart();
	shape_mesh1->Compile();
	//shape_mesh3->Compile();
	shape_boxLight->Compile();
	
	if (p_bVerbose)
		std::cout << "-- Model 01 : [" << fname_model01 << "] compiled in " << compileTimer.elapsed() << " seconds." << std::endl;

	//----------------------------------------------------------------------------------------------
	// Initialise scene space
	//----------------------------------------------------------------------------------------------
	if (p_bVerbose)
		std::cout << "Adding models to scene space..." << std::endl;

	BasicSpace basicSpace;

	GeometricPrimitive pmv_mesh1;
	pmv_mesh1.SetShape((IShape*)shape_mesh1.get());
	pmv_mesh1.SetMaterial(pMeshMaterialGroup);
	//pmv_mesh1.SetMaterial((IMaterial*)&material_mesh1);
	//pmv_mesh1.WorldTransform.SetScaling(Vector3(3.0f, 3.0f, 3.0f));
	//pmv_mesh1.WorldTransform.SetScaling(Vector3(5.0f, 5.0f, 5.0f));
	//pmv_mesh1.WorldTransform.SetTranslation(Vector3(0.0f, -10.0f, 0.0f));
	basicSpace.PrimitiveList.PushBack(&pmv_mesh1);

	//GeometricPrimitive pmv_mesh3;
	//pmv_mesh3.SetShape((IShape*)shape_mesh3.get());
	//pmv_mesh3.SetMaterial(pMaterial2);
	////pmv_mesh1.WorldTransform.SetScaling(Vector3(3.0f, 3.0f, 3.0f));
	////pmv_mesh1.WorldTransform.SetScaling(Vector3(5.0f, 5.0f, 5.0f));
	////pmv_mesh1.WorldTransform.SetTranslation(Vector3(0.0f, -10.0f, 0.0f));
	//basicSpace.PrimitiveList.PushBack(&pmv_mesh3);

	//EmissivePrimitive pmv_mesh2;
	//pmv_mesh2.SetShape(&shape_mesh2);
	////pmv_mesh2.SetShape((IShape*)shape_mesh3.get());
	//IMaterial *pLightMaterial = engineKernel.GetMaterialManager()->CreateInstance("Matte", "light", "Name=light;Reflectivity=0,0,0;");
	//pmv_mesh2.SetMaterial(pLightMaterial);
	////pmv_mesh2.SetLight(&diffuseLight1);
	//pmv_mesh2.SetLight(&diffuseLight2);
	////pmv_mesh2.WorldTransform.SetTranslation(Vector3(0.0f, 0.0f, 0.0f));
	//basicSpace.PrimitiveList.PushBack(&pmv_mesh2);

	//----------------------------------------------------------------------------------------------
	// Add spheres for cornell box tests
	//----------------------------------------------------------------------------------------------
	Sphere shape_cornell_metal_sphere(Vector3(12.0, 7.5f, -5.0), 7.5f);
	shape_cornell_metal_sphere.ComputeBoundingVolume();

	IMaterial *pMaterial_cornell_metal_sphere = engineKernel.GetMaterialManager()->CreateInstance("Mirror", "metalsphere", "Name=metalsphere;Reflectivity=0.9,0.9,0.9;Absorption=1.0;EtaI=1.0;EtaT=1.55;");
	GeometricPrimitive pmv_cornell_metal_sphere;
	pmv_cornell_metal_sphere.SetShape(&shape_cornell_metal_sphere);
	pmv_cornell_metal_sphere.SetMaterial(pMaterial_cornell_metal_sphere);
	basicSpace.PrimitiveList.PushBack(&pmv_cornell_metal_sphere);

	Sphere shape_cornell_glass_sphere(Vector3(-15.0, 20.0f, 2.0), 7.5f);
	//Sphere shape_cornell_glass_sphere(Vector3(10.0, 7.5f, 2.0), 7.5f);
	shape_cornell_glass_sphere.ComputeBoundingVolume();

	IMaterial *pMaterial_cornell_glass_sphere = engineKernel.GetMaterialManager()->CreateInstance("Glass", "glasssphere", "Name=glasssphere;Reflectivity=0.92,0.92,0.92;Absorption=1.0;EtaI=1.0;EtaT=1.55;");
	GeometricPrimitive pmv_cornell_glass_sphere;
	pmv_cornell_glass_sphere.SetShape(&shape_cornell_glass_sphere);
	pmv_cornell_glass_sphere.SetMaterial(pMaterial_cornell_glass_sphere);
	basicSpace.PrimitiveList.PushBack(&pmv_cornell_glass_sphere);

	EmissivePrimitive pmv_cornell_box_light;
	pmv_cornell_box_light.SetShape((IShape*)shape_boxLight.get());
	pmv_cornell_box_light.SetMaterial(engineKernel.GetMaterialManager()->CreateInstance("Matte", "boxlight", "Name=light;Reflectivity=0,0,0;"));
	pmv_cornell_box_light.SetLight(&diffuseBoxLight);
	basicSpace.PrimitiveList.PushBack(&pmv_cornell_box_light);

	// Prepare space
	basicSpace.Initialise();
	basicSpace.Build();

	//----------------------------------------------------------------------------------------------
	// Initialise sampler and filter
	//----------------------------------------------------------------------------------------------
	//JitterSampler sampler;
	RandomSampler sampler;
	BoxFilter filter;

	//----------------------------------------------------------------------------------------------
	// Scene creation complete
	//----------------------------------------------------------------------------------------------
	//PointLight pointLight(Vector3(0, 5, 0), RGBSpectrum(1000,1000,1000));
	//PointLight pointLight(Vector3(0, 7, 0), RGBSpectrum(10000,10000,10000));
	PointLight pointLight(Vector3(0, 7.5, 0), RGBSpectrum(100,100,100));
 
	Scene scene(&basicSpace, &sampler);
	//scene.LightList.PushBack(&pointLight);
	//scene.LightList.PushBack(&diffuseLight1);
	//scene.LightList.PushBack(&diffuseLight2);
	scene.LightList.PushBack(&diffuseBoxLight);
 
	//PathIntegrator integrator(4, 16, 1, false);
	//PathIntegrator integrator(4, 4, false);
	PathIntegrator integrator(6, 1, false);
	//WhittedIntegrator integrator(6, 1);
	integrator.Initialise(&scene, &camera);
 
	ImagePPM imagePPM;
	//int width = 64, height = 64;
	//int width = 256, height = 256;
	//int width = 512, height = 384;
	//int width = 512, height = 512;
	//int width = 640, height = 480;
	//int width = 800, height = 600;
	int width = 1024, height = 1024;
	//int width = 1280, height = 1024;
	//int width = 1920, height = 1080;
	//int width = 1920, height = 1200;

	#if defined(__PLATFORM_WINDOWS__)
		ImageDevice device(width, height, &imagePPM, "D:\\Development\\IlluminaPRT\\Resource\\Output\\result.ppm");
	#elif defined(__PLATFORM_LINUX__)
		ImageDevice device(width, height, &imagePPM, "../../../Resource/Output/result.ppm");
	#endif

	//BasicRenderer renderer(&scene, &camera, &integrator, &device, &filter, 4);
	DistributedRenderer renderer(&scene, &camera, &integrator, &device, &filter, 64, 8, 8);
	renderer.Initialise();
	
	if (p_bVerbose)
		std::cout << "Scene creation completed." << std::endl;

	//char cKey; std::cin >> cKey;

	boost::timer renderTimer;

	double alpha = Maths::Pi,
		totalFPS = 0.0f;

	// Sibenik
	//Vector3 lookFrom(20, 30, -20);
	//Vector3 lookFrom(10, 10, 0);
	//Vector3 lookAt(0, 5, 0);

	// Crytek sponza
	//Vector3 lookFrom(-1000, 750, -400);
	//Vector3 lookFrom(-400, 100, -400);
	//Vecotr3 lookAt(0, 25, 0);
		
	// Cornell box
	//cDistX = -30, cDistY = 30, cDistZ = -10;
	
	// Sponza
	//Vector3 lookFrom(10, 7.5, 0);
	//Vector3 lookAt(0, 0, 0);
	//Vector3 lookAt(0, 5, 0);

	// Cornell box
	//Vector3 lookFrom(20, 20, 40);
	camera.SetFieldOfView(50, 1.0f);

	Vector3 lookFrom(0, 20, 80);
	Vector3 lookAt(0, 20, 0);

	alpha = Maths::PiHalf;

	for (int iteration = 0; iteration < 4e+10; iteration++)
	{
		renderTimer.restart();
		alpha += 0.05f;
	 
		camera.MoveTo(lookFrom);
		camera.MoveTo(Vector3(Maths::Cos(alpha) * lookFrom.X, lookFrom.Y, Maths::Sin(alpha) * lookFrom.Z));
		camera.LookAt(lookAt);
	 
		// Here we rebuild the AS
		basicSpace.Update();
	 
		// Render
		renderer.Render();
	 
		totalFPS += (float)(1.0 / renderTimer.elapsed());
		
		if (p_bVerbose)
		{
			std::cout << shape_mesh1->ToString() << std::endl;
			std::cout << "Total Render Time : " << renderTimer.elapsed() << " seconds" << std::endl;
			std::cout << "FPS: [" << totalFPS / iteration <<" / " << iteration << "]" << std::endl;
		}
	}

	renderer.Shutdown();
	integrator.Shutdown();
}

int main()
{
	int nCores = Platform::GetProcessorCount();
	//std::cout << "Hardware cores : " << nCores << std::endl;

	//TestAtomic();
	//TestUID();
	//TestSpinLock();
	//TestLists();

	//SimpleTracer(4);
	//SimplePacketTracer(nCores);
	//TileBasedTracer(nCores);

	RayTracer(nCores / 2, false);

	//std::cout << "Complete in " << Platform::GetTime() << " seconds " << std::endl;

	//char cKey;
	//cin >> cKey;
	return 0;
}
