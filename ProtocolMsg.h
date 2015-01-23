
#pragma once

#include <iostream>
#include <string>

namespace Msg
{
    const std::string LoginData = "ld";
    const std::string RegisterNewUser = "reg";
    const std::string UserList = "ul";
    const std::string RequestUserDetails = "ud";
    const std::string LogOut = "lo";
    const std::string UsernameHeader = "u";
    const std::string PasswordHeader = "p";
    const std::string IPHeader = "ip";

    const std::string Error = "er";
    enum Errors
    {
        EXISTING_USERNAME,
        INVALID_PASSWORD,
        USER_UNAVAILABLE,
        INVALID_MESSAGE
    };

    class Buffer
    {
    public:
        Buffer( unsigned size )
        :   m_maxSize( size ),
            m_curSize( 0 )
        {
            m_buffer = new char[ size ];
        }

        ~Buffer()
        {
            if( m_buffer )
                delete[] m_buffer;
        }

        friend Buffer& operator>>( Buffer& buffer, std::string& str )
        {
            str = std::string( buffer.GetNextWord() );

            return buffer;
        }

        friend Buffer& operator<<( Buffer& buffer, const std::string& str )
        {
            buffer.AddWord( str );

            return buffer;
        }

       /* friend Buffer& operator<<( Buffer& buffer, const char* str )
        {
            buffer.AddWord( std::string( str ) );

            return buffer;
        }*/

        std::string GetNextWord()
        {
            std::string ret;
            unsigned i = 0;
            while( m_buffer[ i ] != ' ' && i <= m_curSize )
                ret.push_back( m_buffer[ i++ ] );

            i++;
            m_curSize -= i;
            memcpy( m_buffer, m_buffer + i, m_curSize );

            return ret;
        }

        void AddWord( const std::string word )\
        {
            if( word.size() >= m_maxSize - m_curSize )
                return;

            memcpy( m_buffer + m_curSize, word.c_str(), word.size() );
            m_curSize += word.size();

            m_buffer[ m_curSize++ ] = ' ';
        }

        inline bool IsEmpty() const { return !(bool)m_curSize; }

    private:
        char* m_buffer;
        unsigned m_maxSize, m_curSize;

    };

}
