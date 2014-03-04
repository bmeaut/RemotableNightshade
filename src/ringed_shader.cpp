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

// Shader for rendering a planet which has a ring system
// TODO: ambient value should come from ssystem.ini

#include "ringed_shader.h"
#include "GLee.h"

namespace
{
	const char* cVertShaderSource = 
		"                                                               \n\
			uniform vec3 LightPosition;									\n\
			varying vec2 TexCoord;                                      \n\
			varying vec3 Normal;                                        \n\
			varying vec3 Position;                                      \n\
			varying vec3 OriginalPosition;                              \n\
			varying float NdotL;										\n\
			varying vec3 Light;											\n\
			varying vec3 ModelLight;									\n\
			void main()                                                 \n\
			{                                                           \n\
                OriginalPosition = vec3(gl_Color);						\n\
				Position = vec3(gl_ModelViewMatrix * gl_Color);         \n\
				Normal = normalize(gl_NormalMatrix * gl_Normal);        \n\
				TexCoord = gl_MultiTexCoord0.st;                        \n\
				Light = normalize(LightPosition - Position);			\n\
                ModelLight = vec3(gl_ModelViewMatrixInverse * vec4(Light,1.0)); \n\
                NdotL = dot(Normal, Light);								\n\
				gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; \n\
			}                                                           \n\
		";

	const char* cFragShaderSource = 
           "uniform sampler2D Texture;                                  \n\
			uniform sampler2D RingTexture;								\n\
			uniform float RingInnerRadius;								\n\
			uniform float RingOuterRadius;								\n\
			uniform sampler2D ShadowTexture;							\n\
			uniform float SunHalfAngle;									\n\
			uniform vec3 MoonPosition1;									\n\
			uniform float MoonRadius1;									\n\
			uniform vec3 MoonPosition2;									\n\
			uniform float MoonRadius2;									\n\
			uniform vec3 MoonPosition3;									\n\
			uniform float MoonRadius3;									\n\
			uniform vec3 MoonPosition4;									\n\
			uniform float MoonRadius4;									\n\
			varying vec2 TexCoord;										\n\
			uniform float Ambient;  									\n\
			varying vec3 Position;                                      \n\
			varying vec3 OriginalPosition;								\n\
			varying float NdotL;										\n\
			varying vec3 Light;											\n\
			varying vec3 ModelLight;									\n\
			void main(void)												\n\
			{															\n\
				float diffuse = max(NdotL, 0.0);						\n\
				vec3 color = vec3(texture2D(Texture, TexCoord));		\n\
                if(diffuse != 0.0) {									\n\
                    if(RingOuterRadius != 0.0) {						\n\
                        vec3 intersect = ModelLight;					\n\
                        float scale = (-1.0 * OriginalPosition.z/intersect.z); \n\
                        intersect = intersect*scale  + OriginalPosition; \n\
                        if(dot(intersect,ModelLight) >= 0.0) {			 \n\
                            float radius = length(intersect);			 \n\
                            if(radius < RingOuterRadius && radius > RingInnerRadius ) {	\n\
                                vec2 coord = vec2(1.0-(radius-RingInnerRadius)/(RingOuterRadius-RingInnerRadius), 1.0);	\n\
				                vec4 rcolor = vec4(texture2D(RingTexture, coord)); \n\
                                diffuse = diffuse*mix(1.0, 0.3, rcolor.a); \n\
                            }											\n\
                        }												\n\
                    }                                                   \n\
                    vec3 moon;											\n\
                    float moonHalfAngle;								\n\
                    vec2 ratio;											\n\
                    float distance;										\n\
                    vec3 lookup;										\n\
                    if(MoonRadius1 != 0.0) {							\n\
                        moon = MoonPosition1 - Position;				\n\
                        moonHalfAngle = atan( MoonRadius1/ length(moon) ); \n\
                        distance = acos(dot(Light, normalize(moon)));	\n\
                        ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); \n\
                        ratio.x = distance/(moonHalfAngle + SunHalfAngle); \n\
                        lookup = vec3(texture2D(ShadowTexture, ratio)); \n\
                        diffuse = diffuse * lookup.r;					\n\
                    }													\n\
                    if(MoonRadius2 != 0.0) {							\n\
                        moon = MoonPosition2 - Position;				\n\
                        moonHalfAngle = atan( MoonRadius2/ length(moon) ); \n\
                        distance = acos(dot(Light, normalize(moon)));	\n\
                        ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); \n\
                        ratio.x = distance/(moonHalfAngle + SunHalfAngle); \n\
                        lookup = vec3(texture2D(ShadowTexture, ratio)); \n\
                        diffuse = diffuse * lookup.r;					\n\
                    }													\n\
                    if(MoonRadius3 != 0.0) {							\n\
                        moon = MoonPosition3 - Position;				\n\
                        moonHalfAngle = atan( MoonRadius3/ length(moon) ); \n\
                        distance = acos(dot(Light, normalize(moon)));	\n\
                        ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); \n\
                        ratio.x = distance/(moonHalfAngle + SunHalfAngle); \n\
                        lookup = vec3(texture2D(ShadowTexture, ratio)); \n\
                        diffuse = diffuse * lookup.r;					\n\
                    }													\n\
                    if(MoonRadius4 != 0.0) {							\n\
                        moon = MoonPosition4 - Position;				\n\
                        moonHalfAngle = atan( MoonRadius4/ length(moon) ); \n\
                        distance = acos(dot(Light, normalize(moon)));	\n\
                        ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); \n\
                        ratio.x = distance/(moonHalfAngle + SunHalfAngle); \n\
                        lookup = vec3(texture2D(ShadowTexture, ratio)); \n\
                        diffuse = diffuse * lookup.r;					\n\
                    }													\n\
                }														\n\
				gl_FragColor = vec4(color*(min(diffuse+Ambient, 1.0)), 1.0);		\n\
			}															\n\
		";

}

RingedShader::RingedShader() : 
	Shader(cVertShaderSource, cFragShaderSource)
{
}

RingedShader::~RingedShader()
{
}

void RingedShader::setParams(GLhandleARB programObject, GLuint startActiveTex) const
{
}

int RingedShader::getActiveTexCount() const
{
	return 3;
}

RingedShader* RingedShader::instance()
{
	static RingedShader shader;
	return &shader;
}
