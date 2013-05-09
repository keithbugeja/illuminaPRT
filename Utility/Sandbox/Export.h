#include "Environment.h"

class SceneBuilder
{
/*
	AddQuad(Transformation p_transform, const std::string &p_strArguments)

	enum PrimitiveType 
	{
		Quad,
		Sphere,
		Mesh
	};

	void AddPrimitive(PrimitiveType p_type, Transformation p_transform, const std::string &p_strArguments,	
		IShape *p_pShape = NULL, IMaterial *p_pMaterial = NULL)
	{
		ArgumentMap arguments(p_strArguments);

		switch (p_type)
		{
			case Quad:
				break;

			case Sphere:
				break;

			case Mesh:
				break;
		};
	}
*/
	bool Build(const std::string &p_strFilename)
	{
		std::stringstream output;

		output << "# Generate ILM" << std::endl;

		std::cout << output.str() << std::endl;
	}
};
