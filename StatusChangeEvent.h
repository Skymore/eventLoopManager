//
// Created by 陶睿 on 3/27/24.
//

#ifndef EVENTLOOPMANAGER_STATUSCHANGEEVENT_H
#define EVENTLOOPMANAGER_STATUSCHANGEEVENT_H
#include "Event.h"

// 自定义事件类型
class StatusChangeEvent : public Event {
public:
    std::string status;

    StatusChangeEvent(const std::string &status) : status(status) {}
};


#endif //EVENTLOOPMANAGER_STATUSCHANGEEVENT_H
