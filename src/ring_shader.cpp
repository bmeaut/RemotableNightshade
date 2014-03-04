/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2010 Digitalis Education Solutions, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Nightshade is a trademark of Digitalis Education Solutions, Inc.
 * See the TRADEMARKS file for trademark usage requirements.
 *
 */

// Draw rings with planet shadow

// Designed for Saturn type rings
// Assumes Sun is distant (point)

#include "ring_shader.h"
#include "GLee.h"

namespace
{
	const char* cVertShaderSource = 
		"                                                               \n\
			uniform float PlanetRadius;									\n\
			uniform vec3 PlanetPosition;								\n\
			uniform vec3 LightDirection;								\n\
			uniform float SunnySideUp;									\n\
			varying vec2 TexCoord;                                      \n\
			varying vec3 Position;                                      \n\
			varying float PlanetHalfAngle;								\n\
			varying float Separation;									\n\
			varying float SeparationAngle;								\n\
			varying float NdotL;										\n\
			void main()                                                 \n\
			{                                                           \n\
				Position = vec3(gl_ModelViewMatrix * gl_Color);         \n\
				TexCoord = gl_MultiTexCoord0.st;                        \n\
                PlanetHalfAngle = atan(PlanetRadius/distance(PlanetPosition, Position)); \n\
				Separation = dot(LightDirection, normalize(PlanetPosition-Position)); \n\
				SeparationAngle = acos(Separation); \n\
                vec3 modelLight = vec3(gl_ModelViewMatrixInverse * vec4(LightDirection,1.0)); \n\
				NdotL = clamp(16.0*dot(vec3(0.0, 0.0, 1.0-2.0*SunnySideUp), modelLight), -1.0, 1.0); \n\
				gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; \n\
			}                                                           \n\
		";

	const char* cFragShaderSource = 
		"	uniform sampler2D Texture;                                  \n\
			varying vec2 TexCoord;										\n\
			varying float PlanetHalfAngle;								\n\
			varying float Separation;									\n\
			varying float SeparationAngle;								\n\
			varying float NdotL;										\n\
			void main(void)												\n\
			{															\n\
                vec4 color = vec4(texture2D(Texture, TexCoord));		\n\
				float diffuse = clamp(max(NdotL, -NdotL*0.2), 0.0, 1.0);\n\
				if(SeparationAngle < PlanetHalfAngle) diffuse = 0.0;	\n\
                float reflected = 0.3 * max(-1.0*Separation, 0.0);		\n\
				gl_FragColor = vec4(color.rgb*(diffuse+reflected+0.05), color.a); \n\
			}															\n\
		";
}

RingShader::RingShader() : 
	Shader(cVertShaderSource, cFragShaderSource), mLightDirection(0.0f, 0.0f, 0.0f),
	mPlanetPosition(0.0f, 0.0f, 0.0f), mTexMap(0), mPlanetRadius(0)
{
}

RingShader::~RingShader()
{
}

void RingShader::setParams(GLhandleARB programObject, GLuint startActiveTex) const
{
	glActiveTexture(GL_TEXTURE0 + startActiveTex);
    glBindTexture(GL_TEXTURE_2D, mTexMap);
	int loc = glGetUniformLocationARB(programObject, "Texture");
    glUniform1iARB(loc, startActiveTex);

	loc = glGetUniformLocationARB(programObject, "LightDirection");
	glUniform3fvARB(loc, 1, mLightDirection);

	loc = glGetUniformLocationARB(programObject, "PlanetPosition");
	glUniform3fvARB(loc, 1, mPlanetPosition);

	loc = glGetUniformLocationARB(programObject, "PlanetRadius");
	glUniform1fARB(loc, mPlanetRadius);

	loc = glGetUniformLocationARB(programObject, "SunnySideUp");
	glUniform1fARB(loc, mSunnySideUp);

}

int RingShader::getActiveTexCount() const
{
	return 1;
}

void RingShader::setLightDirection(const Vec3f& lightDirection)
{
	mLightDirection = lightDirection;
}

void RingShader::setPlanetPosition(const Vec3f& position)
{
	mPlanetPosition = position;
}


void RingShader::setTexMap(unsigned int texMap)
{
	mTexMap = texMap;
}

void RingShader::setPlanetRadius(float radius)
{
	mPlanetRadius = radius;
}

void RingShader::setSunnySideUp(float up)
{
	mSunnySideUp = up;
}


RingShader* RingShader::instance()
{
	static RingShader shader;
	return &shader;
}
