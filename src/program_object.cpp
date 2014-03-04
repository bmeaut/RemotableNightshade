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

#include "program_object.h"
#include "shader.h"
#include <stdexcept>
#include "s_texture.h"

#include <iostream>
#include <sstream>
#include <string>


using namespace std;

ProgramObject::ProgramObject()
{
	mProgramObject = glCreateProgramObjectARB(); 
	if (!mProgramObject)
		throw runtime_error("Can not create OpenGL program object");  // TODO catch
}

ProgramObject::~ProgramObject()
{
	disable();

	// Shaders must be detached to be deletable
	for(list<const Shader*>::const_iterator it = mShaders.begin(); it != mShaders.end(); ++it) {
		glDetachObjectARB(mProgramObject, (*it)->getVertShader());
		glDetachObjectARB(mProgramObject, (*it)->getFragShader()); 
	}
	mShaders.clear();

	if (mProgramObject)
		glDeleteObjectARB(mProgramObject);
}

void ProgramObject::enable()
{
	if (!mShaders.empty())
	{
		glUseProgramObjectARB(mProgramObject);
		mStartActiveTexture = 0;
		for(list<const Shader*>::const_iterator it = mShaders.begin(); it != mShaders.end(); ++it)
		{
			(*it)->setParams(mProgramObject, mStartActiveTexture);
			mStartActiveTexture += (*it)->getActiveTexCount();
		}
	}
}

void ProgramObject::disable() const
{
	glUseProgramObjectARB(0);
	glActiveTexture(GL_TEXTURE0);
}

void ProgramObject::attachShader(const Shader* shader)
{

	if(shader->isSupported()) {

		mShaders.push_back(shader);
		
		glAttachObjectARB(mProgramObject, shader->getVertShader());
		glAttachObjectARB(mProgramObject, shader->getFragShader()); 
		
		glLinkProgramARB(mProgramObject);
		GLint status = 0;
		glGetObjectParameterivARB(mProgramObject, GL_OBJECT_LINK_STATUS_ARB, &status);
		if (!status)
			throw runtime_error("Can't link OpenGL program object");

		// TODO: catch
	}
}

// Set texture
void ProgramObject::setParam(const string &name, const s_texture* texture) {

	if(!texture) return;

	mStartActiveTexture++;
	glActiveTexture(GL_TEXTURE0 + mStartActiveTexture);
    glBindTexture(GL_TEXTURE_2D, texture->getID());
	int loc = glGetUniformLocationARB(mProgramObject, name.c_str());
    glUniform1i(loc, mStartActiveTexture);

	//cout << "Shader param:  " << name << " : " << mStartActiveTexture << endl;
}

void ProgramObject::setParam(const string &name, const Vec3f &vector) {

	int loc = glGetUniformLocationARB(mProgramObject, name.c_str());
	glUniform3fvARB(loc, 1, vector);

	//cout << "Shader param:  " << name << " : " << vector << endl;
}

void ProgramObject::setParam(const string &name, const Vec3d &vector) {

	Vec3f tmp((float)vector[0], (float)vector[1], (float)vector[2]);
	int loc = glGetUniformLocationARB(mProgramObject, name.c_str());
	glUniform3fvARB(loc, 1, tmp);

	//cout << "Shader param:  " << name << " : " << vector << endl;

}


void ProgramObject::setParam(const string &name, const float number) {

	int loc = glGetUniformLocationARB(mProgramObject, name.c_str());
	glUniform1fARB(loc, number);

	//cout << "Shader param:  " << name << " : " << number << endl;
}
