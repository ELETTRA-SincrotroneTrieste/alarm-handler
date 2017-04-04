/*
 * cmd_thread.cpp
 *
 * $Author: claudio $
 *
 * $Revision: 1.3 $
 *
 * $Log: cmd_thread.cpp,v $
 * Revision 1.3  2015-07-21 13:40:59  claudio
 * minor cleanups
 *
 * Revision 1.2  2008-11-17 13:13:21  graziano
 * command action can be: without arguments or with string argument
 *
 * Revision 1.1  2008/11/10 10:53:31  graziano
 * thread for execution of commands
 *
 *
 *
 *
 *
 * copyleft: Sincrotrone Trieste S.C.p.A. di interesse nazionale
 *           Strada Statale 14 - km 163,5 in AREA Science Park
 *           34012 Basovizza, Trieste ITALY
 */

#include "cmd_thread.h"
static const char __FILE__rev[] = __FILE__ " $Revision: 1.3 $";

/*
 * cmd_thread::cmd_thread()
 */
cmd_thread::cmd_thread() 
{
	cout << __FILE__rev << endl;
	cout << gettime().tv_sec << " cmd_thread::cmd_thread(): constructor... !" << endl;	
	//mutex_dp = new omni_mutex::omni_mutex();
}

/*
 * cmd_thread::~cmd_thread()
 */
cmd_thread::~cmd_thread()
{
	cout << gettime().tv_sec << " cmd_thread::~cmd_thread(): delete device entering..." << endl;
	//delete mutex_dp;	
}

/*
 * cmd_thread::run()
 */
void cmd_thread::run(void *)
{
	long call_id;
	while (true) {
		/*
		 * pop_front() will wait() on condition variable
		 */
		try
		{
			cmd_t cmd = list.pop_front();
			switch(cmd.cmd_id)
			{
				case CMD_THREAD_EXIT:
					cout << gettime().tv_sec << " cmd_thread::run(): received command THREAD_EXIT -> exiting..." << endl;				
				return;
				
				case CMD_COMMAND:
				{
					try {
						cout << gettime().tv_sec << " cmd_thread::run(): COMMAND ... action=" << cmd.arg_s3 << endl;
						dp = (Tango::DeviceProxy *)cmd.dp_add;
						if(cmd.arg_b)
						{
							Tango::DevString str = CORBA::string_dup(cmd.arg_s1.c_str());
							Tango::DeviceData Din;
							Din << str;
							CORBA::string_free(str);
							//found->second.dp_a->ping();
							//call_id = dp->command_inout_asynch(cmd.arg_s2, Din, true);		//true -> "fire and forget" mode: client do not care at all about the server answer
							call_id = dp->command_inout_asynch(cmd.arg_s2, Din);		//true -> "fire and forget" mode: client do not care at all about the server answer
						}
						else
							call_id = dp->command_inout_asynch(cmd.arg_s2);
						cout << gettime().tv_sec <<  " cmd_thread::run() executed action: " << cmd.arg_s3 << " !!! call_id=" << call_id << endl;
						/*cmd_t arg;
						arg.cmd_id = CMD_RESPONSE;
						arg.call_id = call_id;
						arg.dp_add = cmd.dp_add;
						arg.arg_s1 = cmd.arg_s1;
						arg.arg_s2 = cmd.arg_s2;
						arg.arg_s3 = cmd.arg_s3;	
						cmdloop->list.push_back(arg);*/
						cmd.cmd_id = CMD_RESPONSE;			//if no exception till now push in list request of response
						cmd.call_id = call_id;				//if no exception till now push in list request of response
						list.push_back(cmd);		//if no exception till now push in list request of response
					} catch(Tango::DevFailed &e)
					{
						TangoSys_MemStream out_stream;
						string err(e.errors[0].desc);
						if(err.find("is not yet arrived") == string::npos)			//TODO: change this!!
						{
							out_stream << "Failed to execute action " << cmd.arg_s3 << ", err=" << e.errors[0].desc << ends;
							cout << gettime().tv_sec <<  " cmd_thread::run() ERROR: " << out_stream.str() << endl;
						}
						else
						{
							cout << gettime().tv_sec <<  " cmd_thread::run() exception 'is not yet arrived': pushing request of response, call_id=" << call_id << endl;
							cmd.cmd_id = CMD_RESPONSE;			//if no exception till now push in list request of response
							cmd.call_id = call_id;				//if no exception till now push in list request of response
							list.push_back(cmd);		//if no exception till now push in list request of response						
						}
						
					}					
				}	
				break;
				
				case CMD_RESPONSE:
				{
					//cout << gettime().tv_sec << " cmd_thread::run(): RESPONSE WAKE UP... action=" << cmd.arg_s3 << " call_id=" << cmd.call_id << endl;
					Tango::DeviceData resp;
					dp = (Tango::DeviceProxy *)cmd.dp_add;
					try {
						resp = dp->command_inout_reply(cmd.call_id);
						cout << gettime().tv_sec << " cmd_thread::run() RECEIVED response to action " << cmd.arg_s3 << endl;
					} catch(Tango::DevFailed &e)
					{
						TangoSys_MemStream out_stream;
						out_stream << "EXCEPTION executing action " << cmd.arg_s3 << ", err=" << e.errors[0].desc << ends;
						
						if(out_stream.str().find("is not yet arrived") != string::npos)			//TODO: change this!!
						{
							
							list.push_back(cmd);				//if exception "not yet arrived" push in list another request of response		
							omni_thread::sleep(0,300000000);	//0.3 s
						}
						else
						{
							cout << gettime().tv_sec << " cmd_thread::run() " <<  out_stream.str() << endl;
							Tango::Except::print_exception(e);
						}
					}				
				}
				break;		
			}
/*			if(cmd.arg_s == CMD_THREAD_EXIT)
			{
				cout << gettime().tv_sec << " cmd_thread::run(): received command THREAD_EXIT -> exiting..." << endl;				
				return;
			}
			else
			{
				cout << gettime().tv_sec << " cmd_thread::run(): WAKE UP... action=" << cmd.arg_s << endl;
				Tango::DeviceData resp;
				dp = (Tango::DeviceProxy *)cmd.dp_add;
				try {
					mutex_dp->lock();
					resp = dp->command_inout_reply(cmd.cmd_id);
					mutex_dp->unlock();
					cout << gettime().tv_sec << " cmd_thread::run() received response to action " << cmd.arg_s << endl;
				} catch(Tango::DevFailed &e)
				{
					TangoSys_MemStream out_stream;
					out_stream << "EXCEPTION executing action " << cmd.arg_s << ", err=" << e.errors[0].desc << ends;
					
					if(out_stream.str().find("is not yet arrived") != string::npos)			//TODO: change this!!
					{
						
						list.push_back(cmd);
						omni_thread::sleep(0,300000000);	//0.2 s
					}
					else
						cout << gettime().tv_sec << " cmd_thread::run() " <<  out_stream.str() << endl;
				}
			}*/
			
		}
		catch(omni_thread_fatal& ex)
		{
			ostringstream err;
			err << "omni_thread_fatal exception running command thread, err=" << ex.error << ends;
			//WARN_STREAM << "alarm_thread::run(): " << err.str() << endl;	
			printf("cmd_thread::run(): %s", err.str().c_str());			
		}			
		catch(Tango::DevFailed& ex)
		{
			ostringstream err;
			err << "exception running command thread: '" << ex.errors[0].desc << "'" << ends;
			//WARN_STREAM << "alarm_thread::run(): " << err.str() << endl;	
			printf("cmd_thread::run(): %s", err.str().c_str());			
			Tango::Except::print_exception(ex);
		}		
		catch(...)
		{
			//WARN_STREAM << "alarm_thread::run(): catched unknown exception!!" << endl;
			printf("cmd_thread::run(): catched unknown exception!!");
		}		
	}
	//cout << "alarm_thread::run(): returning" << endl;
}  /* cmd_thread::run() */

/*
 * cmd_list class methods
 */
void cmd_list::push_back(cmd_t& cmd)
{
	this->lock();

	//cout << "event_list::push_back: " << e.name << " value=" << e.value[0] << endl;
	try{
		l_cmd.push_back(cmd);		
		empty.signal();
	}
	catch(omni_thread_fatal& ex)
	{
		ostringstream err;
		err << "omni_thread_fatal exception signaling omni_condition, err=" << ex.error << ends;
		//WARN_STREAM << "event_list::push_back(): " << err.str() << endl;	
		printf("cmd_list::push_back(): %s", err.str().c_str());
	}			
	catch(Tango::DevFailed& ex)
	{
		ostringstream err;
		err << "exception  signaling omni_condition: '" << ex.errors[0].desc << "'" << ends;
		//WARN_STREAM << "event_list::push_back(): " << err.str() << endl;	
		printf("cmd_list::push_back: %s", err.str().c_str());
		Tango::Except::print_exception(ex);	
	}		
	catch(...)
	{
		printf("cmd_list::push_back(): catched unknown exception  signaling omni_condition!!");	
	}	
	this->unlock();
}

const cmd_t cmd_list::pop_front(void)
{
	this->lock();
	//omni_mutex_lock l((omni_mutex)this);	//call automatically unlock on destructor and on exception
	try{
		while (l_cmd.empty() == true)
			empty.wait();					//wait release mutex while is waiting, then reacquire when signaled
	}
	catch(omni_thread_fatal& ex)
	{
		ostringstream err;
		err << "omni_thread_fatal exception waiting on omni_condition, err=" << ex.error << ends;
		printf("cmd_list::pop_front(): %s", err.str().c_str());
		this->unlock();
		sleep(1);
		cmd_t c;
		return c;
	}			
	catch(Tango::DevFailed& ex)
	{
		ostringstream err;
		err << "exception  waiting on omni_condition: '" << ex.errors[0].desc << "'" << ends;	
		printf("cmd_list::pop_front: %s", err.str().c_str());
		Tango::Except::print_exception(ex);
		this->unlock();
		sleep(1);
		cmd_t c;
		return c;		
	}		
	catch(...)
	{
		printf("cmd_list::pop_front(): catched unknown exception  waiting on omni_condition!!");
		this->unlock();
		sleep(1);
		cmd_t c;
		return c;		
	}			
	/*const*/ cmd_t cmd;

	cmd = *(l_cmd.begin());
	//cout << "event_list::pop_front: " << e.name << " value=" << e.value[0] << endl; 
	l_cmd.pop_front();

	this->unlock();
	return cmd;
}

void cmd_list::clear(void)
{
	//this->lock();
	l_cmd.clear();
	//this->unlock();
}
