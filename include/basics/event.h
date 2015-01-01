//
//  event.h
//
//  Created by Edmund Kapusniak on 03/05/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#ifndef EMK_EVENT_H
#define EMK_EVENT_H


#include <forward_list>
#include "delegate.h"


template < typename ... arguments_t >
class event
{
public:

    typedef ::delegate_t< void, arguments_t ... > delegate_t;

    void subscribe( delegate_t delegate );
    void unsubscribe( delegate_t delegate );
    
    void operator () ( arguments_t ... arguments ) const;
    
    
private:

    std::forward_list< delegate_t > subscribers;

};




template < typename ... arguments_t >
void event< arguments_t ... >::subscribe( delegate_t delegate )
{
    subscribers.push_front( delegate );
}


template < typename ... arguments_t >
void event< arguments_t ... >::unsubscribe( delegate_t delegate )
{
    for ( auto previous = subscribers.before_begin(), i = subscribers.begin();
                i != subscribers.end(); previous = i++ )
    {
        if ( *i == delegate )
        {
            subscribers.erase_after( previous );
            break;
        }
    }
}


template < typename ... arguments_t >
inline void event< arguments_t ... >::
                operator() ( arguments_t ... arguments ) const
{
    for ( auto i = subscribers.begin(); i != subscribers.end(); ++i )
    {
        const delegate_t& subscriber = *i;
        subscriber( arguments ... );
    }
}



#endif

