/*
 * event_table.cpp
 *
 * $Author: graziano $
 *
 * $Revision: 1.5 $
 *
 * $Log: event_table.cpp,v $
 *
 *
 * copyleft: Sincrotrone Trieste S.C.p.A. di interesse nazionale
 *           Strada Statale 14 - km 163,5 in AREA Science Park
 *           34012 Basovizza, Trieste ITALY
 */

#include <sys/time.h>
#include <tango.h>
#include "event_table.h"

static const char __FILE__rev[] = __FILE__ " $Revision: 1.5 $";

/*
 * event_list class methods
 */
void event_list::push_back(bei_t& e)
{
	this->lock();

	try{
		l_event.push_back(e);		
		empty.signal();
	}
	catch(omni_thread_fatal& ex)
	{
		ostringstream err;
		err << "omni_thread_fatal exception signaling omni_condition, err=" << ex.error << ends;
		//WARN_STREAM << "event_list::push_back(): " << err.str() << endl;	
		printf("event_list::push_back(): %s", err.str().c_str());
	}			
	catch(Tango::DevFailed& ex)
	{
		ostringstream err;
		err << "exception  signaling omni_condition: '" << ex.errors[0].desc << "'" << ends;
		//WARN_STREAM << "event_list::push_back(): " << err.str() << endl;	
		printf("event_list::push_back: %s", err.str().c_str());
		Tango::Except::print_exception(ex);	
	}		
	catch(...)
	{
		//WARN_STREAM << "event_list::push_back(): catched unknown exception!!" << endl;
		printf("event_list::push_back(): catched unknown exception  signaling omni_condition!!");	
	}	
	this->unlock();
}

const bei_t event_list::pop_front(void)
{
	this->lock();
	//omni_mutex_lock l((omni_mutex)this);	//call automatically unlock on destructor and on exception
	try{
		while (l_event.empty() == true)
			empty.wait();					//wait release mutex while is waiting, then reacquire when signaled
	}
	catch(omni_thread_fatal& ex)
	{
		ostringstream err;
		err << "omni_thread_fatal exception waiting on omni_condition, err=" << ex.error << ends;
		//WARN_STREAM << "event_list::pop_front(): " << err.str() << endl;	
		printf("event_list::pop_front(): %s", err.str().c_str());
		bei_t e;
		this->unlock();
		sleep(1);
		return(e);
	}			
	catch(Tango::DevFailed& ex)
	{
		ostringstream err;
		err << "exception  waiting on omni_condition: '" << ex.errors[0].desc << "'" << ends;
		//WARN_STREAM << "event_list::pop_front(): " << err.str() << endl;	
		printf("event_list::pop_front: %s", err.str().c_str());
		Tango::Except::print_exception(ex);
		bei_t e;
		this->unlock();
		sleep(1);
		return(e);		
	}		
	catch(...)
	{
		//WARN_STREAM << "event_list::pop_front(): catched unknown exception!!" << endl;
		printf("event_list::pop_front(): catched unknown exception  waiting on omni_condition!!");
		bei_t e;
		this->unlock();
		sleep(1);
		return(e);		
	}			
	/*const*/ bei_t e;

	e = *(l_event.begin());

	l_event.pop_front();

	this->unlock();
	return(e);
}

void event_list::clear(void)
{
	//this->lock();
	l_event.clear();
	//this->unlock();
}

list<bei_t> event_list::show(void)
{
	list<bei_t> el;
	
	this->lock();
	el = l_event;
	this->unlock();
	return(el);
}


/*
 * event class methods
 */
event::event(string& s, value_t& v, Tango::TimeVal& t) : \
						 name(s), value(v), ts(t)
{
	const char *c = name.c_str();
	int j = 0;
	int num_slashes=3;	//not FQDN
	if(name.find("tango://") != string::npos)	//FQDN!!
		num_slashes = 6;
	while (*c) {
		if (*c == '/')
			j++;
		if (j < num_slashes)
			device.push_back(*c);
		else if (*c != '/')
			attribute.push_back(*c);
		c++;
	}
	type = -1;
	eid = 0;
	err_counter = 0;
	valid = false;
}

event::event(string& s) : name(s)
{
	const char *c = name.c_str();
	int j = 0;
	int num_slashes=3;	//not FQDN
	if(name.find("tango://") != string::npos)	//FQDN!!
		num_slashes = 6;
	while (*c) {
		if (*c == '/')
			j++;
		if (j < num_slashes)
			device.push_back(*c);
		else if (*c != '/')
			attribute.push_back(*c);
		c++;
	}
	type = -1;
	eid = 0;
	err_counter = 0;
	valid = false;	
}

void event::push_alarm(string& n)
{
	m_alarm.push_back(n);
}

void event::pop_alarm(string& n)
{
	vector<string>::iterator it = find(m_alarm.begin(), m_alarm.end(), n);
	m_alarm.erase(it);
	
}

bool event::operator==(const event& e)
{
	return(name == e.name);
}

bool event::operator==(const string& s)
{
	return(name == s);
}

/*
 * event_table class methods
 */
void event_table::push_back(event e)
{
	v_event.push_back(e);
}

void event_table::show(void)
{
	DEBUG_STREAM << "events found:" << endl;
	if (v_event.empty() == false) {
		vector<event>::iterator i = v_event.begin();
		while (i != v_event.end()) {
			DEBUG_STREAM << "\t" << i->name << endl;
			i++;
		}
	}
}

unsigned int event_table::size(void)
{
	return(v_event.size());
}

void event_table::init_proxy(void)	throw(vector<string> &)
{
	vector<string> proxy_error;
	if (v_event.empty() == false) {
		for (vector<event>::iterator i = v_event.begin(); \
				 i != v_event.end(); i++) 
		{
			try	{
				i->dp = new Tango::DeviceProxy(i->device);
			} catch(Tango::DevFailed& e)
			{
				ostringstream o;
				o << "new DeviceProxy() failed for " \
					<< i->device << ends;
				ERROR_STREAM << o.str() << endl;
				//throw o.str();
				proxy_error.push_back(o.str());
			}
		}
	}
	if(!proxy_error.empty())
		throw proxy_error;	
}

void event_table::free_proxy(void)
{
	if (v_event.empty() == false) {
		for (vector<event>::iterator i = v_event.begin(); \
				 i != v_event.end(); i++) {
			try{
				delete i->dp;
				DEBUG_STREAM << gettime().tv_sec << " event_table::free_proxy(): deleted proxy " << i->device << endl;
			} catch(...)
			{
				ERROR_STREAM << "event_table::free_proxy: exception deleting proxy of event: " << i->name << endl;
			}
		}
	}
}

void event_table::subscribe(EventCallBack& ecb) throw(vector<string> &)//throw(string&)
{
	vector<string> subscribe_error;
	if (v_event.empty() == false) {
		for (vector<event>::iterator i = v_event.begin(); \
				 i != v_event.end(); i++) {
			try {
				i->eid = i->dp->subscribe_event(i->attribute, \
												Tango::CHANGE_EVENT, \
												&ecb, i->filter);
			} catch (...) {
				ostringstream o;
				o << "subscribe_event() failed for " \
					<< i->name << ends;
				ERROR_STREAM << o.str() << endl;
				//throw o.str();
				subscribe_error.push_back(o.str());
			}
		}
	}
	if(!subscribe_error.empty())
		throw subscribe_error;
}

void event_table::unsubscribe(void) throw(string&)
{
	ostringstream o;
	if (v_event.empty() == false) {
		for (vector<event>::iterator i = v_event.begin(); \
				 i != v_event.end(); i++) {
			try {
				i->dp->unsubscribe_event(i->eid);
				DEBUG_STREAM << gettime().tv_sec << " event_table::unsubscribe(): unsubscribed " << i->name << endl;
			} catch (Tango::DevFailed& e) {
				o << " unsubscribe_event() failed for "	<< i->name << " err=" << e.errors[0].desc;
				ERROR_STREAM << gettime().tv_sec << " event_table::unsubscribe(): " << o.str() << endl;
				//throw o.str();
			} catch (...) {
				o << " unsubscribe_event() failed for " \
					<< i->name;
				ERROR_STREAM << gettime().tv_sec << " event_table::unsubscribe(): " << o.str() << endl;
				//throw o.str();
			}
		}
	}
	if(o.str().length() > 0)
		throw o.str();
}

void event_table::update_events(bei_t &e) throw(string&)
{
	//LOG_STREAM << "event_table::update_events(bei_t &e): Entering..." << endl ;
	vector<event>::iterator found = \
			find(v_event.begin(), v_event.end(), e.ev_name);

	if (found == v_event.end())
	{
		//try to remove network domain and FQDN
		string ev_name_str(e.ev_name);
		string::size_type pos_slash = ev_name_str.find("tango://");
		if (pos_slash != string::npos)	//FQDN!!
		{
			//first remove network domain if any
			string::size_type pos_dot = ev_name_str.find(".",8);	//look for first . after tango://
			string::size_type pos_colon = ev_name_str.find(":",8);	//look for first : after tango://
			pos_slash = ev_name_str.find('/',8);					//look for first / after tango://
			if(pos_dot < pos_slash && pos_dot != string::npos && pos_colon != string::npos && pos_slash != string::npos)	//dot is in the TANGO_HOST part
			{
				string ev_name_str_no_domain = ev_name_str.substr(0,pos_dot) + ev_name_str.substr(pos_colon);
				//LOG_STREAM << __FUNCTION__ << " event "<< e.ev_name << " not found, trying without domain: " << ev_name_str_no_domain << endl;
				found = \
							find(v_event.begin(), v_event.end(), ev_name_str_no_domain);
			}
			if (found == v_event.end() && pos_slash != string::npos)
			{
				ev_name_str = ev_name_str.substr(pos_slash + 1);//remove FQDN
				//LOG_STREAM << __FUNCTION__ << " event "<< e.ev_name << " not found, trying without fqdn: " << ev_name_str << endl;
				found = \
							find(v_event.begin(), v_event.end(), ev_name_str);
			}
		}
		if (found == v_event.end())
		{
			/*
			 * shouldn't happen!!!
			 */
			ostringstream o;
			o << "event_table::update_events(): event '" \
				<< e.ev_name << "' not found! error=" << e.msg << ends;
				ERROR_STREAM << o.str() << endl;
			//cerr << o.str() << endl;
			throw o.str();
		}
	}

	if (found != v_event.end())
	{
		found->value = e.value;
		found->ts = e.ts;
		found->type = e.type;
	}
}


/*
 * EventCallBack class methods
 */
EventCallBack::EventCallBack(void)
{
	e_ptr = NULL;
}

EventCallBack::~EventCallBack(void)
{
	e_ptr = NULL;
}

void EventCallBack::push_event(Tango::EventData* ev)
{
	string temp_name;	
	bei_t e;
	e.ex_reason = string("");
	e.ex_desc = string("");
	e.ex_origin = string("");
	try {
		//e.errors = ev->errors;
		e.quality = Tango::ATTR_VALID;
		//cout << "EVENT="<<ev->attr_name<<" quality="<<e.quality<<endl;
		if (!ev->err) {
			e.quality = (int)ev->attr_value->get_quality();
#if 0//TANGO_VER >= 711
 			string ev_name_str(ev->attr_name);
 			string::size_type pos = ev_name_str.find("tango://");
 			if (pos != string::npos)
 			{
 				pos = ev_name_str.find('/',8);
 				ev_name_str = ev_name_str.substr(pos + 1);
 			}
 			e.ev_name = ev_name_str.c_str();
#else			
			e.ev_name = ev->attr_name;
#endif
			e.ts = ev->attr_value->time;
			extract_values(ev->attr_value, e.value, e.type);
		} else {
#if 0//TANGO_VER >= 711
 			string ev_name_str(ev->attr_name);
 			string::size_type pos = ev_name_str.find("tango://");
 			if (pos != string::npos)
 			{
 				pos = ev_name_str.find('/',8);
 				ev_name_str = ev_name_str.substr(pos + 1);
 			}
 			temp_name = ev_name_str.c_str() + string(".") + ev->event;
#else
			temp_name = ev->attr_name + string(".") + ev->event;		//TODO: BUG IN TANGO: part of attr_name after first dot continues in field event
#endif
			size_t pos_change = temp_name.find(".change");
			if(pos_change != string::npos)
			{
				temp_name = temp_name.substr(0,pos_change);
			}
			ostringstream o;
			o << "Tango error for '" << temp_name << "'=" << ev->errors[0].desc.in() << ends;			
			e.ev_name = temp_name;
			e.type = TYPE_TANGO_ERR;
			//e.ev_name = INTERNAL_ERROR;
			//e.type = -1;
			e.msg = o.str();
		}
	} 
	catch (string &err) {
		e.msg = err + " for event '" + ev->attr_name + "'";
		e.ev_name = ev->attr_name;
		e.type = TYPE_GENERIC_ERR;
		//e.value.i = 0;
		e.ts = gettime();
		//cerr << o.str() << endl;		
	} catch(Tango::DevFailed& Terr)
	{
		ostringstream o;
		o << "Event exception for'" \
			<< ev->attr_name << "' error=" << Terr.errors[0].desc << ends;
		e.ev_name = ev->attr_name;
		e.type = TYPE_GENERIC_ERR;
		//e.value.i = 0;
		e.ts = gettime();
		e.msg = o.str();
		//cerr << o.str() << endl;		
	}	
	catch (...) {
		ostringstream o;
		o << "Generic Event exception for'" \
			<< ev->attr_name << "'" << ends;
		e.ev_name = ev->attr_name;
		e.type = TYPE_GENERIC_ERR;
		//e.value.i = 0;
		e.ts = gettime();
		e.msg = o.str();
		//cerr << o.str() << endl;		
	}
	e_ptr->push_back(e);
}  /* push_event() */

void EventCallBack::extract_values(Tango::DeviceAttribute *attr_value, vector<double> &val, int &type)
{
	Tango::DevState stval;
	vector<Tango::DevState> v_st;
#if TANGO_VER >= 600
	vector<Tango::DevULong> v_ulo;
#endif
	vector<Tango::DevUChar> v_uch;
	vector<Tango::DevShort> v_sh;
	vector<Tango::DevUShort> v_ush;
	vector<Tango::DevLong> v_lo;
	vector<Tango::DevDouble> v_do;
	vector<Tango::DevFloat> v_fl;
	vector<Tango::DevBoolean> v_bo;

	if (attr_value->get_type() == Tango::DEV_UCHAR) {
		*(attr_value) >> v_uch;
		for(vector<Tango::DevUChar>::iterator it = v_uch.begin(); it != v_uch.end(); it++)
			val.push_back((double)(*it));		//convert all to double
		type = Tango::DEV_UCHAR;		
	} else if (attr_value->get_type() == Tango::DEV_SHORT) {
		*(attr_value) >> v_sh;
		for(vector<Tango::DevShort>::iterator  it = v_sh.begin(); it != v_sh.end(); it++)
			val.push_back((double)(*it));		//convert all to double				
		type = Tango::DEV_SHORT;
	} else if (attr_value->get_type() == Tango::DEV_USHORT) {
		*(attr_value) >> v_ush;
		for(vector<Tango::DevUShort>::iterator  it = v_ush.begin(); it != v_ush.end(); it++)
			val.push_back((double)(*it));		//convert all to double						
		type = Tango::DEV_USHORT;			
	} else if (attr_value->get_type() == Tango::DEV_LONG) {
		*(attr_value) >> v_lo;
		for(vector<Tango::DevLong>::iterator  it = v_lo.begin(); it != v_lo.end(); it++)
			val.push_back((double)(*it));		//convert all to double						
		type = Tango::DEV_LONG;
	} else if (attr_value->get_type() == Tango::DEV_STATE) {
		//*(attr_value) >> v_st;		//doesn't work in tango 5
		*(attr_value) >> stval;
		v_st.push_back(stval);
		for(vector<Tango::DevState>::iterator it = v_st.begin(); it != v_st.end(); it++)
			val.push_back((double)(*it));		//convert all to double
		type = Tango::DEV_STATE;
#if TANGO_VER >= 600	
	} else if (attr_value->get_type() == Tango::DEV_ULONG) {
		*(attr_value) >> v_ulo;
		for(vector<Tango::DevULong>::iterator  it = v_ulo.begin(); it != v_ulo.end(); it++)
			val.push_back((double)(*it));		//convert all to double						
		type = Tango::DEV_ULONG;
#endif  //TANGO_VER >= 600								
	} else if (attr_value->get_type() == Tango::DEV_DOUBLE) {
		*(attr_value) >> v_do;
		for(vector<Tango::DevDouble>::iterator  it = v_do.begin(); it != v_do.end(); it++)
			val.push_back((double)(*it));		//convert all to double						
		type = Tango::DEV_DOUBLE;
	} else if (attr_value->get_type() == Tango::DEV_FLOAT) {
		*(attr_value) >> v_fl;
		for(vector<Tango::DevFloat>::iterator  it = v_fl.begin(); it != v_fl.end(); it++)
			val.push_back((double)(*it));		//convert all to double						
		type = Tango::DEV_FLOAT;
	} else if (attr_value->get_type() == Tango::DEV_BOOLEAN) {
		*(attr_value) >> v_bo;
		for(vector<Tango::DevBoolean>::iterator  it = v_bo.begin(); it != v_bo.end(); it++)
			val.push_back((double)(*it));		//convert all to double		
		type = Tango::DEV_BOOLEAN;
	}
	else {
		ostringstream o;
		o << "unknown type" << ends;
		throw o.str();
	}	
}

void EventCallBack::init(event_list* e)
{
	e_ptr = e;
}


Tango::TimeVal gettime(void)
{
	struct timeval tv;
	struct timezone tz;
	Tango::TimeVal t;
	
	gettimeofday(&tv, &tz);
	t.tv_sec = tv.tv_sec;
	t.tv_usec = tv.tv_usec;
	t.tv_nsec = 0;
	return t;
}

/* EOF */
