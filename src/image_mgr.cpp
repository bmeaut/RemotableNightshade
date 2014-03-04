/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2005 Robert Spearman
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

// manage an assortment of script loaded images

#include <iostream>
#include "image_mgr.h"

map<string, ImageMgr*> ImageMgr::m_managers = map<string, ImageMgr*>();

ImageMgr::ImageMgr()
{

}

ImageMgr::~ImageMgr()
{
	drop_all_images();
}

ImageMgr& ImageMgr::getImageMgr( string id ) {
	if( m_managers.find(id) == m_managers.end() )
		m_managers[id] = new ImageMgr;

    return *m_managers[id];
}

void ImageMgr::cleanUp( void ) {
	for( map<string, ImageMgr*>::iterator iter = m_managers.begin(); iter != m_managers.end(); ++iter ) {
		delete (iter->second);
	}
}

int ImageMgr::load_image(string filename, string name, Image::IMAGE_POSITIONING position_type, bool mipmap)
{

	// if name already exists, replace with new image (hash would have been easier...)
	for (vector<Image*>::iterator iter = active_images.begin(); iter != active_images.end(); ++iter) {
		if ((*iter)->get_name()==name) {
			delete (*iter);
			active_images.erase(iter);
			break;
		}
	}

	Image *img = new Image(filename, name, position_type, mipmap);

	if (!img || img->image_loaded()) {
		active_images.push_back(img);
		return 1;
	} else return 0;

}

int ImageMgr::drop_image(string name)
{
	for (vector<Image*>::iterator iter = active_images.begin(); iter != active_images.end(); ++iter) {
		if ((*iter)->get_name()==name) {
			delete (*iter);
			active_images.erase(iter);
			return 1;
		}
	}
	return 0;  // not found
}

int ImageMgr::drop_all_images()
{

	for (vector<Image*>::iterator iter = active_images.begin(); iter != active_images.end(); ++iter) {
		delete *iter;
	}
	active_images.clear();
	return 0;
}

Image * ImageMgr::get_image(string name)
{
	for (vector<Image*>::iterator iter = active_images.begin(); iter != active_images.end(); ++iter) {
		if ((*iter)->get_name()==name) return (*iter);
	}
	return &m_stubImage; // tlareywi: return stub image, not NULL ptr
}

void ImageMgr::update(int delta_time)
{
	for (vector<Image*>::iterator iter = active_images.begin(); iter != active_images.end(); ++iter) {
		(*iter)->update(delta_time);
	}
}

void ImageMgr::draw(const Navigator * nav, Projector * prj)
{
	for (vector<Image*>::iterator iter = active_images.begin(); iter != active_images.end(); ++iter) {
		(*iter)->draw(nav, prj);
	}
}

void ImageMgr::drawAll( const Navigator * nav, Projector * prj ) {
	for( map<string, ImageMgr*>::iterator iter = m_managers.begin(); iter != m_managers.end(); ++iter ) {
		iter->second->draw(nav, prj);
	}
}

