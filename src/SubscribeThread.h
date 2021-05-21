//=============================================================================
//
// file :        HdbEventHandler.h
//
// description : Include for the HDbDevice class.
//
// project :	Tango Device Server
//
// $Author: graziano $
//
// $Revision: 1.5 $
//
// $Log: SubscribeThread.h,v $
// Revision 1.5  2014-03-06 15:21:43  graziano
// StartArchivingAtStartup,
// start_all and stop_all,
// archiving of first event received at subscribe
//
// Revision 1.4  2013-09-24 08:42:21  graziano
// bug fixing
//
// Revision 1.3  2013-09-02 12:11:32  graziano
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
// copyleft :    European Synchrotron Radiation Facility
//               BP 220, Grenoble 38043
//               FRANCE
//
//=============================================================================

#ifndef _SUBSCRIBE_THREAD_H
#define _SUBSCRIBE_THREAD_H

#include <tango.h>
#include <eventconsumer.h>
#include <stdint.h>
#include "event_table.h"

/**
 * @author	$Author: graziano $
 * @version	$Revision: 1.5 $
 */

 //	constants definitions here.
 //-----------------------------------------------

namespace AlarmHandler_ns
{

//class ArchiveCB;
class AlarmHandler;

class SubscribeThread;

//=========================================================
/**
 *	Create a thread retry to subscribe event.
 */
//=========================================================
class SubscribeThread: public omni_thread, public Tango::LogAdapter
{
private:
	/**
	 *	Shared data
	 */
	event_table	*shared;
	/**
	 *	HdbDevice object
	 */
	AlarmHandler	*alarm_dev;


public:
	int			period;
	SubscribeThread(AlarmHandler *dev);
	void updateProperty();
	void signal();
	/**
	 *	Execute the thread loop.
	 *	This thread is awaken when a command has been received 
	 *	and falled asleep when no command has been received from a long time.
	 */
	void *run_undetached(void *);
	void start() {start_undetached();}
};


}	// namespace_ns

#endif	// _SUBSCRIBE_THREAD_H
