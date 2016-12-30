#include "OptixAdvancedScene.hpp"
#include "LightBenderConfig.hpp"
#include "graphics/Camera.hpp"
#include "optixu/optixu_math_namespace.h"
#include "commonStructs.h"


namespace light
{


///////////////////////////////////////////////////////////////
/// \brief OptixAdvancedScene::OptixAdvancedScene
///////////////////////////////////////////////////////////////
OptixAdvancedScene::OptixAdvancedScene(
                                       int      width,
                                       int      height,
                                       unsigned vbo
                                       )
  : OptixScene( width, height, vbo )

{

  _buildGeometry( );
  _addLights( );

  context_->validate( );
  context_->compile( );

}



///////////////////////////////////////////////////////////////
/// \brief OptixAdvancedScene::~OptixAdvancedScene
///////////////////////////////////////////////////////////////
OptixAdvancedScene::~OptixAdvancedScene( )
{}



///////////////////////////////////////////////////////////////
/// \brief OptixAdvancedScene::_buildScene
///////////////////////////////////////////////////////////////
void
OptixAdvancedScene::_buildGeometry( )
{

  // Create primitives used in the scene
  optix::Geometry quadPrim   = createQuadPrimitive( );
  optix::Geometry boxPrim    = createBoxPrimitive( );
  optix::Geometry spherePrim = createSpherePrimitive( );

  // top group everything will get attached to
  optix::Group topGroup = context_->createGroup( );
  topGroup->setChildCount( 5 );

  // attach materials to geometries
  optix::GeometryGroup quadGroup = createGeomGroup(
                                                   { quadPrim },
                                                   { sceneMaterial_ },
                                                   "NoAccel",
                                                   "NoAccel"
                                                   );

  optix::GeometryGroup boxGroup = createGeomGroup(
                                                  { boxPrim },
                                                  { sceneMaterial_ },
                                                  "NoAccel",
                                                  "NoAccel"
                                                  );

  optix::GeometryGroup sphereGroup = createGeomGroup(
                                                     { spherePrim },
                                                     { sceneMaterial_ },
                                                     "NoAccel",
                                                     "NoAccel"
                                                     );


  // ground quad
  attachToGroup(
                topGroup,
                quadGroup,
                0,
                optix::make_float3( 0.0f ),
                optix::make_float3( 5.0f, 5.0f, 1.0f ),
                M_PIf * 0.5f,
                optix::make_float3( 1.0f, 0.0f, 0.0f )
                );

  // stack of two boxes
  attachToGroup( topGroup, boxGroup, 1, optix::make_float3( -2.0f, 1.0f, -1.0f ) );
  attachToGroup(
                topGroup, boxGroup, 2,
                optix::make_float3(    -2.0f, 2.5f, -1.0f ),
                optix::make_float3( 0.5f )
                );

  // two spheres
  attachToGroup( topGroup, sphereGroup, 3, optix::make_float3( 1.5f, 1.0f, 0.0f ) );
  attachToGroup(
                topGroup, sphereGroup, 4,
                optix::make_float3( 2.5f, 0.5f, 1.0f ),
                optix::make_float3( 0.5f )
                );

  topGroup->setAcceleration( context_->createAcceleration( "Bvh", "Bvh" ) );

  context_[ "top_object" ]->set( topGroup );

} // OptixAdvancedScene::_buildScene



///////////////////////////////////////////////////////////////
/// \brief OptixAdvancedScene::_addLights
///////////////////////////////////////////////////////////////
void
OptixAdvancedScene::_addLights( )
{

  std::vector< BasicLight > lights = {
    { optix::make_float3(  10.0f, 30.0f, 20.0f ),
      optix::make_float3( 2000.0f ),
      1, 0 },

    { optix::make_float3( -10.0f, 20.0f, 15.0f ),
      optix::make_float3( 900.0f ),
      1, 0 },

    { optix::make_float3( 0.0f, 2.0f, -35.0f ),
      optix::make_float3( 400.0f ),
      1, 0 }
  };

  optix::Buffer lightBuffer = context_->createBuffer( RT_BUFFER_INPUT );

  lightBuffer->setFormat( RT_FORMAT_USER );
  lightBuffer->setElementSize( sizeof( lights[ 0 ] ) );
  lightBuffer->setSize( lights.size( ) );
  memcpy( lightBuffer->map( ), lights.data( ), lights.size( ) * sizeof( lights[ 0 ] ) );
  lightBuffer->unmap( );

  context_[ "lights" ]->set( lightBuffer );

} // OptixAdvancedScene::_addLights



} // namespace light