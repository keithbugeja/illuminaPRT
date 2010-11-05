#include <vector>
#include <iostream>
//#include <omp.h>

#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"

#include "boost/timer.hpp"
#include "boost/progress.hpp"

#include "System/Platform.h"
#include "Threading/Spinlock.h"
#include "Threading/Monitor.h"
#include "Threading/List.h"
#include "Threading/Queue.h"
#include "Geometry/Vector2.h"
#include "Geometry/Vector3.h"
#include "Geometry/Matrix3x3.h"
#include "Geometry/Matrix4x4.h"
#include "Geometry/Transform.h"
#include "Geometry/Ray.h"
#include "Sampler/Sampler.h"
#include "Filter/Filter.h"
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
#include "Shape/OctreeMesh.h"
#include "Shape/BVHMesh.h"
#include "Shape/KDTreeMesh.h"
#include "Shape/BIHMesh.h"
#include "Shape/HybridMesh.h"
#include "Staging/GeometricPrimitive.h"
#include "Staging/OctreeAggregate.h"
#include "Space/BasicSpace.h"
#include "Space/BVHSpace.h"
#include "Object/Object.h"
#include "Threading/Atomic.h"
#include "Threading/AtomicReference.h"
#include "Threading/LinkedList.h"
 
using namespace std;
using namespace Illumina::Core;
 
void TestLists(void)
{
	//omp_set_num_threads(8);

	Object *a = new Object("Keith1");
	Object *b = new Object("Keith2");

	Object *c = a;
	bool result = AtomicReference::CompareAndSet((void**)&c, b, b);
	std::cout<<result<<" : "<<c->ToString()<<std::endl;
 
	Queue queue;
	Object *p = NULL;
 
	#pragma omp parallel for 
	for (int j = 0; j < 100000; j++)
	{
		queue.Enqueue(a);
		queue.Enqueue(b);
		queue.Dequeue();
		p = (Object*)queue.Dequeue();
	}
 
	std::cout<<p<<std::endl;
 
	queue.Enqueue(a);
	queue.Enqueue(b);
 
	Object *da = (Object*)queue.Dequeue();
	std::cout<<da->ToString() << std::endl;
 
	Object *db = (Object*)queue.Dequeue();
	std::cout<<db->ToString() << std::endl;
 
	Object *dc = (Object*)queue.Dequeue();
	std::cout<<"Last obj : " << dc << std::endl;

	if (dc == NULL)
		std::cout<<"Queue is empty! Correct!"<< std::endl;
	else
		std::cout<<"Queue is not empty! Wrong!"<<std::endl;
 
	std::cout<<"Instance count:"<<QueueNode::NodeInstanceCount<<std::endl;
}
 
void TestAtomic(void)
{
	Int32	a[2] = {1, 2},
			b[2] = {1, 2};
 
	if (Atomic::DoubleCompareAndSwap(a, 4, 5, b))
		std::cout<<"DCAS -> Success!"<<std::endl;
	else
		std::cout<<"DCAS -> Failed!"<<std::endl;
 
#if defined(__ARCHITECTURE_X64__)
	ALIGN_16 Int64  a64[2] = {3, 4},
					b64[2] = {3, 4};
 
	if (Atomic::DoubleCompareAndSwap(a64, 69, 58, b64))
		std::cout<<"DCAS -> Success!"<<std::endl;
	else
		std::cout<<"DCAS -> Failed!"<<std::endl;
 
	AtomicStampedReference<void> s;
	s.Set((void*)0x1000, 0);
 
	#pragma omp parallel for
	for (int n = 0; n < 1000000; n++)
	{
		s.CompareAndSet((void*)0x1000, (void*)0x2000, 0, 1);
		s.CompareAndSet((void*)0x2000, (void*)0x3000, 1, 2);
		s.CompareAndSet((void*)0x3000, (void*)0x4000, 2, 3);
		s.CompareAndSet((void*)0x4000, (void*)0x1000, 3, 0);
	}
 
	std::cout<<"Ref : [0x1000] " << s.GetReference() <<", [0] " << s.GetStamp() << std::endl;
#endif
}
 
void TestUID(void)
{
	//omp_set_num_threads(8);

	List<Object> objectList;

	#pragma omp parallel for
	for (int j = 0; j < 1000; j++)
	{
		Object t("test");

		std::cout << t.ToString() << " Hash : " << t.GetHashCode() << std::endl;
		objectList.PushBack(t);
	}
 
	std::cout<<objectList.Size()<<std::endl;
	std::cin.get();
 
	exit(0);
}
 
void TestSpinLock(void)
{
	//omp_set_num_threads(Platform::GetProcessorCount());
 
	Spinlock lock(100);

	int sharedvar = 0;
	double start = Platform::GetTime();
 
	#pragma omp parallel for
	for (int j = 0; j < 100000; j++)
	{
		lock.Lock();
		sharedvar++;
		lock.Unlock();
	}
 
	double end = Platform::GetTime();
 
	std::cout << "Result : " << sharedvar << ", Time : " << end - start << std::endl;
}
 
//----------------------------------------------------------------------------------------------
void SimplePacketTracer(int p_nOMPThreads)
{
	//----------------------------------------------------------------------------------------------
	// Set number of OMP Threads
	//----------------------------------------------------------------------------------------------
	std::cout << "Initialising OMP thread count : [Threads = " << p_nOMPThreads << "]" << std::endl;
	//omp_set_num_threads(p_nOMPThreads);
	//omp_set_num_threads(1);
 
	//----------------------------------------------------------------------------------------------
	// Setup camera
	//----------------------------------------------------------------------------------------------
	PerspectiveCamera camera(
		//Vector3(0.0, 0.0, -10), Vector3(0.0f, 0.0f, 1.0f), Vector3::UnitYPos,
		//Vector3(-20.0, 0.0, 0.0), Vector3(1.0f, -0.05f, 0.0f), Vector3::UnitYPos,
		Vector3(-20.0, 10.0, -20.0), Vector3(1.0f, -0.5f, 1.0f), Vector3::UnitYPos,
		-2.0f, 2.0f, -2.0f, 2.0f, 2.0f);
	
	//ThinLensCamera camera(
	//	Vector3(0.0, 0.0, -10), Vector3(0.5f, 0.5f, 1.0f), Vector3::UnitYPos,
	//	0.05f, -2.0f, 2.0f, -2.0f, 2.0f, 2.0f);

	std::cout << "Setting up camera : [" << camera.ToString() << "]" << std::endl;
 
	//----------------------------------------------------------------------------------------------
	// Setup textures
	//----------------------------------------------------------------------------------------------
	// Create marble texture
	std::cout << "Setting up textures..." << std::endl;
	MarbleTexture marbleTexture(0.002f, 1.0f, 4);
	
	// Create image texture
	ImagePPM ppmLoader;
	boost::shared_ptr<ITexture> imgTexture(new ImageTexture("D:\\Media\\Assets\\IlluminaRT\\Textures\\texture.ppm", ppmLoader));
 
	//----------------------------------------------------------------------------------------------
	// Setup scene objects
	//----------------------------------------------------------------------------------------------
	// Initialising scene objects
	std::cout << "Initialising scene objects..." << std::endl;

	// Create Sphere
	Sphere shape_sphere1(Vector3(0, -3.0f, 0), 1.0f);
	Sphere shape_sphere2(Vector3(8.0f, 8.0f, 10.0f), 2.0f);
	
	// Create Box
	std::cout << "-- Create object : [box]" << std::endl;

	boost::shared_ptr<BasicMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh1 = 
		ShapeFactory::CreateBox<BasicMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(Vector3(-2,-2,-2), Vector3(2,2,2));

	// Load Model 1
	std::string fname_model01("D:\\Media\\Assets\\IlluminaRT\\Models\\ducky2.obj");
	std::cout << "-- Load object : [" << fname_model01 << "]" << std::endl;
	//boost::shared_ptr<SimpleMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh2 = 
	//	ShapeFactory::LoadMesh<SimpleMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model01);
	boost::shared_ptr<BVHMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh2 = 
		ShapeFactory::LoadMesh<BVHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model01);
	//boost::shared_ptr<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh2 = 
	//	ShapeFactory::LoadMesh<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model01);

	// Load Model 2
	//std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\sibenik3.obj");
	std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\sponza3.obj");
	//std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\conference3.obj");
	//std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\david.obj");
	//std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\box.obj");
	//std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\bunny2.obj");
	//std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\ducky2.obj");
	//std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\venusm.obj");
	//std::string fname_model02("D:\\Media\\Assets\\IlluminaRT\\Models\\torus.obj");
	std::cout << "-- Load object : [" << fname_model02 << "]" << std::endl;

	//boost::shared_ptr<SimpleMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh3 = 
	//	ShapeFactory::LoadMesh<SimpleMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model02);
	//boost::shared_ptr<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh3 = 
	//	ShapeFactory::LoadMesh<KDTreeMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model02);
	//boost::shared_ptr<BVHMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh3 = 
	//	ShapeFactory::LoadMesh<BVHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model02);
	//boost::shared_ptr<BIHMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh3 = 
	//	ShapeFactory::LoadMesh<BIHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model02);
	boost::shared_ptr<PBIHMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh3 = 
		ShapeFactory::LoadMesh<PBIHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model02);
	//boost::shared_ptr<GridMesh<IndexedTriangle<Vertex>, Vertex>> shape_mesh3 = 
	//	ShapeFactory::LoadMesh<GridMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>(fname_model02);
	//std::cout << "Save object : [sibenik.obj]" << std::endl;
	//ShapeFactory::SaveMesh<BVHMesh<IndexedTriangle<Vertex>, Vertex>, Vertex>("D:\\Assets\\object_out.obj", shape_mesh3);
 
	//----------------------------------------------------------------------------------------------
	// Compute bounding volumes
	//----------------------------------------------------------------------------------------------
	std::cout << "Computing bounding volumes..." << std::endl;
	shape_sphere1.ComputeBoundingVolume();
	shape_sphere2.ComputeBoundingVolume();
	shape_mesh1->ComputeBoundingVolume();
	shape_mesh2->ComputeBoundingVolume();
	shape_mesh3->ComputeBoundingVolume();
	std::cout << std::endl;

	//----------------------------------------------------------------------------------------------
	// Compile meshes
	//----------------------------------------------------------------------------------------------
	std::cout << "Compiling acceleration structure-based meshes..." << std::endl;

	boost::timer compileTimer;
	compileTimer.restart();
	shape_mesh2->Compile();
	std::cout << "-- Model 01 : [" << fname_model01 << "] compiled in " << compileTimer.elapsed() << " seconds." << std::endl;

	compileTimer.restart();
	shape_mesh3->Compile();
	std::cout << "-- Model 02 : [" << fname_model02 << "] compiled in " << compileTimer.elapsed() << " seconds." << std::endl;
 
	//----------------------------------------------------------------------------------------------
	// Initialise scene space
	//----------------------------------------------------------------------------------------------
	std::cout << "Adding models to scene space..." << std::endl;
	//BasicSpace basicSpace;
	BasicSpace basicSpace;
 
	{
	/*
	GeometricPrimitive pmv_sphere1;
	pmv_sphere1.SetShape(&shape_sphere1);
	basicSpace.PrimitiveList.PushBack(&pmv_sphere1);
	*/

	/*
	GeometricPrimitive pmv_sphere2;
	pmv_sphere2.SetShape(&shape_sphere2);
	pmv_sphere2.WorldTransform.SetTranslation(Vector3(2.0f, 2.0f, 2.0f));
	basicSpace.PrimitiveList.PushBack(&pmv_sphere2);
	*/

	/*
	GeometricPrimitive pmv_mesh1;
	pmv_mesh1.SetShape((Shape*)shape_mesh1.get());
	pmv_mesh1.WorldTransform.SetRotation(Matrix3x3::CreateRotation(Vector3::UnitYPos, 1.0f));
	basicSpace.PrimitiveList.PushBack(&pmv_mesh1);
	*/

	/*
	GeometricPrimitive pmv_mesh2;
	pmv_mesh2.SetShape((Shape*)shape_mesh2.get());
	pmv_mesh2.WorldTransform.SetScaling(Vector3(3.0f, 3.0f, 3.0f));
	pmv_mesh2.WorldTransform.SetTranslation(Vector3(4.0f, 0.0f, 0.0f));
	basicSpace.PrimitiveList.PushBack(&pmv_mesh2);
	*/

	/*
	GeometricPrimitive pmv_mesh3;
	pmv_mesh3.SetShape((Shape*)shape_mesh3.get());
	pmv_mesh3.WorldTransform.SetScaling(Vector3(3.0f, 3.0f, 3.0f));
	pmv_mesh3.WorldTransform.SetTranslation(Vector3(-6.0f, 0.0f, 0.0f));
	basicSpace.PrimitiveList.PushBack(&pmv_mesh3);
	*/

	/*
	GeometricPrimitive pmv_mesh4;
	pmv_mesh4.SetShape((Shape*)shape_mesh2.get());
	pmv_mesh4.WorldTransform.SetScaling(Vector3(2.0f, 2.0f, 2.0f));
	pmv_mesh4.WorldTransform.SetTranslation(Vector3(-10.0f, 4.0f, 0.0f));
	basicSpace.PrimitiveList.PushBack(&pmv_mesh4);
	*/
	
	/*
	GeometricPrimitive pmv_mesh5;
	pmv_mesh5.SetShape((Shape*)shape_mesh3.get());
	pmv_mesh5.WorldTransform.SetScaling(Vector3(2.0f, 2.0f, 2.0f));
	pmv_mesh5.WorldTransform.SetTranslation(Vector3(10.0f, -10.0f, -5.0f));
	basicSpace.PrimitiveList.PushBack(&pmv_mesh5);
	*/
	}

	GeometricPrimitive pmv_mesh6;
	pmv_mesh6.SetShape((Shape*)shape_mesh3.get());
	//pmv_mesh6.WorldTransform.SetScaling(Vector3(0.005f, 0.005f, 0.005f));
	//pmv_mesh6.WorldTransform.SetScaling(Vector3(1.0f, 1.0f, 1.0f));
	//pmv_mesh6.WorldTransform.SetScaling(Vector3(2.0f, 2.0f, 2.0f));
	pmv_mesh6.WorldTransform.SetScaling(Vector3(3.0f, 3.0f, 3.0f));
	//pmv_mesh6.WorldTransform.SetScaling(Vector3(5.0f, 5.0f, 5.0f));
	//pmv_mesh6.WorldTransform.SetScaling(Vector3(7.5f, 7.5f, 7.5f));
	//pmv_mesh6.WorldTransform.SetScaling(Vector3(10.0f, 10.0f, 10.0f));
	//pmv_mesh6.WorldTransform.SetTranslation(Vector3(0.0f, -15.0f, 0.0f));

	pmv_mesh6.WorldTransform.SetTranslation(Vector3(0.0f, -10.0f, 0.0f));
	basicSpace.PrimitiveList.PushBack(&pmv_mesh6);
 
	// Prepare space
	basicSpace.Initialise();
	basicSpace.Build();

	//----------------------------------------------------------------------------------------------
	// Scene creation complete
	//----------------------------------------------------------------------------------------------
	char cKey;
	std::cout << "Scene creation completed." << std::endl;
	std::cin >> cKey;

	//----------------------------------------------------------------------------------------------
	// Setup rendering stage
	//----------------------------------------------------------------------------------------------
	const int MaxDepth = 1;

	//const int width = 768, 
	//	height = 768;

	const int width = 512,
		height = 512;

	//const int width = 256, 
	//	height = 256;
 
	// Create image
	Image canvas(width, height);
	boost::timer renderTimer;
	boost::progress_display renderProgress(height);
 
	Vector3 lightPosition(0, 5, 0);
	float alpha = 0, totalFPS = 0;

	// Iterate an arbitrary number of times
	for (int iteration = 1; iteration < 10000; iteration++)
	{
		renderTimer.restart();
		alpha += 0.05f;
 
		camera.MoveTo(Vector3(Maths::Cos(alpha) * -20, 10.0, Maths::Sin(alpha) * -20));
		camera.LookAt(Vector3::Zero);

		// Here we rebuild the AS
		basicSpace.Update();

		// Render the scene
		#pragma omp parallel for schedule(guided)
		for (int y = 0; y < height; y++)
		{
			DifferentialSurface differentialSurface;

			Vector3 lightVector,
				reflectionVector;

			float testDensity, 
				distance,
				diffuse,
				specular,
				shadow,
				contribution;

			bool bHit;
 
			for (int x = 0; x < width; x++)
			{
				RGBPixel pixelColour = RGBPixel::Black,
					finalColour = RGBPixel::Black;
			
				Ray &ray = camera.GetRay(
					((float)x) / width, 
					((float)y) / height, 
					0.5f, 0.5f );

				bHit = false;

				contribution = 
					shadow = 1.0f;

				for (int depth = 0; depth < MaxDepth; depth++)
				{
					bHit = basicSpace.Intersects(ray, 0.0f, differentialSurface, testDensity);

					if (bHit)
					{
						//pixelColour = marbleTexture.GetValue(differentialSurface.PointUV, differentialSurface.PointWS * 20);
						pixelColour.Set(pixelColour.R * 0.5f + 0.5f, pixelColour.G * 0.5f + 0.4f, pixelColour.B * 0.5f + 0.3f);
						Vector3::Subtract(lightPosition, differentialSurface.PointWS, lightVector);
						Vector3 normal(differentialSurface.BasisWS.U);
						normal.Y = -normal.Y;
 
						distance = lightVector.Length();
						lightVector.Normalize();
						diffuse = Maths::Clamp(Vector3::Dot(lightVector, normal), 0.3f, 1.0f);
 
						Ray shadowRay(differentialSurface.PointWS, lightVector, 0.0001f, distance - 0.01f);
						shadow = basicSpace.Intersects(shadowRay, 0.0f) ? 0.4f : 1.0f;

						Vector3::Reflect(lightVector, normal, lightVector);
						specular = Maths::Pow(Vector3::Dot(ray.Direction, lightVector), 64);

						pixelColour *= shadow * (diffuse + specular) * contribution;
						finalColour += pixelColour;
						contribution *= 0.25f;

						if (depth < MaxDepth - 1)
						{
							Vector3::Reflect(ray.Direction, normal, reflectionVector);
							ray.Direction = reflectionVector;
							ray.Origin = differentialSurface.PointWS + reflectionVector * 0.0001f;
						}
					}
					else
						break;
				}
			
				canvas.Set(x, (height - 1) - y, finalColour);
			}
 
			++renderProgress;
		}
 
		totalFPS += (float)(1.0 / renderTimer.elapsed());
		std::cout<< shape_mesh3->ToString() << std::endl;
		std::cout << "Total Render Time : " << renderTimer.elapsed() << " seconds" << std::endl;
		std::cout << "FPS: [" << totalFPS / iteration <<" / " << iteration << "]" << std::endl;
 
		ImagePPM imagePPM;
		imagePPM.Save(canvas, "D:\\Media\\Assets\\IlluminaRT\\Textures\\result.ppm");
	}
}
 
int main(const int& arg)
{
	int nCores = Platform::GetProcessorCount();
	std::cout << "Hardware cores : " << nCores << std::endl;
 
	//TestAtomic();
	//TestUID();
	//TestSpinLock();
	//TestLists();
 
	//SimpleTracer(4);
	SimplePacketTracer(nCores);
	std::cout << "Complete in " << Platform::GetTime() << " seconds " << std::endl;
 
	char cKey;
	cin >> cKey;
}