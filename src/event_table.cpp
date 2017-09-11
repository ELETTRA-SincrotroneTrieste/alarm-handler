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
#include "AlarmHandler.h"

//for get_event_system_for_event_id, to know if ZMQ
#include <eventconsumer.h>

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

size_t event_list::size(void)
{
	size_t res;

	this->lock();
	res = l_event.size();
	this->unlock();
	return(res);
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
			devname.push_back(*c);
		else if (*c != '/')
			attname.push_back(*c);
		c++;
	}
	type = -1;
	event_id = SUB_ERR;
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
			devname.push_back(*c);
		else if (*c != '/')
			attname.push_back(*c);
		c++;
	}
	type = -1;
	event_id = SUB_ERR;
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
	if(it != m_alarm.end())
		m_alarm.erase(it);
	else
		cout << "event::"<<__func__<< ": event="<<name<<" ALARM '"<< n << "' NOT FOUND!"<< endl;
	
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
/*void event_table::push_back(event e)
{
//	v_event.push_back(e);//TODO: replaced with add
}*/

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

event_table::event_table(Tango::DeviceImpl *s):Tango::LogAdapter(s)
{
	mydev = s;
	stop_it = false;
	action = NOTHING;
}

unsigned int event_table::size(void)
{
	return(v_event.size());
}
#if 0
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
#endif
//=============================================================================
/**
 *	get signal by name.
 */
//=============================================================================
event *event_table::get_signal(string signame)
{
	//omni_mutex_lock sync(*this);
	for (unsigned int i=0 ; i<v_event.size() ; i++)
	{
		event	*sig = &v_event[i];
		if (sig->name==signame)
			return sig;
	}
	for (unsigned int i=0 ; i<v_event.size() ; i++)
	{
		event	*sig = &v_event[i];
		if (static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->compare_without_domain(sig->name,signame))
			return sig;
	}
	return NULL;
}


//=============================================================================
/**
 * Stop saving on DB a signal.
 */
//=============================================================================
void event_table::stop(string &signame)
{
	DEBUG_STREAM <<"event_table::"<< __func__<<": entering signame="<< signame << endl;
	ReaderLock lock(veclock);
	for (unsigned int i=0 ; i<v_event.size() ; i++)
	{
		if (v_event[i].name==signame)
		{
			v_event[i].siglock->writerIn();
			if(!v_event[i].stopped)
			{
				v_event[i].stopped=true;
				static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeStoppedNumber_read++;
				if(v_event[i].running)
				{
					static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeStartedNumber_read--;
					try
					{
						remove(signame, true);
					}
					catch (Tango::DevFailed &e)
					{
						//Tango::Except::print_exception(e);
						INFO_STREAM << "event_table::stop: error removing  " << signame << endl;
					}
				}
				if(v_event[i].paused)
				{
					static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributePausedNumber_read--;
					try
					{
						remove(signame, true);
					}
					catch (Tango::DevFailed &e)
					{
						//Tango::Except::print_exception(e);
						INFO_STREAM << "event_table::stop: error removing  " << signame << endl;
					}
				}
				v_event[i].running=false;
				v_event[i].paused=false;
			}
			v_event[i].siglock->writerOut();
			return;
		}
	}
	for (unsigned int i=0 ; i<v_event.size() ; i++)
	{
#ifndef _MULTI_TANGO_HOST
		if (static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->compare_without_domain(v_event[i].name,signame))
#else
		if (!static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->compare_tango_names(v_event[i].name,signame))
#endif
		{
			v_event[i].siglock->writerIn();
			if(!v_event[i].stopped)
			{
				v_event[i].stopped=true;
				static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeStoppedNumber_read++;
				if(v_event[i].running)
				{
					static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeStartedNumber_read--;
					try
					{
						remove(signame, true);
					}
					catch (Tango::DevFailed &e)
					{
						//Tango::Except::print_exception(e);
						INFO_STREAM << "event_table::stop: error removing  " << signame << endl;
					}
				}
				if(v_event[i].paused)
				{
					static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributePausedNumber_read--;
					try
					{
						remove(signame, true);
					}
					catch (Tango::DevFailed &e)
					{
						//Tango::Except::print_exception(e);
						INFO_STREAM << "event_table::stop: error removing  " << signame << endl;
					}
				}
				v_event[i].running=false;
				v_event[i].paused=false;
			}
			v_event[i].siglock->writerOut();
			return;
		}
	}

	//	if not found
	Tango::Except::throw_exception(
				(const char *)"BadSignalName",
				"Signal " + signame + " NOT subscribed",
				(const char *)"event_table::stop()");
}


//=============================================================================
/**
 * Remove a signal in the list.
 */
//=============================================================================
void event_table::remove(string &signame, bool stop)
{
	DEBUG_STREAM <<"event_table::"<< __func__<<": entering signame="<< signame << endl;
	//	Remove in signals list (vector)
	{
		if(!stop)
			veclock.readerIn();
		event	*sig = get_signal(signame);
		int event_id = sig->event_id;
		Tango::AttributeProxy *attr = sig->attr;
		if(!stop)
			veclock.readerOut();
		if(stop)
		{
			try
			{
				if(event_id != SUB_ERR && attr)
				{
					DEBUG_STREAM <<"event_table::"<< __func__<<": unsubscribing... "<< signame << endl;
					//unlocking, locked in event_table::stop but possible deadlock if unsubscribing remote attribute with a faulty event connection
					sig->siglock->writerOut();
					attr->unsubscribe_event(event_id);
					sig->siglock->writerIn();
					DEBUG_STREAM <<"event_table::"<< __func__<<": unsubscribed... "<< signame << endl;
				}
			}
			catch (Tango::DevFailed &e)
			{
				//	Do nothing
				//	Unregister failed means Register has also failed
				sig->siglock->writerIn();
				INFO_STREAM <<"event_table::"<< __func__<<": Exception unsubscribing " << signame << " err=" << e.errors[0].desc << endl;
			}
		}

		if(!stop)
			veclock.writerIn();
		vector<event>::iterator	pos = v_event.begin();

		bool	found = false;
		for (unsigned int i=0 ; i<v_event.size() && !found ; i++, pos++)
		{
			event	*sig = &v_event[i];
			if (sig->name==signame)
			{
				found = true;
				if(stop)
				{
					DEBUG_STREAM <<"event_table::"<<__func__<< ": removing " << signame << endl;
					//sig->siglock->writerIn(); //: removed, already locked in event_table::stop
					try
					{
						if(sig->event_id != SUB_ERR)
						{
							delete sig->event_cb;
						}
						if(sig->attr)
							delete sig->attr;
					}
					catch (Tango::DevFailed &e)
					{
						//	Do nothing
						//	Unregister failed means Register has also failed
						INFO_STREAM <<"event_table::"<< __func__<<": Exception deleting " << signame << " err=" << e.errors[0].desc << endl;
					}
					//sig->siglock->writerOut();
					DEBUG_STREAM <<"event_table::"<< __func__<<": stopped " << signame << endl;
				}
				if(!stop)
				{
					if(sig->running)
						static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeStartedNumber_read--;
					if(sig->paused)
						static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributePausedNumber_read--;
					if(sig->stopped)
						static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeStoppedNumber_read--;
					static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeNumber_read--;
					delete sig->siglock;
					v_event.erase(pos);
					DEBUG_STREAM <<"event_table::"<< __func__<<": removed " << signame << endl;
				}
				break;
			}
		}
		pos = v_event.begin();
		if (!found)
		{
			for (unsigned int i=0 ; i<v_event.size() && !found ; i++, pos++)
			{
				event	*sig = &v_event[i];
#ifndef _MULTI_TANGO_HOST
				if (static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->compare_without_domain(sig->name,signame))
#else
				if (!static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->compare_tango_names(sig->name,signame))
#endif
				{
					found = true;
					DEBUG_STREAM <<"event_table::"<<__func__<< ": removing " << signame << endl;
					if(stop)
					{
						sig->siglock->writerIn();
						try
						{
							if(sig->event_id != SUB_ERR)
							{
								delete sig->event_cb;
							}
							if(sig->attr)
								delete sig->attr;
						}
						catch (Tango::DevFailed &e)
						{
							//	Do nothing
							//	Unregister failed means Register has also failed
							INFO_STREAM <<"event_table::"<< __func__<<": Exception unsubscribing " << signame << " err=" << e.errors[0].desc << endl;
						}
						sig->siglock->writerOut();
						DEBUG_STREAM <<"event_table::"<< __func__<<": stopped " << signame << endl;
					}
					if(!stop)
					{
						if(sig->running)
							static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeStartedNumber_read--;
						if(sig->paused)
							static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributePausedNumber_read--;
						if(sig->stopped)
							static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeStoppedNumber_read--;
						static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeNumber_read--;
						delete sig->siglock;
						v_event.erase(pos);
						DEBUG_STREAM <<"event_table::"<< __func__<<": removed " << signame << endl;
					}
					break;
				}
			}
		}
		if(!stop)
			veclock.writerOut();
		if (!found)
			Tango::Except::throw_exception(
						(const char *)"BadSignalName",
						"Signal " + signame + " NOT subscribed",
						(const char *)"event_table::remove()");
	}
	//	then, update property
/*	if(!stop)
	{
		DEBUG_STREAM <<"event_table::"<< __func__<<": going to increase action... action="<<action<<"++" << endl;
		if(action <= UPDATE_PROP)
			action++;
		//put_signal_property();	//TODO: wakeup thread and let it do it? -> signal()
		signal();
	}*/
}
void event_table::update_property()
{
	DEBUG_STREAM <<"event_table::"<< __func__<<": going to increase action... action="<<action<<"++" << endl;
	if(action <= UPDATE_PROP)
		action++;
	//put_signal_property();	//TODO: wakeup thread and let it do it? -> signal()
	signal();
}
//=============================================================================
/**
 * Remove a signal in the list.
 */
//=============================================================================
void event_table::unsubscribe_events()
{
	DEBUG_STREAM <<"event_table::"<<__func__<< "    entering..."<< endl;
	veclock.readerIn();
	vector<event>	local_signals(v_event);
	veclock.readerOut();
	for (unsigned int i=0 ; i<local_signals.size() ; i++)
	{
		event	*sig = &local_signals[i];
		if (local_signals[i].event_id != SUB_ERR && sig->attr)
		{
			DEBUG_STREAM <<"event_table::"<<__func__<< "    unsubscribe " << sig->name << " id="<<omni_thread::self()->id()<< endl;
			try
			{
				sig->attr->unsubscribe_event(sig->event_id);
				DEBUG_STREAM <<"event_table::"<<__func__<< "    unsubscribed " << sig->name << endl;
			}
			catch (Tango::DevFailed &e)
			{
				//	Do nothing
				//	Unregister failed means Register has also failed
				INFO_STREAM <<"event_table::"<<__func__<< "    ERROR unsubscribing " << sig->name << " err="<<e.errors[0].desc<< endl;
			}
		}
	}
	veclock.writerIn();
	for (unsigned int i=0 ; i<v_event.size() ; i++)
	{
		event	*sig = &v_event[i];
		sig->siglock->writerIn();
		if (v_event[i].event_id != SUB_ERR && sig->attr)
		{
			delete sig->event_cb;
			DEBUG_STREAM <<"event_table::"<<__func__<< "    deleted cb " << sig->name << endl;
		}
		if(sig->attr)
		{
			delete sig->attr;
			DEBUG_STREAM <<"event_table::"<<__func__<< "    deleted proxy " << sig->name << endl;
		}
		sig->siglock->writerOut();
		delete sig->siglock;
		DEBUG_STREAM <<"event_table::"<<__func__<< "    deleted lock " << sig->name << endl;
	}
	DEBUG_STREAM <<"event_table::"<<__func__<< "    ended loop, deleting vector" << endl;

	/*for (unsigned int j=0 ; j<signals.size() ; j++, pos++)
	{
		signals[j].event_id = SUB_ERR;
		signals[j].event_conf_id = SUB_ERR;
		signals[j].archive_cb = NULL;
		signals[j].attr = NULL;
	}*/
	v_event.clear();
	veclock.writerOut();
	DEBUG_STREAM <<"event_table::"<< __func__<< ": exiting..."<<endl;
}
//=============================================================================
/**
 * Add a new signal.
 */
//=============================================================================
void event_table::add(string &signame, vector<string> contexts)
{
	DEBUG_STREAM << "event_table::"<<__func__<< " entering signame=" << signame << endl;
	add(signame, contexts, NOTHING, false);
}
//=============================================================================
/**
 * Add a new signal.
 */
//=============================================================================
void event_table::add(string &signame, vector<string> contexts, int to_do, bool start)
{
	DEBUG_STREAM << "event_table::"<<__func__<<": Adding " << signame << " to_do="<<to_do<<" start="<<(start ? "Y" : "N")<< endl;
	{
		veclock.readerIn();
		event	*sig;
		//	Check if already subscribed
		bool	found = false;
		for (unsigned int i=0 ; i<v_event.size() && !found ; i++)
		{
			sig = &v_event[i];
			found = (sig->name==signame);
		}
		for (unsigned int i=0 ; i<v_event.size() && !found ; i++)
		{
			sig = &v_event[i];
			found = static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->compare_without_domain(sig->name,signame);
		}
		veclock.readerOut();
		//DEBUG_STREAM << "event_table::"<<__func__<<": signame="<<signame<<" found="<<(found ? "Y" : "N") << " start="<<(start ? "Y" : "N")<< endl;
		if (found && !start)
			Tango::Except::throw_exception(
						(const char *)"BadSignalName",
						"Signal " + signame + " already subscribed",
						(const char *)"event_table::add()");
		event	*signal;
		if (!found && !start)
		{
			//	on name, split device name and attrib name
			string::size_type idx = signame.find_last_of("/");
			if (idx==string::npos)
			{
				Tango::Except::throw_exception(
							(const char *)"SyntaxError",
							"Syntax error in signal name " + signame,
							(const char *)"event_table::add()");
			}
			signal = new event();
			//	Build Hdb Signal object
			signal->name      = signame;
			signal->siglock = new(ReadersWritersLock);
			signal->devname = signal->name.substr(0, idx);
			signal->attname = signal->name.substr(idx+1);
			signal->ex_reason = "NOT_connected";
			signal->ex_desc = "Attribute not subscribed";
			signal->ex_origin = "...";
			signal->attr = NULL;
			signal->running = false;
			signal->stopped = true;
			signal->paused = false;
			//DEBUG_STREAM << "event_table::"<<__func__<<": signame="<<signame<<" created signal"<< endl;
		}
		else if(found && start)
		{
			signal = sig;
			signal->siglock->writerIn();
			signal->ex_reason = "NOT_connected";
			signal->ex_desc = "Attribute not subscribed";
			signal->ex_origin = "...";
			signal->siglock->writerOut();
			//DEBUG_STREAM << "created proxy to " << signame << endl;
			//	create Attribute proxy
			signal->attr = new Tango::AttributeProxy(signal->name);	//TODO: OK out of siglock? accessed only inside the same thread?
			DEBUG_STREAM << "event_table::"<<__func__<<": signame="<<signame<<" created proxy"<< endl;
		}
		signal->event_id = SUB_ERR;
		signal->evstate    = Tango::ALARM;
		signal->isZMQ    = false;
		signal->okev_counter = 0;
		signal->okev_counter_freq = 0;
		signal->nokev_counter = 0;
		signal->nokev_counter_freq = 0;
		signal->first = true;
		signal->first_err = true;
		clock_gettime(CLOCK_MONOTONIC, &signal->last_ev);

		if(found && start)
		{

		}

		//DEBUG_STREAM <<"event_table::"<< __func__<< " created proxy to " << signame << endl;
		if (!found && !start)
		{
			veclock.writerIn();
			//	Add in vector
			v_event.push_back(*signal);
			delete signal;
			static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeNumber_read++;
			static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeStoppedNumber_read++;
			veclock.writerOut();
			//DEBUG_STREAM << "event_table::"<<__func__<<": signame="<<signame<<" push_back signal"<< endl;
		}
		else if(found && start)
		{

		}
		DEBUG_STREAM <<"event_table::"<< __func__<<": going to increase action... action="<<action<<" += " << to_do << endl;
		if(action <= UPDATE_PROP)
			action += to_do;
	}
	DEBUG_STREAM <<"event_table::"<< __func__<<": exiting... " << signame << endl;
	signal();
	//condition.signal();
}
//=============================================================================
/**
 * Subscribe archive event for each signal
 */
//=============================================================================
void event_table::subscribe_events()
{
	/*for (unsigned int ii=0 ; ii<v_event.size() ; ii++)
	{
		event	*sig2 = &v_event[ii];
		int ret = pthread_rwlock_trywrlock(&sig2->siglock);
		DEBUG_STREAM << __func__<<": pthread_rwlock_trywrlock i="<<ii<<" name="<<sig2->name<<" just entered " << ret << endl;
		if(ret == 0) pthread_rwlock_unlock(&sig2->siglock);
	}*/
	//omni_mutex_lock sync(*this);
	veclock.readerIn();
	for (unsigned int i=0 ; i<v_event.size() ; i++)
	{
		event	*sig = &v_event[i];
		sig->siglock->writerIn();
		if (sig->event_id==SUB_ERR && !sig->stopped)
		{
			if(!sig->attr)
			{
				try
				{
					vector<string> contexts;	//TODO!!!
					add(sig->name, contexts, NOTHING, true);
				}
				catch (Tango::DevFailed &e)
				{
					//Tango::Except::print_exception(e);
					INFO_STREAM << "event_table::subscribe_events: error adding  " << sig->name <<" err="<< e.errors[0].desc << endl;
					v_event[i].ex_reason = e.errors[0].reason;
					v_event[i].ex_desc = e.errors[0].desc;
					v_event[i].ex_origin = e.errors[0].origin;
					v_event[i].siglock->writerOut();
					continue;
				}
			}
			sig->event_cb = new EventCallBack(static_cast<AlarmHandler_ns::AlarmHandler *>(mydev));
			sig->first  = true;
			sig->first_err  = true;
			DEBUG_STREAM << "event_table::"<<__func__<<":Subscribing for " << sig->name << " " << (sig->first ? "FIRST" : "NOT FIRST") << endl;
			sig->siglock->writerOut();
			int		event_id = SUB_ERR;
			bool	isZMQ = true;
			bool	err = false;

			try
			{
				event_id = sig->attr->subscribe_event(
												Tango::CHANGE_EVENT,
												sig->event_cb,
												/*stateless=*/false);
				/*sig->evstate  = Tango::ON;
				//sig->first  = false;	//first event already arrived at subscribe_event
				sig->status.clear();
				sig->status = "Subscribed";
				DEBUG_STREAM << sig->name <<  "  Subscribed" << endl;*/

				//	Check event source  ZMQ/Notifd ?
				Tango::ZmqEventConsumer	*consumer =
						Tango::ApiUtil::instance()->get_zmq_event_consumer();
				isZMQ = (consumer->get_event_system_for_event_id(event_id) == Tango::ZMQ);

				DEBUG_STREAM << sig->name << "(id="<< event_id <<"):	Subscribed " << ((isZMQ)? "ZMQ Event" : "NOTIFD Event") << endl;
			}
			catch (Tango::DevFailed &e)
			{
				INFO_STREAM <<"event_table::"<<__func__<<": sig->attr->subscribe_event EXCEPTION:" << endl;
				err = true;
				Tango::Except::print_exception(e);
				sig->siglock->writerIn();
				sig->ex_reason = e.errors[0].reason;
				sig->ex_desc = e.errors[0].desc;
				sig->ex_origin = e.errors[0].origin;
				sig->event_id = SUB_ERR;
				delete sig->event_cb;
				sig->siglock->writerOut();
			}
			if(!err)
			{
				sig->siglock->writerIn();
				sig->event_id = event_id;
				sig->isZMQ = isZMQ;
				sig->siglock->writerOut();
			}
		}
		else
		{
			sig->siglock->writerOut();
		}
	}
	veclock.readerOut();
	initialized = true;
}

void event_table::start(string &signame)
{
	DEBUG_STREAM << "event_table::"<<__func__<< " entering signame=" << signame << endl;
	ReaderLock lock(veclock);
	vector<string> contexts;
	for (unsigned int i=0 ; i<v_event.size() ; i++)
	{
		if (v_event[i].name==signame)
		{
			v_event[i].siglock->writerIn();
			if(!v_event[i].running)
			{
				if(v_event[i].stopped)
				{
					static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeStoppedNumber_read--;
					try
					{
						add(signame, contexts, NOTHING, true);
					}
					catch (Tango::DevFailed &e)
					{
						//Tango::Except::print_exception(e);
						INFO_STREAM << "event_table::start: error adding  " << signame <<" err="<< e.errors[0].desc << endl;
						v_event[i].ex_reason = e.errors[0].reason;
						v_event[i].ex_desc = e.errors[0].desc;
						v_event[i].ex_origin = e.errors[0].origin;
						/*v_event[i].siglock->writerOut();
						return;*/
					}
				}
				v_event[i].running=true;
				if(v_event[i].paused)
					static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributePausedNumber_read--;
				static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeStartedNumber_read++;
				v_event[i].paused=false;
				v_event[i].stopped=false;
			}
			v_event[i].siglock->writerOut();
			return;
		}
	}
	for (unsigned int i=0 ; i<v_event.size() ; i++)
	{
#ifndef _MULTI_TANGO_HOST
		if (static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->compare_without_domain(v_event[i].name,signame))
#else
		if (!static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->compare_tango_names(v_event[i].name,signame))
#endif
		{
			v_event[i].siglock->writerIn();
			if(!v_event[i].running)
			{
				if(v_event[i].stopped)
				{
					static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeStoppedNumber_read--;
					try
					{
						add(signame, contexts, NOTHING, true);
					}
					catch (Tango::DevFailed &e)
					{
						//Tango::Except::print_exception(e);
						INFO_STREAM << "event_table::start: error adding  " << signame << endl;
						v_event[i].ex_reason = e.errors[0].reason;
						v_event[i].ex_desc = e.errors[0].desc;
						v_event[i].ex_origin = e.errors[0].origin;
						/*signals[i].siglock->writerOut();
						return;*/
					}
				}
				v_event[i].running=true;
				if(v_event[i].paused)
					static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributePausedNumber_read--;
				static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeStartedNumber_read++;
				v_event[i].paused=false;
				v_event[i].stopped=false;
			}
			v_event[i].siglock->writerOut();
			return;
		}
	}

	//	if not found
	Tango::Except::throw_exception(
				(const char *)"BadSignalName",
				"Signal " + signame + " NOT subscribed",
				(const char *)"event_table::start()");
}

void event_table::start_all()
{
	ReaderLock lock(veclock);
	vector<string> contexts;
	for (unsigned int i=0 ; i<v_event.size() ; i++)
	{
		v_event[i].siglock->writerIn();
		if(!v_event[i].running)
		{
			if(v_event[i].stopped)
			{
				string signame = v_event[i].name;
				static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeStoppedNumber_read--;
				try
				{
					add(signame, contexts, NOTHING, true);
				}
				catch (Tango::DevFailed &e)
				{
					//Tango::Except::print_exception(e);
					INFO_STREAM << "event_table::start: error adding  " << signame <<" err="<< e.errors[0].desc << endl;
					v_event[i].ex_reason = e.errors[0].reason;
					v_event[i].ex_desc = e.errors[0].desc;
					v_event[i].ex_origin = e.errors[0].origin;
					/*v_event[i].siglock->writerOut();
					return;*/
				}
			}
			v_event[i].running=true;
			if(v_event[i].paused)
				static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributePausedNumber_read--;
			static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->attr_AttributeStartedNumber_read++;
			v_event[i].paused=false;
			v_event[i].stopped=false;
		}
		v_event[i].siglock->writerOut();
	}
}


//=============================================================================
//=============================================================================
bool event_table::get_if_stop()
{
	//omni_mutex_lock sync(*this);
	return stop_it;
}
//=============================================================================
//=============================================================================
void event_table::stop_thread()
{
	//omni_mutex_lock sync(*this);
	stop_it = true;
	signal();
	//condition.signal();
}
//=============================================================================
/**
 *	return number of signals to be subscribed
 */
//=============================================================================
int event_table::nb_sig_to_subscribe()
{
	ReaderLock lock(veclock);

	int	nb = 0;
	for (unsigned int i=0 ; i<v_event.size() ; i++)
	{
		v_event[i].siglock->readerIn();
		if (v_event[i].event_id == SUB_ERR && !v_event[i].stopped)
		{
			nb++;
		}
		v_event[i].siglock->readerOut();
	}
	return nb;
}
//=============================================================================
/**
 *	build a list of signal to set HDB device property
 */
//=============================================================================
void event_table::put_signal_property()
{
	DEBUG_STREAM << "event_table::"<<__func__<<": put_signal_property entering action=" << action << endl;
	//ReaderLock lock(veclock);
	if (action>NOTHING)
	{
		static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->put_signal_property();
		if(action >= UPDATE_PROP)
			action--;
	}
	DEBUG_STREAM << "event_table::"<<__func__<<": put_signal_property exiting action=" << action << endl;
}


/*
 * EventCallBack class methods
 */
EventCallBack::EventCallBack(Tango::DeviceImpl *s):Tango::LogAdapter(s)
{
	//e_ptr = NULL;
	mydev = s;
}

EventCallBack::~EventCallBack(void)
{
	//e_ptr = NULL;
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
			extract_values(ev->attr_value, e.value, e.value_string, e.type);
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
	static_cast<AlarmHandler_ns::AlarmHandler *>(mydev)->evlist.push_back(e);
}  /* push_event() */

void EventCallBack::extract_values(Tango::DeviceAttribute *attr_value, vector<double> &val, string &val_string, int &type)
{
	Tango::DevState stval;
	vector<Tango::DevState> v_st;
	vector<Tango::DevULong> v_ulo;
	vector<Tango::DevUChar> v_uch;
	vector<Tango::DevShort> v_sh;
	vector<Tango::DevUShort> v_ush;
	vector<Tango::DevLong> v_lo;
	vector<Tango::DevDouble> v_do;
	vector<Tango::DevFloat> v_fl;
	vector<Tango::DevBoolean> v_bo;
	vector<Tango::DevLong64> v_lo64;
	vector<Tango::DevULong64> v_ulo64;
	vector<string> v_string;
	val_string = string("");

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
#if 1//TANGO_VER >= 600
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
	} else if (attr_value->get_type() == Tango::DEV_LONG64) {
		*(attr_value) >> v_lo64;
		for(vector<Tango::DevLong64>::iterator  it = v_lo64.begin(); it != v_lo64.end(); it++)
			val.push_back((double)(*it));		//convert all to double
		type = Tango::DEV_LONG64;
	} else if (attr_value->get_type() == Tango::DEV_ULONG64) {
		*(attr_value) >> v_ulo64;
		for(vector<Tango::DevULong64>::iterator  it = v_ulo64.begin(); it != v_ulo64.end(); it++)
			val.push_back((double)(*it));		//convert all to double
		type = Tango::DEV_ULONG64;
	} else if (attr_value->get_type() == Tango::DEV_STRING) {
		*(attr_value) >> v_string;
		val_string = *(v_string.begin());	//TODO: support string spectrum attrbutes
		type = Tango::DEV_STRING;
	}
	else {
		ostringstream o;
		o << "unknown type" << ends;
		throw o.str();
	}	
}

/*void EventCallBack::init(event_list* e)
{
	//e_ptr = e;
}*/


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
