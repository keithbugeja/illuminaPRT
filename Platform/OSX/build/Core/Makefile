#------------------------------------------------------------------------------#
#------------------------------------------------------------------------------#

WSDIR = `pwd`


CC = mpicc
CPP = mpic++
F77 = f77
F9X = gfortran
LD = mpic++
AR = ar
RANLIB = ranlib
WINDRES = windres

INC =
CFLAGS = -mtune=opteron-sse3 -march=opteron-sse3
RESINC = 
RCFLAGS = 
LIBDIR =
LIB =
LDFLAGS =

INC_DEBUG = $(INC) -I../../../../Core -I../../../../../Libraries/boost -I../../../../../Libraries/ffmpeg -I../../../../../Libraries/vlc/include
CFLAGS_DEBUG = $(CFLAGS) -std=c++0x -Wall -g
RESINC_DEBUG = $(RESINC)
RCFLAGS_DEBUG = $(RCFLAGS)
LIBDIR_DEBUG = $(LIBDIR)  -L../../../../../Libraries/boost/lib -I../../../../../Libraries/vlc/lib
LIB_DEBUG = $(LIB)
LDFLAGS_DEBUG = $(LDFLAGS)
SRCDIR_DEBUG = ../../../..
OBJDIR_DEBUG = ../../../../Bin/OSX/Debug/obj
DEP_DEBUG = 
OUT_DEBUG = ../../../../Bin/OSX/Debug/libCore.a

INC_RELEASE = $(INC) -I../../../../Core -I../../../../../Libraries/boost -I../../../../../Libraries/ffmeg -I../../../../../Libraries/vlc/include
CFLAGS_RELEASE = $(CFLAGS)  -O3 -fomit-frame-pointer -ftree-vectorize -fno-math-errno -msse3 -march=native -mtune=native -fexpensive-optimizations -std=c++0x 
RESINC_RELEASE = $(RESINC)
RCFLAGS_RELEASE = $(RCFLAGS)
LIBDIR_RELEASE = $(LIBDIR) -L../../../../../Libraries/boost/lib -I../../../../../Libraries/vlc
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE = $(LDFLAGS) -s
SRCDIR_RELEASE = ../../../..
OBJDIR_RELEASE = ../../../../Bin/OSX/Release/obj
DEP_RELEASE = 
OUT_RELEASE = ../../../../Bin/OSX/Release/libCore.a

OBJ_DEBUG = $(OBJDIR_DEBUG)/Core/Camera/Camera.o $(OBJDIR_DEBUG)/Core/Camera/PerspectiveCamera.o $(OBJDIR_DEBUG)/Core/Camera/ThinLensCamera.o $(OBJDIR_DEBUG)/Core/Device/DisplayDevice.o $(OBJDIR_DEBUG)/Core/Device/ImageDevice.o $(OBJDIR_DEBUG)/Core/Device/RTPDevice.o $(OBJDIR_DEBUG)/Core/Device/VideoDevice.o $(OBJDIR_DEBUG)/Core/Exception/Exception.o $(OBJDIR_DEBUG)/Core/External/Compression/lz4.o $(OBJDIR_DEBUG)/Core/External/Video/VideoStream.o $(OBJDIR_DEBUG)/Core/Filter/BoxFilter.o $(OBJDIR_DEBUG)/Core/Filter/TentFilter.o $(OBJDIR_DEBUG)/Core/Geometry/Basis.o $(OBJDIR_DEBUG)/Core/Geometry/BoundingBox.o $(OBJDIR_DEBUG)/Core/Geometry/Intersection.o $(OBJDIR_DEBUG)/Core/Geometry/Interval.o $(OBJDIR_DEBUG)/Core/Geometry/Matrix3x3.o $(OBJDIR_DEBUG)/Core/Geometry/Matrix4x4.o $(OBJDIR_DEBUG)/Core/Geometry/Plane.o $(OBJDIR_DEBUG)/Core/Geometry/Ray.o $(OBJDIR_DEBUG)/Core/Geometry/Transform.o $(OBJDIR_DEBUG)/Core/Geometry/Vector2.o $(OBJDIR_DEBUG)/Core/Geometry/Vector3.o $(OBJDIR_DEBUG)/Core/Image/Image.o $(OBJDIR_DEBUG)/Core/Image/RGBPixel.o $(OBJDIR_DEBUG)/Core/Integrator/IGIIntegrator.o $(OBJDIR_DEBUG)/Core/Integrator/Integrator.o $(OBJDIR_DEBUG)/Core/Integrator/PathIntegrator.o $(OBJDIR_DEBUG)/Core/Integrator/WhittedIntegrator.o $(OBJDIR_DEBUG)/Core/Integrator/TestIntegrator.o $(OBJDIR_DEBUG)/Core/Light/AreaLight.o $(OBJDIR_DEBUG)/Core/Light/DiffuseAreaLight.o $(OBJDIR_DEBUG)/Core/Light/InfiniteAreaLight.o $(OBJDIR_DEBUG)/Core/Light/PointLight.o $(OBJDIR_DEBUG)/Core/Material/BSDF.o $(OBJDIR_DEBUG)/Core/Material/LambertianBxDF.o $(OBJDIR_DEBUG)/Core/Material/SpecularReflectionBxDF.o $(OBJDIR_DEBUG)/Core/Material/SpecularTransmissionBxDF.o $(OBJDIR_DEBUG)/Core/Maths/Random.o $(OBJDIR_DEBUG)/Core/Maths/Maths.o $(OBJDIR_DEBUG)/Core/Object/Object.o $(OBJDIR_DEBUG)/Core/Object/UniqueID.o $(OBJDIR_DEBUG)/Core/Renderer/Renderer.o $(OBJDIR_DEBUG)/Core/Renderer/BaseRenderer.o $(OBJDIR_DEBUG)/Core/Renderer/BasicRenderer.o $(OBJDIR_DEBUG)/Core/Renderer/DistributedRenderer.o $(OBJDIR_DEBUG)/Core/Renderer/TimeConstrainedRenderer.o $(OBJDIR_DEBUG)/Core/Sampler/JitterSampler.o $(OBJDIR_DEBUG)/Core/Sampler/MultijitterSampler.o $(OBJDIR_DEBUG)/Core/Sampler/PrecomputationSampler.o $(OBJDIR_DEBUG)/Core/Sampler/RandomSampler.o $(OBJDIR_DEBUG)/Core/Sampler/SamplerDiagnostics.o $(OBJDIR_DEBUG)/Core/Scene/EmissivePrimitive.o $(OBJDIR_DEBUG)/Core/Scene/Environment.o $(OBJDIR_DEBUG)/Core/Scene/EnvironmentLoader.o $(OBJDIR_DEBUG)/Core/Scene/GeometricPrimitive.o $(OBJDIR_DEBUG)/Core/Scene/ParticleSceneLoader.o $(OBJDIR_DEBUG)/Core/Scene/Scene.o $(OBJDIR_DEBUG)/Core/Scene/SceneLoader.o $(OBJDIR_DEBUG)/Core/Scene/Visibility.o $(OBJDIR_DEBUG)/Core/Scene/WavefrontSceneLoader.o $(OBJDIR_DEBUG)/Core/Shape/BasicMesh.o $(OBJDIR_DEBUG)/Core/Shape/BVHMesh.o $(OBJDIR_DEBUG)/Core/Shape/IndexedTriangle.o $(OBJDIR_DEBUG)/Core/Shape/KDTreeMesh.o $(OBJDIR_DEBUG)/Core/Shape/PersistentMesh.o $(OBJDIR_DEBUG)/Core/Shape/Sphere.o $(OBJDIR_DEBUG)/Core/Shape/TreeMesh.o $(OBJDIR_DEBUG)/Core/Shape/Triangle.o $(OBJDIR_DEBUG)/Core/Shape/TriangleMesh.o $(OBJDIR_DEBUG)/Core/Space/BasicSpace.o $(OBJDIR_DEBUG)/Core/System/EngineKernel.o $(OBJDIR_DEBUG)/Core/System/Lexer.o $(OBJDIR_DEBUG)/Core/System/LibraryManager.o $(OBJDIR_DEBUG)/Core/System/Platform.o $(OBJDIR_DEBUG)/Core/System/PlugInManager.o $(OBJDIR_DEBUG)/Core/System/Version.o $(OBJDIR_DEBUG)/Core/Threading/Spinlock.o 
OBJ_RELEASE = $(OBJDIR_RELEASE)/Core/Camera/Camera.o $(OBJDIR_RELEASE)/Core/Camera/PerspectiveCamera.o $(OBJDIR_RELEASE)/Core/Camera/ThinLensCamera.o $(OBJDIR_RELEASE)/Core/Device/DisplayDevice.o $(OBJDIR_RELEASE)/Core/Device/ImageDevice.o $(OBJDIR_RELEASE)/Core/Device/RTPDevice.o $(OBJDIR_RELEASE)/Core/Device/VideoDevice.o $(OBJDIR_RELEASE)/Core/Exception/Exception.o $(OBJDIR_RELEASE)/Core/External/Compression/lz4.o $(OBJDIR_RELEASE)/Core/External/Video/VideoStream.o $(OBJDIR_RELEASE)/Core/Filter/BoxFilter.o $(OBJDIR_RELEASE)/Core/Filter/TentFilter.o $(OBJDIR_RELEASE)/Core/Geometry/Basis.o $(OBJDIR_RELEASE)/Core/Geometry/BoundingBox.o $(OBJDIR_RELEASE)/Core/Geometry/Intersection.o $(OBJDIR_RELEASE)/Core/Geometry/Interval.o $(OBJDIR_RELEASE)/Core/Geometry/Matrix3x3.o $(OBJDIR_RELEASE)/Core/Geometry/Matrix4x4.o $(OBJDIR_RELEASE)/Core/Geometry/Plane.o $(OBJDIR_RELEASE)/Core/Geometry/Ray.o $(OBJDIR_RELEASE)/Core/Geometry/Transform.o $(OBJDIR_RELEASE)/Core/Geometry/Vector2.o $(OBJDIR_RELEASE)/Core/Geometry/Vector3.o $(OBJDIR_RELEASE)/Core/Image/Image.o $(OBJDIR_RELEASE)/Core/Image/RGBPixel.o $(OBJDIR_RELEASE)/Core/Integrator/IGIIntegrator.o $(OBJDIR_RELEASE)/Core/Integrator/Integrator.o $(OBJDIR_RELEASE)/Core/Integrator/PathIntegrator.o $(OBJDIR_RELEASE)/Core/Integrator/WhittedIntegrator.o $(OBJDIR_RELEASE)/Core/Integrator/TestIntegrator.o $(OBJDIR_RELEASE)/Core/Light/AreaLight.o $(OBJDIR_RELEASE)/Core/Light/DiffuseAreaLight.o $(OBJDIR_RELEASE)/Core/Light/InfiniteAreaLight.o $(OBJDIR_RELEASE)/Core/Light/PointLight.o $(OBJDIR_RELEASE)/Core/Material/BSDF.o $(OBJDIR_RELEASE)/Core/Material/LambertianBxDF.o $(OBJDIR_RELEASE)/Core/Material/SpecularReflectionBxDF.o $(OBJDIR_RELEASE)/Core/Material/SpecularTransmissionBxDF.o $(OBJDIR_RELEASE)/Core/Maths/Random.o $(OBJDIR_RELEASE)/Core/Maths/Maths.o $(OBJDIR_RELEASE)/Core/Object/Object.o $(OBJDIR_RELEASE)/Core/Object/UniqueID.o $(OBJDIR_RELEASE)/Core/Renderer/Renderer.o $(OBJDIR_RELEASE)/Core/Renderer/BaseRenderer.o $(OBJDIR_RELEASE)/Core/Renderer/BasicRenderer.o $(OBJDIR_RELEASE)/Core/Renderer/DistributedRenderer.o $(OBJDIR_RELEASE)/Core/Renderer/TimeConstrainedRenderer.o $(OBJDIR_RELEASE)/Core/Sampler/JitterSampler.o $(OBJDIR_RELEASE)/Core/Sampler/MultijitterSampler.o $(OBJDIR_RELEASE)/Core/Sampler/PrecomputationSampler.o $(OBJDIR_RELEASE)/Core/Sampler/RandomSampler.o $(OBJDIR_RELEASE)/Core/Sampler/SamplerDiagnostics.o $(OBJDIR_RELEASE)/Core/Scene/EmissivePrimitive.o $(OBJDIR_RELEASE)/Core/Scene/Environment.o $(OBJDIR_RELEASE)/Core/Scene/EnvironmentLoader.o $(OBJDIR_RELEASE)/Core/Scene/GeometricPrimitive.o $(OBJDIR_RELEASE)/Core/Scene/ParticleSceneLoader.o $(OBJDIR_RELEASE)/Core/Scene/Scene.o $(OBJDIR_RELEASE)/Core/Scene/SceneLoader.o $(OBJDIR_RELEASE)/Core/Scene/Visibility.o $(OBJDIR_RELEASE)/Core/Scene/WavefrontSceneLoader.o $(OBJDIR_RELEASE)/Core/Shape/BasicMesh.o $(OBJDIR_RELEASE)/Core/Shape/BVHMesh.o $(OBJDIR_RELEASE)/Core/Shape/IndexedTriangle.o $(OBJDIR_RELEASE)/Core/Shape/KDTreeMesh.o $(OBJDIR_RELEASE)/Core/Shape/PersistentMesh.o $(OBJDIR_RELEASE)/Core/Shape/Sphere.o $(OBJDIR_RELEASE)/Core/Shape/TreeMesh.o $(OBJDIR_RELEASE)/Core/Shape/Triangle.o $(OBJDIR_RELEASE)/Core/Shape/TriangleMesh.o $(OBJDIR_RELEASE)/Core/Space/BasicSpace.o $(OBJDIR_RELEASE)/Core/System/EngineKernel.o $(OBJDIR_RELEASE)/Core/System/Lexer.o $(OBJDIR_RELEASE)/Core/System/LibraryManager.o $(OBJDIR_RELEASE)/Core/System/Platform.o $(OBJDIR_RELEASE)/Core/System/PlugInManager.o $(OBJDIR_RELEASE)/Core/System/Version.o $(OBJDIR_RELEASE)/Core/Threading/Spinlock.o 

all: debug release

clean: clean_debug clean_release

debug: $(OUT_DEBUG)

$(OUT_DEBUG): $(OBJ_DEBUG) $(DEP_DEBUG)
	test -d ../../../../Bin/Linux/Debug || mkdir -p ../../../../Bin/Linux/Debug
	$(AR) rcs $(OUT_DEBUG) $(OBJ_DEBUG)
	$(AR) rcs $(OUT_DEBUG) $(OBJ_DEBUG)

#
#Camera
#
$(OBJDIR_DEBUG)/Core/Camera/Camera.o: $(SRCDIR_DEBUG)/Core/Camera/Camera.cpp
	test -d $(OBJDIR_DEBUG)/Core/Camera || mkdir -p $(OBJDIR_DEBUG)/Core/Camera
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Camera/Camera.o $(SRCDIR_DEBUG)/Core/Camera/Camera.cpp

$(OBJDIR_DEBUG)/Core/Camera/PerspectiveCamera.o: $(SRCDIR_DEBUG)/Core/Camera/PerspectiveCamera.cpp
	test -d $(OBJDIR_DEBUG)/Core/Camera || mkdir -p $(OBJDIR_DEBUG)/Core/Camera
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Camera/PerspectiveCamera.o $(SRCDIR_DEBUG)/Core/Camera/PerspectiveCamera.cpp

$(OBJDIR_DEBUG)/Core/Camera/ThinLensCamera.o: $(SRCDIR_DEBUG)/Core/Camera/ThinLensCamera.cpp
	test -d $(OBJDIR_DEBUG)/Core/Camera || mkdir -p $(OBJDIR_DEBUG)/Core/Camera
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Camera/ThinLensCamera.o $(SRCDIR_DEBUG)/Core/Camera/ThinLensCamera.cpp

#
# Device
#	
$(OBJDIR_DEBUG)/Core/Device/DisplayDevice.o: $(SRCDIR_DEBUG)/Core/Device/DisplayDevice.cpp
	test -d $(OBJDIR_DEBUG)/Core/Device || mkdir -p $(OBJDIR_DEBUG)/Core/Device
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Device/DisplayDevice.o $(SRCDIR_DEBUG)/Core/Device/DisplayDevice.cpp

$(OBJDIR_DEBUG)/Core/Device/ImageDevice.o: $(SRCDIR_DEBUG)/Core/Device/ImageDevice.cpp
	test -d $(OBJDIR_DEBUG)/Core/Device || mkdir -p $(OBJDIR_DEBUG)/Core/Device
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Device/ImageDevice.o $(SRCDIR_DEBUG)/Core/Device/ImageDevice.cpp

$(OBJDIR_DEBUG)/Core/Device/RTPDevice.o: $(SRCDIR_DEBUG)/Core/Device/RTPDevice.cpp
	test -d $(OBJDIR_DEBUG)/Core/Device || mkdir -p $(OBJDIR_DEBUG)/Core/Device
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Device/RTPDevice.o $(SRCDIR_DEBUG)/Core/Device/RTPDevice.cpp

$(OBJDIR_DEBUG)/Core/Device/VideoDevice.o: $(SRCDIR_DEBUG)/Core/Device/VideoDevice.cpp
	test -d $(OBJDIR_DEBUG)/Core/Device || mkdir -p $(OBJDIR_DEBUG)/Core/Device
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Device/VideoDevice.o $(SRCDIR_DEBUG)/Core/Device/VideoDevice.cpp

#
# Exception
#
$(OBJDIR_DEBUG)/Core/Exception/Exception.o: $(SRCDIR_DEBUG)/Core/Exception/Exception.cpp
	test -d $(OBJDIR_DEBUG)/Core/Exception || mkdir -p $(OBJDIR_DEBUG)/Core/Exception
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Exception/Exception.o $(SRCDIR_DEBUG)/Core/Exception/Exception.cpp

#
# External
#
$(OBJDIR_DEBUG)/Core/External/Compression/lz4.o: $(SRCDIR_DEBUG)/Core/External/Compression/lz4.c
	test -d $(OBJDIR_DEBUG)/Core/External/Compression || mkdir -p $(OBJDIR_DEBUG)/Core/External/Compression
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/External/Compression/lz4.o $(SRCDIR_DEBUG)/Core/External/Compression/lz4.c

$(OBJDIR_DEBUG)/Core/External/Video/VideoStream.o: $(SRCDIR_DEBUG)/Core/External/Video/VideoStream.cpp
	test -d $(OBJDIR_DEBUG)/Core/External/Video || mkdir -p $(OBJDIR_DEBUG)/Core/External/Video
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/External/Video/VideoStream.o $(SRCDIR_DEBUG)/Core/External/Video/VideoStream.cpp

#
# Filter
#	
$(OBJDIR_DEBUG)/Core/Filter/BoxFilter.o: $(SRCDIR_DEBUG)/Core/Filter/BoxFilter.cpp
	test -d $(OBJDIR_DEBUG)/Core/Filter || mkdir -p $(OBJDIR_DEBUG)/Core/Filter
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Filter/BoxFilter.o $(SRCDIR_DEBUG)/Core/Filter/BoxFilter.cpp

$(OBJDIR_DEBUG)/Core/Filter/TentFilter.o: $(SRCDIR_DEBUG)/Core/Filter/TentFilter.cpp
	test -d $(OBJDIR_DEBUG)/Core/Filter || mkdir -p $(OBJDIR_DEBUG)/Core/Filter
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Filter/TentFilter.o $(SRCDIR_DEBUG)/Core/Filter/TentFilter.cpp

#
# Geometry
#
$(OBJDIR_DEBUG)/Core/Geometry/Basis.o: $(SRCDIR_DEBUG)/Core/Geometry/Basis.cpp
	test -d $(OBJDIR_DEBUG)/Core/Geometry || mkdir -p $(OBJDIR_DEBUG)/Core/Geometry
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Geometry/Basis.o $(SRCDIR_DEBUG)/Core/Geometry/Basis.cpp

$(OBJDIR_DEBUG)/Core/Geometry/BoundingBox.o: $(SRCDIR_DEBUG)/Core/Geometry/BoundingBox.cpp
	test -d $(OBJDIR_DEBUG)/Core/Geometry || mkdir -p $(OBJDIR_DEBUG)/Core/Geometry
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Geometry/BoundingBox.o $(SRCDIR_DEBUG)/Core/Geometry/BoundingBox.cpp

$(OBJDIR_DEBUG)/Core/Geometry/Intersection.o: $(SRCDIR_DEBUG)/Core/Geometry/Intersection.cpp
	test -d $(OBJDIR_DEBUG)/Core/Geometry || mkdir -p $(OBJDIR_DEBUG)/Core/Geometry
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Geometry/Intersection.o $(SRCDIR_DEBUG)/Core/Geometry/Intersection.cpp

$(OBJDIR_DEBUG)/Core/Geometry/Interval.o: $(SRCDIR_DEBUG)/Core/Geometry/Interval.cpp
	test -d $(OBJDIR_DEBUG)/Core/Geometry || mkdir -p $(OBJDIR_DEBUG)/Core/Geometry
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Geometry/Interval.o $(SRCDIR_DEBUG)/Core/Geometry/Interval.cpp

$(OBJDIR_DEBUG)/Core/Geometry/Matrix3x3.o: $(SRCDIR_DEBUG)/Core/Geometry/Matrix3x3.cpp
	test -d $(OBJDIR_DEBUG)/Core/Geometry || mkdir -p $(OBJDIR_DEBUG)/Core/Geometry
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Geometry/Matrix3x3.o $(SRCDIR_DEBUG)/Core/Geometry/Matrix3x3.cpp

$(OBJDIR_DEBUG)/Core/Geometry/Matrix4x4.o: $(SRCDIR_DEBUG)/Core/Geometry/Matrix4x4.cpp
	test -d $(OBJDIR_DEBUG)/Core/Geometry || mkdir -p $(OBJDIR_DEBUG)/Core/Geometry
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Geometry/Matrix4x4.o $(SRCDIR_DEBUG)/Core/Geometry/Matrix4x4.cpp

$(OBJDIR_DEBUG)/Core/Geometry/Plane.o: $(SRCDIR_DEBUG)/Core/Geometry/Plane.cpp
	test -d $(OBJDIR_DEBUG)/Core/Geometry || mkdir -p $(OBJDIR_DEBUG)/Core/Geometry
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Geometry/Plane.o $(SRCDIR_DEBUG)/Core/Geometry/Plane.cpp

$(OBJDIR_DEBUG)/Core/Geometry/Ray.o: $(SRCDIR_DEBUG)/Core/Geometry/Ray.cpp
	test -d $(OBJDIR_DEBUG)/Core/Geometry || mkdir -p $(OBJDIR_DEBUG)/Core/Geometry
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Geometry/Ray.o $(SRCDIR_DEBUG)/Core/Geometry/Ray.cpp

$(OBJDIR_DEBUG)/Core/Geometry/Transform.o: $(SRCDIR_DEBUG)/Core/Geometry/Transform.cpp
	test -d $(OBJDIR_DEBUG)/Core/Geometry || mkdir -p $(OBJDIR_DEBUG)/Core/Geometry
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Geometry/Transform.o $(SRCDIR_DEBUG)/Core/Geometry/Transform.cpp

$(OBJDIR_DEBUG)/Core/Geometry/Vector2.o: $(SRCDIR_DEBUG)/Core/Geometry/Vector2.cpp
	test -d $(OBJDIR_DEBUG)/Core/Geometry || mkdir -p $(OBJDIR_DEBUG)/Core/Geometry
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Geometry/Vector2.o $(SRCDIR_DEBUG)/Core/Geometry/Vector2.cpp

$(OBJDIR_DEBUG)/Core/Geometry/Vector3.o: $(SRCDIR_DEBUG)/Core/Geometry/Vector3.cpp
	test -d $(OBJDIR_DEBUG)/Core/Geometry || mkdir -p $(OBJDIR_DEBUG)/Core/Geometry
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Geometry/Vector3.o $(SRCDIR_DEBUG)/Core/Geometry/Vector3.cpp
	
#
# Image
#
$(OBJDIR_DEBUG)/Core/Image/Image.o: $(SRCDIR_DEBUG)/Core/Image/Image.cpp
	test -d $(OBJDIR_DEBUG)/Core/Image || mkdir -p $(OBJDIR_DEBUG)/Core/Image
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Image/Image.o $(SRCDIR_DEBUG)/Core/Image/Image.cpp

$(OBJDIR_DEBUG)/Core/Image/RGBPixel.o: $(SRCDIR_DEBUG)/Core/Image/RGBPixel.cpp
	test -d $(OBJDIR_DEBUG)/Core/Image || mkdir -p $(OBJDIR_DEBUG)/Core/Image
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Image/RGBPixel.o $(SRCDIR_DEBUG)/Core/Image/RGBPixel.cpp

#
# Integrator
#
$(OBJDIR_DEBUG)/Core/Integrator/Integrator.o: $(SRCDIR_DEBUG)/Core/Integrator/Integrator.cpp
	test -d $(OBJDIR_DEBUG)/Core/Integrator || mkdir -p $(OBJDIR_DEBUG)/Core/Integrator
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Integrator/Integrator.o $(SRCDIR_DEBUG)/Core/Integrator/Integrator.cpp
		
$(OBJDIR_DEBUG)/Core/Integrator/PathIntegrator.o: $(SRCDIR_DEBUG)/Core/Integrator/PathIntegrator.cpp
	test -d $(OBJDIR_DEBUG)/Core/Integrator || mkdir -p $(OBJDIR_DEBUG)/Core/Integrator
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Integrator/PathIntegrator.o $(SRCDIR_DEBUG)/Core/Integrator/PathIntegrator.cpp

$(OBJDIR_DEBUG)/Core/Integrator/WhittedIntegrator.o: $(SRCDIR_DEBUG)/Core/Integrator/WhittedIntegrator.cpp
	test -d $(OBJDIR_DEBUG)/Core/Integrator || mkdir -p $(OBJDIR_DEBUG)/Core/Integrator
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Integrator/WhittedIntegrator.o $(SRCDIR_DEBUG)/Core/Integrator/WhittedIntegrator.cpp

$(OBJDIR_DEBUG)/Core/Integrator/IGIIntegrator.o: $(SRCDIR_DEBUG)/Core/Integrator/IGIIntegrator.cpp
	test -d $(OBJDIR_DEBUG)/Core/Integrator || mkdir -p $(OBJDIR_DEBUG)/Core/Integrator
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Integrator/IGIIntegrator.o $(SRCDIR_DEBUG)/Core/Integrator/IGIIntegrator.cpp

$(OBJDIR_DEBUG)/Core/Integrator/TestIntegrator.o: $(SRCDIR_DEBUG)/Core/Integrator/TestIntegrator.cpp
	test -d $(OBJDIR_DEBUG)/Core/Integrator || mkdir -p $(OBJDIR_DEBUG)/Core/Integrator
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Integrator/TestIntegrator.o $(SRCDIR_DEBUG)/Core/Integrator/TestIntegrator.cpp

#
# Light
#
$(OBJDIR_DEBUG)/Core/Light/AreaLight.o: $(SRCDIR_DEBUG)/Core/Light/AreaLight.cpp
	test -d $(OBJDIR_DEBUG)/Core/Light || mkdir -p $(OBJDIR_DEBUG)/Core/Light
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Light/AreaLight.o $(SRCDIR_DEBUG)/Core/Light/AreaLight.cpp

$(OBJDIR_DEBUG)/Core/Light/DiffuseAreaLight.o: $(SRCDIR_DEBUG)/Core/Light/DiffuseAreaLight.cpp
	test -d $(OBJDIR_DEBUG)/Core/Light || mkdir -p $(OBJDIR_DEBUG)/Core/Light
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Light/DiffuseAreaLight.o $(SRCDIR_DEBUG)/Core/Light/DiffuseAreaLight.cpp

$(OBJDIR_DEBUG)/Core/Light/InfiniteAreaLight.o: $(SRCDIR_DEBUG)/Core/Light/InfiniteAreaLight.cpp
	test -d $(OBJDIR_DEBUG)/Core/Light || mkdir -p $(OBJDIR_DEBUG)/Core/Light
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Light/InfiniteAreaLight.o $(SRCDIR_DEBUG)/Core/Light/InfiniteAreaLight.cpp

$(OBJDIR_DEBUG)/Core/Light/PointLight.o: $(SRCDIR_DEBUG)/Core/Light/PointLight.cpp
	test -d $(OBJDIR_DEBUG)/Core/Light || mkdir -p $(OBJDIR_DEBUG)/Core/Light
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Light/PointLight.o $(SRCDIR_DEBUG)/Core/Light/PointLight.cpp

#
# Material
#
$(OBJDIR_DEBUG)/Core/Material/BSDF.o: $(SRCDIR_DEBUG)/Core/Material/BSDF.cpp
	test -d $(OBJDIR_DEBUG)/Core/Material || mkdir -p $(OBJDIR_DEBUG)/Core/Material
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Material/BSDF.o $(SRCDIR_DEBUG)/Core/Material/BSDF.cpp

$(OBJDIR_DEBUG)/Core/Material/LambertianBxDF.o: $(SRCDIR_DEBUG)/Core/Material/LambertianBxDF.cpp
	test -d $(OBJDIR_DEBUG)/Core/Material || mkdir -p $(OBJDIR_DEBUG)/Core/Material
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Material/LambertianBxDF.o $(SRCDIR_DEBUG)/Core/Material/LambertianBxDF.cpp

$(OBJDIR_DEBUG)/Core/Material/SpecularReflectionBxDF.o: $(SRCDIR_DEBUG)/Core/Material/SpecularReflectionBxDF.cpp
	test -d $(OBJDIR_DEBUG)/Core/Material || mkdir -p $(OBJDIR_DEBUG)/Core/Material
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Material/SpecularReflectionBxDF.o $(SRCDIR_DEBUG)/Core/Material/SpecularReflectionBxDF.cpp

$(OBJDIR_DEBUG)/Core/Material/SpecularTransmissionBxDF.o: $(SRCDIR_DEBUG)/Core/Material/SpecularTransmissionBxDF.cpp
	test -d $(OBJDIR_DEBUG)/Core/Material || mkdir -p $(OBJDIR_DEBUG)/Core/Material
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Material/SpecularTransmissionBxDF.o $(SRCDIR_DEBUG)/Core/Material/SpecularTransmissionBxDF.cpp

#
# Maths
#	
$(OBJDIR_DEBUG)/Core/Maths/Random.o: $(SRCDIR_DEBUG)/Core/Maths/Random.cpp
	test -d $(OBJDIR_DEBUG)/Core/Maths || mkdir -p $(OBJDIR_DEBUG)/Core/Maths
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Maths/Random.o $(SRCDIR_DEBUG)/Core/Maths/Random.cpp

$(OBJDIR_DEBUG)/Core/Maths/Maths.o: $(SRCDIR_DEBUG)/Core/Maths/Maths.cpp
	test -d $(OBJDIR_DEBUG)/Core/Maths || mkdir -p $(OBJDIR_DEBUG)/Core/Maths
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Maths/Maths.o $(SRCDIR_DEBUG)/Core/Maths/Maths.cpp

#
# Object
#
$(OBJDIR_DEBUG)/Core/Object/Object.o: $(SRCDIR_DEBUG)/Core/Object/Object.cpp
	test -d $(OBJDIR_DEBUG)/Core/Object || mkdir -p $(OBJDIR_DEBUG)/Core/Object
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Object/Object.o $(SRCDIR_DEBUG)/Core/Object/Object.cpp

$(OBJDIR_DEBUG)/Core/Object/UniqueID.o: $(SRCDIR_DEBUG)/Core/Object/UniqueID.cpp
	test -d $(OBJDIR_DEBUG)/Core/Object || mkdir -p $(OBJDIR_DEBUG)/Core/Object
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Object/UniqueID.o $(SRCDIR_DEBUG)/Core/Object/UniqueID.cpp

#
# Postproc
#	

#
# Renderer
#
$(OBJDIR_DEBUG)/Core/Renderer/BaseRenderer.o: $(SRCDIR_DEBUG)/Core/Renderer/BaseRenderer.cpp
	test -d $(OBJDIR_DEBUG)/Core/Renderer || mkdir -p $(OBJDIR_DEBUG)/Core/Renderer
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Renderer/BaseRenderer.o $(SRCDIR_DEBUG)/Core/Renderer/BaseRenderer.cpp

$(OBJDIR_DEBUG)/Core/Renderer/BasicRenderer.o: $(SRCDIR_DEBUG)/Core/Renderer/BasicRenderer.cpp
	test -d $(OBJDIR_DEBUG)/Core/Renderer || mkdir -p $(OBJDIR_DEBUG)/Core/Renderer
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Renderer/BasicRenderer.o $(SRCDIR_DEBUG)/Core/Renderer/BasicRenderer.cpp

$(OBJDIR_DEBUG)/Core/Renderer/DistributedRenderer.o: $(SRCDIR_DEBUG)/Core/Renderer/DistributedRenderer.cpp
	test -d $(OBJDIR_DEBUG)/Core/Renderer || mkdir -p $(OBJDIR_DEBUG)/Core/Renderer
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Renderer/DistributedRenderer.o $(SRCDIR_DEBUG)/Core/Renderer/DistributedRenderer.cpp

$(OBJDIR_DEBUG)/Core/Renderer/Renderer.o: $(SRCDIR_DEBUG)/Core/Renderer/Renderer.cpp
	test -d $(OBJDIR_DEBUG)/Core/Renderer || mkdir -p $(OBJDIR_DEBUG)/Core/Renderer
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Renderer/Renderer.o $(SRCDIR_DEBUG)/Core/Renderer/Renderer.cpp
		
$(OBJDIR_DEBUG)/Core/Renderer/TimeConstrainedRenderer.o: $(SRCDIR_DEBUG)/Core/Renderer/TimeConstrainedRenderer.cpp
	test -d $(OBJDIR_DEBUG)/Core/Renderer || mkdir -p $(OBJDIR_DEBUG)/Core/Renderer
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Renderer/TimeConstrainedRenderer.o $(SRCDIR_DEBUG)/Core/Renderer/TimeConstrainedRenderer.cpp

#
# Sampler
#
$(OBJDIR_DEBUG)/Core/Sampler/JitterSampler.o: $(SRCDIR_DEBUG)/Core/Sampler/JitterSampler.cpp
	test -d $(OBJDIR_DEBUG)/Core/Sampler || mkdir -p $(OBJDIR_DEBUG)/Core/Sampler
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Sampler/JitterSampler.o $(SRCDIR_DEBUG)/Core/Sampler/JitterSampler.cpp

$(OBJDIR_DEBUG)/Core/Sampler/MultijitterSampler.o: $(SRCDIR_DEBUG)/Core/Sampler/MultijitterSampler.cpp
	test -d $(OBJDIR_DEBUG)/Core/Sampler || mkdir -p $(OBJDIR_DEBUG)/Core/Sampler
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Sampler/MultijitterSampler.o $(SRCDIR_DEBUG)/Core/Sampler/MultijitterSampler.cpp

$(OBJDIR_DEBUG)/Core/Sampler/PrecomputationSampler.o: $(SRCDIR_DEBUG)/Core/Sampler/PrecomputationSampler.cpp
	test -d $(OBJDIR_DEBUG)/Core/Sampler || mkdir -p $(OBJDIR_DEBUG)/Core/Sampler
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Sampler/PrecomputationSampler.o $(SRCDIR_DEBUG)/Core/Sampler/PrecomputationSampler.cpp

$(OBJDIR_DEBUG)/Core/Sampler/RandomSampler.o: $(SRCDIR_DEBUG)/Core/Sampler/RandomSampler.cpp
	test -d $(OBJDIR_DEBUG)/Core/Sampler || mkdir -p $(OBJDIR_DEBUG)/Core/Sampler
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Sampler/RandomSampler.o $(SRCDIR_DEBUG)/Core/Sampler/RandomSampler.cpp

$(OBJDIR_DEBUG)/Core/Sampler/SamplerDiagnostics.o: $(SRCDIR_DEBUG)/Core/Sampler/SamplerDiagnostics.cpp
	test -d $(OBJDIR_DEBUG)/Core/Sampler || mkdir -p $(OBJDIR_DEBUG)/Core/Sampler
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Sampler/SamplerDiagnostics.o $(SRCDIR_DEBUG)/Core/Sampler/SamplerDiagnostics.cpp

#
# Scene
#
$(OBJDIR_DEBUG)/Core/Scene/EmissivePrimitive.o: $(SRCDIR_DEBUG)/Core/Scene/EmissivePrimitive.cpp
	test -d $(OBJDIR_DEBUG)/Core/Scene || mkdir -p $(OBJDIR_DEBUG)/Core/Scene
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Scene/EmissivePrimitive.o $(SRCDIR_DEBUG)/Core/Scene/EmissivePrimitive.cpp

$(OBJDIR_DEBUG)/Core/Scene/Environment.o: $(SRCDIR_DEBUG)/Core/Scene/Environment.cpp
	test -d $(OBJDIR_DEBUG)/Core/Scene || mkdir -p $(OBJDIR_DEBUG)/Core/Scene
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Scene/Environment.o $(SRCDIR_DEBUG)/Core/Scene/Environment.cpp

$(OBJDIR_DEBUG)/Core/Scene/EnvironmentLoader.o: $(SRCDIR_DEBUG)/Core/Scene/EnvironmentLoader.cpp
	test -d $(OBJDIR_DEBUG)/Core/Scene || mkdir -p $(OBJDIR_DEBUG)/Core/Scene
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Scene/EnvironmentLoader.o $(SRCDIR_DEBUG)/Core/Scene/EnvironmentLoader.cpp

$(OBJDIR_DEBUG)/Core/Scene/GeometricPrimitive.o: $(SRCDIR_DEBUG)/Core/Scene/GeometricPrimitive.cpp
	test -d $(OBJDIR_DEBUG)/Core/Scene || mkdir -p $(OBJDIR_DEBUG)/Core/Scene
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Scene/GeometricPrimitive.o $(SRCDIR_DEBUG)/Core/Scene/GeometricPrimitive.cpp

$(OBJDIR_DEBUG)/Core/Scene/ParticleSceneLoader.o: $(SRCDIR_DEBUG)/Core/Scene/ParticleSceneLoader.cpp
	test -d $(OBJDIR_DEBUG)/Core/Scene || mkdir -p $(OBJDIR_DEBUG)/Core/Scene
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Scene/ParticleSceneLoader.o $(SRCDIR_DEBUG)/Core/Scene/ParticleSceneLoader.cpp	
	
$(OBJDIR_DEBUG)/Core/Scene/Scene.o: $(SRCDIR_DEBUG)/Core/Scene/Scene.cpp
	test -d $(OBJDIR_DEBUG)/Core/Scene || mkdir -p $(OBJDIR_DEBUG)/Core/Scene
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Scene/Scene.o $(SRCDIR_DEBUG)/Core/Scene/Scene.cpp

$(OBJDIR_DEBUG)/Core/Scene/SceneLoader.o: $(SRCDIR_DEBUG)/Core/Scene/SceneLoader.cpp
	test -d $(OBJDIR_DEBUG)/Core/Scene || mkdir -p $(OBJDIR_DEBUG)/Core/Scene
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Scene/SceneLoader.o $(SRCDIR_DEBUG)/Core/Scene/SceneLoader.cpp

$(OBJDIR_DEBUG)/Core/Scene/Visibility.o: $(SRCDIR_DEBUG)/Core/Scene/Visibility.cpp
	test -d $(OBJDIR_DEBUG)/Core/Scene || mkdir -p $(OBJDIR_DEBUG)/Core/Scene
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Scene/Visibility.o $(SRCDIR_DEBUG)/Core/Scene/Visibility.cpp

$(OBJDIR_DEBUG)/Core/Scene/WavefrontSceneLoader.o: $(SRCDIR_DEBUG)/Core/Scene/WavefrontSceneLoader.cpp
	test -d $(OBJDIR_DEBUG)/Core/Scene || mkdir -p $(OBJDIR_DEBUG)/Core/Scene
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Scene/WavefrontSceneLoader.o $(SRCDIR_DEBUG)/Core/Scene/WavefrontSceneLoader.cpp

#
# Shape
#
$(OBJDIR_DEBUG)/Core/Shape/BasicMesh.o: $(SRCDIR_DEBUG)/Core/Shape/BasicMesh.cpp
	test -d $(OBJDIR_DEBUG)/Core/Shape || mkdir -p $(OBJDIR_DEBUG)/Core/Shape
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Shape/BasicMesh.o $(SRCDIR_DEBUG)/Core/Shape/BasicMesh.cpp

$(OBJDIR_DEBUG)/Core/Shape/BVHMesh.o: $(SRCDIR_DEBUG)/Core/Shape/BVHMesh.cpp
	test -d $(OBJDIR_DEBUG)/Core/Shape || mkdir -p $(OBJDIR_DEBUG)/Core/Shape
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Shape/BVHMesh.o $(SRCDIR_DEBUG)/Core/Shape/BVHMesh.cpp
	
$(OBJDIR_DEBUG)/Core/Shape/IndexedTriangle.o: $(SRCDIR_DEBUG)/Core/Shape/IndexedTriangle.cpp
	test -d $(OBJDIR_DEBUG)/Core/Shape || mkdir -p $(OBJDIR_DEBUG)/Core/Shape
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Shape/IndexedTriangle.o $(SRCDIR_DEBUG)/Core/Shape/IndexedTriangle.cpp

$(OBJDIR_DEBUG)/Core/Shape/KDTreeMesh.o: $(SRCDIR_DEBUG)/Core/Shape/KDTreeMesh.cpp
	test -d $(OBJDIR_DEBUG)/Core/Shape || mkdir -p $(OBJDIR_DEBUG)/Core/Shape
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Shape/KDTreeMesh.o $(SRCDIR_DEBUG)/Core/Shape/KDTreeMesh.cpp

$(OBJDIR_DEBUG)/Core/Shape/PersistentMesh.o: $(SRCDIR_DEBUG)/Core/Shape/PersistentMesh.cpp
	test -d $(OBJDIR_DEBUG)/Core/Shape || mkdir -p $(OBJDIR_DEBUG)/Core/Shape
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Shape/PersistentMesh.o $(SRCDIR_DEBUG)/Core/Shape/PersistentMesh.cpp
	
$(OBJDIR_DEBUG)/Core/Shape/Sphere.o: $(SRCDIR_DEBUG)/Core/Shape/Sphere.cpp
	test -d $(OBJDIR_DEBUG)/Core/Shape || mkdir -p $(OBJDIR_DEBUG)/Core/Shape
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Shape/Sphere.o $(SRCDIR_DEBUG)/Core/Shape/Sphere.cpp

$(OBJDIR_DEBUG)/Core/Shape/TreeMesh.o: $(SRCDIR_DEBUG)/Core/Shape/TreeMesh.cpp
	test -d $(OBJDIR_DEBUG)/Core/Shape || mkdir -p $(OBJDIR_DEBUG)/Core/Shape
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Shape/TreeMesh.o $(SRCDIR_DEBUG)/Core/Shape/TreeMesh.cpp

$(OBJDIR_DEBUG)/Core/Shape/Triangle.o: $(SRCDIR_DEBUG)/Core/Shape/Triangle.cpp
	test -d $(OBJDIR_DEBUG)/Core/Shape || mkdir -p $(OBJDIR_DEBUG)/Core/Shape
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Shape/Triangle.o $(SRCDIR_DEBUG)/Core/Shape/Triangle.cpp

$(OBJDIR_DEBUG)/Core/Shape/TriangleMesh.o: $(SRCDIR_DEBUG)/Core/Shape/TriangleMesh.cpp
	test -d $(OBJDIR_DEBUG)/Core/Shape || mkdir -p $(OBJDIR_DEBUG)/Core/Shape
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Shape/TriangleMesh.o $(SRCDIR_DEBUG)/Core/Shape/TriangleMesh.cpp

#
# Space
#
$(OBJDIR_DEBUG)/Core/Space/BasicSpace.o: $(SRCDIR_DEBUG)/Core/Space/BasicSpace.cpp
	test -d $(OBJDIR_DEBUG)/Core/Space || mkdir -p $(OBJDIR_DEBUG)/Core/Space
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Space/BasicSpace.o $(SRCDIR_DEBUG)/Core/Space/BasicSpace.cpp

#
# System
#	
$(OBJDIR_DEBUG)/Core/System/EngineKernel.o: $(SRCDIR_DEBUG)/Core/System/EngineKernel.cpp
	test -d $(OBJDIR_DEBUG)/Core/System || mkdir -p $(OBJDIR_DEBUG)/Core/System
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/System/EngineKernel.o $(SRCDIR_DEBUG)/Core/System/EngineKernel.cpp

$(OBJDIR_DEBUG)/Core/System/Lexer.o: $(SRCDIR_DEBUG)/Core/System/Lexer.cpp
	test -d $(OBJDIR_DEBUG)/Core/System || mkdir -p $(OBJDIR_DEBUG)/Core/System
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/System/Lexer.o $(SRCDIR_DEBUG)/Core/System/Lexer.cpp
	
$(OBJDIR_DEBUG)/Core/System/LibraryManager.o: $(SRCDIR_DEBUG)/Core/System/LibraryManager.cpp
	test -d $(OBJDIR_DEBUG)/Core/System || mkdir -p $(OBJDIR_DEBUG)/Core/System
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/System/LibraryManager.o $(SRCDIR_DEBUG)/Core/System/LibraryManager.cpp

$(OBJDIR_DEBUG)/Core/System/Platform.o: $(SRCDIR_DEBUG)/Core/System/Platform.cpp
	test -d $(OBJDIR_DEBUG)/Core/System || mkdir -p $(OBJDIR_DEBUG)/Core/System
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/System/Platform.o $(SRCDIR_DEBUG)/Core/System/Platform.cpp

$(OBJDIR_DEBUG)/Core/System/PlugInManager.o: $(SRCDIR_DEBUG)/Core/System/PlugInManager.cpp
	test -d $(OBJDIR_DEBUG)/Core/System || mkdir -p $(OBJDIR_DEBUG)/Core/System
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/System/PlugInManager.o $(SRCDIR_DEBUG)/Core/System/PlugInManager.cpp

$(OBJDIR_DEBUG)/Core/System/Version.o: $(SRCDIR_DEBUG)/Core/System/Version.cpp
	test -d $(OBJDIR_DEBUG)/Core/System || mkdir -p $(OBJDIR_DEBUG)/Core/System
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/System/Version.o $(SRCDIR_DEBUG)/Core/System/Version.cpp

#
# Texture
# 	

#
# Threading
#

$(OBJDIR_DEBUG)/Core/Threading/Spinlock.o: $(SRCDIR_DEBUG)/Core/Threading/Spinlock.cpp
	test -d $(OBJDIR_DEBUG)/Core/Threading || mkdir -p $(OBJDIR_DEBUG)/Core/Threading
	$(CPP) $(CFLAGS_DEBUG) $(INC_DEBUG) -c -o $(OBJDIR_DEBUG)/Core/Threading/Spinlock.o $(SRCDIR_DEBUG)/Core/Threading/Spinlock.cpp

clean_debug:
	rm -f $(OBJ_DEBUG) $(OUT_DEBUG)

##
## Release
##	
	
release: $(OUT_RELEASE)

$(OUT_RELEASE): $(OBJ_RELEASE) $(DEP_RELEASE)
	test -d ../../../../Bin/Linux/Release || mkdir -p ../../../../Bin/Linux/Release
	$(AR) rcs $(OUT_RELEASE) $(OBJ_RELEASE)
	$(AR) rcs $(OUT_RELEASE) $(OBJ_RELEASE)

#
#Camera
#
$(OBJDIR_RELEASE)/Core/Camera/Camera.o: $(SRCDIR_RELEASE)/Core/Camera/Camera.cpp
	test -d $(OBJDIR_RELEASE)/Core/Camera || mkdir -p $(OBJDIR_RELEASE)/Core/Camera
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Camera/Camera.o $(SRCDIR_RELEASE)/Core/Camera/Camera.cpp

$(OBJDIR_RELEASE)/Core/Camera/PerspectiveCamera.o: $(SRCDIR_RELEASE)/Core/Camera/PerspectiveCamera.cpp
	test -d $(OBJDIR_RELEASE)/Core/Camera || mkdir -p $(OBJDIR_RELEASE)/Core/Camera
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Camera/PerspectiveCamera.o $(SRCDIR_RELEASE)/Core/Camera/PerspectiveCamera.cpp

$(OBJDIR_RELEASE)/Core/Camera/ThinLensCamera.o: $(SRCDIR_RELEASE)/Core/Camera/ThinLensCamera.cpp
	test -d $(OBJDIR_RELEASE)/Core/Camera || mkdir -p $(OBJDIR_RELEASE)/Core/Camera
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Camera/ThinLensCamera.o $(SRCDIR_RELEASE)/Core/Camera/ThinLensCamera.cpp

#
# Device
#	
$(OBJDIR_RELEASE)/Core/Device/DisplayDevice.o: $(SRCDIR_RELEASE)/Core/Device/DisplayDevice.cpp
	test -d $(OBJDIR_RELEASE)/Core/Device || mkdir -p $(OBJDIR_RELEASE)/Core/Device
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Device/DisplayDevice.o $(SRCDIR_RELEASE)/Core/Device/DisplayDevice.cpp

$(OBJDIR_RELEASE)/Core/Device/ImageDevice.o: $(SRCDIR_RELEASE)/Core/Device/ImageDevice.cpp
	test -d $(OBJDIR_RELEASE)/Core/Device || mkdir -p $(OBJDIR_RELEASE)/Core/Device
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Device/ImageDevice.o $(SRCDIR_RELEASE)/Core/Device/ImageDevice.cpp

$(OBJDIR_RELEASE)/Core/Device/RTPDevice.o: $(SRCDIR_RELEASE)/Core/Device/RTPDevice.cpp
	test -d $(OBJDIR_RELEASE)/Core/Device || mkdir -p $(OBJDIR_RELEASE)/Core/Device
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Device/RTPDevice.o $(SRCDIR_RELEASE)/Core/Device/RTPDevice.cpp

$(OBJDIR_RELEASE)/Core/Device/VideoDevice.o: $(SRCDIR_RELEASE)/Core/Device/VideoDevice.cpp
	test -d $(OBJDIR_RELEASE)/Core/Device || mkdir -p $(OBJDIR_RELEASE)/Core/Device
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Device/VideoDevice.o $(SRCDIR_RELEASE)/Core/Device/VideoDevice.cpp

#
# Exception
#
$(OBJDIR_RELEASE)/Core/Exception/Exception.o: $(SRCDIR_RELEASE)/Core/Exception/Exception.cpp
	test -d $(OBJDIR_RELEASE)/Core/Exception || mkdir -p $(OBJDIR_RELEASE)/Core/Exception
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Exception/Exception.o $(SRCDIR_RELEASE)/Core/Exception/Exception.cpp

#
# External
#
$(OBJDIR_RELEASE)/Core/External/Compression/lz4.o: $(SRCDIR_RELEASE)/Core/External/Compression/lz4.c
	test -d $(OBJDIR_RELEASE)/Core/External/Compression || mkdir -p $(OBJDIR_RELEASE)/Core/External/Compression
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/External/Compression/lz4.o $(SRCDIR_RELEASE)/Core/External/Compression/lz4.c

$(OBJDIR_RELEASE)/Core/External/Video/VideoStream.o: $(SRCDIR_RELEASE)/Core/External/Video/VideoStream.cpp
	test -d $(OBJDIR_RELEASE)/Core/External/Video || mkdir -p $(OBJDIR_RELEASE)/Core/External/Video
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/External/Video/VideoStream.o $(SRCDIR_RELEASE)/Core/External/Video/VideoStream.cpp

#
# Filter
#	
$(OBJDIR_RELEASE)/Core/Filter/BoxFilter.o: $(SRCDIR_RELEASE)/Core/Filter/BoxFilter.cpp
	test -d $(OBJDIR_RELEASE)/Core/Filter || mkdir -p $(OBJDIR_RELEASE)/Core/Filter
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Filter/BoxFilter.o $(SRCDIR_RELEASE)/Core/Filter/BoxFilter.cpp

$(OBJDIR_RELEASE)/Core/Filter/TentFilter.o: $(SRCDIR_RELEASE)/Core/Filter/TentFilter.cpp
	test -d $(OBJDIR_RELEASE)/Core/Filter || mkdir -p $(OBJDIR_RELEASE)/Core/Filter
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Filter/TentFilter.o $(SRCDIR_RELEASE)/Core/Filter/TentFilter.cpp

#
# Geometry
#
$(OBJDIR_RELEASE)/Core/Geometry/Basis.o: $(SRCDIR_RELEASE)/Core/Geometry/Basis.cpp
	test -d $(OBJDIR_RELEASE)/Core/Geometry || mkdir -p $(OBJDIR_RELEASE)/Core/Geometry
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Geometry/Basis.o $(SRCDIR_RELEASE)/Core/Geometry/Basis.cpp

$(OBJDIR_RELEASE)/Core/Geometry/BoundingBox.o: $(SRCDIR_RELEASE)/Core/Geometry/BoundingBox.cpp
	test -d $(OBJDIR_RELEASE)/Core/Geometry || mkdir -p $(OBJDIR_RELEASE)/Core/Geometry
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Geometry/BoundingBox.o $(SRCDIR_RELEASE)/Core/Geometry/BoundingBox.cpp

$(OBJDIR_RELEASE)/Core/Geometry/Intersection.o: $(SRCDIR_RELEASE)/Core/Geometry/Intersection.cpp
	test -d $(OBJDIR_RELEASE)/Core/Geometry || mkdir -p $(OBJDIR_RELEASE)/Core/Geometry
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Geometry/Intersection.o $(SRCDIR_RELEASE)/Core/Geometry/Intersection.cpp

$(OBJDIR_RELEASE)/Core/Geometry/Interval.o: $(SRCDIR_RELEASE)/Core/Geometry/Interval.cpp
	test -d $(OBJDIR_RELEASE)/Core/Geometry || mkdir -p $(OBJDIR_RELEASE)/Core/Geometry
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Geometry/Interval.o $(SRCDIR_RELEASE)/Core/Geometry/Interval.cpp

$(OBJDIR_RELEASE)/Core/Geometry/Matrix3x3.o: $(SRCDIR_RELEASE)/Core/Geometry/Matrix3x3.cpp
	test -d $(OBJDIR_RELEASE)/Core/Geometry || mkdir -p $(OBJDIR_RELEASE)/Core/Geometry
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Geometry/Matrix3x3.o $(SRCDIR_RELEASE)/Core/Geometry/Matrix3x3.cpp

$(OBJDIR_RELEASE)/Core/Geometry/Matrix4x4.o: $(SRCDIR_RELEASE)/Core/Geometry/Matrix4x4.cpp
	test -d $(OBJDIR_RELEASE)/Core/Geometry || mkdir -p $(OBJDIR_RELEASE)/Core/Geometry
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Geometry/Matrix4x4.o $(SRCDIR_RELEASE)/Core/Geometry/Matrix4x4.cpp

$(OBJDIR_RELEASE)/Core/Geometry/Plane.o: $(SRCDIR_RELEASE)/Core/Geometry/Plane.cpp
	test -d $(OBJDIR_RELEASE)/Core/Geometry || mkdir -p $(OBJDIR_RELEASE)/Core/Geometry
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Geometry/Plane.o $(SRCDIR_RELEASE)/Core/Geometry/Plane.cpp

$(OBJDIR_RELEASE)/Core/Geometry/Ray.o: $(SRCDIR_RELEASE)/Core/Geometry/Ray.cpp
	test -d $(OBJDIR_RELEASE)/Core/Geometry || mkdir -p $(OBJDIR_RELEASE)/Core/Geometry
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Geometry/Ray.o $(SRCDIR_RELEASE)/Core/Geometry/Ray.cpp

$(OBJDIR_RELEASE)/Core/Geometry/Transform.o: $(SRCDIR_RELEASE)/Core/Geometry/Transform.cpp
	test -d $(OBJDIR_RELEASE)/Core/Geometry || mkdir -p $(OBJDIR_RELEASE)/Core/Geometry
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Geometry/Transform.o $(SRCDIR_RELEASE)/Core/Geometry/Transform.cpp

$(OBJDIR_RELEASE)/Core/Geometry/Vector2.o: $(SRCDIR_RELEASE)/Core/Geometry/Vector2.cpp
	test -d $(OBJDIR_RELEASE)/Core/Geometry || mkdir -p $(OBJDIR_RELEASE)/Core/Geometry
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Geometry/Vector2.o $(SRCDIR_RELEASE)/Core/Geometry/Vector2.cpp

$(OBJDIR_RELEASE)/Core/Geometry/Vector3.o: $(SRCDIR_RELEASE)/Core/Geometry/Vector3.cpp
	test -d $(OBJDIR_RELEASE)/Core/Geometry || mkdir -p $(OBJDIR_RELEASE)/Core/Geometry
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Geometry/Vector3.o $(SRCDIR_RELEASE)/Core/Geometry/Vector3.cpp
	
#
# Image
#
$(OBJDIR_RELEASE)/Core/Image/Image.o: $(SRCDIR_RELEASE)/Core/Image/Image.cpp
	test -d $(OBJDIR_RELEASE)/Core/Image || mkdir -p $(OBJDIR_RELEASE)/Core/Image
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Image/Image.o $(SRCDIR_RELEASE)/Core/Image/Image.cpp

$(OBJDIR_RELEASE)/Core/Image/RGBPixel.o: $(SRCDIR_RELEASE)/Core/Image/RGBPixel.cpp
	test -d $(OBJDIR_RELEASE)/Core/Image || mkdir -p $(OBJDIR_RELEASE)/Core/Image
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Image/RGBPixel.o $(SRCDIR_RELEASE)/Core/Image/RGBPixel.cpp

#
# Integrator
#
$(OBJDIR_RELEASE)/Core/Integrator/Integrator.o: $(SRCDIR_RELEASE)/Core/Integrator/Integrator.cpp
	test -d $(OBJDIR_RELEASE)/Core/Integrator || mkdir -p $(OBJDIR_RELEASE)/Core/Integrator
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Integrator/Integrator.o $(SRCDIR_RELEASE)/Core/Integrator/Integrator.cpp
		
$(OBJDIR_RELEASE)/Core/Integrator/PathIntegrator.o: $(SRCDIR_RELEASE)/Core/Integrator/PathIntegrator.cpp
	test -d $(OBJDIR_RELEASE)/Core/Integrator || mkdir -p $(OBJDIR_RELEASE)/Core/Integrator
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Integrator/PathIntegrator.o $(SRCDIR_RELEASE)/Core/Integrator/PathIntegrator.cpp

$(OBJDIR_RELEASE)/Core/Integrator/WhittedIntegrator.o: $(SRCDIR_RELEASE)/Core/Integrator/WhittedIntegrator.cpp
	test -d $(OBJDIR_RELEASE)/Core/Integrator || mkdir -p $(OBJDIR_RELEASE)/Core/Integrator
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Integrator/WhittedIntegrator.o $(SRCDIR_RELEASE)/Core/Integrator/WhittedIntegrator.cpp

$(OBJDIR_RELEASE)/Core/Integrator/IGIIntegrator.o: $(SRCDIR_RELEASE)/Core/Integrator/IGIIntegrator.cpp
	test -d $(OBJDIR_RELEASE)/Core/Integrator || mkdir -p $(OBJDIR_RELEASE)/Core/Integrator
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Integrator/IGIIntegrator.o $(SRCDIR_RELEASE)/Core/Integrator/IGIIntegrator.cpp

$(OBJDIR_RELEASE)/Core/Integrator/TestIntegrator.o: $(SRCDIR_RELEASE)/Core/Integrator/TestIntegrator.cpp
	test -d $(OBJDIR_RELEASE)/Core/Integrator || mkdir -p $(OBJDIR_RELEASE)/Core/Integrator
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Integrator/TestIntegrator.o $(SRCDIR_RELEASE)/Core/Integrator/TestIntegrator.cpp

#
# Light
#
$(OBJDIR_RELEASE)/Core/Light/AreaLight.o: $(SRCDIR_RELEASE)/Core/Light/AreaLight.cpp
	test -d $(OBJDIR_RELEASE)/Core/Light || mkdir -p $(OBJDIR_RELEASE)/Core/Light
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Light/AreaLight.o $(SRCDIR_RELEASE)/Core/Light/AreaLight.cpp

$(OBJDIR_RELEASE)/Core/Light/DiffuseAreaLight.o: $(SRCDIR_RELEASE)/Core/Light/DiffuseAreaLight.cpp
	test -d $(OBJDIR_RELEASE)/Core/Light || mkdir -p $(OBJDIR_RELEASE)/Core/Light
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Light/DiffuseAreaLight.o $(SRCDIR_RELEASE)/Core/Light/DiffuseAreaLight.cpp

$(OBJDIR_RELEASE)/Core/Light/InfiniteAreaLight.o: $(SRCDIR_RELEASE)/Core/Light/InfiniteAreaLight.cpp
	test -d $(OBJDIR_RELEASE)/Core/Light || mkdir -p $(OBJDIR_RELEASE)/Core/Light
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Light/InfiniteAreaLight.o $(SRCDIR_RELEASE)/Core/Light/InfiniteAreaLight.cpp

$(OBJDIR_RELEASE)/Core/Light/PointLight.o: $(SRCDIR_RELEASE)/Core/Light/PointLight.cpp
	test -d $(OBJDIR_RELEASE)/Core/Light || mkdir -p $(OBJDIR_RELEASE)/Core/Light
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Light/PointLight.o $(SRCDIR_RELEASE)/Core/Light/PointLight.cpp

#
# Material
#
$(OBJDIR_RELEASE)/Core/Material/BSDF.o: $(SRCDIR_RELEASE)/Core/Material/BSDF.cpp
	test -d $(OBJDIR_RELEASE)/Core/Material || mkdir -p $(OBJDIR_RELEASE)/Core/Material
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Material/BSDF.o $(SRCDIR_RELEASE)/Core/Material/BSDF.cpp

$(OBJDIR_RELEASE)/Core/Material/LambertianBxDF.o: $(SRCDIR_RELEASE)/Core/Material/LambertianBxDF.cpp
	test -d $(OBJDIR_RELEASE)/Core/Material || mkdir -p $(OBJDIR_RELEASE)/Core/Material
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Material/LambertianBxDF.o $(SRCDIR_RELEASE)/Core/Material/LambertianBxDF.cpp

$(OBJDIR_RELEASE)/Core/Material/SpecularReflectionBxDF.o: $(SRCDIR_RELEASE)/Core/Material/SpecularReflectionBxDF.cpp
	test -d $(OBJDIR_RELEASE)/Core/Material || mkdir -p $(OBJDIR_RELEASE)/Core/Material
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Material/SpecularReflectionBxDF.o $(SRCDIR_RELEASE)/Core/Material/SpecularReflectionBxDF.cpp

$(OBJDIR_RELEASE)/Core/Material/SpecularTransmissionBxDF.o: $(SRCDIR_RELEASE)/Core/Material/SpecularTransmissionBxDF.cpp
	test -d $(OBJDIR_RELEASE)/Core/Material || mkdir -p $(OBJDIR_RELEASE)/Core/Material
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Material/SpecularTransmissionBxDF.o $(SRCDIR_RELEASE)/Core/Material/SpecularTransmissionBxDF.cpp

#
# Maths
#	
$(OBJDIR_RELEASE)/Core/Maths/Random.o: $(SRCDIR_RELEASE)/Core/Maths/Random.cpp
	test -d $(OBJDIR_RELEASE)/Core/Maths || mkdir -p $(OBJDIR_RELEASE)/Core/Maths
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Maths/Random.o $(SRCDIR_RELEASE)/Core/Maths/Random.cpp

$(OBJDIR_RELEASE)/Core/Maths/Maths.o: $(SRCDIR_RELEASE)/Core/Maths/Maths.cpp
	test -d $(OBJDIR_RELEASE)/Core/Maths || mkdir -p $(OBJDIR_RELEASE)/Core/Maths
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Maths/Maths.o $(SRCDIR_RELEASE)/Core/Maths/Maths.cpp

#
# Object
#
$(OBJDIR_RELEASE)/Core/Object/Object.o: $(SRCDIR_RELEASE)/Core/Object/Object.cpp
	test -d $(OBJDIR_RELEASE)/Core/Object || mkdir -p $(OBJDIR_RELEASE)/Core/Object
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Object/Object.o $(SRCDIR_RELEASE)/Core/Object/Object.cpp

$(OBJDIR_RELEASE)/Core/Object/UniqueID.o: $(SRCDIR_RELEASE)/Core/Object/UniqueID.cpp
	test -d $(OBJDIR_RELEASE)/Core/Object || mkdir -p $(OBJDIR_RELEASE)/Core/Object
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Object/UniqueID.o $(SRCDIR_RELEASE)/Core/Object/UniqueID.cpp

#
# Postproc
#	

# N/A

#
# Renderer
#
$(OBJDIR_RELEASE)/Core/Renderer/BaseRenderer.o: $(SRCDIR_RELEASE)/Core/Renderer/BaseRenderer.cpp
	test -d $(OBJDIR_RELEASE)/Core/Renderer || mkdir -p $(OBJDIR_RELEASE)/Core/Renderer
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Renderer/BaseRenderer.o $(SRCDIR_RELEASE)/Core/Renderer/BaseRenderer.cpp

$(OBJDIR_RELEASE)/Core/Renderer/BasicRenderer.o: $(SRCDIR_RELEASE)/Core/Renderer/BasicRenderer.cpp
	test -d $(OBJDIR_RELEASE)/Core/Renderer || mkdir -p $(OBJDIR_RELEASE)/Core/Renderer
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Renderer/BasicRenderer.o $(SRCDIR_RELEASE)/Core/Renderer/BasicRenderer.cpp

$(OBJDIR_RELEASE)/Core/Renderer/DistributedRenderer.o: $(SRCDIR_RELEASE)/Core/Renderer/DistributedRenderer.cpp
	test -d $(OBJDIR_RELEASE)/Core/Renderer || mkdir -p $(OBJDIR_RELEASE)/Core/Renderer
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Renderer/DistributedRenderer.o $(SRCDIR_RELEASE)/Core/Renderer/DistributedRenderer.cpp

$(OBJDIR_RELEASE)/Core/Renderer/Renderer.o: $(SRCDIR_RELEASE)/Core/Renderer/Renderer.cpp
	test -d $(OBJDIR_RELEASE)/Core/Renderer || mkdir -p $(OBJDIR_RELEASE)/Core/Renderer
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Renderer/Renderer.o $(SRCDIR_RELEASE)/Core/Renderer/Renderer.cpp
		
$(OBJDIR_RELEASE)/Core/Renderer/TimeConstrainedRenderer.o: $(SRCDIR_RELEASE)/Core/Renderer/TimeConstrainedRenderer.cpp
	test -d $(OBJDIR_RELEASE)/Core/Renderer || mkdir -p $(OBJDIR_RELEASE)/Core/Renderer
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Renderer/TimeConstrainedRenderer.o $(SRCDIR_RELEASE)/Core/Renderer/TimeConstrainedRenderer.cpp

#
# Sampler
#
$(OBJDIR_RELEASE)/Core/Sampler/JitterSampler.o: $(SRCDIR_RELEASE)/Core/Sampler/JitterSampler.cpp
	test -d $(OBJDIR_RELEASE)/Core/Sampler || mkdir -p $(OBJDIR_RELEASE)/Core/Sampler
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Sampler/JitterSampler.o $(SRCDIR_RELEASE)/Core/Sampler/JitterSampler.cpp

$(OBJDIR_RELEASE)/Core/Sampler/MultijitterSampler.o: $(SRCDIR_RELEASE)/Core/Sampler/MultijitterSampler.cpp
	test -d $(OBJDIR_RELEASE)/Core/Sampler || mkdir -p $(OBJDIR_RELEASE)/Core/Sampler
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Sampler/MultijitterSampler.o $(SRCDIR_RELEASE)/Core/Sampler/MultijitterSampler.cpp

$(OBJDIR_RELEASE)/Core/Sampler/PrecomputationSampler.o: $(SRCDIR_RELEASE)/Core/Sampler/PrecomputationSampler.cpp
	test -d $(OBJDIR_RELEASE)/Core/Sampler || mkdir -p $(OBJDIR_RELEASE)/Core/Sampler
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Sampler/PrecomputationSampler.o $(SRCDIR_RELEASE)/Core/Sampler/PrecomputationSampler.cpp

$(OBJDIR_RELEASE)/Core/Sampler/RandomSampler.o: $(SRCDIR_RELEASE)/Core/Sampler/RandomSampler.cpp
	test -d $(OBJDIR_RELEASE)/Core/Sampler || mkdir -p $(OBJDIR_RELEASE)/Core/Sampler
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Sampler/RandomSampler.o $(SRCDIR_RELEASE)/Core/Sampler/RandomSampler.cpp

$(OBJDIR_RELEASE)/Core/Sampler/SamplerDiagnostics.o: $(SRCDIR_RELEASE)/Core/Sampler/SamplerDiagnostics.cpp
	test -d $(OBJDIR_RELEASE)/Core/Sampler || mkdir -p $(OBJDIR_RELEASE)/Core/Sampler
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Sampler/SamplerDiagnostics.o $(SRCDIR_RELEASE)/Core/Sampler/SamplerDiagnostics.cpp

#
# Scene
#
$(OBJDIR_RELEASE)/Core/Scene/EmissivePrimitive.o: $(SRCDIR_RELEASE)/Core/Scene/EmissivePrimitive.cpp
	test -d $(OBJDIR_RELEASE)/Core/Scene || mkdir -p $(OBJDIR_RELEASE)/Core/Scene
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Scene/EmissivePrimitive.o $(SRCDIR_RELEASE)/Core/Scene/EmissivePrimitive.cpp

$(OBJDIR_RELEASE)/Core/Scene/Environment.o: $(SRCDIR_RELEASE)/Core/Scene/Environment.cpp
	test -d $(OBJDIR_RELEASE)/Core/Scene || mkdir -p $(OBJDIR_RELEASE)/Core/Scene
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Scene/Environment.o $(SRCDIR_RELEASE)/Core/Scene/Environment.cpp

$(OBJDIR_RELEASE)/Core/Scene/EnvironmentLoader.o: $(SRCDIR_RELEASE)/Core/Scene/EnvironmentLoader.cpp
	test -d $(OBJDIR_RELEASE)/Core/Scene || mkdir -p $(OBJDIR_RELEASE)/Core/Scene
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Scene/EnvironmentLoader.o $(SRCDIR_RELEASE)/Core/Scene/EnvironmentLoader.cpp

$(OBJDIR_RELEASE)/Core/Scene/GeometricPrimitive.o: $(SRCDIR_RELEASE)/Core/Scene/GeometricPrimitive.cpp
	test -d $(OBJDIR_RELEASE)/Core/Scene || mkdir -p $(OBJDIR_RELEASE)/Core/Scene
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Scene/GeometricPrimitive.o $(SRCDIR_RELEASE)/Core/Scene/GeometricPrimitive.cpp

$(OBJDIR_RELEASE)/Core/Scene/ParticleSceneLoader.o: $(SRCDIR_RELEASE)/Core/Scene/ParticleSceneLoader.cpp
	test -d $(OBJDIR_RELEASE)/Core/Scene || mkdir -p $(OBJDIR_RELEASE)/Core/Scene
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Scene/ParticleSceneLoader.o $(SRCDIR_RELEASE)/Core/Scene/ParticleSceneLoader.cpp	
	
$(OBJDIR_RELEASE)/Core/Scene/Scene.o: $(SRCDIR_RELEASE)/Core/Scene/Scene.cpp
	test -d $(OBJDIR_RELEASE)/Core/Scene || mkdir -p $(OBJDIR_RELEASE)/Core/Scene
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Scene/Scene.o $(SRCDIR_RELEASE)/Core/Scene/Scene.cpp

$(OBJDIR_RELEASE)/Core/Scene/SceneLoader.o: $(SRCDIR_RELEASE)/Core/Scene/SceneLoader.cpp
	test -d $(OBJDIR_RELEASE)/Core/Scene || mkdir -p $(OBJDIR_RELEASE)/Core/Scene
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Scene/SceneLoader.o $(SRCDIR_RELEASE)/Core/Scene/SceneLoader.cpp

$(OBJDIR_RELEASE)/Core/Scene/Visibility.o: $(SRCDIR_RELEASE)/Core/Scene/Visibility.cpp
	test -d $(OBJDIR_RELEASE)/Core/Scene || mkdir -p $(OBJDIR_RELEASE)/Core/Scene
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Scene/Visibility.o $(SRCDIR_RELEASE)/Core/Scene/Visibility.cpp

$(OBJDIR_RELEASE)/Core/Scene/WavefrontSceneLoader.o: $(SRCDIR_RELEASE)/Core/Scene/WavefrontSceneLoader.cpp
	test -d $(OBJDIR_RELEASE)/Core/Scene || mkdir -p $(OBJDIR_RELEASE)/Core/Scene
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Scene/WavefrontSceneLoader.o $(SRCDIR_RELEASE)/Core/Scene/WavefrontSceneLoader.cpp

#
# Shape
#
$(OBJDIR_RELEASE)/Core/Shape/BasicMesh.o: $(SRCDIR_RELEASE)/Core/Shape/BasicMesh.cpp
	test -d $(OBJDIR_RELEASE)/Core/Shape || mkdir -p $(OBJDIR_RELEASE)/Core/Shape
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Shape/BasicMesh.o $(SRCDIR_RELEASE)/Core/Shape/BasicMesh.cpp

$(OBJDIR_RELEASE)/Core/Shape/BVHMesh.o: $(SRCDIR_RELEASE)/Core/Shape/BVHMesh.cpp
	test -d $(OBJDIR_RELEASE)/Core/Shape || mkdir -p $(OBJDIR_RELEASE)/Core/Shape
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Shape/BVHMesh.o $(SRCDIR_RELEASE)/Core/Shape/BVHMesh.cpp
	
$(OBJDIR_RELEASE)/Core/Shape/IndexedTriangle.o: $(SRCDIR_RELEASE)/Core/Shape/IndexedTriangle.cpp
	test -d $(OBJDIR_RELEASE)/Core/Shape || mkdir -p $(OBJDIR_RELEASE)/Core/Shape
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Shape/IndexedTriangle.o $(SRCDIR_RELEASE)/Core/Shape/IndexedTriangle.cpp

$(OBJDIR_RELEASE)/Core/Shape/KDTreeMesh.o: $(SRCDIR_RELEASE)/Core/Shape/KDTreeMesh.cpp
	test -d $(OBJDIR_RELEASE)/Core/Shape || mkdir -p $(OBJDIR_RELEASE)/Core/Shape
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Shape/KDTreeMesh.o $(SRCDIR_RELEASE)/Core/Shape/KDTreeMesh.cpp

$(OBJDIR_RELEASE)/Core/Shape/PersistentMesh.o: $(SRCDIR_RELEASE)/Core/Shape/PersistentMesh.cpp
	test -d $(OBJDIR_RELEASE)/Core/Shape || mkdir -p $(OBJDIR_RELEASE)/Core/Shape
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Shape/PersistentMesh.o $(SRCDIR_RELEASE)/Core/Shape/PersistentMesh.cpp
	
$(OBJDIR_RELEASE)/Core/Shape/Sphere.o: $(SRCDIR_RELEASE)/Core/Shape/Sphere.cpp
	test -d $(OBJDIR_RELEASE)/Core/Shape || mkdir -p $(OBJDIR_RELEASE)/Core/Shape
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Shape/Sphere.o $(SRCDIR_RELEASE)/Core/Shape/Sphere.cpp

$(OBJDIR_RELEASE)/Core/Shape/TreeMesh.o: $(SRCDIR_RELEASE)/Core/Shape/TreeMesh.cpp
	test -d $(OBJDIR_RELEASE)/Core/Shape || mkdir -p $(OBJDIR_RELEASE)/Core/Shape
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Shape/TreeMesh.o $(SRCDIR_RELEASE)/Core/Shape/TreeMesh.cpp

$(OBJDIR_RELEASE)/Core/Shape/Triangle.o: $(SRCDIR_RELEASE)/Core/Shape/Triangle.cpp
	test -d $(OBJDIR_RELEASE)/Core/Shape || mkdir -p $(OBJDIR_RELEASE)/Core/Shape
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Shape/Triangle.o $(SRCDIR_RELEASE)/Core/Shape/Triangle.cpp

$(OBJDIR_RELEASE)/Core/Shape/TriangleMesh.o: $(SRCDIR_RELEASE)/Core/Shape/TriangleMesh.cpp
	test -d $(OBJDIR_RELEASE)/Core/Shape || mkdir -p $(OBJDIR_RELEASE)/Core/Shape
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Shape/TriangleMesh.o $(SRCDIR_RELEASE)/Core/Shape/TriangleMesh.cpp

#
# Space
#
$(OBJDIR_RELEASE)/Core/Space/BasicSpace.o: $(SRCDIR_RELEASE)/Core/Space/BasicSpace.cpp
	test -d $(OBJDIR_RELEASE)/Core/Space || mkdir -p $(OBJDIR_RELEASE)/Core/Space
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Space/BasicSpace.o $(SRCDIR_RELEASE)/Core/Space/BasicSpace.cpp

#
# System
#	
$(OBJDIR_RELEASE)/Core/System/EngineKernel.o: $(SRCDIR_RELEASE)/Core/System/EngineKernel.cpp
	test -d $(OBJDIR_RELEASE)/Core/System || mkdir -p $(OBJDIR_RELEASE)/Core/System
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/System/EngineKernel.o $(SRCDIR_RELEASE)/Core/System/EngineKernel.cpp

$(OBJDIR_RELEASE)/Core/System/Lexer.o: $(SRCDIR_RELEASE)/Core/System/Lexer.cpp
	test -d $(OBJDIR_RELEASE)/Core/System || mkdir -p $(OBJDIR_RELEASE)/Core/System
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/System/Lexer.o $(SRCDIR_RELEASE)/Core/System/Lexer.cpp
	
$(OBJDIR_RELEASE)/Core/System/LibraryManager.o: $(SRCDIR_RELEASE)/Core/System/LibraryManager.cpp
	test -d $(OBJDIR_RELEASE)/Core/System || mkdir -p $(OBJDIR_RELEASE)/Core/System
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/System/LibraryManager.o $(SRCDIR_RELEASE)/Core/System/LibraryManager.cpp

$(OBJDIR_RELEASE)/Core/System/Platform.o: $(SRCDIR_RELEASE)/Core/System/Platform.cpp
	test -d $(OBJDIR_RELEASE)/Core/System || mkdir -p $(OBJDIR_RELEASE)/Core/System
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/System/Platform.o $(SRCDIR_RELEASE)/Core/System/Platform.cpp

$(OBJDIR_RELEASE)/Core/System/PlugInManager.o: $(SRCDIR_RELEASE)/Core/System/PlugInManager.cpp
	test -d $(OBJDIR_RELEASE)/Core/System || mkdir -p $(OBJDIR_RELEASE)/Core/System
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/System/PlugInManager.o $(SRCDIR_RELEASE)/Core/System/PlugInManager.cpp

$(OBJDIR_RELEASE)/Core/System/Version.o: $(SRCDIR_RELEASE)/Core/System/Version.cpp
	test -d $(OBJDIR_RELEASE)/Core/System || mkdir -p $(OBJDIR_RELEASE)/Core/System
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/System/Version.o $(SRCDIR_RELEASE)/Core/System/Version.cpp

#
# Texture
# 	

#
# Threading
#
$(OBJDIR_RELEASE)/Core/Threading/Spinlock.o: $(SRCDIR_RELEASE)/Core/Threading/Spinlock.cpp
	test -d $(OBJDIR_RELEASE)/Core/Threading || mkdir -p $(OBJDIR_RELEASE)/Core/Threading
	$(CPP) $(CFLAGS_RELEASE) $(INC_RELEASE) -c -o $(OBJDIR_RELEASE)/Core/Threading/Spinlock.o $(SRCDIR_RELEASE)/Core/Threading/Spinlock.cpp

clean_release:
	rm -f $(OBJ_RELEASE) $(OUT_RELEASE)

.PHONY: clean clean_debug clean_release

