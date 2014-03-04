/*
 * Nightshade (TM) astronomy simulation and visualization
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

#ifndef RINGED_SHADER_H
#define RINGED_SHADER_H

#include "shader.h"
#include "vecmath.h"

class RingedShader : public Shader
{
public:
	virtual ~RingedShader();
	virtual void setParams(GLhandleARB programObject, GLuint startActiveTex) const;
	virtual int getActiveTexCount() const;
	static RingedShader* instance();
private:
	RingedShader();
};

#endif // RINGED_SHADER_H
