/*----- PROTECTED REGION ID(Alarm::DynAttrUtils.cpp) ENABLED START -----*/
static const char *RcsId = "$Id:  $";
//=============================================================================
//
// file :        AlarmDynAttrUtils.cpp
//
// description : Dynamic attributes utilities file for the Alarm class
//
// project :     Elettra alarm device server
//
// This file is part of Tango device class.
// 
// Tango is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Tango is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Tango.  If not, see <http://www.gnu.org/licenses/>.
// 
// $Author:  $
//
// $Revision:  $
// $Date:  $
//
// $HeadURL:  $
//
//=============================================================================
//                This file is generated by POGO
//        (Program Obviously used to Generate tango Object)
//=============================================================================


#include <Alarm.h>
#include <AlarmClass.h>

/*----- PROTECTED REGION END -----*/	//	Alarm::DynAttrUtils.cpp

//================================================================
//  Attributes managed are:
//================================================================
//  AlarmState    |  Tango::DevEnum	Scalar
//  AlarmFormula  |  Tango::DevString	Scalar
//================================================================

//	For compatibility reason, this file (AlarmDynAttrUtils)
//	manage also the dynamic command utilities.
//================================================================
//  The following table gives the correspondence
//  between command and method names.
//
//  Command name  |  Method name
//================================================================
//================================================================

namespace Alarm_ns
{
//=============================================================
//	Add/Remove dynamic attribute methods
//=============================================================

//--------------------------------------------------------
/**
 *	Add a AlarmState dynamic attribute.
 *
 *  parameter attname: attribute name to be cretated and added.
 */
//--------------------------------------------------------
void Alarm::add_AlarmState_dynamic_attribute(string attname)
{
	//	Attribute : AlarmState
	AlarmStateAttrib	*alarmstate = new AlarmStateAttrib(attname);
	Tango::UserDefaultAttrProp	alarmstate_prop;
	//	description	not set for AlarmState
	//	label	not set for AlarmState
	//	unit	not set for AlarmState
	//	standard_unit	not set for AlarmState
	//	display_unit	not set for AlarmState
	//	format	not set for AlarmState
	//	max_value	not set for AlarmState
	//	min_value	not set for AlarmState
	//	max_alarm	not set for AlarmState
	//	min_alarm	not set for AlarmState
	//	max_warning	not set for AlarmState
	//	min_warning	not set for AlarmState
	//	delta_t	not set for AlarmState
	//	delta_val	not set for AlarmState
	
	/*----- PROTECTED REGION ID(Alarm::att_AlarmState_dynamic_attribute) ENABLED START -----*/
	DEBUG_STREAM << __func__<<": entering name="<<attname;
	alarm_container_t::iterator i = alarms.v_alarm.find(attname);
	if(i != alarms.v_alarm.end())
	{
		alarmstate_prop.set_description(i->second.formula.c_str());
	}
	else
	{
		INFO_STREAM << __func__<<": name="<<attname<<" NOT FOUND while looking for formula to add as attribute description";
	}
	
	/*----- PROTECTED REGION END -----*/	//	Alarm::att_AlarmState_dynamic_attribute
	{
		vector<string> labels;
		labels.push_back("NORM");
		labels.push_back("UNACK");
		labels.push_back("ACKED");
		labels.push_back("RTNUN");
		labels.push_back("SHLVD");
		labels.push_back("DSUPR");
		labels.push_back("OOSRV");
		alarmstate_prop.set_enum_labels(labels);
	}
	alarmstate->set_default_properties(alarmstate_prop);
	//	Not Polled
	alarmstate->set_disp_level(Tango::OPERATOR);
	//	Not Memorized
	alarmstate->set_change_event(true, true);
	alarmstate->set_archive_event(true, true);
	AlarmState_data.insert(make_pair(attname, 0));
	add_attribute(alarmstate);
}
//--------------------------------------------------------
/**
 *	remove a AlarmState dynamic attribute.
 *
 *  parameter attname: attribute name to be removed.
 */
//--------------------------------------------------------
void Alarm::remove_AlarmState_dynamic_attribute(string attname)
{
	remove_attribute(attname, true);
	map<string,Tango::DevEnum>::iterator ite;
    if ((ite=AlarmState_data.find(attname))!=AlarmState_data.end())
    {
    	/*----- PROTECTED REGION ID(Alarm::remove_AlarmState_dynamic_attribute) ENABLED START -----*/
    	DEBUG_STREAM << __func__<<": entering name="<<attname;
    	/*----- PROTECTED REGION END -----*/	//	Alarm::remove_AlarmState_dynamic_attribute
		AlarmState_data.erase(ite);
	}
}
//--------------------------------------------------------
/**
 *	Add a AlarmFormula dynamic attribute.
 *
 *  parameter attname: attribute name to be cretated and added.
 */
//--------------------------------------------------------
void Alarm::add_AlarmFormula_dynamic_attribute(string attname)
{
	//	Attribute : AlarmFormula
	AlarmFormulaAttrib	*alarmformula = new AlarmFormulaAttrib(attname);
	Tango::UserDefaultAttrProp	alarmformula_prop;
	//	description	not set for AlarmFormula
	//	label	not set for AlarmFormula
	//	unit	not set for AlarmFormula
	//	standard_unit	not set for AlarmFormula
	//	display_unit	not set for AlarmFormula
	//	format	not set for AlarmFormula
	//	max_value	not set for AlarmFormula
	//	min_value	not set for AlarmFormula
	//	max_alarm	not set for AlarmFormula
	//	min_alarm	not set for AlarmFormula
	//	max_warning	not set for AlarmFormula
	//	min_warning	not set for AlarmFormula
	//	delta_t	not set for AlarmFormula
	//	delta_val	not set for AlarmFormula
	
	/*----- PROTECTED REGION ID(Alarm::att_AlarmFormula_dynamic_attribute) ENABLED START -----*/
	DEBUG_STREAM << __func__<<": entering name="<<attname;
	
	/*----- PROTECTED REGION END -----*/	//	Alarm::att_AlarmFormula_dynamic_attribute
	alarmformula->set_default_properties(alarmformula_prop);
	//	Not Polled
	alarmformula->set_disp_level(Tango::OPERATOR);
	//	Not Memorized
	alarmformula->set_change_event(true, false);
	alarmformula->set_archive_event(true, false);
	char array[1];
	array[0] = '\0';
	AlarmFormula_data.insert(make_pair(attname, array));
	add_attribute(alarmformula);
}
//--------------------------------------------------------
/**
 *	remove a AlarmFormula dynamic attribute.
 *
 *  parameter attname: attribute name to be removed.
 */
//--------------------------------------------------------
void Alarm::remove_AlarmFormula_dynamic_attribute(string attname)
{
	remove_attribute(attname, true);
	map<string,Tango::DevString>::iterator ite;
    if ((ite=AlarmFormula_data.find(attname))!=AlarmFormula_data.end())
    {
    	/*----- PROTECTED REGION ID(Alarm::remove_AlarmFormula_dynamic_attribute) ENABLED START -----*/
    	
    	/*----- PROTECTED REGION END -----*/	//	Alarm::remove_AlarmFormula_dynamic_attribute
		AlarmFormula_data.erase(ite);
	}
}


//============================================================
//	Tool methods to get pointer on attribute data buffer 
//============================================================
//--------------------------------------------------------
/**
 *	Return a pointer on AlarmState data.
 *
 *  parameter attname: the specified attribute name.
 */
//--------------------------------------------------------
Tango::DevEnum *Alarm::get_AlarmState_data_ptr(string &name)
{
	map<string,Tango::DevEnum>::iterator ite;
    if ((ite=AlarmState_data.find(name))==AlarmState_data.end())
    {
		TangoSys_OMemStream	tms;
		tms << "Dynamic attribute " << name << " has not been created";
		Tango::Except::throw_exception(
					(const char *)"ATTRIBUTE_NOT_FOUND",
					tms.str().c_str(),
					(const char *)"Alarm::get_AlarmState_data_ptr()");
    }
	return  &(ite->second);
}
//--------------------------------------------------------
/**
 *	Return a pointer on AlarmFormula data.
 *
 *  parameter attname: the specified attribute name.
 */
//--------------------------------------------------------
Tango::DevString *Alarm::get_AlarmFormula_data_ptr(string &name)
{
	map<string,Tango::DevString>::iterator ite;
    if ((ite=AlarmFormula_data.find(name))==AlarmFormula_data.end())
    {
		TangoSys_OMemStream	tms;
		tms << "Dynamic attribute " << name << " has not been created";
		Tango::Except::throw_exception(
					(const char *)"ATTRIBUTE_NOT_FOUND",
					tms.str().c_str(),
					(const char *)"Alarm::get_AlarmFormula_data_ptr()");
    }
	return  &(ite->second);
}


//=============================================================
//	Add/Remove dynamic command methods
//=============================================================


} //	namespace
