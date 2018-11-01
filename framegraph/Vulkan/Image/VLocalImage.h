// Copyright (c) 2018,  Zhirnov Andrey. For more information see 'LICENSE'

#pragma once

#include "VImage.h"
#include "framegraph/Public/LowLevel/EResourceState.h"
#include "framegraph/Shared/ImageDataRange.h"

namespace FG
{

	//
	// Vulkan Image thread local
	//

	class VLocalImage final : public ResourceBase
	{
		friend class VImageUnitTest;

	// types
	public:
		using ImageRange	= ImageDataRange;

		struct ImageState
		{
		// variables
			EResourceState		state;
			VkImageLayout		layout;
			VkImageAspectFlags	aspect;
			ImageRange			range;
			Task				task;
			
		// methods
			ImageState () {}

			ImageState (EResourceState state, VkImageLayout layout, const ImageRange &range, VkImageAspectFlags aspect, Task task) :
				state{state}, layout{layout}, aspect{aspect}, range{range}, task{task} {}
		};

	private:
		using SubRange	= ImageRange::SubRange_t;

		struct ImageBarrier
		{
		// variables
			SubRange				range;
			VkImageLayout			layout		= VK_IMAGE_LAYOUT_MAX_ENUM;
			VkPipelineStageFlags	stages		= 0;
			VkAccessFlags			access		= 0;
			ExeOrderIndex			index		= ExeOrderIndex::Initial;
			bool					isReadable : 1;
			bool					isWritable : 1;

		// methods
			ImageBarrier () : isReadable{false}, isWritable{false} {}
		};

		using ImageViewMap_t	= VImage::ImageViewMap_t;
		using BarrierArray_t	= Array< ImageBarrier >;		// TODO: fixed size array or custom allocator

		
	// variables
	private:
		mutable BarrierArray_t	_pendingBarriers;
		mutable BarrierArray_t	_readWriteBarriers;

		mutable ImageViewMap_t	_viewMap;
		VImage const*			_imageData		= null;						// readonly access is thread safe
		VkImageLayout			_finalLayout	= VK_IMAGE_LAYOUT_GENERAL;
		

	// methods
	public:
		VLocalImage () {}
		~VLocalImage ();

		bool Create (const VImage *);
		void Destroy (OUT AppendableVkResources_t, OUT AppendableResourceIDs_t);

		void AddPendingState (const ImageState &) const;
		void CommitBarrier (VBarrierManager &barrierMngr, VFrameGraphDebugger *debugger = null) const;
		
		ND_ VkImageView			GetView (const VDevice &, const ImageViewDesc &) const;
		ND_ VkImageView			GetView (const VDevice &, const Optional<ImageViewDesc> &) const;

		ND_ bool				IsCreated ()		const	{ return _imageData != null; }
		ND_ VkImage				Handle ()			const	{ return _imageData->Handle(); }
		ND_ ImageDesc const&	Description ()		const	{ return _imageData->Description(); }
		ND_ VkImageAspectFlags	AspectMask ()		const	{ return _imageData->AspectMask(); }
		ND_ uint3 const&		Dimension ()		const	{ return Description().dimension; }
		ND_ uint const			Width ()			const	{ return Description().dimension.x; }
		ND_ uint const			Height ()			const	{ return Description().dimension.y; }
		ND_ uint const			Depth ()			const	{ return Description().dimension.z; }
		ND_ uint const			ArrayLayers ()		const	{ return Description().arrayLayers.Get(); }
		ND_ uint const			MipmapLevels ()		const	{ return Description().maxLevel.Get(); }
		ND_ EPixelFormat		PixelFormat ()		const	{ return Description().format; }
		ND_ EImage				ImageType ()		const	{ return Description().imageType; }
		ND_ uint const			Samples ()			const	{ return Description().samples.Get(); }


	private:
		bool _CreateView (const VDevice &, const HashedImageViewDesc &, OUT VkImageView &) const;

		ND_ static BarrierArray_t::iterator	_FindFirstBarrier (BarrierArray_t &arr, const SubRange &range);
			static void						_ReplaceBarrier (BarrierArray_t &arr, BarrierArray_t::iterator iter, const ImageBarrier &barrier);

	};


}	// FG
