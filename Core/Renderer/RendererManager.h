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

#include "System/FactoryManager.h"

#include "Renderer/Renderer.h"
#include "Renderer/BasicRenderer.h"
//#include "Renderer/DistributedRenderer.h"
#include "Renderer/MultithreadedRenderer.h"

namespace Illumina
{
	namespace Core
	{
		typedef FactoryManager<IRenderer> RendererManager;
		
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

				std::string stdId;
				if (p_argumentMap.GetArgument("Id", stdId))
					return CreateInstance(stdId, samples);
				else
					return CreateInstance(samples);
			}

			Illumina::Core::IRenderer *CreateInstance(const std::string &p_strId, int p_nSamples)
			{
				return new BasicRenderer(p_strId, NULL, NULL, NULL, NULL, p_nSamples);
			}

			Illumina::Core::IRenderer *CreateInstance(int p_nSamples)
			{
				return new BasicRenderer(NULL, NULL, NULL, NULL, p_nSamples);
			}
		};

		class MultithreadedRendererFactory : public Illumina::Core::Factory<Illumina::Core::IRenderer>
		{
		public:
			Illumina::Core::IRenderer *CreateInstance(void)
			{
				return new MultithreadedRenderer();
			}

			// Arguments
			// -- Id
			// -- Samples
			Illumina::Core::IRenderer *CreateInstance(ArgumentMap &p_argumentMap)
			{
				int samples = 1;
				p_argumentMap.GetArgument("Samples", samples);

				std::string stdId;
				if (p_argumentMap.GetArgument("Id", stdId))
					return CreateInstance(stdId, samples);
				else
					return CreateInstance(samples);
			}

			Illumina::Core::IRenderer *CreateInstance(const std::string &p_strId, int p_nSamples)
			{
				return new MultithreadedRenderer(p_strId, NULL, NULL, NULL, NULL, p_nSamples);
			}

			Illumina::Core::IRenderer *CreateInstance(int p_nSamples)
			{
				return new MultithreadedRenderer(NULL, NULL, NULL, NULL, p_nSamples);
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

				std::string stdId;
				if (p_argumentMap.GetArgument("Id", stdId))
					return CreateInstance(stdId, samples, tileWidth, tileHeight);
				else
					return CreateInstance(samples, tileWidth, tileHeight);
			}

			Illumina::Core::IRenderer *CreateInstance(const std::string &p_strId, int p_nSamples, int p_nTileWidth, int p_nTileHeight)
			{
				return new DistributedRenderer(p_strId, NULL, NULL, NULL, NULL, p_nSamples, p_nTileWidth, p_nTileHeight);
			}

			Illumina::Core::IRenderer *CreateInstance(int p_nSamples, int p_nTileWidth, int p_nTileHeight)
			{
				return new DistributedRenderer(NULL, NULL, NULL, NULL, p_nSamples, p_nTileWidth, p_nTileHeight);
			}
		};
		*/
	}
}