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
#include "Renderer/MultipassRenderer.h"
#include "Renderer/DistributedRenderer.h"
#include "Renderer/MultithreadedRenderer.h"

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
				return new BasicRenderer(p_strId, NULL, NULL, NULL, NULL, p_nSamples);
			}

			Illumina::Core::IRenderer *CreateInstance(int p_nSamples)
			{
				return new BasicRenderer(NULL, NULL, NULL, NULL, p_nSamples);
			}
		};

		class MultipassRendererFactory : public Illumina::Core::Factory<Illumina::Core::IRenderer>
		{
		public:
			Illumina::Core::IRenderer *CreateInstance(void)
			{
				return new MultipassRenderer();
			}

			// Arguments
			// -- Id
			// -- Samples
			Illumina::Core::IRenderer *CreateInstance(ArgumentMap &p_argumentMap)
			{
				int samples = 1;

				// disc. buffer (will be moved to pp-filter factory)
				int dbSize = 3;
				float dbDistance = 10,
					dbCos = 0.75;

				p_argumentMap.GetArgument("Samples", samples);
				p_argumentMap.GetArgument("DB_Size", dbSize);
				p_argumentMap.GetArgument("DB_Distance", dbDistance);
				p_argumentMap.GetArgument("DB_Cos", dbCos);


				std::string strId;
				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, samples, dbSize, dbDistance, dbCos);

				return CreateInstance(samples, dbSize, dbDistance, dbCos);
			}

			Illumina::Core::IRenderer *CreateInstance(const std::string &p_strId, int p_nSamples, int p_nDBSize, float p_fDBDist, float p_fDBCos)
			{
				return new MultipassRenderer(p_strId, NULL, NULL, NULL, NULL, p_nSamples, p_nDBSize, p_fDBDist, p_fDBCos);
			}

			Illumina::Core::IRenderer *CreateInstance(int p_nSamples, int p_nDBSize, float p_fDBDist, float p_fDBCos)
			{
				return new MultipassRenderer(NULL, NULL, NULL, NULL, p_nSamples, p_nDBSize, p_fDBDist, p_fDBCos);
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

				std::string strId;
				if (p_argumentMap.GetArgument("Id", strId))
					return CreateInstance(strId, samples);

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
				return new DistributedRenderer(p_strId, NULL, NULL, NULL, NULL, p_nSamples, p_nTileWidth, p_nTileHeight);
			}

			Illumina::Core::IRenderer *CreateInstance(int p_nSamples, int p_nTileWidth, int p_nTileHeight)
			{
				return new DistributedRenderer(NULL, NULL, NULL, NULL, p_nSamples, p_nTileWidth, p_nTileHeight);
			}
		};
	}
}