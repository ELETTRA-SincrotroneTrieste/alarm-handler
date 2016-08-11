/*
 * log_thread.cpp
 *
 * $Author: graziano $
 *
 * $Revision: 1.7 $
 *
 * $Log: log_thread.cpp,v $
 *
 *
 * copyleft: Sincrotrone Trieste S.C.p.A. di interesse nazionale
 *           Strada Statale 14 - km 163,5 in AREA Science Park
 *           34012 Basovizza, Trieste ITALY
 */

#include "log_thread.h"
//#define _DEBUG_LOG_THREAD 0
//#define _DEBUG_LOG_THREAD 1

static const char __FILE__rev[] = __FILE__ " $Revision: 1.7 $";

/*
 * log_thread::log_thread()
 */
log_thread::log_thread(string dbhost, string dbuser, string dbpw, string dbname, int dbport, string instance_name/*, Alarm_ns::Alarm *p*/)/*: Tango::LogAdapter(p)*/
{
	//p_Alarm = p;
	m_dbhost = dbhost;
	m_dbuser = dbuser;
	m_dbpw = dbpw;
	m_dbname = dbname;
	m_dbport = dbport;
	m_instance_name = instance_name;
	
	//cout << __FILE__rev << endl;

	ostringstream err;
	if(!mysql_init(&log_db))
	{
		err << "mysql_init failed for log DB" << ends;
		throw err.str();
	}
	my_bool my_auto_reconnect=1;
	if(mysql_options(&log_db,MYSQL_OPT_RECONNECT,&my_auto_reconnect) !=0)
	{
		err << "mysql auto reconnection error for log DB: " << mysql_error(&log_db) << ends;;
		throw err.str();
	}

	if(!mysql_real_connect(&log_db, m_dbhost.c_str(), m_dbuser.c_str(), m_dbpw.c_str(), m_dbname.c_str(), m_dbport, NULL, 0))
	{
		err << "mysql_real_connect failed for log DB" << ends;
		throw err.str();
	}
}

/*
 * log_thread::~log_thread()
 */
log_thread::~log_thread()
{
	//p_Alarm = NULL;
	mysql_close(&log_db);
}

/*
 * log_thread::run()
 */
void log_thread::run(void *)
{
	while (true) {
		/*
		 * pop_front() will wait() on condition variable
		 */
		 try
		 {
			alm_log_t a = al_list.pop_front();
			if ((a.name == LOG_THREAD_EXIT) && \
					(a.time_s == LOG_THREAD_EXIT_TIME))
				break;

			write_db(a);
		}
		catch(omni_thread_fatal& ex)
		{
			ostringstream err;
			err << "omni_thread_fatal exception running log thread, err=" << ex.error << ends;
			//WARN_STREAM << "alarm_thread::run(): " << err.str() << endl;	
			printf("log_thread::run(): %s", err.str().c_str());
		}			
		catch(Tango::DevFailed& ex)
		{
			ostringstream err;
			err << "exception running log thread: '" << ex.errors[0].desc << "'" << ends;
			//WARN_STREAM << "alarm_thread::run(): " << err.str() << endl;	
			printf("log_thread::run(): %s", err.str().c_str());
			Tango::Except::print_exception(ex);
		}		
		catch(...)
		{
			//WARN_STREAM << "alarm_thread::run(): catched unknown exception!!" << endl;
			printf("log_thread::run(): catched unknown exception!!");
		}		
	}
}  /* alarm_thread::run() */

void log_thread::log_alarm_db(alm_log_t& a)
{
	al_list.push_back(a);
}

void log_thread::write_db(alm_log_t& a)
{
	ostringstream query_str;
	ostringstream err_msg;
	
	char *values_escaped = new char [2 * a.values.length() + 1];
	char *name_escaped = new char [2 * a.name.length() + 1];
	char *formula_escaped = new char [2 * a.formula.length() + 1];
	char *grp_escaped = new char [2 * a.grp.length() + 1];
	char *msg_escaped = new char [2 * a.msg.length() + 1];
	char *level_escaped = new char [2 * a.level.length() + 1];
	char *action_escaped = new char [2 * a.action.length() + 1];
	char *instance_escaped = new char [2 * m_instance_name.length() + 1];
	mysql_real_escape_string(&log_db, values_escaped, a.values.c_str(), a.values.length());
	mysql_real_escape_string(&log_db, name_escaped, a.name.c_str(), a.name.length());
	mysql_real_escape_string(&log_db, formula_escaped, a.formula.c_str(), a.formula.length());
	mysql_real_escape_string(&log_db, grp_escaped, a.grp.c_str(), a.grp.length());	
	mysql_real_escape_string(&log_db, msg_escaped, a.msg.c_str(), a.msg.length());	
	mysql_real_escape_string(&log_db, level_escaped, a.level.c_str(), a.level.length());
	mysql_real_escape_string(&log_db, action_escaped, a.action.c_str(), a.action.length());
	mysql_real_escape_string(&log_db, instance_escaped, m_instance_name.c_str(), m_instance_name.length());
	
	char *tmp_name_escaped;
	ostringstream tmp_names;
	for(vector<string>::iterator it=a.alm_list.begin(); it!=a.alm_list.end(); it++)
	{
		tmp_name_escaped = new char [2 * it->length() + 1];
		mysql_real_escape_string(&log_db, tmp_name_escaped, it->c_str(), it->length());
		tmp_names << " AND " << DESC_COL_NAME << "!='" << tmp_name_escaped << "'";
		delete [] tmp_name_escaped;
	}
			
	switch (a.type_log)
	{
		case TYPE_LOG_STATUS:
		//example: 
		//INSERT INTO alarms 
		//		(time_sec, time_usec, status, ack, id_description, attr_values) 
		//		SELECT 1234567890, 123456, 'ALARM', 'NACK', id_description, 'ev/ev/ev/ev2=1.;' 	//take id_description
		//			FROM description 															//from description
		//			WHERE name='al/al/al/al' AND active=1 AND instance='alarm_1';				//where this alarm is active
			if((a.status.size() == 0) || (a.ack.size() == 0) || (strlen(name_escaped) == 0))
			{
				err_msg << "log_thread::write_db(): ERROR some mandatory values adding alarm status are empty: status=" << 
						a.status << " ack=" << a.ack << " name=" << name_escaped << ends;
				break;
			}
			query_str << 
				"INSERT INTO " << m_dbname << "." << STAT_TABLE_NAME <<
					" (" << STAT_COL_TIME_S << "," << STAT_COL_TIME_U << "," <<		
						STAT_COL_STATUS << "," << STAT_COL_ACK << "," <<
						STAT_COL_DESC_ID <<	"," << STAT_COL_VALUES <<		
					") SELECT " << a.time_s << "," << a.time_us << ",'" <<				
						a.status << "','" << a.ack << "'," << 
						DESC_COL_ID << ",'" << values_escaped << "'" <<								
						" FROM " << DESC_TABLE_NAME <<
						" WHERE " << DESC_COL_NAME << "='" << name_escaped << "'" <<
							" AND " << DESC_COL_ACTIVE << "=" << ALARM_ACTIVE <<
							" AND " << DESC_COL_INSTANCE << "='" << instance_escaped << "'" <<
				ends;
			break;
		case TYPE_LOG_DESC_ADD:
		//example: 
		//INSERT INTO description 
		//		(name, active, time_sec, formula, time_threashold, level, grp, msg, action, instance) 
		//		SELECT 'al/al/al/al', 'alarm_1', 1, 1234567890, '(ev/ev/ev/evn < 1.2)', 'log', 'gr_none', 'msg'
		//			FROM description 
		//			WHERE name='al/al/al/al' AND active=1 AND instance='alarm_1'	//look if exists an alarm with the same name and active
		//			HAVING count(*) = 0;											//insert only if it does not  exist 
			if((strlen(name_escaped) == 0) || (strlen(instance_escaped) == 0) || (strlen(formula_escaped) == 0) || (strlen(level_escaped) == 0) ||
				 (strlen(grp_escaped) == 0) || (strlen(msg_escaped) == 0))
			{
				err_msg << "log_thread::write_db(): ERROR some mandatory values adding alarm description are empty: name=" << 
					name_escaped << " instance=" << instance_escaped << " formula=" << formula_escaped << " level=" << level_escaped << " grp=" << grp_escaped << 
					" msg=" << msg_escaped << ends;
				break;
			}			
			query_str << 
				"INSERT INTO " << m_dbname << "." << DESC_TABLE_NAME <<
					" (" << DESC_COL_NAME << "," << DESC_COL_ACTIVE << "," <<		
						DESC_COL_TIME_S << "," << DESC_COL_FORMULA << "," << 
						DESC_COL_TIME_THRESHOLD << "," << DESC_COL_LEVEL << "," << 
						DESC_COL_GRP << "," << DESC_COL_MSG << "," << 
						DESC_COL_ACTION << "," << DESC_COL_INSTANCE << "," <<
						DESC_COL_SILENT_TIME <<
					") SELECT '" << name_escaped << "'," << ALARM_ACTIVE << "," <<
						a.time_s << ",'" << formula_escaped << "','" <<				
					 	a.time_threshold << "','" << level_escaped << "','" << 
					 	grp_escaped << "','" <<	msg_escaped << "','" <<	
					 	action_escaped << "','" <<instance_escaped << "','" <<
					 	a.silent_time << "'" <<
						" FROM " << DESC_TABLE_NAME <<
						" WHERE " << DESC_COL_NAME << "='" << name_escaped << "'" << 
							" AND " << DESC_COL_ACTIVE << "=" << ALARM_ACTIVE <<
							" AND " << DESC_COL_INSTANCE << "='" << instance_escaped << "'" <<
						" HAVING COUNT(*) = 0" << 
				ends;
			break;
		case TYPE_LOG_DESC_DIS:
		//example: 
		//UPDATE description 								
		//		SET active=0 													//set active to 0
		//		WHERE name='al/al/al/al' AND active=1 AND instance='alarm_1';	//in alarm specified that is active
			if(strlen(name_escaped) == 0 || strlen(instance_escaped) == 0)
			{
				err_msg << "log_thread::write_db(): ERROR some mandatory values setting non-active alarm are empty: name=" << 
					name_escaped << " instance=" << instance_escaped << ends;
				break;
			}		
			query_str << 
				"UPDATE " << m_dbname << "." << DESC_TABLE_NAME <<
					" SET " << DESC_COL_ACTIVE << "=" << ALARM_REMOVED <<		
					" WHERE " << DESC_COL_NAME << "='" << name_escaped << "'" << 
							" AND " << DESC_COL_ACTIVE << "=" << ALARM_ACTIVE <<
							" AND " << DESC_COL_INSTANCE << "='" << instance_escaped << "'" <<
				ends;
			break;	
		case TYPE_LOG_DESC_REM:
		//example: 
		//DELETE FROM description 								
		//		WHERE name='al/al/al/al' AND active=1 AND instance='alarm_1';	//in alarm specified that is active
			if(strlen(name_escaped) == 0 || strlen(instance_escaped) == 0)
			{
				err_msg << "log_thread::write_db(): ERROR some mandatory values removing alarm are empty: name=" << 
					name_escaped << " instance=" << instance_escaped<< ends;
				break;
			}			
			query_str << 
				"DELETE FROM " << m_dbname << "." << DESC_TABLE_NAME <<
					" WHERE " << DESC_COL_NAME << "='" << name_escaped << "'" << 
							" AND " << DESC_COL_ACTIVE << "=" << ALARM_ACTIVE <<
							" AND " << DESC_COL_INSTANCE << "='" << instance_escaped << "'" <<
				ends;
			break;	
		case TYPE_LOG_DESC_SYNC:
		//example:
		//UPDATE description 
		//		SET active=0 
		//		WHERE name='al/al/al/al' 					//disable this alarm
		//			AND active=1 							//if it is active and
		//			AND instance='alarm_1'					//for this alarm instance and
		//			AND (formula!='(ev/ev/ev/ev > 1)'		//or formula is changed
		//				OR time_threashold!=5				//or time_threshold is changed		
		//				OR level!='log' 					//or level is changed
		//				OR grp!='gr_none' 					//or grp is changed
		//				OR msg!='message');					//or msg is changed
		//				OR action!='act/act/act/act');		//or action is changed		
			query_str << 
				"UPDATE " << m_dbname << "." << DESC_TABLE_NAME <<
					" SET " << DESC_COL_ACTIVE << "=" << ALARM_REMOVED <<		
					" WHERE " << DESC_COL_NAME << "='" << name_escaped << "'" << 
							" AND " << DESC_COL_ACTIVE << "=" << ALARM_ACTIVE <<
							" AND " << DESC_COL_INSTANCE << "='" << instance_escaped << "'" <<
							" AND (" << DESC_COL_FORMULA << "!='" << formula_escaped << "'" <<
									" OR " << DESC_COL_TIME_THRESHOLD << "!='" << a.time_threshold << "'" <<							
									" OR " << DESC_COL_LEVEL << "!='" << level_escaped << "'" <<
									" OR " << DESC_COL_GRP << "!='" << grp_escaped << "'" <<
									" OR " << DESC_COL_MSG << "!='" << msg_escaped << "'" <<
									" OR " << DESC_COL_ACTION << "!='" << action_escaped << "'" <<									
							")" <<
				ends;
			break;
		case TYPE_LOG_DESC_UPD_OLD:
		//example:
		//UPDATE description 
		//		SET active=0 
		//		WHERE name!='al1/al1/al1/al1' 					//disable every alarm not in this list
		//			AND name !='al2/al2/al2/al2'
		//			AND name !=...
		//			AND active=1 								//if it is active
		//			AND instance='alarm_1'
			query_str << 
				"UPDATE " << m_dbname << "." << DESC_TABLE_NAME <<
					" SET " << DESC_COL_ACTIVE << "=" << ALARM_REMOVED <<		
					" WHERE " << DESC_COL_ACTIVE << "=" << ALARM_ACTIVE << 
						" AND " << DESC_COL_INSTANCE << "='" << instance_escaped << "'" <<
						tmp_names.str() <<
				ends;
			break;
		case TYPE_LOG_DESC_UPDATE:
		//example:
		//UPDATE description
		//		SET time_sec=1234567890, time_threashold=1, level='log', grp='gr_none', msg='msg'
		//		WHERE name='al1/al1/al1/al1' 					//
		//			AND active=1 								//
		//			AND instance='alarm_1'
			query_str <<
				"UPDATE " << m_dbname << "." << DESC_TABLE_NAME <<
					" SET " << DESC_COL_TIME_S << "=" << a.time_s <<
					" , " << DESC_COL_TIME_THRESHOLD << "=" << a.time_threshold <<
					" , " << DESC_COL_LEVEL << "='" << level_escaped << "'" <<
					" , " << DESC_COL_GRP << "='" << grp_escaped << "'" <<
					" , " << DESC_COL_MSG << "='" << msg_escaped << "'" <<
					" , " << DESC_COL_ACTION << "='" << action_escaped << "'" <<
					" , " << DESC_COL_SILENT_TIME << "='" << a.silent_time << "'" <<
					" WHERE " << DESC_COL_NAME << "='" << name_escaped << "'" <<
						" AND " << DESC_COL_ACTIVE << "=" << ALARM_ACTIVE <<
						" AND " << DESC_COL_INSTANCE << "='" << instance_escaped << "'" <<
				ends;
			break;		
	}				
	delete [] values_escaped;
	delete [] name_escaped; 	
	delete [] formula_escaped; 	
	delete [] grp_escaped; 	
	delete [] msg_escaped; 	 
	delete [] level_escaped;
	delete [] action_escaped;	
	delete [] instance_escaped;
	if(err_msg.str().size() != 0)
	{
		cout << err_msg.str();
		return;
	} 		
	if(mysql_query(&log_db, query_str.str().c_str()))
	{
		err_msg << " log_thread::write_db(): ERROR in query=" << query_str.str() << endl;
		//cout << err_msg.str();
		//throw err_msg.str();
	}

	if(err_msg.str().size() != 0)
	{
		cout << gettime().tv_sec << err_msg.str();
		return;
	} 
	int num_rows = mysql_affected_rows(&log_db);
#ifdef _DEBUG_LOG_THREAD	
	cout << gettime().tv_sec << " log_thread::write_db(): Affected rows=" << num_rows << " in query=" << query_str.str() << endl;	
#else
	if(((a.type_log == TYPE_LOG_STATUS) || (a.type_log == TYPE_LOG_DESC_DIS)) && (num_rows != 1))
		cout << gettime().tv_sec << " log_thread::write_db(): Error num_rows=" << num_rows << " in query=" << query_str.str() << endl;			
#endif
}

void log_thread::get_alarm_list(vector<string> &al_list)
{
	ostringstream query_str;
	ostringstream err_msg;
	MYSQL_RES *res;
	MYSQL_ROW row;
	
	char *instance_escaped = new char [2 * m_instance_name.length() + 1];
	mysql_real_escape_string(&log_db, instance_escaped, m_instance_name.c_str(), m_instance_name.length());	
//	unsigned int num_fields;
//	unsigned int i;	
	
	//example:
	//SELECT description 
	//		CONCAT('\t',name,'\t',formula, 
	//			'\t',IFNULL(time_threshold,0),'\t',level,
	//			'\t',grp,'\t','\"',IFNULL(msg,''),'\"',
	//			'\t',IFNULL(action,''))
	//		FROM alarm.description
	//		WHERE active=1
	//			AND instance='alarm_1'
	query_str << 
		"SELECT" <<
			" CONCAT('\t', " << DESC_COL_NAME << ",'\t'," << DESC_COL_FORMULA <<
				",'\t'," << "IFNULL(" << DESC_COL_TIME_THRESHOLD << ",0)" << ",'\t'," << DESC_COL_LEVEL <<
				",'\t'," << "IFNULL(" << DESC_COL_SILENT_TIME << ",-1)" <<
				",'\t'," << DESC_COL_GRP <<	",'\t','\"'," << "IFNULL(" << DESC_COL_MSG << ",'')" <<
				",'\"','\t'," << "IFNULL(" << DESC_COL_ACTION << ",';')" << ")" <<
			" FROM " << m_dbname << "." << DESC_TABLE_NAME <<
			" WHERE " << DESC_COL_ACTIVE << "=" << ALARM_ACTIVE <<
				" AND " << DESC_COL_INSTANCE << "='" << instance_escaped << "'" <<
		ends;
	delete [] instance_escaped;		
	if(mysql_query(&log_db, query_str.str().c_str()))
	{
		err_msg << "log_thread::get_alarm_list(): ERROR in query=" << query_str.str() << endl;
		//cout << err_msg.str();
		throw err_msg.str();
	}	
//#define _DEBUG_LOG_THREAD 1
#ifdef _DEBUG_LOG_THREAD	
	else
		cout << gettime().tv_sec << " log_thread::get_alarm_list(): Success with query=" << query_str.str() << endl;
#endif 		//_DEBUG_LOG_THREAD	
	
	res = mysql_use_result(&log_db);
	if(res == NULL)
	{
		if(*mysql_error(&log_db))
			err_msg << "log_thread::get_alarm_list(): ERROR while retrieving result, err=" << mysql_error(&log_db) << endl;
		else
			err_msg << "log_thread::get_alarm_list(): ERROR while retrieving result" << endl;
		//cout << err_msg.str();
		throw err_msg.str();
	}
	//num_fields = mysql_num_fields(res);
	while ((row = mysql_fetch_row(res)))
	{
		al_list.push_back(row[0]);

#ifdef _DEBUG_LOG_THREAD	
		cout << gettime().tv_sec << " log_thread::get_alarm_list(): Retrieved line: " << row[0] << endl;
#endif 		//_DEBUG_LOG_THREAD		
	}	
	mysql_free_result(res);
}
/*
 * alarm_list class methods
 */
void alarm_list::push_back(alm_log_t& a)
{
	this->lock();
	try{
		l_alarm.push_back(a);		
		empty.signal();
	}
	catch(omni_thread_fatal& ex)
	{
		ostringstream err;
		err << "omni_thread_fatal exception signaling omni_condition, err=" << ex.error << ends;
		//WARN_STREAM << "alarm_list::push_back(): " << err.str() << endl;	
		printf("alarm_list::push_back(): %s", err.str().c_str());
	}			
	catch(Tango::DevFailed& ex)
	{
		ostringstream err;
		err << "exception  signaling omni_condition: '" << ex.errors[0].desc << "'" << ends;
		//WARN_STREAM << "alarm_list::push_back(): " << err.str() << endl;	
		printf("alarm_list::push_back: %s", err.str().c_str());
		Tango::Except::print_exception(ex);	
	}		
	catch(...)
	{
		//WARN_STREAM << "alarm_list::push_back(): catched unknown exception!!" << endl;
		printf("alarm_list::push_back(): catched unknown exception  signaling omni_condition!!");	
	}			
	this->unlock();
}

const alm_log_t alarm_list::pop_front(void)
{
	this->lock();
	//omni_mutex_lock l((omni_mutex)this);	//call automatically unlock on destructor and on exception
	try{
		while (l_alarm.empty() == true)
			empty.wait();					//wait release mutex while is waiting, then reacquire when signaled
	}
	catch(omni_thread_fatal& ex)
	{
		ostringstream err;
		err << "omni_thread_fatal exception waiting on omni_condition, err=" << ex.error << ends;
		//WARN_STREAM << "alarm_list::pop_front(): " << err.str() << endl;	
		printf("alarm_list::pop_front(): %s", err.str().c_str());
		alm_log_t a;
		this->unlock();
		sleep(1);
		return(a);
	}			
	catch(Tango::DevFailed& ex)
	{
		ostringstream err;
		err << "exception  waiting on omni_condition: '" << ex.errors[0].desc << "'" << ends;
		//WARN_STREAM << "alarm_list::pop_front(): " << err.str() << endl;	
		printf("alarm_list::pop_front: %s", err.str().c_str());
		Tango::Except::print_exception(ex);
		alm_log_t a;
		this->unlock();
		sleep(1);
		return(a);		
	}		
	catch(...)
	{
		//WARN_STREAM << "alarm_list::pop_front(): catched unknown exception!!" << endl;
		printf("alarm_list::pop_front(): catched unknown exception  waiting on omni_condition!!");
		alm_log_t a;
		this->unlock();
		sleep(1);
		return(a);		
	}			
	/*const*/ alm_log_t a;

	a = *(l_alarm.begin());
	l_alarm.pop_front();

	this->unlock();
	return(a);
}

void alarm_list::clear(void)
{
	//this->lock();
	l_alarm.clear();
	//this->unlock();
}

list<alm_log_t> alarm_list::show(void)
{
	list<alm_log_t> al;
	
	this->lock();
	al = l_alarm;
	this->unlock();
	return(al);
}
