/*
 * alarm_table.h
 *
 * $Author: graziano $
 *
 * $Revision: 1.5 $
 *
 * $Log: alarm_table.h,v $
 *
 *
 * copyleft: Sincrotrone Trieste S.C.p.A. di interesse nazionale
 *           Strada Statale 14 - km 163,5 in AREA Science Park
 *           34012 Basovizza, Trieste ITALY
 */

#ifndef ALARM_TABLE_H
#define ALARM_TABLE_H

#define _RW_LOCK

#include <iostream>
#include <string>
#include <map>

#include <tango.h>

//spirit defines have to be put befor first inclusion of spirit headers
#ifndef BOOST_SPIRIT_RULE_SCANNERTYPE_LIMIT
#define BOOST_SPIRIT_RULE_SCANNERTYPE_LIMIT 2 //tmp scanner_list
#endif

/*#ifndef BOOST_SPIRIT_THREADSAFE 
#define BOOST_SPIRIT_THREADSAFE 
#endif

#ifndef PHOENIX_THREADSAFE
#define PHOENIX_THREADSAFE
#endif
*/	
//#include "spirit-parser.h"
//#include <boost/spirit/core.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION  < 103600
#include <boost/spirit/tree/ast.hpp>					//for ast parse trees (in tree_formula)
#else
#include <boost/spirit/include/classic_ast.hpp>			//for ast parse trees (in tree_formula)
#endif

//#include "log_thread.h"

#define LOG_STREAM		cout

using namespace std;

//#define _ACCESS_NODE_D 1
#if BOOST_VERSION  < 103600
#ifndef _ACCESS_NODE_D
typedef char const*         iterator_t;
typedef boost::spirit::tree_match<iterator_t> parse_tree_match_t;
typedef boost::spirit::tree_parse_info<>    tree_parse_info_t;
#else
typedef std::string::iterator  iterator_t;
typedef boost::spirit::node_val_data_factory<unsigned int> factory_t;		//want a modified node to contain an unsigned int value
typedef boost::spirit::tree_match<iterator_t, factory_t> parse_tree_match_t;     
typedef boost::spirit::tree_parse_info<iterator_t, factory_t>    tree_parse_info_t;   
#endif
#else
#ifndef _ACCESS_NODE_D
typedef char const*         iterator_t;
typedef boost::spirit::classic::tree_match<iterator_t> parse_tree_match_t;
typedef boost::spirit::classic::tree_parse_info<>    tree_parse_info_t;
#else
typedef std::string::iterator  iterator_t;
typedef boost::spirit::classic::node_val_data_factory<unsigned int> factory_t;		//want a modified node to contain an unsigned int value
typedef boost::spirit::classic::tree_match<iterator_t, factory_t> parse_tree_match_t;     
typedef boost::spirit::classic::tree_parse_info<iterator_t, factory_t>    tree_parse_info_t;   
#endif
#endif

typedef parse_tree_match_t::tree_iterator iter_t;

#define S_NORMAL	"NORMAL"
#define S_ALARM		"ALARM"

#define NOT_ACK		"NACK"
#define ACK		"ACK"

#define GR_ALL		0xffffffff
#define GR_NONE		0x00000000
#define GR_DEFAULT	GR_NONE			//or GR_ALL??
//#define MAX_GRP		32
#define GR_ALL_NAME		"gr_all"
#define GR_NONE_NAME	"gr_none"

#define LEV_LOG			"log"
#define LEV_WARNING		"warning"
#define LEV_FAULT		"fault"
#define LEV_DEFAULT	LEV_FAULT

class alarm_t;
class alarm_table;
class log_thread;
class cmd_thread;


struct formula_res_t
{
	formula_res_t(){value=0;quality=Tango::ATTR_VALID;ex_reason=string("");ex_desc=string("");ex_origin=string("");}
	double value;
	int quality;
	Tango::DevErrorList 	errors;	//TODO: error stack
	string ex_reason;
	string ex_desc;
	string ex_origin;
	int combine_quality(int quality1, int quality2)
	{
		int res;
		if(quality1 == Tango::ATTR_INVALID ||  quality2 == Tango::ATTR_INVALID)
			res = Tango::ATTR_INVALID;
		else if(quality1 == Tango::ATTR_ALARM ||  quality2 == Tango::ATTR_ALARM)
			res = Tango::ATTR_ALARM;
		else if(quality1 == Tango::ATTR_WARNING ||  quality2 == Tango::ATTR_WARNING)
			res = Tango::ATTR_WARNING;
		else if(quality1 == Tango::ATTR_CHANGING ||  quality2 == Tango::ATTR_CHANGING)
			res = Tango::ATTR_CHANGING;
		else
			res = (quality1 > quality2) ? quality1 : quality2;	//TODO: decide priority in enum AttrQuality { ATTR_VALID, ATTR_INVALID, ATTR_ALARM, ATTR_CHANGING, ATTR_WARNING /*, __max_AttrQuality=0xffffffff */ };
		return res;
	}
	string combine_exception(string ex_1, string ex_2)
	{
		if(ex_1.length() > 0)
			return ex_1;
		else
			return ex_2;
	}
	formula_res_t operator==(const formula_res_t& e)
		{
			formula_res_t res;
			res.value = value == e.value;
			res.quality = combine_quality(quality, e.quality);
			res.ex_reason = combine_exception(ex_reason, e.ex_reason);
			res.ex_desc = combine_exception(ex_desc, e.ex_desc);
			res.ex_origin = combine_exception(ex_origin, e.ex_origin);
			return res;
		}
	formula_res_t operator!=(const formula_res_t& e)
		{
			formula_res_t res;
			res.value = value != e.value;
			res.quality = combine_quality(quality, e.quality);
			res.ex_reason = combine_exception(ex_reason, e.ex_reason);
			res.ex_desc = combine_exception(ex_desc, e.ex_desc);
			res.ex_origin = combine_exception(ex_origin, e.ex_origin);
			return res;
		}
	formula_res_t operator<=(const formula_res_t& e)
		{
			formula_res_t res;
			res.value = value <= e.value;
			res.quality = combine_quality(quality, e.quality);
			res.ex_reason = combine_exception(ex_reason, e.ex_reason);
			res.ex_desc = combine_exception(ex_desc, e.ex_desc);
			res.ex_origin = combine_exception(ex_origin, e.ex_origin);
			return res;
		}
	formula_res_t operator>=(const formula_res_t& e)
		{
			formula_res_t res;
			res.value = value >= e.value;
			res.quality = combine_quality(quality, e.quality);
			res.ex_reason = combine_exception(ex_reason, e.ex_reason);
			res.ex_desc = combine_exception(ex_desc, e.ex_desc);
			res.ex_origin = combine_exception(ex_origin, e.ex_origin);
			return res;
		}
	formula_res_t operator<(const formula_res_t& e)
		{
			formula_res_t res;
			res.value = value < e.value;
			res.quality = combine_quality(quality, e.quality);
			res.ex_reason = combine_exception(ex_reason, e.ex_reason);
			res.ex_desc = combine_exception(ex_desc, e.ex_desc);
			res.ex_origin = combine_exception(ex_origin, e.ex_origin);
			return res;
		}
	formula_res_t operator>(const formula_res_t& e)
		{
			formula_res_t res;
			res.value = value > e.value;
			res.quality = combine_quality(quality, e.quality);
			res.ex_reason = combine_exception(ex_reason, e.ex_reason);
			res.ex_desc = combine_exception(ex_desc, e.ex_desc);
			res.ex_origin = combine_exception(ex_origin, e.ex_origin);
			return res;
		}
	formula_res_t operator||(const formula_res_t& e)
		{
			formula_res_t res;
			res.value = value || e.value;
			res.quality = combine_quality(quality, e.quality);
			res.ex_reason = combine_exception(ex_reason, e.ex_reason);
			res.ex_desc = combine_exception(ex_desc, e.ex_desc);
			res.ex_origin = combine_exception(ex_origin, e.ex_origin);
			return res;
		}
	formula_res_t operator&&(const formula_res_t& e)
		{
			formula_res_t res;
			res.value = value && e.value;
			res.quality = combine_quality(quality, e.quality);
			res.ex_reason = combine_exception(ex_reason, e.ex_reason);
			res.ex_desc = combine_exception(ex_desc, e.ex_desc);
			res.ex_origin = combine_exception(ex_origin, e.ex_origin);
			return res;
		}
	formula_res_t operator+(const formula_res_t& e)
		{
			formula_res_t res;
			res.value = value + e.value;
			res.quality = combine_quality(quality, e.quality);
			res.ex_reason = combine_exception(ex_reason, e.ex_reason);
			res.ex_desc = combine_exception(ex_desc, e.ex_desc);
			res.ex_origin = combine_exception(ex_origin, e.ex_origin);
			return res;
		}
	formula_res_t operator-(const formula_res_t& e)
		{
			formula_res_t res;
			res.value = value - e.value;
			res.quality = combine_quality(quality, e.quality);
			res.ex_reason = combine_exception(ex_reason, e.ex_reason);
			res.ex_desc = combine_exception(ex_desc, e.ex_desc);
			res.ex_origin = combine_exception(ex_origin, e.ex_origin);
			return res;
		}
	formula_res_t operator*(const formula_res_t& e)
		{
			formula_res_t res;
			res.value = value * e.value;
			res.quality = combine_quality(quality, e.quality);
			res.ex_reason = combine_exception(ex_reason, e.ex_reason);
			res.ex_desc = combine_exception(ex_desc, e.ex_desc);
			res.ex_origin = combine_exception(ex_origin, e.ex_origin);
			return res;
		}
	formula_res_t operator/(const formula_res_t& e)
		{
			formula_res_t res;
			res.value = value / e.value;
			res.quality = combine_quality(quality, e.quality);
			res.ex_reason = combine_exception(ex_reason, e.ex_reason);
			res.ex_desc = combine_exception(ex_desc, e.ex_desc);
			res.ex_origin = combine_exception(ex_origin, e.ex_origin);
			return res;
		}
	/*string operator<<(const formula_res_t& e)
		{
			stringstream res;
			res << "value="<<e.value<<" quality="<<e.quality<<" EX reason="<<e.ex_reason<<" desc="<<e.ex_desc<<" origin="<<e.ex_origin;
			return res.str();
		}*/
};

/*
 * store the alarm-name/alarm-formula pair
 */
class alarm_t {
	public:
		string name,
					 formula;
		string attr_name;
		Tango::DevBoolean *attr_value;
		int quality;
		string ex_reason;
		string ex_desc;
		string ex_origin;
		Tango::TimeVal ts;
		string stat,
					 ack;
		unsigned int counter;
		
		tree_parse_info_t formula_tree;
					 
		static map<string, unsigned int> grp_str;

		bool done;
		bool to_be_evaluated;
		string msg;
		unsigned int grp;
		string lev;
		set<string> s_event;
		int is_new;
		Tango::TimeVal ts_time_threshold;	//says when it has gone in alarm status for the first time
		unsigned int time_threshold;		//TODO: seconds, is it enough precision?

		Tango::TimeVal ts_time_silenced;	//says when it has been silenced
		int silent_time;			//minutes max to be silent
		int silenced;				//minutes still to be silent
		string attr_values_time_threshold;	//attr_values of first occurrence of alarm waiting for time threshold
		string cmd_name_a;					//action to execute: when NORMAL -> ALARM, cmd_name = cmd_dp_a/cmd_action_a
		string cmd_dp_a;						//device proxy part of cmd_name_a
		string cmd_action_a;					//action part of cmd_name_a
		bool send_arg_a;					//send as string argument alarm name and attr values
		Tango::DeviceProxy *dp_a;
		string cmd_name_n;					//action to execute: when ALARM -> NORMAL, cmd_name_n = cmd_dp_n/cmd_action_n
		string cmd_dp_n;						//device proxy part of cmd_name_n
		string cmd_action_n;					//action part of cmd_name_n
		bool send_arg_n;					//send as string argument alarm name and attr values
		Tango::DeviceProxy *dp_n;
		/*
		 * methods
		 */
		alarm_t();							//constructor
		void init_static_map(vector<string> &group_names);
		bool operator==(const alarm_t& that);
		bool operator==(const string& n);
		void str2alm(const string &s);
		string alm2str(void);
		string grp2str(void);
		void add_grp_from_str(string &s);
		void str2grp(string &s);
		void insert(const string& s);
		void clear();
		
	protected:
	private:
		
};

typedef map<string,alarm_t> alarm_container_t;	
#ifndef _RW_LOCK
class alarm_table  : public omni_mutex {
#else
class alarm_table {
#endif
	public:
		alarm_table() {}
		~alarm_table() {}
		void set_dev(Tango::DeviceImpl* devImpl) {mydev=devImpl;}

		//void init(vector<string>& avs);
		//void init(vector<string>& avs, vector<string> &evn, map< string,vector<string> > &alarm_event);		
		void push_back(alarm_t& a);
		void show(vector<string> &al_table_string);
		unsigned int size(void);
		alarm_container_t& get(void);
		void stored(vector<alarm_t>& a);
		bool update(const string& alm_name, Tango::TimeVal ts, formula_res_t res, string &attr_values, string grp, string msg, string formula);
		bool timer_update();
		void erase(alarm_container_t::iterator i);
		bool exist(string& s);
		//vector<alarm_t> v_alarm;
		alarm_container_t v_alarm;
#ifdef _RW_LOCK
		ReadersWritersLock *vlock;
		void new_rwlock();
		void del_rwlock();
#endif
		

		void init_logdb(string dbhost, string dbuser, string dbpw, string dbname, int dbport, string instance_name);
		void stop_logdb();
		void log_alarm_db(unsigned int type, Tango::TimeVal ts, string name, string status, string ack,
				 string formula, unsigned int time_threshold, string grp, string lev, string msg, string action, int silent_time, vector<string> alm_list=vector<string>());
		void get_alarm_list_db(vector<string> &al_list);
		void init_cmdthread();
		void stop_cmdthread();
		Tango::TimeVal startup_complete;			//to disable action execution at startup		
	
	protected:
	private:
		Tango::DeviceImpl* mydev;
		log_thread *logloop;
		cmd_thread *cmdloop;		
};


#endif	/* ALARM_TABLE_H */
