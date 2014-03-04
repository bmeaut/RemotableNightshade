/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Courtesy IT Laboratory of the Faculty of Mathematics and Cybernetics
 * at Nizhny Novgorod State University, Russian Federation
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

#include "shader.h"
#include "GLee.h"
#include <stdexcept>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

Shader::Shader(const string& vertShader, const string& fragShader)
{

	compiled = false;

	// Uncomment below to test with no shader support
/*
	mVertShader = 0;
	mFragShader = 0;
	if(0)
*/
	{

		compiled = true;

		mVertShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
		if (!mVertShader) {
			compiled = false;
			cerr << "Error creating vertex shader.\n";
		}
			
		mFragShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
		if (!mFragShader) {
			compiled = false;
			cerr << "Can't create fragment shader\n";
		}

		const char* shaderstring = vertShader.c_str();
		glShaderSourceARB(mVertShader, 1, &shaderstring, NULL);
		shaderstring = fragShader.c_str();
		glShaderSourceARB(mFragShader, 1, &shaderstring, NULL);
		
		glCompileShaderARB(mVertShader);
		int status = 0;
		glGetObjectParameterivARB(mVertShader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
		if (!status) {
			compiled = false;
			cerr << "Can't compile vertex shader\n";
			printInfoLog(mVertShader);
		}
		glCompileShaderARB(mFragShader); 
		glGetObjectParameterivARB(mFragShader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
		if (!status) {
			compiled = false;
			cerr << "Can't compile fragment shader\n";
			printInfoLog(mFragShader);
		}
	}
}

Shader::~Shader()
{
	if (mVertShader)
		glDeleteObjectARB(mVertShader);
    if (mFragShader)
		glDeleteObjectARB(mFragShader);
}

GLhandleARB Shader::getVertShader() const
{
	return mVertShader;
}

GLhandleARB Shader::getFragShader() const
{
	return mFragShader;
}

bool Shader::isSupported() const
{
	return compiled;
}

// From very helpful GLSL tutorial at lighthouse3d.com

void Shader::printInfoLog(GLhandleARB obj) {
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;
	
	glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
							  &infologLength);
	
	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
		free(infoLog);
	}
}
