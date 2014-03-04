/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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

#include "object_base.h"
#include "object.h"
#include "nightshade.h"
#include "projector.h"
#include "navigator.h"
#include "utility.h"
#include "s_texture.h"

void intrusive_ptr_add_ref(ObjectBase* p)
{
	p->retain();
}

void intrusive_ptr_release(ObjectBase* p)
{
	p->release();
}

ObjectBaseP ObjectBase::getBrightestStarInConstellation(void) const
{
	return ObjectBaseP();
}

s_texture * ObjectBase::pointer_star = NULL;
s_texture * ObjectBase::pointer_planet = NULL;
s_texture * ObjectBase::pointer_nebula = NULL;
s_texture * ObjectBase::pointer_telescope = NULL;

int ObjectBase::local_time = 0;

// Draw a nice animated pointer around the object
void ObjectBase::draw_pointer(int delta_time, const Projector* prj, const Navigator * nav)
{
	local_time+=delta_time;
	Vec3d pos=get_earth_equ_pos(nav);
	Vec3d screenpos;
	// Compute 2D pos and return if outside screen
	if (!prj->project_earth_equ(pos, screenpos)) return;

	// If object is large enough, no need for distracting pointer
	if (get_type()==ObjectRecord::OBJECT_NEBULA || get_type()==ObjectRecord::OBJECT_PLANET) {
		double size = get_on_screen_size(prj, nav);
		if ( size > prj->getViewportRadius()*.1f ) return;
	}


	prj->set_orthographic_projection();

	if (get_type()==ObjectRecord::OBJECT_NEBULA) glColor3f(0.4f,0.5f,0.8f);
	if (get_type()==ObjectRecord::OBJECT_PLANET) glColor3f(1.0f,0.3f,0.3f);

	if (get_type()==ObjectRecord::OBJECT_STAR||get_type()==ObjectRecord::OBJECT_TELESCOPE) {
		glColor3fv(get_RGB());
		float radius;
		if (get_type()==ObjectRecord::OBJECT_STAR) {
			radius = 13.f;
			glBindTexture (GL_TEXTURE_2D, pointer_star->getID());
		} else {
			radius = 25.f;
			glBindTexture (GL_TEXTURE_2D, pointer_telescope->getID());
		}
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glTranslatef(screenpos[0], screenpos[1], 0.0f);
		glRotatef((float)local_time/20.,0.,0.,1.);
		glBegin(GL_QUADS );
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(-radius,-radius,0.);      //Bas Gauche
		glTexCoord2f(1.0f,0.0f);
		glVertex3f(radius,-radius,0.);       //Bas Droite
		glTexCoord2f(1.0f,1.0f);
		glVertex3f(radius,radius,0.);        //Haut Droit
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(-radius,radius,0.);       //Haut Gauche
		glEnd ();
	}

	float size = get_on_screen_size(prj, nav);
	size+=20.f;
	size+=10.f*sin(0.002f * local_time);

	if (get_type()==ObjectRecord::OBJECT_NEBULA || get_type()==ObjectRecord::OBJECT_PLANET) {
		if (get_type()==ObjectRecord::OBJECT_PLANET)
			glBindTexture(GL_TEXTURE_2D, pointer_planet->getID());
		if (get_type()==ObjectRecord::OBJECT_NEBULA)
			glBindTexture(GL_TEXTURE_2D, pointer_nebula->getID());

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glTranslatef(screenpos[0], screenpos[1], 0.0f);
		if (get_type()==ObjectRecord::OBJECT_PLANET) glRotatef((float)local_time/100,0,0,-1);

		glTranslatef(-size/2, -size/2,0.0f);
		glRotatef(90,0,0,1);
		glBegin(GL_QUADS );
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(-10,-10,0);      //Bas Gauche
		glTexCoord2f(1.0f,0.0f);
		glVertex3f(10,-10,0);       //Bas Droite
		glTexCoord2f(1.0f,1.0f);
		glVertex3f(10,10,0);        //Haut Droit
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(-10,10,0);       //Haut Gauche
		glEnd ();

		glRotatef(-90,0,0,1);
		glTranslatef(0,size,0.0f);
		glBegin(GL_QUADS );
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(-10,-10,0);      //Bas Gauche
		glTexCoord2f(1.0f,0.0f);
		glVertex3f(10,-10,0);       //Bas Droite
		glTexCoord2f(1.0f,1.0f);
		glVertex3f(10,10,0);        //Haut Droit
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(-10,10,0);       //Haut Gauche
		glEnd ();

		glRotatef(-90,0,0,1);
		glTranslatef(0, size,0.0f);
		glBegin(GL_QUADS );
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(-10,-10,0);      //Bas Gauche
		glTexCoord2f(1.0f,0.0f);
		glVertex3f(10,-10,0);       //Bas Droite
		glTexCoord2f(1.0f,1.0f);
		glVertex3f(10,10,0);        //Haut Droit
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(-10,10,0);       //Haut Gauche
		glEnd ();

		glRotatef(-90,0,0,1);
		glTranslatef(0,size,0);
		glBegin(GL_QUADS );
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(-10,-10,0);      //Bas Gauche
		glTexCoord2f(1.0f,0.0f);
		glVertex3f(10,-10,0);       //Bas Droite
		glTexCoord2f(1.0f,1.0f);
		glVertex3f(10,10,0);        //Haut Droit
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(-10,10,0);       //Haut Gauche
		glEnd ();
	}

	prj->reset_perspective_projection();
}


void ObjectBase::init_textures(void)
{
	pointer_star = new s_texture("pointeur2.png");
	pointer_planet = new s_texture("pointeur4.png");
	pointer_nebula = new s_texture("pointeur5.png");
	pointer_telescope = new s_texture("pointeur2.png");
}

void ObjectBase::delete_textures(void)
{
	delete pointer_star;
	pointer_star = NULL;
	delete pointer_planet;
	pointer_planet = NULL;
	delete pointer_nebula;
	pointer_nebula = NULL;
	delete pointer_telescope;
	pointer_telescope = NULL;
}
