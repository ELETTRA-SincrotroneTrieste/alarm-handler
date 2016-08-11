/*
 * log_thread.h
 *
 * $Author: graziano $
 *
 * $Revision: 1.4 $
 *
 * $Log: log_thread.h,v $
 *
 *
 * copyleft: Sincrotrone Trieste S.C.p.A. di interesse nazionale
 *           Strada Statale 14 - km 163,5 in AREA Science Park
 *           34012 Basovizza, Trieste ITALY
 */

#ifndef LOG_THREAD_H
#define LOG_THREAD_H

#include <omnithread.h>
#include <tango.h>
#include <Alarm.h>
#include <mysql.h>

#define LOG_THREAD_EXIT				"log_thread_exit"
#define LOG_THREAD_EXIT_TIME		1

//############# DB ############
//#define LOG_DB_NAME			"alarm"
//######## ALARM_DESC ########
#define DESC_TABLE_NAME			"description"
#define DESC_COL_ID				"id_description"
#define DESC_COL_NAME			"name"
#define DESC_COL_INSTANCE		"instance"
#define DESC_COL_ACTIVE			"active"
#define DESC_COL_TIME_S			"time_sec"
#define DESC_COL_FORMULA		"formula"
#define DESC_COL_SILENT_TIME	"silent_time"
#define DESC_COL_TIME_THRESHOLD	"time_threshold"
#define DESC_COL_LEVEL			"level"
#define DESC_COL_GRP			"grp"
#define DESC_COL_MSG			"msg"
#define DESC_COL_ACTION			"action"
//######## ALARM_STATUS #######
#define STAT_TABLE_NAME		"alarms"
#define STAT_COL_ID			"id_alarms"
#define STAT_COL_TIME_S		"time_sec"
#define STAT_COL_TIME_U		"time_usec"
#define STAT_COL_STATUS		"status"
#define STAT_COL_ACK		"ack"
#define STAT_COL_DESC_ID	DESC_COL_ID
#define STAT_COL_VALUES		"attr_values"


#define TYPE_LOG_STATUS			1
#define TYPE_LOG_DESC_ADD		2
#define TYPE_LOG_DESC_DIS		3
#define TYPE_LOG_DESC_REM		4
#define TYPE_LOG_DESC_SYNC		5
#define TYPE_LOG_DESC_UPD_OLD	6
#define TYPE_LOG_DESC_UPDATE	7

#define ALARM_ACTIVE		1
#define ALARM_REMOVED		0


typedef struct {
	unsigned int type_log;
	unsigned int time_s;
	unsigned int time_us;
	unsigned int time_threshold;
	int silent_time;
	string name;
	string status;
	string ack;
	string level;
	string grp;
	string msg;
	string formula;
	string action;
	string values;
	vector<string> alm_list;
} alm_log_t;

/*
 * here Alarm insert data to log, log_thread process data
 * and store in db
 */
class alarm_list : public omni_mutex {
	public:
		alarm_list(void): full(this), empty(this) {}
		~alarm_list(void) {}
		void push_back(alm_log_t& a);
		const alm_log_t pop_front(void);
		void clear(void);
		list<alm_log_t> show(void);
	protected:
		list<alm_log_t> l_alarm;
	private:
		omni_condition full,
					empty;
};


class log_thread : public omni_thread/*, public Tango::LogAdapter*/ 
{
	public:
		log_thread(string dbhost, string dbuser, string dbpw, string dbname, int dbport, string instance_name/*, Alarm_ns::Alarm *p*/);
		~log_thread();
		
		void log_alarm_db(alm_log_t& a);
		void get_alarm_list(vector<string> &al_list);		
		
	protected:
		void run(void *);
	private:
		//Alarm_ns::Alarm *p_Alarm;
		MYSQL log_db;
		
		alarm_list al_list;	
		
		string m_dbhost;
		string m_dbuser;
		string m_dbpw;
		string m_dbname;
		int m_dbport;
		string m_instance_name;
		
		void write_db(alm_log_t& a);
};

#endif	/* LOG_THREAD_H */

/* EOF */
