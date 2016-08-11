/*
 *cmd_thread.h
 *
 * $Author: graziano $
 *
 * $Revision: 1.2 $
 *
 * $Log: cmd_thread.h,v $
 * Revision 1.2  2008-11-17 13:10:36  graziano
 * command action can be: without arguments or with string argument
 *
 * Revision 1.1  2008/11/10 10:53:31  graziano
 * thread for execution of commands
 *
 * Revision 1.2  2008/10/07 14:03:07  graziano
 * added handling of multi axis groups and spindle groups
 *
 * Revision 1.1  2008/09/30 09:46:32  graziano
 * first version
 *
 * Revision 1.3  2008/04/23 15:00:57  graziano
 * small code cleanings
 *
 *
 *
 * copyleft: Sincrotrone Trieste S.C.p.A. di interesse nazionale
 *           Strada Statale 14 - km 163,5 in AREA Science Park
 *           34012 Basovizza, Trieste ITALY
 */

#ifndef CMD_THREAD_H
#define CMD_THREAD_H

#ifndef _LOGA
#define _LOGA	1
#endif		// _LOGA

//#define CMD_THREAD_EXIT	"EXIT"
#define CMD_COMMAND		1
#define CMD_RESPONSE	2
#define CMD_THREAD_EXIT	3

#include <omnithread.h>
#include <tango.h>
#include "Alarm.h"

struct cmd_t
{
	short cmd_id;
	long call_id;
	string arg_s1;
	string arg_s2;
	string arg_s3;
	bool arg_b;
	long dp_add;
};

class cmd_list : public omni_mutex {
	public:
		cmd_list(void): empty(this) {}
		~cmd_list(void) {}
		void push_back(cmd_t& cmd);
		const cmd_t pop_front(void);
		void clear(void);
	protected:
		list<cmd_t> l_cmd;
	private:
		omni_condition empty;
};

class cmd_thread : public omni_thread{
	public:
		cmd_thread();
		~cmd_thread();
		cmd_list list;
		//omni_mutex::omni_mutex *mutex_dp;
	protected:
		void run(void *);
	private:
		Tango::DeviceProxy *dp;
};

#endif	/* CMD_THREAD_H */

/* EOF */
