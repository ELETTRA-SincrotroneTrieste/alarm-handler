/*
 * alarm-thread.h
 *
 * $Author: graziano $
 *
 * $Revision: 1.3 $
 *
 * $Log: alarm-thread.h,v $
 * Revision 1.3  2008-04-23 15:00:57  graziano
 * small code cleanings
 *
 *
 *
 * copyleft: Sincrotrone Trieste S.C.p.A. di interesse nazionale
 *           Strada Statale 14 - km 163,5 in AREA Science Park
 *           34012 Basovizza, Trieste ITALY
 */

#ifndef ALARM_THREAD_H
#define ALARM_THREAD_H

#include <omnithread.h>
#include <tango.h>
#include <Alarm.h>

#define ALARM_THREAD_EXIT				"alarm_thread_exit"
#define ALARM_THREAD_EXIT_VALUE	-100
#define ALARM_THREAD_TO_BE_EVAL			"to_be_evaluated"
#define ALARM_THREAD_TO_BE_EVAL_VALUE	-200

class alarm_thread : public omni_thread {
	public:
		alarm_thread(Alarm_ns::Alarm *p);
		~alarm_thread();
		//int period;
	protected:
		void run(void *);
	private:
		Alarm_ns::Alarm *p_Alarm;
};

#endif	/* ALARM_THREAD_H */

/* EOF */
