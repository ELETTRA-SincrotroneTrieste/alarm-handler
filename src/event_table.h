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
#include <atomic>
#include <tango.h>


using namespace std;

#define INTERNAL_ERROR	"internal_error"
#define TYPE_TANGO_ERR		-2
#define TYPE_GENERIC_ERR	-3		
#define		SUB_ERR			-1
constexpr int NOTHING = 0;
constexpr int UPDATE_PROP = 1;

class alarm_list {
	public:
		alarm_list(void) {}
		alarm_list(const alarm_list& la) {l_alarm = la.l_alarm;}
		~alarm_list(void) {}
		void push(string& a);
		void pop(const string &a);
		void clear(void);
		list<string> show(void);
		bool empty();
		alarm_list& operator=(const alarm_list& other) {if (this != &other) {l_alarm = other.l_alarm;} return *this;}
	protected:
		list<string> l_alarm;
		omni_mutex l;
};

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
		string name;					/* event name */
		string	devname;
		string	attname;
		value_t value;				/* event value */
		string value_string;	//added for DevString attributes
		int quality;
		string ex_reason;
		string ex_desc;
		string ex_origin;
		Tango::TimeVal ts;		/* event timestamp */
		int type,							/* attribute data type */
				read_size,                                      /* attribute size of read part */
				counter,					/* molteplicita' */
				err_counter;					/* molteplicita' errore */				
		alarm_list m_alarm;
		bool valid;	//TODO: old
		bool 	first;//TODO: new
		bool 	first_err;//TODO: new
		Tango::AttributeProxy *attr;
		Tango::DevState			evstate;
		unsigned int event_id;
		bool	isZMQ;
		EventCallBack	*event_cb;
		bool running;
		bool paused;
		bool stopped;
		uint32_t okev_counter;
		uint32_t okev_counter_freq;
		timeval last_okev;
		uint32_t nokev_counter;
		uint32_t nokev_counter_freq;
		timeval last_nokev;
		timespec last_ev;
		vector<string> filter;
		/*
		 * methods
		 */
		event(string& s, value_t& v, Tango::TimeVal& t);
		event(string& s);
		event() {}
		~event() {}
		//void push_alarm(string& n);
		//void pop_alarm(string& n);
//		bool event::operator==(const event& e);		//TODO: gcc 4 problem??
		bool operator==(const event& e);
//		bool event::operator==(const string& s);	//TODO: gcc 4 problem??
		bool operator==(const string& s);
		ReadersWritersLock *siglock;
	protected:
	private:
};

typedef struct basic_event_info_s {
	string ev_name;
	value_t value;
	string value_string;
	int quality;
	//Tango::DevErrorList 	errors;
	string ex_reason;
	string ex_desc;
	string ex_origin;
	int type;
	int read_size;
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
		event_list(void): empty(this) {}
		~event_list(void) {}
		void push_back(bei_t& e);
		const bei_t pop_front(void);
		void clear(void);
		list<bei_t> show(void);
		size_t size();
	protected:
		list<bei_t> l_event;
	private:
		omni_condition empty;
};

/*
 * store all the events
 */
class event_table : public Tango::TangoMonitor, public Tango::LogAdapter {
	public:
		event_table(Tango::DeviceImpl *s);//:Tango::LogAdapter(s) {mydev = s;}
		~event_table(void) {stop_thread();}
		//void push_back(event e);
		void show(list<string> &evl);
		void summary(list<string> &evs);
		unsigned int size(void);
#if 0
		void init_proxy(void)  throw(vector<string> &);
		void free_proxy(void);
		void subscribe(EventCallBack& ecb) throw(vector<string> &);//throw(string&);
		void unsubscribe(void) throw(string&);
#endif
		/**
		 * Add a new signal.
		 */
		void add(string &signame, vector<string> contexts);
		void add(string &signame, vector<string> contexts, int to_do, bool start);
		event *get_signal(string signame);
		void stop(string &signame);
		void remove(string &signame, bool stop);
		void subscribe_events();
		void unsubscribe_events();
		void start(string &signame);
		void start_all();
		void update_property();
		/**
		 *	return number of signals to be subscribed
		 */
		int nb_sig_to_subscribe();
		/**
		 *	build a list of signal to set HDB device property
		 */
		void put_signal_property();
		void check_signal_property();
		bool is_initialized();
		bool get_if_stop();
		void stop_thread();
		vector<event> v_event;
		ReadersWritersLock      veclock;
		bool	stop_it;
		bool	initialized;
		atomic_int		action;
	private:
		Tango::DeviceImpl *mydev;
};  /* class event_table */


/*
 * event callback
 */
class EventCallBack : public Tango::CallBack, public Tango::LogAdapter
{
	public:
		EventCallBack(Tango::DeviceImpl *s);
		~EventCallBack(void);
		void push_event(Tango::EventData* ev);
		//void init(event_list* e);
		void extract_values(Tango::DeviceAttribute *attr_value, vector<double> &val, string &val_string, int &type, int &read_size);
	private:
		//event_list* e_ptr;
		Tango::DeviceImpl *mydev;
};

/*
 * utility function
 */
Tango::TimeVal gettime(void);

#endif	/* EVENT_TABLE_H */
