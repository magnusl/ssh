#ifndef _CHANNEL_ACTION_H_
#define _CHANNEL_ACTION_H_

#include "ssh_channel.h"

namespace ssh
{
    /* Class:           ChannelAction
     * Description:     Represents a operation on a channel, e.g. sending channel data
     *                  or update the channel window.
     */
    class ChannelAction
    {
    public:
        enum Action {SendData = 0, UpdateWindow};
        ChannelAction(ssh_channel * channel, ChannelAction::Action op) : m_channel(channel), m_op(op) {
        }

        ssh_channel * m_channel;
        Action m_op;
    };

    /* Class:           ChannelActionCompare
     * Description:     Used to remove a channel action from a list.
     */
    class ChannelActionCompare
    {
    public:
        ChannelActionCompare(ssh_channel * channel) : m_channel(channel) {
        }

        bool operator()(const ChannelAction & action) {
            return action.m_channel == m_channel;
        }

    private:
        ssh_channel * m_channel;        
    };
};

#endif