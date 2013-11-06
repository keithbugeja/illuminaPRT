#pragma once
#include "Environment.h"
#include <boost/filesystem.hpp>

class GBuffer 
{
public:
	static bool Persist(const std::string &p_strFilename, int p_nFrameNumber, ICamera* p_pCamera, RadianceBuffer *p_pRadianceBuffer)
	{
		std::ofstream imageFile;
		std::stringstream imageNameStream;

		boost::filesystem::path imagePath(p_strFilename);
		imageNameStream << std::setfill('0') << std::setw(5) << p_nFrameNumber;
		std::string imageFilename = 
			(imagePath.parent_path() / (imageNameStream.str() + imagePath.extension().string())).string();

		// Open image file writer stream
		imageFile.open(imageFilename.c_str(), std::ios::binary);
		if (!imageFile.is_open()) return false;

		// Dump header
		int width = p_pRadianceBuffer->GetWidth(), 
			height = p_pRadianceBuffer->GetHeight();

		// Width, height of image raster
		imageFile.write((char*)&width, sizeof(int));
		imageFile.write((char*)&height, sizeof(int));

		// Camera position
		imageFile.write((char*)(&p_pCamera->GetObserver()), sizeof(float) * 3);
		
		// Camera frame
		imageFile.write((char*)(&p_pCamera->GetFrame().U), sizeof(float) * 3);
		imageFile.write((char*)(&p_pCamera->GetFrame().V), sizeof(float) * 3);
		imageFile.write((char*)(&p_pCamera->GetFrame().W), sizeof(float) * 3);
		
		// Field of view (degrees), aspect ratio
		float fov, aspect; p_pCamera->GetFieldOfView(&fov, &aspect);
		imageFile.write((char*)(&fov), sizeof(float));
		imageFile.write((char*)(&aspect), sizeof(float));

		// G buffer
		RadianceContext *pContext = p_pRadianceBuffer->GetBuffer();
		for (int i = 0; i < p_pRadianceBuffer->GetArea(); i++)
		{
			imageFile.write((char*)(&pContext->Position), sizeof(float) * 3);
			imageFile.write((char*)(&pContext->Normal), sizeof(float) * 3);
			imageFile.write((char*)(&pContext->Albedo), sizeof(float) * 3);
			imageFile.write((char*)(&pContext->Direct), sizeof(float) * 3);
			imageFile.write((char*)(&pContext->Indirect), sizeof(float) * 3);
			imageFile.write((char*)(&pContext->ViewRay.Origin), sizeof(float) * 3);
			imageFile.write((char*)(&pContext->ViewRay.Direction), sizeof(float) * 3);
			imageFile.write((char*)(&pContext->Distance), sizeof(float));

			pContext++;
		}
				
		imageFile.close();

		return true;
	}
};
