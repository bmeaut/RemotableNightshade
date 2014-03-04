#include "mapping.h"
#include "mapping_classes.h"
#include "app_settings.h"

std::map<Mapping::MappingType, Mapping*> Mapping::m_mapMap;

Mapping::Mapping() : minFov(0.0001)
{
	InitParser conf;
	conf.load(AppSettings::Instance()->getConfigFile());

	// Inherited classes will set a default maxFov if this base class sets it to zero
	maxFov = conf.get_double("projection", "max_fov", 0);
}

Mapping::~Mapping() {
	if( m_mapMap[m_type] ) {
		delete m_mapMap[m_type];
		m_mapMap[m_type] = NULL;
	}
}

Mapping* const Mapping::Create( Mapping::MappingType type ) {
	if( !m_mapMap[type] ) {
		switch( type ) {
		case Mapping::CYLINDER:
			m_mapMap[type] = new MappingCylinder;
			break;
		case Mapping::EQUALAREA:
			m_mapMap[type] = new MappingEqualArea;
			break;
		case Mapping::FISHEYE:
			m_mapMap[type] = new MappingFisheye;
			break;
		case Mapping::ORTHOGRAPHIC:
			m_mapMap[type] = new MappingOrthographic;
			break;
		case Mapping::PERSPECTIVE:
			m_mapMap[type] = new MappingPerspective;
			break;
		case Mapping::STEREOGRAPHIC:
			m_mapMap[type] = new MappingStereographic;
			break;
		default:
			m_mapMap[type] = new MappingFisheye;
			break;
		}
	}

	m_mapMap[type]->m_type = type;
	return m_mapMap[type];
}

