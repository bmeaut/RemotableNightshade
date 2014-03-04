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

#ifndef PROGRAM_OBJECT_H
#define PROGRAM_OBJECT_H

#include <list>
#include "GLee.h"
#include <string>
#include "vecmath.h"

class Shader;
class s_texture;

class ProgramObject
{
public:
	ProgramObject();
	~ProgramObject();

	void enable();
	void disable() const;

	void attachShader(const Shader* shader);

	void setParam(const std::string &name, const s_texture *texture);
	void setParam(const std::string &name, const Vec3f &vector);
	void setParam(const std::string &name, const Vec3d &vector);
	void setParam(const std::string &name, const float number);

private:
	std::list<const Shader*> mShaders;

	GLhandleARB mProgramObject;
    GLuint mStartActiveTexture;
};

#endif // PROGRAM_OBJECT_H
