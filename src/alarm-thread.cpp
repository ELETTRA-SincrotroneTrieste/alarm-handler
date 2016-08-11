/*
 * alarm-thread.cpp
 *
 * $Author: claudio $
 *
 * $Revision: 1.7 $
 *
 * $Log: alarm-thread.cpp,v $
 * Revision 1.7  2015-07-21 13:40:59  claudio
 * minor cleanups
 *
 * Revision 1.6  2013-03-06 10:41:11  claudio
 * commented out debug print statements
 *
 * Revision 1.5  2008-07-08 12:11:39  graziano
 * omni_thread_fatal exception handling
 *
 * Revision 1.4  2008/07/07 09:13:12  graziano
 * omni_thread_fatal exception handling
 *
 * Revision 1.3  2008/04/24 06:51:34  graziano
 * small code cleanings
 *
 *
 *
 * copyleft: Sincrotrone Trieste S.C.p.A. di interesse nazionale
 *           Strada Statale 14 - km 163,5 in AREA Science Park
 *           34012 Basovizza, Trieste ITALY
 */

#include "alarm-thread.h"
static const char __FILE__rev[] = __FILE__ " $Revision: 1.7 $";

/*
 * alarm_thread::alarm_thread()
 */
alarm_thread::alarm_thread(Alarm_ns::Alarm *p) : p_Alarm(p) 
{
	//cout << __FILE__rev << endl;
}

/*
 * alarm_thread::~alarm_thread()
 */
alarm_thread::~alarm_thread()
{
	p_Alarm = NULL;
}

/*
 * alarm_thread::run()
 */
void alarm_thread::run(void *)
{
	while (true) {
		/*
		 * pop_front() will wait() on condition variable
		 */
		try
		{
			bei_t e = p_Alarm->evlist.pop_front();
			//DEBUG_STREAM << "alarm_thread::run(): woken up!!!! " << e.name << endl;
			if ((e.ev_name == ALARM_THREAD_EXIT) && \
				(e.value[0] == ALARM_THREAD_EXIT_VALUE))
				break;
			p_Alarm->do_alarm(e);
		}
		catch(omni_thread_fatal& ex)
		{
			ostringstream err;
			err << "omni_thread_fatal exception running alarm thread, err=" << ex.error << ends;
			//WARN_STREAM << "alarm_thread::run(): " << err.str() << endl;	
			printf("alarm_thread::run(): %s", err.str().c_str());
		}			
		catch(Tango::DevFailed& ex)
		{
			ostringstream err;
			err << "exception running alarm thread: '" << ex.errors[0].desc << "'" << ends;
			//WARN_STREAM << "alarm_thread::run(): " << err.str() << endl;	
			printf("alarm_thread::run(): %s", err.str().c_str());
			Tango::Except::print_exception(ex);
		}		
		catch(...)
		{
			//WARN_STREAM << "alarm_thread::run(): catched unknown exception!!" << endl;
			printf("alarm_thread::run(): catched unknown exception!!");
		}		
	}
	//cout << "alarm_thread::run(): returning" << endl;
}  /* alarm_thread::run() */
