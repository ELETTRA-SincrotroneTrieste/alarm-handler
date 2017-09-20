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
alarm_thread::alarm_thread(AlarmHandler_ns::AlarmHandler *p) : p_Alarm(p)
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
	size_t to_be_evaluated = 0;
	//int period = 5; //seconds
	int awaken = 0;
	vector<string> alm_to_be_eval;
	bool starting = true;
	timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	double last_eval = (now.tv_sec) + ((double)(now.tv_nsec))/1000000000;

	while(starting)
	{
		starting = Tango::Util::instance()->is_svr_starting();
		usleep(200000);
	}

	alm_to_be_eval = p_Alarm->alarms.to_be_evaluated_list();
	to_be_evaluated = alm_to_be_eval.size();

	while (true) {
		/*
		 * pop_front() will wait() on condition variable
		 */
		try
		{
			if(to_be_evaluated > 0)
			{
				clock_gettime(CLOCK_MONOTONIC, &now);
				double dnow = (now.tv_sec) + ((double)(now.tv_nsec))/1000000000;
				if(dnow - last_eval > 10)	//TODO: configurable
				{
					last_eval = dnow;
					bool changed = true;
					int num_changed = 0;
					if(alm_to_be_eval.size() > 0)
					{
						for(vector<string>::iterator i = alm_to_be_eval.begin(); i != alm_to_be_eval.end(); i++)
						{
							changed = p_Alarm->do_alarm_eval(*i, "FORCED_EVAL", gettime());
							if(changed)
								num_changed++;
						}
#if 0	//TODO
						prepare_alarm_attr();
						if(num_changed==0)
							return;
						if(ds_num == 0)
						{
							//attr.set_value_date_quality(ds,0/*gettime()*/,Tango::ATTR_WARNING, ds_num, 0, false);
							struct timeval now;
							gettimeofday(&now,NULL);
							push_change_event("alarm",(char**)ds,now,Tango::ATTR_WARNING, ds_num, 0, false);
						}
						else
							//attr.set_value(ds, ds_num, 0, false);
							push_change_event("alarm",ds, ds_num, 0, false);
#endif
					}
#if 1
					alm_to_be_eval = p_Alarm->alarms.to_be_evaluated_list();
					to_be_evaluated = alm_to_be_eval.size();
#else
					to_be_evaluated = 0;
#endif
					//usleep(200000);	//TODO
				}
			}
			bei_t e;
			{
				e = p_Alarm->evlist.pop_front();
				//DEBUG_STREAM << "alarm_thread::run(): woken up!!!! " << e.name << endl;
				if ((e.ev_name == ALARM_THREAD_EXIT) && \
					(e.value[0] == ALARM_THREAD_EXIT_VALUE))
				{
					break;
				}
				else if ((e.ev_name == ALARM_THREAD_TO_BE_EVAL) && \
					(e.value[0] == ALARM_THREAD_TO_BE_EVAL_VALUE))
				{
					alm_to_be_eval = p_Alarm->alarms.to_be_evaluated_list();
					to_be_evaluated = alm_to_be_eval.size();
					continue;
				}
				p_Alarm->do_alarm(e);
			}
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
