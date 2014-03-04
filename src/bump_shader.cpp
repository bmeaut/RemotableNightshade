/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Initial version courtesy IT Laboratory of the Faculty of 
 * Mathematics and Cybernetics at Nizhny Novgorod State University, 
 * Russian Federation
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

#include "bump_shader.h"
#include "GLee.h"

namespace
{
	const char* cVertShaderSource = 
		"                                                               \n\
			uniform vec3 LightPosition;									\n\
			varying vec2 TexCoord;										\n\
			varying vec3 TangentLight;									\n\
			varying vec3 Normal;										\n\
			varying vec3 Light;											\n\
			varying vec3 Position;										\n\
			void main(void)												\n\
			{															\n\
				Position = vec3(gl_ModelViewMatrix * gl_Color);			\n\
				Normal = normalize(gl_NormalMatrix * gl_Normal);		\n\
				Light = normalize(LightPosition - Position);			\n\
				vec3 binormal = vec3(0,-Normal.z,Normal.y);				\n\
				vec3 tangent = cross(Normal,binormal);					\n\
				TangentLight = vec3(dot(Light, tangent), dot(Light, binormal), dot(Light, Normal)); \n\
				TexCoord = gl_MultiTexCoord0.st;						\n\
				gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;	\n\
			}															\n\
		";

	const char* cFragShaderSource = 
		"                                                               \n\
			uniform sampler2D TextureMap;								\n\
			uniform sampler2D NormalTexture;							\n\
			uniform sampler2D ShadowTexture;							\n\
			uniform float SunHalfAngle;									\n\
			uniform vec3 UmbraColor;									\n\
			uniform vec3 MoonPosition1;									\n\
			uniform float MoonRadius1;									\n\
			uniform vec3 MoonPosition2;									\n\
			uniform float MoonRadius2;									\n\
			uniform vec3 MoonPosition3;									\n\
			uniform float MoonRadius3;									\n\
			uniform vec3 MoonPosition4;									\n\
			uniform float MoonRadius4;									\n\
			varying vec2 TexCoord;										\n\
			varying vec3 TangentLight;									\n\
			varying vec3 Normal;										\n\
			varying vec3 Light;											\n\
			varying vec3 Position;										\n\
			void main(void)												\n\
			{															\n\
                vec3 umbra = vec3(0.0, 0.0, 0.0);						\n\
				vec4 color = texture2D(TextureMap, TexCoord);			\n\
				vec3 light_b = normalize(TangentLight);					\n\
				vec3 normal_b = 2.0 * vec3(texture2D(NormalTexture, TexCoord)) - vec3(1.0); \n\
				float diffuse = max(dot(normal_b, light_b), 0.0);		\n\
				float shadowScale = 1.0;								\n\
                if(diffuse != 0.0) {									\n\
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
                        shadowScale = shadowScale * lookup.r;			\n\
                        umbra = UmbraColor;								\n\
                    }													\n\
                    if(MoonRadius2 != 0.0) {							\n\
                        moon = MoonPosition2 - Position;				\n\
                        moonHalfAngle = atan( MoonRadius2/ length(moon) ); \n\
                        distance = acos(dot(Light, normalize(moon)));	\n\
                        ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); \n\
                        ratio.x = distance/(moonHalfAngle + SunHalfAngle); \n\
                        lookup = vec3(texture2D(ShadowTexture, ratio)); \n\
                        shadowScale = shadowScale * lookup.r;			\n\
                    }													\n\
                    if(MoonRadius3 != 0.0) {							\n\
                        moon = MoonPosition3 - Position;				\n\
                        moonHalfAngle = atan( MoonRadius3/ length(moon) ); \n\
                        distance = acos(dot(Light, normalize(moon)));	\n\
                        ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); \n\
                        ratio.x = distance/(moonHalfAngle + SunHalfAngle); \n\
                        lookup = vec3(texture2D(ShadowTexture, ratio)); \n\
                        shadowScale = shadowScale * lookup.r;			\n\
                    }													\n\
                    if(MoonRadius4 != 0.0) {							\n\
                        moon = MoonPosition4 - Position;				\n\
                        moonHalfAngle = atan( MoonRadius4/ length(moon) ); \n\
                        distance = acos(dot(Light, normalize(moon)));	\n\
                        ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); \n\
                        ratio.x = distance/(moonHalfAngle + SunHalfAngle); \n\
                        lookup = vec3(texture2D(ShadowTexture, ratio)); \n\
                        shadowScale = shadowScale * lookup.r;			\n\
                    }													\n\
                }														\n\
				gl_FragColor = vec4(color.rgb*diffuse*(shadowScale+umbra*max(0.0,1.0-shadowScale)), color.a); \n\
			}															\n\
		";
}

BumpShader::BumpShader() : 
	Shader(cVertShaderSource, cFragShaderSource)
{
}

BumpShader::~BumpShader()
{
}

void BumpShader::setParams(GLhandleARB programObject, GLuint startActiveTex) const
{
}	

int BumpShader::getActiveTexCount() const
{
	return 3;
}

BumpShader* BumpShader::instance()
{
	static BumpShader shader;
	return &shader;
}
