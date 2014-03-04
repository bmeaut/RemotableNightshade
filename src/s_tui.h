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

#ifndef _TUI_H_
#define _TUI_H_

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "utility.h"
#include "translator.h"
#include <ctime>

// SDL is used only for the key codes, i'm lasy to redefine them
// This is TODO to make the s_ library independent
#include "SDL.h"

#include <set>
#include <list>
#include <map>
#include <string>
#include <iostream>
#include <sstream>

#include "vecmath.h"
#include "callbacks.hpp"

using namespace std;
using namespace boost;


namespace s_tui
{
// tui Return Values:
enum S_TUI_VALUE {
	S_TUI_PRESSED,
	S_TUI_RELEASED
};

// Pango specific markup to produce hilighting of active edit areas in TUI
static const string start_active("<span foreground=\"white\">");  // white is hilight
static const string stop_active("</span>");

static const string start_tui("<span foreground=\"#7FFF7F\">");
static const string end_tui("</span>");

// Base class. Note that the method bool isEditable(void) has to be overrided by returning true
// for all the non passives components.
class Component
{
public:
	Component() : active(false) {
		;
	}
	virtual ~Component() {
		;
	}
	virtual string getString(void) {
		return string();
	}
	virtual string getCleanString(void);
	// Return true if key signal intercepted, false if not
	virtual bool onKey(Uint16, S_TUI_VALUE) {
		return false;
	}
	virtual bool isEditable(void) const {
		return false;
	}
	void setActive(bool a) {
		active = a;
	}
	bool getActive(void) const {
		return active;
	}
protected:
	bool active;
};

// simple display a string component
class Display : public Component
{
public:
	Display(string _label, string _value) : Component(), label(_label), value(_value) {
		;
	}
	string getString(void) {
		return label + value;
	}
	string getCleanString(void) {
		return label + value;
	}
	void setLabel(const string& _label) {
		label = _label;
	}
protected:
	string label;
	string value;
};

// Store a callback on a function taking no parameters
class CallbackComponent : public Component
{
public:
	virtual void set_OnChangeCallback(const callback<void>& c) {
		onChangeCallback = c;
	}
protected:
	callback<void> onChangeCallback;
};

// Manage lists of components
class Container : public CallbackComponent
{
public:
	virtual ~Container();
	virtual string getString(void);
	virtual void addComponent(Component*);
	virtual bool onKey(Uint16, S_TUI_VALUE);
protected:
	list<Component*> childs;
};

// Component which manages 2 states
class Bistate : public CallbackComponent
{
public:
	Bistate(bool init_state = false) : CallbackComponent(), state(init_state) {
		;
	}
	virtual string getString(void) {
		return state ? string_activated : string_disabled;
	}
	bool getValue(void) const {
		return state;
	}
	void setValue(bool s) {
		state = s;
	}
protected:
	string string_activated;
	string string_disabled;
	bool state;
};

// Component which manages integer value
class Integer : public CallbackComponent
{
public:
	Integer(int init_value = 0) : CallbackComponent(), value(init_value) {
		;
	}
	virtual string getString(void);
	int getValue(void) const {
		return value;
	}
	void setValue(int v) {
		value = v;
	}
protected:
	int value;
};

// Boolean item widget. The callback function is called when the state is changed
class Boolean_item : public Bistate
{
public:
	Boolean_item(bool init_state = false, const string& _label = string(),
	             const string& _string_activated = string("ON"),
	             const string& string_disabled  = string("OFF"));
	virtual bool onKey(Uint16, S_TUI_VALUE);
	virtual string getString(void);
	virtual bool isEditable(void) const {
		return true;
	}
	void setLabel(const string& _label, const string& _active, const string& _disabled) {
		label = _label;
		string_activated=_active;
		string_disabled=_disabled;
	}
protected:
	string label;
};

// Component which manages decimal (double) value
class Decimal : public CallbackComponent
{
public:
	Decimal(double init_value = 0.) : CallbackComponent(), value(init_value) {
		;
	}
	virtual string getString(void);
	double getValue(void) const {
		return value;
	}
	void setValue(double v) {
		value = v;
	}
protected:
	double value;
};

// Integer item widget. The callback function is called when the value is changed
class Integer_item : public Integer
{
public:
	Integer_item(int _min, int _max, int init_value, const string& _label = string()) :
			Integer(init_value), numInput(false), mmin(_min), mmax(_max), label(_label) {
		;
	}
	virtual string getString(void);
	virtual bool isEditable(void) const {
		return true;
	}
	virtual bool onKey(Uint16, S_TUI_VALUE);
	void setLabel(const string& _label) {
		label = _label;
	}
	virtual int increment(void) {
		return ++value;
	}
	virtual int decrement(void) {
		return --value;
	}
protected:
	bool numInput;
	string strInput;
	int mmin, mmax;
	string label;
};

// Password item widget. The callback function is called when the value is changed
class Password_item : public CallbackComponent
{
public:
	Password_item(string init_value, const string& _label = string(), string confirm = string(),
			string error = string(), string success = string(), string moreChars = string() ) :
				CallbackComponent(), confirm(false), init(true),
				label(_label), value(init_value), english_string_error(error), english_string_success(success),
				english_string_confirm(confirm), english_string_moreChars(moreChars), dispStr(NULL) {};

	virtual bool isEditable(void) const {
		return true;
	}
	virtual string getString(void);
	virtual bool onKey(Uint16, S_TUI_VALUE);
	virtual void translateActions();
	void setLabel(const string& _label) {
		label = _label;
	}

private:
	bool confirm;
	bool init;
	time_t tempo;
	string label;
	string value;
	string value_conf;
	string english_string_error;
	string english_string_success;
	string english_string_confirm;
	string english_string_moreChars;
	string string_confirm;
	string string_error;
	string string_success;
	string string_moreChars;
	string* dispStr;

	string getMask( void );
};

// logarithmic steps
class Integer_item_logstep : public Integer_item
{
public:
	Integer_item_logstep(int _min, int _max, int init_value, const string& _label = string()) :
			Integer_item(_min, _max, init_value, _label) {
		;
	}

	virtual int increment(void);
	virtual int decrement(void);
};


// added wrap option (for latitude for example)
// Decimal item widget. The callback function is called when the value is changed
class Decimal_item : public Decimal
{
public:
	Decimal_item(double _min, double _max, double init_value, const string& _label = string(), double _delta = 1.0, bool _wrap = false) :
			Decimal(init_value), numInput(false), mmin(_min), mmax(_max), label(_label), delta(_delta) {
		wrap = _wrap;
	}
	virtual string getString(void);
	virtual bool isEditable(void) const {
		return true;
	}
	virtual bool onKey(Uint16, S_TUI_VALUE);
	void setLabel(const string& _label) {
		label = _label;
	}
protected:
	bool numInput;
	string strInput;
	double mmin, mmax;
	string label;
	double delta;
	bool wrap;
};

// Passive widget which only display text
class Label_item : public Component
{
public:
	Label_item(const string& _label) : Component(), label(_label) {
		;
	}
	virtual string getString(void) {
		return label;
	}
	void setLabel(const string& s) {
		label=s;
	}
protected:
	string label;
};

// Manage list of components with one of them selected.
// Can navigate thru the components list with the arrow keys
class Branch : public Container
{
public:
	Branch();
	virtual string getString(void);
	virtual bool onKey(Uint16, S_TUI_VALUE);
	virtual void addComponent(Component*);
	virtual Component* getCurrent(void) const {
		if (current==childs.end()) return NULL;
		else return *current;
	}
	virtual bool setValue(const string&);
	virtual bool setValue_Specialslash(const string&);
protected:
	list<Component*>::const_iterator current;
};

// Base widget used for tree construction. Can navigate thru the components list with the arrow keys.
// Activate the currently edited widget.
class MenuBranch : public Branch
{
public:
	MenuBranch(const string& s);
	virtual bool onKey(Uint16, S_TUI_VALUE);
	virtual string getString(void);
	virtual bool isEditable(void) const {
		return true;
	}
	string getLabel(void) const {
		return label;
	}
	void setLabel(const string& _label) {
		label = _label;
	}
protected:
	string label;
	bool isNavigating;
	bool isEditing;
};

// Widget quite like Menu Branch but always navigating, and always display the label
class MenuBranch_item : public Branch
{
public:
	MenuBranch_item(const string& s);
	virtual bool onKey(Uint16, S_TUI_VALUE);
	virtual string getString(void);
	virtual bool isEditable(void) const {
		return true;
	}
	string getLabel(void) const {
		return label;
	}
protected:
	string label;
	bool isEditing;
};


// Widget used to set time and date. The internal format is the julian day notation
class Time_item : public CallbackComponent
{
public:
	Time_item(const string& _label = string(), double _JD = 2451545.0);
	~Time_item();
	virtual bool onKey(Uint16, S_TUI_VALUE);
	virtual string getString(void);
	virtual string getDateString(void);
	virtual bool isEditable(void) const {
		return true;
	}
	double getJDay(void) const {
		return JD;
	}
	void setJDay(double jd) {
		JD = jd;
	}
	void setLabel(const string& _label) {
		label = _label;
	}
protected:
	void compute_ymdhms(void);
	void compute_JD(void);
	double JD;
	double second;
	Integer_item* current_edit;	// 0 to 5 year to second
	string label;
	Integer_item *y, *m, *d, *h, *mn, *s;
};


// Widget which simply launch the callback when the user press enter
class Action_item : public CallbackComponent
{
public:
	Action_item(const string& _label = "", const string& sp1 = "Do", const string& sp2 = "Done") :
			CallbackComponent(), label(_label), english_string_prompt1(sp1), english_string_prompt2(sp2) {
		tempo = 0;
	}
	virtual bool onKey(Uint16, S_TUI_VALUE);
	virtual string getString(void);
	virtual bool isEditable(void) const {
		return true;
	}
	void setLabel(const string& _label) {
		label = _label;
	}
	virtual void translateActions();

protected:
	string label;
	string english_string_prompt1;
	string english_string_prompt2;
	string string_prompt1;
	string string_prompt2;
	time_t tempo;
};

// Same as before but ask for a confirmation
class ActionConfirm_item : public Action_item
{
public:
	ActionConfirm_item(const string& _label = "", const string& sp1 = "Do", const string& sp2 = "Done",	const string& sc = "Are you sure ?") :
			Action_item(_label, sp1, sp2), isConfirming(false), english_string_confirm(sc) {
		;
	}
	virtual bool onKey(Uint16, S_TUI_VALUE);
	virtual string getString(void);
	virtual void translateActions();

protected:
	bool isConfirming;
	string english_string_confirm;
	string string_confirm;
};

// List item widget. The callback function is called when the selected item changes
template <class T>
class MultiSet_item : public CallbackComponent
{
public:
	MultiSet_item(const string& _label = string()) : CallbackComponent(), label(_label) {
		current = items.end();
	}
	MultiSet_item(const MultiSet_item& m) : CallbackComponent(), label(m.label) {
		setCurrent(m.getCurrent());
	}
	virtual string getString(void) {
		if (current==items.end()) return label;
		return label + (active ? start_active : "") + *current + (active ? stop_active : "");
	}
	virtual bool isEditable(void) const {
		return true;
	}
	virtual bool onKey(Uint16 k, S_TUI_VALUE v) {
		if (current==items.end() || v==S_TUI_RELEASED) return false;
		if (k==SDLK_RETURN) {
			if (!onTriggerCallback.empty()) onTriggerCallback();
			return false;
		}
		if (k==SDLK_UP) {
			if (current!=items.begin()) --current;
			else current = --items.end();
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (k==SDLK_DOWN) {
			if (current!= --items.end()) ++current;
			else current = items.begin();
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (k==SDLK_LEFT || k==SDLK_ESCAPE) return false;
		return false;
	}
	void addItem(const T& newitem) {
		items.insert(newitem);
		if (current==items.end()) current = items.begin();
	}
	void addItemList(const string& s) {
		istringstream is(s);
		T elem;
		while (getline(is, elem)) {
			addItem(elem);
		}
	}
	void replaceItemList(string s, int selection) {
		items.clear();
		addItemList(s);
		current = items.begin();
		for (int j=0; j<selection; j++) {
			++current;
		}
	}
	const T& getCurrent(void) const {
		if (current==items.end()) return emptyT;
		else return *current;
	}
	void setCurrent(const T& i) {
		current = items.find(i);

		// if not found, set to first item!
		if (current==items.end()) {
			current = items.begin();
			if (!onChangeCallback.empty()) onChangeCallback();
		}
	}
	bool setValue(const T& i) {
		if (items.find(i) == items.end()) return false;
		else current = items.find(i);
		return true;
	}
	string getLabel(void) const {
		return label;
	}
	virtual void set_OnTriggerCallback(const callback<void>& c) {
		onTriggerCallback = c;
	}
	void setLabel(const string& _label) {
		label = _label;
	}
protected:
	T emptyT;
	multiset<T> items;
	typename multiset<T>::iterator current;
	string label;
	callback<void> onTriggerCallback;
};

// Specialization for strings because operator wstream << string does not exists..
template<>
string MultiSet_item<string>::getString(void);


// List item widget with separation between UI keys (will be translated) and code value (never translated).
// Assumes one-to-one mapping of keys to values
// The callback function is called when the selected item changes
template <class T>
class MultiSet2_item : public CallbackComponent
{
public:
	MultiSet2_item(const string& _label = string()) : CallbackComponent(), label(_label) {
		current = items.end();
	}
	MultiSet2_item(const MultiSet2_item& m) : CallbackComponent(), label(m.label) {
		setCurrent(m.getCurrent());
	}
	virtual string getString(void) {
		if (current==items.end()) return label;
		return label + (active ? start_active : "") + *current + (active ? stop_active : "");
	}
	virtual bool isEditable(void) const {
		return true;
	}
	virtual bool onKey(Uint16 k, S_TUI_VALUE v) {
		if (current==items.end() || v==S_TUI_RELEASED) return false;
		if (k==SDLK_RETURN) {
			if (!onTriggerCallback.empty()) onTriggerCallback();
			return false;
		}
		if (k==SDLK_UP) {
			if (current!=items.begin()) --current;
			else current = --items.end();
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (k==SDLK_DOWN) {
			if (current!= --items.end()) ++current;
			else current = items.begin();
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (k==SDLK_LEFT || k==SDLK_ESCAPE) return false;
		return false;
	}
	void addItem(const T& newkey, const T& newvalue) {
		items.insert(newkey);
		value[newkey] = newvalue;
		if (current==items.end()) current = items.begin();
	}
	void addItemList(string s) { // newline delimited, key and value alternate
		istringstream is(s);
		T key, value;
		while (getline(is, key) && getline(is, value)) {
			addItem(key, value);
		}
	}
	void replaceItemList(string s, int selection) {
		items.clear();
		value.clear();
		addItemList(s);
		current = items.begin();
		for (int j=0; j<selection; j++) {
			++current;
		}
	}
	const T& getCurrent(void) {
		if (current==items.end()) return emptyT;
		else return value[(*current)];
	}
	void setCurrent(const T& i) {  // set by value, not key

		typename multiset<T>::iterator iter;

		bool found =0;
		for (iter=items.begin(); iter!=items.end(); iter++ ) {
			if ( i == value[(*iter)]) {
				current = iter;
				found = 1;
				break;
			}
		}

		if (!found) current = items.begin();
		if (!onChangeCallback.empty()) onChangeCallback();
	}

	bool setValue(const T& i) {
		typename multiset<T>::iterator iter;

		bool found =0;
		for (iter=items.begin(); iter!=items.end(); iter++ ) {
			if ( i == value[(*iter)]) {
				current = iter;
				found = 1;
				break;
			}
		}
		return found;
	}
	string getLabel(void) const {
		return label;
	}
	virtual void set_OnTriggerCallback(const callback<void>& c) {
		onTriggerCallback = c;
	}
	void setLabel(const string& _label) {
		label = _label;
	}
protected:
	T emptyT;
	multiset<T> items;
	typename multiset<T>::iterator current;
	string label;
	callback<void> onTriggerCallback;
	map<T, T> value;  // hash of key, value pairs
};


// List item widget. NOT SORTED (FIFO). The callback function is called when the selected item changes
template <class T>
class List_item : public CallbackComponent
{
public:
	List_item(const string& _label = string()) : CallbackComponent(), label(_label) {
		current = items.end();
	}
	List_item(const List_item& m) : CallbackComponent(), label(m.label) {
		setCurrent(m.getCurrent());
	}
	virtual string getString(void) {
		if (current==items.end()) return label;
		return label + (active ? start_active : "") + *current + (active ? stop_active : "");
	}
	virtual bool isEditable(void) const {
		return true;
	}
	virtual bool onKey(Uint16 k, S_TUI_VALUE v) {
		if (current==items.end() || v==S_TUI_RELEASED) return false;
		if (k==SDLK_RETURN) {
			if (!onTriggerCallback.empty()) onTriggerCallback();
			return false;
		}
		if (k==SDLK_UP) {
			if (current!=items.begin()) --current;
			else current = --items.end();
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (k==SDLK_DOWN) {
			if (current!= --items.end()) ++current;
			else current = items.begin();
			if (!onChangeCallback.empty()) onChangeCallback();
			return true;
		}
		if (k==SDLK_LEFT || k==SDLK_ESCAPE) return false;
		return false;
	}
	void addItem(const T& newitem) {
		items.push_back(newitem);

		if (current==items.end()) current = items.begin();
	}
	void addItemList(const string& s) {
		istringstream is(s);
		T elem;
		while (getline(is, elem)) {
			addItem(elem);
		}
	}
	void replaceItemList(string s, int selection) {
		items.clear();
		addItemList(s);

		current = items.begin();
		for (int j=0; j<selection; j++) {
			++current;
		}
	}
	const T& getCurrent(void) const {
		if (current==items.end()) return emptyT;
		else return *current;
	}
	void setCurrent(const T& i) {

		typename list<T>::iterator iter;

		bool found =0;
		for (iter=items.begin(); iter!=items.end(); iter++ ) {
			if ( i == (*iter)) {
				current = iter;
				found = 1;
				break;
			}
		}
		if (!found) current = items.begin();

		// No callback if set intentionally
		
	}
	bool setValue(const T& i) {
		if (items.find(i) == items.end()) return false;
		else current = items.find(i);
		return true;
	}
	string getLabel(void) const {
		return label;
	}
	virtual void set_OnTriggerCallback(const callback<void>& c) {
		onTriggerCallback = c;
	}
	void setLabel(const string& _label) {
		label = _label;
	}
protected:
	T emptyT;
	list<T> items;
	typename list<T>::iterator current;
	string label;
	callback<void> onTriggerCallback;
};



// Widget used to set time zone. Initialized from a file of type /usr/share/zoneinfo/zone.tab
class Time_zone_item : public CallbackComponent
{
public:
	Time_zone_item(const string& zonetab_file, const string& _label = string());
	virtual ~Time_zone_item();
	virtual bool onKey(Uint16, S_TUI_VALUE);
	virtual string getString(void);
	virtual bool isEditable(void) const {
		return true;
	}
	string gettz(void); // should be const but gives a boring error...
	void settz(const string& tz);
	void setLabel(const string& _label) {
		label = _label;
	}
protected:
	MultiSet_item<string> continents_names;
	map<string, MultiSet_item<string> > continents;
	string label;
	MultiSet_item<string>* current_edit;
};


// Widget used to edit a vector
class Vector_item : public CallbackComponent
{
public:
	Vector_item(const string& _label = string(), Vec3d _init_vector = Vec3d(0,0,0));
	~Vector_item();
	virtual bool onKey(Uint16, S_TUI_VALUE);
	virtual string getString(void);
	virtual bool isEditable(void) const {
		return true;
	}
	Vec3d getVector(void) const {
		return Vec3d( a->getValue(), b->getValue(), c->getValue());
	}
	void setVector(Vec3d _vector) {
		a->setValue(_vector[0]);
		b->setValue(_vector[1]);
		c->setValue(_vector[2]);
	}
	void setLabel(const string& _label) {
		label = _label;
	}
protected:
	Decimal_item* current_edit;	// 0 to 2
	string label;
	Decimal_item *a, *b, *c;
};



}; // namespace s_tui

#endif // _TUI_H_
