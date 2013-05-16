#include "Environment.h"

class IObjectDescriptor
	: public Object
{
public:
	virtual const std::string ToString(void) = 0;
};

typedef IObjectDescriptor IIntegratorDescriptor;
typedef IObjectDescriptor ICameraDescriptor;
typedef IObjectDescriptor ILightDescriptor;
typedef IObjectDescriptor IMaterialDescriptor;

class IShapeDescriptor
	: public IObjectDescriptor
{
public:
	const std::string GetMaterialGroupId(void) {
		return this->GetName() + "MaterialGroup";
	}
};

class IPrimitiveDescriptor
	: public IObjectDescriptor
{
public:
	Transformation Transform;
	IShapeDescriptor *ShapeDescriptor;
	IMaterialDescriptor *MaterialDescriptor;

	const std::string GetMaterialId(void) 
	{
		if (MaterialDescriptor == NULL)
			return ShapeDescriptor->GetMaterialGroupId();

		return MaterialDescriptor->GetName();
	}

	virtual const std::string ToString(void) = 0;
};

/*
 * Light descriptors
 */
class PointLightDescriptor
	: public ILightDescriptor
{
public:
	Vector3 Position;
	Spectrum Power;

	const std::string ToString(void)
	{
		std::stringstream result;

		result	<< "Light" << std::endl 
				<< "{" << std::endl
				<< "\tId\t=\t" << this->GetName() << std::endl
				<< "\tType\t=\tPoint" << std::endl 
				<< "\tPosition\t=\t{" << Position.X << ", " << Position.Y << ", " << Position.Z << "}" << std::endl
				<< "\tPower\t=\t{" << Power[0] << ", " << Power[1] << ", " << Power[2] << "}" << std::endl 
				<< "}" << std::endl;

		return result.str();
	}
};

/*
 * Material descriptors
 */

class MatteDescriptor 
	: public IMaterialDescriptor
{
public:
	Spectrum Reflectivity;

	const std::string ToString(void) 
	{
		std::stringstream result;

		result << "Material" << std::endl 
			<< "{" << std::endl 
			<< "\tId\t=\t" << this->GetName() << std::endl 
			<< "\tType\t=\tMatte" << std::endl 
			<< "\tReflectivity\t=\t{" << Reflectivity[0] << ", " << Reflectivity[1] << ", " << Reflectivity[2] << "}" << std::endl
			<< "}" << std::endl;

		return result.str();
	}
};

/*
 * Shape descriptors
 */

class ObjDescriptor
	: public IShapeDescriptor
{
public:
	std::string Filename;

	const std::string ToString(void) 
	{
		std::stringstream result;

		result << "Shape" << std::endl
			<< "{" << std::endl
			<< "\tId\t=\t" << this->GetName() << std::endl
			<< "\tType\t=\tWavefrontModel" << std::endl 
			<< "\tFilename\t=\t" << Filename << std::endl
			<< "\tMaterialGroupId\t=\t" << GetMaterialGroupId() << std::endl
			<< "\tContainer\t=\tKDTreeMesh" << std::endl
			<< "}" << std::endl;

		return result.str();
	}
};

/* 
 * Primitive descriptor
 */

class GeometricPrimitiveDescriptor
	: public IPrimitiveDescriptor
{
protected:
	const std::string TransformToString(void)
	{
		Matrix3x3 m3x3 = Transform.GetRotation();
		Vector3 t = Transform.GetTranslation(),
			s = Transform.GetScaling();

		std::stringstream result;

		result << "Rotation\t=\t{" << m3x3[0];
		for (int i = 1; i < 9; i++)
			result << "," << m3x3[i];
		result << "}" << std::endl
			<< "Translation\t=\t{"<<t[0]<<","<<t[1]<<","<<t[2]<<"}" << std::endl
			<< "Scaling\t=\t{"<<s[0]<<","<<s[1]<<","<<s[2]<<"}" << std::endl;

		return result.str();
	}

public:
	const std::string ToString(void)
	{
		std::stringstream result;

		result << "Primitive" << std::endl
			<< "{" << std::endl 
			<< "\tId\t=\t" << this->GetName() << std::endl
			<< "\tType\t=\tGeometry" << std::endl 
			<< "\tMaterial\t=\t" << GetMaterialId() << std::endl
			<< "\tShape\t=\t" << ShapeDescriptor->GetName() << std::endl
			<< "\t" << TransformToString() << std::endl
			<< "}" << std::endl;

		return result.str();
	}
};

class EmissivePrimitiveDescriptor
	: public GeometricPrimitiveDescriptor
{
public:
	ILightDescriptor *LightDescriptor;

	const std::string ToString(void)
	{
		std::stringstream result;

		result << "Primitive" << std::endl
			<< "{" << std::endl 
			<< "\tId\t=\t" << this->GetName() << std::endl
			<< "\tType\t=\tEmissive" << std::endl 
			<< "\tMaterial\t=\t" << GetMaterialId() << std::endl
			<< "\tShape\t=\t" << ShapeDescriptor->GetName() << std::endl
			<< "\tLight\t=\t" << LightDescriptor->GetName() << std::endl
			<< "\t" << TransformToString() << std::endl
			<< "}" << std::endl;

		return result.str();
	}
};

/*
 * Camera descriptor
 */
class PerspectiveCameraDescriptor
	: public ICameraDescriptor
{
public:
	Vector3 Position;
	Vector3 Target;

	const std::string ToString(void)
	{
		std::stringstream result;

		result << "Camera" << std::endl 
			<< "{" << std::endl 
			<< "\tId\t=\tCamera" << std::endl
			<< "\tType\t=\tPerspective" << std::endl 
			<< "\tFrame\t=\t-1, -1, 1, 1" << std::endl
			<< "\tDistance\t=\t1.0" << std::endl
			<< "\tPosition\t=\t{" << Position.X << ", " << Position.Y << ", " << Position.Z << "}" << std::endl
			<< "\tTarget\t=\t{" << Target.X << ", " << Target.Y << ", " << Target.Z << "}" << std::endl
			<< "\tUp\t=\t{0, 1, 0}" << std::endl
			<< "\tAspect\t=\t1.0" << std::endl
			<< "\tFov\t=\t55.0" << std::endl
			<< "}" << std::endl;

		return result.str();
	}
};


/*
 * Integrator descriptor
 */
class WhittedDescriptor
	: public IIntegratorDescriptor
{
public:
	const std::string ToString(void)
	{
		std::stringstream result;

		result << "Integrator" << std::endl 
			<< "{" << std::endl 
			<< "\tId\t=\tIntegrator" << std::endl
			<< "\tType\t=\tWhitted" << std::endl 
			<< "\tRayDepth\t=\t6" << std::endl
			<< "\tIndirectRays\t=\t0" << std::endl
			<< "\tShadowRays\t=\t1" << std::endl
			<< "\tEpsilon\t=\t1e-5" << std::endl
			<< "}" << std::endl;

		return result.str();
	}
};

class PathDescriptor
	: public IIntegratorDescriptor
{
public:
	const std::string ToString(void)
	{
		std::stringstream result;

		result << "Integrator" << std::endl 
			<< "{" << std::endl 
			<< "\tId\t=\tIntegrator" << std::endl
			<< "\tType\t=\tPathTracing" << std::endl 
			<< "\tRayDepth\t=\t6" << std::endl
			<< "\tIndirectRays\t=\t0" << std::endl
			<< "\tShadowRays\t=\t1" << std::endl
			<< "\tEpsilon\t=\t1e-5" << std::endl
			<< "}" << std::endl;

		return result.str();
	}
};

class SceneBuilder
{
	ICameraDescriptor *m_pCamera;
	IIntegratorDescriptor *m_pIntegrator;
	std::vector<ILightDescriptor*> m_lightList;
	std::vector<IPrimitiveDescriptor*> m_primitiveList;
	std::map<std::string, IShapeDescriptor*> m_shapeMap;
	std::map<std::string, IMaterialDescriptor*> m_materialMap;

public:
	void SetIntegrator(IIntegratorDescriptor *p_pIntegratorDescriptor)
	{
		m_pIntegrator = p_pIntegratorDescriptor;
	}

	void SetCamera(ICameraDescriptor *p_pCameraDescriptor)
	{
		m_pCamera = p_pCameraDescriptor;
	}

	void AddLight(ILightDescriptor *p_pLightDescriptor)
	{
		m_lightList.push_back(p_pLightDescriptor);
	}

	void AddPrimitive(Transformation p_transform, IShapeDescriptor *p_pShapeDescriptor, IMaterialDescriptor *p_pMaterialDescriptor, ILightDescriptor *p_pLightDescriptor)
	{
		GeometricPrimitiveDescriptor* primitive;

		if (p_pLightDescriptor != NULL)
		{
			EmissivePrimitiveDescriptor* emissive = new EmissivePrimitiveDescriptor();
			emissive->LightDescriptor = p_pLightDescriptor;
			primitive = emissive;
		}
		else
			primitive = new GeometricPrimitiveDescriptor();

		primitive->Transform = p_transform;
		primitive->ShapeDescriptor = p_pShapeDescriptor;
		primitive->MaterialDescriptor = p_pMaterialDescriptor;

		m_shapeMap[p_pShapeDescriptor->GetName()] = p_pShapeDescriptor;
		
		if (p_pMaterialDescriptor != NULL)
			m_materialMap[p_pMaterialDescriptor->GetName()] = p_pMaterialDescriptor;

		m_primitiveList.push_back(primitive);
	}

	bool Build(const std::string &p_strFilename)
	{
		std::stringstream output;

		output << "# Generated ILM" << std::endl;

		// Cameras
		output << "Cameras\n{" << std::endl << m_pCamera->ToString() << std::endl << "}" << std::endl;

		// Lights
		output << "Lights\n{" << std::endl;
		for (auto lightIt = m_lightList.begin(); lightIt != m_lightList.end(); lightIt++)
		{
			output << (*lightIt)->ToString() << std::endl;
		}
		output <<"}" << std::endl;

		// Shapes
		output << "Shapes\n{" << std::endl;
		for (auto shapeIt = m_shapeMap.begin(); shapeIt != m_shapeMap.end(); shapeIt++)
		{
			output << (*shapeIt).second->ToString() << std::endl;
		}
		output <<"}" << std::endl;

		// Materials
		output << "Materials\n{" << std::endl;
		for (auto materialIt = m_materialMap.begin(); materialIt != m_materialMap.end(); materialIt++)
		{
			output << (*materialIt).second->ToString() << std::endl;
		}
		output <<"}" << std::endl;

		// Samplers and filters
		output << "Samplers\n{\nSampler\n{\n\tId\t=\tSampler\n\tType\t=\tRandom\n}\n}\n" 
			<< "Filters\n{\nFilter\n{\n\tId\t=\tFilter\n\tType\t=\tBox\n}\n}\n" << std::endl;

		// Integrators
		output << "Integrators\n{" << std::endl << m_pIntegrator->ToString() << std::endl << "}" << std::endl;

		// Device
		output << "Devices\n{\nDevice\n{\n\tId\t=\tDevice\n\tType\t=\tDisplay\n\tWidth\t=\t1280\n\tHeight\t=\t800\n}\n}" << std::endl;

		// Renderer
		output << "Renderers\n{\nRenderer\n{\n\tId\t=\tRenderer\n\tType\t=\tBasic\n\tSamples\t=\t1\n\tFilter\t=\tFilter\n\tDevice\t=\tDevice\n\tIntegrator\t=\tIntegrator\n}\n}" << std::endl;

		// Environment
		output	<< "Environment" << std::endl
				<< "{" << std::endl
				<< "\tRenderer\t=\tRenderer" << std::endl
				<< "\tScene" << std::endl
				<< "\t{" << std::endl
				<< "\t\tSampler\t=\tSampler" << std::endl
				<< "\t\tCamera\t=\tCamera" << std::endl
				<< "\t\tLights\t=\t{"; 
		
		if (!m_lightList.empty())
		{
			output << m_lightList[0]->GetName();
			for (auto lightIt = m_lightList.begin() + 1; lightIt != m_lightList.end(); lightIt++)
				output << "," << (*lightIt)->GetName();
		}

		output	<< "}" << std::endl
				<< "\t\tSpace" << std::endl
				<< "\t\t{" << std::endl
				<< "\t\t\tId\t=\tSpace" << std::endl
				<< "\t\t\tType\t=\tBasic" << std::endl
				<< "\t\t\tPrimitives" << std::endl
				<< "\t\t\t{" << std::endl;

		for(auto primIt = m_primitiveList.begin(); primIt != m_primitiveList.end(); primIt++)
		{
			output << (*primIt)->ToString() << std::endl;
		}

		output  << "\t\t\t}" << std::endl
				<< "\t\t}" << std::endl
				<< "\t}" << std::endl
				<< "}" << std::endl;
				

		std::ofstream file(p_strFilename);
		file << output.str() << std::endl;
		file.close();

		std::cout << output.str() << std::endl;

		return true;
	}
};
