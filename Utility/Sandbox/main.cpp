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

#include "Device/ImageDevice.h"

#include "Filter/Filter.h"
#include "Filter/BoxFilter.h"
#include "Filter/TentFilter.h"

#include "Object/Object.h"

#include "Distributed/Tile.h"

using namespace std;
using namespace Illumina::Core;
// 
// void TestLists(void)
// {
// 	//omp_set_num_threads(8);
// 
// 	Object *a = new Object("Keith1");
// 	Object *b = new Object("Keith2");
// 
// 	Object *c = a;
// 	bool result = AtomicReference::CompareAndSet((void**)&c, b, b);
// 	std::cout<<result<<" : "<<c->ToString()<<std::endl;
// 
// 	Queue queue;
// 	Object *p = NULL;
// 
// 	//#pragma omp parallel for
// 	for (int j = 0; j < 100000; j++)
// 	{
// 		queue.Enqueue(a);
// 		queue.Enqueue(b);
// 		queue.Dequeue();
// 		p = (Object*)queue.Dequeue();
// 	}
// 
// 	std::cout<<p<<std::endl;
// 
// 	queue.Enqueue(a);
// 	queue.Enqueue(b);
// 
// 	Object *da = (Object*)queue.Dequeue();
// 	std::cout<<da->ToString() << std::endl;
// 
// 	Object *db = (Object*)queue.Dequeue();
// 	std::cout<<db->ToString() << std::endl;
// 
// 	Object *dc = (Object*)queue.Dequeue();
// 	std::cout<<"Last obj : " << dc << std::endl;
// 
// 	if (dc == NULL)
// 		std::cout<<"Queue is empty! Correct!"<< std::endl;
// 	else
// 		std::cout<<"Queue is not empty! Wrong!"<<std::endl;
// 
// 	std::cout<<"Instance count:"<<QueueNode::NodeInstanceCount<<std::endl;
// }
// 
// void TestAtomic(void)
// {
// 	Int32	a[2] = {1, 2},
// 			b[2] = {1, 2};
// 
// 	if (Atomic::DoubleWidthCompareAndSwap(a, 4, 5, b))
// 		std::cout<<"DCAS -> Success!"<<std::endl;
// 	else
// 		std::cout<<"DCAS -> Failed!"<<std::endl;
// 
// #if defined(__ARCHITECTURE_X64__)
// 	ALIGN_16 Int64  a64[2] = {3, 4},
// 					b64[2] = {3, 4};
// 
// 	if (Atomic::DoubleWidthCompareAndSwap(a64, 69, 58, b64))
// 		std::cout<<"DCAS -> Success!"<<std::endl;
// 	else
// 		std::cout<<"DCAS -> Failed!"<<std::endl;
// 
// 	AtomicStampedReference<void> s;
// 	s.Set((void*)0x1000, 0);
// 
// 	//#pragma omp parallel for
// 	for (int n = 0; n < 1000000; n++)
// 	{
// 		s.CompareAndSet((void*)0x1000, (void*)0x2000, 0, 1);
// 		s.CompareAndSet((void*)0x2000, (void*)0x3000, 1, 2);
// 		s.CompareAndSet((void*)0x3000, (void*)0x4000, 2, 3);
// 		s.CompareAndSet((void*)0x4000, (void*)0x1000, 3, 0);
// 	}
// 
// 	std::cout<<"Ref : [0x1000] " << s.GetReference() <<", [0] " << s.GetStamp() << std::endl;
// #endif
// }
// 
// void TestUID(void)
// {
// 	//omp_set_num_threads(8);
// 
// 	List<Object> objectList;
// 
// 	//#pragma omp parallel for
// 	for (int j = 0; j < 1000; j++)
// 	{
// 		Object t("test");
// 
// 		std::cout << t.ToString() << " Hash : " << t.GetHashCode() << std::endl;
// 		objectList.PushBack(t);
// 	}
// 
// 	std::cout<<objectList.Size()<<std::endl;
// 	std::cin.get();
// 
// 	exit(0);
// }
// 
// void TestSpinLock(void)
// {
// 	//omp_set_num_threads(Platform::GetProcessorCount());
// 
// 	Spinlock lock(100);
// 
// 	int sharedvar = 0;
// 	double start = Platform::GetTime();
// 
// 	//#pragma omp parallel for
// 	for (int j = 0; j < 100000; j++)
// 	{
// 		lock.Lock();
// 		sharedvar++;
// 		lock.Unlock();
// 	}
// 
// 	double end = Platform::GetTime();
// 
// 	std::cout << "Result : " << sharedvar << ", Time : " << end - start << std::endl;
// }
// 
// void RenderTile(Tile &p_tile, const ICamera &p_camera, const ISpace &p_space, Image &p_image)
// {
// // 	const int MaxDepth = 1;
// // 	Vector3 lightPosition(0, 5, 0);
// // 
// // 	int width = (int)p_tile.GetWidth(),
// // 		height = (int)p_tile.GetHeight();
// // 
// // 	std::string str = p_tile.ToString();
// // 
// // 	std::cout << str << std::endl;
// // 
// // 	for (int y = p_tile.GetRegionStart().Y, yMax = p_tile.GetRegionEnd().Y; y < yMax; ++y)
// // 	{
// // 		DifferentialSurface differentialSurface;
// // 
// // 		Vector3 lightVector,
// // 			reflectionVector;
// // 
// // 		float distance,
// // 			diffuse,
// // 			specular,
// // 			shadow,
// // 			contribution;
// // 
// // 		bool bHit;
// // 
// // 		for (int x = p_tile.GetRegionStart().X, xMax = p_tile.GetRegionEnd().X; x < xMax; ++x)
// // 		{
// // 			RGBPixel pixelColour = RGBPixel::Black,
// // 				finalColour = RGBPixel::Black;
// // 
// // 			Ray ray = p_camera.GetRay(
// // 				((float)x) / width,
// // 				((float)y) / height,
// // 				0.5f, 0.5f );
// // 
// // 			bHit = false;
// // 
// // 			contribution =
// // 				shadow = 1.0f;
// // 
// // 			for (int depth = 0; depth < MaxDepth; depth++)
// // 			{
// // 				bHit = p_space.Intersects(ray, 0, differentialSurface);
// // 
// // 				if (bHit)
// // 				{
// // 					//pixelColour = marbleTexture.GetValue(differentialSurface.PointUV, differentialSurface.PointWS * 20);
// // 					pixelColour.Set(pixelColour.R * 0.5f + 0.5f, pixelColour.G * 0.5f + 0.4f, pixelColour.B * 0.5f + 0.3f);
// // 					Vector3::Subtract(lightPosition, differentialSurface.PointWS, lightVector);
// // 					Vector3 normal(differentialSurface.BasisWS.U);
// // 					normal.Y = -normal.Y;
// // 
// // 					distance = lightVector.Length();
// // 					lightVector.Normalize();
// // 					diffuse = Maths::Clamp(Vector3::Dot(lightVector, normal), 0.3f, 1.0f);
// // 
// // 					Ray shadowRay(differentialSurface.PointWS, lightVector, 0.0001f, distance - 0.01f);
// // 					shadow = p_space.Intersects(shadowRay, 0.0f) ? 0.4f : 1.0f;
// // 
// // 					Vector3::Reflect(lightVector, normal, lightVector);
// // 					specular = Maths::Pow(Vector3::Dot(ray.Direction, lightVector), 64);
// // 
// // 					pixelColour *= shadow * (diffuse + specular) * contribution;
// // 					finalColour += pixelColour;
// // 					contribution *= 0.25f;
// // 
// // 					if (depth < MaxDepth - 1)
// // 					{
// // 						Vector3::Reflect(ray.Direction, normal, reflectionVector);
// // 						ray.Direction = reflectionVector;
// // 						ray.Origin = differentialSurface.PointWS + reflectionVector * 0.0001f;
// // 					}
// // 				}
// // 				else
// // 					break;
// // 			}
// // 
// // 			//p_image.Set(x, (height - 1) - y, finalColour);
// // 			p_image.Set(x, y, finalColour);
// // 		}
// // 	}
// }
// 
// void TileBasedTracer(int p_nOMPThreads)
// {
// 	//----------------------------------------------------------------------------------------------
// 	// Set number of OMP Threads
// 	//----------------------------------------------------------------------------------------------
// 	std::cout << "Initialising OMP thread count : [Threads = " << p_nOMPThreads << "]" << std::endl;
// 	omp_set_num_threads(p_nOMPThreads);
// 
// 	//----------------------------------------------------------------------------------------------
// 	// Setup camera
// 	//----------------------------------------------------------------------------------------------
// 	PerspectiveCamera camera(
// 		Vector3(-20.0, 10.0, -20.0), Vector3(1.0f, -0.5f, 1.0f), Vector3::UnitYPos,
// 		-2.0f, 2.0f, -2.0f, 2.0f, 2.0f);
// 
// 	std::cout << "Setting up camera : [" << camera.ToString() << "]" << std::endl;
// 
// 	//----------------------------------------------------------------------------------------------
// 	// Setup textures
// 	//----------------------------------------------------------------------------------------------
// 
// 	// Create marble texture
// 	std::cout << "Setting up textures..." << std::endl;
// 	MarbleTexture marbleTexture(0.002f, 1.0f, 4);
// 
// 	// Create image texture
// 	ImagePPM ppmLoader;
// 
// 	#if defined(__PLATFORM_WINDOWS__)
// 		boost::shared_ptr<ITexture> imgTexture(new ImageTexture("D:\\Media\\Assets\\IlluminaRT\\Textures\\texture.ppm", ppmLoader));
// 	#elif defined(__PLATFORM_LINUX__)
// 		boost::shared_ptr<ITexture> imgTexture(new ImageTexture("../../../Resource/Texture/texture.ppm", ppmLoader));
// 	#endif
// 
// 	//----------------------------------------------------------------------------------------------
// 	// Setup scene objects
// 	//----------------------------------------------------------------------------------------------
// 	// Initialising scene objects
// 	std::cout << "Initialising scene objects..." << std::endl;
// 
// 	// Load Model
// 	#if defined(__PLATFORM_WINDOWS__)
// 		//std::string fname_model01("D:\\Media\\Assets\\IlluminaRT\\Models\\sibenik3.obj");
// 		std::string fname_model01("D:\\Media\\Assets\\IlluminaRT\\Models\\sponza3.obj");
// 		//std::string fname_model01("D:\\Media\\Assets\\IlluminaRT\\Models\\conference3.obj");
// 		//std::string fname_model01("D:\\Media\\Assets\\IlluminaRT\\Models\\david.obj");
// 		//std::string fname_model01("D:\\Media\\Assets\\IlluminaRT\\Models\\box.obj");
// 		//std::string fname_model01("D:\\Media\\Assets\\IlluminaRT\\Models\\bunny2.obj");
// 		//std::string fname_model01("D:\\Media\\Assets\\IlluminaRT\\Models\\ducky2.obj");
// 		//std::string fname_model01("D:\\Media\\Assets\\IlluminaRT\\Models\\venusm.obj");
// 		//std::string fname_model01("D:\\Media\\Assets\\IlluminaRT\\Models\\torus.obj");
// 	#elif defined(__PLATFORM_LINUX__)
// 		//std::string fname_model01("../../../Resource/Model/sibenik3.obj");
// 		std::string fname_model01("../../../Resource/Model/sponza3.obj");
// 		//std::string fname_model01("../../../Resource/Model/conference3.obj");
// 		//std::string fname_model01("../../../Resource/Model/david.obj");
// 		//std::string fname_model01("../../../Resource/Model/box.obj");
// 		//std::string fname_model01("../../../Resource/Model/bunny2.obj");
// 		//std::string fname_model01("../../../Resource/Model/ducky2.obj");
// 		//std::string fname_model01("../../../Resource/Model/venusm.obj");
// 		//std::string fname_model01("../../../Resource/Model/torus.obj");
// 	#endif
// 
// 	std::cout << "-- Load object : [" << fname_model01 << "]" << std::endl;
// 
// 	//boost::shared_ptr<SimpleMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh1 =
// 	//	ShapeFactory::LoadMesh<SimpleMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model01);
// 	//boost::shared_ptr<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh1 =
// 	//	ShapeFactory::LoadMesh<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model01);
// 	//boost::shared_ptr<BVHMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh1 =
// 	//	ShapeFactory::LoadMesh<BVHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model01);
// 	//boost::shared_ptr<BIHMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh1 =
// 	//	ShapeFactory::LoadMesh<BIHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model01);
// 	boost::shared_ptr<PBIHMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh1 =
// 		ShapeFactory::LoadMesh<PBIHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model01);
// 	//boost::shared_ptr<GridMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh1 =
// 	//	ShapeFactory::LoadMesh<GridMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model01);
// 
// 	//std::cout << "Save object : [sibenik.obj]" << std::endl;
// 	//ShapeFactory::SaveMesh<BVHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>("D:\\Assets\\object_out.obj", shape_mesh1);
// 
// 	//----------------------------------------------------------------------------------------------
// 	// Compute bounding volumes
// 	//----------------------------------------------------------------------------------------------
// 	std::cout << "Computing bounding volumes..." << std::endl;
// 	shape_mesh1->ComputeBoundingVolume();
// 	std::cout << std::endl;
// 
// 	//----------------------------------------------------------------------------------------------
// 	// Compile meshes
// 	//----------------------------------------------------------------------------------------------
// 	std::cout << "Compiling acceleration structure-based meshes..." << std::endl;
// 
// 	boost::timer compileTimer;
// 	compileTimer.restart();
// 	shape_mesh1->Compile();
// 	std::cout << "-- Model 01 : [" << fname_model01 << "] compiled in " << compileTimer.elapsed() << " seconds." << std::endl;
// 
// 	//----------------------------------------------------------------------------------------------
// 	// Initialise scene space
// 	//----------------------------------------------------------------------------------------------
// 	std::cout << "Adding models to scene space..." << std::endl;
// 	BasicSpace basicSpace;
// 
// 	GeometricPrimitive pmv_mesh1;
// 	pmv_mesh1.SetShape((Shape*)shape_mesh1.get());
// 	pmv_mesh1.WorldTransform.SetScaling(Vector3(3.0f, 3.0f, 3.0f));
// 
// 	pmv_mesh1.WorldTransform.SetTranslation(Vector3(0.0f, -10.0f, 0.0f));
// 	basicSpace.PrimitiveList.PushBack(&pmv_mesh1);
// 
// 	// Prepare space
// 	basicSpace.Initialise();
// 	basicSpace.Build();
// 
// 	//----------------------------------------------------------------------------------------------
// 	// Scene creation complete
// 	//----------------------------------------------------------------------------------------------
// 	PointLight pointLight(Vector3(0,5,0), RGBSpectrum(1,1,1));
// 
// 	Scene scene(&basicSpace);
// 	scene.GetLightList().PushBack(&pointLight);
// 
// 	WhittedIntegrator integrator;
// 	integrator.Initialise(&scene, &camera);
// 
// 	ImagePPM imagePPM;
// 
// 	#if defined(__PLATFORM_WINDOWS__)
// 	ImageDevice device(640, 480, &imagePPM, "D:\\Media\\Assets\\IlluminaRT\\Textures\\result.ppm");
// 	#elif defined(__PLATFORM_LINUX__)
// 	ImageDevice device(640, 480, &imagePPM, "../../../Resource/Texture/result.ppm");
// 
// 	BasicRenderer basicRenderer(&scene, &camera, &Integrator, &device);
// 	
// 	char cKey;
// 	std::cout << "Scene creation completed." << std::endl;
// 	std::cin >> cKey;
// 
// 	boost::timer renderTimer;
// 	boost::progress_display renderProgress(height);
// 
// 	for (int iteration = 1; iteration < 10000; iteration++)
// 	{
// 		renderTimer.restart();
// 		alpha += 0.05f;
// 
// 		camera.MoveTo(Vector3(Maths::Cos(alpha) * -20, 10.0, Maths::Sin(alpha) * -20));
// 		camera.LookAt(Vector3::Zero);
// 
// 		// Here we rebuild the AS
// 		basicSpace.Update();
// 
// 		// Render
// 		basicRenderer.Render();
// 
// 		totalFPS += (float)(1.0 / renderTimer.elapsed());
// 		std::cout << shape_mesh1->ToString() << std::endl;
// 		std::cout << "Total Render Time : " << renderTimer.elapsed() << " seconds" << std::endl;
// 		std::cout << "FPS: [" << totalFPS / iteration <<" / " << iteration << "]" << std::endl;
// 	}
// 
// 	/*
// 	//----------------------------------------------------------------------------------------------
// 	// Setup rendering stage
// 	//----------------------------------------------------------------------------------------------
// 	const int MaxDepth = 1;
// 
// 	const int width = 320,
// 		height = 256;
// 
// 	// Create image
// 	Image canvas(width, height);
// 
// 	boost::timer renderTimer;
// 	boost::progress_display renderProgress(height);
// 
// 	Vector3 lightPosition(0, 5, 0);
// 	float alpha = 0, totalFPS = 0;
// 
// 	// Iterate an arbitrary number of times
// 	for (int iteration = 1; iteration < 10000; iteration++)
// 	{
// 		renderTimer.restart();
// 		alpha += 0.05f;
// 
// 		camera.MoveTo(Vector3(Maths::Cos(alpha) * -20, 10.0, Maths::Sin(alpha) * -20));
// 		camera.LookAt(Vector3::Zero);
// 
// 		// Here we rebuild the AS
// 		basicSpace.Update();
// 
// 		// Render the scene
// 		Image *c = (Image*)&canvas;
// 
// 		int sliceHeight = height / p_nOMPThreads;
// 
// 		std::cout << sliceHeight << std::endl;
// 
// 		#pragma omp parallel for schedule(guided)
// 		for (int core = 0; core < p_nOMPThreads; core++)
// 		{
// 			Tile tile(Vector2(0, sliceHeight * core), Vector2(width, sliceHeight * (core + 1)));
// 			//Tile tile(Vector2(0,0), Vector2(width, height));
// 			RenderTile(tile, camera, basicSpace, canvas);
// 		}
// 
// 		totalFPS += (float)(1.0 / renderTimer.elapsed());
// 		std::cout<< shape_mesh1->ToString() << std::endl;
// 		std::cout << "Total Render Time : " << renderTimer.elapsed() << " seconds" << std::endl;
// 		std::cout << "FPS: [" << totalFPS / iteration <<" / " << iteration << "]" << std::endl;
// 
// 		ImagePPM imagePPM;
// 		#if defined(__PLATFORM_WINDOWS__)
// 		imagePPM.Save(canvas, "D:\\Media\\Assets\\IlluminaRT\\Textures\\result.ppm");
// 		#elif defined(__PLATFORM_LINUX__)
// 		imagePPM.Save(canvas, "../../../Resource/Texture/result.ppm");
// 		#endif
// 	}
// 	*/
// }
// 
// //----------------------------------------------------------------------------------------------
// void SimplePacketTracer(int p_nOMPThreads)
// {
// 	//EngineKernel engine;
// 	//engine.GetPlugInManager()->Load("plugin.dll");
// 	//IDummy* pDummy = engine.GetDummyManager()->CreateInstance("DummyFactory", "MyInstance");
// 
// 	//std::cout<<"DummyPlugIn Name:"<<pDummy->GetName()<<std::endl;
// 
// 	//engine.GetPlugInManager()->Unload("plugin.dll");
// 
// 	//DummyFactory *pDummyFactory = new DummyFactory();
// 	//engine.dummyManager.RegisterFactory("Dummy", pDummyFactory);
// 	//Dummy *pDummy = engine.dummyManager.CreateInstance("Dummy", "Shitty");
// 
// 	//----------------------------------------------------------------------------------------------
// 	// Set number of OMP Threads
// 	//----------------------------------------------------------------------------------------------
// 	std::cout << "Initialising OMP thread count : [Threads = " << p_nOMPThreads << "]" << std::endl;
// 	omp_set_num_threads(p_nOMPThreads);
// 	//omp_set_num_threads(1);
// 
// 	//----------------------------------------------------------------------------------------------
// 	// Setup camera
// 	//----------------------------------------------------------------------------------------------
// 	PerspectiveCamera camera(
// 		//Vector3(0.0, 0.0, -10), Vector3(0.0f, 0.0f, 1.0f), Vector3::UnitYPos,
// 		//Vector3(-20.0, 0.0, 0.0), Vector3(1.0f, -0.05f, 0.0f), Vector3::UnitYPos,
// 		Vector3(-20.0, 10.0, -20.0), Vector3(1.0f, -0.5f, 1.0f), Vector3::UnitYPos,
// 		-2.0f, 2.0f, -2.0f, 2.0f, 2.0f);
// 
// 	//ThinLensCamera camera(
// 	//	Vector3(0.0, 0.0, -10), Vector3(0.5f, 0.5f, 1.0f), Vector3::UnitYPos,
// 	//	0.05f, -2.0f, 2.0f, -2.0f, 2.0f, 2.0f);
// 
// 	std::cout << "Setting up camera : [" << camera.ToString() << "]" << std::endl;
// 
// 	//----------------------------------------------------------------------------------------------
// 	// Setup textures
// 	//----------------------------------------------------------------------------------------------
// 	// Create marble texture
// 	std::cout << "Setting up textures..." << std::endl;
// 	MarbleTexture marbleTexture(0.002f, 1.0f, 4);
// 
// 	// Create image texture
// 	ImagePPM ppmLoader;
// 
// 	#if defined(__PLATFORM_WINDOWS__)
// 	boost::shared_ptr<ITexture> imgTexture(new ImageTexture("D:\\Media\\Assets\\IlluminaRT\\Textures\\texture.ppm", ppmLoader));
// 	#elif defined(__PLATFORM_LINUX__)
// 	boost::shared_ptr<ITexture> imgTexture(new ImageTexture("../../../Resource/Texture/texture.ppm", ppmLoader));
// 	#endif
// 
// 	//----------------------------------------------------------------------------------------------
// 	// Setup scene objects
// 	//----------------------------------------------------------------------------------------------
// 	// Initialising scene objects
// 	std::cout << "Initialising scene objects..." << std::endl;
// 
// 	// Create Sphere
// 	Sphere shape_sphere1(Vector3(0, -3.0f, 0), 1.0f);
// 	Sphere shape_sphere2(Vector3(8.0f, 8.0f, 10.0f), 2.0f);
// 
// 	// Create Box
// 	std::cout << "-- Create object : [box]" << std::endl;
// 
// 	boost::shared_ptr<BasicMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh1 =
// 		ShapeFactory::CreateBox<BasicMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(Vector3(-2,-2,-2), Vector3(2,2,2));
// 
// 	// Load Model 1
// 	#if defined(__PLATFORM_WINDOWS__)
// 	std::string fname_model01("D:\\Media\\Assets\\IlluminaRT\\Models\\ducky2.obj");
// 	#elif defined(__PLATFORM_LINUX__)
// 	std::string fname_model01("../../../Resource/Model/ducky2.obj");
// 	#endif
// 
// 	std::cout << "-- Load object : [" << fname_model01 << "]" << std::endl;
// 	//boost::shared_ptr<SimpleMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh2 =
// 	//	ShapeFactory::LoadMesh<SimpleMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model01);
// 	boost::shared_ptr<BVHMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh2 =
// 		ShapeFactory::LoadMesh<BVHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model01);
// 	//boost::shared_ptr<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh2 =
// 	//	ShapeFactory::LoadMesh<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model01);
// 
// 	// Load Model 2
// 	#if defined(__PLATFORM_WINDOWS__)
// 	//std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\sibenik3.obj");
// 	std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\sponza3.obj");
// 	//std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\conference3.obj");
// 	//std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\david.obj");
// 	//std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\box.obj");
// 	//std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\bunny2.obj");
// 	//std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\ducky2.obj");
// 	//std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\venusm.obj");
// 	//std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\torus.obj");
// 	#elif defined(__PLATFORM_LINUX__)
// 	//std::string fname_model02("../../../Resource/Model/sibenik3.obj");
// 	std::string fname_model02("../../../Resource/Model/sponza3.obj");
// 	//std::string fname_model02("../../../Resource/Model/conference3.obj");
// 	//std::string fname_model02("../../../Resource/Model/david.obj");
// 	//std::string fname_model02("../../../Resource/Model/box.obj");
// 	//std::string fname_model02("../../../Resource/Model/bunny2.obj");
// 	//std::string fname_model02("../../../Resource/Model/ducky2.obj");
// 	//std::string fname_model02("../../../Resource/Model/venusm.obj");
// 	//std::string fname_model02("../../../Resource/Model/torus.obj");
// 	#endif
// 
// 	std::cout << "-- Load object : [" << fname_model02 << "]" << std::endl;
// 
// 	//boost::shared_ptr<SimpleMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh3 =
// 	//	ShapeFactory::LoadMesh<SimpleMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model02);
// 	//boost::shared_ptr<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh3 =
// 	//	ShapeFactory::LoadMesh<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model02);
// 	//boost::shared_ptr<BVHMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh3 =
// 	//	ShapeFactory::LoadMesh<BVHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model02);
// 	//boost::shared_ptr<BIHMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh3 =
// 	//	ShapeFactory::LoadMesh<BIHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model02);
// 	boost::shared_ptr<PBIHMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh3 =
// 		ShapeFactory::LoadMesh<PBIHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model02);
// 	//boost::shared_ptr<GridMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh3 =
// 	//	ShapeFactory::LoadMesh<GridMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model02);
// 	//std::cout << "Save object : [sibenik.obj]" << std::endl;
// 	//ShapeFactory::SaveMesh<BVHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>("D:\\Assets\\object_out.obj", shape_mesh3);
// 
// 	//----------------------------------------------------------------------------------------------
// 	// Compute bounding volumes
// 	//----------------------------------------------------------------------------------------------
// 	std::cout << "Computing bounding volumes..." << std::endl;
// 	shape_sphere1.ComputeBoundingVolume();
// 	shape_sphere2.ComputeBoundingVolume();
// 	shape_mesh1->ComputeBoundingVolume();
// 	shape_mesh2->ComputeBoundingVolume();
// 	shape_mesh3->ComputeBoundingVolume();
// 	std::cout << std::endl;
// 
// 	//----------------------------------------------------------------------------------------------
// 	// Compile meshes
// 	//----------------------------------------------------------------------------------------------
// 	std::cout << "Compiling acceleration structure-based meshes..." << std::endl;
// 
// 	boost::timer compileTimer;
// 	compileTimer.restart();
// 	shape_mesh2->Compile();
// 	std::cout << "-- Model 01 : [" << fname_model01 << "] compiled in " << compileTimer.elapsed() << " seconds." << std::endl;
// 
// 	compileTimer.restart();
// 	shape_mesh3->Compile();
// 	std::cout << "-- Model 02 : [" << fname_model02 << "] compiled in " << compileTimer.elapsed() << " seconds." << std::endl;
// 
// 	//----------------------------------------------------------------------------------------------
// 	// Initialise scene space
// 	//----------------------------------------------------------------------------------------------
// 	std::cout << "Adding models to scene space..." << std::endl;
// 	//BasicSpace basicSpace;
// 	BasicSpace basicSpace;
// 
// 	{
// 	/*
// 	GeometricPrimitive pmv_sphere1;
// 	pmv_sphere1.SetShape(&shape_sphere1);
// 	basicSpace.PrimitiveList.PushBack(&pmv_sphere1);
// 	*/
// 
// 	/*
// 	GeometricPrimitive pmv_sphere2;
// 	pmv_sphere2.SetShape(&shape_sphere2);
// 	pmv_sphere2.WorldTransform.SetTranslation(Vector3(2.0f, 2.0f, 2.0f));
// 	basicSpace.PrimitiveList.PushBack(&pmv_sphere2);
// 	*/
// 
// 	/*
// 	GeometricPrimitive pmv_mesh1;
// 	pmv_mesh1.SetShape((Shape*)shape_mesh1.get());
// 	pmv_mesh1.WorldTransform.SetRotation(Matrix3x3::CreateRotation(Vector3::UnitYPos, 1.0f));
// 	basicSpace.PrimitiveList.PushBack(&pmv_mesh1);
// 	*/
// 
// 	/*
// 	GeometricPrimitive pmv_mesh2;
// 	pmv_mesh2.SetShape((Shape*)shape_mesh2.get());
// 	pmv_mesh2.WorldTransform.SetScaling(Vector3(3.0f, 3.0f, 3.0f));
// 	pmv_mesh2.WorldTransform.SetTranslation(Vector3(4.0f, 0.0f, 0.0f));
// 	basicSpace.PrimitiveList.PushBack(&pmv_mesh2);
// 	*/
// 
// 	/*
// 	GeometricPrimitive pmv_mesh3;
// 	pmv_mesh3.SetShape((Shape*)shape_mesh3.get());
// 	pmv_mesh3.WorldTransform.SetScaling(Vector3(3.0f, 3.0f, 3.0f));
// 	pmv_mesh3.WorldTransform.SetTranslation(Vector3(-6.0f, 0.0f, 0.0f));
// 	basicSpace.PrimitiveList.PushBack(&pmv_mesh3);
// 	*/
// 
// 	/*
// 	GeometricPrimitive pmv_mesh4;
// 	pmv_mesh4.SetShape((Shape*)shape_mesh2.get());
// 	pmv_mesh4.WorldTransform.SetScaling(Vector3(2.0f, 2.0f, 2.0f));
// 	pmv_mesh4.WorldTransform.SetTranslation(Vector3(-10.0f, 4.0f, 0.0f));
// 	basicSpace.PrimitiveList.PushBack(&pmv_mesh4);
// 	*/
// 
// 	/*
// 	GeometricPrimitive pmv_mesh5;
// 	pmv_mesh5.SetShape((Shape*)shape_mesh3.get());
// 	pmv_mesh5.WorldTransform.SetScaling(Vector3(2.0f, 2.0f, 2.0f));
// 	pmv_mesh5.WorldTransform.SetTranslation(Vector3(10.0f, -10.0f, -5.0f));
// 	basicSpace.PrimitiveList.PushBack(&pmv_mesh5);
// 	*/
// 	}
// 
// 	GeometricPrimitive pmv_mesh6;
// 	pmv_mesh6.SetShape((Shape*)shape_mesh3.get());
// 	//pmv_mesh6.WorldTransform.SetScaling(Vector3(0.005f, 0.005f, 0.005f));
// 	//pmv_mesh6.WorldTransform.SetScaling(Vector3(1.0f, 1.0f, 1.0f));
// 	//pmv_mesh6.WorldTransform.SetScaling(Vector3(2.0f, 2.0f, 2.0f));
// 	pmv_mesh6.WorldTransform.SetScaling(Vector3(3.0f, 3.0f, 3.0f));
// 	//pmv_mesh6.WorldTransform.SetScaling(Vector3(5.0f, 5.0f, 5.0f));
// 	//pmv_mesh6.WorldTransform.SetScaling(Vector3(7.5f, 7.5f, 7.5f));
// 	//pmv_mesh6.WorldTransform.SetScaling(Vector3(10.0f, 10.0f, 10.0f));
// 	//pmv_mesh6.WorldTransform.SetTranslation(Vector3(0.0f, -15.0f, 0.0f));
// 
// 	pmv_mesh6.WorldTransform.SetTranslation(Vector3(0.0f, -10.0f, 0.0f));
// 	basicSpace.PrimitiveList.PushBack(&pmv_mesh6);
// 
// 	// Prepare space
// 	basicSpace.Initialise();
// 	basicSpace.Build();
// 
// 	//----------------------------------------------------------------------------------------------
// 	// Scene creation complete
// 	//----------------------------------------------------------------------------------------------
// 	char cKey;
// 	std::cout << "Scene creation completed." << std::endl;
// 	std::cin >> cKey;
// 
// 	//----------------------------------------------------------------------------------------------
// 	// Setup rendering stage
// 	//----------------------------------------------------------------------------------------------
// 	const int MaxDepth = 1;
// 
// 	//const int width = 768,
// 	//	height = 768;
// 
// 	const int width = 512,
// 		height = 512;
// 
// 	//const int width = 256,
// 	//	height = 256;
// 
// 	// Create image
// 	Image canvas(width, height);
// 	boost::timer renderTimer;
// 	boost::progress_display renderProgress(height);
// 
// 	Vector3 lightPosition(0, 5, 0);
// 	float alpha = 0, totalFPS = 0;
// 
// 	// Iterate an arbitrary number of times
// 	for (int iteration = 1; iteration < 10000; iteration++)
// 	{
// 		renderTimer.restart();
// 		alpha += 0.05f;
// 
// 		camera.MoveTo(Vector3(Maths::Cos(alpha) * -20, 10.0, Maths::Sin(alpha) * -20));
// 		camera.LookAt(Vector3::Zero);
// 
// 		// Here we rebuild the AS
// 		basicSpace.Update();
// 
// 		// Render the scene
// 		Image *c = (Image*)&canvas;
// 		
// 		#pragma omp parallel for schedule(guided)
// 		for (int y = 0; y < height; ++y)
// 		{
// 			DifferentialSurface differentialSurface;
// 
// 			Vector3 lightVector,
// 				reflectionVector;
// 
// 			float testDensity,
// 				distance,
// 				diffuse,
// 				specular,
// 				shadow,
// 				contribution;
// 
// 			bool bHit;
// 
// 			for (int x = 0; x < width; x++)
// 			{
// 				RGBPixel pixelColour = RGBPixel::Black,
// 					finalColour = RGBPixel::Black;
// 
// 				Ray ray = camera.GetRay(
// 					((float)x) / width,
// 					((float)y) / height,
// 					0.5f, 0.5f );
// 
// 				bHit = false;
// 
// 				contribution =
// 					shadow = 1.0f;
// 
// 				for (int depth = 0; depth < MaxDepth; depth++)
// 				{
// 					bHit = basicSpace.Intersects(ray, 0, differentialSurface); // .Intersects(ray, 0.0f, differentialSurface, testDensity);
// 
// 					if (bHit)
// 					{
// 						//pixelColour = marbleTexture.GetValue(differentialSurface.PointUV, differentialSurface.PointWS * 20);
// 						pixelColour.Set(pixelColour.R * 0.5f + 0.5f, pixelColour.G * 0.5f + 0.4f, pixelColour.B * 0.5f + 0.3f);
// 						Vector3::Subtract(lightPosition, differentialSurface.PointWS, lightVector);
// 						Vector3 normal(differentialSurface.BasisWS.U);
// 						normal.Y = -normal.Y;
// 
// 						distance = lightVector.Length();
// 						lightVector.Normalize();
// 						diffuse = Maths::Clamp(Vector3::Dot(lightVector, normal), 0.3f, 1.0f);
// 
// 						Ray shadowRay(differentialSurface.PointWS, lightVector, 0.0001f, distance - 0.01f);
// 						shadow = basicSpace.Intersects(shadowRay, 0.0f) ? 0.4f : 1.0f;
// 
// 						Vector3::Reflect(lightVector, normal, lightVector);
// 						specular = Maths::Pow(Vector3::Dot(ray.Direction, lightVector), 64);
// 
// 						pixelColour *= shadow * (diffuse + specular) * contribution;
// 						finalColour += pixelColour;
// 						contribution *= 0.25f;
// 
// 						if (depth < MaxDepth - 1)
// 						{
// 							Vector3::Reflect(ray.Direction, normal, reflectionVector);
// 							ray.Direction = reflectionVector;
// 							ray.Origin = differentialSurface.PointWS + reflectionVector * 0.0001f;
// 						}
// 					}
// 					else
// 						break;
// 				}
// 
// 				//std::cout << "Block Y = " << y << std::endl;
// 				c->Set(x, (height - 1) - y, finalColour);
// 				//canvas.Set(x, (height - 1) - y, finalColour);
// 			}
// 
// 			++renderProgress;
// 		}
// 
// 		totalFPS += (float)(1.0 / renderTimer.elapsed());
// 		std::cout<< shape_mesh3->ToString() << std::endl;
// 		std::cout << "Total Render Time : " << renderTimer.elapsed() << " seconds" << std::endl;
// 		std::cout << "FPS: [" << totalFPS / iteration <<" / " << iteration << "]" << std::endl;
// 
// 		ImagePPM imagePPM;
// 		#if defined(__PLATFORM_WINDOWS__)
// 		imagePPM.Save(canvas, "D:\\Media\\Assets\\IlluminaRT\\Textures\\result.ppm");
// 		#elif defined(__PLATFORM_LINUX__)
// 		imagePPM.Save(canvas, "../../../Resource/Texture/result.ppm");
// 		#endif
// 	}
// }

void RayTracer(int p_nOMPThreads)
{
	//----------------------------------------------------------------------------------------------
	// Set number of OMP Threads
	//----------------------------------------------------------------------------------------------
	std::cout << "Initialising OMP thread count : [Threads = " << p_nOMPThreads << "]" << std::endl;
	omp_set_num_threads(p_nOMPThreads);

	//----------------------------------------------------------------------------------------------
	// Setup camera
	//----------------------------------------------------------------------------------------------
	PerspectiveCamera camera(
		Vector3(-20.0, 10.0, -20.0), Vector3(1.0f, -0.5f, 1.0f), Vector3::UnitYPos,
		-2.0f, 2.0f, -2.0f, 2.0f, 2.0f);

	std::cout << "Setting up camera : [" << camera.ToString() << "]" << std::endl;

	//----------------------------------------------------------------------------------------------
	// Setup textures
	//----------------------------------------------------------------------------------------------

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

	#if defined(__PLATFORM_WINDOWS__)
		ITexture* pTexture1 = engineKernel.GetTextureManager()->CreateInstance("Image", "Image Texture 1", "Name=Image_Texture;Filename=D:\\Development\\IlluminaPRT\\Resource\\Texture\\texture.ppm;Filetype=PPM;");
	#elif defined(__PLATFORM_LINUX__)
		ITexture* pTexture1 = engineKernel.GetTextureManager()->CreateInstance("Image", "Image Texture 1", "Name=Image_Texture;Filename=../../../Resource/Texture/texture.ppm;Filetype=PPM;");
	#endif

	ITexture* pTexture2 = engineKernel.GetTextureManager()->CreateInstance("Marble", "Marble Texture 1", "Name=Marble_Texture;Stripes=0.002;Scale=1.0;Octaves=4;");

	//----------------------------------------------------------------------------------------------
	// Materials
	//----------------------------------------------------------------------------------------------
	engineKernel.GetMaterialManager()->RegisterFactory("Matte", new MatteMaterialFactory());
	engineKernel.GetMaterialManager()->RegisterFactory("Mirror", new MirrorMaterialFactory());
	engineKernel.GetMaterialManager()->RegisterFactory("Group", new MaterialGroupFactory());

	IMaterial* pMaterial1 = engineKernel.GetMaterialManager()->CreateInstance("Matte", "Diffuse Material 1", "Name=Diffuse_Material;Reflectivity=0.75,0.75,0.5;");
	IMaterial* pMaterial2 = engineKernel.GetMaterialManager()->CreateInstance("Mirror", "Phong Material 1", "Name=Phong_Material;Reflectivity=0.75,0.75,0.5;Exponent=32;");
	MaterialGroup* pMaterialGroup = (MaterialGroup*)engineKernel.GetMaterialManager()->CreateInstance("Group", "Material Group 1", "Name=Group1");
	pMaterialGroup->Add(pMaterial1, 0);
	pMaterialGroup->Add(pMaterial2, 1);
	
	MatteMaterial material_mesh1(Spectrum(0.75,0.75,0.30));
	//PhongMaterial material_mesh1(Spectrum(0.75,0.75,0.30), 32);

	//----------------------------------------------------------------------------------------------
	// Setup scene objects
	//----------------------------------------------------------------------------------------------
	// Initialising scene objects
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

	// teapot light
	// Load secondary model to use for light
	//boost::shared_ptr<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh3 =
	//	ShapeFactory::LoadMesh<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(
	//	"D:\\Development\\IlluminaPRT\\Resource\\Model\\Bunny\\bunny.obj",
	//	&engineKernel, &pMeshMaterialGroup2);
	//DiffuseAreaLight diffuseLight1(NULL, (IShape*)shape_mesh3.get(), Spectrum(500, 500, 500));

	// sphere arealight
	// Sponza, et al.
	//Sphere shape_mesh2(Vector3(0, 7.0f, 0), 2.0f);
	//Sphere shape_mesh2(Vector3(0.0, 15.0f, 0.0), 0.5f);
	Sphere shape_mesh2(Vector3(0.0, 16.5f, 0.0), 0.5f);
	//DiffuseAreaLight diffuseLight2(NULL, &shape_mesh2, Spectrum(1e+3, 1e+3, 1e+3));
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
	std::cout << "Computing bounding volumes..." << std::endl;
	shape_mesh1->ComputeBoundingVolume();
	shape_mesh2.ComputeBoundingVolume();
	shape_mesh3->ComputeBoundingVolume();
	std::cout << std::endl;

	//----------------------------------------------------------------------------------------------
	// Compute normals
	//----------------------------------------------------------------------------------------------
	std::cout << "Computing mesh normals..." << std::endl;
	shape_mesh1->UpdateNormals();
	//shape_mesh3->UpdateNormals();
	std::cout << std::endl;

	//----------------------------------------------------------------------------------------------
	// Compile meshes
	//----------------------------------------------------------------------------------------------
	std::cout << "Compiling acceleration structure-based meshes..." << std::endl;

	boost::timer compileTimer;
	compileTimer.restart();
	shape_mesh1->Compile();
	shape_mesh3->Compile();
	std::cout << "-- Model 01 : [" << fname_model01 << "] compiled in " << compileTimer.elapsed() << " seconds." << std::endl;

	//----------------------------------------------------------------------------------------------
	// Initialise scene space
	//----------------------------------------------------------------------------------------------
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
	pmv_mesh2.SetMaterial((IMaterial*)&material_mesh1);
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

	BasicRenderer basicRenderer(&scene, &camera, &integrator, &device, &filter, 1);
	
	char cKey;
	std::cout << "Scene creation completed." << std::endl;
	std::cin >> cKey;

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

	for (int iteration = 1; iteration < 10000; iteration++)
	{
		renderTimer.restart();
		alpha += 0.1f;
	 
		camera.MoveTo(Vector3(Maths::Cos(alpha) * cDistX, cDistY, Maths::Sin(alpha) * cDistZ));
		camera.LookAt(lookat);
	 
		// Here we rebuild the AS
		basicSpace.Update();
	 
		// Render
		basicRenderer.Render();
	 
		totalFPS += (float)(1.0 / renderTimer.elapsed());
		std::cout << shape_mesh1->ToString() << std::endl;
		std::cout << "Total Render Time : " << renderTimer.elapsed() << " seconds" << std::endl;
		std::cout << "FPS: [" << totalFPS / iteration <<" / " << iteration << "]" << std::endl;
	}
}

int main()
{
	int nCores = Platform::GetProcessorCount();
	std::cout << "Hardware cores : " << nCores << std::endl;

	//TestAtomic();
	//TestUID();
	//TestSpinLock();
	//TestLists();

	//SimpleTracer(4);
	//SimplePacketTracer(nCores);
	//TileBasedTracer(nCores);

	RayTracer(nCores / 2);

	std::cout << "Complete in " << Platform::GetTime() << " seconds " << std::endl;

	char cKey;
	cin >> cKey;
}
