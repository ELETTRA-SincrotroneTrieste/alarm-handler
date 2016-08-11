/*
 * event_table.h
 *
 * $Author: graziano $
 *
 * $Revision: 1.2 $
 *
 * $Log: event_table.h,v $
 *
 *
 * copyleft: Sincrotrone Trieste S.C.p.A. di interesse nazionale
 *           Strada Statale 14 - km 163,5 in AREA Science Park
 *           34012 Basovizza, Trieste ITALY
 */

#ifndef EVENT_TABLE_H
#define EVENT_TABLE_H

#include <iostream>
#include <string>
#include <map>

#include <tango.h>


using namespace std;

#define INTERNAL_ERROR	"internal_error"
#define TYPE_TANGO_ERR		-2
#define TYPE_GENERIC_ERR	-3		

class event;
class event_list;
class event_table;
class EventCallBack;

typedef vector<Tango::DevDouble> value_t;

/*
 * basic event type
 */
class event {
	public:
		string name,					/* event name */
					 device,				/* device name */
					 attribute;			/* attribute name */
		value_t value;				/* event value */
		Tango::TimeVal ts;		/* event timestamp */
		int type,							/* attribute data type */
				counter,					/* molteplicita' */
				err_counter;					/* molteplicita' errore */				
		//map<string, string> m_alarm;
		vector<string> m_alarm;
		bool valid;
		
		Tango::DeviceProxy *dp;
		unsigned int eid;
		vector<string> filter;
		/*
		 * methods
		 */
		event(string& s, value_t& v, Tango::TimeVal& t);
		event(string& s);
		event() {}
		~event() {}
		void push_alarm(string& n);
		void pop_alarm(string& n);
//		bool event::operator==(const event& e);		//TODO: gcc 4 problem??
		bool operator==(const event& e);
//		bool event::operator==(const string& s);	//TODO: gcc 4 problem??
		bool operator==(const string& s);
	protected:
	private:
};

typedef struct basic_event_info_s {
	string ev_name;
	value_t value;
	int type;
	Tango::TimeVal ts;
	string msg;
} bei_t;
	
/*
 * here the event callback handler will store the relevant
 * events info coming from subscribed events for the
 * processing thread
 */
class event_list : public omni_mutex {
	public:
		event_list(void): full(this), empty(this) {}
		~event_list(void) {}
		void push_back(bei_t& e);
		const bei_t pop_front(void);
		void clear(void);
		list<bei_t> show(void);
	protected:
		list<bei_t> l_event;
	private:
		omni_condition full,
									 empty;
};

/*
 * store all the events
 */
class event_table : public event , Tango::LogAdapter {
	public:
		event_table(Tango::DeviceImpl *s):Tango::LogAdapter(s) {}
		~event_table(void) {}
		void push_back(event e);
		void show(void);
		unsigned int size(void);
		void init_proxy(void)  throw(vector<string> &);
		void free_proxy(void);
		void subscribe(EventCallBack& ecb) throw(vector<string> &);//throw(string&);
		void unsubscribe(void) throw(string&);
		void update_events(bei_t& e) throw(string&);
		vector<event> v_event;
	protected:
	private:
};  /* class event_table */


/*
 * event callback
 */
class EventCallBack : public Tango::CallBack {
	public:
		EventCallBack(void);
		~EventCallBack(void);
		void push_event(Tango::EventData* ev);
		void init(event_list* e);
		void extract_values(Tango::DeviceAttribute *attr_value, vector<double> &val, int &type);
	private:
		event_list* e_ptr;
};

/*
 * utility function
 */
Tango::TimeVal gettime(void);

#endif	/* EVENT_TABLE_H */
