/*
 * Stellarium
 * Copyright (C) 2002 Fabien Chereau
 *
 * The big star catalogue extension to Stellarium:
 * Author and Copyright: Johannes Gajdosik, 2006
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

// class used to manage groups of Stars

#include "projector.h"
#include "hip_star_mgr.h"
#include "object.h"
#include "s_texture.h"
#include "navigator.h"
#include "utility.h"
#include "tone_reproductor.h"
#include "geodesic_grid.h"
#include "loadingbar.h"
#include "translator.h"
#include "app.h"
#include "init_parser.h"
#include "bytes.h"
#include "core.h"
#include "app_settings.h"

#include "zone_array.h"
#include "string_array.h"

#include <string>
#include <list>

#include <errno.h>
#include <string.h>
#include <unistd.h>

using namespace BigStarCatalogExtension;

static StringArray spectral_array;
static StringArray component_array;

string HipStarMgr::convertToSpectralType(int index) {
  if (index < 0 || index >= spectral_array.getSize()) {
    cout << "convertToSpectralType: bad index: " << index
         << ", max: " << spectral_array.getSize() << endl;
    return "";
  }
  return spectral_array[index];
}

string HipStarMgr::convertToComponentIds(int index) {
  if (index < 0 || index >= component_array.getSize()) {
    cout << "convertToComponentIds: bad index: " << index
         << ", max: " << component_array.getSize() << endl;
    return "";
  }
  return component_array[index];
}

void HipStarMgr::initTriangle(int lev,int index,
                           const Vec3d &c0,
                           const Vec3d &c1,
                           const Vec3d &c2) {
  ZoneArrayMap::const_iterator it(zone_arrays.find(lev));
  if (it!=zone_arrays.end()) it->second->initTriangle(index,c0,c1,c2);
}



Vec3f HipStarMgr::color_table[128] = {
  Vec3f(0.587877,0.755546,1.000000),
  Vec3f(0.609856,0.750638,1.000000),
  Vec3f(0.624467,0.760192,1.000000),
  Vec3f(0.639299,0.769855,1.000000),
  Vec3f(0.654376,0.779633,1.000000),
  Vec3f(0.669710,0.789527,1.000000),
  Vec3f(0.685325,0.799546,1.000000),
  Vec3f(0.701229,0.809688,1.000000),
  Vec3f(0.717450,0.819968,1.000000),
  Vec3f(0.733991,0.830383,1.000000),
  Vec3f(0.750857,0.840932,1.000000),
  Vec3f(0.768091,0.851637,1.000000),
  Vec3f(0.785664,0.862478,1.000000),
  Vec3f(0.803625,0.873482,1.000000),
  Vec3f(0.821969,0.884643,1.000000),
  Vec3f(0.840709,0.895965,1.000000),
  Vec3f(0.859873,0.907464,1.000000),
  Vec3f(0.879449,0.919128,1.000000),
  Vec3f(0.899436,0.930956,1.000000),
  Vec3f(0.919907,0.942988,1.000000),
  Vec3f(0.940830,0.955203,1.000000),
  Vec3f(0.962231,0.967612,1.000000),
  Vec3f(0.984110,0.980215,1.000000),
  Vec3f(1.000000,0.986617,0.993561),
  Vec3f(1.000000,0.977266,0.971387),
  Vec3f(1.000000,0.967997,0.949602),
  Vec3f(1.000000,0.958816,0.928210),
  Vec3f(1.000000,0.949714,0.907186),
  Vec3f(1.000000,0.940708,0.886561),
  Vec3f(1.000000,0.931787,0.866303),
  Vec3f(1.000000,0.922929,0.846357),
  Vec3f(1.000000,0.914163,0.826784),
  Vec3f(1.000000,0.905497,0.807593),
  Vec3f(1.000000,0.896884,0.788676),
  Vec3f(1.000000,0.888389,0.770168),
  Vec3f(1.000000,0.879953,0.751936),
  Vec3f(1.000000,0.871582,0.733989),
  Vec3f(1.000000,0.863309,0.716392),
  Vec3f(1.000000,0.855110,0.699088),
  Vec3f(1.000000,0.846985,0.682070),
  Vec3f(1.000000,0.838928,0.665326),
  Vec3f(1.000000,0.830965,0.648902),
  Vec3f(1.000000,0.823056,0.632710),
  Vec3f(1.000000,0.815254,0.616856),
  Vec3f(1.000000,0.807515,0.601243),
  Vec3f(1.000000,0.799820,0.585831),
  Vec3f(1.000000,0.792222,0.570724),
  Vec3f(1.000000,0.784675,0.555822),
  Vec3f(1.000000,0.777212,0.541190),
  Vec3f(1.000000,0.769821,0.526797),
  Vec3f(1.000000,0.762496,0.512628),
  Vec3f(1.000000,0.755229,0.498664),
  Vec3f(1.000000,0.748032,0.484926),
  Vec3f(1.000000,0.740897,0.471392),
  Vec3f(1.000000,0.733811,0.458036),
  Vec3f(1.000000,0.726810,0.444919),
  Vec3f(1.000000,0.719856,0.431970),
  Vec3f(1.000000,0.712983,0.419247),
  Vec3f(1.000000,0.706154,0.406675),
  Vec3f(1.000000,0.699375,0.394265),
  Vec3f(1.000000,0.692681,0.382075),
  Vec3f(1.000000,0.686003,0.369976),
  Vec3f(1.000000,0.679428,0.358120),
  Vec3f(1.000000,0.672882,0.346373),
  Vec3f(1.000000,0.666372,0.334740),
  Vec3f(1.000000,0.659933,0.323281),
  Vec3f(1.000000,0.653572,0.312004),
  Vec3f(1.000000,0.647237,0.300812),
  Vec3f(1.000000,0.640934,0.289709),
  Vec3f(1.000000,0.634698,0.278755),
  Vec3f(1.000000,0.628536,0.267954),
  Vec3f(1.000000,0.622390,0.257200),
  Vec3f(1.000000,0.616298,0.246551),
  Vec3f(1.000000,0.610230,0.235952),
  Vec3f(1.000000,0.604259,0.225522),
  Vec3f(1.000000,0.598288,0.215083),
  Vec3f(1.000000,0.592391,0.204756),
  Vec3f(1.000000,0.586501,0.194416),
  Vec3f(1.000000,0.580657,0.184120),
  Vec3f(1.000000,0.574901,0.173930),
  Vec3f(1.000000,0.569127,0.163645),
  Vec3f(1.000000,0.563449,0.153455),
  Vec3f(1.000000,0.557758,0.143147),
  Vec3f(1.000000,0.552134,0.132843),
  Vec3f(1.000000,0.546541,0.122458),
  Vec3f(1.000000,0.540984,0.111966),
  Vec3f(1.000000,0.535464,0.101340),
  Vec3f(1.000000,0.529985,0.090543),
  Vec3f(1.000000,0.524551,0.079292),
  Vec3f(1.000000,0.519122,0.068489),
  Vec3f(1.000000,0.513743,0.058236),
  Vec3f(1.000000,0.508417,0.048515),
  Vec3f(1.000000,0.503104,0.039232),
  Vec3f(1.000000,0.497805,0.030373),
  Vec3f(1.000000,0.492557,0.021982),
  Vec3f(1.000000,0.487338,0.014007),
  Vec3f(1.000000,0.482141,0.006417),
  Vec3f(1.000000,0.477114,0.000000),
  Vec3f(1.000000,0.473268,0.000000),
  Vec3f(1.000000,0.469419,0.000000),
  Vec3f(1.000000,0.465552,0.000000),
  Vec3f(1.000000,0.461707,0.000000),
  Vec3f(1.000000,0.457846,0.000000),
  Vec3f(1.000000,0.453993,0.000000),
  Vec3f(1.000000,0.450129,0.000000),
  Vec3f(1.000000,0.446276,0.000000),
  Vec3f(1.000000,0.442415,0.000000),
  Vec3f(1.000000,0.438549,0.000000),
  Vec3f(1.000000,0.434702,0.000000),
  Vec3f(1.000000,0.430853,0.000000),
  Vec3f(1.000000,0.426981,0.000000),
  Vec3f(1.000000,0.423134,0.000000),
  Vec3f(1.000000,0.419268,0.000000),
  Vec3f(1.000000,0.415431,0.000000),
  Vec3f(1.000000,0.411577,0.000000),
  Vec3f(1.000000,0.407733,0.000000),
  Vec3f(1.000000,0.403874,0.000000),
  Vec3f(1.000000,0.400029,0.000000),
  Vec3f(1.000000,0.396172,0.000000),
  Vec3f(1.000000,0.392331,0.000000),
  Vec3f(1.000000,0.388509,0.000000),
  Vec3f(1.000000,0.384653,0.000000),
  Vec3f(1.000000,0.380818,0.000000),
  Vec3f(1.000000,0.376979,0.000000),
  Vec3f(1.000000,0.373166,0.000000),
  Vec3f(1.000000,0.369322,0.000000),
  Vec3f(1.000000,0.365506,0.000000),
  Vec3f(1.000000,0.361692,0.000000),
};


#define GAMMA 0.45

static double Gamma(double gamma,double x) {
  return ((x<=0.0) ? 0.0 : exp(gamma*log(x)));
}

static Vec3f Gamma(double gamma,const Vec3f &x) {
  return Vec3f(Gamma(gamma,x[0]),Gamma(gamma,x[1]),Gamma(gamma,x[2]));
}

static
void InitColorTableFromConfigFile(const InitParser &conf) {
  std::map<float,Vec3f> color_map;
  for (float b_v=-0.5f;b_v<=4.0f;b_v+=0.01) {
    char entry[256];
    sprintf(entry,"bv_color_%+5.2f",b_v);
    const string s(conf.get_str("stars",entry,""));
    if (!s.empty()) {
      const Vec3f c(Utility::str_to_vec3f(s));
      color_map[b_v] = Gamma(1/0.45,c);
    }
  }
  if (color_map.size() > 1) {
    for (int i=0;i<128;i++) {
      const float b_v = IndexToBV(i);
      std::map<float,Vec3f>::const_iterator greater(color_map.upper_bound(b_v));
      if (greater == color_map.begin()) {
        HipStarMgr::color_table[i] = greater->second;
      } else {
        std::map<float,Vec3f>::const_iterator less(greater);--less;
        if (greater == color_map.end()) {
        	HipStarMgr::color_table[i] = less->second;
        } else {
        	HipStarMgr::color_table[i] = Gamma(0.45,
                                          ((b_v-less->first)*greater->second
                                          + (greater->first-b_v)*less->second)
                                          *(1.f/(greater->first-less->first)));
                      }
      }
    }
  }
}


HipStarMgr::HipStarMgr(void) :
    starTexture(),
    hip_index(new HipIndexStruct[NR_OF_HIP+1]),
    mag_converter(new MagConverter(*this)),
	fontSize(13.),
    starFont(0)
{
  setMagConverterMaxScaled60DegMag(6.5f);
  if (hip_index == 0 || mag_converter == 0) {
    cerr << "ERROR: HipStarMgr::HipStarMgr: no memory" << endl;
    exit(1);
  }
  max_geodesic_grid_level = -1;
  last_max_search_level = -1;
}


HipStarMgr::~HipStarMgr(void) {
	if( SharedData::Instance()->DB() ) {
		dbCursor<ObjectRecord> cursor(dbCursorForUpdate);
		dbQuery q;
		q = "type=",ObjectRecord::OBJECT_STAR;
		if( cursor.select(q) )
			cursor.removeAllSelected();
		SharedData::Instance()->DB()->commit();
	}

  ZoneArrayMap::iterator it(zone_arrays.end());
  while (it!=zone_arrays.begin()) {
    --it;
    delete it->second;
    it->second = NULL;
  }
  zone_arrays.clear();
  if (mag_converter) {delete mag_converter;mag_converter = 0;}
  if (hip_index) delete[] hip_index;
}

bool HipStarMgr::flagSciNames = true;
double HipStarMgr::current_JDay = 2451545.0;  // Default to J2000 so that constellation art shows up in correct positions at init
map<int,string> HipStarMgr::common_names_map;
map<int,string> HipStarMgr::common_names_map_i18n;
map<string,int> HipStarMgr::common_names_index;
map<string,int> HipStarMgr::common_names_index_i18n;

map<int,string> HipStarMgr::sci_names_map_i18n;
map<string,int> HipStarMgr::sci_names_index_i18n;

string HipStarMgr::getCommonName(int hip) {
  map<int,string>::const_iterator it(common_names_map_i18n.find(hip));
  if (it!=common_names_map_i18n.end()) return it->second;
  return "";
}

string HipStarMgr::getSciName(int hip) {
  map<int,string>::const_iterator it(sci_names_map_i18n.find(hip));
  if (it!=sci_names_map_i18n.end()) return it->second;
  return "";
}

void HipStarMgr::init(float font_size, const string& font_name, LoadingBar& lb, const InitParser &conf) {
  load_data(conf, lb);
  InitColorTableFromConfigFile(conf);
    // Load star texture no mipmap:
  starTexture = new s_texture("star16x16.png",TEX_LOAD_TYPE_PNG_SOLID,false);  // Load star texture no mipmap
  starFont = new s_font(font_size, font_name);
	if (!starFont) {
		printf("Can't create starFont\n");
		assert(0);
	}
}

void HipStarMgr::setGrid(GeodesicGrid* geodesic_grid) {
  geodesic_grid->visitTriangles(max_geodesic_grid_level,initTriangleFunc,this);
  for (ZoneArrayMap::const_iterator it(zone_arrays.begin());
       it!=zone_arrays.end();it++) {
    it->second->scaleAxis();
  }
}

void HipStarMgr::setColorScheme(const InitParser& conf, const string& section)
{
	// Load colors from config file
	string defaultColor = conf.get_str(section,"default_color");
	setLabelColor(Utility::str_to_vec3f(conf.get_str(section,"star_label_color", defaultColor)));
	setCircleColor(Utility::str_to_vec3f(conf.get_str(section,"star_circle_color", defaultColor)));
}

/***************************************************************************
 Load star catalogue data from files.
 If a file is not found, it will be skipped.
***************************************************************************/
void HipStarMgr::load_data(const InitParser &baseConf, LoadingBar& lb)
{
	// Please do not init twice:
	assert(max_geodesic_grid_level < 0);

	cout << "Loading star data..." << endl;

	InitParser conf;
	conf.load(AppSettings::Instance()->getDataRoot() + "/stars/default/stars.ini");
				         
	for (int i=0; i<100; i++)
	{
		char key_name[64];
		sprintf(key_name,"cat_file_name_%02d",i);
		const string cat_file_name = conf.get_str("stars",key_name,"").c_str();
		if (!cat_file_name.empty()) {
			lb.SetMessage(_("Loading catalog ") + cat_file_name);
			ZoneArray *const z = ZoneArray::create(*this,cat_file_name,lb);
			if (z)
			{
				if (max_geodesic_grid_level < z->level)
				{
					max_geodesic_grid_level = z->level;
				}
				ZoneArray *&pos(zone_arrays[z->level]);
				if (pos)
				{
					cerr << cat_file_name << ", " << z->level
						 << ": duplicate level" << endl;
					delete z;
				}
				else
				{
					pos = z;
				}
			}
		}
	}

	for (int i=0; i<=NR_OF_HIP; i++)
	{
		hip_index[i].a = 0;
		hip_index[i].z = 0;
		hip_index[i].s = 0;
	}
	for (ZoneArrayMap::const_iterator it(zone_arrays.begin());
	                it != zone_arrays.end();it++)
	{
		it->second->updateHipIndex(hip_index);
	}

	const string cat_hip_sp_file_name = conf.get_str("stars","cat_hip_sp_file_name","").c_str();
	if (cat_hip_sp_file_name.empty())
	{
		cerr << "ERROR: stars:cat_hip_sp_file_name not found" << endl;
	}
	else
	{
		try
		{
			spectral_array.initFromFile(AppSettings::Instance()->getDataRoot() +
										"/stars/default/" + cat_hip_sp_file_name);
		}
		catch (std::exception& e)
		{
			cerr << "ERROR while loading data from "
			     << ("stars/default/" + cat_hip_sp_file_name)
		    	 << ": " << e.what();
		}
	}

	const string cat_hip_cids_file_name = conf.get_str("stars","cat_hip_cids_file_name","").c_str();
	if (cat_hip_cids_file_name.empty())
	{
		cerr << "ERROR: stars:cat_hip_cids_file_name not found" << endl;
	}
	else
	{
		try
		{
			component_array.initFromFile(AppSettings::Instance()->getDataRoot() +
										 "/stars/default/" + cat_hip_cids_file_name);
		}
		catch (std::exception& e)
		{
			cerr << "ERROR while loading data from "
			     << ("stars/default/" + cat_hip_cids_file_name)
		    	 << ": " << e.what() << endl;
		}
	}

	last_max_search_level = max_geodesic_grid_level;
	cout << "finished, max_geodesic_level: " << max_geodesic_grid_level << endl;
}

// Load common names from file 
int HipStarMgr::load_common_names(const string& commonNameFile) {
  common_names_map.clear();
  common_names_map_i18n.clear();
  common_names_index.clear();
  common_names_index_i18n.clear();

  cout << "Loading star names from " << commonNameFile << endl;

  if( SharedData::Instance()->DB() ) {
    dbCursor<ObjectRecord> cursor(dbCursorForUpdate);
	dbQuery q;
	q = "type=",ObjectRecord::OBJECT_STAR;
	if( cursor.select(q) )
	  cursor.removeAllSelected();
	  SharedData::Instance()->DB()->commit();
  }

  FILE *cnFile=fopen(commonNameFile.c_str(),"r");
  if (!cnFile)
  {
	  cerr << "Warning " << commonNameFile << " not found." << endl;
    return 0;
  }

  // Assign names to the matching stars, now support spaces in names
  char line[256];
  while (fgets(line, sizeof(line), cnFile)) {
    line[sizeof(line)-1] = '\0';
    unsigned int hip;
    if (sscanf(line,"%u",&hip)!=1) {
		cerr << "ERROR: StarMgr::load_common_names(" << commonNameFile
           << "): bad line: \"" << line << '"' << endl;

		return 0;
    }
    unsigned int i = 0;
    while (line[i]!='|' && i<sizeof(line)-2) ++i;
    i++;
    string englishCommonName =  line+i;
    // remove newline
    englishCommonName.erase(englishCommonName.length()-1, 1);

    // remove underscores
    for (string::size_type j=0;j<englishCommonName.length();++j) {
        if (englishCommonName[j]=='_') englishCommonName[j]=' ';
    }
    const string commonNameI18n = _(englishCommonName.c_str());
    string commonNameI18n_cap = commonNameI18n;
    transform(commonNameI18n.begin(), commonNameI18n.end(),
              commonNameI18n_cap.begin(), ::toupper);

    common_names_map[hip] = englishCommonName;
    common_names_index[englishCommonName] = hip;
    common_names_map_i18n[hip] = commonNameI18n;
    common_names_index_i18n[commonNameI18n_cap] = hip;

	if( SharedData::Instance()->DB() ) {
		ObjectBaseP star = searchHP(hip);
		if(star) {
			ObjectRecord rec( englishCommonName.c_str(), commonNameI18n.c_str(),
							  hip, ObjectRecord::OBJECT_STAR, star->get_mag(0), star->getStarDistance());
			insert(rec);
		}
	}
  }

  // keep outside of loop for performance
  if( SharedData::Instance()->DB() ) {
    SharedData::Instance()->DB()->commit();
  }

  fclose(cnFile);
  return 1;
}


// Load scientific names from file 
void HipStarMgr::load_sci_names(const string& sciNameFile)
{
  sci_names_map_i18n.clear();
  sci_names_index_i18n.clear();

  cout << "Loading star sci names from " << sciNameFile << endl;

  FILE *snFile;
  snFile=fopen(sciNameFile.c_str(),"r");
  if (!snFile) {
	  cerr << "Warning " << sciNameFile << " not found." << endl;
	  return;
  }

  // Assign names to the matching stars, now support spaces in names
  char line[256];
  while (fgets(line, sizeof(line), snFile)) {
    line[sizeof(line)-1] = '\0';
    unsigned int hip;
    if (sscanf(line,"%u",&hip)!=1) {
		cerr << "ERROR: StarMgr::load_sci_names(" << sciNameFile
           << "): bad line: \"" << line << '"' << endl;
		return;
    }
	if (sci_names_map_i18n.find(hip)!=sci_names_map_i18n.end())
		continue;
   
	unsigned int i = 0;
    while (line[i]!='|' && i<sizeof(line)-2) ++i;
    i++;
    char *tempc = line+i;
    string sci_name = tempc;
    sci_name.erase(sci_name.length()-1, 1);
    string sci_name_i18n = sci_name;

    // remove underscores
    for (string::size_type j=0;j<sci_name_i18n.length();++j) {
      if (sci_name_i18n[j]=='_') sci_name_i18n[j]=' ';
    }

    string sci_name_i18n_cap = sci_name_i18n;
    transform(sci_name_i18n.begin(), sci_name_i18n.end(),
              sci_name_i18n_cap.begin(), ::toupper);
    
    sci_names_map_i18n[hip] = sci_name_i18n;
    sci_names_index_i18n[sci_name_i18n_cap] = hip;
  }

  fclose(snFile);
}

int HipStarMgr::drawStar(const Projector *prj,const Vec3d &XY,
                      const float rc_mag[2],
                      const Vec3f &color) const {
//cout << "StarMgr::drawStar: " << XY[0] << '/' << XY[1] << ", " << rmag << endl;
  //assert(rc_mag[1]>= 0.f);
  if (rc_mag[0]<=0.f || rc_mag[1]<=0.f) return -1;

  // Random coef for star twinkling
  glColor3fv(color*rc_mag[1]*(1.-twinkle_amount*rand()/RAND_MAX));

    // Blending is really important. Otherwise faint stars in the vicinity of
    // bright star will cause tiny black squares on the bright star, e.g.
    // see Procyon.
  glBlendFunc(GL_ONE, GL_ONE);

  float mag = 2.f*rc_mag[0];

  if (flagPointStar) {
    //! Draw the star rendered as GLpoint. This may be faster but it is not so nice
    prj->drawPoint2d(XY[0], XY[1]);
  } else {
    float mag = 2.f*rc_mag[0];
	
	// Roll off star size limit as fov decreases to match planet halo scale
    RangeMap<float> rmap(180, 1, -starSizeLimit, -(starSizeLimit + objectSizeLimit)); 
    float rolloff = -rmap.Map(prj->get_fov());
    if( mag > rolloff )
    	mag = rolloff;

    prj->drawSprite2dMode(XY[0], XY[1], mag);
  }
  return 0;
}

int HipStarMgr::getMaxSearchLevel(const ToneReproductor *eye,
                               const Projector *prj) const {
  int rval = -1;
  mag_converter->setFov(prj->get_fov());
  mag_converter->setEye(eye);
  for (ZoneArrayMap::const_iterator it(zone_arrays.begin());
       it!=zone_arrays.end();it++) {
    const float mag_min = 0.001f*it->second->mag_min;
    float rcmag[2];
    if (mag_converter->computeRCMag(mag_min,getFlagPointStar(),
                                    eye,rcmag) < 0) break;
    rval = it->first;
  }
  return rval;
}

void HipStarMgr::MagConverter::setFov(float fov) {
  if (fov > max_fov) fov = max_fov;
  else if (fov < min_fov) fov = min_fov;
  fov_factor = 108064.73f / (fov*fov);
}

void HipStarMgr::MagConverter::setEye(const ToneReproductor *eye) {
  min_rmag
    = std::sqrt(eye->adapt_luminance(
        std::exp(-0.92103f*(max_scaled_60deg_mag + mag_shift + 12.12331f))
          * (108064.73f / 3600.f))) * 30.f;
//cout << "min_rmag: " << min_rmag << endl;
}

int HipStarMgr::MagConverter::computeRCMag(float mag,bool point_star,
                                        const ToneReproductor *eye,
                                        float rc_mag[2]) const {
  if (mag > max_mag) {
    rc_mag[0] = rc_mag[1] = 0.f;
    return -1;
  }

    // rmag:
  rc_mag[0] = std::sqrt(
           eye->adapt_luminance(
             std::exp(-0.92103f*(mag + mag_shift + 12.12331f)) * fov_factor))
           * 30.f;

  if (rc_mag[0] < min_rmag) {
    rc_mag[0] = rc_mag[1] = 0.f;
    return -1;
  }

  if (point_star) {
    if (rc_mag[0] * mgr.getScale() < 0.1f) { // 0.05f
      rc_mag[0] = rc_mag[1] = 0.f;
      return -1;
    }
    rc_mag[1] = rc_mag[0] * rc_mag[0] / 1.44f;
    if (rc_mag[1] * mgr.getMagScale() < 0.1f) {  // 0.05f
      rc_mag[0] = rc_mag[1] = 0.f;
      return -1;
    }
    // Global scaling
    rc_mag[1] *= mgr.getMagScale();
  } else {
    // if size of star is too small (blink) we put its size to 1.2 --> no more blink
    // And we compensate the difference of brighteness with cmag
    if (rc_mag[0]<1.2f) {
      if (rc_mag[0] * mgr.getScale() < 0.1f) {
        rc_mag[0] = rc_mag[1] = 0.f;
        return -1;
      }
      rc_mag[1] = rc_mag[0] * rc_mag[0] / 1.44f;
      if (rc_mag[1] * mgr.getMagScale() < 0.1f) {
        rc_mag[0] = rc_mag[1] = 0.f;
        return -1;
      }
      rc_mag[0] = 1.2f;
    } else {
        // cmag:
      rc_mag[1] = 1.f;
	  if (rc_mag[0] > mgr.getStarSizeLimit()) {
		  rc_mag[0] = mgr.getStarSizeLimit();
	  }
    }
    // Global scaling
    rc_mag[0] *= mgr.getScale();
    rc_mag[1] *= mgr.getMagScale();
  }
  return 0;
}

// Draw all the stars
double HipStarMgr::draw(Core* core, ToneReproductor* eye, Projector* prj)
{
	Navigator* nav = core->getNavigation();
		
    current_JDay = nav->get_JDay();

    // If stars are turned off don't waste time below
    // projecting all stars just to draw disembodied labels
    if(!starsFader.getInterstate()) return 0.;

    prj->set_orthographic_projection();
	int max_search_level = getMaxSearchLevel(eye, prj);
	const GeodesicSearchResult* geodesic_search_result = core->getGeodesicGrid()->search(prj->unprojectViewport(),max_search_level);

    mag_converter->setFov(prj->get_fov());
    mag_converter->setEye(eye);

    // Set temporary static variable for optimization
    if (flagStarTwinkle) twinkle_amount = twinkleAmount;
    else twinkle_amount = 0;
    const float names_brightness = names_fader.getInterstate() 
                                 * starsFader.getInterstate();
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    // Bind the star texture
    starTexture->bind();

    // Set the draw mode
    glBlendFunc(GL_ONE, GL_ONE);

    // draw all the stars of all the selected zones
    float rcmag_table[2*256];
//static int count = 0;
//count++;
	
    for (ZoneArrayMap::const_iterator it(zone_arrays.begin());
         it!=zone_arrays.end();it++) {
      const float mag_min = 0.001f*it->second->mag_min;
//      if (maxMag < mag_min) break;
      const float k = (0.001f*it->second->mag_range)/it->second->mag_steps;
      for (int i=it->second->mag_steps-1;i>=0;i--) {
        const float mag = mag_min+k*i;
          // Compute the equivalent star luminance for a 5 arc min circle
          // and convert it in function of the eye adaptation
//        rmag_table[i] =
//          eye->adaptLuminance(std::exp(-0.92103f*(mag + 12.12331f)) * 108064.73f)
//            * std::pow(prj->getFov(),-0.85f) * 70.f;
        if (mag_converter->computeRCMag(mag,getFlagPointStar(),eye,
                                        rcmag_table + 2*i) < 0) {
          if (i==0) goto exit_loop;
        }
        if (getFlagPointStar()) {
          rcmag_table[2*i+1] *= starsFader.getInterstate();
        } else {
          rcmag_table[2*i] *= starsFader.getInterstate();
        }
      }
      last_max_search_level = it->first;

      unsigned int max_mag_star_name = 0;
      if (names_fader.getInterstate()) {
        int x = (int)((maxMagStarName-mag_min)/k);
        if (x > 0) max_mag_star_name = x;
      }
      int zone;
      for (GeodesicSearchInsideIterator it1(*geodesic_search_result,it->first);
           (zone = it1.next()) >= 0;) {
        it->second->draw(zone,true,rcmag_table,prj,
                         max_mag_star_name,names_brightness,
                         starFont,starTexture);
      }
      for (GeodesicSearchBorderIterator it1(*geodesic_search_result,it->first);
           (zone = it1.next()) >= 0;) {
        it->second->draw(zone,false,rcmag_table,prj,
                         max_mag_star_name,names_brightness,
                         starFont,starTexture);
      }
    }
    exit_loop:

    prj->reset_perspective_projection();
    return 0.;
}

// Look for a star by XYZ coords
ObjectBaseP HipStarMgr::search(Vec3d pos) const {
assert(0);
  pos.normalize();
  vector<ObjectBaseP > v = searchAround(pos,
                                        0.8, // just an arbitrary number
                                        NULL);
  ObjectBaseP nearest;
  double cos_angle_nearest = -10.0;
  for (vector<ObjectBaseP >::const_iterator it(v.begin());it!=v.end();it++) {
    const double c = (*it)->getObsJ2000Pos(0)*pos;
    if (c > cos_angle_nearest) {
      cos_angle_nearest = c;
      nearest = *it;
    }
  }
  return nearest;
}

// Return a stl vector containing the stars located
// inside the lim_fov circle around position v
vector<ObjectBaseP > HipStarMgr::searchAround(const Vec3d& vv,
                                           double lim_fov, // degrees
										   const Core* core) const {
  vector<ObjectBaseP > result;
  if (!getFlagStars())
  	return result;
  	
  Vec3d v(vv);
  v.normalize();
    // find any vectors h0 and h1 (length 1), so that h0*v=h1*v=h0*h1=0
  int i;
  {
    const double a0 = fabs(v[0]);
    const double a1 = fabs(v[1]);
    const double a2 = fabs(v[2]);
    if (a0 <= a1) {
      if (a0 <= a2) i = 0;
      else i = 2;
    } else {
      if (a1 <= a2) i = 1;
      else i = 2;
    }
  }
  Vec3d h0(0.0,0.0,0.0);
  h0[i] = 1.0;
  Vec3d h1 = h0 ^ v;
  h1.normalize();
  h0 = h1 ^ v;
  h0.normalize();
    // now we have h0*v=h1*v=h0*h1=0.
    // construct a region with 4 corners e0,e1,e2,e3 inside which
    // all desired stars must be:
  double f = 1.4142136 * tan(lim_fov * M_PI/180.0);
  h0 *= f;
  h1 *= f;
  Vec3d e0 = v + h0;
  Vec3d e1 = v + h1;
  Vec3d e2 = v - h0;
  Vec3d e3 = v - h1;
  f = 1.0/e0.length();
  e0 *= f;
  e1 *= f;
  e2 *= f;
  e3 *= f;
    // search the triangles
 	const GeodesicSearchResult* geodesic_search_result = core->getGeodesicGrid()->search(e0,e1,e2,e3,last_max_search_level);
    // iterate over the stars inside the triangles:
  f = cos(lim_fov * M_PI/180.);
  for (ZoneArrayMap::const_iterator it(zone_arrays.begin());
       it!=zone_arrays.end();it++) {
    int zone;
    for (GeodesicSearchInsideIterator it1(*geodesic_search_result,it->first);
         (zone = it1.next()) >= 0;) {
      it->second->searchAround(zone,v,f,result);
    }
    for (GeodesicSearchBorderIterator it1(*geodesic_search_result,it->first);
         (zone = it1.next()) >= 0;) {
      it->second->searchAround(zone,v,f,result);
    }
  }
  return result;
}

//! Update i18 names from english names according to passed translator.
//! The translation is done using gettext with translated strings defined in translations.h
void HipStarMgr::updateI18n(Translator& trans) {
  common_names_map_i18n.clear();
  common_names_index_i18n.clear();
  for (map<int,string>::iterator it(common_names_map.begin());
       it!=common_names_map.end();it++) {
    const int i = it->first;
    const string t(trans.translateUTF8(it->second));
    common_names_map_i18n[i] = t;
    string t_cap = t;
    transform(t.begin(), t.end(), t_cap.begin(), ::toupper);
    common_names_index_i18n[t_cap] = i;
  }
 // starFont = &StelApp::getInstance().getFontManager().getStandardFont(trans.getTrueLocaleName(), fontSize);
}

ObjectBaseP HipStarMgr::search(const string& name) const
{
    const string catalogs("HP HD SAO");

    string n = name;
    for (string::size_type i=0;i<n.length();++i)
    {
        if (n[i]=='_') n[i]=' ';
    }    
    
    istringstream ss(n);
    string cat;
    unsigned int num;
    
    ss >> cat;
    
    // check if a valid catalog reference
    if (catalogs.find(cat,0) == string::npos)
    {
        // try see if the string is a HP number
        istringstream cat_to_num(cat);
        cat_to_num >> num;
        if (!cat_to_num.fail()) return searchHP(num);
        return NULL;
    }

    ss >> num;
    if (ss.fail()) return NULL;

    if (cat == "HP") return searchHP(num);
    assert(0);
    return NULL;
}    

// Search the star by HP number
ObjectBaseP HipStarMgr::searchHP(int _HP) const {
  if (0 < _HP && _HP <= NR_OF_HIP) {
    const Star1 *const s = hip_index[_HP].s;
    if (s) {
      const SpecialZoneArray<Star1> *const a = hip_index[_HP].a;
      const SpecialZoneData<Star1> *const z = hip_index[_HP].z;
      return s->createStelObject(a,z);
    }
  }
  return ObjectBaseP();
}

ObjectBaseP HipStarMgr::searchByNameI18n(const string& nameI18n) const
{
    string objw = nameI18n;
    transform(objw.begin(), objw.end(), objw.begin(), ::toupper);
    
    // Search by HP number if it's an HP formated number
    // Please help, if you know a better way to do this:
    if (nameI18n.length() >= 2 && nameI18n[0]=='H' && nameI18n[1]=='P')
    {
        bool hp_ok = false;
        string::size_type i=2;
        // ignore spaces
        for (;i<nameI18n.length();i++)
        {
            if (nameI18n[i] != ' ') break;
        }
        // parse the number
        unsigned int nr = 0;
        for (;i<nameI18n.length();i++)
        {
            if ( (hp_ok = ('0' <= nameI18n[i] && nameI18n[i] <= '9')) )
            {
                nr = 10*nr+(nameI18n[i]-'0');
            }
            else
            {
                break;
            }
        }
        if (hp_ok)
        {
            return searchHP(nr);
        }
    }

    // Search by I18n common name
  map<string,int>::const_iterator it(common_names_index_i18n.find(objw));
  if (it!=common_names_index_i18n.end()) {
    return searchHP(it->second);
  }

    // Search by sci name
  it = sci_names_index_i18n.find(objw);
  if (it!=sci_names_index_i18n.end()) {
    return searchHP(it->second);
  }

  return ObjectBaseP();
}

ObjectBaseP HipStarMgr::searchByName(const string& name) const
{
    string objw = name;
    transform(objw.begin(), objw.end(), objw.begin(), ::toupper);
    
    // Search by HP number if it's an HP formated number
    // Please help, if you know a better way to do this:
    if (name.length() >= 2 && name[0]=='H' && name[1]=='P')
    {
        bool hp_ok = false;
        string::size_type i=2;
        // ignore spaces
        for (;i<name.length();i++)
        {
            if (name[i] != ' ') break;
        }
        // parse the number
        unsigned int nr = 0;
        for (;i<name.length();i++)
        {
            if ( (hp_ok = ('0' <= name[i] && name[i] <= '9')) )
            {
                nr = 10*nr+(name[i]-'0');
            }
            else
            {
                break;
            }
        }
        if (hp_ok)
        {
            return searchHP(nr);
        }
    }


  map<string,int>::const_iterator it(common_names_index_i18n.find(objw));

/* Should we try this anyway?
    // Search by common name

  if (it!=common_names_index_i18n.end()) {
    return searchHP(it->second);
  }
*/
    // Search by sci name
  it = sci_names_index_i18n.find(objw);
  if (it!=sci_names_index_i18n.end()) {
    return searchHP(it->second);
  }

  return ObjectBaseP();
}

//! Find and return the list of at most maxNbItem objects auto-completing
//! the passed object I18n name
vector<string> HipStarMgr::listMatchingObjectsI18n(
                              const string& objPrefix,
                              unsigned int maxNbItem) const {
  vector<string> result;
  if (maxNbItem==0) return result;

  string objw = objPrefix;
  transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

    // Search for common names
  for (map<string,int>::const_iterator
       it(common_names_index_i18n.lower_bound(objw));
       it!=common_names_index_i18n.end();it++) {
    const string constw(it->first.substr(0,objw.size()));
    if (constw == objw) {
      if (maxNbItem == 0) break;
      result.push_back(getCommonName(it->second));
      maxNbItem--;
    } else {
      break;
    }
  }
    // Search for sci names
  for (map<string,int>::const_iterator
       it(sci_names_index_i18n.lower_bound(objw));
       it!=sci_names_index_i18n.end();it++) {
    const string constw(it->first.substr(0,objw.size()));
    if (constw == objw) {
      if (maxNbItem == 0) break;
      result.push_back(getSciName(it->second));
      maxNbItem--;
    } else {
      break;
    }
  }

  sort(result.begin(), result.end());

  return result;
}

//! Define font file name and size to use for star names display
void HipStarMgr::setFont(float font_size, const string& font_name)
{
	if (starFont) delete starFont;
	starFont = new s_font(font_size, font_name);
	assert(starFont);
}

