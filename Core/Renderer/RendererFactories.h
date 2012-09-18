//----------------------------------------------------------------------------------------------
//	Filename:	RendererManager.h
//	Author:		Keith Bugeja
//	Date:		27/02/2010
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
#pragma once

#include <map>
#include <string>
#include <iostream>

#include "Renderer/Renderer.h"
#include "Renderer/BasicRenderer.h"
#include "Renderer/DistributedRenderer.h"
#include "Renderer/TimeConstrainedRenderer.h"

namespace Illumina
{
	namespace Core
	{		
		class BasicRendererFactory : public Illumina::Core::Factory<Illumina::Core::IRenderer>
		{
		public:
			Illumina::Core::IRenderer *CreateInstance(void)
			{
				return new BasicRenderer();
			}

			// Arguments
			// -- Id
			// -- Samples
			Illumina::Core::IRenderer *CreateInstance(ArgumentMap &p_argumentMap)
			{
				int samples = 1;
				p_argumentMap.GetArgument("Samples", samples);

				std::string strId;
				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, samples);

				return CreateInstance(samples);
			}

			Illumina::Core::IRenderer *CreateInstance(const std::string &p_strId, int p_nSamples)
			{
				return new BasicRenderer(p_strId, NULL, NULL, NULL, NULL, NULL, p_nSamples);
			}

			Illumina::Core::IRenderer *CreateInstance(int p_nSamples)
			{
				return new BasicRenderer(NULL, NULL, NULL, NULL, NULL, p_nSamples);
			}
		};

		
		class TimeConstrainedRendererFactory : public Illumina::Core::Factory<Illumina::Core::IRenderer>
		{
		public:
			Illumina::Core::IRenderer *CreateInstance(void)
			{
				return new TimeConstrainedRenderer();
			}

			// Arguments
			// -- Id
			// -- Samples
			Illumina::Core::IRenderer *CreateInstance(ArgumentMap &p_argumentMap)
			{
				int samples = 1;
				p_argumentMap.GetArgument("Samples", samples);

				std::string strId;
				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, samples);

				return CreateInstance(samples);
			}

			Illumina::Core::IRenderer *CreateInstance(const std::string &p_strId, int p_nSamples)
			{
				return new TimeConstrainedRenderer(p_strId, NULL, NULL, NULL, NULL, NULL, p_nSamples);
			}

			Illumina::Core::IRenderer *CreateInstance(int p_nSamples)
			{
				return new TimeConstrainedRenderer(NULL, NULL, NULL, NULL, NULL, p_nSamples);
			}
		};
		
		/*
		class DistributedRendererFactory : public Illumina::Core::Factory<Illumina::Core::IRenderer>
		{
		public:
			Illumina::Core::IRenderer *CreateInstance(void)
			{
				return new DistributedRenderer();
			}

			// Arguments
			// -- Id
			// -- Samples
			// -- TileWidth
			// -- TileHeight
			Illumina::Core::IRenderer *CreateInstance(ArgumentMap &p_argumentMap)
			{
				int samples = 1, 
					tileWidth = 8, 
					tileHeight = 8;
				
				p_argumentMap.GetArgument("Samples", samples);
				p_argumentMap.GetArgument("TileWidth", tileWidth);
				p_argumentMap.GetArgument("TileHeight", tileHeight);

				std::string strId;
				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, samples, tileWidth, tileHeight);

				return CreateInstance(samples, tileWidth, tileHeight);
			}

			Illumina::Core::IRenderer *CreateInstance(const std::string &p_strId, int p_nSamples, int p_nTileWidth, int p_nTileHeight)
			{
				return new DistributedRenderer(p_strId, NULL, NULL, NULL, NULL, NULL, p_nSamples, p_nTileWidth, p_nTileHeight);
			}

			Illumina::Core::IRenderer *CreateInstance(int p_nSamples, int p_nTileWidth, int p_nTileHeight)
			{
				return new DistributedRenderer(NULL, NULL, NULL, NULL, NULL, p_nSamples, p_nTileWidth, p_nTileHeight);
			}
		};
		*/
	}
}