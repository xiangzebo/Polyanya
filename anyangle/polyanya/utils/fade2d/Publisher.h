#pragma once
#include "common.h"
#include "MsgBase.h"

#if GEOM_PSEUDO3D==GEOM_TRUE
	namespace GEOM_FADE25D {
#elif GEOM_PSEUDO3D==GEOM_FALSE
	namespace GEOM_FADE2D {
#else
	#error GEOM_PSEUDO3D is not defined
#endif

typedef std::multimap<MsgType,MsgBase*> MMTypMsg;
typedef MMTypMsg::iterator MMTypMsgIt;
class Publisher
{
public:
	Publisher():progress_todo(0),progress_done(0),progress_broadcast_interval(0),bEndPublished(false)
	{
	}
	void startProgress(const std::string& name,size_t todo)
	{
		if(mmSubscribers.empty()) return;
		progress_todo=todo;
		progress_done=0;
		progress_name=name;
		progress_broadcast_interval=progress_todo/100;
		if(progress_broadcast_interval<1) progress_broadcast_interval=1;
		bEndPublished=false;
	}
	void endProgress()
	{
		if(mmSubscribers.empty()) return;
		progress_done=progress_todo;
		if(!bEndPublished) broadcast(MSG_PROGRESS,progress_name,1.0);
		progress_name.clear();
		progress_todo=0;
		progress_done=0;
	}
	void incProgress(size_t increment=1)
	{
		if(mmSubscribers.empty() || progress_todo==0 || bEndPublished) return;
		progress_done+=increment;
		if(progress_done%progress_broadcast_interval==0)
		{
			double prog(double(progress_done)/double(progress_todo));
			if(prog>=1.0)
			{
				prog=1.0;
				bEndPublished=true;
			}
			broadcast(MSG_PROGRESS,progress_name,prog);
		}
	}

	void incTodo(size_t increment=1)
	{
		if(mmSubscribers.empty()) return;
		progress_todo+=increment;
	}

	void broadcast(MsgType msgType,const std::string& s,double d)
	{
		std::pair<MMTypMsgIt,MMTypMsgIt> result(mmSubscribers.equal_range(msgType));
		for(;result.first!=result.second;++result.first)
		{
			MsgBase* pCurrentMsgBase(result.first->second);
			pCurrentMsgBase->update(msgType,s,d);
		}
	}

	void subscribe(MsgType msgType,MsgBase* pMsg)
	{
		std::pair<MMTypMsgIt,MMTypMsgIt> result(mmSubscribers.equal_range(msgType));
		for(;result.first!=result.second;++result.first)
		{
			MsgBase* pCurrentMsgBase(result.first->second);
			if(pCurrentMsgBase==pMsg)
			{
				return;
			}
		}
		mmSubscribers.insert(std::make_pair(msgType,pMsg));
	}
	void unsubscribe(MsgType msgType,MsgBase* pMsg)
	{
		std::pair<MMTypMsgIt,MMTypMsgIt> result(mmSubscribers.equal_range(msgType));
		for(;result.first!=result.second;++result.first)
		{
			MsgBase* pCurrentMsgBase(result.first->second);
			if(pCurrentMsgBase==pMsg)
			{
				mmSubscribers.erase(result.first);
				return;
			}
		}
		std::cerr<<"Publisher::unsubscribe(), warning, no such subscriber found"<<std::endl;
	}

protected:
	MMTypMsg mmSubscribers;
	size_t progress_todo;
	size_t progress_done;
	size_t progress_broadcast_interval;
	std::string progress_name;
	bool bEndPublished;
};

} // Namespace
