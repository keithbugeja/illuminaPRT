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
	PerspectiveCamera camera(
		Vector3(-20.0, 10.0, -20.0), Vector3(1.0f, -0.5f, 1.0f), Vector3::UnitYPos,
		-2.0f, 2.0f, -2.0f, 2.0f, 2.0f);

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

	/*
	#if defined(__PLATFORM_WINDOWS__)
		ITexture* pTexture1 = engineKernel.GetTextureManager()->CreateInstance("Image", "Image Texture 1", "Name=Image_Texture;Filename=D:\\Development\\IlluminaPRT\\Resource\\Texture\\texture.ppm;Filetype=PPM;");
	#elif defined(__PLATFORM_LINUX__)
		ITexture* pTexture1 = engineKernel.GetTextureManager()->CreateInstance("Image", "Image Texture 1", "Name=Image_Texture;Filename=../../../Resource/Texture/texture.ppm;Filetype=PPM;");
	#endif

	ITexture* pTexture2 = engineKernel.GetTextureManager()->CreateInstance("Marble", "Marble Texture 1", "Name=Marble_Texture;Stripes=0.002;Scale=1.0;Octaves=4;");
	*/

	//----------------------------------------------------------------------------------------------
	// Materials
	//----------------------------------------------------------------------------------------------
	engineKernel.GetMaterialManager()->RegisterFactory("Matte", new MatteMaterialFactory());
	engineKernel.GetMaterialManager()->RegisterFactory("Mirror", new MirrorMaterialFactory());
	engineKernel.GetMaterialManager()->RegisterFactory("Group", new MaterialGroupFactory());

	/*
	IMaterial* pMaterial1 = engineKernel.GetMaterialManager()->CreateInstance("Matte", "Diffuse Material 1", "Name=Diffuse_Material;Reflectivity=0.75,0.75,0.5;");
	IMaterial* pMaterial2 = engineKernel.GetMaterialManager()->CreateInstance("Mirror", "Phong Material 1", "Name=Phong_Material;Reflectivity=0.75,0.75,0.5;Exponent=32;");
	MaterialGroup* pMaterialGroup = (MaterialGroup*)engineKernel.GetMaterialManager()->CreateInstance("Group", "Material Group 1", "Name=Group1");
	pMaterialGroup->Add(pMaterial1, 0);
	pMaterialGroup->Add(pMaterial2, 1);
	
	//PhongMaterial material_mesh1(Spectrum(0.75,0.75,0.30), 32);
	*/

	MatteMaterial material_mesh2(Spectrum(0.75,0.75,0.30));

	//----------------------------------------------------------------------------------------------
	// Setup scene objects
	//----------------------------------------------------------------------------------------------
	// Initialising scene objects
	if (p_bVerbose)
		std::cout << "Initialising scene objects..." << std::endl;

	// Load Model
	#if defined(__PLATFORM_WINDOWS__)
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\Tests\\testAxes.obj");
		std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\Sibenik\\sibenik.obj");
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\Sponza\\original\\sponza.obj");
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\Sponza\\sponza_clean.obj");
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\Sponza\\Crytek\\sponza.obj");
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\Kalabsha\\Kalabsha12.obj");
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\Cornell\\cornellbox.obj");
		//std::string fname_model01("D:\\Development\\IlluminaPRT\\Resource\\Model\\Bunny\\bunny.obj");
	#elif defined(__PLATFORM_LINUX__)
		//std::string fname_model01("../../../Resource/Model/sibenik3.obj");
		std::string fname_model01("../../../Resource/Model/sponza3.obj");
		//std::string fname_model01("../../../Resource/Model/conference3.obj");
		//std::string fname_model01("../../../Resource/Model/david.obj");
		//std::string fname_model01("../../../Resource/Model/box.obj");
		//std::string fname_model01("../../../Resource/Model/bunny2.obj");
		//std::string fname_model01("../../../Resource/Model/ducky2.obj");
		//std::string fname_model01("../../../Resource/Model/venusm.obj");
		//std::string fname_model01("../../../Resource/Model/torus.obj");
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
	//	fname_model01, engineKernel.GetMaterialManager(), &pMeshMaterialGroup );
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
	Sphere shape_mesh2(Vector3(0.0, 16.5f, 0.0), 0.5f);
	DiffuseAreaLight diffuseLight2(NULL, &shape_mesh2, Spectrum(1e+2, 1e+2, 1e+2));
	
	// crytek sponza
	//Sphere shape_mesh2(Vector3(0.0, 1700.0f, 0.0), 100.0f);
	//DiffuseAreaLight diffuseLight2(NULL, &shape_mesh2, Spectrum(1e+7, 1e+7, 1e+7));
	//DiffuseAreaLight diffuseLight2(NULL, &shape_mesh2, Spectrum(1e+7, 1e+7, 1e+7));

	// Cornell Box
	//Sphere shape_mesh2(Vector3(0, 30.0f, 0), 2.0f);
	//DiffuseAreaLight diffuseLight2(NULL, &shape_mesh2, Spectrum(1e+4, 1e+4, 1e+4));
	//DiffuseAreaLight diffuseLight2(NULL, &shape_mesh2, Spectrum(1000, 1000, 1000));

	// box sky
	boost::shared_ptr<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh3 =
		ShapeFactory::CreateBox<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(Vector3(-5, 14.5, -5), Vector3(5, 15, 5));
	DiffuseAreaLight diffuseLight1(NULL, (IShape*)shape_mesh3.get(), Spectrum(40000, 40000, 40000));

	//----------------------------------------------------------------------------------------------
	// Compute bounding volumes
	//----------------------------------------------------------------------------------------------
	if (p_bVerbose)
		std::cout << "Computing bounding volumes..." << std::endl;
	
	shape_mesh1->ComputeBoundingVolume();
	shape_mesh2.ComputeBoundingVolume();
	shape_mesh3->ComputeBoundingVolume();
	//std::cout << std::endl;

	//----------------------------------------------------------------------------------------------
	// Compute normals
	//----------------------------------------------------------------------------------------------
	if (p_bVerbose)
		std::cout << "Computing mesh normals..." << std::endl;
	
	shape_mesh1->UpdateNormals();
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
	shape_mesh3->Compile();
	
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

	EmissivePrimitive pmv_mesh2;
	pmv_mesh2.SetShape(&shape_mesh2);
	//pmv_mesh2.SetShape((IShape*)shape_mesh3.get());
	pmv_mesh2.SetMaterial((IMaterial*)&material_mesh2);
	//pmv_mesh2.SetLight(&diffuseLight1);
	pmv_mesh2.SetLight(&diffuseLight2);
	//pmv_mesh2.WorldTransform.SetTranslation(Vector3(0.0f, 0.0f, 0.0f));
	basicSpace.PrimitiveList.PushBack(&pmv_mesh2);

	// Prepare space
	basicSpace.Initialise();
	basicSpace.Build();

	//----------------------------------------------------------------------------------------------
	// Initialise sampler and filter
	//----------------------------------------------------------------------------------------------
	//JitterSampler sampler;
	MultijitterSampler sampler;
	TentFilter filter;

	//----------------------------------------------------------------------------------------------
	// Scene creation complete
	//----------------------------------------------------------------------------------------------
	//PointLight pointLight(Vector3(0, 5, 0), RGBSpectrum(1000,1000,1000));
	//PointLight pointLight(Vector3(0, 7, 0), RGBSpectrum(10000,10000,10000));
	PointLight pointLight(Vector3(0, 7.5, 0), RGBSpectrum(100,100,100));
 
	Scene scene(&basicSpace, &sampler);
	//scene.LightList.PushBack(&pointLight);
	//scene.LightList.PushBack(&diffuseLight1);
	scene.LightList.PushBack(&diffuseLight2);
 
	//PathIntegrator integrator(4, 16, 1, false);
	//PathIntegrator integrator(4, 4, false);
	PathIntegrator integrator(1, 1, false);
	integrator.Initialise(&scene, &camera);
 
	ImagePPM imagePPM;
	//int width = 64, height = 64;
	//int width = 256, height = 256;
	//int width = 512, height = 512;
	//int width = 640, height = 480;
	int width = 1920, height = 1080;

	#if defined(__PLATFORM_WINDOWS__)
		ImageDevice device(width, height, &imagePPM, "D:\\Development\\IlluminaPRT\\Resource\\Output\\result.ppm");
	#elif defined(__PLATFORM_LINUX__)
		ImageDevice device(width, height, &imagePPM, "../../../Resource/Texture/result.ppm");
	#endif

	//BasicRenderer renderer(&scene, &camera, &integrator, &device, &filter, 1);
	DistributedRenderer renderer(&scene, &camera, &integrator, &device, &filter, 1, 32, 24);
	renderer.Initialise();
	
	if (p_bVerbose)
		std::cout << "Scene creation completed." << std::endl;

	//char cKey; std::cin >> cKey;

	boost::timer renderTimer;

	double alpha = 0.0f,
		totalFPS = 0.0f,
		// Sponza
		//cDistX = -10, cDistY = 17.5, cDistZ = -3;
		//cDistX = -10, cDistY = 12.5, cDistZ = -3;
		//cDistX = 10, cDistY = 7.5, cDistZ = -3;
		//cDistX = -5, cDistY = 1.0, cDistZ = -3;
		//cDistX = -20, cDistY = 30.5, cDistZ = -20;

		// Sibenik
		cDistX = 20, cDistY = 7.5, cDistZ = -3;

		// Crytek sponza
		//cDistX = -1000, cDistY = 750.0, cDistZ = -400;
		//cDistX = -400, cDistY = 100.0, cDistZ = -400;
		
		// Cornell box
		//cDistX = -30, cDistY = 30, cDistZ = -10;

		//cDistX = -10, cDistY = 5, cDistZ = -10;
		//cDistX = 10, cDistY = -10, cDistZ = 5;

	//Vector3 lookat(0, -10, 0);
	
	// Sponza
	//Vector3 lookat(0, 5, 0);
	Vector3 lookat(0, 0, 0);

	// Cornell box
	//Vector3 lookat(0, 14, 0);

	for (int iteration = 0; iteration < 8; iteration++)
	{
		renderTimer.restart();
		alpha += 0.1f;
	 
		camera.MoveTo(Vector3(Maths::Cos(alpha) * cDistX, cDistY, Maths::Sin(alpha) * cDistZ));
		camera.LookAt(lookat);
	 
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

	exit(0);
}
