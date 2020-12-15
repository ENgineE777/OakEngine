
#pragma once

#include "Support/Support.h"
#include "Support/PointerRef.h"

namespace Oak
{
	/**
	\ingroup gr_code_root_render
	*/

	enum class TextureFormat
	{
		FMT_A8R8G8B8 = 0,
		FMT_A8R8,
		FMT_A8,
		FMT_R16_FLOAT,
		FMT_D16
	};

	/**
	\ingroup gr_code_root_render
	*/

	enum class TextureFilter
	{
		Point = 0 /*!< Point filter */,
		Linear /*!< Linera filtraion */
	};

	/**
	\ingroup gr_code_root_render
	*/

	enum class TextureAddress
	{
		Wrap = 0 /*!< Wrapping */,
		Mirror /*!< Mirroring */,
		Clamp /*!< Clamping */,
		Border /*!< Use border color */
	};

	/**
	\ingroup gr_code_root_render
	*/

	enum class TextureType
	{
		Tex2D = 0 /*!< 2D texture */,
		Array /*!< Array texture */,
		Cube /*!< Cube texture */,
		Volume /*!< Volumetric texture */
	};

	/**
	\ingroup gr_code_root_render
	*/

	/**
	\brief Texture

	This is representation of a texture.

	*/

	class Texture
	{
		private:

		friend class Render;
		friend class DeviceDX11;
		friend class TextureDX11;
		friend class PointerRef<Texture>;

		#ifndef DOXYGEN_SKIP
		eastl::string name;

		static void NextMip(int& width, int& height, int block_size);
		static int GetLevels(int width, int height, int block_size);

		virtual void Apply(int slot) = 0;

		public:

		Texture(int w, int h, TextureFormat f, int l, TextureType tp)
		{
			width = w;
			height = h;
			format = f;
			lods = l;
			type = tp;

			magminf = TextureFilter::Linear;
			mipmapf = TextureFilter::Linear;

			adressU = TextureAddress::Wrap;
			adressV = TextureAddress::Wrap;
			adressW = TextureAddress::Wrap;
		};
		virtual ~Texture() = default;
		#endif

		/**
			\brief Resize Texture

			\param[in] width Set width
			\param[in] height Set height
		*/
		virtual void Resize(int width, int height) = 0;

		/**
		\brief Get width of a texture

		\return Width of a texture
		*/
		virtual int GetWidth() { return width; };

		/**
		\brief Get height of a texture

		\return Height of a texture
		*/
		virtual int GetHeight() { return height; };

		/**
		\brief Get format of a texture

		\return Format of a texture
		*/
		virtual TextureFormat GetFormat() { return format; };

		/**
		\brief Get number of lods of a texture

		\return Number of a lods of a texture
		*/
		virtual int GetLods() { return lods; };

		/**
		\brief Get type of a texture

		\return Type of a texture
		*/
		virtual TextureType GetType() { return type; };

		/**
		\brief Set filtering type

		\param[in] magmin Filtering for magnifying
		\param[in] mipmap Filtering for mip mapping

		*/
		virtual void SetFilters(TextureFilter magmin, TextureFilter mipmap)
		{
			magminf = magmin;
			mipmapf = mipmap;
		};

		/**
		\brief Set adress type

		\param[in] adress Adress type
		*/
		virtual void SetAdress(TextureAddress adress)
		{
			adressU = adress;
			adressV = adress;
			adressW = adress;
		};

		/**
		\brief Set adress type for u coordinate

		\param[in] adress Adress type
		*/
		virtual void SetAdressU(TextureAddress adress)
		{
			adressU = adress;
		};

		/**
		\brief Set adress type for v coordinate

		\param[in] adress Adress type
		*/
		virtual void SetAdressV(TextureAddress adress)
		{
			adressV = adress;
		};

		/**
		\brief Set adress type for w coordinate

		\param[in] adress Adress type
		*/
		virtual void SetAdressW(TextureAddress adress)
		{
			adressW = adress;
		};

		/**
		\brief Generate full mip map chain
		*/
		virtual void GenerateMips() = 0;

		/**
		\brief Fill particular mip map by data

		\param[in] level Mip map level
		\param[in] layer Number of a layer
		\param[in] data Pointer to a data
		\param[in] stride Stride in data
		*/
		virtual void Update(int level, int layer, uint8_t* data, int stride) = 0;

	protected:

		#ifndef DOXYGEN_SKIP

		int width;
		int height;
		TextureFormat format;
		int lods;

		TextureFilter magminf;
		TextureFilter mipmapf;
		TextureAddress adressU;
		TextureAddress adressV;
		TextureAddress adressW;
		TextureType type;
		int refCounter = 0;

		virtual void Release() = 0;
		#endif
	};

	typedef PointerRef<Texture> TextureRef;
}