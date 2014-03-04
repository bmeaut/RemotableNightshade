/*
 * The big star catalogue extension to Stellarium:
 * Author and Copyright: Johannes Gajdosik, 2006, 2007
 * The implementation of most functions in this file
 * (getInfoString,getShortInfoString,...) is taken from
 * Stellarium, Copyright (C) 2002 Fabien Chereau,
 * and therefore the copyright of these belongs to Fabien Chereau.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 */

#include "hip_star_wrapper.h"
#include "zone_array.h"
#include <sstream>
#include "utility.h"
#include "translator.h"

namespace BigStarCatalogExtension {

string StarWrapperBase::getInfoString(const Navigator *nav) const {
  const Vec3d j2000_pos = getObsJ2000Pos(nav);
  double dec_j2000, ra_j2000;
  rect_to_sphe(&ra_j2000,&dec_j2000,j2000_pos);
  const Vec3d equatorial_pos = nav->j2000_to_earth_equ(j2000_pos);
  double dec_equ, ra_equ;
  rect_to_sphe(&ra_equ,&dec_equ,equatorial_pos);
  stringstream oss;
  oss.setf(ios::fixed, ios::floatfield);
  oss.precision(2);
  oss << "Magnitude: " << get_mag(nav) << " B-V: " << getBV() << endl;
  oss << "J2000" << " " << "RA/DE: " << Utility::printAngleHMS(ra_j2000,true)
		  << " / " << Utility::printAngleDMS(dec_j2000,true) << endl;
  oss << "Equ of date" << " " << "RA/DE: " << Utility::printAngleHMS(ra_equ)
		  << " / " << Utility::printAngleDMS(dec_equ) << endl;

    // calculate alt az
  double az,alt;
  rect_to_sphe(&az,&alt,nav->earth_equ_to_local(equatorial_pos));
  az = 3*M_PI - az;  // N is zero, E is 90 degrees
  if(az > M_PI*2) az -= M_PI*2;    
  oss << "Alt/Az: " << Utility::printAngleDMS(alt) << " / " << Utility::printAngleDMS(az) << endl;
  
  return oss.str();
}

string StarWrapperBase::getShortInfoString(const Navigator *nav) const
{
	stringstream oss;
	oss.setf(ios::fixed, ios::floatfield);
	oss.precision(2);
	oss << "Magnitude: " << get_mag(nav);
	return oss.str();
}

float StarWrapper1::getStarDistance( void ) {
	  if (s->plx)
		  return (AU/(SPEED_OF_LIGHT*86400*365.25)) / (s->plx*((0.00001/3600)*(M_PI/180)));
	  else
		  return 0;
}

string StarWrapper1::getEnglishName(void) const {
  if (s->hip) {
    char buff[64];
    sprintf(buff,"HP %d",s->hip);
    return buff;
  }
  return StarWrapperBase::getEnglishName();
}

string StarWrapper1::getInfoString(const Navigator *nav) const {
  const Vec3d j2000_pos = getObsJ2000Pos(nav);
  double dec_j2000, ra_j2000;
  rect_to_sphe(&ra_j2000,&dec_j2000,j2000_pos);
  const Vec3d equatorial_pos = nav->j2000_to_earth_equ(j2000_pos);
  double dec_equ, ra_equ;
  rect_to_sphe(&ra_equ,&dec_equ,equatorial_pos);
  stringstream oss;
  if (s->hip)
  {
    const string commonNameI18 = HipStarMgr::getCommonName(s->hip);
    const string sciName = HipStarMgr::getSciName(s->hip);
    if (commonNameI18!="" || sciName!="")
	{
		oss << commonNameI18 << (commonNameI18 == "" ? "" : " ");
		if (commonNameI18!="" && sciName!="") oss << "(";
		oss << (sciName=="" ? "" : sciName);
		if (commonNameI18!="" && sciName!="") oss << ")";
		oss << endl;
    }
    oss << "HP " << s->hip;
    if (s->component_ids)
	{
		oss << " " << HipStarMgr::convertToComponentIds(s->component_ids).c_str();
    }
    oss << endl;
  }

  oss.setf(ios::fixed, ios::floatfield);
  oss.precision(2);
  oss << _("Magnitude: ") << get_mag(nav) << " " << _("B-V: ") << s->getBV() << endl;
  oss << _("J2000 RA/DE: ") << Utility::printAngleHMS(ra_j2000,true)
		  << " / " << Utility::printAngleDMS(dec_j2000,true) << endl;
  
  oss << _("Equ of date RA/DE: ") << Utility::printAngleHMS(ra_equ)
		  << " / " << Utility::printAngleDMS(dec_equ) << endl;

    // calculate alt az
  double az,alt;
  rect_to_sphe(&az,&alt,nav->earth_equ_to_local(equatorial_pos));
  az = 3*M_PI - az;  // N is zero, E is 90 degrees
  if (az > M_PI*2)
	  az -= M_PI*2;    
  oss << _("Alt/Az: ") << Utility::printAngleDMS(alt) << " / " << Utility::printAngleDMS(az) << endl;

  if (s->plx)
  {
		oss.precision(5);
		oss << _("Parallax: ") << (0.00001*s->plx) << endl;
		oss.precision(2);
		oss << _("Distance: ") << (AU/(SPEED_OF_LIGHT*86400*365.25)) / (s->plx*((0.00001/3600)*(M_PI/180)))
			<< " " << _("ly") << endl;
  }

  if (s->sp_int)
  {
	  oss << _("Spectral Type: ") << HipStarMgr::convertToSpectralType(s->sp_int).c_str() << endl;
  }
  return oss.str();
}


string StarWrapper1::getShortInfoString(const Navigator *nav) const
{
	stringstream oss;
	if (s->hip)
	{
		const string commonNameI18 = HipStarMgr::getCommonName(s->hip);
		const string sciName = HipStarMgr::getSciName(s->hip);
		if (commonNameI18!="" || sciName!="")
		{
			oss << commonNameI18 << (commonNameI18 == "" ? "" : " ");
			if (commonNameI18!="" && sciName!="") oss << "(";
			oss << (sciName=="" ? "" : sciName);
			if (commonNameI18!="" && sciName!="") oss << ")";
			oss << "  ";
		}
		oss << "HP " << s->hip;
		if (s->component_ids)
		{
			oss << " " << HipStarMgr::convertToComponentIds(s->component_ids).c_str();
		}
		oss << "  ";
	}
	
	oss.setf(ios::fixed, ios::floatfield);
	oss.precision(2);
	oss << _("Magnitude: ") << get_mag(nav) << "  ";

	if (s->plx)
	{
		oss << _("Distance: ") << (AU/(SPEED_OF_LIGHT*86400*365.25)) / (s->plx*((0.00001/3600)*(M_PI/180)))
			<< " " << _("ly") << "  ";
	}
	
	if (s->sp_int)
	{
		oss << _("Spectral Type: ") << HipStarMgr::convertToSpectralType(s->sp_int).c_str();
	}

	return oss.str();
}


ObjectBaseP Star1::createStelObject(const SpecialZoneArray<Star1> *a,
                                    const SpecialZoneData<Star1> *z) const {
  return ObjectBaseP(new StarWrapper1(a,z,this));
}

ObjectBaseP Star2::createStelObject(const SpecialZoneArray<Star2> *a,
                                    const SpecialZoneData<Star2> *z) const {
  return ObjectBaseP(new StarWrapper2(a,z,this));
}

ObjectBaseP Star3::createStelObject(const SpecialZoneArray<Star3> *a,
                                    const SpecialZoneData<Star3> *z) const {
  return ObjectBaseP(new StarWrapper3(a,z,this));
}


} // namespace BigStarCatalogExtension

