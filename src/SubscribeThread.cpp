static const char *RcsId = "$Header: /home/cvsadm/cvsroot/fermi/servers/hdb++/hdb++es/src/SubscribeThread.cpp,v 1.6 2014-03-06 15:21:43 graziano Exp $";
//+=============================================================================
//
// file :         HdbEventHandler.cpp
//
// description :  C++ source for thread management
// project :      TANGO Device Server
//
// $Author: graziano $
//
// $Revision: 1.6 $
//
// $Log: SubscribeThread.cpp,v $
// Revision 1.6  2014-03-06 15:21:43  graziano
// StartArchivingAtStartup,
// start_all and stop_all,
// archiving of first event received at subscribe
//
// Revision 1.5  2014-02-20 14:59:02  graziano
// name and path fixing
// removed start acquisition from add
//
// Revision 1.4  2013-09-24 08:42:21  graziano
// bug fixing
//
// Revision 1.3  2013-09-02 12:13:22  graziano
// cleaned
//
// Revision 1.2  2013-08-23 10:04:53  graziano
// development
//
// Revision 1.1  2013-07-17 13:37:43  graziano
// *** empty log message ***
//
//
//
// copyleft :     European Synchrotron Radiation Facility
//                BP 220, Grenoble 38043
//                FRANCE
//
//-=============================================================================


#include "AlarmHandler.h"
#include "event_table.h"


namespace AlarmHandler_ns
{

//=============================================================================
//=============================================================================
SubscribeThread::SubscribeThread(AlarmHandler *dev):Tango::LogAdapter(dev)
{
	alarm_dev = dev;
	period = 10;	//TODO: configurable
	shared  = dev->events;
}
//=============================================================================
//=============================================================================
void SubscribeThread::updateProperty()
{
	shared->put_signal_property();
}
//=============================================================================
//=============================================================================
void *SubscribeThread::run_undetached(void *ptr)
{
	INFO_STREAM << "SubscribeThread id="<<omni_thread::self()->id()<<endl;
	while(shared->get_if_stop()==false)
	{
		//	Try to subscribe
		usleep(500000);		//TODO: try to delay a bit subscribe_events
		if(shared->get_if_stop())
			break;
		DEBUG_STREAM << "SubscribeThread::"<<__func__<<": AWAKE"<<endl;
		updateProperty();
		alarm_dev->events->subscribe_events();
		int	nb_to_subscribe = shared->nb_sig_to_subscribe();
		shared->check_signal_property(); //check if, while subscribing, new alarms to be saved in properties where added (update action)
		//	And wait a bit before next time or
		//	wait a long time if all signals subscribed
		{
			omni_mutex_lock sync(*shared);
			//shared->lock();
			int act=shared->action.load();
			if (nb_to_subscribe==0 && act == NOTHING)
			{
				DEBUG_STREAM << "SubscribeThread::"<<__func__<<": going to wait nb_to_subscribe=0"<<endl;
				//shared->condition.wait();
				shared->wait();
				//shared->wait(3*period*1000);
			}
			else if(shared->action == NOTHING)
			{
				DEBUG_STREAM << "SubscribeThread::"<<__func__<<": going to wait period="<<period<<"  nb_to_subscribe="<<nb_to_subscribe<<endl;
				//unsigned long s,n;
				//omni_thread::get_time(&s,&n,period,0);
				//shared->condition.timedwait(s,n);
				shared->wait(period*1000);
			}
			//shared->unlock();
		}
	}
	//shared->unsubscribe_events();
	INFO_STREAM <<"SubscribeThread::"<< __func__<<": exiting..."<<endl;
	return NULL;
}
//=============================================================================
//=============================================================================



}	//	namespace
