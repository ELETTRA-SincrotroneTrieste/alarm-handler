/*
 * update-thread.cpp
 *
 * $Author: claudio $
 *
 * $Revision: 1.2 $
 *
 * $Log: update-thread.cpp,v $
 * Revision 1.2  2013-03-06 10:38:43  claudio
 * commented out debug print statements
 *
 * Revision 1.1  2008-11-10 10:54:09  graziano
 * thread for update of alarms with time threshold > 0
 *
 *
 *
 *
 * copyleft: Sincrotrone Trieste S.C.p.A. di interesse nazionale
 *           Strada Statale 14 - km 163,5 in AREA Science Park
 *           34012 Basovizza, Trieste ITALY
 */

#include "update-thread.h"
static const char __FILE__rev[] = __FILE__ " $Revision: 1.2 $";

/*
 * alarm_thread::alarm_thread()
 */
update_thread::update_thread(AlarmHandler_ns::AlarmHandler *p) : p_Alarm(p),Tango::LogAdapter(p)
{
	//cout << __FILE__rev << endl;
}

/*
 * alarm_thread::~alarm_thread()
 */
update_thread::~update_thread()
{
	DEBUG_STREAM << __func__ << "update_thread::~update_thread(): entering!" << endl;
	p_Alarm = NULL;
}

/*
 * alarm_thread::run()
 */
void update_thread::run(void *)
{
	DEBUG_STREAM << __func__ << "update_thread::run(): entering!" << endl;
	//printf("update_thread::run(): running...\n");
	unsigned int pausasec, pausanano;
	pausasec=1;
	pausanano = 100000000;		//0.1 sec
	omni_thread::sleep(pausasec,pausanano);
	while (!p_Alarm->abortflag) {
		try
		{
			//omni_thread::sleep(pausasec,pausanano);
			abort_sleep(pausasec+(double)pausanano/1000000000);
			if(!p_Alarm->abortflag)
				p_Alarm->timer_update();
			//printf("update_thread::run(): TIMER!!\n");
		}		
		catch(...)
		{
			INFO_STREAM << "update_thread::run(): catched unknown exception!!";
		}		
	}
	DEBUG_STREAM << "update_thread::run(): exiting!" << endl;
}  /* update_thread::run() */

void update_thread::abort_sleep(double time)
{
	omni_mutex_lock sync(*this);
	long time_millis = (long)(time*1000);
	int res = wait(time_millis);
}
