// Copyright (c) 2018-2019,  Zhirnov Andrey. For more information see 'LICENSE'

#pragma once

#include "scene/Renderer/ScenePreRender.h"

namespace FG
{

	//
	// Render Queue
	//

	class RenderQueue
	{
	// types
	protected:
		struct PerLayer
		{
			bool				enabled		= false;
			SubmitRenderPass	pass		{ LogicalPassID() };
		};

		using Layers_t	= StaticArray< PerLayer, uint(ERenderLayer::_Count) >;


	// variables
	protected:
		FGThreadPtr		_frameGraph;
		CameraInfo		_camera;

		Layers_t		_layers;


	// methods
	protected:
		RenderQueue () {}

		void _Create (const FGThreadPtr &fg, const CameraInfo &camera);

		ND_ Task  _Submit (ArrayView<Task> dependsOn);

		void _AddLayer (ERenderLayer layer, LogicalPassID passId, StringView dbgName);
		void _AddLayer (ERenderLayer layer, const RenderPassDesc &desc, StringView dbgName);

	public:
		template <typename DrawTask>
		void AddRenderObj (ERenderLayer layer, const DrawTask &task);

		template <typename TaskType>
		void AddTask (ERenderLayer beforeLayer, const TaskType &task);

		ND_ CameraInfo  const&	GetCamera ()		const	{ return _camera; }
		ND_ FGThreadPtr const&	GetFrameGraph ()	const	{ return _frameGraph; }
	};

	
		
	
/*
=================================================
	_Create
=================================================
*/
	inline void  RenderQueue::_Create (const FGThreadPtr &fg, const CameraInfo &camera)
	{
		ASSERT( fg );

		_frameGraph	= fg;
		_camera		= camera;

		for (auto& layer : _layers) {
			layer = Default;
		}
	}
	
/*
=================================================
	_Submit
=================================================
*/
	inline Task  RenderQueue::_Submit (ArrayView<Task> dependsOn)
	{
		Task	temp = null;

		for (auto& layer : _layers)
		{
			for (auto& dep : dependsOn) {
				layer.pass.DependsOn( dep );
			}

			temp = _frameGraph->AddTask( layer.pass );
			dependsOn = { &temp, 1 };
		}
		return temp;
	}

/*
=================================================
	_AddLayer
=================================================
*/
	inline void  RenderQueue::_AddLayer (ERenderLayer layer, LogicalPassID passId, StringView dbgName)
	{
		ASSERT( uint(layer) < _layers.size() );
		ASSERT( _camera.layers[uint(layer)] );

		auto&	data = _layers[uint(layer)];

		ASSERT( not data.enabled );		// layer already enabled
		data.enabled = true;
		data.pass	 = SubmitRenderPass{ passId };

		if ( not dbgName.empty() )
			data.pass.SetName( dbgName );
	}
	
	inline void  RenderQueue::_AddLayer (ERenderLayer layer, const RenderPassDesc &desc, StringView dbgName)
	{
		return _AddLayer( layer, _frameGraph->CreateRenderPass( desc ), dbgName );
	}

/*
=================================================
	AddRenderObj
=================================================
*/
	template <typename DrawTask>
	inline void  RenderQueue::AddRenderObj (ERenderLayer layer, const DrawTask &task)
	{
		ASSERT( _camera.layers[uint(layer)] );
		_frameGraph->AddTask( _layers[uint(layer)].pass.renderPassId, task );
	}
	
/*
=================================================
	AddTask
=================================================
*/
	template <typename TaskType>
	inline void  RenderQueue::AddTask (ERenderLayer beforeLayer, const TaskType &task)
	{
		ASSERT( _camera.layers[uint(layer)] );
		_layers[uint(layer)].pass.DependsOn( _frameGraph->AddTask( task ) );
	}


}	// FG