// SulClouds.cpp

#include "stdafx.h"
#include "SulClouds.h"
#include "SulScreenAlignedQuad.h"
#include "SulGeomQuadTiles.h"
#include "SulTransToEye.h"
#include "SulString.h"
#include "SulTexImage.h"
#include "SulScreenAlignedQuad.h"
#include <osg/BlendFunc>
#include <osg/positionattitudetransform>
#include <osgDB/ReadFile>

// TODO: the warnings are hard to implement because of the way things are initialized in here
// this class needs to be redesigned

CSulClouds::CSulClouds( CSulCloudPlane* pCloudPlane ) :
m_coverage( 0.5f ),
m_size( 20000.0f ),
m_wind( 0, 0 ),
m_height(1500)
{
	//m_rTexImage = new CSulTexImage( 256, 256 );
	m_rPerlin = new CSulPerlinNoise2D;

	m_rImage = new osg::Image;
	m_rImage->setPixelFormat( GL_ALPHA );
	/*
	m_rImage->setImage(
        256, 256, 1,                                                // 1=r? depth perhaps?
		GL_ALPHA,                                                   // internal format
        GL_ALPHA, GL_FLOAT,			                                // pixelformat, type
        reinterpret_cast<unsigned char*>(m_pData),							// data
        osg::Image::NO_DELETE, 
        1 );                                                        // packing
*/


	if ( pCloudPlane )
	{
		setCloudPlane( pCloudPlane );
	}

	create();
}

CSulCloudPlane*	CSulClouds::getCloudPlane()
{
	return m_rPlane;
}

void CSulClouds::setTextureStates()
{
	m_rPlane->getQuad()->getTexture()->setUseHardwareMipMapGeneration( true );
	m_rPlane->getQuad()->getTexture()->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
	m_rPlane->getQuad()->getTexture()->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
	m_rPlane->getQuad()->getTexture()->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
	m_rPlane->getQuad()->getTexture()->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT );
}

osg::Node* CSulClouds::createPlane( float size, float height )
{
	if ( !m_rPlane.valid() )
	{
		m_rPlane = new CSulCloudPlane( size );
		m_rPlane->create();
	}
	else
	{
		m_rPlane->setSize( size );
	}

	setCoverage( m_coverage );

//	setTextureStates();


	osg::StateSet* ss = m_rPlane->getOrCreateStateSet();
	osg::BlendFunc *trans = new osg::BlendFunc();
	trans->setFunction(osg::BlendFunc::SRC_ALPHA ,osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
	ss->setAttributeAndModes( trans );

	// transform to height
	m_rPat = new osg::PositionAttitudeTransform;
	m_rPat->setPosition( osg::Vec3(0, 0, height) );
	m_rPat->addChild( m_rPlane );

	return m_rPat;
}

osg::Node* CSulClouds::getNode()
{
	return m_rPat;
}

void CSulClouds::setCoverage( float coverage )
{
	m_coverage = coverage;

	if ( !m_rPlane.valid() )
	{
		osg::notify(osg::WARN) << "WARNING: CSulClouds::setCoverage m_rPlane not valid" << std::endl;
		return;
	}

	/*
	// m_coverage is a value from 0.0-1.0 we need to map this to values between 0-10
	int cloudNumber = osg::clampBetween( m_coverage*10.0f, 0.0f, 10.0f );
	CSulString texturePath;
	texturePath.Format( "Textures/clouds/CloudTexture%d.dds", cloudNumber );
	osg::Image* image = osgDB::readImageFile( texturePath );
	if ( image )
	{
		m_rPlane->getQuad()->setTexture( image, GL_RGBA );
	}
	else
	{
		osg::notify(osg::WARN) << "ERROR: CSulClouds::setCoverage image not found: " << texturePath << std::endl;
	}
	*/
	

	// create perlin noise cloud
	/*
	CSulPerlinNoise2D* pPerlin = new CSulPerlinNoise2D;
	pPerlin->CreateArray2D( m_rTexImage, 100.0f, 128, 0, 255 ); 
	m_rTexImage->Process();
	m_rPlane->getQuad()->setTexture( m_rTexImage->getTexture() );
	*/

	m_rPerlin->SetOctave( 8 );
	m_rPerlin->create( m_rImage );
	m_rPlane->getQuad()->setTexture( m_rImage, GL_LUMINANCE );

	setTextureStates();
}

void CSulClouds::setOctave( float o )
{
	m_rPerlin->SetOctave( o );
}

void CSulClouds::setPersistence( float p )
{
	m_rPerlin->SetPersistence( p );
}

void CSulClouds::setScale( float s )
{
	m_rPerlin->setScale( s );
}

void CSulClouds::setOfs( float o )
{
	m_rPerlin->setOfs( o );
}

void CSulClouds::generate()
{
	m_rPerlin->create( m_rImage );
	m_rPlane->getQuad()->setTexture( m_rImage, GL_LUMINANCE );
	setTextureStates();
}

CSulTexImage* CSulClouds::getTexImage()
{
	return m_rTexImage;
}

osg::Image* CSulClouds::getImage()
{
	return m_rImage;
}

float CSulClouds::getCoverage()
{
	return m_coverage;
}

void CSulClouds::setColor( const osg::Vec4& color )
{
	//@TKG FIXME, we should transfer color to cloudplane or adjust the light setting 
	//They are very bright during the night 
}

void CSulClouds::setWind( float x, float y )
{
	//@TKG FIXME, what is the range of valid values 
	m_wind.set( x, y );

	if ( !m_rPlane.valid() )
	{
		osg::notify(osg::WARN) << "WARNING: CSulClouds::setWind m_rPlane not valid" << std::endl;
		return;
	}

	m_rPlane->setWind( x, y );
}

void CSulClouds::setHeight( float h )
{
	m_height = h;

	if ( !m_rPlane.valid() )
	{
		osg::notify(osg::WARN) << "WARNING: CSulClouds::setHeight m_rPlane not valid" << std::endl;
		return;
	}

	m_rPat->setPosition( osg::Vec3(0, 0, h) );
}

float CSulClouds::getHeight()
{
	return m_height;
}

void CSulClouds::setSize( float s )
{
	m_size = s;

	if ( !m_rPlane.valid() )
	{
		osg::notify(osg::WARN) << "WARNING: CSulClouds::setSize m_rPlane not valid" << std::endl;
		return;
	}

	m_rPlane->setSize( s );
}

float CSulClouds::getSize()
{
	return m_size;
}

void CSulClouds::setUV( float s )
{
	if ( !m_rPlane.valid() )
	{
		osg::notify(osg::WARN) << "WARNING: CSulClouds::setUV m_rPlane not valid" << std::endl;
		return;
	}

	m_rPlane->getQuad()->setUV( s );
}

osg::Node* CSulClouds::create()
{
	/*
	// create perlin noise cloud
	m_rTexImage = new CSulTexImage( 256, 256 );

	CSulPerlinNoise2D* pPerlin = new CSulPerlinNoise2D;
	pPerlin->CreateArray2D( m_rTexImage, 100.0f, 128, 0, 255 ); 
	m_rTexImage->Process();
*/

	m_rGroup = new osg::Group;	
	m_rGroup->setName( "CSulClouds" );


	/*
	// debug create screen aligned quad to see clouds
	CSulScreenAlignedQuad* pQuad = new CSulScreenAlignedQuad( osg::Vec3(128,128,0), 256, 256, 800, 600 );
	pQuad->setTexture( m_rTexImage->getTexture() );
	pGroup->addChild( pQuad->getProjection() );
*/

	osg::Node* pGeodeCloud = createPlane( m_size, getHeight() );

	CSulTransToEye* mXform = new CSulTransToEye;
	mXform->IgnoreZ( true );
	mXform->setCullingActive(false);
	mXform->addChild( pGeodeCloud );

	m_rGroup->addChild( mXform );
/*
	// to keep the texture at the correct position and wind
	osg::TexMat* texMat = new osg::TexMat();
	osg::StateSet* stateset = m_rPlane->getOrCreateStateSet();
	stateset->setTextureAttribute( 0, texMat );
	m_rPlane->setCullCallback( new CSulCloudPlaneCullback( texMat, m_size ) );
*/

	return m_rGroup;
}

osg::Group* CSulClouds::getGroup()
{
	return m_rGroup;
}

void CSulClouds::setCloudPlane( CSulCloudPlane* pCloudPlane )
{
	m_rPlane = pCloudPlane;
}