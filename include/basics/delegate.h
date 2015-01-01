//
//  delegate.h
//
//  Created by Edmund Kapusniak on 21/10/2014.
//  Copyright (c) 2014 Edmund Kapusniak. All rights reserved.
//


#ifndef DELEGATE_H
#define DELEGATE_H



/*
    Delegate class based on fast delegates:
    
    http://www.codeproject.com/Articles/7150/Member-Function-Pointers-and-the-Fastest-Possible
    http://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates

*/


#include <cstddef>
#include <memory>


template < typename signature_t >
class delegate;

template < typename return_t, typename ... arguments_t >
class delegate< return_t( arguments_t ... ) >
{
public:

    delegate();
    delegate( std::nullptr_t );
    delegate( return_t (*function)( arguments_t ... ) );
    template < typename target_t > delegate( target_t* target, return_t (target_t::*method)( arguments_t ... ) );
    template < typename target_t > delegate( const target_t* target, return_t (target_t::*method)( arguments_t ... ) const );

    operator bool () const;
    bool operator == ( const delegate& b ) const;
    bool operator != ( const delegate& b ) const;
    
    return_t operator () ( arguments_t&& ... arguments );


private:
    
    class undefined;
    typedef return_t (*function_type)( arguments_t ... );
    typedef return_t (undefined::*method_type)( arguments_t ... );
    
    undefined* target;
    union
    {
        function_type function;
        method_type method;
    };
    
};


template < typename target_t, typename return_t, typename ... arguments_t >
delegate< return_t( arguments_t ... ) > bind( target_t* target,
                return_t (target_t::*method)( arguments_t ... ) );
template < typename target_t, typename return_t, typename ... arguments_t >
delegate< return_t( arguments_t ... ) > bind( const target_t* target,
                return_t (target_t::*method)( arguments_t ... ) const );


namespace delegate_impl
{

template < typename object_t, typename signature_t >
struct member_pointer;

template < typename object_t, typename return_t, typename ... arguments_t >
struct member_pointer< object_t, return_t ( arguments_t ... ) >
{
    typedef return_t (object_t::*type)( arguments_t ... );
    typedef return_t (object_t::*const_type)( arguments_t ... ) const;
};

}

template < typename signature_t, typename target_t >
delegate< signature_t > bind( target_t* target,
    typename delegate_impl::member_pointer< target_t, signature_t >::type method );
template < typename signature_t, typename target_t >
    delegate< signature_t > bind( const target_t* target,
        typename delegate_impl::member_pointer< target_t, signature_t >::const_type method );





template < typename return_t, typename ... arguments_t >
delegate< return_t( arguments_t ... ) >::delegate()
    :   target( nullptr )
    ,   function( nullptr )
{
}

template < typename return_t, typename ... arguments_t >
delegate< return_t( arguments_t ... ) >::delegate( std::nullptr_t )
    :   target( nullptr )
    ,   function( nullptr )
{
}

template < typename return_t, typename ... arguments_t >
delegate< return_t( arguments_t ... ) >::delegate(
                return_t (*function)( arguments_t ... ) )
    :   target( nullptr )
    ,   function( function )
{
}

template < typename return_t, typename ... arguments_t >
template < typename target_t >
delegate< return_t( arguments_t ... ) >::delegate(
            target_t* target, return_t (target_t::*method)( arguments_t ... ) )
    :   target( (undefined*)target )
    ,   method( (method_type)method )
{
}

template < typename return_t, typename ... arguments_t >
template < typename target_t >
delegate< return_t( arguments_t ... ) >::delegate(
            const target_t* target, return_t (target_t::*method)( arguments_t ... ) const )
    :   target( (undefined*)target )
    ,   method( (method_type)method )
{
}



template < typename return_t, typename ... arguments_t >
delegate< return_t( arguments_t ... ) >::operator bool () const
{
    return target ? method != nullptr : function != nullptr;
}


template < typename return_t, typename ... arguments_t >
bool delegate< return_t( arguments_t ... ) >::
                operator == ( const delegate& b ) const
{
    return target == b.target &&
            target ? method == b.method : function == b.function;
}

template < typename return_t, typename ... arguments_t >
bool delegate< return_t( arguments_t ... ) >::
                operator != ( const delegate& b ) const
{
    return !( *this == b );
}

    
template < typename return_t, typename ... arguments_t >
return_t delegate< return_t( arguments_t ... ) >::
                operator () ( arguments_t&& ... arguments )
{
    if ( target )
        return ( target->*method )( std::forward< arguments_t >( arguments ) ... );
    else
        return function( std::forward< arguments_t >( arguments ) ... );
}



template < typename target_t, typename return_t, typename ... arguments_t >
delegate< return_t( arguments_t ... ) > bind( target_t* target,
                return_t (target_t::*method)( arguments_t ... ) )
{
    return delegate< return_t( arguments_t ... ) >( target, method );
}

template < typename target_t, typename return_t, typename ... arguments_t >
delegate< return_t( arguments_t ... ) > bind( const target_t* target,
                return_t (target_t::*method)( arguments_t ... ) const )
{
    return delegate< return_t( arguments_t ... ) >( target, method );
}



template < typename signature_t, typename target_t >
delegate< signature_t > bind( target_t* target,
    typename delegate_impl::member_pointer< target_t, signature_t >::type method )
{
    return delegate< signature_t >( target, method );
}

template < typename signature_t, typename target_t >
    delegate< signature_t > bind( const target_t* target,
        typename delegate_impl::member_pointer< target_t, signature_t >::const_type method )
{
    return delegate< signature_t >( target, method );
}















#if USING_STD_BIND


/*
    Returns a std::function which calls a method on an object.
    
    When the method has no overloads the signature of the method can be
    deduced by the compiler:
 
        delegate( object, &object::method )
 
 
    Otherwise provide a function type to select a particular overload, e.g:
 
        delegate< void( int ) >( object, &object::method )
 
 
    I think this is all legal.  The part I am most wary of is the overloading
    of the function template with the explicit-signature forms.
*/



#include <functional>


namespace delegate_impl
{

template < int n >
struct placeholder
{
};

}

namespace std
{

template < int n >
struct is_placeholder< delegate_impl::placeholder< n > >
    :   std::integral_constant< int, n >
{
};

}

namespace delegate_impl
{

template < typename object_t, typename signature_t >
struct member_pointer;

template < typename object_t, typename return_t, typename ... arguments_t >
struct member_pointer< object_t, return_t ( arguments_t ... ) >
{
    typedef return_t (object_t::*type)( arguments_t ... );
};

template < typename object_t, typename signature_t >
struct const_member_pointer;

template < typename object_t, typename return_t, typename ... arguments_t >
struct const_member_pointer< object_t, return_t ( arguments_t ... ) >
{
    typedef return_t (object_t::*type)( arguments_t ... ) const;
};

template < int n >
struct bind_delegate
{
    template < typename object_t, typename return_t, typename ... arguments_t, typename ... placeholders_t >
    static std::function< return_t( arguments_t ... ) > bind(
                object_t* object, return_t (object_t::*method)( arguments_t ... ), placeholders_t ... placeholders )
    {
        return bind_delegate< n - 1 >::bind( object, method, placeholder< n >(), placeholders ... );
    }

    template < typename object_t, typename return_t, typename ... arguments_t, typename ... placeholders_t >
    static std::function< return_t( arguments_t ... ) > bind(
                const object_t* object, return_t (object_t::*method)( arguments_t ... ) const, placeholders_t ... placeholders )
    {
        return bind_delegate< n - 1 >::bind( object, method, placeholder< n >(), placeholders ... );
    }
};

template <>
struct bind_delegate< 0 >
{
    template < typename object_t, typename return_t, typename ... arguments_t, typename ... placeholders_t >
    static std::function< return_t( arguments_t ... ) > bind(
                object_t* object, return_t (object_t::*method)( arguments_t ... ), placeholders_t ... placeholders )
    {
        return std::bind( method, *object, placeholders ... );
    }

    template < typename object_t, typename return_t, typename ... arguments_t, typename ... placeholders_t >
    static std::function< return_t( arguments_t ... ) > bind(
                const object_t* object, return_t (object_t::*method)( arguments_t ... ) const, placeholders_t ... placeholders )
    {
        return std::bind( method, *object, placeholders ... );
    }
};
    
};

template < typename object_t, typename return_t, typename ... arguments_t >
std::function< return_t( arguments_t ... ) >
            delegate( object_t* object, return_t (object_t::*method)( arguments_t ... ) )
{
    const int n = sizeof ... ( arguments_t );
    return delegate_impl::bind_delegate< n >::bind( object, method );
}

template < typename object_t, typename return_t, typename ... arguments_t >
std::function< return_t( arguments_t ... ) >
            delegate( const object_t* object, return_t (object_t::*method)( arguments_t ... ) const )
{
    const int n = sizeof ... ( arguments_t );
    return delegate_impl::bind_delegate< n >::bind( object, method );
}

template < typename object_t, typename return_t, typename ... arguments_t >
std::function< return_t( arguments_t ... ) >
            delegate( object_t& object, return_t (object_t::*method)( arguments_t ... ) )
{
    const int n = sizeof ... ( arguments_t );
    return delegate_impl::bind_delegate< n >::bind( &object, method );
}

template < typename object_t, typename return_t, typename ... arguments_t >
std::function< return_t( arguments_t ... ) >
            delegate( const object_t& object, return_t (object_t::*method)( arguments_t ... ) const )
{
    const int n = sizeof ... ( arguments_t );
    return delegate_impl::bind_delegate< n >::bind( &object, method );
}

template < typename signature_t, typename object_t >
std::function< signature_t > delegate( object_t* object,
            typename delegate_impl::member_pointer< object_t, signature_t >::type method )
{
    return delegate( object, method );
}

template < typename signature_t, typename object_t >
std::function< signature_t > delegate( const object_t* object,
            typename delegate_impl::const_member_pointer< object_t, signature_t >::type method )
{
    return delegate( object, method );
}

template < typename signature_t, typename object_t >
std::function< signature_t > delegate( object_t& object,
            typename delegate_impl::member_pointer< object_t, signature_t >::type method )
{
    return delegate( object, method );
}

template < typename signature_t, typename object_t >
std::function< signature_t > delegate( const object_t& object,
            typename delegate_impl::const_member_pointer< object_t, signature_t >::type method )
{
    return delegate( object, method );
}



#endif


#endif
