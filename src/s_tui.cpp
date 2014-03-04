/*
 * Nightshade (TM) astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
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

// Class which manages a Text User Interface "widgets"

#include <fstream>
#include <cmath>
#include "s_tui.h"
#include "app_settings.h"
#include "shared_data.h"
#include <nshade_state.h>
#include "utility.h"

using namespace std;
using namespace s_tui;

// Same function as getString but cleaned of every color informations
string Component::getCleanString(void)
{
	string result, s(getString());
	for (unsigned int i=0; i<s.length(); ++i) {
		if (s[i]!=start_active[0] && s[i]!=stop_active[0]) result.push_back(s[i]);
	}
	return result;
}

Container::~Container()
{
	list<Component*>::iterator iter = childs.begin();
	while (iter != childs.end()) {
		if (*iter) {
			delete (*iter);
			(*iter)=NULL;
		}
		iter++;
	}
}

void Container::addComponent(Component* c)
{
	childs.push_back(c);
}

string Container::getString(void)
{
	string s;
	list<Component*>::const_iterator iter = childs.begin();
	while (iter != childs.end()) {
		s+=(*iter)->getString();
		iter++;
	}
	return s;
}


bool Container::onKey(Uint16 k, S_TUI_VALUE s)
{
	list<Component*>::iterator iter = childs.begin();
	while (iter != childs.end()) {
		if ((*iter)->onKey(k, s)) return true;	// The signal has been intercepted
		iter++;
	}
	return false;
}

Branch::Branch() : Container()
{
	current = childs.begin();
}

string Branch::getString(void)
{
	if (!*current) return string();
	else return (*current)->getString();
}

void Branch::addComponent(Component* c)
{
	childs.push_back(c);
	if (childs.size()==1) current = childs.begin();
}

bool Branch::setValue(const string& s)
{
	list<Component*>::iterator c;
	for (c=childs.begin(); c!=childs.end(); c++) {
		//wcout << (*c)->getCleanString() << endl;
		if ((*c)->getCleanString()==s) {
			current = c;
			return true;
		}
	}
	return false;
}

bool Branch::setValue_Specialslash(const string& s)
{
	list<Component*>::iterator c;
	for (c=childs.begin(); c!=childs.end(); c++) {
		string cs = (*c)->getCleanString();
		int pos = cs.find('/');
		string ccs = cs.substr(0,pos);
		if (ccs==s) {
			current = c;
			return true;
		}
	}
	return false;
}

bool Branch::onKey(Uint16 k, S_TUI_VALUE v)
{
	if (!*current) return false;
	if (v==S_TUI_RELEASED) return (*current)->onKey(k,v);
	if (v==S_TUI_PRESSED) {
		if ((*current)->onKey(k,v)) return true;
		else {
			if (k==SDLK_UP) {
				if (current!=childs.begin()) --current;
				else current=--childs.end();
				return true;
			}
			if (k==SDLK_DOWN) {
				if (current!=--childs.end()) ++current;
				else current=childs.begin();
				return true;
			}
			return false;
		}
	}
	return false;
}

MenuBranch::MenuBranch(const string& s) : Branch(), label(s), isNavigating(false), isEditing(false)
{
}

bool MenuBranch::onKey(Uint16 k, S_TUI_VALUE v)
{
	if (isNavigating) {
		if (isEditing) {
			if ((*Branch::current)->onKey(k, v)) return true;
			if (v==S_TUI_PRESSED && (k==SDLK_LEFT || k==SDLK_ESCAPE || k==SDLK_RETURN)) {
				isEditing = false;
				return true;
			}
			return false;
		} else {
			if (v==S_TUI_PRESSED && k==SDLK_UP) {
				if (Branch::current!=Branch::childs.begin()) --Branch::current;
				return true;
			}
			if (v==S_TUI_PRESSED && k==SDLK_DOWN) {
				if (Branch::current!=--Branch::childs.end()) ++Branch::current;
				return true;
			}
			if (v==S_TUI_PRESSED && (k==SDLK_RIGHT || k==SDLK_RETURN)) {
				if ((*Branch::current)->isEditable()) isEditing = true;
				return true;
			}
			if (v==S_TUI_PRESSED && (k==SDLK_LEFT || k==SDLK_ESCAPE)) {
				isNavigating = false;
				return true;
			}
			return false;
		}
	} else {
		if (v==S_TUI_PRESSED && (k==SDLK_RIGHT || k==SDLK_RETURN)) {
			isNavigating = true;
			return true;
		}
		return false;
	}
	return false;
}

string MenuBranch::getString(void)
{
	if (!isNavigating) return label;
	if (isEditing) (*Branch::current)->setActive(true);
	string s(Branch::getString());
	if (isEditing) (*Branch::current)->setActive(false);
	return s;
}



MenuBranch_item::MenuBranch_item(const string& s) : Branch(), label(s), isEditing(false)
{
}

bool MenuBranch_item::onKey(Uint16 k, S_TUI_VALUE v)
{
	if (isEditing) {
		if ((*Branch::current)->onKey(k, v)) return true;
		if (v==S_TUI_PRESSED && (k==SDLK_LEFT || k==SDLK_ESCAPE || k==SDLK_RETURN)) {
			isEditing = false;
			return true;
		}
		return false;
	} else {
		if (v==S_TUI_PRESSED && k==SDLK_UP) {
			if (Branch::current!=Branch::childs.begin()) --Branch::current;
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (v==S_TUI_PRESSED && k==SDLK_DOWN) {
			if (Branch::current!=--Branch::childs.end()) ++Branch::current;
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (v==S_TUI_PRESSED && (k==SDLK_RIGHT || k==SDLK_RETURN)) {
			if ((*Branch::current)->isEditable()) isEditing = true;
			return true;
		}
		if (v==S_TUI_PRESSED && (k==SDLK_LEFT || k==SDLK_ESCAPE)) {
			return false;
		}
		return false;
	}
	return false;
}

string MenuBranch_item::getString(void)
{
	if (active) (*Branch::current)->setActive(true);
	string s(label + Branch::getString());
	if (active) (*Branch::current)->setActive(false);
	return s;
}

Boolean_item::Boolean_item(bool init_state, const string& _label, const string& _string_activated,
                           const string& _string_disabled) :
		Bistate(init_state), label(_label)
{
	string_activated = _string_activated;
	string_disabled = _string_disabled;
}

bool Boolean_item::onKey(Uint16 k, S_TUI_VALUE v)
{
	if (v==S_TUI_PRESSED && (k==SDLK_UP || k==SDLK_DOWN) ) {
		state = !state;
		if (!onChangeCallback.empty()) onChangeCallback();
		return true;
	}
	return false;
}

string Boolean_item::getString(void)
{
	return label + (active ? start_active : "") +
	       (state ? string_activated : string_disabled) +
	       (active ? stop_active : "");
}

string Integer::getString(void)
{
	return (active ? start_active : "") + Utility::doubleToString(value) + (active ? stop_active : "");
}

string Decimal::getString(void)
{
	return (active ? start_active : "") + Utility::doubleToString(value) + (active ? stop_active : "");
}


bool Integer_item::onKey(Uint16 k, S_TUI_VALUE v)
{
	if (v==S_TUI_RELEASED) return false;
	if (!numInput) {
		if (k==SDLK_UP) {
			increment();
			if (value>mmax) {
				value = mmax;
				// return true;
			}
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (k==SDLK_DOWN) {
			decrement();
			if (value<mmin) {
				value = mmin;
				// return true;
			}
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}

		if (k==SDLK_0 || k==SDLK_1 || k==SDLK_2 || k==SDLK_3 || k==SDLK_4 || k==SDLK_5 ||
		        k==SDLK_6 || k==SDLK_7 || k==SDLK_8 || k==SDLK_9 || k==SDLK_MINUS) {
			// Start editing with numerical numbers
			numInput = true;
			strInput.clear();

			char c = k;
			strInput += c;

			return true;
		}
		return false;
	} else {	// numInput == true
		if (k==SDLK_RETURN) {
			numInput=false;
			istringstream is(strInput);
			is >> value;
			if (value>mmax) value = mmax;
			if (value<mmin) value = mmin;
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}

		if (k==SDLK_UP) {
			istringstream is(strInput);
			is >> value;
			increment();
			if (value>mmax) value = mmax;
			if (value<mmin) value = mmin;
			strInput = Utility::doubleToString(value);
			return true;
		}
		if (k==SDLK_DOWN) {
			istringstream is(strInput);
			is >> value;
			decrement();
			if (value>mmax) value = mmax;
			if (value<mmin) value = mmin;
			strInput = Utility::doubleToString(value);
			return true;
		}

		if (k==SDLK_0 || k==SDLK_1 || k==SDLK_2 || k==SDLK_3 || k==SDLK_4 || k==SDLK_5 ||
		        k==SDLK_6 || k==SDLK_7 || k==SDLK_8 || k==SDLK_9 || k==SDLK_MINUS) {
			// The user was already editing
			char c = k;
			strInput += c;
			return true;
		}

		if (k==SDLK_ESCAPE) {
			numInput=false;
			return false;
		}
		return true; // Block every other characters
	}

	return false;
}


string Integer_item::getString(void)
{
	if (numInput) return label + (active ? start_active : "") + strInput + (active ? stop_active : "");
	else return label + (active ? start_active : "") + Utility::doubleToString(value) + (active ? stop_active : "");
}


string Password_item::getMask( void ) {
	char aster[128];
	string* editing = NULL;
	if( confirm )
		editing = &value_conf;
	else
		editing = &value;

	unsigned int sz = sizeof(aster);
	memset( aster, '*', sz);
	unsigned int len = editing->length();
	if( len > sz )
		len = sz;

	return string( aster, len );
}


bool Password_item::onKey(Uint16 k, S_TUI_VALUE v)
{
	if (v==S_TUI_RELEASED)
		return false;

	string* editing = NULL;

	if( confirm )
		editing = &value_conf;
	else
		editing = &value;

	if ( k > 31 && k < 128 ) {
		if( init ) {
			value.clear();
			init = false;
		}
		*editing += (char)k;
		return true;
	}
	else if( k == SDLK_BACKSPACE || k == SDLK_DELETE ) {
		editing->clear();
		return true;
	}
	else if (k==SDLK_RETURN) {
		if (!confirm && value.length() > 3 ) {
			confirm = true;
		}
		else if( !confirm ) {
			tempo = time(NULL);
			dispStr = &string_moreChars;
			value = "********";
			init = true;
		}
		else if( value.compare(value_conf) == 0 ) {
			system( (AppSettings::Instance()->getDataRoot() + "/data/script_update_htpasswd " + value).c_str() );
			tempo = time(NULL);
			dispStr = &string_success;
			value_conf.clear();
			value = "********";
			init = true;
			confirm = false;
			return false;
		}
		else {
			tempo = time(NULL);
			dispStr = &string_error;
			value_conf.clear();
			value = "********";
			init = true;
			confirm = false;
		}

		return true;
	}
	else if (k==SDLK_ESCAPE || k==SDLK_LEFT) {
		init = true;
		confirm = false;
		value_conf.clear();
		value = "********";
		return false;
	}

	return false;
}


void Password_item::translateActions()
{
	string_confirm   = _(english_string_confirm);
	string_error     = _(english_string_error);
	string_success   = _(english_string_success);
	string_moreChars = _(english_string_moreChars);
}


string Password_item::getString( void ) {
	if( dispStr && difftime(time(NULL), tempo) < 3 )
		return label + (active ? start_active : "") + *dispStr + (active ? stop_active : "");
	else if( confirm )
		return string_confirm + (active ? start_active : "") + getMask() + (active ? stop_active : "");
	else
		return label + (active ? start_active : "") + getMask() + (active ? stop_active : "");
}


// logarithmic steps
int Integer_item_logstep::increment()
{

	for (int i=1; i<10; i++) {
		if (value < pow(10.f, i)) {
			value += int(pow(10.f, i-1));
			return value;
		}
	}
	return (++value);
}

// logarithmic steps
int Integer_item_logstep::decrement()
{

	for (int i=10; i>0; i--) {
		if (value > pow(10.f, i)) {
			if (value >= 2 * pow(10.f, i)) {
				value -= int(pow(10.f, i));
			} else {
				value -= int(pow(10.f, i-1));
			}
			return value;
		}
	}
	return(--value);
}




bool Decimal_item::onKey(Uint16 k, S_TUI_VALUE v)
{
	if (v==S_TUI_RELEASED) return false;
	if (!numInput) {
		if (k==SDLK_UP) {
			value+=delta;
			if (value>mmax) {
				if (wrap) value = mmin + (value-mmax);
				else value = mmax;
				// return true;
			}
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (k==SDLK_DOWN) {
			value-=delta;
			if (value<mmin) {
				if (wrap) value = mmax - (mmin-value);
				else value = mmin;
				// return true;
			}
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}

		if (k==SDLK_0 || k==SDLK_1 || k==SDLK_2 || k==SDLK_3 || k==SDLK_4 || k==SDLK_5 ||
		        k==SDLK_6 || k==SDLK_7 || k==SDLK_8 || k==SDLK_9 || k==SDLK_PERIOD || k==SDLK_MINUS) {
			// Start editing with numerical numbers
			numInput = true;
			strInput.clear();

			char c = k;
			strInput += c;

			return true;
		}
	} else {	// numInput == true
		if (k==SDLK_RETURN || k==SDLK_LEFT || k==SDLK_RIGHT) {
			numInput=false;
			istringstream is(strInput);
			is >> value;
			if (value>mmax) value = mmax;
			if (value<mmin) value = mmin;
			if (!onChangeCallback.empty()) onChangeCallback();

// somewhat improved
			if (k==SDLK_RETURN) return false;
			else return true;
		}

		if (k==SDLK_UP) {
			istringstream is(strInput);
			is >> value;
			value+=delta;
			if (value>mmax) value = mmax;
			if (value<mmin) value = mmin;
			strInput = Utility::doubleToString(value);
			return true;
		}
		if (k==SDLK_DOWN) {
			istringstream wistemp(strInput);
			wistemp >> value;
			value-=delta;
			if (value>mmax) value = mmax;
			if (value<mmin) value = mmin;
			strInput = Utility::doubleToString(value);
			return true;
		}

		if (k==SDLK_0 || k==SDLK_1 || k==SDLK_2 || k==SDLK_3 || k==SDLK_4 || k==SDLK_5 ||
		        k==SDLK_6 || k==SDLK_7 || k==SDLK_8 || k==SDLK_9 || k==SDLK_PERIOD || k==SDLK_MINUS) {
			// The user was already editing
			wchar_t c = (wchar_t)k;
			strInput += c;
			return true;
		}

		if (k==SDLK_ESCAPE) {
			numInput=false;
			return false;
		}
		return true; // Block every other characters
	}

	return false;
}

string Decimal_item::getString(void)
{
	// Can't directly write value in os because there is a float precision limit bug..
	static char tempstr[16];
	sprintf(tempstr,"%.2f", value);
	string vstr = tempstr;

	if (numInput) return label + (active ? start_active : "") + strInput + (active ? stop_active : "");
	else return label + (active ? start_active : "") + vstr + (active ? stop_active : "");
}


Time_item::Time_item(const string& _label, double _JD) :
		CallbackComponent(), JD(_JD), current_edit(NULL), label(_label),
		y(NULL), m(NULL), d(NULL), h(NULL), mn(NULL), s(NULL)
{
	// Note: range limits are 1 beyond normal range to allow for rollover
	// as date is calculated after updates and range limits are enforced then.
	// trystan 10-7-2010: enforce a max negative date of -4712. The Julian calendar
	// is undefined for lesser years and the math falls apart.
	y = new Integer_item(NShadeDateTime::getMinSimulationJD(), 
						 NShadeDateTime::getMaxSimulationJD(), 2000);
	m = new Integer_item(0, 13, 1);
	d = new Integer_item(0, 32, 1);
	h = new Integer_item(-1, 24, 0);
	mn = new Integer_item(-1, 60, 0);
	s = new Integer_item(-1, 60, 0);
	current_edit = y;
}

Time_item::~Time_item()
{
	delete y;
	delete m;
	delete d;
	delete h;
	delete mn;
	delete s;
}

bool Time_item::onKey(Uint16 k, S_TUI_VALUE v)
{
	if (v==S_TUI_RELEASED) return false;

	if (current_edit->onKey(k,v)) {
		compute_JD();
		compute_ymdhms();
		if (!onChangeCallback.empty()) onChangeCallback();
		return true;
	} else {
		if (k==SDLK_ESCAPE) {
			return false;
		}

		if (k==SDLK_RIGHT) {
			if (current_edit==y) current_edit=m;
			else if (current_edit==m) current_edit=d;
			else if (current_edit==d) current_edit=h;
			else if (current_edit==h) current_edit=mn;
			else if (current_edit==mn) current_edit=s;
			else if (current_edit==s) current_edit=y;
			return true;
		}
		if (k==SDLK_LEFT) {
			if (current_edit==y) current_edit=s;
			else if (current_edit==m) current_edit=y;
			else if (current_edit==d) current_edit=m;
			else if (current_edit==h) current_edit=d;
			else if (current_edit==mn) current_edit=h;
			else if (current_edit==s) current_edit=mn;
			return true;
		}
	}

	return false;
}

// Convert Julian day to yyyy/mm/dd hh:mm:ss and return the string
string Time_item::getString(void)
{
	compute_ymdhms();

	string s1[6];
	string s2[6];
	if (current_edit==y && active) {
		s1[0] = start_active;
		s2[0] = stop_active;
	}
	if (current_edit==m && active) {
		s1[1] = start_active;
		s2[1] = stop_active;
	}
	if (current_edit==d && active) {
		s1[2] = start_active;
		s2[2] = stop_active;
	}
	if (current_edit==h && active) {
		s1[3] = start_active;
		s2[3] = stop_active;
	}
	if (current_edit==mn && active) {
		s1[4] = start_active;
		s2[4] = stop_active;
	}
	if (current_edit==s && active) {
		s1[5] = start_active;
		s2[5] = stop_active;
	}

	return label +
	       s1[0] + y->getString() + s2[0] + "/" +
	       s1[1] + m->getString() + s2[1] + "/" +
	       s1[2] + d->getString() + s2[2] + " " +
	       s1[3] + h->getString() + s2[3] + ":" +
	       s1[4] + mn->getString() + s2[4] + ":" +
	       s1[5] + s->getString() + s2[5];
}


// for use with commands - no special characters, just the local date
string Time_item::getDateString(void)
{
	compute_ymdhms();  // possibly redundant

	return y->getString() + ":" +
	       m->getString() + ":" +
	       d->getString() + "T" +
	       h->getString() + ":" +
	       mn->getString() + ":" +
	       s->getString();
}

// trystan 10-7-2010: clean-up to use consistent date conversion algorithms
void Time_item::compute_ymdhms(void)
{
	int year, month, day, hour, minute;
	NShadeDateTime::DateTimeFromJulianDay(JD, &year, &month, &day, &hour, &minute, &second);

	y->setValue(year);
	m->setValue(month);
	d->setValue(day);
	h->setValue(hour);
	mn->setValue(minute);
	s->setValue(floor(second));
}

// trystan 10-7-2010: clean-up to use consistent date conversion algorithms
void Time_item::compute_JD(void)
{
	JD = NShadeDateTime::JulianDayFromDateTime(y->getValue(), m->getValue(), d->getValue(),
			h->getValue(), mn->getValue(), second);
}


// Widget used to set time zone. Initialized from a file of type /usr/share/zoneinfo/zone.tab
Time_zone_item::Time_zone_item(const string& zonetab_file, const string& _label) : label(_label)
{
	if (zonetab_file.empty()) {
		cerr << "Can't find file \"" << zonetab_file << "\"\n" ;
		exit(0);
	}

	ifstream is(zonetab_file.c_str());

	string unused, tzname;
	char zoneline[256];
	int i;

	while (is.getline(zoneline, 256)) {
		if (zoneline[0]=='#') continue;
		istringstream istr(zoneline);
		istr >> unused >> unused >> tzname;

		if( SharedData::Instance()->DB() ){
			TZRecord rec( tzname.c_str() );
			insert(rec);
		}

		i = tzname.find("/");
		if (continents.find(tzname.substr(0,i))==continents.end()) {
			continents.insert(pair<string, MultiSet_item<string> >(tzname.substr(0,i),MultiSet_item<string>()));
			continents[tzname.substr(0,i)].addItem(tzname.substr(i+1,tzname.size()));
			continents_names.addItem(tzname.substr(0,i));
		} else {
			continents[tzname.substr(0,i)].addItem(tzname.substr(i+1,tzname.size()));
		}
	}

	if( SharedData::Instance()->DB() ) {
		SharedData::Instance()->DB()->commit();
	}

	is.close();
	current_edit=&continents_names;
}

Time_zone_item::~Time_zone_item(){
	if( SharedData::Instance()->DB()) {
		dbCursor<TZRecord> cursor(dbCursorForUpdate);
		if( cursor.select() )
			cursor.removeAllSelected();
		SharedData::Instance()->DB()->commit();
	}
}

bool Time_zone_item::onKey(Uint16 k, S_TUI_VALUE v)
{
	if (v==S_TUI_RELEASED) return false;

	if (current_edit->onKey(k,v)) {
		if (!onChangeCallback.empty()) onChangeCallback();
		return true;
	} else {
		if (k==SDLK_ESCAPE) {
			return false;
		}

		if (k==SDLK_RIGHT) {
			if (current_edit==&continents_names) current_edit = &continents[continents_names.getCurrent()];
			else current_edit=&continents_names;
			return true;
		}
		if (k==SDLK_LEFT) {
			if (current_edit==&continents_names) return false;
			else current_edit = &continents_names;
			return true;
		}
	}

	return false;
}

string Time_zone_item::getString(void)
{
	string s1[2], s2[2];
	if (current_edit==&continents_names && active) {
		s1[0] = start_active;
		s2[0] = stop_active;
	}
	if (current_edit!=&continents_names && active) {
		s1[1] = start_active;
		s2[1] = stop_active;
	}

	return label + s1[0] + continents_names.getCurrent() + s2[0] + "/" + s1[1] +
	       continents[continents_names.getCurrent()].getCurrent() + s2[1];
}

string Time_zone_item::gettz(void)   // should be const but gives a boring error...
{
	if (continents.find(continents_names.getCurrent())!=continents.end())
		return continents_names.getCurrent() + "/" + continents[continents_names.getCurrent()].getCurrent();
	else return continents_names.getCurrent() + "/error" ;
}

void Time_zone_item::settz(const string& tz)
{
	int i = tz.find("/");
	continents_names.setCurrent(tz.substr(0,i));
	continents[continents_names.getCurrent()].setCurrent(tz.substr(i+1,tz.size()));
}


string Action_item::getString(void)
{
	if (difftime(time(NULL), tempo) > 3) {
		if (active) {
			return label + start_active + string_prompt1 + stop_active;
		} else return label + string_prompt1;
	} else {
		if (active) {
			return label + start_active + string_prompt2 + stop_active;
		} else return label + string_prompt2;
	}
}

bool Action_item::onKey(Uint16 k, S_TUI_VALUE v)
{
	if (v==S_TUI_PRESSED && k==SDLK_RETURN) {
		// Call the callback if enter is pressed
		if (!onChangeCallback.empty()) onChangeCallback();
		tempo = time(NULL);
		return false; // menubranch will now make inactive
	}
	return false;
}

void Action_item::translateActions()
{
	string_prompt1 = _(english_string_prompt1);
	string_prompt2 = _(english_string_prompt2);
}

string ActionConfirm_item::getString(void)
{

	if (difftime(time(NULL), tempo) < 3) {
		return label + string_prompt2;
	}

	if (active) {
		if (isConfirming) {
			return label + start_active + string_confirm + stop_active;
		} else {
			return label + start_active + string_prompt1 + stop_active;
		}
	} else return label + string_prompt1;
}

bool ActionConfirm_item::onKey(Uint16 k, S_TUI_VALUE v)
{

	if (difftime(time(NULL), tempo) < 3) return false;  // no edits while displays Done message

	if (v==S_TUI_PRESSED) {

		if(k==SDLK_RETURN) {
			if (isConfirming) {
				// Call the callback if enter is pressed
				if (!onChangeCallback.empty()) onChangeCallback();
				tempo = time(NULL);
				isConfirming = false;
				return false; // menubranch will now make inactive
			} else {
				isConfirming = true;
				return true;
			}
		} else if(k==SDLK_ESCAPE || k==SDLK_LEFT) {  // NOTE: SDLK_LEFT is not working here
			if (isConfirming) {
				isConfirming = false;
				return true;
			}
			return false; // menubranch will now make inactive
		} else {
			return true;  
		}
	}
	return false;
}

void ActionConfirm_item::translateActions()
{
	string_prompt1 = _(english_string_prompt1);
	string_prompt2 = _(english_string_prompt2);
	string_confirm = _(english_string_confirm);
}


Vector_item::Vector_item(const string& _label, Vec3d _init_vector) :
		CallbackComponent(), current_edit(NULL), label(_label),
		a(NULL), b(NULL), c(NULL)
{
	a = new Decimal_item(0, 1, 0, "", 0.05);
	b = new Decimal_item(0, 1, 0, "", 0.05);
	c = new Decimal_item(0, 1, 0, "", 0.05);
	current_edit = a;
	setVector(_init_vector);
}

Vector_item::~Vector_item()
{
	delete a;
	delete b;
	delete c;
}

bool Vector_item::onKey(Uint16 k, S_TUI_VALUE v)
{
	if (v==S_TUI_RELEASED) return false;

	if (current_edit->onKey(k,v)) {

		if (!onChangeCallback.empty()) onChangeCallback();
		return true;
	} else {

// somewhat improved
		if (k==SDLK_RETURN) {
			if (!onChangeCallback.empty()) onChangeCallback();
			return false;
		}


		if (k==SDLK_ESCAPE) {
			return false;
		}

		if (k==SDLK_RIGHT) {
			if (current_edit==a) current_edit=b;
			else if (current_edit==b) current_edit=c;
			else if (current_edit==c) current_edit=a;
			return true;
		}
		if (k==SDLK_LEFT) {
			if (current_edit==a) current_edit=c;
			else if (current_edit==c) current_edit=b;
			else if (current_edit==b) current_edit=a;
			return true;
		}
	}

	return false;
}


string Vector_item::getString(void)
{
	string s1[3];
	string s2[3];
	if (current_edit==a && active) {
		s1[0] = start_active;
		s2[0] = stop_active;
	}
	if (current_edit==b && active) {
		s1[1] = start_active;
		s2[1] = stop_active;
	}
	if (current_edit==c && active) {
		s1[2] = start_active;
		s2[2] = stop_active;
	}

	return label +
	       s1[0] + a->getString() + s2[0] + " " +
	       s1[1] + b->getString() + s2[1] + " " +
	       s1[2] + c->getString() + s2[2] + " ";
}

// Specialization for strings because operator wstream << string does not exists..
template<>
string MultiSet_item<string>::getString(void)
{
	if (current==items.end()) return label;
	return label + (active ? start_active : "") + *current + (active ? stop_active : "");
}
